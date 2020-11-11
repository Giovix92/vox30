/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "lib/event.h"
#include "lib/usock.h"
#include "lib/md5.h"
#include "lib/config.h"
#include "lib/urandom.h"
#include "lib/str.h"
#include "lib/list.h"
#include "radius.h"
#include "core/hotspotd.h"

#define RADIUS_SOCKETS 4
#define SERVER_BUFSIZE 128

struct radius_request {
	struct radius_pkt *packet;
	radius_cb *cb;
	void *ctx;
	int32_t timeout;
	uint16_t try;
	uint16_t first;
};

struct radius_cluster {
	struct event_epoll sockets[RADIUS_SOCKETS];
	struct event_timer timer;
	struct radius_request queue[256];
	char secret[16];
	char section[16];
	char port[8];
	uint8_t current;
	uint8_t count;
	uint8_t timeout;
	uint8_t tries_max;
	uint8_t tries_server;
	uint8_t queuepos;
};

struct radius_server {
	struct list_head _head;
	char server[SERVER_BUFSIZE];
};

struct list_head default_servers;

static char *radius_type[] = {
	[RADIUS_ACCESS_REQUEST] = "Access-Request",
	[RADIUS_ACCESS_ACCEPT] = "Access-Accept",
	[RADIUS_ACCESS_REJECT] = "Access-Reject",
	[RADIUS_ACCOUNTING_REQUEST] = "Accounting-Request",
	[RADIUS_ACCOUNTING_RESPONSE] = "Accounting-Response",
	[RADIUS_ACCESS_CHALLENGE] = "Access-Challenge",
};

static void radius_hash_user_password
(const char *secret, const uint8_t *auth, uint8_t *password, ssize_t len);
static void radius_hmac_authen
(const char *secret, struct radius_pkt *pkt, uint8_t hash[16]);
static void radius_mppe_encrypt(const char *secret, const uint8_t *auth,
		uint8_t *payload, ssize_t len);
static void radius_mppe_decrypt(const char *secret, const uint8_t *auth,
		uint8_t *payload, ssize_t len);
static void radius_config_server(const char *server, void *cb);
static void radius_receiver(struct event_epoll *ev, uint32_t events);
static void radius_sender(struct event_timer *timer, int64_t now);

struct radius_config_args {
	struct radius_cluster *cl;
	const char *templatefill;
	const char *section;
};

void radius_init_default_servers()
{
	INIT_LIST_HEAD(&default_servers);
}

void radius_clear_default_servers()
{
	while(!list_empty(&default_servers)) {
		struct radius_server *s = list_first_entry(&default_servers, 
				struct radius_server, _head);
		free(s);
	}
}

static void radius_reconfigure(struct radius_cluster *cl, const char *section,
			const char *port, const char *templatefill) {
	for (size_t i = 0; i < cl->count; ++i)
		close(cl->sockets[i].fd);
	cl->count = 0;
	cl->current = 0;

	if (cl->timer.handler)
		event_ctl(EVENT_TIMER_DEL, &cl->timer);

	if (!section) { // Deconfigure
		for (size_t i = 0; i < 256; ++i) {
			struct radius_request *r = &cl->queue[i];
			if (r->packet) {
				if (r->cb)
					r->cb(NULL, r->ctx); // Notify callee
				free(r->packet);
				r->packet = NULL;
			}
		}
	} else { // Configure
		cl->tries_max = config_get_int(section, "tries_max", 4);
		cl->tries_server = config_get_int(section, "tries_server", 1);
		cl->timeout = config_get_int(section, "timeout", 7);
		const char *secret = config_get_string(section, "secret", NULL);
		if (secret)
			strncpy(cl->secret, secret, sizeof(cl->secret) - 1);

		strncpy(cl->section, section, sizeof(cl->section) - 1);

		const char *sport = (port) ? port :
				config_get_string(section, "port", NULL);
		if (sport)
			strncpy(cl->port, sport, sizeof(cl->port) - 1);

		cl->timer.handler = radius_sender;
		cl->timer.interval = 10000,
		cl->timer.context = cl;
		struct radius_config_args args;
		args.cl = cl;
		args.templatefill = NULL;
		args.section = section;
		config_foreach_list(section, "server", radius_config_server, &args);
		if(!cl->count) {	// Fallback to default servers
#ifndef __SC_BUILD__
			syslog(LOG_INFO, "%s: using default radius servers", __FUNCTION__);
#endif
			if(list_empty(&default_servers)) {
				args.templatefill = templatefill;
				config_foreach_list(section, "templateserver", radius_config_server,
						&args);
			} else {
				struct radius_server *s;
				args.templatefill = NULL;
				list_for_each_entry(s, &default_servers, _head) {	
					radius_config_server(s->server, &args);
				} 
			}
		}
		event_ctl(EVENT_TIMER_ADD, &cl->timer);
	}
}

