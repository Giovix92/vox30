/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <libiptc/libiptc.h>

#include "lib/usock.h"
#include "lib/event.h"
#include "lib/config.h"
#include "lib/hexlify.h"
#include "lib/ipt.h"

#include "core/client.h"
#include "core/firewall.h"
#include "core/hotspotd.h"
#include "core/neigh.h"
#include "core/routing.h"

#define PRIV_CLIENT_ID_OFFSET 0x800
#define PRIV_CLIENT_LIMIT (0xFF0 - PRIV_CLIENT_ID_OFFSET)// First reserved user_id minus PRIV_CLIENT_ID_OFFSET
#define IPT_MANGLE(...) { if (ipt_mangle(__VA_ARGS__)) return -1; }
#define IP6T_MANGLE(...) { if (ip6t_mangle(__VA_ARGS__)) return -1; }

#define FIREWALL_CHAIN_PRIV_NAME "hotspot_private"
#define PRIV_CHAIN_NAME_MAX_LENGTH 32

#define USERNAME_PREFIX "MyPlace/"

struct sockaddr_in6 srcaddr;
socklen_t srcalen;

static bool enabled;
static bool wasenabled;
static int cfg_idletime = 0;
static unsigned priv_nclients = 0;
static struct client *privclients = NULL;

static const char *iface;
static bool fw_ipv4 = false, fw_ipv6 = false;
static char chain_priv_prert[PRIV_CHAIN_NAME_MAX_LENGTH];
static char chain_priv_postrt[PRIV_CHAIN_NAME_MAX_LENGTH];

static client_auth_cb *privbackend = NULL;
static int privacct_interim_interval = 0;
static const char *privacct_conf = NULL;

static void private_client_update(struct client *cl, enum client_login status, const struct client_backend *next);
static void private_client_timer(struct event_timer *timer, int64_t now);
static int private_enable();
static void private_disable();
static void private_handler(uint8_t mac[6], char ip[INET6_ADDRSTRLEN], enum neigh_event nevent);
static struct neigh_handler priv_handler = {
	.cb = private_handler,
};

static struct event_timer privtimer_acct = {
	.handler = private_client_timer,
};

