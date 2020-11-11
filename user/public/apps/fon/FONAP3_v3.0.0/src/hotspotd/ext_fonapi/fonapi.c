/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h>

#include "core/hotspotd.h"
#include "core/trigger.h"
#include "core/client.h"
#ifndef __SC_BUILD__
#include "core/routing.h"
#include "core/traffic.h"
#endif
#include "lib/list.h"
#include "lib/config.h"
#include "lib/hexlify.h"
#include "lib/event.h"
static struct list_head handles = LIST_HEAD_INIT(handles);
static const char *cfg_cmd = NULL;

struct fonapi_handle {
	struct list_head head;
	struct client *client;
	client_update_cb *update;
	struct trigger_handle *handle;
	bool noacct;
};


static int fonapi_apply()
{
	cfg_cmd = config_get_string("fonapi", "trigger", NULL);
	return 0;
}

static int fonapi_init()
{
	fonapi_apply();
	char *argv[] = {"fon-startup", NULL};
	trigger_run(cfg_cmd, argv, NULL, NULL, NULL);
	return 0;
}

static void fonapi_deinit()
{
	char *argv[] = {"fon-shutdown", NULL};
	trigger_run(cfg_cmd, argv, NULL, NULL, NULL);
}




// Propagate environment
static char** fonapi_env_init(char **env, struct client *cl, char buffer[]) {
	//TODO: FRAMED_IP_ADDRESS

	if (cl->sess.username) {
		*env++ = buffer;
		buffer += sprintf(buffer, "USER_NAME=%.255s",
					cl->sess.username);
	}

	*env++ = ++buffer;
	buffer += sprintf(buffer, "SESSION_TIMEOUT=%u",
			(unsigned)cl->sess.limit_time_conn);

	*env++ = ++buffer;
	buffer += sprintf(buffer, "IDLE_TIMEOUT=%u",
			(unsigned)cl->limit_time_idle);

	*env++ = ++buffer;
	buffer += sprintf(buffer,
			"CALLING_STATION_ID=%02X-%02X-%02X-%02X-%02X-%02X",
			cl->mac[0], cl->mac[1], cl->mac[2],
			cl->mac[3], cl->mac[4], cl->mac[5]);

	*env++ = ++buffer;
	buffer += sprintf(buffer, "CALLED_STATION_ID=%s",
			config_get_string("main", "nasid", NULL));

	*env++ = ++buffer;
	buffer += sprintf(buffer, "WISPR_LOCATION_ID=%.255s",
			config_get_string("main", "location_id", ""));

	*env++ = ++buffer;
	buffer += sprintf(buffer, "WISPR_LOCATION_NAME=%.255s",
			config_get_string("main", "location_name", ""));

	*env++ = ++buffer;
	buffer += sprintf(buffer, "NAS_ID=%.64s",
			config_get_string("main", "nasid", ""));

	if (cl->sess.cui) {
		*env++ = ++buffer;
		buffer += sprintf(buffer, "CUI=%.*s", cl->sess.cui_len,
			(char*)cl->sess.cui);
	}

	*env++ = ++buffer;
	buffer += sprintf(buffer, "ACCT_SESSION_ID=%.8x",
					(unsigned)cl->sess.id);

	*env++ = ++buffer;
	buffer += sprintf(buffer, "ACCT_INTERIM_INTERVAL=%u",
			(unsigned)cl->sess.interim_interval);

	*env++ = ++buffer;
	buffer += sprintf(buffer, "CHILLISPOT_MAX_INPUT_OCTETS=%u",
				(unsigned)cl->sess.limit_bytes_in);

	*env++ = ++buffer;
	buffer += sprintf(buffer, "CHILLISPOT_MAX_OUTPUT_OCTETS=%u",
				(unsigned)cl->sess.limit_bytes_out);

	*env++ = ++buffer;
	buffer += sprintf(buffer, "CHILLISPOT_MAX_TOTAL_OCTETS=%u",
				(unsigned)cl->sess.limit_bytes_total);

	if (cl->status == CLSTAT_AUTHED) {
		// Generate live stats
		int64_t now = event_time();
		unsigned stime = now - cl->sess.time_start;
		unsigned itime = now - cl->time_last_touched;

		uint64_t byte_in, byte_out;
		uint32_t pi, po;

		cl->sess.stats_cb(client_id(cl), &byte_in, &byte_out, &pi, &po);
		*env++ = ++buffer;
		buffer += sprintf(buffer, "INPUT_OCTETS=%llu",
						(unsigned long long)byte_in);

		*env++ = ++buffer;
		buffer += sprintf(buffer, "OUTPUT_OCTETS=%llu",
						(unsigned long long)byte_out);

		*env++ = ++buffer;
		buffer += sprintf(buffer, "SESSION_TIME=%u", stime);

		*env++ = ++buffer;
		buffer += sprintf(buffer, "IDLE_TIME=%u", itime);
	} else if (cl->status == CLSTAT_LOGIN) {
		if (cl->sess.key && cl->sess.keylen <= 256) {
			*env++ = ++buffer;
			buffer += sprintf(buffer, "PASSWORD=%.*s", (int)cl->sess.keylen, cl->sess.key);
		}

		*env++ = ++buffer;
		buffer += sprintf(buffer, "SESSION_CHALLENGE=");
		hexlify(buffer, cl->challenge, sizeof(cl->challenge), 0);
		buffer += sizeof(cl->challenge) * 2 + 1;
	}

	*env = NULL;
	return env;
}