static void radius_config_server(const char *server, void *cb) {
	struct radius_config_args *args = (struct radius_config_args *)cb;
	struct radius_cluster *cl = args->cl;
	char *port;
	const char *__server;

	if(args->templatefill) {
		char *__templatefill = strdup(args->templatefill);
		struct radius_server *default_server;
		if(!(default_server = (struct radius_server *)malloc(sizeof(struct
				radius_server)))) {
#ifndef __SC_BUILD__
			syslog(LOG_ERR, "%s: error in malloc", __FUNCTION__);
#endif
			return;	
		}
		snprintf(default_server->server, sizeof(default_server->server), server,
				strlwr(__templatefill));
		free(__templatefill);
		list_add(&default_server->_head, &default_servers);
		__server = default_server->server;
	} else {
		__server = server;
	}

	if (cl->count >= RADIUS_SOCKETS)
		return;

	__server = strtok((char *)__server, ":");
	port = strtok(NULL, ":");
	
	if (!port) {
		if (!cl->port || !strlen(cl->port)) {
#ifndef __SC_BUILD__
			syslog(LOG_DEBUG, "No radius ports provided. Using default 1812 and 1813.");
#endif
			if (!strcmp(cl->section, "radacct")) {
				strncpy(cl->port, "1813", sizeof(cl->port) - 1);
			} else {
				strncpy(cl->port, "1812", sizeof(cl->port) - 1);
			}
		} 
	} else {
		strncpy(cl->port, port, sizeof(cl->port) - 1);	
	}

	int fd = usock(USOCK_UDP, __server, cl->port);
	if (fd < 0) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to connect to RADIUS %s:%s (%s)",
			__server, cl->port, strerror(errno));
#else
		syslog(LOG_WARNING, "Failed to connect to RADIUS %s:%s (%s)",
			__server, cl->port, strerror(errno));
#endif
		return;
	}
	struct event_epoll *ep = &cl->sockets[cl->count++];
	ep->fd = fd;
	ep->events = EPOLLIN | EPOLLET;
	ep->handler = radius_receiver;
	ep->context = cl;
	event_ctl(EVENT_EPOLL_ADD, ep);
}

struct radius_cluster* radius_create(const char *section, const char *port,
			const char *templatefill) {
	if (!config_get_string(section, "secret", NULL))
		return NULL;

	struct radius_cluster *cl = calloc(1, sizeof(*cl));
	if (cl)
		radius_reconfigure(cl, section, port, templatefill);
	return cl;
}

void radius_destroy(struct radius_cluster *cl) {
	if (cl) {
		// Last sending / receiving
		radius_sender(&cl->timer, event_time());
		radius_reconfigure(cl, NULL, 0, NULL);
		free(cl);
	}
}