/* Mark client to later retrieve accounting stats 
 * Equivalent to:
 * iptables -t mangle -N hotspot_private_prert_<clientID>
 * iptables -t mangle -N hotspot_private_post_<clientID>
 * iptables -t mangle -A hotspot_private_prert -m mac --mac-source <MAC> -j hotspot_private_prert_<clientID>
 * iptables -t mangle -A hotspot_private_postrt -m connmark --mark <CLIENT_MARK> -j hotspot_private_postrt_<clientID>
 * iptables -t mangle -A hotspot_private_prert_<clientID> -j CONNMARK --set-mark <CLIENT_MARK>
 * iptables -t mangle -A hotspot_private_postrt_<clientID> -j ACCEPT
 * or clear rules if set==false
**/
int private_client_set_mark(struct client *cl, bool set) {
	char macbuf[24];
	snprintf(macbuf, sizeof(macbuf), "%02x:%02x:%02x:%02x:%02x:%02x",
		cl->mac[0], cl->mac[1], cl->mac[2], cl->mac[3], cl->mac[4], cl->mac[5]);

	char chain_client_prert[PRIV_CHAIN_NAME_MAX_LENGTH];
	char chain_client_postrt[PRIV_CHAIN_NAME_MAX_LENGTH];

	snprintf(chain_client_prert, sizeof(chain_client_prert), "%s_%u", chain_priv_prert, client_id(cl));
	snprintf(chain_client_postrt, sizeof(chain_client_postrt), "%s_%u", chain_priv_postrt, client_id(cl));

	if (set) {
		if (fw_ipv4) {
			IPT_MANGLE(IPT_NEWCHAIN, chain_client_prert);
			IPT_MANGLE(IPT_NEWCHAIN, chain_client_postrt);
		} else if (fw_ipv6) {
			IP6T_MANGLE(IPT_NEWCHAIN, chain_client_prert);
			IP6T_MANGLE(IPT_NEWCHAIN, chain_client_postrt);
		}
	}

	char markbuf[32];
	snprintf(markbuf, sizeof(markbuf), "%u/%u",
		 firewall_id_to_authmark(client_id(cl) | PRIV_CLIENT_ID_OFFSET), FIREWALL_AUTHMASK); 

	int action = (set) ? IPT_APPEND : IPT_DELETE;

	if (fw_ipv4){
		IPT_MANGLE(action, chain_priv_prert, "-m", "mac", "--mac-source", macbuf, "-j", chain_client_prert);
		IPT_MANGLE(action, chain_priv_postrt, "-m", "connmark", "--mark", markbuf, "-j", chain_client_postrt);
	} else if (fw_ipv6){
		IP6T_MANGLE(action, chain_priv_prert, "-m", "mac", "--mac-source", macbuf, "-j", chain_client_prert);
		IP6T_MANGLE(action, chain_priv_postrt, "-m", "connmark", "--mark", markbuf, "-j", chain_client_postrt);
	}

	if (fw_ipv4) {
		IPT_MANGLE(action, chain_client_prert, "-j", "CONNMARK", "--set-mark", markbuf);
		IPT_MANGLE(action, chain_client_postrt, "-j", "ACCEPT");
	} else if (fw_ipv6) {
		IP6T_MANGLE(action, chain_client_prert, "-j", "CONNMARK", "--set-mark", markbuf);
		IP6T_MANGLE(action, chain_client_postrt, "-j", "ACCEPT");
	}

	if (!set) {
		if (fw_ipv4) {
			IPT_MANGLE(IPT_DELETECHAIN, chain_client_prert);
			IPT_MANGLE(IPT_DELETECHAIN, chain_client_postrt);
		}
		if (fw_ipv6) {
			IP6T_MANGLE(IPT_DELETECHAIN, chain_client_prert);
			IP6T_MANGLE(IPT_DELETECHAIN, chain_client_postrt);
		}
	}

	if (set) 
    {
#ifdef __SC_BUILD__
        log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Private client with mac addr %s marked for accounting", macbuf);
#else
		syslog(LOG_DEBUG, "Private client with mac addr %s marked for accounting", macbuf);
#endif
    }
	return 0;
}

// Remove from private client list and clear mark
static void private_client_unregister(struct client *cl, enum client_login reason) {
	client_renew_addresses(cl);
	if ((cl->sess.req_ipaddr_cnt > 0 || cl->sess.req_ip6addr_cnt > 0)
	    && reason != LOGOUT_ADMIN_REBOOT)
		return;		//only unregister if it had only one entry registered

	int oldstat = cl->status;

	if (client_id(cl) == priv_nclients)
		priv_nclients--;

	cl->status = CLSTAT_LOGOUT;
	cl->login_status = reason;
	if (oldstat == CLSTAT_AUTHED) {
		if (cl->sess.backend && !cl->sess.noacct) {
			cl->sess.backend(cl, CLIENT_ACCOUNTING_STOP, NULL,
				&cl->sess.backend_ctx);
		}
		private_client_set_mark(cl, false);
	}

	free(cl->sess.acct_conf);
	cl->sess.acct_conf = NULL;
	free(cl->sess.username);
	cl->sess.username = NULL;

	cl->status = CLSTAT_NOALLOC;
#ifdef __SC_BUILD__
    log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Private client %u (%02x:%02x:%02x:%02x:%02x:%02x) unregistered",
		client_id(cl), cl->mac[0], cl->mac[1],
		cl->mac[2], cl->mac[3], cl->mac[4], cl->mac[5]);
#else
	syslog(LOG_INFO, "Private client %u (%02x:%02x:%02x:%02x:%02x:%02x) unregistered",
		client_id(cl), cl->mac[0], cl->mac[1],
		cl->mac[2], cl->mac[3], cl->mac[4], cl->mac[5]);
#endif
}

static void private_client_update(struct client *cl, enum client_login status,
	const struct client_backend *next) {
	cl->login_status = LOGIN_SUCCESS;
}

/**
 * Get counters for given private client 
 *
 */
