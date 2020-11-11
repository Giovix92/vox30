/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "lib/event.h"
#include "lib/config.h"
#include "lib/radius.h"
#include "lib/hexlify.h"

#include "core/hotspotd.h"
#include "core/client.h"
#ifndef __SC_BUILD__
#include "core/routing.h"
#endif
#define MAX_NAME_LENGTH 16

struct radius_ctx {
	struct client *client;
	client_update_cb *update;
	int handle;
};

struct radius_cluster_entry {
	struct list_head head;
	char name[MAX_NAME_LENGTH];
	struct radius_cluster *rc;
};

static void radius_login(struct radius_ctx *ctx, bool updated_addresses);
static void radius_account(struct client *cl, enum client_req type,
		enum client_login status);
static struct radius_cluster* radius_get_cluster(char *name);

static const char *cfg_nasid = NULL;
static const struct client_backend *client_next = NULL;

static struct list_head radius_clusters = LIST_HEAD_INIT(radius_clusters);

static void radius_deinit()
{
#ifdef __SC_BUILD__
	struct radius_cluster_entry *rc_entry;

	radius_clear_default_servers();

	while (!list_empty(&radius_clusters)) {
		rc_entry = list_first_entry(&radius_clusters,
					    struct radius_cluster_entry, head);
		radius_destroy(rc_entry->rc);
		list_del(&rc_entry->head);
		free(rc_entry);
	}
#endif
	return;
}

static void radius_resident_deinit()
{
	struct radius_cluster_entry *rc_entry;

	radius_clear_default_servers();

	while (!list_empty(&radius_clusters)) {
		rc_entry = list_first_entry(&radius_clusters,
					    struct radius_cluster_entry, head);
		radius_destroy(rc_entry->rc);
		list_del(&rc_entry->head);
		free(rc_entry);
	}
}

static int radius_apply()
{
	const char *cfg_next = config_get_string("radeapauth", "next", NULL);
	if (!cfg_next)
		cfg_next = config_get_string("radauth", "next", NULL);
	client_next = client_get_backend(cfg_next);

	return 0;
}

static int radius_resident_apply()
{
	int ret = 0;
	cfg_nasid = config_get_string("main", "nasid", NULL);
	if(!hotspot_assertconf_string("main.nasid", cfg_nasid)) {
		ret = -1;
	}
	
	radius_apply();
	radius_deinit();
	return ret;
}

static int radius_init()
{
	return radius_apply();
}

static int radius_resident_init()
{
	radius_init_default_servers();
	return radius_resident_apply();
}

static void radius_handler(struct client *cl, enum client_req req,
		client_update_cb *update, void **vctx)
{
    struct radius_ctx *ctx = *vctx;
    struct radius_cluster *rad_auth = radius_get_cluster(cl->sess.auth_conf);
#ifdef __SC_BUILD__
    if(!client_is_reach_max_login_user(cl))
    {
#endif
        if (req == CLIENT_LOGIN) {
            if (!ctx) {
                ctx = malloc(sizeof(*ctx));
                if (!ctx) {
#ifndef __SC_BUILD__
                    syslog(LOG_ERR, "%s: Context memory allocation failed", __FUNCTION__);
#endif
                    update(cl, LOGIN_ERROR, NULL);
                }
            }
            if (ctx) {
                *vctx = ctx;
                ctx->update = update;
                ctx->client = cl;
                ctx->handle = 0;
                radius_login(ctx, false);
            }
        } else if (req == CLIENT_UPDATE_ADDRESSES) {
            radius_login(ctx, true);
        } else if (req == CLIENT_ACCOUNTING_START
                || req == CLIENT_ACCOUNTING_STOP
                || req == CLIENT_ACCOUNTING_INTERIM) {
            radius_account(ctx->client, req, cl->login_status);
        } else if (req == CLIENT_LOGOUT && ctx) {
            *vctx = NULL;
            if (ctx->handle && rad_auth)	// Abort login
                radius_cancel(rad_auth, ctx->handle);
            free(ctx);
        }
#ifdef __SC_BUILD__
    }
#endif
}

