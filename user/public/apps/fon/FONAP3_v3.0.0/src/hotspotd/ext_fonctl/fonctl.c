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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "core/hotspotd.h"
#include "libfonrpc/fonrpc-local.h"
#include "rpc/021-radconf.h"
#include "rpc/014-client.h"
#include "rpc/012-whitelist.h"
#include "rpc/011-firewall.h"
#include "rpc/010-core.h"

// RPC connection and state
static int sock = -1;
static uint32_t seq = 0;
static uint8_t buffer[8192] = {0};
static struct frmsg *next = NULL;
static size_t buffered = 0;

// Send an RPC request
static int request(struct frmsg *frm) {
	frm->frm_seq = fswap32(++seq);
#ifdef __SC_BUILD__
	frm->frm_flags |= fswap32(FRM_F_REQUEST);
#else
	frm->frm_flags |= fswap16(FRM_F_REQUEST);
#endif
	if (send(sock, frm, frm_length(frm), 0) > 0)
		return seq;
	return -1;
}

// Receive reply
static struct frmsg* receive(int seq) {
	struct frmsg *frm;
	// Test for valid messages that match our sequence ID
	while (!(frm = frm_load(next, buffered)) || frm->frm_seq != fswap32(seq)) {
		ssize_t len = recv(sock, buffer, sizeof(buffer), 0);
		if (len < 0) {
			if (errno == EINTR)
				continue;
			return NULL; // Socket error
		}

		// Add content to buffer
		buffered = len;
		next = (void*)buffer;
	}
	// Move buffer to next position
	next = FRM_NEXT(next, buffered);
	return frm;
}

// Send a message, expecting only a status reply
static int call(struct frmsg *frm) {
	int s = request(frm);
	if (s < 0 || !(frm = receive(s)))
		return errno;
	if (frm_type(frm) != FRMSG_ERROR)
		return 0;
	struct frattr *attrs[_FRE_SIZE];
	frm_parse(frm, attrs, _FRE_SIZE);
	return fra_to_u32(attrs[FRE_CODE], 0);
}

// info
static int info(int argc, char *const argv[]) {
	uint8_t buffer[64];
	struct frmsg *frm = frm_init(buffer, FRT_SYS_INFO, 0);
	int seq = request(frm);
	if (seq < 0)
		return errno;

	if (!(frm = receive(seq)))
		return errno;

	if ((seq = frm_status(frm)))
		return seq;

	struct frattr *fra[_FRA_SYS_SIZE];
	frm_parse(frm, fra, _FRA_SYS_SIZE);

	fprintf(stdout, "hotspotd pid=%u dynmemory=%u\n",
				fra_to_u32(fra[FRA_SYS_PID], 0),
				fra_to_u32(fra[FRA_SYS_DYNMEMORY], 0));
	return 0;
}

//radconf
static int radconf(int argc, char *const argv[]) {
	uint8_t buffer[384];
	struct frmsg *frm = frm_init(buffer, FRT_RC_CALL, 0);

	if (argc >= 2 && strlen(argv[1]) < 256)
		frm_put_string(frm, FRA_RC_USER, argv[1]);

	return call(frm);
}

// config
static int config(int argc, char *const argv[]) {
	uint8_t buffer[1024];
	struct frmsg *frm;
	if (argc < 2)
		return EINVAL;

	if (!strcmp(argv[1], "restart")) {
		frm = frm_init(buffer, FRT_CFG_RESTART, 0);
	} else if (!strcmp(argv[1], "apply")) {
		frm = frm_init(buffer, FRT_CFG_APPLY, 0);
	} else if (!strcmp(argv[1], "commit")) {
		frm = frm_init(buffer, FRT_CFG_COMMIT, 0);
	} else if (!strcmp(argv[1], "set") && argc >= 5) {
		frm = frm_init(buffer, FRT_CFG_SET, 0);
	} else if (!strcmp(argv[1], "append") && argc >= 5) {
		frm = frm_init(buffer, FRT_CFG_SET, FRM_F_APPEND);
	} else if (!strcmp(argv[1], "unset") && argc >= 3) {
		frm = frm_init(buffer, FRT_CFG_SET, 0);
	} else {
		return EINVAL;
	}

	if (argc >= 3 && strlen(argv[2]) < 256)
		frm_put_string(frm, FRA_CFG_SECTION, argv[2]);

	if (argc >= 4 && argv[3] && strlen(argv[3]) < 256)
		frm_put_string(frm, FRA_CFG_OPTION, argv[3]);

	if (argc >= 5 && strlen(argv[4]) < 256)
		frm_put_string(frm, FRA_CFG_VALUE, argv[4]);

	return call(frm);
}

