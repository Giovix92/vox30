/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <unistd.h>
#include <net/if.h>

#include "lib/config.h"
#include "lib/urandom.h"
#include "lib/event.h"
#include "lib/insmod.h"
#include "lib/str.h"
#include "core/hotspotd.h"
#include "core/client.h"
#include "core/trigger.h"
#include "core/routing.h"
#include "core/firewall.h"

#define TUNNEL_PPPIFCOUNT HOTSPOT_LIMIT_RES
#define TUNNEL_SERVERCOUNT 2
#define TUNNEL_LISTCOUNT 4

struct tunnel {
	struct client *cl;
	client_update_cb *report;
	struct trigger_handle *trigger;
	int handle;
	int policy_ipv4;
	int policy_ipv6;
	int32_t timeout;
	bool connected;
};

static enum tunnel_type {
	TUNNEL_NONE,
	TUNNEL_L2TP,
    TUNNEL_GRE,
} tunnel_type = TUNNEL_NONE;

#ifdef WITH_TUNNEL_L2TP
#include "l2tp.h"
static void tunnel_l2tp_cb(int handle, enum l2tp_state state,
					uint32_t code, void *ctx);
static struct l2tp_cfg l2tp_cfg = { .cb = tunnel_l2tp_cb };
static struct l2tp_lac *l2tp_lac = NULL, *l2tp_lac_old = NULL;
static enum l2tp_state l2tp_oldstate = L2TP_NONE;
static int l2tp_tries = 0;
static int tunnel_l2tp_up();
#endif

static char* tunnel_servers[TUNNEL_SERVERCOUNT] = {NULL};
static size_t tunnel_server_count = 0;
static int tunnel_server_index = 0;
static char* pppd_paths[] = {"/usr/sbin/pppd", "/sbin/pppd"};

static struct tunnel* pppif_alloc[TUNNEL_PPPIFCOUNT] = {NULL};
static int pppd_offset, pppd_timeout;
static const char *lcp_echo_interval, *lcp_echo_failure;
static const char *pppd_path = NULL, *pppd_config = NULL;
static const struct client_backend *client_fallback = NULL, *eap_handler = NULL;

static void tunnel_ifevent(struct event_signal *ev, const siginfo_t *sig);
static void tunnel_timeout(struct event_timer *timer, int64_t now);
static void tunnel_shutdown(int tunnelid, int status);
static int tunnel_startppp(int tunnelid);
static int tunnel_open(struct client *cl, client_update_cb *report);
static void tunnel_close();
static void tunnel_pppterm(void *data, short code, short status);


static struct event_signal event_iface = {
	.signal = SIGNAL_ROUTING_IFSTATE,
	.handler = tunnel_ifevent,
};

static struct event_timer event_timer = {
	.interval = 5000,
	.handler = tunnel_timeout,
};

struct tunnel_apply_args {
	int idx;
	char *templatefill;
};

static void tunnel_apply_server(const char *server, void *ctx) {
	struct tunnel_apply_args *args = (struct tunnel_apply_args *)ctx;

	char *__server = NULL;
	__server = (char *)malloc(128);
	if(args->templatefill) {
		char *__templatefill = strdup(args->templatefill);
		snprintf(__server, 128, server, strlwr(__templatefill));
		free(__templatefill);
	} else {
		snprintf(__server, 128, server);
	}

	if (args->idx < TUNNEL_SERVERCOUNT)
		tunnel_servers[(args->idx)++] = __server;
}

static void free_tunnelservers() 
{
	for(int ii=0; ii<TUNNEL_SERVERCOUNT; ii++) {
		free(tunnel_servers[ii]);
		tunnel_servers[ii] = NULL;
	}
}

