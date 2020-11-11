/*
 * ubus - Ubus extesion to control hotspot daemon
 * Copyright (C) 2013 Fon Wireless Ltd.
 *
 * Author:
 * Alejandro Martin <alejandro.martin@fon.com>
 *
 */

#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <syslog.h>
#include <uci.h>
#include <uci_blob.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>


#include "core/hotspotd.h"
#include "core/trigger.h"

#include "lib/config.h"
#include "lib/event.h"

static const char *ubus_socket;
static struct ubus_context *ctx;
static struct uci_context *uci;
static struct uci_package *uci_package;
static struct blob_buf b;
static const char *offline_path, *online_path;

static int __hotspotd_reload();

enum {
	CONF_SECTION,
	CONF_VALUES,
	__CONF_MAX
};

enum {
	CAP,
	__CAP_MAX
};

enum {
	UNSET_SEC,
	UNSET_OPT,
	__UNSET_MAX
};

enum {
	GET_SEC,
	GET_SRC,
	GET_OPT,
	__GET_MAX
};

static const struct blobmsg_policy hotspotd_conf_policy[] = {
	[CONF_SECTION] = { .name = "section", .type = BLOBMSG_TYPE_STRING },
	[CONF_VALUES] = { .name = "values", .type = BLOBMSG_TYPE_TABLE }
};

static const struct blobmsg_policy hotspotd_cap_policy[] = {
	[CAP] = { .name = "cap", .type = BLOBMSG_TYPE_TABLE }
};

static const struct blobmsg_policy hotspotd_unset_policy[] = {
	[UNSET_SEC] = { .name = "section", .type = BLOBMSG_TYPE_STRING },
	[UNSET_OPT] = { .name = "options", .type = BLOBMSG_TYPE_ARRAY }
};

static const struct blobmsg_policy hotspotd_get_policy[] = {
	[GET_SEC] = { .name = "section", .type = BLOBMSG_TYPE_STRING },
	[GET_SRC] = { .name = "src", .type = BLOBMSG_TYPE_STRING },
	[GET_OPT] = { .name = "options", .type = BLOBMSG_TYPE_ARRAY }
};


static void ubus_handle(struct event_epoll *event, uint32_t revent);

static struct event_epoll event_ubus = {
	.fd = -1,
	.events = EPOLLIN | EPOLLET,
	.handler = ubus_handle
};

static int ubus_uci_format_blob(struct blob_attr *v, const char **p)
{
	static char buf[21];
	
	switch (blobmsg_type(v)) {
	case BLOBMSG_TYPE_STRING:
		if (blobmsg_data_len(v) > 1)
			*p = blobmsg_data(v);
		break;
		
	case BLOBMSG_TYPE_INT64:
		snprintf(buf, sizeof(buf), "%"PRIu64, blobmsg_get_u64(v));
		*p = buf;
		break;

	case BLOBMSG_TYPE_INT32:
		snprintf(buf, sizeof(buf), "%u", blobmsg_get_u32(v));
		*p = buf;
		break;

	case BLOBMSG_TYPE_INT16:
		snprintf(buf, sizeof(buf), "%u", blobmsg_get_u16(v));
		*p = buf;
		break;

	case BLOBMSG_TYPE_INT8:
		snprintf(buf, sizeof(buf), "%u", !!blobmsg_get_u8(v));
		*p = buf;
		break;

	default:
		break;
	}
	
	return !!*p;
}

static void ubus_uci_merge_set(struct blob_attr *opt, struct uci_ptr *ptr)
{
	struct blob_attr *cur;
	int rem, err;

	ptr->option = blobmsg_name(opt);

	if (uci_lookup_ptr(uci, ptr, NULL, false))
		return;

	if (blobmsg_type(opt) == BLOBMSG_TYPE_ARRAY) {
		ptr->value = NULL;
		uci_delete(uci, ptr);
		
		blobmsg_for_each_attr(cur, opt, rem)
			if (ubus_uci_format_blob(cur, &ptr->value)) {
				err = uci_add_list(uci, ptr);
				if (err)
					syslog(LOG_ERR, "%s: Error adding option %s to %s list\n",
						__FUNCTION__, ptr->value, ptr->option);
			}
	} else if (ubus_uci_format_blob(opt, &ptr->value)) {
		err = uci_set(uci, ptr);
		if (err) 
			syslog(LOG_ERR, "%s: Error setting option %s\n",
				__FUNCTION__, ptr->value);
	}
}

