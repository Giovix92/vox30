/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 */

#define __STDC_FORMAT_MACROS
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include "lib/config.h"
#include "lib/list.h"

#include "core/client.h"
#include "core/firewall.h"
#include "core/hotspotd.h"
#include "core/neigh.h"
#include "core/routing.h"

uint64_t next_mask = 0;
static char *cap = NULL;
bool mdnsed = false;

struct service {
	struct list_head _head;
	uint64_t mask;
	char *name;
	char **str;
	unsigned int strnum;
	int proto;
	char *port;
	bool mdns;
};

struct grantee {
	struct list_head _head;
	uint64_t servicemap;
	char *name;
};

struct list_head services;
struct list_head grantees;

struct build_cap_args {
	char *cap;
	size_t len;
};

void service_build_cap(struct service *s, void *ctx) 
{
	if(!s || !ctx) {	
		return;
	}

	struct build_cap_args *args = (struct build_cap_args *)ctx;
	for(int ii = 0; ii < s->strnum; ii++) {
		if(s->str[ii]) {
			if(!(args->cap = (char *)realloc(args->cap,
					args->len+strlen(s->str[ii])+1))) {
#ifndef __SC_BUILD__
				syslog(LOG_ERR, "%s: error in realloc", __FUNCTION__);
#endif
				return;
			}
			memcpy(args->cap+args->len, s->str[ii], strlen(s->str[ii]));
			args->len += strlen(s->str[ii]) + 1;
			memset(args->cap+args->len-1, '|', 1);
		}
	}
}

void service_finish_cap(struct build_cap_args *args)
{
	if(args && args->cap && args->len) {
		args->cap[args->len-1] = '\0';
	}
}

static void free_service(struct service *e)
{
	if(!e) {


		syslog(LOG_ERR, "no service to free");
		return;
	}

	list_del(&e->_head);
	free(e->name);
	for (int ii = 0; ii < e->strnum; ii++) {
		free(e->str[ii]);
	}
	free(e->str);
	free(e);
}

static void free_services() 
{
	while (!list_empty(&services)) {
		struct service *e = list_first_entry(&services, struct service, _head);
		free_service(e);
	}
	next_mask = 0;
}

static void free_grantees()
{
	while (!list_empty(&grantees)) {
		struct grantee *e = list_first_entry(&grantees, struct grantee, _head);
		list_del(&e->_head);
		free(e->name);
		free(e);
	}
}

static void service_add_str(const char *str, void *ctx)
{
	if(!ctx) {
		return;
	}

	struct service *serv = (struct service *)ctx;
	if(str) {
		serv->strnum++;
		if(!(serv->str = (char **)realloc(serv->str, serv->strnum))) {
#ifndef __SC_BUILD__
			syslog(LOG_ERR, "%s: error in realloc", __FUNCTION__);
#endif
			free_service(serv);
			return;
		}
		if(!(serv->str[serv->strnum-1] = (char *)malloc(strlen(str)+1))) {
#ifndef __SC_BUILD__
			syslog(LOG_ERR, "%s: error in malloc", __FUNCTION__);
#endif
			free_service(serv);
			return;
		}
		strncpy(serv->str[serv->strnum-1], str, strlen(str)+1);
#ifndef __SC_BUILD__
		syslog(LOG_DEBUG, "%s: registered string %d \"%s\" for service %s",
			__FUNCTION__, serv->strnum, str, serv->name);
#endif
	} 
}

static void service_add_service(const char *name, void *ctx)
{
	if(!name) {
		return;
	}

	struct service *serv = NULL;
	if(!(serv= (struct service *)malloc(sizeof(struct service)))) {
#ifndef __SC_BUILD__
		syslog(LOG_ERR, "%s: error in malloc", __FUNCTION__);
#endif
		return;
	}
	next_mask = next_mask?(next_mask << 1):1;
	serv->mask = next_mask;
	if(!(serv->name = (char *)malloc(strlen(name)+1))) {
#ifndef __SC_BUILD__
		syslog(LOG_ERR, "%s: error in malloc", __FUNCTION__);
#endif
		free(serv);
		return;
	}
	serv->strnum = 0;
	serv->str = NULL;
	strncpy(serv->name, name, strlen(name)+1);

	config_foreach_list(name, "name", service_add_str, serv);

	if(!serv->str) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "no cap strings for service %s",
				serv->name);