static int tunnel_apply() {
	free_tunnelservers();

	if (!tunnel_type)
		return 0;

	pppd_config = config_get_string("tunnel", "pppd_config", NULL);
	if(!hotspot_assertconf_string("tunnel.pppd_config", pppd_config)) {
		return -1;
	}
	pppd_timeout = config_get_int("tunnel", "pppd_timeout", 0);
	if(!hotspot_assertconf_int("tunnel.pppd_timeout", &pppd_timeout)) {
		return -1;
	}

	const char *fallback = config_get_string("tunnel", "fallback", NULL);
	if (fallback && !strcmp(fallback, "true"))
		fallback = config_get_string("tunnel", "fallback_handler", "radius");

	if (!strcmp(config_get_string("tunnel", "enabled", "false"), "true")) {
		config_set_string("client", "auth", "ppp");
		config_set_string("tunnel", "eap_handler", "radius");
		config_set_string("radauth", "next", "ppp");
	} else {
		config_set_string("client", "auth", "radius");
		config_set_string("radauth", "next", NULL);
	}

	const char *eaphndl = config_get_string("tunnel", "eap_handler", NULL);
	client_fallback = client_get_backend(fallback);
	eap_handler = client_get_backend(eaphndl);

	const char *val = config_get_string("tunnel", "pppd_path", NULL);
	if (!val && !access(pppd_paths[0], X_OK)) {
		pppd_path = pppd_paths[0];
	} else if (!val && !access(pppd_paths[1], X_OK)) {
		pppd_path = pppd_paths[1];
	} else if (val && !access(val, X_OK)) {
		pppd_path = val;
	} else {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel: PPP daemon not found: %s", val);
#else
		syslog(LOG_ERR, "tunnel: PPP daemon not found: %s", val);
#endif
		return -1;
	}

	lcp_echo_interval = config_get_string("tunnel", "lcp_echo_interval", NULL);
	if(!hotspot_assertconf_string("tunnel.lcp_echo_interval", lcp_echo_interval)) {
		return -1;
	}
	lcp_echo_failure = config_get_string("tunnel", "lcp_echo_failure", NULL);
	if(!hotspot_assertconf_string("tunnel.lcp_echo_failure", lcp_echo_failure)) {
		return -1;
	}
#ifndef __SC_BUILD__	
	syslog(LOG_DEBUG, "tunnel: PPP daemon found: %s\n", pppd_path);
#endif
	if (tunnel_type == TUNNEL_L2TP) {
#ifdef WITH_TUNNEL_L2TP
		l2tp_cfg.hello_delay = config_get_int("tunnel",
						"l2tp_hello_delay", 0);
		if(!hotspot_assertconf_u16("tunnel.l2tp_hello_delay",
				&l2tp_cfg.hello_delay)) {
			return -1;
		}
		l2tp_cfg.max_retry = config_get_int("tunnel",
						"l2tp_max_retry", 0);
		if(!hotspot_assertconf_u16("tunnel.l2tp_max_retry",
				&l2tp_cfg.max_retry)) {
			return -1;
		}
		l2tp_cfg.rtx_delay = config_get_int("tunnel",
						"l2tp_rtx_delay", 0);
		if(!hotspot_assertconf_u16("tunnel.l2tp_rtx_delay",
				&l2tp_cfg.rtx_delay)) {
			return -1;
		}
		l2tp_cfg.start_max_retry = config_get_int("tunnel",
				"l2tp_start_max_retry", 0);
		if(!hotspot_assertconf_u16("tunnel.l2tp_start_max_retry",
				&l2tp_cfg.start_max_retry)) {
			return -1;
		}
		l2tp_cfg.start_rtx_delay = config_get_int("tunnel",
				"l2tp_start_rtx_delay", 0);
		if(!hotspot_assertconf_u16("tunnel.l2tp_start_rtx_delay",
				&l2tp_cfg.start_rtx_delay)) {
			return -1;
		}
#endif
	}

	tunnel_server_index = 0;
	struct tunnel_apply_args args;
	args.idx = 0;
	args.templatefill = NULL;
	config_foreach_list("tunnel", "server", tunnel_apply_server, &args);
	if(!args.idx) {
		args.templatefill = devname;
		config_foreach_list("tunnel", "templateserver", tunnel_apply_server,
				&args);
	}

	tunnel_server_count = args.idx;
	if (tunnel_server_count) {
		tunnel_server_index = 0;	// Start always with first server in list
		//urandom(&tunnel_server_index, sizeof(tunnel_server_index));
		//tunnel_server_index %= tunnel_server_count;
	}

	return 0;
}