size_t radius_finalize(struct radius_pkt *npkt, const char *secret) {
	// Finalize messages
	struct radius_attr *attr;
	size_t len;
	radius_foreach_attr(attr, npkt) {
		if ((len = radius_pkt_match(attr,
				RADIUS_MS_MPPE_RECV_KEY, RADIUS_VENDOR_MS))
				|| (len = radius_pkt_match(attr,
				RADIUS_MS_MPPE_SEND_KEY, RADIUS_VENDOR_MS)))
			radius_mppe_encrypt(secret, npkt->authenticator,
					&attr->payload[6], len);
	}

	if (npkt->code == RADIUS_ACCESS_REQUEST) {
		struct radius_attr *attr, *authen = NULL;
		radius_foreach_attr(attr, npkt) {
			if (attr->type == RADIUS_USER_PASSWORD) {
				urandom(npkt->authenticator,
						sizeof(npkt->authenticator));
				radius_hash_user_password(secret,
						npkt->authenticator,
						attr->payload,
						attr->length - sizeof(*attr));
			} else if (attr->type == RADIUS_MSG_AUTHENTICATOR) {
				authen = attr;
			}
		}
		npkt->length = htons(npkt->length);
		if (authen)
			radius_hmac_authen(secret, npkt, authen->payload);
	} else if (npkt->code == RADIUS_ACCOUNTING_REQUEST) {
		npkt->length = htons(npkt->length);
		memset(npkt->authenticator, 0, sizeof(npkt->authenticator));
		md5_state_t md5;
		md5_init(&md5);
		md5_append(&md5, (void*)npkt, ntohs(npkt->length));
		md5_append(&md5, (void*)secret, strlen(secret));
		md5_finish(&md5, npkt->authenticator);
	} else { // Reply messages
		struct radius_attr *attr, *authen = NULL;
		radius_foreach_attr(attr, npkt)
			if (attr->type == RADIUS_MSG_AUTHENTICATOR)
				authen = attr;
		size_t plen = npkt->length;
		npkt->length = htons(plen);
		if (authen)
			radius_hmac_authen(secret, npkt, authen->payload);

		md5_state_t md5;
		md5_init(&md5);
		md5_append(&md5, (const void*)npkt, plen);
		md5_append(&md5, (const void*)secret, strlen(secret));
		md5_finish(&md5, npkt->authenticator);
	}
	return ntohs(npkt->length);
}

// Initiate a new RADIUS request
int radius_request(struct radius_cluster *cl,
const struct radius_pkt *pkt, radius_cb *cb, void *ctx) {
	if (cl->count < 1) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "No RADIUS connection found\n");
#else
		syslog(LOG_WARNING, "No RADIUS connection found");
		syslog(LOG_WARNING, "Trying to reconfigure RADIUS server...");
#endif
		radius_reconfigure(cl, cl->section, cl->port, NULL);
		if (cl->count < 1) {
			return 0;
		}
	}

	struct radius_request *req = NULL;
	for (size_t i = 0; i < 256; ++i) {
		req = &cl->queue[(cl->queuepos + i) % 256];
		if (!req->packet)
			break;
		req = NULL;
	}

	if (!req || !(req->packet = malloc(pkt->length))) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "RADIUS queue is full!\n");
#else
		syslog(LOG_WARNING, "RADIUS queue is full!");
#endif
		return 0;
	}

	struct radius_pkt *npkt = memcpy(req->packet, pkt, pkt->length);
	npkt->identifier = req - cl->queue;

	radius_finalize(npkt, cl->secret?:"");
	
	req->cb = cb;
	req->ctx = ctx;
	req->timeout = 0;
	req->try = 0;
	req->first = cl->current;

	cl->queuepos = (npkt->identifier + 1) % 256;

	// Rearm timer with value 0 to fire immediately
	cl->timer.value = 0;
	event_ctl(EVENT_TIMER_MOD, &cl->timer);
	return npkt->identifier + 1;
}

int radius_cancel(struct radius_cluster *cl, int handle) {
	handle--;
	if (handle < 0 || handle > 255 || !cl->queue[handle].packet) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "%s: Error removing handle from radius queue (handle %d)", 
			__FUNCTION__, handle);
#else
		syslog(LOG_ERR, "%s: Error removing handle from radius queue (handle %d)", 
			__FUNCTION__, handle);
#endif
		return -1;
	}

	free(cl->queue[handle].packet);
	cl->queue[handle].packet = NULL;
	return 0;
}

int radius_sock(struct radius_cluster *cl) {
	if (cl->count <= 0)
		return -1;
	return cl->sockets[cl->current].fd;
}

