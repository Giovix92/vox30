/* vi: set sw=4 ts=4: */
/*
 * ipmonitor.c		
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 *
 * Changes:
 *
 * Rani Assaf <rani@magic.metawire.com> 980929:	resolve addresses
 * Kunihiro Ishiguro <kunihiro@zebra.org> 001102: rtnh_ifindex was not initialized
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "utils.h"
#include "libnetlink.h"
#include "ip_common.h"

int accept_msg(const struct sockaddr_nl *who,
	       struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE*)arg;

	if (timestamp)
		print_timestamp(fp);

	if (n->nlmsg_type == RTM_NEWADDR || n->nlmsg_type == RTM_DELADDR) {
		handle_addrinfo(who, n, arg);
		return 0;
	}
	if (n->nlmsg_type == RTM_NEWROUTE || n->nlmsg_type == RTM_DELROUTE) {
		handle_route(who, n, arg);
		return 0;
	}
	return 0;
}

int start_ipv6wd(unsigned groups, int scan_interval)
{
	if (rtnl_open(&rth, groups) < 0)
		exit(1);
	ll_init_map(&rth);

	if (rtnl_listen2(scan_interval, &rth, accept_msg, stdout) < 0)
		exit(2);

	return 0;
}
