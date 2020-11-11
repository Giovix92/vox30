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
#include <string.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include "lib/config.h"
#include "core/hotspotd.h"
#include "core/client.h"

static const char *cfg_file = NULL;
static const struct client_backend *cfg_next_backend = NULL;


static int localusers_apply()
{
	cfg_file = config_get_string("localusers", "file", NULL);
	if(!hotspot_assertconf_string("localusers.file", cfg_file)) {
		return -1;
	}
	const char *c = config_get_string("localusers", "next_backend", NULL);
	cfg_next_backend = client_get_backend(c);

	return 0;
}

static int localusers_init()
{
	localusers_apply();
	return 0;
}

static void localusers_deinit()
{
	return;
}

static void localusers_handler(struct client *cl, enum client_req req,
		client_update_cb *report, void **ctx)
{
#ifdef __SC_BUILD__
    log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "localusers: called prelogin\n");
#else
	syslog(LOG_WARNING, "localusers: called prelogin");
#endif
	bool authed = false;

	if (req != CLIENT_LOGIN)
		return;

	if (!cfg_file || cl->sess.method != CLIENT_PAP)
		goto out;

	FILE *fp = fopen(cfg_file, "r");
	if (!fp) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "localusers: failed to open %s", cfg_file);
#else
		syslog(LOG_WARNING, "localusers: failed to open %s", cfg_file);
#endif
		goto out;
	}

	char buf[512];
	while (fgets(buf, sizeof(buf), fp)) {
		char *saveptr;
		char *user = strtok_r(buf, ":", &saveptr);
		if (!user || strcmp(user, cl->sess.username))
			continue;

		char *pass = strtok_r(NULL, "\n", &saveptr);
		if (!pass)
			continue;

		size_t plen = strlen(pass);
		size_t klen = cl->sess.keylen;

		if (klen > plen && cl->sess.key[plen] == 0)
			klen = plen; // Strip 0-Bytes

		if (klen == plen && !memcmp(pass, cl->sess.key, plen)) {
#ifndef __SC_BUILD__
			syslog(LOG_INFO, "localusers: user '%s' auth'd", user);
#endif
			authed = true;
			break;
		}
	}

	fclose(fp);

out:
	if (authed)
		report(cl, LOGIN_SUCCESS, NULL);
	else if (cfg_next_backend)
		report(cl, LOGIN_NOTYET, cfg_next_backend);
	else
		report(cl, LOGIN_DENIED, NULL);
}

static struct client_backend localusers = {
	.auth = localusers_handler,
	.name = "localusers",
};

BACKEND_REGISTER(localusers)
MODULE_REGISTER(localusers, 300)
