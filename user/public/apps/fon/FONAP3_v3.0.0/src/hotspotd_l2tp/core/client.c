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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log.h>
#ifdef CONFIG_SUPPORT_WEB_PRIVOXY
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#endif
#include <unistd.h>

#include "lib/event.h"
#include "lib/config.h"
#include "lib/urandom.h"
#include "client.h"
#include "neigh.h"
#include "hotspotd.h"
#include "firewall.h"
#include "routing.h"
#include "traffic.h"
#include "trigger.h"

static struct client *clients = NULL;
static unsigned clients_max = 0, clients_cmax = 0; // Absolute and current max
#ifdef __SC_BUILD__
static unsigned MaxLoginUser = 0;
#endif
static unsigned client_count = 0, client_sum = 0;
static uint32_t session_id;
static int cfg_idletime = 0;
static bool cfg_confirm_addresses = false;
static client_auth_cb *backend = NULL;
static struct list_head backends = LIST_HEAD_INIT(backends);
static client_get_servicemap_cb *client_get_servicemap = NULL;
static client_set_services_cb *_client_set_services = NULL;
static client_get_cap_cb *_client_get_cap = NULL;

static const char *tr_newuser, *tr_deluser;
static const char *auth_conf, *acct_conf;
static const char *eapauth_conf, *eapacct_conf;

static void client_update(struct client *cl, enum client_login status, const struct client_backend *next);
static void client_timer(struct event_timer *timer, int64_t now);
static struct event_timer timer_acct = {
	.handler = client_timer,
};


static int client_apply() {
	int ret = 0;

	cfg_idletime = config_get_int("client", "idle_max", 0);
	if(!hotspot_assertconf_int("client.idle_max", &cfg_idletime)) {
		ret = -1;
	}
	cfg_confirm_addresses = config_get_int("client", "confirm_addresses", 0);

	tr_newuser = config_get_string("client", "trigger_newuser", NULL);
	tr_deluser = config_get_string("client", "trigger_deluser", NULL);

	const char *auth = config_get_string("client", "auth", NULL);
	const struct client_backend *be = client_get_backend(auth);
	backend = (be) ? be->auth : NULL;

	auth_conf = config_get_string("client", "auth_conf", "radauth");
	acct_conf = config_get_string("client", "acct_conf", "radacct");
	eapauth_conf = config_get_string("client", "eapauth_conf", "radeapauth");
	/* use public essid radius accounting server for EAP by default */
	eapacct_conf = config_get_string("client", "eapacct_conf", "radacct");

	if (auth && !backend) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Could not resolve client backend.\n");
#else
		syslog(LOG_WARNING, "Could not resolve client backend.");
#endif
		ret = -1;
	}

	return ret;
}
static int client_init() {
	clients_max = config_get_int("client", "limit", 0);
	if(!hotspot_assertconf_u32("client.limit", &clients_max) || 
			(clients_max > HOTSPOT_LIMIT_USER) || 
			!(clients = calloc(clients_max, sizeof(struct client)))) {
		return -1;
	}
#ifdef __SC_BUILD__
    MaxLoginUser = config_get_int("client", "MaxLoginUser", 1);
#endif
	timer_acct.interval = config_get_int("client", "timer", 15) * 1000;
	event_ctl(EVENT_TIMER_ADD, &timer_acct);

	clients_cmax = 0;
	urandom(&session_id, sizeof(session_id));
	session_id /= 2;
	return client_apply();
}

static void client_deinit() {
	event_ctl(EVENT_TIMER_DEL, &timer_acct);
	for (int i = 0; i < clients_cmax; ++i)
		if (clients[i].status)
			client_logout(&clients[i], LOGOUT_ADMIN_REBOOT);

	free(clients);
	clients = NULL;
}

int client_id(const struct client *client) {
	return client->id;
}

int client_served() {
	return client_sum;
}

// Search for a client by MAC, IPv4 or IPv6
static struct client* client_get(int ifindex, int af, const void *addr) {
	uint8_t mac[6];
	if (af == AF_INET || af == AF_INET6) {
		if (neigh_ip2mac(ifindex, mac, af, addr))
			return NULL;
		addr = mac;
		af = AF_PACKET;
	}


	for (int i = 0; i < clients_cmax; ++i) {
		struct client *c = &clients[i];
		if (c->status && ((af == AF_PACKET && c->ifindex == ifindex
				&& !memcmp(c->mac, addr, sizeof(c->mac))) ||
		(af == AF_LOCAL && *((uint32_t*)addr) == c->sess.id))) {
			return c;
		}
	}
	return NULL;
}