// list, listip and listmac
// List current sessions
static int list(int argc, char *const argv[]) {
	uint8_t buffer[64];
	struct frmsg *frm = frm_init(buffer, FRT_CL_GET, 0);
	if (!strcmp(argv[0], "list")) {
#ifdef __SC_BUILD__
		frm->frm_flags |= fswap32(FRM_F_DUMP);
#else
		frm->frm_flags |= fswap16(FRM_F_DUMP);
#endif
	} else if (!strcmp(argv[0], "listmac") && argc > 1) {
		uint8_t addr[6] = {0};
		if (sscanf(argv[1], "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx",
			&addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) < 6)
			return EINVAL;
		frm_put_buffer(frm, FRA_CL_HWADDR, addr, 6);
	} else {
		return EINVAL;
	}

	int seq = request(frm);
	if (seq < 0)
		return errno;

	while ((frm = receive(seq)) && !frm_status(frm)
			&& frm_type(frm) != FRMSG_DONE) {
		struct frattr *fra[_FRA_CL_SIZE];
		frm_parse(frm, fra, _FRA_CL_SIZE);

		// Preformat data
		char mac[24] = "", *stat;
		if (fra_length(fra[FRA_CL_HWADDR]) >= 6) {
			const uint8_t *data = fra_data(fra[FRA_CL_HWADDR]);
			snprintf(mac, sizeof(mac), "%02X-%02X-%02X-%02X-%02X-%02X",
				data[0], data[1], data[2], data[3], data[4], data[5]);
		}

		const char *user = fra_to_string(fra[FRA_CL_USERNAME]);
		if (!user)
			user = "-";

		switch (fra_to_u32(fra[FRA_CL_STATUS], 0)) {
		case CL_CLSTAT_NOAUTH:
			stat = "noauth";
			break;
		case CL_CLSTAT_LOGOUT:
			stat = "logout";
			break;
		case CL_CLSTAT_LOGIN:
			stat = "login";
			break;
		case CL_CLSTAT_AUTHED:
			stat = "authed";
			break;
		default:
			stat = "unknown";
			break;
		}

		fprintf(stdout, "%s - %s %016x %u %s %u/%u %u/%u "
				"%llu/%u %llu/%u %u 1 0/0 0/0 -\n",
				mac, stat, fra_to_u32(fra[FRA_CL_SESSID], 0),
				!!user, user,
				fra_to_u32(fra[FRA_CL_TIME], 0),
				fra_to_u32(fra[FRA_CL_TIME_MAX], 0),
				fra_to_u32(fra[FRA_CL_IDLE], 0),
				fra_to_u32(fra[FRA_CL_IDLE_MAX], 0),
				(unsigned long long)fra_to_u64(fra[FRA_CL_BIN], 0),
				fra_to_u32(fra[FRA_CL_BIN_MAX], 0),
				(unsigned long long)fra_to_u64(fra[FRA_CL_BOUT], 0),
				fra_to_u32(fra[FRA_CL_BOUT_MAX], 0),
				fra_to_u32(fra[FRA_CL_BOTH_MAX], 0)
		);

		if (!(frm_flags(frm) & FRM_F_MULTI))
			break;
	}

	return frm_status(frm);
}

static int listgarden(int argc, char *const argv[]) {
	uint8_t buffer[64];
	struct frmsg *frm = frm_init(buffer, FRT_WL_GETACTIVE, FRM_F_DUMP);
	int seq = request(frm);
	if (seq < 0)
		return errno;

	while ((frm = receive(seq)) && !frm_status(frm)) {
		if (!(frm_flags(frm) & FRM_F_MULTI) || frm_type(frm) == FRMSG_DONE)
			break;
		struct frattr *fra[_FRA_WL_SIZE];
		frm_parse(frm, fra, _FRA_WL_SIZE);

		char ip[INET6_ADDRSTRLEN] = "";
		if (fra_length(fra[FRA_WL_IPV4]) >= 4) {
			inet_ntop(AF_INET, fra_data(fra[FRA_WL_IPV4]), ip, sizeof(ip));
		} else if (fra_length(fra[FRA_WL_IPV6]) >= 16) {
			inet_ntop(AF_INET6, fra_data(fra[FRA_WL_IPV6]), ip, sizeof(ip));
		}

		fprintf(stdout, "host=%s proto=0 port=0\n", ip);
	}

	return frm_status(frm);
}

