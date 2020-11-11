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
#include <errno.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log.h>
#endif
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/sysinfo.h>

#include "core/hotspotd.h"
#include "core/trigger.h"
#include "core/routing.h"
#include "core/traffic.h"
#include "core/client.h"

#include "lib/config.h"
#include "lib/urandom.h"
#include "lib/hexlify.h"
#include "lib/usock.h"
#include "lib/event.h"

#include "aes.h"
#include "handler.h"

static const char *cfg_server, *cfg_port;
static const char *cfg_tr_offline, *cfg_tr_online;
static uint32_t cfg_threshold, cfg_resend_tries, cfg_resend_delay, cfg_delay;
static uint32_t cfg_delay_offline, cfg_cfgtime;
static int cfg_settime;
static uint8_t cfg_version[4] = {0};
static uint8_t cfg_mac[6];

static uint32_t sessiontoken;

static uint64_t bytes_in_off = 0, bytes_out_off = 0;
static uint64_t bytes_in_cur = 0, bytes_out_cur = 0;

static struct {
	uint8_t aesiv[16]; // AES IV
	uint32_t seqid; // sequence number
} hb_req;

static uint8_t hb_key[32] = {0}; // AES-key, DNS packet buffer
static uint32_t hb_retry; // Retry counter
static uint32_t hb_resend; // Request resend counter
static uint32_t hb_online; // Online state
static uint32_t hb_timeoffset;

static void fonrpc_heartbeat(struct event_timer *timer, int64_t now);
static void fonrpc_receive(struct event_epoll *event, uint32_t revent);
static void fonrpc_parseconfig(char *data);

static struct event_epoll event_heartbeat = {
	.fd = -1,
	.events = EPOLLIN | EPOLLET,
	.handler = fonrpc_receive,
};

static struct event_timer event_retry = {
	.handler = fonrpc_heartbeat,
};

static void fonrpc_stats() {
	uint64_t byte_in = 0, byte_out = 0;
	uint32_t pkts_in, pkts_out;
	traffic_stats(0, &byte_in, &byte_out, &pkts_in, &pkts_out);

	if (byte_in < bytes_in_cur)
		bytes_in_off += bytes_in_cur;

	if (byte_out < bytes_out_cur)
		bytes_out_off += bytes_out_cur;

	bytes_in_cur = byte_in;
	bytes_out_cur = byte_out;
}

static int fonrpc_apply() {
	cfg_server = config_get_string("fonsmc", "rpcserver", NULL);
	cfg_port = config_get_string("fonsmc", "rpcport", NULL);
	cfg_resend_tries = config_get_int("fonsmc", "resend_tries", 4);
	cfg_resend_delay = config_get_int("fonsmc", "resend_delay", 60);
	cfg_threshold = config_get_int("fonsmc", "threshold", 0);
	cfg_delay = config_get_int("fonsmc", "delay", 1000);
	cfg_delay_offline = config_get_int("fonsmc", "delay_offline", 250);
	cfg_settime = config_get_int("fonsmc", "settime", 0);

	const char *version = config_get_string("fonsmc", "version", "");
	sscanf(version, "%hhu.%hhu.%hhu.%hhu", &cfg_version[0],
			&cfg_version[1], &cfg_version[2], &cfg_version[3]);

	const char *mac = config_get_string("main", "nasid", "");
	sscanf(mac, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &cfg_mac[0], &cfg_mac[1],
			&cfg_mac[2], &cfg_mac[3], &cfg_mac[4], &cfg_mac[5]);

	const char *key = config_get_string("fonsmc", "key", NULL);
	if (!key || strlen(key) != 64 || unhexlify(hb_key, key, 64)) {
#ifdef __SC_BUILD__
       log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "fonsmc: Invalid AES-key\n");
#else
		syslog(LOG_ERR, "fonsmc: Invalid AES-key");
#endif
		return -1;
	}

	cfg_cfgtime = config_get_int("fonsmc", "cfgtime", 0);

	// Get trigger conf
	cfg_tr_online = config_get_string("fonsmc", "trigger_online", NULL);
	cfg_tr_offline = config_get_string("fonsmc", "trigger_offline", NULL);

	return 0;
}

static int fonrpc_init() {
	urandom(&sessiontoken, sizeof(sessiontoken));
	hb_resend = hb_online = hb_retry = 0;
	hb_timeoffset = time(NULL) - (event_time() / 1000);
	event_retry.value = 0;
	event_ctl(EVENT_TIMER_ADD, &event_retry);
	return fonrpc_apply();
}