static int tunnel_init() {
	const char *val = config_get_string("tunnel", "type", "none");
	if (!strcmp(val, "none")) {
		tunnel_type = TUNNEL_NONE;
		return 0;
#ifdef WITH_TUNNEL_L2TP
	} else if (!strcmp(val, "l2tp")) {
		tunnel_type = TUNNEL_L2TP;
		if (insmod("l2tp_ppp", NULL))
			insmod("pppol2tp", NULL);
#ifndef __SC_BUILD__
		syslog(LOG_INFO, "tunnel: Initializing L2TP support.");
#endif
#endif
	} else {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Invalid tunnel type: %s", val);
#else
		syslog(LOG_ERR, "Invalid tunnel type: %s", val);
#endif
		return -1;
	}

	pppd_offset = config_get_int("tunnel", "pppd_offset", 10000);
	event_ctl(EVENT_SIGNAL_ADD, &event_iface);
	event_ctl(EVENT_TIMER_ADD, &event_timer);
	return tunnel_apply();
}


static void tunnel_deinit() {
	if (!tunnel_type)
		return;

	event_ctl(EVENT_SIGNAL_DEL, &event_iface);
	event_ctl(EVENT_TIMER_DEL, &event_timer);

	// Send SIGTERM to all PPPd processes
	for (int i = 0; i < TUNNEL_PPPIFCOUNT; ++i)
		if (pppif_alloc[i])
			tunnel_shutdown(i, LOGOUT_ADMIN_REBOOT);

#ifdef WITH_TUNNEL_L2TP
	if (tunnel_type == TUNNEL_L2TP) {
		tunnel_close();
	}
#endif
	free_tunnelservers();
}

static void tunnel_handler(struct client *client, enum client_req req,
		client_update_cb *report, void **ctx) {
	intptr_t handle = (intptr_t)*ctx;
	if (req == CLIENT_LOGIN) {
		if (client->sess.method == CLIENT_EAP && eap_handler) {
			report(client, LOGIN_NOTYET, eap_handler);
		} else {
			handle = tunnel_open(client, report) + 1;
			if (!handle)
				report(client, LOGIN_ERROR, NULL);
			else
				*ctx = (void*)handle;
		}
	} else if (req == CLIENT_LOGOUT && handle) {
		*ctx = NULL;
		tunnel_shutdown(handle - 1, LOGOUT_USER);
	}
}

static int tunnel_open(struct client *cl, client_update_cb *report) {
	int tunnelid = -1;
	struct tunnel *tun = NULL;

	for (int i = 0; i < TUNNEL_PPPIFCOUNT; ++i) {
		if (!pppif_alloc[i]) {
			tunnelid = i;
			break;
		}
	}
	if (tunnelid < 0) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Out of tunnel slots. Leak?");
#else
		syslog(LOG_WARNING, "Out of tunnel slots. Leak?");
#endif
		goto err;
	}

	if (!(tun = calloc(1, sizeof(struct tunnel))))
		goto err;

	tun->cl = cl;
	tun->timeout = event_time() / 1000 + pppd_timeout;
	tun->report = report;
	tun->connected = false;
	pppif_alloc[tunnelid] = tun;
#ifndef __SC_BUILD__
	syslog(LOG_DEBUG, "tunnel: Created tunnel %i", tunnelid);
#endif
	if (0) {
		// Dummy
#ifdef WITH_TUNNEL_L2TP
	} else if (tunnel_type == TUNNEL_L2TP) {
		// Make sure the tunnel is up
		if (tunnel_l2tp_up())
			goto err;
#ifndef __SC_BUILD__
		syslog(LOG_DEBUG, "tunnel: Creating L2TP session for tunnel %i...", tunnelid);
#endif
		if (!(tun->handle = l2tp_session(l2tp_lac)))
			goto err;
#endif
	} else {
		goto err;
	}

	return tunnelid;