static int hotspotd_set(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__CONF_MAX], *cur;
	struct uci_ptr ptr = { 0 };
	int rem;

	blobmsg_parse(hotspotd_conf_policy, __CONF_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[CONF_SECTION]) {
		syslog(LOG_ERR, "%s: No section provided\n", __FUNCTION__);
		return UBUS_STATUS_INVALID_ARGUMENT;
	}
	
	ptr.package = "hotspotd";
	ptr.section = blobmsg_data(tb[CONF_SECTION]);

	if (!uci_lookup_section(uci, uci_package, ptr.section)) {
		struct uci_ptr ptr_section = {
			.p = uci_package,
			.value = ptr.section
		};
		uci_add_section(uci, uci_package, ptr.section, &(ptr_section.s));
		uci_rename(uci, &ptr_section);
	}
	
	if (!tb[CONF_VALUES])
		goto exit;

	blobmsg_for_each_attr(cur, tb[CONF_VALUES], rem)
		ubus_uci_merge_set(cur, &ptr);

exit:
	if (uci_lookup_ptr(uci, &ptr, NULL, false))
		return UBUS_STATUS_UNKNOWN_ERROR;
	
	return uci_commit(uci, &ptr.p, false);
}

static int hotspotd_set_cap(struct ubus_context *ctx, 
		struct ubus_object *obj, struct ubus_request_data *req, 
		const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__CAP_MAX], *_grantee, *_serv;
	int remg, rems;

	blobmsg_parse(hotspotd_cap_policy, __CAP_MAX, tb, blob_data(msg),
			blob_len(msg));

	if(!tb[CAP]) {
		return UBUS_STATUS_INVALID_ARGUMENT;	
	}

	blobmsg_for_each_attr(_grantee, tb[CAP], remg) {
		const char *grantee = blobmsg_name(_grantee);
		if(!config_section_exists(grantee)) {
			syslog(LOG_ERR, "%s: section %s does not exist", __FUNCTION__,
					grantee);
			return UBUS_STATUS_INVALID_ARGUMENT;
		}
		blobmsg_for_each_attr(_serv, _grantee, rems) {
			const char *serv = blobmsg_name(_serv);
			if(!config_section_exists(serv)) {
				syslog(LOG_ERR, "%s: section %s does not exist",
						__FUNCTION__, serv);
				return UBUS_STATUS_INVALID_ARGUMENT;
			}
			if(!config_get_bool(serv, "locked", true)) {
				config_set_bool(grantee, serv, blobmsg_get_bool(_serv));
			}
		}
	}
	if(config_commit()) {
		return UBUS_STATUS_UNKNOWN_ERROR;
	}
	__hotspotd_reload();
	return UBUS_STATUS_OK;
}

static int hotspotd_unset(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__UNSET_MAX], *cur;
	struct uci_ptr ptr = { 0 };
	int rem, err = 0;
	
	blobmsg_parse(hotspotd_unset_policy, __UNSET_MAX, tb, blob_data(msg), blob_len(msg));
	
	if (!tb[UNSET_SEC]) {
		syslog(LOG_ERR, "No section or params provided\n");
		return UBUS_STATUS_INVALID_ARGUMENT;
	}
	
	ptr.package = "hotspotd";
	ptr.section =  blobmsg_get_string(tb[UNSET_SEC]);

	if (!tb[UNSET_OPT]) {
		uci_delete(uci, &ptr);
	} else {
	       	blobmsg_for_each_attr(cur, tb[UNSET_OPT], rem) {
			ptr.option = blobmsg_data(cur);
			ptr.value = NULL;
			
			err = uci_delete(uci, &ptr);
			if (err) {
				syslog(LOG_ERR, "Error trying to unset %s\n", ptr.section);
			}
		}
	}

	if (uci_lookup_ptr(uci, &ptr, NULL, false))
		return UBUS_STATUS_UNKNOWN_ERROR;
	
	return uci_commit(uci, &ptr.p, false);
}