static void radius_updatecb(struct radius_pkt *pkt, void *vctx)
{
	struct radius_ctx *ctx = vctx;
	struct client *cl = ctx->client;
	ctx->handle = 0;

	enum client_login status;
	if (!pkt) // No response received
		status = LOGIN_TIMEOUT;
	else if (pkt->code == RADIUS_ACCESS_ACCEPT)
		status = LOGIN_SUCCESS;
	else
		status = LOGIN_DENIED;

	ctx->update(cl, status, NULL);
}

static void radius_authcb(struct radius_pkt *pkt, void *vctx)
{
	struct radius_ctx *ctx = vctx;
	struct client *cl = ctx->client;
	ctx->handle = 0;

	if (!pkt) { // No response received
		ctx->update(cl, LOGIN_TIMEOUT, NULL);
		return;
	}

	struct radius_attr *attr;
	int len;

	if (cl->sess.method == CLIENT_EAP) {
		uint8_t eapbuf[1265];
		size_t eaplen = 0;
		radius_foreach_attr(attr, pkt) { // Rebuild EAP-Reply
			if ((len = radius_pkt_match(attr, RADIUS_EAP_MSG, 0))) {
				if ((sizeof(eapbuf) - eaplen) < len) {
#ifdef __SC_BUILD__
                log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "radius: EAP-"
							"reply too big\n");
#else
				syslog(LOG_WARNING, "radius: EAP-"
							"reply too big");
#endif
					ctx->update(cl, LOGIN_ERROR, NULL);
					return;
				}
				memcpy(eapbuf + eaplen, attr->payload, len);
				eaplen += len;
			} else if ((len = radius_pkt_match(attr,
					RADIUS_STATE, 0))) {
				cl->sess.statelen = 0;
				free(cl->sess.state);
				if ((cl->sess.state = malloc(len))) {
					memcpy(cl->sess.state,
							attr->payload, len);
					cl->sess.statelen = len;
				}
			}
		}

		if (eaplen) { // Copy EAP-Reply
			free(cl->sess.eap);
			if (!(cl->sess.eap = malloc(eaplen))) {
#ifndef __SC_BUILD__
				syslog(LOG_ERR, "%s: Context memory allocation failed", __FUNCTION__);
#endif
				ctx->update(cl, LOGIN_ERROR, NULL);
				return;
			}
			memcpy(cl->sess.eap, eapbuf, eaplen);
			cl->sess.eaplen = eaplen;
		}
	}

	if (pkt->code == RADIUS_ACCESS_CHALLENGE) {
		radius_foreach_attr(attr, pkt) { // Configure session according to RADIUS
			if (radius_pkt_match(attr,
					     RADIUS_SESSION_TIMEOUT, 0) == 4) {
				cl->sess.limit_time_conn = radius_pkt_get_u32(attr);
			}
		}
		ctx->update(cl, LOGIN_CHALLENGE, NULL);
		return;
	} else if (pkt->code != RADIUS_ACCESS_ACCEPT) {	
		radius_foreach_attr(attr, pkt) {
			if ((len = radius_pkt_match(attr,
					RADIUS_REPLY_MSG, 0))) {
				cl->sess.reply = strndup(
						(char*)attr->payload, len);
			}
		}

		ctx->update(cl, LOGIN_DENIED, NULL);
		return;
	}

	uint8_t *fon_tun_user = NULL, *fon_tun_pass = NULL;
	size_t fon_tun_user_l = 0, fon_tun_pass_l = 0;

	// Preapre savinf of class
	size_t class_len = 0;
	free(cl->sess.class);
	cl->sess.class = NULL;

	radius_foreach_attr(attr, pkt) { // Configure session according to RADIUS
		struct radius_attr *vattr = radius_vattr_to_attr(attr);
		if (radius_pkt_match(attr,
				RADIUS_ACCT_INTERIM_INTERVAL, 0) == 4) {
			uint32_t val = radius_pkt_get_u32(attr);
			if (val >= 60)
				cl->sess.interim_interval = val;
		} else if ((len = radius_pkt_match(attr, RADIUS_CLASS, 0))) {
			if (!cl->sess.class) {
				uint8_t *newcls = malloc(len + 2);
				if (!newcls)
					continue;
				newcls[0] = len;
				memcpy(&newcls[1], attr->payload, len);
				newcls[len + 1] = 0;
				class_len = len + 2;
				cl->sess.class = newcls;
			} else {
				uint8_t *newcls = realloc(cl->sess.class,
						class_len + len + 1);
				if (!newcls)
					continue;
				newcls[class_len - 1] = len;
				memcpy(&newcls[class_len], attr->payload, len);
				newcls[class_len + len] = 0;
				class_len += len + 1;
				cl->sess.class = newcls;
			}
		} else if ((len = radius_pkt_match(attr, RADIUS_CUI, 0))) {
			if (!cl->sess.cui && (cl->sess.cui = malloc(len))) {
				memcpy(cl->sess.cui, attr->payload, len);
				cl->sess.cui_len = len;
			}
		} else if (radius_pkt_match(attr,
					RADIUS_SESSION_TIMEOUT, 0) == 4) {
			cl->sess.limit_time_conn = radius_pkt_get_u32(attr);
		} else if (radius_pkt_match(attr,
					RADIUS_IDLE_TIMEOUT, 0) == 4) {
			cl->limit_time_idle = radius_pkt_get_u32(attr);
		} else if (radius_pkt_match(attr,
					RADIUS_TERMINATION_ACTION, 0) == 4) {
			cl->sess.term_action = radius_pkt_get_u32(attr);
		} else if (radius_pkt_match(attr,
					RADIUS_CHILLISPOT_MAX_INPUT_OCTETS,
					RADIUS_VENDOR_CHILLISPOT) == 4) {
			cl->sess.limit_bytes_in = radius_pkt_get_u32(vattr);
		} else if (radius_pkt_match(attr,
					RADIUS_CHILLISPOT_MAX_OUTPUT_OCTETS,
					RADIUS_VENDOR_CHILLISPOT) == 4) {
			cl->sess.limit_bytes_out = radius_pkt_get_u32(vattr);
		} else if (radius_pkt_match(attr,
					RADIUS_CHILLISPOT_MAX_TOTAL_OCTETS,
					RADIUS_VENDOR_CHILLISPOT) == 4) {
			cl->sess.limit_bytes_total = radius_pkt_get_u32(vattr);
		} else if ((len = radius_pkt_match(attr,
				RADIUS_MS_MPPE_RECV_KEY, RADIUS_VENDOR_MS))) {
			if (!cl->sess.mppe.recv_key &&
					(cl->sess.mppe.recv_key = malloc(len))) {
				cl->sess.mppe.recv_keylen = len;
				memcpy(cl->sess.mppe.recv_key,
						vattr->payload, len);
			}
		} else if ((len = radius_pkt_match(attr,
				RADIUS_MS_MPPE_SEND_KEY, RADIUS_VENDOR_MS))) {
			if (!cl->sess.mppe.send_key &&
					(cl->sess.mppe.send_key = malloc(len))) {
				cl->sess.mppe.send_keylen = len;
				memcpy(cl->sess.mppe.send_key,
						vattr->payload, len);
			}
		} else if ((len = radius_pkt_match(attr,
				RADIUS_USER_NAME, 0))) {
			fon_tun_user = attr->payload;
			fon_tun_user_l = len;
		} else if ((len = radius_pkt_match(attr,
				RADIUS_FON_TUNNEL_PASS, RADIUS_VENDOR_FON))) {
			fon_tun_pass = vattr->payload;
			fon_tun_pass_l = len;
		}
	}

	if (client_next) { // Want tunneling
		if (fon_tun_user && fon_tun_pass) {
			free(cl->sess.username);
			free(cl->sess.key);

			cl->sess.authed = true;
			cl->sess.method = CLIENT_PAP;
			cl->sess.username = strndup((char*)fon_tun_user, fon_tun_user_l);
			if ((cl->sess.key = malloc(fon_tun_pass_l)))
				memcpy(cl->sess.key, fon_tun_pass, fon_tun_pass_l);
			cl->sess.keylen = fon_tun_pass_l;
#ifndef __SC_BUILD__
			syslog(LOG_INFO, "radius: RADIUS provided tunnel credentials. Proceeding...");
#endif
			ctx->update(cl, LOGIN_NOTYET, client_next);

		} else if (cl->sess.method == CLIENT_EAP) {
#ifndef __SC_BUILD__
			syslog(LOG_INFO, "radius: Tunnel credentials not provided. Abort login...");
#endif
			ctx->update(cl, LOGIN_DENIED, NULL);
		} else {
			ctx->update(cl, LOGIN_SUCCESS, NULL);
		}
	} else {
		if (fon_tun_user && cl->sess.method == CLIENT_EAP) { // Use OTU for EAP Accounting
			free(cl->sess.username);
			cl->sess.username = strndup((char*)fon_tun_user, fon_tun_user_l);
		}
		ctx->update(cl, LOGIN_SUCCESS, NULL);
	}
}

