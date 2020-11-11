/*
 * Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>

#include <unistd.h>
#include <getopt.h>
#include <libgen.h>

#include "procd.h"
#include "watchdog.h"
#include "plug/hotplug.h"

unsigned int debug=5; //soohoo
#ifdef __SC_BUILD__
char *host;;
#endif
static int usage(const char *prog)
{
	ERROR("Usage: %s [options]\n"
		"Options:\n"
		"\t-s <path>\tPath to ubus socket\n"
		"\t-h <path>\trun as hotplug daemon\n"
		"\t-d <level>\tEnable debug messages\n"
		"\n", prog);
	return 1;
}

int main(int argc, char **argv)
{
	int ch;
	char *dbglvl = getenv("DBGLVL");

#ifdef __SC_BUILD__
    char rootdir[512] = "";
    int init = 0;
    host = getenv("EXECENV_INDEX");
    if ((NULL == host) && (1 != getpid()))
        host = "System-EE";
#endif
	ulog_open(ULOG_KMSG, LOG_DAEMON, "procd");

	if (dbglvl) {
		debug = atoi(dbglvl);
		unsetenv("DBGLVL");
	}
#ifdef __SC_BUILD__
	while ((ch = getopt(argc, argv, "d:s:h:r:i")) != -1) {
#else
	while ((ch = getopt(argc, argv, "d:s:h:")) != -1) {
#endif
		switch (ch) {
		case 'h':
			return hotplug_run(optarg);
		case 's':
			ubus_socket = optarg;
			break;
		case 'd':
			debug = atoi(optarg);
			break;
#ifdef __SC_BUILD__
        case 'r':
            snprintf(rootdir, sizeof(rootdir), "%s", optarg);
            break;
        case 'i':
            init = 1;
            break;
#endif
		default:
			return usage(argv[0]);
		}
	}
#ifdef __SC_BUILD__
    if ('\0' != rootdir[0])
        chroot(rootdir);
#endif
	setsid();
    
	uloop_init();
	procd_signal();
	trigger_init();
#ifdef __SC_BUILD__
	if ((getpid() != 1) && (0 == init))
#else
	if (getpid() != 1)
#endif
		procd_connect_ubus();
	else
		procd_state_next();
	uloop_run();
	uloop_done();

	return 0;
}