void client_renew_addresses(struct client *cl)
{
	union {
		struct in6_addr in6[8];
		struct in_addr in[8];
	} addr;

	ssize_t cnt = neigh_mac2ip(cl->ifindex, AF_INET, &addr, 8, cl->mac);
	if (cnt >= 0) {
		size_t len = cnt * sizeof(struct in_addr);
		free(cl->sess.req_ipaddr);
		cl->sess.req_ipaddr_cnt = 0;
		if ((cl->sess.req_ipaddr = malloc(len))) {
			memcpy(cl->sess.req_ipaddr, &addr, len);
			cl->sess.req_ipaddr_cnt = cnt;
		}
	}

	cnt = neigh_mac2ip(cl->ifindex, AF_INET6, &addr, 8, cl->mac);
	if (cnt >= 0) {
		size_t len = cnt * sizeof(struct in6_addr);
		free(cl->sess.req_ip6addr);
		cl->sess.req_ip6addr_cnt = 0;
		if ((cl->sess.req_ip6addr = malloc(len))) {
			memcpy(cl->sess.req_ip6addr, &addr, len);
			cl->sess.req_ip6addr_cnt = cnt;
		}
	}
}

static void client_commit_addresses(struct client *cl);
static void client_update_addresses(struct client *cl)
{
	if (cl->sess.addr_update_in_progress ||
			!cl->sess.addrs_need_update)
		return;

	cl->sess.addrs_need_update = false;
	cl->sess.addr_update_in_progress = true;

	client_renew_addresses(cl);

	if (cl->status == CLSTAT_AUTHED) {
		if (!cfg_confirm_addresses || !cl->sess.backend)
			client_commit_addresses(cl);
		else
			cl->sess.backend(cl, CLIENT_UPDATE_ADDRESSES,
					client_update, &cl->sess.backend_ctx);
	}
}


static void client_commit_addresses(struct client *cl)
{
	firewall_flush_auth_ips(client_id(cl));

	// Allow requested addresses
	for (ssize_t i = 0; i < cl->sess.req_ipaddr_cnt; ++i)
		firewall_add_auth_ip(client_id(cl), AF_INET,
				&cl->sess.req_ipaddr[i]);

	for (ssize_t i = 0; i < cl->sess.req_ip6addr_cnt; ++i)
		firewall_add_auth_ip(client_id(cl), AF_INET6,
				&cl->sess.req_ip6addr[i]);

	free(cl->sess.req_ipaddr);
	cl->sess.req_ipaddr = NULL;

	free(cl->sess.req_ip6addr);
	cl->sess.req_ip6addr = NULL;

	cl->sess.addr_update_in_progress = false;
	client_update_addresses(cl); // Test for new updates
}


void client_inform_address_update(int ifindex, const uint8_t mac[6])
{
	if (!clients)
		return;

	struct client *cl = client_get(ifindex, AF_PACKET, mac);
	if (cl) {
		cl->sess.addrs_need_update = true;
		client_update_addresses(cl);
	}
}


// Initiate login attempt with authserver
static void client_authenticate(struct client *cl) {
	syslog(LOG_DEBUG, "Client %i attempts login as %s",
					client_id(cl), cl->sess.username);
	cl->status = CLSTAT_LOGIN;
	if (cl->sess.method == CLIENT_GRANT || !cl->sess.backend)
		client_update(cl, LOGIN_SUCCESS, NULL);
	else
		cl->sess.backend(cl, CLIENT_LOGIN, client_update, &cl->sess.backend_ctx);
}
// Login client with given method and credentials, supplying a callback
void client_login(struct client *cl, enum client_method method,
const char *user, const uint8_t *key, size_t len,
client_cb *login_cb, void *login_ctx) {
	syslog(LOG_INFO, "Login request for client %i with client status %d",
		client_id(cl), cl->status);
	if (cl->status == CLSTAT_LOGIN) {
		// Login is pending, abort old auth handle
		cl->login_status = LOGOUT_ABORT;
		if (cl->sess.backend)
			cl->sess.backend(cl, CLIENT_LOGOUT, client_update,
					&cl->sess.backend_ctx);

		cl->sess.backend_ctx = NULL;
	} else if (cl->status != CLSTAT_NOAUTH) {
		int login_reason = LOGIN_BUSY;
		if (cl->status == CLSTAT_AUTHED)
			login_reason = LOGIN_SUCCESS;

		if (login_cb)
			login_cb(cl, login_reason, login_ctx);
		return;
	}

	if (!user && cl->sess.username)
		user = cl->sess.username;

	if (method)
		cl->sess.method = method;

	if (method == CLIENT_EAP) {
		cl->sess.auth_conf = strndup(eapauth_conf, strlen(eapauth_conf));
		cl->sess.acct_conf = strndup(eapacct_conf, strlen(eapacct_conf));
	} else {
		cl->sess.auth_conf = strndup(auth_conf, strlen(auth_conf));
		cl->sess.acct_conf = strndup(acct_conf, strlen(acct_conf));
	}

	if (!cl->sess.login_cb) { // don't override old login cb
		cl->sess.login_cb = login_cb;
		cl->sess.login_ctx = login_ctx;
	} else if (login_cb) {
		// new login callback gets busy reply as the current is blocked
		login_cb(cl, LOGIN_BUSY, login_ctx);
	}

	// Copy username
	if (user && user != cl->sess.username) {
		free(cl->sess.username);
		if (!(cl->sess.username = strndup(user, 253))) {
			client_logout(cl, LOGOUT_NAS_ERROR);
			return;
		}
	}

	// Copy key
	if (key) {
		free(cl->sess.key);
		if (!(cl->sess.key = malloc(len + 1))) {
			client_logout(cl, LOGOUT_NAS_ERROR);
			return;
		}
		memcpy(cl->sess.key, key, len);
		cl->sess.key[len] = 0;
		cl->sess.keylen = len;
	}

	cl->time_last_touched = event_time();

	if (cl->status == CLSTAT_LOGIN) { // PRELOGIN already done
		client_authenticate(cl);
		return;
	}

	// Set session defaults
	if (!cl->sess.backend)
		cl->sess.backend = backend;
	if (!cl->sess.stats_cb)
		cl->sess.stats_cb = &traffic_stats;
	cl->sess.id = ++session_id;
	cl->sess.interim_interval = cl->sess.limit_time_conn = UINT32_MAX;
	cl->sess.limit_bytes_in = cl->sess.limit_bytes_out =
	cl->sess.limit_bytes_total = UINT64_MAX;

	// Collect IP-addresses for authorization
	cl->sess.addrs_need_update = true;
	client_update_addresses(cl);
	client_authenticate(cl);
}