// Cleanup socket and requests and rearm timer
static void fonrpc_cleanup() {
	close(event_heartbeat.fd);
	event_heartbeat.fd = -1;
	hb_resend = 0;

	// Rearm timer for next full retry cycle
	urandom(&event_retry.value, sizeof(event_retry.value));
	if (hb_online)
		event_retry.value %= cfg_delay * 1000;
	else
		event_retry.value %= cfg_delay_offline * 1000;

	event_ctl(EVENT_TIMER_MOD, &event_retry);
}

static void fonrpc_deinit() {
	fonrpc_cleanup();
	event_ctl(EVENT_TIMER_DEL, &event_retry);
	if (hb_online) {
		fonrpc_stats();
		struct trigger_handle *tr =
			trigger_run(cfg_tr_offline, NULL, NULL, NULL, NULL);
		if (tr)
			trigger_wait(tr);
	}
}

// Timer function to (re)send heartbeats
static void fonrpc_heartbeat(struct event_timer *timer, int64_t now) {
	fonrpc_stats();
	if (++hb_resend > cfg_resend_tries) { // No reply from server
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "fonrpc: heartbeat request time out\n");
#else
		syslog(LOG_WARNING, "fonrpc: heartbeat request time out");
#endif
		if (hb_retry > cfg_threshold && hb_online) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "fonrpc: status => offline\n");
#else
		syslog(LOG_WARNING, "fonrpc: status => offline");
#endif
			hb_online = 0;
			trigger_run(cfg_tr_offline, NULL, NULL, NULL, NULL);
		}

		fonrpc_cleanup();
		return;
	}

	const char *host = (cfg_server) ? cfg_server : "rpc.fon-box.com";

	// Rearm timer for next resend cycle
	event_retry.value = cfg_resend_delay * 1000;
	event_ctl(EVENT_TIMER_MOD, &event_retry);

	// Open socket if not already done
	if (event_heartbeat.fd < 0) {
		event_heartbeat.fd = usock(USOCK_UDP, host,
				(cfg_port) ? cfg_port : "54");
		// Register event handler for receiving heartbeat replies
		event_ctl(EVENT_EPOLL_ADD, &event_heartbeat);
	}

	// Create new AESIV / sequence when new request is sent
	if (hb_resend == 1) {
		urandom(&hb_req, sizeof(hb_req));
		hb_retry++;
	}

	// Initialize query
	uint8_t buf[512], req[512];
	struct fr0hdr *fr0 = (struct fr0hdr*)req;
	struct frmsghdr *frm = (struct frmsghdr*)buf;

	// Construct header
	fr0->version = 0;
	fr0->code = 0;
	memcpy(fr0->nodeid, cfg_mac, sizeof(fr0->nodeid));
	memcpy(fr0->aesiv, hb_req.aesiv, sizeof(fr0->aesiv));
	frm->seq = hb_req.seqid;
	frm->time = fswap32((event_time() / 1000) + hb_timeoffset);
	struct frattr *fra = fra_init(&frm->fra, FRMSG_CHECKIN);

	// Construct attribs
	fra_put_u32(fra, FRA_CI_CFGTIME, cfg_cfgtime);
	fra_put_u32(fra, FRA_CI_USERS, client_served());
	fra_put(fra, FRA_CI_FWVER, cfg_version, sizeof(cfg_version));

	struct sysinfo si;
	sysinfo(&si);
	fra_put_u32(fra, FRA_CI_UPTIME, si.uptime);
	fra_put_u64(fra, FRA_CI_BYTESIN, bytes_in_off + bytes_in_cur);
	fra_put_u64(fra, FRA_CI_BYTESOUT, bytes_out_off + bytes_out_cur);
	fra_put_u32(fra, FRA_CI_SESSID, sessiontoken);

	size_t aeslen = AES_ALIGN(FRA_PAYLOAD(fra) + sizeof(*frm));

	aes_context aes;
	uint8_t aesiv[16];
	memcpy(aesiv, fr0->aesiv, sizeof(aesiv));

	aes_setkey_enc(&aes, hb_key, sizeof(hb_key) * 8);
	aes_crypt_cbc(&aes, AES_ENCRYPT, aeslen, aesiv, (uint8_t*)frm,
						req + sizeof(*fr0));

	aeslen += sizeof(*fr0);

	if (send(event_heartbeat.fd, req, aeslen, MSG_DONTWAIT) < 0)
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "fonrpc: unable to send heartbeat: %s\n", strerror(errno));
#else
		syslog(LOG_WARNING, "fonrpc: unable to send heartbeat: %s",
							strerror(errno));