err:
	free(tun);
	pppif_alloc[tunnelid] = NULL;
	return -1;
}

static void tunnel_close()
{
	if (l2tp_lac && l2tp_lac_old && tunnel_type == TUNNEL_L2TP) {
		l2tp_destroy(l2tp_lac_old);
		l2tp_lac_old = NULL;
		l2tp_destroy(l2tp_lac);
		l2tp_lac = NULL;
		l2tp_lac_old = NULL;
	}
}

static int tunnel_startppp(int tunnelid) {
#ifndef __SC_BUILD__
	syslog(LOG_INFO, "Starting pppd for tunnel %i", tunnelid + 1);
#endif
	struct tunnel *tun = pppif_alloc[tunnelid];
	int passwdfd[2] = {-1, -1};
	int duphndl = -1;
	char passwdfdname[16], pppunit[16];
	if (tun->cl->sess.keylen > 256 || pipe(passwdfd))
		goto err;

	snprintf(passwdfdname, sizeof(passwdfdname), "%i", passwdfd[0]);
	snprintf(pppunit, sizeof(pppunit), "%i", pppd_offset + tunnelid);

	// Write password to pipe
#ifndef DEBUG_TUNNEL
	if (tun->cl->sess.key &&
	write(passwdfd[1], tun->cl->sess.key, tun->cl->sess.keylen)) {
		// Make compiler happy
	}
#else
	if (write(passwdfd[1], "test", 4)) {
		// Make compiler happy
	}
#endif
	close(passwdfd[1]);

	int argc = 0;
	const char *argv[32];
	char buffer[16];

	if (pppd_config) {
		argv[argc++] = "file";
		argv[argc++] = pppd_config;
	}
	argv[argc++] = "unit";
	argv[argc++] = pppunit;
	argv[argc++] = "plugin";
	if (tunnel_type == TUNNEL_L2TP) {
		argv[argc++] = "pppol2tp.so";
		argv[argc++] = "pppol2tp";
		duphndl = dup(tun->handle);
		snprintf(buffer, sizeof(buffer), "%i", duphndl);
		argv[argc++] = buffer;
	} else {
		goto err;
	}
	if (tun->cl->sess.key) {
		argv[argc++] = "plugin";
		argv[argc++] = "passwordfd.so";
		argv[argc++] = "passwordfd";
		argv[argc++] = passwdfdname;
	}
	argv[argc++] = "noauth";
	argv[argc++] = "nodetach";

	if (lcp_echo_interval) {
		argv[argc++] = "lcp-echo-interval";
		argv[argc++] = lcp_echo_interval;
	}

	if (lcp_echo_failure) {
		argv[argc++] = "lcp-echo-failure";
		argv[argc++] = lcp_echo_failure;
	}

	if (tun->cl->sess.username) {
		argv[argc++] = "user";
#ifndef DEBUG_TUNNEL
		argv[argc++] = tun->cl->sess.username;
#else
		argv[argc++] = "test";
#endif
	}
	argv[argc] = NULL;

	tun->trigger = trigger_run(pppd_path, (char**)argv, NULL,
			tunnel_pppterm, (void*)((intptr_t)tunnelid));
	if (!tun->trigger)
		goto err;

	close(duphndl);
	close(passwdfd[0]);
	return 0;

err:
	close(duphndl);
	close(passwdfd[0]);
	tunnel_shutdown(tunnelid, LOGOUT_NAS_ERROR);
	return -1;
}


static void tunnel_shutdown(int tunnelid, int status) {
	struct tunnel *tun;
	if (tunnelid < 0 || tunnelid >= TUNNEL_PPPIFCOUNT
	|| !(tun = pppif_alloc[tunnelid]))
		return;

	// Prevent from recursive calling
	pppif_alloc[tunnelid] = NULL;
#ifdef __SC_BUILD__
    log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Tunnel shutdown %i (code %i)",
				tunnelid + 1, status);
