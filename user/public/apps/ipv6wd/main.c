/* vi: set sw=4 ts=4: */
/*
 * main.c		
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


#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/klog.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <signal.h>
#include <stdio.h>
#include <net/if.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <syslog.h>
#include "utils.h"
#include "ip_common.h"
#include <sal/sal_wan.h>
#define SCAN_INTERVAL		        3
int preferred_family = AF_UNSPEC;
struct rtnl_handle rth;
static int scan_interval = SCAN_INTERVAL;
int show_stats = 0;
int timestamp = 0;
static void signal_handler(int sig)
{
    switch (sig)
    {
        case SIGTERM:
            exit(0);
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    int opt = -1;
    unsigned groups = 0;
    int laddr=0;
    int lroute=0;
    int preferred_family = AF_UNSPEC;

    signal(SIGCHLD, SIG_IGN);
    signal (SIGTERM, signal_handler);
    while ((opt = getopt(argc, argv, "m:")) > 0)
    {
        switch (opt)
        {
            case 'm':
                preferred_family = AF_INET6;
                if(0 == strcmp(optarg, "address"))
                {
                    laddr = 1;
                    groups = 0;
                }
                else if(0 == strcmp(optarg, "route"))
                {
                    lroute = 1;
                    groups = 0;
                }
                else if(0 == strcmp(optarg, "addr_route"))
                {
                    laddr = 1;
                    lroute = 1;
                    groups = 0;
                }
                break;
            default:
                break;
        }
    }
    if (laddr) {
        if (!preferred_family || preferred_family == AF_INET6)
            groups |= nl_mgrp(RTNLGRP_IPV6_IFADDR);
    }
    if (lroute) {
        if (!preferred_family || preferred_family == AF_INET6)
            groups |= nl_mgrp(RTNLGRP_IPV6_ROUTE);
    }
    start_ipv6wd(groups, scan_interval);
    return 0;
}