#endif
}

// Receive DNS replies if we have datagrams queued
static void fonrpc_receive(struct event_epoll *event, uint32_t revent) {
	uint8_t buf[32768], buf2[32768];
	ssize_t len;
	struct fr0hdr *rep = (void*)buf; // Reply

	while ((len = recv(event->fd, buf, sizeof(buf), MSG_DONTWAIT)) >= 0) {
		if (len < sizeof(struct fr0hdr))
			continue;

		if (rep->code) { // Error reply
			if (memcmp(rep->aesiv, hb_req.aesiv, 16))
				continue; // validation failed

			if (rep->code == FR0_E_TIMESYNC) {
				struct fr0hdr_sync *fr0s = (void*)buf;
				if (cfg_settime) {
					struct timespec ts = {
						.tv_sec = fswap32(fr0s->time),
					};
					syslog(LOG_INFO, "fonrpc: Time set");
					clock_settime(CLOCK_REALTIME, &ts);
					hb_timeoffset = time(NULL) -
							(event_time() / 1000);
				} else {
					hb_timeoffset = fswap32(fr0s->time) -
							(event_time() / 1000);
				}
				// Create new sequence ID and AES-IV
				urandom(&hb_req, sizeof(hb_req));
				event_retry.value = 0;
				event_ctl(EVENT_TIMER_MOD, &event_retry);
				continue;
			} else if (rep->code == FR0_E_NOACCESS) {
#ifdef __SC_BUILD__
                log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "fonrpc: Access Denied\n");
#else
				syslog(LOG_WARNING, "fonrpc: Access Denied");
#endif
			} else {
#ifdef __SC_BUILD__
                log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "fonrpc: error %s\n",
						strerror(rep->code));
#else
				syslog(LOG_WARNING, "fonrpc: error %s",
						strerror(rep->code));
#endif
			}

			// We don't have to resend this round as we got a reply
			hb_resend = -2;
			continue;
		}

		struct frmsghdr *frm = (struct frmsghdr*)buf2;
		size_t frmlen = len - sizeof(*rep);
		if (frmlen < sizeof(*frm) || AES_ALIGN(frmlen) != frmlen)
			continue; // message to short or not AES-CBC-aligned

		// We don't have to resend this round as we got a reply
		hb_resend = -2;

		aes_context aes;
		aes_setkey_dec(&aes, hb_key, sizeof(hb_key) * 8);
		aes_crypt_cbc(&aes, AES_DECRYPT, frmlen, rep->aesiv,
					buf + sizeof(*rep), (uint8_t*)frm);

		struct frattr *fra = &frm->fra;
		if (AES_ALIGN(FRA_PAYLOAD(fra) + sizeof(*frm)) != frmlen
		|| FRA_TYPE(fra) != FRMSG_CHECKIN || frm->seq != hb_req.seqid)
			continue; // sequence mismatch or invalid length

		struct frattr *fr[FRA_CI_MAX];
		fra_parse(fra, fr, FRA_CI_MAX);
		uint32_t cfgtime = fra_to_u32(fr[FRA_CI_CFGTIME], 0);
		if ((!cfgtime || cfgtime > cfg_cfgtime) && fr[FRA_CI_CFGDATA]) {
			char cfghash[33];
			snprintf(cfghash, sizeof(cfghash), "%lu",
					(unsigned long)cfgtime);
			config_set_string("fonsmc", "cfgtime", cfghash);
			char *cfg = (char*)fra_to_string(fr[FRA_CI_CFGDATA]);
			fonrpc_parseconfig(cfg);

			char *fwurl = (char*)fra_to_string(fr[FRA_CI_FWURL]);
			const char *tr_upgrade = config_get_string("fonsmc",
						"trigger_upgrade", NULL);
			if (fwurl) {
				char *uargv[] = {fwurl, NULL};
				struct trigger_handle *tr = trigger_run
					(tr_upgrade, uargv, NULL, NULL, NULL);
				if (tr)
					trigger_wait(tr);
			}

		}

		// We are online
		fonrpc_cleanup();
		hb_retry = 0;
		if (!hb_online) {
#ifdef __SC_BUILD__
            log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "fonrpc: status => online\n");
#else
			syslog(LOG_WARNING, "fonrpc: status => online");
#endif
			hb_online = 1;
			trigger_run(cfg_tr_online, NULL, NULL, NULL, NULL);
		}
	}
}