static void radius_hash_user_password
(const char *secret, const uint8_t *auth, uint8_t *password, ssize_t len) {
	md5_state_t md5;
	uint8_t hash[16];
	const uint8_t *src = auth;
	size_t secretlen = strlen(secret);

	// RFC 2865 5.2 User-Password
	for (size_t k = 0; (len -= 16) >= 0; src = &password[k++ * 16]) {
		md5_init(&md5);
		md5_append(&md5, (const md5_byte_t*)secret, secretlen);
		md5_append(&md5, src, 16);
		md5_finish(&md5, hash);
		for (size_t i = 0; i < sizeof(hash); ++i)
			password[16 * k + i] ^= hash[i];
	}
}

static void radius_mppe_encrypt(const char *secret, const uint8_t *auth,
		uint8_t *payload, ssize_t len) {
	md5_state_t md5;
	uint8_t hash[16];
	const uint8_t *src = auth;

	uint8_t *salt = payload;
	urandom(salt, 2);
	*salt |= 0x80;

	size_t secretlen = strlen(secret);

	// RFC 2548
	for (size_t k = 0; (len -= 16) >= 0; src = &payload[k++ * 16 + 2]) {
		md5_init(&md5);
		md5_append(&md5, (const md5_byte_t*)secret, secretlen);
		md5_append(&md5, src, 16);
		if (k == 0)
			md5_append(&md5, salt, 2);
		md5_finish(&md5, hash);
		for (size_t i = 0; i < sizeof(hash); ++i)
			payload[16 * k + i + 2] ^= hash[i];
	}
}

static void radius_mppe_decrypt(const char *secret, const uint8_t *auth,
		uint8_t *payload, ssize_t len) {
	md5_state_t md5;
	uint8_t hash[16], iv[16];
	size_t secretlen = strlen(secret);
	memcpy(iv, auth, sizeof(iv));

	// RFC 2548
	for (size_t k = 0; (len -= 16) >= 0; ++k) {
		md5_init(&md5);
		md5_append(&md5, (const md5_byte_t*)secret, secretlen);
		md5_append(&md5, iv, sizeof(iv));
		if (k == 0)
			md5_append(&md5, payload, 2);
		md5_finish(&md5, hash);
		memcpy(iv, &payload[16 * k + 2], sizeof(iv));
		for (size_t i = 0; i < sizeof(hash); ++i)
			payload[16 * k + i + 2] ^= hash[i];
	}
}

static void radius_hmac_authen
(const char *secret, struct radius_pkt *pkt, uint8_t hash[16])
{
	md5_state_t md5;
	uint8_t secretbytes[64] = {0};
#ifdef __SC_BUILD__
    if(secret && strlen(secret))
    {
    }
    else
    {
        return;
    }
#endif
	if (strlen(secret) > 64) {
		md5_init(&md5);
		md5_append(&md5, (uint8_t*)secret, strlen(secret));
		md5_finish(&md5, secretbytes);
	} else {
		memcpy(secretbytes, secret, strlen(secret));
	}

	for (size_t i = 0; i < sizeof(secretbytes); ++i)
		secretbytes[i] ^= 0x36;

	md5_init(&md5);
	md5_append(&md5, secretbytes, sizeof(secretbytes));
	md5_append(&md5, (uint8_t*)pkt, ntohs(pkt->length));
	md5_finish(&md5, hash);

	for (size_t i = 0; i < sizeof(secretbytes); ++i) {
		secretbytes[i] ^= 0x36;
		secretbytes[i] ^= 0x5c;
	}

	md5_init(&md5);
	md5_append(&md5, secretbytes, sizeof(secretbytes));
	md5_append(&md5, hash, 16);
	md5_finish(&md5, hash);
}

struct radius_pkt* radius_pkt_init(void* buffer, uint8_t code) {
	struct radius_pkt *pkt = buffer;
	pkt->code = code;
	pkt->length = 20;
	return pkt;
}


