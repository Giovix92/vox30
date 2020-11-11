/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lib/usock.h"
#include "lib/event.h"
#include "lib/radius.h"
#include "lib/config.h"
#include "lib/hexlify.h"

#include "core/client.h"
#ifndef __SC_BUILD__
#include "core/routing.h"
#endif
#include "core/trigger.h"
#include "core/hotspotd.h"

struct eapradhandle {
	uint8_t identifier;
	struct sockaddr_in6 addr;
	socklen_t addrlen;
	uint8_t authen[16];
};

static char *cfg_secret = NULL;
static char eapstatus[10] = {0};
static int cfg_eap_idletime = 0, cfg_main_idletime = 0;

static void eapradserv_handle(struct event_epoll *ev, uint32_t revents);

static struct event_epoll ev_rpc = {
	.events = EPOLLIN | EPOLLET,
	.handler = eapradserv_handle,
};
#ifdef CONFIG_SUPPORT_5G_QD
static struct event_epoll ev_rpc_5g = {
	.events = EPOLLIN | EPOLLET,
	.handler = eapradserv_handle,
};
#endif
static int eapradserv_apply()
{
	free(cfg_secret);
	cfg_secret = strdup(config_get_string("eapradserv", "secret", "eureka"));

	const char *status = config_get_string("eapradserv", "eapstatus", "");
	if (!strlen(eapstatus) || strcmp(status, eapstatus)) {
		strncpy(eapstatus, status, sizeof(eapstatus) - 1);

		char trigger_name[24] = "trigger_";
		strncat(trigger_name, status, 10);

		const char *trigger = config_get_string("eapradserv", trigger_name, "/sbin/hotspot-up");
		trigger_run(trigger, NULL, NULL, NULL, NULL);
	}
	cfg_eap_idletime = config_get_int("eapradserv", "eap_idletime", 10);
	cfg_main_idletime = config_get_int("client", "idle_max", 600);

	return -!cfg_secret;
}

static int eapradserv_init()
{
#ifdef CONFIG_SUPPORT_5G_QD
	const char *host_5g = config_get_string("eapradserv", "bind", "1.1.1.1");
#endif
	const char *host = config_get_string("eapradserv", "bind", "127.0.0.1");
	const char *port = config_get_string("eapradserv", "port", "1818");

	ev_rpc.fd = usock(SOCK_DGRAM | USOCK_SERVER, host, port);
	if (ev_rpc.fd < 0)
		return -1;

	event_ctl(EVENT_EPOLL_ADD, &ev_rpc);
#ifdef CONFIG_SUPPORT_5G_QD
	ev_rpc_5g.fd = usock(SOCK_DGRAM | USOCK_SERVER , host_5g, port);
    
	if (ev_rpc_5g.fd < 0)
		return -1;

	event_ctl(EVENT_EPOLL_ADD, &ev_rpc_5g);
#endif
	return eapradserv_apply();
}

static void eapradserv_deinit()
{

	close(ev_rpc.fd);
	ev_rpc.fd = -1;
#ifdef CONFIG_SUPPORT_5G_QD
	close(ev_rpc_5g.fd);
	ev_rpc_5g.fd = -1;
#endif
	free(cfg_secret);
	cfg_secret = NULL;
}

static void eapradserv_cb(struct client* cl, enum client_login status, void *ctx)
{
	struct eapradhandle *hndl = ctx;
	int code = (status == LOGIN_SUCCESS) ? RADIUS_ACCESS_ACCEPT :
			(status == LOGIN_CHALLENGE) ? RADIUS_ACCESS_CHALLENGE :
			RADIUS_ACCESS_REJECT;

	uint8_t buf[4096];
	struct radius_pkt *pkt = radius_pkt_init(buf, code);
	pkt->identifier = hndl->identifier;
	memcpy(pkt->authenticator, hndl->authen, 16);

	if (code == RADIUS_ACCESS_ACCEPT) {
		//set idletime to the idle_max value on session establishment
		cl->limit_time_idle = cfg_main_idletime;

		if (cl->sess.limit_time_conn < 10000000)
			radius_pkt_append(pkt, RADIUS_SESSION_TIMEOUT,
					NULL, cl->sess.limit_time_conn);
		if (cl->limit_time_idle < 10000000)
			radius_pkt_append(pkt, RADIUS_IDLE_TIMEOUT,
					NULL, cl->limit_time_idle);
		if (cl->sess.mppe.recv_key)
			radius_pkt_append_vendor(pkt, RADIUS_VENDOR_MS,
					RADIUS_MS_MPPE_RECV_KEY,
					cl->sess.mppe.recv_key,
					cl->sess.mppe.recv_keylen);
		if (cl->sess.mppe.send_key)
			radius_pkt_append_vendor(pkt, RADIUS_VENDOR_MS,
					RADIUS_MS_MPPE_SEND_KEY,
					cl->sess.mppe.send_key,
					cl->sess.mppe.send_keylen);

		/* rfc3580 3.17 3.19 . If not setted by radius backend 
		 it will contain Default (0) */
		radius_pkt_append(pkt, RADIUS_TERMINATION_ACTION,
				  NULL, cl->sess.term_action);
	}

	/* rfc3580 3.17 forwarding Session-Timeout in access-challenge 
	   to set EAP-req timer */ 
	if (code == RADIUS_ACCESS_CHALLENGE) {
		if (cl->sess.limit_time_conn < 10000000)
			radius_pkt_append(pkt, RADIUS_SESSION_TIMEOUT,
					NULL, cl->sess.limit_time_conn);
	}

	if (cl->sess.eap && cl->sess.eaplen <= 1265) {
		size_t eaplen = cl->sess.eaplen;
		uint8_t *ckey = cl->sess.eap;
		while (eaplen > 0) {
			size_t clen = (eaplen > 253) ? 253 : eaplen;
			radius_pkt_append(pkt, RADIUS_EAP_MSG, ckey, clen);
			eaplen -= clen;
			ckey += clen;
		}
	}

	uint8_t zero[16] = {0};
	radius_pkt_append(pkt, RADIUS_MSG_AUTHENTICATOR, zero, sizeof(zero));

	size_t plen = radius_finalize(pkt, cfg_secret?:"");
#ifdef CONFIG_SUPPORT_5G_QD
    if(cl->is_5g)
    {
        sendto(ev_rpc_5g.fd, pkt, plen, MSG_DONTWAIT,
                (struct sockaddr*)&hndl->addr, hndl->addrlen);
    }
    else
#endif
        sendto(ev_rpc.fd, pkt, plen, MSG_DONTWAIT,
                (struct sockaddr*)&hndl->addr, hndl->addrlen);
	free(hndl);
}