static int block(int argc, char *const argv[]) {
	uint8_t buffer[64], mac[6];
	if (argc < 2 || sscanf(argv[1], "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx",
	&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) < 6)
		return EINVAL;

	struct frmsg *frm = frm_init(buffer, FRT_FW_NEWBLOCK, 0);
	frm_put_buffer(frm, FRA_FW_HWADDR, mac, 6);
	return call(frm);
}

static int client(int argc, char *const argv[]) {
	uint8_t buffer[1024];
	struct frmsg *frm;
	if (argc < 3) {
		return EINVAL;
	} else if (!strcmp(argv[0], "authorize")) {
		frm = frm_init(buffer, FRT_CL_LOGIN, 0);
		frm_put_u32(frm, FRA_CL_METHOD, CL_CLIENT_GRANT);
	} else if (!strcmp(argv[0], "login")) {
		frm = frm_init(buffer, FRT_CL_LOGIN, 0);
		frm_put_string(frm, FRA_CL_BACKEND, "radius");
		frm_put_u32(frm, FRA_CL_METHOD, CL_CLIENT_PAP);
	} else if (!strcmp(argv[0], "update")) {
		frm = frm_init(buffer, FRT_CL_SET, 0);
	} else if (!strcmp(argv[0], "logout")) {
		frm = frm_init(buffer, FRT_CL_DEL, 0);
	} else {
		return EINVAL;
	}

	if (!strcmp(argv[1], "mac")) {
		uint8_t addr[6] = {0};
		if (sscanf(argv[2], "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx",
			&addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) < 6)
			return EINVAL;
		frm_put_buffer(frm, FRA_CL_HWADDR, addr, 6);
	} else if (!strcmp(argv[1], "sessionid")) {
		frm_put_u32(frm, FRA_CL_SESSID, strtoul(argv[2], NULL, 16));
	} else {
		return EINVAL;
	}

	// Options
	for (size_t i = 3; i < argc; ++i) {
		if (!strcmp(argv[i], "username") && ++i < argc) {
			if (strlen(argv[i]) < 256) {
				frm_put_string(frm, FRA_CL_USERNAME, argv[i]);
			} else {
				return EINVAL;
			}
		} else if (!strcmp(argv[i], "password") && ++i < argc) {
			if (strlen(argv[i]) < 256) {
				frm_put_buffer(frm, FRA_CL_KEY, argv[i], strlen(argv[i]));
			} else {
				return EINVAL;
			}
		} else if (!strcmp(argv[i], "sessiontimeout") && ++i < argc) {
			frm_put_u32(frm, FRA_CL_TIME_MAX, strtoul(argv[i], NULL, 10));
		} else if (!strcmp(argv[i], "idletimeout") && ++i < argc) {
			frm_put_u32(frm, FRA_CL_IDLE_MAX, strtoul(argv[i], NULL, 10));
		} else if (!strcmp(argv[i], "interiminterval") && ++i < argc) {
			frm_put_u32(frm, FRA_CL_INTERIM, strtoul(argv[i], NULL, 10));
		} else if (!strcmp(argv[i], "maxoctets") && ++i < argc) {
			frm_put_u32(frm, FRA_CL_BOTH_MAX, strtoul(argv[i], NULL, 10));
		} else if (!strcmp(argv[i], "maxinputoctets") && ++i < argc) {
			frm_put_u32(frm, FRA_CL_BIN_MAX, strtoul(argv[i], NULL, 10));
		} else if (!strcmp(argv[i], "maxoutputoctets") && ++i < argc) {
			frm_put_u32(frm, FRA_CL_BOUT_MAX, strtoul(argv[i], NULL, 10));
		} else if (!strcmp(argv[i], "maxbwup") && ++i < argc) {
			// ignored
		} else if (!strcmp(argv[i], "maxbwdown") && ++i < argc) {
			// ignored
		} else if (!strcmp(argv[i], "splash") && ++i < argc) {
			// ignored
		} else if (!strcmp(argv[i], "url") && ++i < argc) {
			// ignored
		} else if (!strcmp(argv[i], "routeidx") && ++i < argc) {
			// ignored
		} else if (!strcmp(argv[i], "noacct")) {
			frm_put_flag(frm, FRA_CL_NOACCT);
		} else {
			return EINVAL;
		}
	}

	return call(frm);
}