// Add RADIUS attributes shared between auth and acct
static void radius_common_attrs(struct radius_pkt *pkt, struct client *cl, struct radius_cluster *radius_be)
{
	if (cl->sess.username)
		radius_pkt_append(pkt, RADIUS_USER_NAME, cl->sess.username,
						strlen(cl->sess.username));
	// Add NAS-IP-Address
	union {
		struct sockaddr_in in;
		struct sockaddr_in6 in6;
		struct sockaddr s;
	} addr;
	socklen_t addrlen = sizeof(addr);
	getsockname(radius_sock(radius_be), &addr.s, &addrlen);

	/* Map mapped-IPv4 back to IPv4 */
	if (addr.in6.sin6_family == AF_INET6
	&& IN6_IS_ADDR_V4MAPPED(&addr.in6.sin6_addr)) {
		addr.in.sin_addr.s_addr = addr.in6.sin6_addr.s6_addr32[3];
		addr.in.sin_family = AF_INET;
	}

	if (addr.in.sin_family == AF_INET)
		radius_pkt_append(pkt, RADIUS_NAS_IP, NULL,
					htonl(addr.in.sin_addr.s_addr));

	for (size_t i = 0; cl->sess.req_ipaddr && i < cl->sess.req_ipaddr_cnt; ++i)
		radius_pkt_append(pkt, RADIUS_FRAMED_IP_ADDRESS,
				cl->sess.req_ipaddr, sizeof(struct in_addr));

	for (size_t i = 0; cl->sess.req_ip6addr && i < cl->sess.req_ip6addr_cnt; ++i) {
		uint8_t buf[18];
		buf[0] = 0;
		buf[1] = 128;
		memcpy(&buf[2], &cl->sess.req_ip6addr[i], 16);
		radius_pkt_append(pkt, RADIUS_FRAMED_IPV6_PREFIX,
			buf, sizeof(buf));
	}

	char buf[256];
#ifdef CONFIG_SUPPORT_WIFI_5G
    if(cl->is_5g)
    {
        heXlify(buf, routing_cfg.iface_addr_5g, 6, '-');
    }
    else
#endif
    {
        heXlify(buf, routing_cfg.iface_addr, 6, '-');
    }
#ifdef __SC_BUILD__
    strcat(buf,":");
#ifdef CONFIG_SUPPORT_WIFI_5G
    if(cl->is_5g)
    {
        strcat(buf,routing_cfg.eap_5g_ssid); 
    }
    else
#endif
    {
        strcat(buf,routing_cfg.eap_ssid); 
    }
#endif
	radius_pkt_append(pkt, RADIUS_CALLED_STATION_ID, buf, strlen(buf));


#ifndef __SC_BUILD__
	snprintf(buf, sizeof(buf), "%.8x", cl->sess.id);
	radius_pkt_append(pkt, RADIUS_ACCT_SESSION_ID, buf, strlen(buf));
#endif
	heXlify(buf, cl->mac, 6, '-');
	radius_pkt_append(pkt, RADIUS_CALLING_STATION_ID, buf, strlen(buf));
#ifdef __SC_BUILD__
	heXlify(buf, routing_cfg.iface_addr, 6, '-');
    radius_pkt_append(pkt, RADIUS_NAS_IDENTIFIER, buf,
            strlen(buf));
#else
	if (cfg_nasid)
		radius_pkt_append(pkt, RADIUS_NAS_IDENTIFIER, cfg_nasid,
							strlen(cfg_nasid));
#endif
#ifdef __SC_BUILD__
	radius_pkt_append(pkt, RADIUS_NAS_PORT_TYPE, NULL,
			RADIUS_NAS_PORT_TYPE_80211);
#endif
	const char *loc_id = config_get_string("main", "location_id",
									NULL);
	const char *loc_nam = config_get_string("main", "location_name",
									NULL);
	const char *v_class = config_get_string("main", "venue_class", NULL);
	const char *op_name = config_get_string("main", "operator_name",
									NULL);
	const char *loc_inf = config_get_string("main", 
						"location_information", NULL);
	const char *loc_data = config_get_string("main", "location_data",
									NULL);
	const char *basic_loc_policy = config_get_string("main", 
						"basic_policy_rules", NULL);
	const char *ext_loc_policy = config_get_string("main",
						"extended_policy_rules", NULL);
	const char *loc_capable = config_get_string("main", 
						"location_capable", NULL);
	const char *req_loc_info = config_get_string("main",
					"requested_location_info", NULL);


	if (loc_id)
		radius_pkt_append_vendor(pkt, RADIUS_VENDOR_WISPR,
			RADIUS_WISPR_LOCATION_ID, loc_id, strlen(loc_id));

	if (loc_nam)
		radius_pkt_append_vendor(pkt, RADIUS_VENDOR_WISPR,
			RADIUS_WISPR_LOCATION_NAME, loc_nam, strlen(loc_nam));

	if (v_class)
		radius_pkt_append_vendor(pkt, RADIUS_VENDOR_TMOBILE,
				RADIUS_TMOBILE_VENUE_CLASS, NULL, atoi(v_class));

	if (op_name)
		radius_pkt_append(pkt, RADIUS_OPERATOR_NAME, op_name, strlen(op_name));

	if (loc_inf)
		radius_pkt_append(pkt, RADIUS_LOCATION_INFORMATION, loc_inf, strlen(loc_inf));

	if (loc_data)
		radius_pkt_append(pkt, RADIUS_LOCATION_DATA, loc_data, strlen(loc_data));

	if (basic_loc_policy)
		radius_pkt_append(pkt, RADIUS_BASIC_POLICY_RULES, basic_loc_policy, strlen(basic_loc_policy));

	if (ext_loc_policy)
		radius_pkt_append(pkt, RADIUS_EXTENDED_POLICY_RULES, ext_loc_policy, strlen(ext_loc_policy));

	if (loc_capable)
		radius_pkt_append(pkt, RADIUS_LOCATION_CAPABLE, NULL, atoi(loc_capable));

	if (req_loc_info)
		radius_pkt_append(pkt, RADIUS_REQUESTED_LOCATION, NULL, atoi(req_loc_info));


}