#else
	syslog(LOG_INFO, "Tunnel shutdown %i (code %i)",
				tunnelid + 1, status);
#endif
	// This is to inform client mgmt on SIGCHLD events
	if (tun->cl && (tun->connected || !client_fallback)) {
		struct client *cl = tun->cl;
		tun->cl = NULL;
		if (!tun->connected && status < LOGIN_ALREADY)
			status = LOGIN_TIMEOUT;
		tun->report(cl, status, NULL);
	}


	char devname[16];
	snprintf(devname, sizeof(devname), "ppp%i",
				pppd_offset + tunnelid);

	firewall_set_nat(devname, false);

	if (tun->trigger)
		trigger_cancel(tun->trigger);

	if (tun->policy_ipv4)
		routing_policy_del(tun->policy_ipv4);

	if (tun->policy_ipv6)
		routing_policy_del(tun->policy_ipv6);

#ifdef WITH_TUNNEL_L2TP
	if (tun->handle && tunnel_type == TUNNEL_L2TP) {
		l2tp_shutdown(l2tp_lac, tun->handle, 2);
		tun->handle = 0;
	}
#endif

	if (tun->cl && !tun->connected && client_fallback)
		tun->report(tun->cl, LOGIN_NOTYET, client_fallback);

	free(tun);
}

#ifdef WITH_TUNNEL_L2TP
// This is called by the l2tp lib whenever an event occurs
static void tunnel_l2tp_cb(int handle, enum l2tp_state state,
					uint32_t code, void *ctx) {
	// Event for a single session
	if (handle) {
		for (int i = 0; i < TUNNEL_PPPIFCOUNT; ++i) {
			struct tunnel *tun;
			if (!(tun = pppif_alloc[i]) || tun->handle != handle)
				continue;

			if (state == L2TP_ESTABLISHED) {
				tunnel_startppp(i);
			} else {
				tun->handle = 0;
				tunnel_shutdown(i, LOGOUT_SERVICE_UNAVAILABLE);
			}
			break;
		}
		return;
	}

	if (state == L2TP_ESTABLISHED) {
		// Update last state (tunnel was established)
#ifdef __SC_BUILD__
        log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel: LNS connection established\n");
#else
		syslog(LOG_INFO, "tunnel: LNS connection established");
#endif
		l2tp_oldstate = state;
		l2tp_tries = 0;
		return;
	}


	//At this point we have a global tunnel shutdown
	if (l2tp_oldstate == L2TP_ESTABLISHED || tunnel_l2tp_up()) {
		// The tunnel was established before or no more LNS to try
		// shutdown all sessions
		for (int i = 0; i < TUNNEL_PPPIFCOUNT; ++i)
			if (pppif_alloc[i]) {
				pppif_alloc[i]->handle = 0;
				tunnel_shutdown(i, LOGOUT_LOST_SERVICE);
			}

		tunnel_close();
		if (l2tp_oldstate != L2TP_ESTABLISHED)
#ifdef __SC_BUILD__
            log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel: No more LNS to try. Giving up.");
#else
			syslog(LOG_INFO, "tunnel: No more LNS to try. Giving up.");
#endif
		// Reset try counter and offset, so that the next tunnel attempt
		// does not fail immediately without trying the LNS again
		l2tp_tries = 0;
	} else {
		// We switched to the next LNS
		// Our old handles are invalid now, refresh them
		for (int i = 0; i < TUNNEL_PPPIFCOUNT; ++i)
			if (pppif_alloc[i] && (!(pppif_alloc[i]->handle =
						l2tp_session(l2tp_lac))))
				tunnel_shutdown(i, LOGOUT_NAS_ERROR); // Couldn't refresh
	}
	l2tp_oldstate = L2TP_NONE;
}