// This is called when a trigger is finished
static void fonapi_authcb(void *ctx, short code, short status) {
	struct fonapi_handle *hnd = ctx;

	if (code != CLD_EXITED || (status && status < _LOGIN_FAIL_MIN))
		status = LOGIN_ERROR;
#ifndef __SC_BUILD__
	syslog(LOG_INFO, "fonapi reply for user %i: %i %i",
			client_id(hnd->client), (int)code, (int)status);
#endif
	hnd->handle = NULL;
	hnd->update(hnd->client, status, NULL);
}

// Generic handler
static void fonapi_handler(struct client *cl, enum client_req req,
		client_update_cb *update, void **vctx)
{
	struct fonapi_handle *hnd = *vctx;
	char buffer[2048] = {0}, *env[32];
	char **eenv = fonapi_env_init(env, cl, buffer);

	if (req == CLIENT_LOGIN) {
		if (cl->sess.method != CLIENT_PAP
				|| !(hnd = malloc(sizeof(*hnd)))) {
			update(cl, LOGIN_ERROR, NULL);
			return;
		}

		char *argv[] = {"login", cl->sess.username,
				(char*)cl->sess.key, NULL};
		hnd->handle = trigger_run(cfg_cmd, argv, env,
				fonapi_authcb, hnd);

		if (!hnd->handle) { // Failed to execute trigger
			free(hnd);
			update(cl, LOGIN_ERROR, NULL);
		} else { // Save trigger handle
			hnd->client = cl;
			hnd->update = update;
			hnd->noacct = false;
			list_add(&hnd->head, &handles);
			*vctx = hnd;
		}
	} else if (req == CLIENT_LOGOUT && hnd) { // If login is still in progress
		if (hnd->handle)
			trigger_cancel(hnd->handle);
		list_del(&hnd->head);
		free(hnd);
		*vctx = NULL;
	} else if (!hnd->noacct && (req == CLIENT_ACCOUNTING_START
			|| req == CLIENT_ACCOUNTING_STOP
			|| req == CLIENT_ACCOUNTING_INTERIM)) {
		char *argv[] = {NULL, NULL};

		if (req == CLIENT_ACCOUNTING_START) {
			argv[0] = "session-start";
		} else if (req == CLIENT_ACCOUNTING_STOP) {
			argv[0] = "session-stop";

			char cause[32];
			snprintf(cause, sizeof(cause), "TERMINATE_CAUSE=%u",
					(unsigned)cl->login_status);
			eenv[0] = cause;
			eenv[1] = NULL;
		} else if (req == CLIENT_ACCOUNTING_INTERIM) {
			argv[0] = "session-update";
		}


		trigger_run(cfg_cmd, argv, env, NULL, NULL);
	}
}


static struct client_backend fonapi_backend = {
	.auth = fonapi_handler,
	.name = "fonapi",
};

MODULE_REGISTER(fonapi, 450)
BACKEND_REGISTER(fonapi_backend)