static int usage(const char *arg0) {
	fprintf(stderr,
	"fonctl - fonapi remote control\n"
	"(c) 2011 Steven Barth, John Crispin\n\n"
	"Usage: %s [options] <command> [parameters]\n\n"
	"Options:\n"
	"	-c <name>			Use a custom control socket\n"
	"	-t <timeout>			Set timeout in milliseconds\n"
	"	-h				Show this help\n\n"
	"Commands:\n"
	"	info				Dump hotspot info\n"
	"	config set <sec> <opt> <val>	Add or set a config option\n"
	"	config append <sec> <opt> <val>	Append to a config list\n"
	"	config unset <sec> <opt>	Delete a config option\n"
	"	config commit			Save config changes to file\n"
	"	config apply			Reapply configuration (soft)\n"
	"	config restart			Restart hotspot (hard)\n"
	"	list				List all active sessions\n"
	"	listmac <mac-addr>		List one session by MAC\n"
	"	listgarden			List active IP whitelist\n"
	"	block				Block a client MAC-address\n"
	"	authorize mac|sessionid <id>	Authorize a given session\n"
	"		[session options...]	and apply session options\n"
	"	login mac|sessionid <id>	Login a given session\n"
	"		[session options...]	and apply session options\n"
	"	update mac|sessionid <id>	Update a given session\n"
	"		[session options...]	and apply session options\n"
	"	logout mac|sessionid <id>	Terminate a given session\n"
	"	radconf				Do a radconfig call\n\n"
	"Session Options:\n"
	"	username <val>			Login username\n"
	"	password <val>			Login password\n"
	"	sessiontimeout <val>		Maximum session time in s\n"
	"	idletimeout <val>		Maximum idle time in s\n"
	"	interiminterval <val>		Acct. interim interval in s\n"
	"	maxoctets <val>			Total traffic limit in B\n"
	"	maxinputoctets <val>		Input traffic limit in B\n"
	"	maxoutputoctets <val>		Ouput traffic limit in B\n"
	"	noacct				Disable accounting\n"
	"	notun				Disable tunneling\n\n",
	arg0);
	return EINVAL;
}

int fonctl(int argc, char *const argv[]) {
	const char *path = "/var/run/hotspotd.sock";
	uint32_t timeout = 1000;
	int c;

	// Scan options
	while ((c = getopt(argc, argv, "c:t:h")) != -1) {
		switch (c) {
		case 'c':
			path = optarg;
			break;

		case 't':
			timeout = strtoul(optarg, NULL, 10);
			break;

		default:
			return usage(argv[0]);
			break;
		}
	}

	if (!argv[optind])
		return usage(argv[0]);

	// Open RPC socket
	struct sockaddr_un sa = { .sun_family = AF_UNIX };

	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	struct timeval tv = {
		.tv_sec = timeout / 1000,
		.tv_usec = (timeout % 1000) * 1000,
	};
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	sprintf(sa.sun_path + 1, "fonctl%i", (int)getpid());
	bind(sock, (void*)&sa, sizeof(sa));

	strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
	if (connect(sock, (void*)&sa, sizeof(sa))) {
		perror("fonctl: failed to connect to hotspotd");
		return errno;
	}
	seq = time(NULL);

	int status;
	if (!strcmp(argv[optind], "info")) {
		status = info(argc - optind, &argv[optind]);
	} else if (!strcmp(argv[optind], "config")) {
		status = config(argc - optind, &argv[optind]);
	} else if (!strcmp(argv[optind], "list")
	|| !strcmp(argv[optind], "listip")
	|| !strcmp(argv[optind], "listmac")) {
		status = list(argc - optind, &argv[optind]);
	} else if (!strcmp(argv[optind], "listgarden")) {
		status = listgarden(argc - optind, &argv[optind]);
	} else if (!strcmp(argv[optind], "block")) {
		status = block(argc - optind, &argv[optind]);
	} else if (!strcmp(argv[optind], "authorize")
	|| !strcmp(argv[optind], "login")
	|| !strcmp(argv[optind], "update")
	|| !strcmp(argv[optind], "logout")) {
		status = client(argc - optind, &argv[optind]);
	} else if (!strcmp(argv[optind], "radconf")) {
			status = radconf(argc - optind, &argv[optind]);
	} else {
		fprintf(stderr, "Unknown Command: %s\n", argv[optind]);
		status = EINVAL;
	}

	close(sock);
	if (status)
		fprintf(stderr, "fonctl: %s\n", strerror(status));
	return status;
}

MULTICALL_REGISTER(fonctl, fonctl)