void radius_pkt_append
(struct radius_pkt *pkt, uint8_t type, const void *payload, uint32_t len) {
	uint32_t u32 = htonl(len);
	size_t pad = 0;

	if (!payload) {
		payload = &u32;
		len = 4;
	}

	if (len > 253)
		len = 253;

	struct radius_attr *attr = (struct radius_attr*)
					(((uint8_t*)pkt) + pkt->length);
	attr->type = type;

	if (type == RADIUS_USER_PASSWORD) { //HACKZ!!! (Has to be padded to 16B)
		if (len > 128)
			len = 128;

		if (!len || len % 16) {
			pad = 16 - (len % 16);
			memset(attr->payload + len, 0, pad);
			len += pad;
		}
	}

	pkt->length += (attr->length = len + sizeof(*attr));
	memcpy(attr->payload, payload, len - pad);
}

void radius_pkt_append_vendor(struct radius_pkt *pkt, uint32_t vendor,
		uint8_t type, const void *payload, uint32_t len) {
	union {
		struct radius_vattr va;
		uint8_t buf[256];
	} b;

	uint32_t u32 = htonl(len);
	if (!payload) {
		payload = &u32;
		len = 4;
	}

	if (len > 247)
		len = 247;

	b.va.vtype = type;
	b.va.vendor = htonl(vendor);
	b.va.vlength = len + 2;
	memcpy(b.va.vpayload, payload, len);
	radius_pkt_append(pkt, RADIUS_VENDOR, &b.va, b.va.vlength + 4);
}

int radius_pkt_match(struct radius_attr *attr, uint8_t type, uint32_t vendor) {
	if (!vendor)
		return (attr->type == type)
					? (attr->length - sizeof(*attr)) : 0;

	struct radius_vattr *vattr = (void*)attr->payload;

	return (attr->type == RADIUS_VENDOR && attr->length >= 8
	&& (vattr->vlength == attr->length - sizeof(*attr) - sizeof(vendor))
	&& vattr->vtype == type
	&& vattr->vendor == htonl(vendor))
		? vattr->vlength - sizeof(*attr) : 0;
}

uint32_t radius_pkt_get_u32(struct radius_attr *attr) {
	uint32_t val;
	memcpy(&val, attr->payload, sizeof(val));
	return ntohl(val);
}

static void radius_receiver(struct event_epoll *ev, uint32_t events) {
	struct radius_cluster *cl = ev->context;
	uint8_t buffer[4096];
	md5_state_t md5;
	uint8_t md5_hash[16];
	ssize_t len;

	struct radius_pkt *pkt = (struct radius_pkt*)buffer;
	while ((len = recv(ev->fd, buffer, sizeof(buffer), MSG_DONTWAIT)) >= 0) {
		if (len < 20) // Invalid packet
			continue;
		uint16_t plen = ntohs(pkt->length);
		struct radius_request *req = &cl->queue[pkt->identifier];
		if (plen > len || !req->packet) // Truncated || unknown reply
			continue;

		if (req->packet->code == RADIUS_ACCESS_REQUEST
		&& pkt->code != RADIUS_ACCESS_ACCEPT
		&& pkt->code != RADIUS_ACCESS_CHALLENGE
		&& pkt->code != RADIUS_ACCESS_REJECT)
			continue; // Invalid reply type

		if (req->packet->code == RADIUS_ACCOUNTING_REQUEST
		&& pkt->code != RADIUS_ACCOUNTING_RESPONSE)
			continue; // Invalid reply type

		uint8_t autht[16];
		memcpy(autht, pkt->authenticator, 16);
		memcpy(pkt->authenticator, req->packet->authenticator, 16);

		md5_init(&md5);
		md5_append(&md5, (const void*)pkt, plen);
		md5_append(&md5, (const void*)cl->secret, strlen(cl->secret));
		md5_finish(&md5, md5_hash);

		if (memcmp(autht, md5_hash, sizeof(md5_hash)))
        {
			continue; // Invalid authenticator
        }
		pkt->length = plen;
		struct radius_attr *attr, *eap = NULL, *authen = NULL;

		radius_foreach_attr(attr, pkt) {
			if (attr->type == RADIUS_EAP_MSG)
				eap = attr;
			else if (radius_pkt_match(attr,
					RADIUS_MSG_AUTHENTICATOR, 0) == 16)
				authen = attr;
		}

		if (eap && !authen) {
#ifdef __SC_BUILD__
            log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "RADIUS: discarding unauth'd EAP "
				"reply message %i, Type: %s",
				(int)pkt->identifier, radius_type[pkt->code]);
#else
			syslog(LOG_WARNING, "RADIUS: discarding unauth'd EAP "
				"reply message %i, Type: %s",
				(int)pkt->identifier, radius_type[pkt->code]);
#endif
			continue;
		}

		if (authen) {
			pkt->length = htons(plen);
			memcpy(autht, authen->payload, sizeof(autht));
			memset(authen->payload, 0, sizeof(autht));
			radius_hmac_authen(cl->secret, pkt, authen->payload);
			if (memcmp(autht, authen->payload, sizeof(autht))) {
#ifdef __SC_BUILD__
                log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "RADIUS: dropped packet "
				" %i (type %s) with invalid authenticator",
				(int)pkt->identifier, radius_type[pkt->code]);
#else

				syslog(LOG_WARNING, "RADIUS: dropped packet "
				" %i (type %s) with invalid authenticator",
				(int)pkt->identifier, radius_type[pkt->code]);
#endif
				continue; // Invalid authenticator
			}
			pkt->length = plen;
		}