#ifdef __SC_BUILD__
static void client_add_filter_ip(struct client *cl)
{
    char ipaddr[32] = {0};
    char cmd[128] = {0};
    const char *ip = NULL;
    const char *mask = NULL;
    struct in_addr open_subnet;
    struct in_addr osubnet;
    ip = config_get_string("main", "open_lan_ip", "192.168.182.1");
    mask = config_get_string("main", "open_lan_mask", "255.255.255.0");
   
    open_subnet.s_addr = ((inet_addr(ip))&(inet_addr(mask)));

    if(cl->sess.req_ipaddr)
    {
        for (ssize_t i = 0; i < cl->sess.req_ipaddr_cnt; ++i)
        {
            inet_ntop(AF_INET, &cl->sess.req_ipaddr[i], ipaddr, sizeof(ipaddr));
            osubnet.s_addr = ((inet_addr(ipaddr)) & (inet_addr(mask)));

            if(open_subnet.s_addr == osubnet.s_addr)
            {
                snprintf(cmd,sizeof(cmd),"/usr/sbin/dns_cmset -a %s",ipaddr);
                system(cmd);
            }
            memset(ipaddr,0,sizeof(ipaddr));
        }
    }
    else
    {
        union {
            struct in6_addr in6[8];
            struct in_addr in[8];
        } addr;

        ssize_t cnt = neigh_mac2ip(cl->ifindex, AF_INET, &addr, 8, cl->mac);
        for (ssize_t i = 0; i < cnt; ++i)
        {
            inet_ntop(AF_INET, &addr.in[i], ipaddr, sizeof(ipaddr));
            snprintf(cmd,sizeof(cmd),"/usr/sbin/dns_cmset -a %s",ipaddr);
            system(cmd);
            memset(ipaddr,0,sizeof(ipaddr));
        }
    }
    return;
}
#endif
// Authorize client when all login steps were successful
static void client_update(struct client *cl, enum client_login status,
		const struct client_backend *next) {

	// Already authenticated, address update request
	if (cl->status == CLSTAT_AUTHED) {
		if (status == LOGIN_SUCCESS) {
			client_set_services(cl, true);
			client_commit_addresses(cl);
			return;
		} else if (status != LOGOUT_LOST_SERVICE && status != LOGOUT_SERVICE_UNAVAILABLE) {
				cl->sess.addr_update_in_progress = false;
				return;
		}
	}

	cl->login_status = status;
	if (next) {
		cl->sess.backend(cl, CLIENT_LOGOUT, client_update,
				&cl->sess.backend_ctx);
		cl->sess.backend = next->auth;
	}

	if (status == LOGIN_NOTYET && next) {
		cl->sess.backend(cl, CLIENT_LOGIN, client_update,
				&cl->sess.backend_ctx);
		return;
	} else if (status == LOGIN_CHALLENGE) {
		if (cl->status != CLSTAT_LOGIN || !cl->sess.login_cb) {
			client_logout(cl, LOGOUT_NAS_ERROR);
		} else {
			cl->sess.login_cb(cl, LOGIN_CHALLENGE,
					cl->sess.login_ctx);
			cl->sess.login_cb = NULL;
		}
		return;
	} else if (status != LOGIN_SUCCESS) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "LOGIN not equal to SUCCESS status(%d)", status);