static void radius_login(struct radius_ctx *ctx, bool update_addresses)
{
	struct client *cl = ctx->client;
	struct radius_cluster *rad_auth = radius_get_cluster(cl->sess.auth_conf);

	if (cl->sess.authed) { // Already authenticated, only do accounting
		ctx->update(cl, LOGIN_SUCCESS, NULL);
		return;
	} else if (!cl->sess.username || !cl->sess.key) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "No username and/or key provided\n");
#else
		syslog(LOG_ERR, "%s: No username and/or key provided", __FUNCTION__);
#endif
		ctx->update(cl, LOGIN_ERROR, NULL);
		return;
	} else if (!rad_auth) { // No RADIUS authentication cluster
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "No radius authentication cluster available\n");
#else
		syslog(LOG_ERR, "%s: No radius authentication cluster available", __FUNCTION__);
#endif
		ctx->update(cl, LOGIN_ERROR, NULL);
		return;
	}

	uint8_t buffer[3072];
	struct radius_pkt *pkt = radius_pkt_init(buffer, RADIUS_ACCESS_REQUEST);

	// Service-Type: login
#ifndef __SC_BUILD__
	radius_pkt_append(pkt, RADIUS_SERVICE_TYPE, NULL, 1);
#endif
	// Common attributes
	radius_common_attrs(pkt, cl, rad_auth);