#else
		syslog(LOG_ERR, "%s: no cap strings for service %s", __FUNCTION__,
				serv->name);
#endif
	}
	const char *port = config_get_string(name, "port", NULL);
	if(port) {
		if(!(serv->port = (char *)malloc(strlen(port)+1))) {
#ifndef __SC_BUILD__
			syslog(LOG_ERR, "%s: error in malloc", __FUNCTION__);
#endif
			free_service(serv);
			return;
		}
		strncpy(serv->port, port, strlen(port)+1);
	} else {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "no port for service %s\n", serv->name);
#else
		syslog(LOG_ERR, "%s: no port for service %s", __FUNCTION__, serv->name);
#endif
		serv->port = NULL;
	}

	serv->mdns = config_get_bool(name, "mdns", false);

	const char *proto = config_get_string(name, "proto", NULL);
	if(proto) {
		if(!strcmp(proto, "tcp")) {
			serv->proto = IPPROTO_TCP;
		} else if(!strcmp(proto, "udp")) {
			serv->proto = IPPROTO_UDP;
		}
	} else {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "unknown proto for service %s\n", serv->name);
#else
		syslog(LOG_ERR, "%s: unknown proto for service %s", __FUNCTION__, serv->name);
#endif
		serv->proto = IPPROTO_NONE;
	}

	if(serv->proto != IPPROTO_NONE && serv->port && serv->str) {
		list_add(&serv->_head, &services);
#ifndef __SC_BUILD__
		syslog(LOG_DEBUG, "%s: new service %s, proto %s, port %s, mask 0x%"
				PRIu64, __FUNCTION__, serv->name, proto, serv->port, serv->mask);
#endif
	} else {
		free_service(serv);
	}
}

/* Find service using name unless name == NULL, then search mask */
static void service_find_service(struct service **serv, const char *name, uint64_t mask)
{	
	struct service *s = NULL;

	list_for_each_entry(s, &services, _head) {
		if(name && !strcmp(s->name, name)) {
			*serv = s;
			return;
		} else if(!name && s->mask == mask) {
			*serv = s;
			return;
		}
	}	
}

static void servicemap_foreach_service(uint64_t servicemap, void(*cb)(struct service *s, void*), void *ctx)
{
	struct service *s = NULL;
	uint64_t servmask = 1;
	while(servicemap) {
		if((servicemap | servmask) == servicemap) {		// service present
			service_find_service(&s, NULL, servmask);
			cb(s, ctx);
		}
		servicemap ^= servmask;	 // Remove processed service from local map
		servmask <<= 1;
	}
}

static void service_add_to_map(const char *service, uint64_t *map)
{
	if(!map) {
		return;
	}
	struct service *s = NULL;

	service_find_service(&s, service, 0);
	if(s) {
		*map |= s->mask;
	}	
}

static uint64_t _service_get_map(const char *type)
{
	uint64_t map = 0;
	struct service *s;
	if(type) {
		list_for_each_entry(s, &services, _head) {
			if(config_get_bool(type, s->name, false)) {
				service_add_to_map(s->name, &map);
			}
		}
	}
	return map;
}

static void service_add_grantee(const char *name, void *ctx)
{
	if(!name) {
		return;
	}
	struct grantee *grantee = NULL;
	if(!(grantee = (struct grantee *)malloc(sizeof(struct grantee)))) {
#ifndef __SC_BUILD__
		syslog(LOG_ERR, "%s: error in malloc", __FUNCTION__);
#endif
		return;
	}
	if(!(grantee->name = (char *)malloc(strlen(name)+1))) {
#ifndef __SC_BUILD__
		syslog(LOG_ERR, "%s: error in malloc", __FUNCTION__);
#endif
		free(grantee);
		return;
	}
	strncpy(grantee->name, name, strlen(name)+1);
	grantee->servicemap = _service_get_map(name);
	list_add(&grantee->_head, &grantees);
#ifndef __SC_BUILD__
	syslog(LOG_DEBUG, "%s: new grantee %s with servicemap 0x%" PRIu64,
			__FUNCTION__, grantee->name, grantee->servicemap);
#endif
}