static int private_stats(uint16_t id,
uint64_t *byte_in, uint64_t *byte_out, uint32_t *pkt_in, uint32_t *pkt_out) {
	// get byte and packet count for client 
	const struct ipt_entry *rule_prert = NULL;
	const struct ipt_entry *rule_postrt = NULL;
	int err = 0;
	static struct iptc_handle *iptc_h = NULL;
	const char table[] = "mangle";

	char chain_client_prert[PRIV_CHAIN_NAME_MAX_LENGTH];
	char chain_client_postrt[PRIV_CHAIN_NAME_MAX_LENGTH];
#ifdef IPTABLES_PRE_1_4_3
	static struct iptc_handle **iptc_handler = NULL;
#else
	static struct iptc_handle *iptc_handler = NULL;
#endif

	iptc_h = iptc_init(table);
	if (iptc_h) {
#ifdef IPTABLES_PRE_1_4_3
		iptc_handler = &iptc_h;
#else
		iptc_handler = iptc_h;
#endif
		snprintf(chain_client_prert, sizeof(chain_client_prert), "%s_%u", chain_priv_prert, id);
		rule_prert = iptc_first_rule(chain_client_prert, iptc_handler);

		snprintf(chain_client_postrt, sizeof(chain_client_postrt), "%s_%u", chain_priv_postrt, id);
		rule_postrt = iptc_first_rule(chain_client_postrt, iptc_handler);

		if (rule_prert && rule_postrt) {
			*pkt_out = (uint32_t)rule_prert->counters.pcnt; /* Warning: it may overflow */
			*byte_out = rule_prert->counters.bcnt;
			*pkt_in = (uint32_t)rule_postrt->counters.pcnt; /* Warning: it may overflow */
			*byte_in = rule_postrt->counters.bcnt;
		} else {
			err = 2;
#ifdef __SC_BUILD__
            log_wifi(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "%s: couldn't get rules for private user %i", __FUNCTION__, id);
#else
			syslog(LOG_ERR, "%s: couldn't get rules for private user %i", __FUNCTION__, id);
#endif
		}
		iptc_free(iptc_handler);
	} else {
#ifdef __SC_BUILD__
        log_wifi(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "%s: couldn't get iptc handler for table %s",__FUNCTION__, table);
#else
		syslog(LOG_ERR, "%s: couldn't get iptc handler for table %s",__FUNCTION__, table);
#endif
		err = 1;
	}
	
	return err;
}

static void private_client_timer(struct event_timer *timer, int64_t now) {
	for (int i = 0; i < priv_nclients; ++i) {
		if (!privclients[i].status) 
			continue;
	
		if (privclients[i].status != CLSTAT_AUTHED)
			continue;	

		// Accounting
		uint64_t byte_in = 0, byte_out = 0;
		uint32_t pkt_in = 0, pkt_out = 0;
		if (privclients[i].sess.stats_cb(client_id(&privclients[i]),
			&byte_in, &byte_out, &pkt_in, &pkt_out)) {
#ifdef __SC_BUILD__
            log_wifi(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unable to get stats for private user %i",
					client_id(&privclients[i]));
#else
			syslog(LOG_WARNING, "Unable to get stats for private user %i",
					client_id(&privclients[i]));	
#endif
			private_client_unregister(&privclients[i], LOGOUT_NAS_ERROR);
			continue;
		} else {
#ifndef __SC_BUILD__
			syslog(LOG_DEBUG, "Traffic for private user %i: "
				"%llu B in %llu B out", client_id(&privclients[i]),
				(unsigned long long)byte_in,
				(unsigned long long)byte_out);
#endif
		}
		if (privclients[i].sess.pkts != pkt_in + pkt_out) {
			privclients[i].sess.pkts = pkt_in + pkt_out;
			privclients[i].time_last_touched = now;
		}

		if (privclients[i].sess.backend && !privclients[i].sess.noacct
		&& (now - privclients[i].sess.time_last_interim) / 1000
		>= privclients[i].sess.interim_interval) {
			privclients[i].sess.backend(&privclients[i],
				CLIENT_ACCOUNTING_INTERIM, NULL,
					&privclients[i].sess.backend_ctx);
				privclients[i].sess.time_last_interim = now;
		}
	}
}