#else
		syslog(LOG_WARNING, "%s LOGIN not equal to SUCCESS status(%d)", __FUNCTION__, status);
#endif
		client_logout(cl, status);
		return;
	}

	if (traffic_create(client_id(cl))) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to setup traffic control "
					"for user %i", client_id(cl));
#else
		syslog(LOG_WARNING, "Failed to setup traffic control "
					"for user %i", client_id(cl));
#endif
		client_logout(cl, LOGOUT_NAS_ERROR);
		return;
	}
	int64_t now = event_time();
	cl->status = CLSTAT_AUTHED;
	cl->sess.time_start = now;
	cl->sess.time_last_interim = now;

	if (cl->sess.backend && !cl->sess.noacct)
		cl->sess.backend(cl, CLIENT_ACCOUNTING_START,
				client_update, &cl->sess.backend_ctx);

	// Mark traffic as authed
#ifdef __SC_BUILD__
    client_add_filter_ip(cl);
#endif
	firewall_set_auth(cl->ifindex, cl->mac, client_id(cl), true);
	client_set_services(cl, true);
	client_commit_addresses(cl);
	syslog(LOG_INFO, "Client %i authorized", client_id(cl));
	if (cl->sess.login_cb) {
		cl->sess.login_cb(cl, LOGIN_SUCCESS, cl->sess.login_ctx);
		cl->sess.login_cb = NULL;
	}

	free(cl->sess.eap);
	cl->sess.eap = NULL;
	cl->sess.eaplen = 0;

	free(cl->sess.state);
	cl->sess.state = NULL;
	cl->sess.statelen = 0;

	free(cl->sess.mppe.recv_key);
	free(cl->sess.mppe.send_key);
	cl->sess.mppe.recv_key = NULL;
	cl->sess.mppe.send_key = NULL;
	cl->sess.mppe.recv_keylen = 0;
	cl->sess.mppe.send_keylen = 0;

	free(cl->sess.userurl);
	cl->sess.userurl = NULL;

	client_count++;
	syslog(LOG_INFO, "Current authenticated users: %d", client_count);
	client_sum++;
	if (tr_newuser) {
		char buf1[16], buf2[16];
		snprintf(buf1, sizeof(buf1), "%u", client_count);
		snprintf(buf2, sizeof(buf2), "%u", cl->sess.id);
		char *argv[] = {buf1, buf2, NULL};
		trigger_run(tr_newuser, argv, NULL, NULL, NULL);
	}
}