static int service_apply()
{
	free_services();
	free_grantees();
	if(cap) {
		free(cap);
	}
	INIT_LIST_HEAD(&services);
	INIT_LIST_HEAD(&grantees);
	config_foreach_section("service", service_add_service, NULL);
	config_foreach_section("grantee", service_add_grantee, NULL);

	struct service *s = NULL;
	struct build_cap_args _args;
	_args.cap = NULL;
	_args.len = 0;
	list_for_each_entry(s, &services, _head) {
		service_build_cap(s, &_args);
	}
	service_finish_cap(&_args);
	cap = _args.cap;
#ifndef __SC_BUILD__
	syslog(LOG_DEBUG, "%s: saved router cap = %s", __FUNCTION__, cap);
#endif
	return 0;
}

static int service_init()
{
	INIT_LIST_HEAD(&services);
	INIT_LIST_HEAD(&grantees);
	return service_apply();
}
	
static void service_deinit()
{
	free_services();
	free_grantees();
	if(cap) {
		free(cap);
	}
	cap = NULL;
}

uint64_t service_get_map(const char *type)
{
	if(type) {
		struct grantee *g;
		list_for_each_entry(g, &grantees, _head) {
			if(!strcmp(g->name, type)) {
#ifndef __SC_BUILD__
				syslog(LOG_DEBUG, "%s: found servicemap %" PRIu64
						" for grantee %s", __FUNCTION__, g->servicemap, type);
#endif
				return g->servicemap;
			}
		}
	}
	syslog(LOG_DEBUG, "%s: grantee %s not found, return servicemap 0",
			__FUNCTION__, type);
	return 0;
}

char *service_get_cap(bool all, uint64_t servicemap)
{
	if(!all) {
		struct build_cap_args _args;
		_args.cap = NULL;
		_args.len = 0;
		servicemap_foreach_service(servicemap, service_build_cap, &_args); 
		service_finish_cap(&_args);
#ifndef __SC_BUILD__
		syslog(LOG_DEBUG, "%s: return custom cap = %s", __FUNCTION__,
				_args.cap);
#endif
		return _args.cap;
	}
#ifndef __SC_BUILD__
	syslog(LOG_DEBUG, "%s: return global cap = %s", __FUNCTION__, cap);
#endif
	return cap;
}

struct open_ports_args {
	bool open;
	uint32_t id;
};

void service_open_ports(struct service *s, void *ctx)
{
	if(!s || !ctx) {
		return;
	}

	struct open_ports_args *args = (struct open_ports_args *)ctx;
	if(s->port) {
		// default proto tcp
		if(args->open) {
#ifndef __SC_BUILD__
			syslog(LOG_DEBUG, "%s: opening port %s for service %s and"
					" userid %d", __FUNCTION__, s->port, s->name, args->id);
#endif
		}
		firewall_set_client_service(args->id,
			(s->proto==IPPROTO_NONE)?IPPROTO_TCP:s->proto, s->port,
			args->open);

		// Allow mdns queries for this client if any service requires it
		if(args->open && !mdnsed && s->mdns) {
			firewall_set_client_service(args->id,
				IPPROTO_UDP, "5353", true); 
#ifndef __SC_BUILD__     
			syslog(LOG_DEBUG, "%s: opening port 5353 for service mdns and"
					" userid %d", __FUNCTION__, args->id);
#endif
			mdnsed = true;
		} else if(!args->open) {
			firewall_set_client_service(args->id,
				IPPROTO_UDP, "5353", false); 
		}
	}
		
}

void service_set_services(const uint32_t id, uint64_t servicemap, bool open)
{
	struct open_ports_args _args;
	_args.open = open;
	_args.id = id;

	mdnsed = false;
	servicemap_foreach_service(servicemap, service_open_ports, &_args);
	return;
}

struct client_serv serv = {
	.get_servicemap = service_get_map,
	.set_services = service_set_services,
	.get_cap = service_get_cap,
};

MODULE_REGISTER(service, 410)
SERVICES_REGISTER(serv)