static int private_fw(bool set) {
	fw_ipv4 = config_get_int("main", "ipv4", 0);
	if (!fw_ipv4)
		fw_ipv4 = !!config_get_string("main", "addr_ipv4", "")[0];
	
#ifdef HOTSPOTD_IPV6
	fw_ipv6 = config_get_int("main", "ipv6", 0);
	if (!fw_ipv6)
		fw_ipv6 = !!config_get_string("main", "addr_ipv6", "")[0];
#endif
	
	iface = config_get_string("main", "iface_priv", "ra0");

	if (set) {
		if (fw_ipv4) {
			IPT_MANGLE(IPT_NEWCHAIN, chain_priv_prert);
			IPT_MANGLE(IPT_NEWCHAIN, chain_priv_postrt);
		} else if (fw_ipv6) {
			IP6T_MANGLE(IPT_NEWCHAIN, chain_priv_prert);
			IP6T_MANGLE(IPT_NEWCHAIN, chain_priv_postrt);
		}
	}
	
	int action = (set) ? IPT_APPEND : IPT_DELETE;
	
	if (fw_ipv4){
		IPT_MANGLE(action, "PREROUTING", "-i", iface, "-j", chain_priv_prert);
		IPT_MANGLE(action, "POSTROUTING", "-o", iface, "-j", chain_priv_postrt);
	} else if (fw_ipv6){
		IP6T_MANGLE(action, "PREROUTING", "-i", iface, "-j", chain_priv_prert);
		IP6T_MANGLE(action, "POSTROUTING", "-o", iface, "-j", chain_priv_postrt);
	}

	if (!set) {
		if (fw_ipv4) {
			IPT_MANGLE(IPT_DELETECHAIN, chain_priv_prert);
			IPT_MANGLE(IPT_DELETECHAIN, chain_priv_postrt);
		} else if (fw_ipv6) {
			IP6T_MANGLE(IPT_DELETECHAIN, chain_priv_prert);
			IP6T_MANGLE(IPT_DELETECHAIN, chain_priv_postrt);
		}
	}
	return 0;
}

static int private_apply()
{
	wasenabled = enabled;

	if(!strcmp(config_get_string("private", "enabled", "false"), "true")) {
		if(private_enable())
			return -1;
		
		// idle time to be replaced by neigh cache mechanism:
		cfg_idletime = config_get_int("private", "idle_max", 600);

		// Private client backend for accounting
		const char *privacct = config_get_string("private", "acct", "radius");
		const struct client_backend *be = client_get_backend(privacct);
		privbackend = (be) ? be->auth : NULL;
		privacct_conf = config_get_string("private", "acct_conf", "privacct");
		privacct_interim_interval = config_get_int(privacct_conf, "interval", 0);
		if(!hotspot_assertconf_int("privacct.interval",
				&(privacct_interim_interval))) {
			return -1;
		}

		if (privacct && !privbackend) {

#ifdef __SC_BUILD__
            log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Could not resolve private client backend.");
#else
			syslog(LOG_WARNING, "Could not resolve private client backend.");
#endif
			return -1;
		}
		enabled = true;
	} else {	
		private_disable();	
		enabled = false;
	}
	return 0;
}

static int private_init()
{
	enabled = false;
	if (!(privclients = calloc(PRIV_CLIENT_LIMIT, sizeof(struct client))))
		return -1;

	priv_nclients = 0;

	// Prepare firewall
	snprintf(chain_priv_prert, sizeof(chain_priv_prert), "%s_prert", FIREWALL_CHAIN_PRIV_NAME);
	snprintf(chain_priv_postrt, sizeof(chain_priv_postrt), "%s_postrt", FIREWALL_CHAIN_PRIV_NAME);
	priv_handler.ifindex = routing_cfg.iface_priv_index;
	neigh_handler(&priv_handler);
	return private_apply();
}

static int private_enable() {
	if(wasenabled)
		return 0;
	privtimer_acct.interval = config_get_int("private", "timer", 15) * 1000;
	event_ctl(EVENT_TIMER_ADD, &privtimer_acct);

	private_fw(true);

	wasenabled = true;
	return 0;
}

static void private_disable() {
	if (!wasenabled)
		return;
	// Unregister all private clients and free list
	for (int i = 0; i < priv_nclients; ++i) {
		if (privclients[i].status)
			private_client_unregister(&privclients[i], LOGOUT_ADMIN_REBOOT);
	}
	
	private_fw(false);
	event_ctl(EVENT_TIMER_DEL, &privtimer_acct);
	wasenabled = false;
}