// logout or cancel current login operation
// We assume that this cannot fail and is atomic
void client_logout(struct client *cl, enum client_login reason) {
	if (cl->status == CLSTAT_LOGOUT)
		return; // Ensure we don't call ourself

	int oldstat = cl->status;
	cl->status = CLSTAT_LOGOUT;
	cl->login_status = reason;

#ifdef CONFIG_SUPPORT_WEB_PRIVOXY
    char ipaddr[32] = {0};
    char cmd[128] = {0};
#endif
	if (oldstat == CLSTAT_AUTHED) {
#ifdef __SC_BUILD__
        log_fon(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Logout request of client %i", client_id(cl));
#else
		syslog(LOG_INFO, "Logout request of client %i", client_id(cl));
#endif
		// Remove traffic mark
		firewall_set_auth(cl->ifindex, cl->mac, client_id(cl), false);

		// Reset open connections when on open interface
		union {
			struct in6_addr in6[8];
			struct in_addr in[8];
		} addr;

		ssize_t cnt = neigh_mac2ip(cl->ifindex, AF_INET, &addr, 8, cl->mac);
		for (ssize_t i = 0; i < cnt; ++i)
        {
			routing_connkill(AF_INET, &addr.in[i]);
#ifdef __SC_BUILD__
            inet_ntop(AF_INET, &addr.in[i], ipaddr, sizeof(ipaddr));
            snprintf(cmd,sizeof(cmd),"/usr/sbin/dns_cmset -d %s",ipaddr);
            system(cmd);
            memset(ipaddr,0,sizeof(ipaddr));
#endif
        }

		cnt = neigh_mac2ip(cl->ifindex, AF_INET6, &addr, 8, cl->mac);
		for (ssize_t i = 0; i < cnt; ++i)
			routing_connkill(AF_INET6, &addr.in6[i]);
		client_count--;
		if (tr_deluser) {
			char buf1[16], buf2[16];
			snprintf(buf1, sizeof(buf1), "%u", client_count);
			snprintf(buf2, sizeof(buf2), "%u", cl->sess.id);
			char *argv[] = {buf1, buf2, NULL};
			trigger_run(tr_deluser, argv, NULL, NULL, NULL);
		}
		if (cl->sess.backend && !cl->sess.noacct)
			cl->sess.backend(cl, CLIENT_ACCOUNTING_STOP, client_update,
					&cl->sess.backend_ctx);
	}

	// Set default reason
	if (!reason)
		reason = LOGOUT_ADMIN_RESET;

	// Deauth
	if (cl->sess.backend)
		cl->sess.backend(cl, CLIENT_LOGOUT, client_update,
				&cl->sess.backend_ctx);

	// Reset challenge
	urandom(cl->challenge, sizeof(cl->challenge));

	// Inform login callback
	if (cl->sess.login_cb)
		cl->sess.login_cb(cl, reason, cl->sess.login_ctx);

	// Clear traffic control rules
	traffic_destroy(client_id(cl));
	// Clear service access rules
	client_set_services(cl, false);

	// Disconnect user when dying an unnatural death
	if (reason != LOGOUT_DISCONNECT && reason != LOGOUT_USER
			&& reason != LOGOUT_SESSION_LIMIT)
		routing_disconnect(cl->ifindex, cl->mac);
#ifdef __SC_BUILD__
        log_fon(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Client %u (%02x:%02x:%02x:%02x:%02x:%02x) logout",
			client_id(cl), cl->mac[0], cl->mac[1],
			cl->mac[2], cl->mac[3], cl->mac[4], cl->mac[5]);
#else
	// Clear session
	syslog(LOG_INFO, "Client %u (%02x:%02x:%02x:%02x:%02x:%02x) logout",
			client_id(cl), cl->mac[0], cl->mac[1],
			cl->mac[2], cl->mac[3], cl->mac[4], cl->mac[5]);
	syslog(LOG_INFO, "Current authenticated users: %d", client_count);
#endif

	free(cl->sess.class);
	free(cl->sess.cui);
	free(cl->sess.username);
	free(cl->sess.key);
	free(cl->sess.state);
	free(cl->sess.mppe.recv_key);
	free(cl->sess.mppe.send_key);
	free(cl->sess.userurl);
	free(cl->sess.logon_unknown_params);
	free(cl->sess.eap);
	free(cl->sess.reply);
	free(cl->sess.req_ipaddr);
	free(cl->sess.req_ip6addr);
	free(cl->sess.auth_conf);
	free(cl->sess.acct_conf);
	memset(&cl->sess, 0, sizeof(cl->sess));
	cl->status = CLSTAT_NOAUTH;
	cl->login_status = LOGIN_PENDING;
}

static void client_remove(struct client *cl, enum client_login reason) {
	client_logout(cl, reason);
	cl->status = CLSTAT_NOALLOC;
	if (client_id(cl) == clients_cmax)
		clients_cmax--;

#ifdef __SC_BUILD__
    log_fon(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Client %u (%02x:%02x:%02x:%02x:%02x:%02x) removed",
				client_id(cl), cl->mac[0], cl->mac[1],
				cl->mac[2], cl->mac[3], cl->mac[4], cl->mac[5]);
#else
	syslog(LOG_INFO, "Client %u (%02x:%02x:%02x:%02x:%02x:%02x) removed",
				client_id(cl), cl->mac[0], cl->mac[1],
				cl->mac[2], cl->mac[3], cl->mac[4], cl->mac[5]);
#endif
}
#ifdef __SC_BUILD__
int client_add_current_authercated_users_to_dnrd(void)
{
	for (int i = 0; i < clients_max; ++i) {
		if ((clients[i].status == CLSTAT_AUTHED)) {
            client_add_filter_ip(&clients[i]);
		}
	}
    return 0;
}
unsigned client_get_current_authercated_users(struct client *cl)
{
    int users =0;
	if (!cl)
		return 0;

	for (int i = 0; i < clients_max; ++i) {
		if ((clients[i].status == CLSTAT_AUTHED) && cl != &clients[i]) {
            users ++;
		}
	}
    return users;
}

unsigned client_is_reach_max_login_user(struct client *cl)
{
    int ret = 0;
    if(client_get_current_authercated_users(cl) >= MaxLoginUser)
        ret = 1;
    return ret;
}
#endif
// Get a user handle with given address or create a new
// one if none currently exists
struct client* client_register(int ifindex, int af, const void *addr) {
	struct client *cl = client_get(ifindex, af, addr);
	if (cl)
		return cl;

	for (int i = 0; i < clients_max; ++i) {
		if (!clients[i].status) {
			cl = &clients[i];
			cl->id = i + 1; //Clients ids starts at 1
			break;
		}
	}

	if (!cl) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Ignoring new client (limit reached)");
#else
		syslog(LOG_WARNING, "Ignoring new client (limit reached)");
#endif
		return NULL;
	}

	uint8_t hwaddr[6] = {0};
	// Resolve IP/MAC addresses
	if (af == AF_PACKET)
		memcpy(hwaddr, addr, sizeof(hwaddr));
	else if (neigh_ip2mac(ifindex, hwaddr, af, addr))
		return NULL; // Couldn't resolve MAC

	// Test for older client entries and drop them
	struct client *oldcl;
	while ((oldcl = client_get(ifindex, AF_PACKET, hwaddr)))
		client_logout(oldcl, LOGOUT_DISCONNECT);

	if (clients_cmax < client_id(cl))
		clients_cmax = client_id(cl);

	cl->status = CLSTAT_NOAUTH;
	cl->ifindex = ifindex;
	cl->time_last_touched = event_time();
	cl->limit_time_idle = cfg_idletime;
	cl->servicemap = 0;
	cl->serviced = false;
	urandom(cl->challenge, sizeof(cl->challenge));
	memcpy(cl->mac, hwaddr, sizeof(cl->mac));
	syslog(LOG_INFO, "New client %u (%02x:%02x:%02x:%02x:%02x:%02x)",
				client_id(cl), hwaddr[0], hwaddr[1],
				hwaddr[2],hwaddr[3], hwaddr[4], hwaddr[5]);
	return cl;
}