static int hotspotd_get(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__GET_MAX], *cur;
	struct blobmsg_policy *option_policy = NULL, *index = NULL;
	struct uci_blob_param_list param_list;
	const char *section, *option = NULL;
	int rem, nopts = 0, err = 0;
	bool src = true;
	const char *src_str = NULL;

	blobmsg_parse(hotspotd_get_policy, __GET_MAX, tb, blob_data(msg), blob_len(msg));
	
	if (!tb[GET_SEC]) {
		syslog(LOG_ERR, "%s: No section or params provided\n", __FUNCTION__);
		return UBUS_STATUS_INVALID_ARGUMENT;
	}
	section = blobmsg_get_string(tb[GET_SEC]);
	
	if (!uci_lookup_section(uci, uci_package, section)) {
		syslog(LOG_DEBUG, "%s: Trying to get an unexistent section %s\n", 
			__FUNCTION__, section);
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	if (tb[GET_SRC]) {
		/*
		 * The data source is actual load config, otherwise
		 * the source is /etc/config/hotspotd file
		 */

		src_str = blobmsg_get_string(tb[GET_SRC]);
		if (!strcmp(src_str, "actual")) 
			src = false;
	}
	 
	if (!tb[GET_OPT]) {
		param_list.params = NULL;
		param_list.n_params = 0;
	} else {
		blobmsg_for_each_attr(cur, tb[GET_OPT], rem) 
			nopts++;

		option_policy = (struct blobmsg_policy *)malloc(sizeof(struct blobmsg_policy) * nopts);
		if (!option_policy) {
			syslog(LOG_ERR, "%s: Error getting memory\n", __FUNCTION__);
			return UBUS_STATUS_UNKNOWN_ERROR;
		}
	
		index = option_policy;
		blobmsg_for_each_attr(cur, tb[GET_OPT], rem) {
			option = blobmsg_get_string(cur);
			index->name = option;
			index++;
		}
		param_list.params = option_policy; 
		param_list.n_params = nopts;
	}
	
	blob_buf_init(&b, 0);
	
	if (config_dump_section(&b, section, param_list, src)) {
		syslog(LOG_ERR, "%s: Invalid parameter\n", __FUNCTION__);
		err = UBUS_STATUS_INVALID_ARGUMENT;
		goto exit;
	}
	
        ubus_send_reply(ctx, req, b.head);
exit:
	if (option_policy)
		free(option_policy);
	return err;
}

static void __hotspotd_offline()
{
	struct trigger_handle *hndl = NULL;
	
	if (status_online) {
		hndl = trigger_run(offline_path, NULL, NULL, NULL, NULL);
		if (hndl)
			trigger_wait(hndl);
		syslog(LOG_WARNING, "%s: status => offline", __FUNCTION__);
	}
}

static int hotspotd_offline(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	__hotspotd_offline();
	return 0;
}

static int hotspotd_online(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct trigger_handle *hndl = NULL;

	if (!status_online) {
		hndl = trigger_run(online_path, NULL, NULL, NULL, NULL);
		if (hndl)
			trigger_wait(hndl);
		syslog(LOG_WARNING, "%s: status => online", __FUNCTION__);
	}
	return 0;
}

static int hotspotd_deploy(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	const char *deploy = config_get_string("main", "deploy", "false");
	if (strcmp(deploy, "true")) {
		config_set_string("main", "deploy", "true");
		config_commit();
		hotspot_control(HOTSPOT_CTRL_APPLY);
	}

	return 0;
}