static int tunnel_l2tp_up() {
	if (l2tp_lac_old) {
		l2tp_destroy(l2tp_lac_old);
		l2tp_lac_old = NULL;
	}

	if (tunnel_server_count < 1)
		return -1;

	if (l2tp_state(l2tp_lac) == L2TP_SHUTDOWN) {
		l2tp_lac_old = l2tp_lac;
		l2tp_lac = NULL;
	}

	while (l2tp_state(l2tp_lac) == L2TP_NONE) {
		if (++l2tp_tries > tunnel_server_count) {
			l2tp_tries = 0;
			return -1;
		}
#ifdef __SC_BUILD__
        log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "tunnel: Connecting to LNS %s",
				tunnel_servers[tunnel_server_index]);
#else
		syslog(LOG_INFO, "tunnel: Connecting to LNS %s",
				tunnel_servers[tunnel_server_index]);
#endif
		l2tp_lac = l2tp_create(tunnel_servers[tunnel_server_index],
								&l2tp_cfg);
		tunnel_server_index = (tunnel_server_index + 1)
							% tunnel_server_count;
	}

	return 0;
}

#endif


// PPPd SIGCHLD handler
static void tunnel_pppterm(void *data, short code, short status) {
	intptr_t tunnelid = (intptr_t)data;
	pppif_alloc[tunnelid]->trigger = NULL;

	int reason = LOGOUT_LOST_SERVICE;
	if (code == CLD_EXITED) {
		if (status == 19)
			reason = LOGIN_DENIED;
		else if (status == 10)
			reason = LOGIN_ISPFAIL_OVERLOAD;
	}

	tunnel_shutdown(tunnelid, reason);
}

// We get events if any interface changes state
static void tunnel_ifevent(struct event_signal *ev, const siginfo_t *sig) {
	if (sig->si_errno & IFF_UP
	&& sig->si_ptr && !strncmp(sig->si_ptr, "ppp", 3)) {
		unsigned pppid = atoi(((char*)sig->si_ptr) + 3) - pppd_offset;
		struct tunnel *tun;
		if (pppid < TUNNEL_PPPIFCOUNT && (tun = pppif_alloc[pppid])
		&& tun->timeout < INT32_MAX) { // Don't run this twice
			tun->timeout = INT32_MAX;

			const char *iniface = routing_cfg.iface_name;
			if (tun->cl->ifindex == routing_cfg.iface_eap_index)
				iniface = routing_cfg.iface_eap_name;

			uint32_t fwmark = firewall_id_to_authmark(
					client_id(tun->cl));
			uint32_t fwmask = FIREWALL_AUTHMASK;

			// Setup routing policies
			tun->policy_ipv4 = routing_policy_new(iniface, AF_INET,
					fwmark, fwmask, sig->si_code);

			tun->policy_ipv6 = routing_policy_new(iniface, AF_INET6,
					fwmark, fwmask, sig->si_code);

			// TODO: Make NAT configurable?
			firewall_set_nat((const char*)sig->si_ptr, true);

			if (tun->cl)
				tun->report(tun->cl, LOGIN_SUCCESS, NULL);

			tun->connected = true;
		}
	}
}

// Regularly check for tunnel timeouts
static void tunnel_timeout(struct event_timer *timer, int64_t now) {
	for (int i = 0; i < TUNNEL_PPPIFCOUNT; ++i) {
		struct tunnel *tun;
		if (!(tun = pppif_alloc[i]) || tun->timeout > now / 1000)
			continue;
#ifdef __SC_BUILD__
        log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Tunnel timeout reached for %i", i + 1);
#else
		syslog(LOG_WARNING, "Tunnel timeout reached for %i", i + 1);
#endif
		tunnel_shutdown(i, LOGIN_TIMEOUT);
	}
}

static struct client_backend ppp_backend = {
	.auth = tunnel_handler,
	.name = "ppp",
};

BACKEND_REGISTER(ppp_backend)
MODULE_REGISTER(tunnel, 305)