// Register a new auth handler
void client_backend(struct client_backend *ba) {
	list_add(&ba->_head, &backends);
}

// Register the service controller
void client_services(struct client_serv *serv) {
	if(serv) {
		client_get_servicemap = serv->get_servicemap;
		_client_set_services = serv->set_services;
		_client_get_cap = serv->get_cap;
	}
}

void client_get_services(struct client *cl, const char *type)
{
	if(cl) {
		if(type && client_get_servicemap) {
			cl->servicemap = client_get_servicemap(type);
		} else {
			cl->servicemap = 0;
		}
	}
}

void client_set_services(struct client *cl, bool open)
{
	if(!cl) {
		return;
	}

	if((!open && cl->serviced) || (open && !cl->serviced)) {
		if(_client_set_services) {
			_client_set_services(cl->id, cl->servicemap, open);
		}
		cl->serviced = open;
	}
}

char *client_get_cap(const struct client *cl)
{
	if(_client_get_cap) {
		if(!cl) {
			return _client_get_cap(true, 0);
		}
		return _client_get_cap(false, cl->servicemap);
	}
	return NULL;
}

// Timer for client watching / accounting
static void client_timer(struct event_timer *timer, int64_t now) {
	for (int i = 0; i < clients_cmax; ++i) {
		if (!clients[i].status) // Don't service clients with empty slot
			continue;

		if ((now - clients[i].time_last_touched) / 1000
		>= clients[i].limit_time_idle)
			client_remove(&clients[i], LOGOUT_IDLE);
		if (clients[i].status != CLSTAT_AUTHED)
			continue;

		uint64_t byte_in, byte_out;
		uint32_t pkt_in, pkt_out;
		if (clients[i].sess.stats_cb(client_id(&clients[i]),
				&byte_in, &byte_out, &pkt_in, &pkt_out)) {
#ifdef __SC_BUILD__
            log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unable to get stats for user %i",
						client_id(&clients[i]));
#else
			syslog(LOG_WARNING, "Unable to get stats for user %i",
						client_id(&clients[i]));
#endif
			client_logout(&clients[i], LOGOUT_NAS_ERROR);
			continue;
		} else {
			syslog(LOG_DEBUG, "Traffic for user %i: "
				"%llu B in %llu B out", client_id(&clients[i]),
				(unsigned long long)byte_in,
				(unsigned long long)byte_out);
		}

		if (clients[i].sess.pkts != pkt_in + pkt_out) {
			clients[i].sess.pkts = pkt_in + pkt_out;
			clients[i].time_last_touched = now;
		}

		uint32_t stime = (now - clients[i].sess.time_start) / 1000;
		if (stime >= clients[i].sess.limit_time_conn
		|| clients[i].sess.limit_bytes_in < byte_in
		|| clients[i].sess.limit_bytes_out < byte_out
		|| clients[i].sess.limit_bytes_total < byte_in + byte_out) {
			client_logout(&clients[i], LOGOUT_SESSION_LIMIT);
			continue;
		}

		if (clients[i].sess.backend && !clients[i].sess.noacct
		&& (now - clients[i].sess.time_last_interim) / 1000
		>= clients[i].sess.interim_interval) {
			clients[i].sess.backend(&clients[i],
					CLIENT_ACCOUNTING_INTERIM, client_update,
					&clients[i].sess.backend_ctx);
			clients[i].sess.time_last_interim = now;
		}
	}
}

const struct client_backend* client_get_backend(const char *name) {
	struct client_backend *be;
	list_for_each_entry(be, &backends, _head) {
		if (name && !strcmp(name, be->name))
			return be;
	}
	return NULL;
}

MODULE_REGISTER(client, 400)

#ifdef HOTSPOTD_RPC
#include <errno.h>
#include "ext_rpc/rpc.h"
#include "rpc/014-client.h"