static void private_deinit()
{
	private_disable();
	neigh_deregister_handler(&priv_handler);
	free(privclients);
	privclients = NULL;
	
}

static struct client* private_client_get(const void *addr) {

	for (int i = 0; i < priv_nclients; ++i) {
		struct client *c = &privclients[i];
		if (c->status && !memcmp(c->mac, addr, sizeof(c->mac))) {
			return c;
		}
	}
	return NULL;
}

static int private_client_mark(struct client *cl) {
	cl->status = CLSTAT_AUTHED;
	cl->sess.authed = true;
	int64_t now = event_time();
	cl->sess.time_start = now;
	cl->sess.time_last_interim = now;
	cl->sess.limit_time_conn = UINT32_MAX;
	cl->sess.limit_bytes_in = cl->sess.limit_bytes_out =
	cl->sess.limit_bytes_total = UINT64_MAX;

	private_client_set_mark(cl, true);
#ifndef __SC_BUILD__
	syslog(LOG_DEBUG, "Private client %i marked for accounting", client_id(cl));
#endif
	if(cl->sess.backend) {
		// pseudo login first to obtain radius context
		cl->sess.backend(cl, CLIENT_LOGIN, private_client_update,
			&cl->sess.backend_ctx);

		if (!cl->sess.noacct) {
			cl->sess.backend(cl, CLIENT_ACCOUNTING_START,
				NULL, &cl->sess.backend_ctx);
		}
	}
	return 0;
}

static int private_client_register(const void *addr) {
	struct client *cl = private_client_get(addr);
	char buf[256];
	if (cl)
	{
                /*
                 * Use the IP counter to know if a MAC has been requested 
                 * to be registered multiple times, then substract this number 
                 * deletion/unregister.
                 */
		client_renew_addresses(cl);
		return 0;
	}

	if(!priv_nclients) {
		priv_nclients = 1;
	}
	
	// Find next client free in privclients array
	for (int i = 0; i <= priv_nclients; ++i) {
		if (!privclients[i].status) {
			cl = &privclients[i];
			cl->id = i + 1; //Clients ids starts at 1
			break;
		}
	}

	if (!cl) {
		// We should never get here (very high limit)
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Ignoring new private client (limit reached)");
#else
		syslog(LOG_WARNING, "Ignoring new private client (limit reached)");
#endif
		return -1;
	}

	cl->ifindex = routing_cfg.iface_priv_index;
	cl->sess.id = cl->id;
	memcpy(cl->mac, addr, sizeof(cl->mac));
	cl->limit_time_idle = cfg_idletime;
	cl->sess.backend_ctx = NULL;

	if (priv_nclients < client_id(cl)) {
		priv_nclients = client_id(cl);
	}

	cl->time_last_touched = event_time();
	client_renew_addresses(cl);

	if (!cl->sess.backend) {
		cl->sess.backend = privbackend;
	}

	cl->sess.acct_conf = strndup(privacct_conf, strlen(privacct_conf));

	if (!cl->sess.stats_cb) {
		cl->sess.stats_cb = &private_stats;
	}

	if (!cl->sess.interim_interval) {
		cl->sess.interim_interval = privacct_interim_interval;
	}

	heXlify(buf, cl->mac, sizeof(cl->mac), '-');
	if ((cl->sess.username = malloc(strlen(USERNAME_PREFIX)+strlen(buf)+1)))
		sprintf(cl->sess.username,"%s%s",USERNAME_PREFIX, buf);

	private_client_mark(cl);
	return 0;
}

static void private_handler(uint8_t mac[6], char ip[INET6_ADDRSTRLEN], enum neigh_event nevent)
{
	if(!enabled)
		return;

	struct client *cl = private_client_get(mac);	
	switch (nevent) { 
		case NEIGH_ADD:
			private_client_register(mac);
			break;

		case NEIGH_DEL:
			private_client_unregister(cl, LOGOUT_IDLE);
			break;

		default:
#ifdef __SC_BUILD__
            log_wifi(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "neigh event not recognized");
#else
			syslog(LOG_WARNING, "neigh event not recognized");
#endif
	}

}

RESIDENT_REGISTER(private, 500)
