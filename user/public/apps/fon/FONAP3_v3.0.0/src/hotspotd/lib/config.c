/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

// UCI API simplification module
#include <uci.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "config.h"

static struct uci_context *uci = NULL;
static char *package = NULL;

static struct uci_package *uci_package;
#ifdef HOTSPOTD_UBUS
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
static struct uci_blob_param_list _param_list;

int config_dump_section(struct blob_buf *b, const char *sec, 
			const struct uci_blob_param_list param_list, bool src)
{
	void *c, *d;
	struct uci_section *s;
	struct uci_element *e, *ee;
	struct uci_option *o;
	struct blobmsg_policy *index;
	struct uci_package *p = NULL;
	struct uci_context *ctx = NULL;
	int i;

	_param_list.params = param_list.params;
	_param_list.n_params = param_list.n_params;

	if (src) {
		// Reload the config file data
		ctx = uci_alloc_context();
		uci_load(ctx, "hotspotd", &p);
	} else {
		// Actual loaded data
		ctx = uci;
		p = uci_package;
	}
	
	if (!(s = uci_lookup_section(ctx, p, sec))) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Uci lookup section error\n");
#else

		syslog(LOG_ERR, "%s: Uci lookup section error\n",
			__FUNCTION__);
#endif
		return -1;
	}

	c = blobmsg_open_table(b, sec);
	
	uci_foreach_element(&s->options, e) {
		o = uci_to_option(e);

		if (_param_list.n_params != 0) {
			index = (struct blobmsg_policy*)_param_list.params;
			for (i = 0; i < _param_list.n_params; i++) {
				if (!strcmp(index->name,  o->e.name)) {
					// Option available
					break;
				}
				index++;
			}
			
			if (i == _param_list.n_params) // Not option selected
				continue;
		}
		
		switch (o->type) {
		case UCI_TYPE_STRING:
			blobmsg_add_string(b, o->e.name, o->v.string);
			break;
		case UCI_TYPE_LIST:
			d = blobmsg_open_array(b, o->e.name);
			uci_foreach_element(&o->v.list, ee) {
				blobmsg_add_string(b, NULL, ee->name);
			}
			blobmsg_close_array(b, d);
			break;
		default:
			break;
		}
	}

	blobmsg_close_table(b, c);

	if (src) {
		uci_free_context(ctx);
	}
	
	return 0;
}
#endif

int config_section_exists(const char *name) {
	return !!config_get_string(name, NULL, NULL);
}

int config_init(const char *name) {
	if (!name && package)
		name = strdupa(package);

	config_deinit();
	if (!(package = strdup(name))
	|| !(uci = uci_alloc_context()) || uci_load(uci, package, &uci_package))
		return -1;

	return 0;
}

void config_deinit() {
	if (uci) {
		uci_free_context(uci);
		uci = NULL;
	}
	if (package) {
		free(package);
		package = NULL;
	}
}

const char* config_get_string(const char *sec, const char *opt, const char *def) {
	struct uci_ptr ptr = {
		.package = package,
		.section = sec,
		.option = opt
	};
	if (!uci_lookup_ptr(uci, &ptr, NULL, false)
	&& (ptr.flags & UCI_LOOKUP_COMPLETE)) {
		struct uci_element *e = ptr.last;
		if (e->type == UCI_TYPE_SECTION) {
			return uci_to_section(e)->type;
		} else if (e->type == UCI_TYPE_OPTION
		&& ptr.o->type == UCI_TYPE_STRING) {
			return ptr.o->v.string;
		}
	}
	return def;
}

bool config_get_bool(const char *sec, const char *opt, bool def) {
	const char *val = config_get_string(sec, opt, NULL);
	if(!val)
		return def;
	if(!strcmp(val, "true") || (atoi(val) == 1)) {
		return true;
	}
	if(!strcmp(val, "false") || !atoi(val)) {
		return false;
	}
	return def;
}

int config_foreach_list(const char *sec, const char *opt, void(*cb)(const char*, void*), void *ctx) {
	struct uci_ptr ptr = {
		.package = package,
		.section = sec,
		.option = opt
	};

	if (uci_lookup_ptr(uci, &ptr, NULL, false)
	|| !(ptr.flags & UCI_LOOKUP_COMPLETE)
	|| ptr.last->type != UCI_TYPE_OPTION
	|| ptr.o->type != UCI_TYPE_LIST)
		return -1;

	struct uci_element *e;
	uci_foreach_element(&ptr.o->v.list, e)
		cb(e->name, ctx);

	return 0;
}

int config_foreach_section(const char *type, void(*cb)(const char *, void*), void *ctx) {
	struct uci_element *e;
	unsigned int all = 0;

	if(!type)
		all = 1;

	uci_foreach_element(&uci_package->sections, e) {
		struct uci_section *s = uci_to_section(e);
		if (all || !strcmp(s->type, type)) {
			cb(s->e.name, ctx);
		}
	}
	return 0;
}

int config_get_int(const char *sec, const char *opt, int def) {
	const char *val = config_get_string(sec, opt, NULL);
	return (!val) ? def : atoi(val);
}

int config_get_ipv4(struct in_addr *addr, const char *sec, const char *opt) {
	struct in_addr tmpaddr;
	const char *val = config_get_string(sec, opt, NULL);
	if (!val || inet_pton(AF_INET, val, &tmpaddr) < 0)
		return -1;

	*addr = tmpaddr;
	return 0;
}

int config_get_ipv6(struct in6_addr *addr, const char *sec, const char *opt) {
	struct in6_addr tmpaddr;
	const char *val = config_get_string(sec, opt, NULL);
	if (!val || inet_pton(AF_INET6, val, &tmpaddr) < 0)
		return -1;

	*addr = tmpaddr;
	return 0;
}

static int config_set(const char *sec, const char *opt, const char *val, int list) {
	struct uci_ptr ptr = {
		.package = package,
		.section = sec,
		.option = opt,
		.value = val
	};
	if (uci_lookup_ptr(uci, &ptr, NULL, false)) {
		return -1;
	} else if (list) {
		return uci_add_list(uci, &ptr);
	} else if (val) {
		return uci_set(uci, &ptr);
	} else {
		return uci_delete(uci, &ptr);
	}
}

int config_add_string(const char *sec, const char *opt, const char *val) {
	return (!val) ? -1 : config_set(sec, opt, val, 1);
}

int config_set_string(const char *sec, const char *opt, const char *val) {
	return config_set(sec, opt, val, 0);
}

int config_set_bool(const char *sec, const char *opt, bool val) {
	char buf[2];
	sprintf(buf, "%d", val);
	return config_set_string(sec, opt, buf);
}

int config_commit() {
	struct uci_ptr ptr = { .package = package };
	if (uci_lookup_ptr(uci, &ptr, NULL, false))
		return -1;
	return uci_commit(uci, &ptr.p, false);
}