static int client_rpc_set(struct rpc_handle *hndl, struct frmsg *frr) {
	if (!clients)
		return -ENOTSUP;

	struct frattr *attrs[_FRA_CL_SIZE];
	frm_parse(frr, attrs, _FRA_CL_SIZE);
	struct client *cl = NULL;

	uint32_t ifindex = fra_to_u32(attrs[FRA_CL_IFINDEX],
			routing_cfg.iface_index);

	// Get client using key
	if (fra_length(attrs[FRA_CL_HWADDR]) >= 6) {
		cl = client_get(routing_cfg.iface_index, AF_PACKET,
				fra_data(attrs[FRA_CL_HWADDR]));
	} else if (fra_length(attrs[FRA_CL_SESSID]) >= 4) {
		uint32_t sessid = fra_to_u32(attrs[FRA_CL_SESSID], 0);
		cl = client_get(0, AF_LOCAL, &sessid);
	} else if (fra_length(attrs[FRA_CL_IPADDR]) == 4) {
		cl = client_get(ifindex, AF_INET,
				fra_data(attrs[FRA_CL_IPADDR]));
	} else if (fra_length(attrs[FRA_CL_IPADDR]) == 16) {
		cl = client_get(ifindex, AF_INET6,
				fra_data(attrs[FRA_CL_IPADDR]));
	}

	// Basic sanity checks
	if (!cl) {
		return -ENOENT;
	} else if (frm_type(frr) == FRT_CL_DEL) { // Logout request
		client_logout(cl, fra_to_u32(attrs[FRA_CL_REASON],
							LOGOUT_ADMIN_RESET));
		return 0;
	} else if (frm_type(frr) == FRT_CL_LOGIN
	&& !(cl->status == CLSTAT_NOAUTH || cl->status == CLSTAT_LOGIN)) {
		return -EBUSY;
	}

	// Set session options
	uint32_t u32;

	if ((u32 = fra_to_u32(attrs[FRA_CL_TIME_MAX], 0)))
		cl->sess.limit_time_conn = u32;

	if ((u32 = fra_to_u32(attrs[FRA_CL_IDLE_MAX], 0)))
		cl->limit_time_idle = u32;

	if ((u32 = fra_to_u32(attrs[FRA_CL_INTERIM], 0)))
		cl->sess.interim_interval = u32;

	if ((u32 = fra_to_u32(attrs[FRA_CL_BIN_MAX], 0)))
		cl->sess.limit_bytes_in = (uint64_t) u32;

	if ((u32 = fra_to_u32(attrs[FRA_CL_BOUT_MAX], 0)))
		cl->sess.limit_bytes_out = (uint64_t) u32;

	if ((u32 = fra_to_u32(attrs[FRA_CL_BOTH_MAX], 0)))
		cl->sess.limit_bytes_total = (uint64_t) u32;

	if (attrs[FRA_CL_NOACCT])
		cl->sess.noacct = fra_to_u32(attrs[FRA_CL_NOACCT], 0);

	if (frm_type(frr) == FRT_CL_SET) // Skip login part
		return 0;

	//Login
	uint32_t method = fra_to_u32(attrs[FRA_CL_METHOD], 0);
	const char *user = fra_to_string(attrs[FRA_CL_USERNAME]);
	const void *key = fra_data(attrs[FRA_CL_KEY]);
	size_t keylen = fra_length(attrs[FRA_CL_KEY]);

	const struct client_backend *backend = client_get_backend(
			fra_to_string(attrs[FRA_CL_BACKEND]));


	if (cl->status == CLSTAT_NOAUTH) {
		if (backend)
			cl->sess.backend = backend->auth;
		client_login(cl, method, user, key, keylen, NULL, NULL);
	} else {
		if (user) {
			free(cl->sess.username);
			cl->sess.username = strdup(user);
		}

		if (key) {
			cl->sess.keylen = 0;
			free(cl->sess.key);
			if ((cl->sess.key = malloc(keylen))) {
				memcpy(cl->sess.key, key, keylen);
				cl->sess.keylen = keylen;
			}
		}

		if (method)
			cl->sess.method = method;

		client_update(cl, LOGIN_NOTYET, backend);
	}
	return 0;
}