static int hotspotd_init_uci()
{
	uci = uci_alloc_context();
	if (!uci)
		return UBUS_STATUS_UNKNOWN_ERROR;
	
	uci_load(uci, "hotspotd", &uci_package);
	if (!uci_package)
		return UBUS_STATUS_UNKNOWN_ERROR;

	return 0;
}

static void hotspotd_deinit_uci()
{
	if (uci) {
		uci_free_context(uci);
		uci = NULL;
	}
	
	if (uci_package) {
		uci_unload(uci, uci_package);
		uci_package = NULL;
	}
}

static int __hotspotd_reload() 
{	
	config_init("hotspotd");

	hotspotd_deinit_uci();
	if (hotspotd_init_uci())
		return UBUS_STATUS_UNKNOWN_ERROR;
	
	hotspot_control(HOTSPOT_CTRL_APPLY);
	return 0;
}

static int hotspotd_reload(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	return __hotspotd_reload();
}

static const struct ubus_method hotspotd_methods[] = {
	UBUS_METHOD("set", hotspotd_set, hotspotd_conf_policy),
	UBUS_METHOD("set_capabilities", hotspotd_set_cap, hotspotd_cap_policy),
	UBUS_METHOD("unset", hotspotd_unset, hotspotd_unset_policy),
	UBUS_METHOD("get", hotspotd_get, hotspotd_get_policy),
	{ .name = "set_offline", .handler = hotspotd_offline },
	{ .name = "set_online", .handler = hotspotd_online },
	{ .name = "reload", .handler = hotspotd_reload },
	{ .name = "deploy", .handler = hotspotd_deploy }
};

static struct ubus_object_type hotspotd_object_type =
	UBUS_OBJECT_TYPE("hotspotd", hotspotd_methods);

static struct ubus_object hotspotd_object = {
	.name = "hotspotd",
	.type = &hotspotd_object_type,
	.methods = hotspotd_methods,
	.n_methods = ARRAY_SIZE(hotspotd_methods)
};

static void ubus_clear()
{
	if (event_ubus.fd != -1)
		event_ctl(EVENT_EPOLL_DEL, &event_ubus);

	hotspotd_deinit_uci();

	if (ctx) {
		ubus_remove_object(ctx, &hotspotd_object);
		ubus_free(ctx);
		ctx = NULL;
	}
}

static void ubus_deinit() 
{
	__hotspotd_offline();
	ubus_clear();
}

static int ubus_apply() 
{
	online_path = config_get_string("trigger", "hotspot_online", 
					"/sbin/hotspot-up");
	offline_path = config_get_string("trigger", "hotspot_offline", 
					"/sbin/hotspot-down");

	const char *enable = config_get_string("main", "enable", "false");
	const char *deploy = config_get_string("main", "deploy", "false");
	if ((!strcmp(enable, "true") || !strcmp(enable, "1")) &&
			(!strcmp(deploy, "true") || !strcmp(deploy, "1")))
		hotspotd_online(NULL, NULL, NULL, NULL, NULL);
	else
		__hotspotd_offline();

	return 0;
}

static int ubus_init() 
{
	ubus_clear();

	/*
	 * if the param is NULL the ubus connects to default socket
	 * which is defined by UBUS_UNIX_SOCKET (/var/run/ubus.sock)
	 */

	ctx = ubus_connect(NULL);
	if (!ctx) {
		syslog(LOG_ERR, "Failed to connect to ubus socket %s\n", ubus_socket);
		return -1;
	}

	event_ubus.fd = ctx->sock.fd;
	event_ctl(EVENT_EPOLL_ADD, &event_ubus);

	if (hotspotd_init_uci())
		return UBUS_STATUS_UNKNOWN_ERROR;

	ubus_add_object(ctx, &hotspotd_object);
	
	return ubus_apply();
}

static void ubus_handle(struct event_epoll *event, uint32_t revent) 
{
	ubus_handle_event(ctx);
}


RESIDENT_REGISTER(ubus, 190)