#ifndef __SC_BUILD__
     	syslog(LOG_INFO, "RADIUS: Received message %i, Type: %s, "
					"Bytes: %i", (int)pkt->identifier,
					radius_type[pkt->code], (int)plen);
#endif
		radius_foreach_attr(attr, pkt) {
			if ((len = radius_pkt_match(attr,
					RADIUS_MS_MPPE_RECV_KEY,
					RADIUS_VENDOR_MS))
					|| (len = radius_pkt_match(attr,
					RADIUS_MS_MPPE_SEND_KEY,
					RADIUS_VENDOR_MS)))
				radius_mppe_decrypt(cl->secret,
						req->packet->authenticator,
						&attr->payload[6], len);
			else if ((len = radius_pkt_match(attr,
					RADIUS_REPLY_MSG, 0)))
            {
#ifndef __SC_BUILD__
				syslog(LOG_INFO, "RADIUS: Reply message %i: "
						"%.*s", (int)pkt->identifier,
						(int)len, attr->payload);
#endif
            }
		}

		// Reply received, invoke callback
		if (req->cb)
			req->cb(pkt, req->ctx);
		free(req->packet);
		req->packet = NULL;
	}
}

static void radius_sender(struct event_timer *timer, int64_t now) {
	struct radius_cluster *cl = timer->context;
	int32_t time = now / 1000, next = INT32_MAX;

	for (size_t i = 0; i < 256; ++i) {
		struct radius_request *c = &cl->queue[i];
		if (!c->packet)
			continue;

		if (c->timeout > time) { // Not yet timed out
			if (c->timeout < next)
				next = c->timeout;
			continue;
		}


		if (c->try >= cl->tries_max) { // Max tries reached
#ifdef __SC_BUILD__
            log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "RADIUS: Request lost!\n");
#else
			syslog(LOG_WARNING, "RADIUS: Request lost!");
#endif
			if (c->cb)
				c->cb(NULL, c->ctx); // Notify the callee with empty response
			free(c->packet);
			c->packet = NULL;
			continue;
		}

		// Send packet to radius server according to strategy
		int socki = (c->first + c->try / cl->tries_server) % cl->count;

		if (socki != c->first && c->first == cl->current)
			cl->current = socki; // Switch to next default server

		int fd = cl->sockets[socki].fd;


		if (send(fd, c->packet, ntohs(c->packet->length),
						MSG_DONTWAIT) < 0) {
			radius_reconfigure(cl, cl->section, cl->port, NULL); // Reopen sockets
			next = time;
			break;
		}
#ifndef __SC_BUILD__
		syslog(LOG_INFO, "RADIUS: Sent message %uc, Type: %s, "
					"Bytes: %i", c->packet->identifier,
					radius_type[c->packet->code],
					(int)ntohs(c->packet->length));
#endif
		c->try++;
		c->timeout = time + cl->timeout;
		if (c->timeout < next)
			next = c->timeout;
	}

	if (next < INT32_MAX) {
		timer->value = ((int64_t)next * 1000) - now;
		event_ctl(EVENT_TIMER_MOD, timer);
	}
}