static int client_rpc_get(struct rpc_handle *hndl, struct frmsg *frr) {
	if (!clients)
		return -ENOTSUP;

	struct frattr *attrs[_FRA_CL_SIZE];
	frm_parse(frr, attrs, _FRA_CL_SIZE);
	struct client *clsel = NULL;
	int64_t now = event_time();

	uint32_t ifindex = fra_to_u32(attrs[FRA_CL_IFINDEX],
			routing_cfg.iface_index);

	// Get client using key
	if (fra_length(attrs[FRA_CL_HWADDR]) >= 6) {
		clsel = client_get(routing_cfg.iface_index, AF_PACKET,
				fra_data(attrs[FRA_CL_HWADDR]));
	} else if (fra_length(attrs[FRA_CL_SESSID]) >= 4) {
		uint32_t sessid = fra_to_u32(attrs[FRA_CL_SESSID], 0);
		clsel = client_get(0, AF_LOCAL, &sessid);
	} else if (fra_length(attrs[FRA_CL_IPADDR]) == 4) {
		clsel = client_get(ifindex, AF_INET,
				fra_data(attrs[FRA_CL_IPADDR]));
	} else if (fra_length(attrs[FRA_CL_IPADDR]) == 16) {
		clsel = client_get(ifindex, AF_INET6,
				fra_data(attrs[FRA_CL_IPADDR]));
	}

	if (!clsel && !(frm_flags(frr) & FRM_F_DUMP))
		return -ENOENT;

	struct frmsg *frm = NULL;
	uint8_t buffer[8192];
	size_t len = 0;

	for (int i = 0; i < clients_cmax; ++i) {
		struct client *cl = &clients[i];
		if (!clients[i].status || (clsel && cl != clsel))
			continue;

		// Build message for 1 entry
		frm = frm_init(buffer + len, FRT_CL_GET, FRM_F_MULTI);
		frm->frm_seq = frr->frm_seq;

		uint64_t byte_in, byte_out;
		uint32_t pi, po;
        cl->sess.stats_cb(client_id(cl), &byte_in, &byte_out, &pi, &po);

		// Dump one client
		frm_put_buffer(frm, FRA_CL_HWADDR, cl->mac, 6);
		frm_put_u32(frm, FRA_CL_SESSID, cl->sess.id);
		if (cl->sess.username)
			frm_put_string(frm, FRA_CL_USERNAME, cl->sess.username);
		frm_put_u32(frm, FRA_CL_STATUS, cl->status);
		frm_put_u32(frm, FRA_CL_TIME, (now - cl->sess.time_start) / 1000);
		frm_put_u32(frm, FRA_CL_IDLE, (now - cl->time_last_touched) / 1000);
		frm_put_u64(frm, FRA_CL_BIN, byte_in);
		frm_put_u64(frm, FRA_CL_BOUT, byte_out);
		frm_put_u32(frm, FRA_CL_TIME_MAX, cl->sess.limit_time_conn);
		frm_put_u32(frm, FRA_CL_IDLE_MAX, cl->limit_time_idle);
		frm_put_u32(frm, FRA_CL_INTERIM, cl->sess.interim_interval);
		frm_put_u32(frm, FRA_CL_BIN_MAX, cl->sess.limit_bytes_in);
		frm_put_u32(frm, FRA_CL_BOUT_MAX, cl->sess.limit_bytes_out);
		frm_put_u32(frm, FRA_CL_BOUT_MAX, cl->sess.limit_bytes_total);

		len += frm_align(frm_length(frm));
		if (sizeof(buffer) - len < 512) {
			rpc_send(hndl, buffer, len);
			len = 0;
		}
	}

	if (frm_flags(frr) & FRM_F_DUMP) {
		frm = frm_init(buffer + len, FRMSG_DONE, 0);
		frm->frm_seq = frr->frm_seq;
		rpc_send(hndl, buffer, len + frm_length(frm));
	} else if (frm) {
		frm->frm_flags = 0;
		rpc_send(hndl, frm, frm_length(frm));
	}
	return 1;
}

#ifdef __SC_BUILD__
static int client_rpc_get_info(struct rpc_handle *hndl, struct frmsg *frr) {
	uint8_t buffer[8192];
	struct frmsg *frm = frm_init(buffer, FRT_CL_GET_INFO, 0);
	frm_put_u32(frm, FRA_CL_AUTH_COUNT, client_count);
	frm_put_u32(frm, FRA_CL_TOTAL_COUNT, clients_cmax);
	frm_put_u32(frm, FRA_CL_AUTH_SUM, client_sum);
	frm->frm_seq = frr->frm_seq;
	rpc_send(hndl, frm, frm_length(frm));
	return 1;
}
static int client_rpc_add_auth_to_dnrd(struct rpc_handle *hndl, struct frmsg *frr) {
    client_add_current_authercated_users_to_dnrd();
	return 1;
}
#endif

static struct rpc_handler rpcs[] = {
		{FRT_CL_SET, client_rpc_set},
		{FRT_CL_DEL, client_rpc_set},
		{FRT_CL_LOGIN, client_rpc_set},
		{FRT_CL_GET, client_rpc_get},
#ifdef __SC_BUILD__
		{FRT_CL_GET_INFO, client_rpc_get_info},
		{FRT_CL_ADD_AUTH_TO_DNRD, client_rpc_add_auth_to_dnrd},
#endif
		{0}
};

RPC_REGISTER(rpcs)
#endif