static void eapradserv_handle(struct event_epoll *ev, uint32_t revents)
{
	uint8_t buf[4096];
	struct radius_pkt *pkt = (struct radius_pkt*)buf;
	struct client *cl;
#ifdef __SC_BUILD__
#ifdef CONFIG_SUPPORT_WIFI_5G
    char if_name[64] = {0};
#endif
#endif

	for (;;) {
		struct sockaddr_in6 addr;
		socklen_t alen = sizeof(addr);
#ifdef CONFIG_SUPPORT_5G_QD
		ssize_t len = recvfrom(ev->fd, buf, sizeof(buf) - 1,
				MSG_DONTWAIT, (void*)&addr, &alen);
#else
		ssize_t len = recvfrom(ev_rpc.fd, buf, sizeof(buf) - 1,
				MSG_DONTWAIT, (void*)&addr, &alen);
#endif
		if (len < 0 && errno == EAGAIN)
			return;
		if (len < sizeof(*pkt) || ntohs(pkt->length) != len)
			continue;

		pkt->length = len;
		buf[len] = 0;

		uint8_t mac[6] = {0};
		uint8_t eapbuf[1265];
		char username[254] = "";
		size_t eaplen = 0;

		struct radius_attr *attr;
		radius_foreach_attr(attr, pkt) {
			if ((len = radius_pkt_match(attr,
					RADIUS_USER_NAME, 0))) {
				memcpy(username, attr->payload, len);
			} else if ((len = radius_pkt_match(attr,
					RADIUS_CALLING_STATION_ID, 0))) {
				if (len > 17)
					continue;

				char macbuf[18];
				memcpy(macbuf, attr->payload, len);
				macbuf[len] = 0;

				if (sscanf(macbuf, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx",
						&mac[0], &mac[1], &mac[2],
						&mac[3], &mac[4], &mac[5]) == 6)
					continue;

				if (sscanf(macbuf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
						&mac[0], &mac[1], &mac[2],
						&mac[3], &mac[4], &mac[5]) == 6)
					continue;

				if (len == 12)
					unhexlify(mac, macbuf, 12);

			} else if ((len = radius_pkt_match(attr,
					RADIUS_EAP_MSG, 0))) {
				if ((sizeof(eapbuf) - eaplen) < len)
					continue; // EAP too long
				memcpy(eapbuf + eaplen, attr->payload, len);
				eaplen += len;
			}
#ifdef __SC_BUILD__
#ifdef CONFIG_SUPPORT_WIFI_5G
#ifdef CONFIG_SUPPORT_5G_QD
            else if ((len = radius_pkt_match(attr,
                            RADIUS_NAS_IP, 0))) 
            {
                unsigned int ip = 0;
				if (len != 4)
					continue; // len not match
                memcpy(&ip, attr->payload, 4);
                inet_ntop(AF_INET, (void *)&ip, if_name, sizeof(if_name) - 1);
            }
#else
            else if ((len = radius_pkt_match(attr,
                            RADIUS_NAS_PORT_ID, 0))) 
            {
				if (sizeof(if_name) < len)
					continue; // len not match
                memcpy(if_name, attr->payload, len);
            }
#endif
#endif
#endif
		}

		struct eapradhandle *hndl;

restart_cl:
		if (!eaplen || !(cl = client_register(routing_cfg.iface_eap_index
                , AF_PACKET, mac
#ifdef CONFIG_SUPPORT_WIFI_5G
#ifdef CONFIG_SUPPORT_5G_QD
                ,(strcmp(if_name,"1.1.1.10"))?0:1
#else
                ,(!strcmp(if_name,routing_cfg.iface_eap))?0:1
#endif
#endif
                )))
			continue; // No EAP packet or unable to alloc user

		if (cl->sess.login_cb == eapradserv_cb) {
			// Already a eapradserv-login in progress
			hndl = cl->sess.login_ctx;
			if (!memcmp(hndl->authen, pkt->authenticator, 16))
				continue; // Skip simple retransmissions
		}

		if (cl->status == CLSTAT_AUTHED) {
			client_logout(cl, LOGOUT_USER);
			goto restart_cl;
		}

		if (!(hndl = malloc(sizeof(*hndl))))
			continue;

		//Force idle timer on eap, in order to logout stalled clients
		cl->limit_time_idle = cfg_eap_idletime;

		hndl->identifier = pkt->identifier;
		hndl->addr = addr;
		hndl->addrlen = alen;
		memcpy(hndl->authen, pkt->authenticator, 16);

		client_login(cl, CLIENT_EAP, username,
				eapbuf, eaplen, eapradserv_cb, hndl);

	}
}

MODULE_REGISTER(eapradserv, 500)