#ifndef __SC_BUILD__
	// Request CUI
	char nul = 0;
	radius_pkt_append(pkt, RADIUS_CUI, &nul, 1);

	// WISPr Logoff URL
	char ip[INET6_ADDRSTRLEN], buf[64];

	if (cl->sess.uamaddr.sa.sa_family == AF_INET6) {
		inet_ntop(AF_INET6, &cl->sess.uamaddr.in6.sin6_addr,
					ip, sizeof(ip));
		snprintf(buf, sizeof(buf), "http://[%s]:%u/logoff", ip,
				ntohs(cl->sess.uamaddr.in6.sin6_port));
	} else {
		inet_ntop(AF_INET, &cl->sess.uamaddr.in.sin_addr,
					ip, sizeof(ip));
		snprintf(buf, sizeof(buf), "http://%s:%u/logoff", ip,
				ntohs(cl->sess.uamaddr.in.sin_port));
	}
	radius_pkt_append_vendor(pkt, RADIUS_VENDOR_WISPR,
		RADIUS_WISPR_LOGOFF_URL, buf, strlen(buf));
#endif

	if (cl->sess.state)
		radius_pkt_append(pkt, RADIUS_STATE, cl->sess.state,
				cl->sess.statelen);

	if (cl->sess.method == CLIENT_PAP) {
		radius_pkt_append(pkt, RADIUS_USER_PASSWORD, cl->sess.key,
							cl->sess.keylen);
	} else if (cl->sess.method == CLIENT_CHAP) {
		if (cl->sess.keylen != 16) {
			ctx->update(cl, LOGIN_INVALID_REQUEST, NULL);
			return;
		}

		uint8_t chap[17] = {0};
		memcpy(&chap[1], cl->sess.key, cl->sess.keylen);

		memcpy(pkt->authenticator, cl->challenge,
						sizeof(pkt->authenticator));
		radius_pkt_append(pkt, RADIUS_CHAP_PASSWORD, chap, 17);
	} else if (cl->sess.method == CLIENT_EAP && cl->sess.keylen <= 1265) {
		size_t eaplen = cl->sess.keylen;
		uint8_t *ckey = cl->sess.key;
		while (eaplen > 0) {
			size_t clen = (eaplen > 253) ? 253 : eaplen;
			radius_pkt_append(pkt, RADIUS_EAP_MSG, ckey, clen);
			eaplen -= clen;
			ckey += clen;
		}
	} else {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Invalid session login method\n");
#else
		syslog(LOG_ERR, "%s: Invalid session login method", __FUNCTION__);
#endif
		ctx->update(cl, LOGIN_ERROR, NULL);
		return;
	}


	uint8_t zero[16] = {0};
	radius_pkt_append(pkt, RADIUS_MSG_AUTHENTICATOR, zero, 16);

	radius_cb *cb = (!update_addresses) ? radius_authcb : radius_updatecb;
	if (!(ctx->handle = radius_request(rad_auth, pkt, cb, ctx))) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Handle error in radius request\n");