static void fonrpc_parseconfig(char *data) {

	int new_radius = 0, new_staticwl = 0, new_dynamicwl = 0;
	int use_anydns = 0;

	char *cfg_argv[129];
	size_t cfg_argc = 0;


	char *line, *saveptr, *saveptr2;
	for (line = strtok_r(data, "\n", &saveptr); line;
				line = strtok_r(NULL, "\n", &saveptr)) {
		syslog(LOG_INFO, "fonrpc: received config value: %s", line);
		char *key = strtok_r(line, " \t\n", &saveptr2), *val = NULL;
		if (key)
			val = strtok_r(NULL, "\n", &saveptr2);

		if (!key) {
			// WTF?!
		} else if (!strncmp(key, "radiusserver", 12)) {
			if (!new_radius++) { // Clear RADIUS servers once
				config_set_string("radauth", "server", NULL);
				config_set_string("radacct", "server", NULL);
			}
			config_add_string("radauth", "server", val);
			config_add_string("radacct", "server", val);
		} else if (!strcmp(key, "radiussecret")) {
			config_set_string("radauth", "secret", val);
			config_set_string("radacct", "secret", val);
		} else if (!strcmp(key, "uamallowed")) {
			if (!new_staticwl++) // Clear static WL
				config_set_string("whitelist", "static", NULL);
			val = strtok_r(val, ",", &saveptr);
			while (val) {
				config_add_string("whitelist", "static", val);
				val = strtok_r(NULL, ",", &saveptr);
			}
		} else if (!strcmp(key, "newdomain")) {
			if (!new_dynamicwl++) { // Clear dynamic WL
				config_set_string("whitelist", "host", NULL);
				config_set_string("whitelist", "zone", NULL);
				config_set_string("whitelist", "exclude", NULL);
			}
			val = strtok_r(val, ",", &saveptr);
			while (val) {
				if (val[0] == '!') {
					config_add_string("whitelist", "exclude", val + 1);
				} else if (val[0] == '*' && val[1] == '.') {
					config_add_string("whitelist", "zone", val + 2);
				} else {
					config_add_string("whitelist", "host", val);
				}
				val = strtok_r(NULL, ",", &saveptr);
			}
		} else if (!strcmp(key, "uamserver")) {
			config_set_string("redirect", "url", val);
		} else if (!strcmp(key, "uamsecret")) {
			config_set_string("portal", "secret", val);
		} else if (!strcmp(key, "uamanydns")) {
			use_anydns = 1;
		} else if (!strcmp(key, "rpc_interval")) {
			config_set_string("fonsmc", "delay", val);
		} else if (!strcmp(key, "rpc_interval_offline")) {
			config_set_string("fonsmc", "delay_offline", val);
		} else if (!strcmp(key, "rpc_server")) {
			config_set_string("fonsmc", "rpcserver", val);
		} else if (!strcmp(key, "rpc_port")) {
			config_set_string("fonsmc", "rpcport", val);
		} else if (!strcmp(key, "rpc_tries_max")) {
			config_set_string("fonsmc", "resend_tries", val);
		} else if (!strcmp(key, "rpc_tries_time")) {
			config_set_string("fonsmc", "resend_delay", val);
		} else if (!strcmp(key, "cfg_bw_in")) {
			config_set_string("traffic", "overall_kbit_in", val);
		} else if (!strcmp(key, "cfg_bw_out")) {
			config_set_string("traffic", "overall_kbit_out", val);
		} else if (!strncmp(key, "cfg_", 4)) {
			while (cfg_argc < 128) {
				cfg_argv[cfg_argc++] = key + 4;
				cfg_argv[cfg_argc++] = (val) ? val : "";
			}
		} else {
#ifdef __SC_BUILD__
            log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "fonrpc: unknown option %s %s", key, val);
#else

			syslog(LOG_WARNING, "fonrpc: unknown option %s %s", key, val);
#endif
		}
	}

	config_set_string("firewall", "anydns", (use_anydns) ? "1" : "0");

	config_commit();
	hotspot_control(HOTSPOT_CTRL_APPLY);

	cfg_argv[cfg_argc] = NULL;
	const char *tr_setup = config_get_string("fonsmc",
						"trigger_setup", NULL);
	struct trigger_handle *tr = trigger_run(tr_setup, cfg_argv,
							NULL, NULL, NULL);
	if (tr)
		trigger_wait(tr);
}

RESIDENT_REGISTER(fonrpc, 200)
