/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

// iptables invocation API

#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log.h>
#endif
#include "ipt.h"

static const char *cmdparm[] = {
	[IPT_APPEND] = "-A",
	[IPT_PREPEND] = "-I",
	[IPT_DELETE] = "-D",
	[IPT_FLUSHCHAIN] = "-F",
	[IPT_NEWCHAIN] = "-N",
	[IPT_DELETECHAIN] = "-X",
	[IPT_POLICY] = "-P",
	[IPT_TEST] = "-v",
};

static char path_iptables[64] = "/sbin/iptables";
static char path_ip6tables[64] = "/sbin/ip6tables";

void ipt_path(const char *iptables, const char *ip6tables) {
	if (iptables)
		strncpy(path_iptables, iptables, sizeof(path_iptables) - 1);

	if (ip6tables)
		strncpy(path_ip6tables, ip6tables, sizeof(path_ip6tables) - 1);
}

int ipt(enum ipt_tbl tbl, enum ipt_cmd cmd, const char *chain, ...) {
	size_t argc = 6;
	if (cmd >= (sizeof(cmdparm) / sizeof(*cmdparm))) return -1;
	va_list varg;
	va_start(varg, chain);
	while(va_arg(varg, char *)) {
		argc++;
	}
	va_end(varg);

	const char *argv[argc];
	switch(tbl) {
	case IPT_FILTER:
		argv[0] = path_iptables;
		argv[1] = "-t";
		argv[2] = "filter";
		break;

	case IP6T_FILTER:
		argv[0] = path_ip6tables;
		argv[1] = "-t";
		argv[2] = "filter";
		break;

	case IPT_NAT:
		argv[0] = path_iptables;
		argv[1] = "-t";
		argv[2] = "nat";
		break;

	case IP6T_NAT:
		argv[0] = path_ip6tables;
		argv[1] = "-t";
		argv[2] = "nat";
		break;

	case IPT_MANGLE:
		argv[0] = path_iptables;
		argv[1] = "-t";
		argv[2] = "mangle";
		break;

	case IP6T_MANGLE:
		argv[0] = path_ip6tables;
		argv[1] = "-t";
		argv[2] = "mangle";
		break;

	default:
		argv[0] = argv[1] = argv[2] = NULL;
		break;
	}

	argv[3] = cmdparm[cmd];
	argv[4] = chain;

	va_start(varg, chain);
	for (size_t i = 5; i < argc; i++)
		argv[i] = va_arg(varg, char*);
	va_end(varg);

#ifdef IPT_DEBUG
#ifdef __SC_BUILD__
        log_fon(LOG_DEBUG, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "ipt: %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		    argv[0], argv[1], argv[2], argv[3], argv[4],
		    (argc < 7)  ? "" : argv[5],	(argc < 8) ? "" : argv[6],
		    (argc < 9)  ? "" : argv[7],	(argc < 10) ? "" : argv[8],
		    (argc < 11) ? "" : argv[9],	(argc < 12) ? "" : argv[10],
		    (argc < 13) ? "" : argv[11], (argc < 14) ? "" : argv[12],
		    (argc < 15) ? "" : argv[13], (argc < 16) ? "" : argv[14]
);
#else

	syslog(LOG_DEBUG, "ipt: %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		argv[0], argv[1], argv[2], argv[3], argv[4],
		(argc < 7)  ? "" : argv[5],	(argc < 8) ? "" : argv[6],
		(argc < 9)  ? "" : argv[7],	(argc < 10) ? "" : argv[8],
		(argc < 11) ? "" : argv[9],	(argc < 12) ? "" : argv[10],
		(argc < 13) ? "" : argv[11], (argc < 14) ? "" : argv[12],
		(argc < 15) ? "" : argv[13], (argc < 16) ? "" : argv[14]
	);
#endif
#endif

	pid_t proc = fork();
	if (!proc) {
		execv(argv[0], (char**)argv);
		_exit(128);
	}

	int status;
	if (proc > 0 && waitpid(proc, &status, 0) == proc && WIFEXITED(status)) {
		return -WEXITSTATUS(status);
	} else {
		return -1;
	}
}