#else
		syslog(LOG_ERR, "%s: Handle error in radius request", __FUNCTION__);
#endif
		ctx->update(cl, LOGIN_ERROR, NULL);
	}
}

// Send accounting messages
static void radius_account(struct client *cl, enum client_req type,
		enum client_login status)
{
	uint8_t buffer[1536];
	struct radius_cluster *radius_accounting = radius_get_cluster(cl->sess.acct_conf);
	struct radius_pkt *pkt = NULL;

	if (!radius_accounting) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "no radius accounting backend for client %i\n", client_id(cl));
#else
		syslog(LOG_WARNING, "%s: no radius accounting backend for client %i",__FUNCTION__, client_id(cl));
#endif
		return;
	}

	pkt = radius_pkt_init(buffer,
			      RADIUS_ACCOUNTING_REQUEST);
	radius_pkt_append(pkt, RADIUS_ACCT_STATUS_TYPE, NULL, type);

	// Common attributes
	radius_common_attrs(pkt, cl, radius_accounting);

	if (cl->sess.class)
		for (uint8_t *cls = cl->sess.class; *cls; cls += *cls + 1)
			radius_pkt_append(pkt, RADIUS_CLASS, cls + 1, *cls);

	if (cl->sess.cui)
		radius_pkt_append(pkt, RADIUS_CUI, cl->sess.cui,
							cl->sess.cui_len);

	/* FON-AP requirements... */
	radius_pkt_append(pkt, RADIUS_ATTR_EVENT_TIMESTAMP, NULL,
							(uint32_t)time(NULL));

	/* ...FON-AP requirements */

	if (type == CLIENT_ACCOUNTING_STOP || type == CLIENT_ACCOUNTING_INTERIM) {
		uint64_t byte_in, byte_out;
		uint32_t pkt_in, pkt_out;
		if (cl->sess.stats_cb(client_id(cl), &byte_in, &byte_out,
				&pkt_in, &pkt_out)) {
#ifdef __SC_BUILD__
            log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unable to get stats for user %i",
					client_id(cl));
#else
			syslog(LOG_WARNING, "Unable to get stats for user %i",
					client_id(cl));
#endif
			return;
		}

		uint32_t stime = (event_time() - cl->sess.time_start) / 1000;

		radius_pkt_append(pkt, RADIUS_ACCT_INPUT_OCTETS,
							NULL, byte_in);
		radius_pkt_append(pkt, RADIUS_ACCT_INPUT_GIGAWORDS,
							NULL, byte_in >> 32);
		radius_pkt_append(pkt, RADIUS_ACCT_OUTPUT_OCTETS,
							NULL, byte_out);
		radius_pkt_append(pkt, RADIUS_ACCT_OUTPUT_GIGAWORDS,
							NULL, byte_out >> 32);
		radius_pkt_append(pkt, RADIUS_ACCT_INPUT_PACKETS,
							NULL, pkt_in);
		radius_pkt_append(pkt, RADIUS_ACCT_OUTPUT_PACKETS,
							NULL, pkt_out);
		radius_pkt_append(pkt, RADIUS_ACCT_SESSION_TIME,
							NULL, stime);

		if (type == CLIENT_ACCOUNTING_STOP)
			radius_pkt_append(pkt, RADIUS_ACCT_TERMINATE_CAUSE,
							NULL, status);
	}

	// Not interested in the reply, just make sure it is received	
	radius_request(radius_accounting, pkt, NULL, NULL);
}


static struct radius_cluster* radius_get_cluster(char *name)
{
	struct radius_cluster_entry *rc_entry = NULL;
	struct radius_cluster *rc = NULL;

	if (!name)
		return NULL;

	/* Search for an already created backend with that name */
	list_for_each_entry(rc_entry, &radius_clusters, head) {
	  if (!strncmp(name, rc_entry->name, MAX_NAME_LENGTH)) {
			rc = rc_entry->rc;
			break;
		}
	}

	if (rc == NULL) { /* No radius backend found. Create a new one */
		if ((rc = radius_create(name, NULL, devname)) != NULL) {
			rc_entry = malloc(sizeof(struct radius_cluster_entry));
			strncpy(rc_entry->name, name, MAX_NAME_LENGTH);
			rc_entry->rc = rc;
			list_add(&rc_entry->head, &radius_clusters);
		}
	}

	return rc;
}

static struct client_backend rad_backend = {
	.auth = radius_handler,
	.name = "radius",
};

BACKEND_REGISTER(rad_backend)
RESIDENT_REGISTER(radius_resident, 309)
MODULE_REGISTER(radius, 310)
