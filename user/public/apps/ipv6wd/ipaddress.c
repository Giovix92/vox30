/* vi: set sw=4 ts=4: */
/*
 * ipaddress.c		"ip address".
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 * Changes:
 *	Laszlo Valko <valko@linux.karinthy.hu> 990223: address label must be zero terminated
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fnmatch.h>

#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>

#include "utils.h"
#include "ip_common.h"
#include <sal/sal_wan.h>

#define PATH_PROCNET_IFINET6    "/proc/net/if_inet6"
#define IPV6_ADDR_GLOBAL        0x0000U
static struct
{
    int ifindex;
    int family;
    int oneline;
    int showqueue;
    inet_prefix pfx;
    int scope, scopemask;
    int flags, flagmask;
    int up;
    char *label;
    int flushed;
    char *flushb;
    int flushp;
    int flushe;
} filter;
static int _util_getIP6_GlobalInfo(char *ifname, char *address )
{
    FILE *fp;
    char addr6[40], devname[20];
    struct in6_addr inaddr6;
    int plen, scope, dad_status, if_idx;
    char addr6p[8][5];

    if ((fp = fopen(PATH_PROCNET_IFINET6, "r")) != NULL) {
        while ( fscanf(fp, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
                    addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
                    addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
                    &dad_status, devname) != EOF) {
            if (!strcmp(devname, ifname) && (scope == IPV6_ADDR_GLOBAL)
                    && !(addr6p[0][0] == 'f' && (addr6p[0][1] == 'c' || addr6p[0][1] == 'd')))
            {
                sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
                        addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                        addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
                inet_pton(AF_INET6, addr6, &inaddr6);
                inet_ntop(AF_INET6, (void *)&inaddr6, address, INET6_ADDRSTRLEN);
              //  sprintf(address,"%s",address);

                fclose(fp);
                return 0;
            }
            else
                continue;
        }
        fclose(fp);
    }
    return -1;	
}
static int flush_update(void)
{
    if (rtnl_send(&rth, filter.flushb, filter.flushp) < 0) {
        perror("Failed to send flush request\n");
        return -1;
    }
    filter.flushp = 0;
    return 0;
}

int handle_addrinfo(const struct sockaddr_nl *who, struct nlmsghdr *n,
		   void *arg)
{
    FILE *fp = (FILE*)arg;
    struct ifaddrmsg *ifa = NLMSG_DATA(n);
    int len = n->nlmsg_len;
    char *if_name;

    struct rtattr * rta_tb[IFA_MAX+1];
    char abuf[256];
    SPRINT_BUF(b1);
    if (n->nlmsg_type != RTM_NEWADDR && n->nlmsg_type != RTM_DELADDR)
        return 0;
    len -= NLMSG_LENGTH(sizeof(*ifa));
    if (len < 0) {
        fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
        return -1;
    }

    if (filter.flushb && n->nlmsg_type != RTM_NEWADDR)
        return 0;

    parse_rtattr(rta_tb, IFA_MAX, IFA_RTA(ifa), n->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa)));

    if (!rta_tb[IFA_LOCAL])
        rta_tb[IFA_LOCAL] = rta_tb[IFA_ADDRESS];
    if (!rta_tb[IFA_ADDRESS])
        rta_tb[IFA_ADDRESS] = rta_tb[IFA_LOCAL];

    if (filter.ifindex && filter.ifindex != ifa->ifa_index)
        return 0;
    if ((filter.scope^ifa->ifa_scope)&filter.scopemask)
        return 0;
    if ((filter.flags^ifa->ifa_flags)&filter.flagmask)
        return 0;
    if (filter.label) {
        SPRINT_BUF(b1);
        const char *label;
        if (rta_tb[IFA_LABEL])
            label = RTA_DATA(rta_tb[IFA_LABEL]);
        else
            label = ll_idx_n2a(ifa->ifa_index, b1);
        if (fnmatch(filter.label, label, 0) != 0)
            return 0;
    }
    if (filter.pfx.family) {
        if (rta_tb[IFA_LOCAL]) {
            inet_prefix dst;
            memset(&dst, 0, sizeof(dst));
            dst.family = ifa->ifa_family;
            memcpy(&dst.data, RTA_DATA(rta_tb[IFA_LOCAL]), RTA_PAYLOAD(rta_tb[IFA_LOCAL]));
            if (inet_addr_match(&dst, &filter.pfx, filter.pfx.bitlen))
                return 0;
        }
    }

    if (filter.family && filter.family != ifa->ifa_family)
        return 0;

    if (filter.flushb) {
        struct nlmsghdr *fn;
        if (NLMSG_ALIGN(filter.flushp) + n->nlmsg_len > filter.flushe) {
            if (flush_update())
                return -1;
        }
        fn = (struct nlmsghdr*)(filter.flushb + NLMSG_ALIGN(filter.flushp));
        memcpy(fn, n, n->nlmsg_len);
        fn->nlmsg_type = RTM_DELADDR;
        fn->nlmsg_flags = NLM_F_REQUEST;
        fn->nlmsg_seq = ++rth.seq;
        filter.flushp = (((char*)fn) + n->nlmsg_len) - filter.flushb;
        filter.flushed++;
        if (show_stats < 2)
            return 0;
    }

    if_name = ll_index_to_name(ifa->ifa_index);
    if(0 == strncmp(ll_index_to_name(ifa->ifa_index), "if", 2))
    {
	    ll_init_map(&rth);
        if_name = ll_index_to_name(ifa->ifa_index);
    }
    if ((n->nlmsg_type == RTM_DELADDR) && (strcmp(rtnl_rtscope_n2a(ifa->ifa_scope, b1, sizeof(b1)), "global") == 0))
    {
        char global_addr[64] = {0};
        int i;
        for(i=0;i<WAN_MAX_NUM;i++)
        {
            if(strcmp(if_name, sal_wan_get_ipv6_wan_if_t(i)) == 0)
            {
                if(_util_getIP6_GlobalInfo(if_name, global_addr) == -1)
                {
                    sal_wan_set_ipv6_open_state_t(i, "0");
                }
                break;
            }
        }
    }
    if ((n->nlmsg_type == RTM_NEWADDR) && (strcmp(rtnl_rtscope_n2a(ifa->ifa_scope, b1, sizeof(b1)), "global") == 0))
    {
        int i;
        for(i=0;i<WAN_MAX_NUM;i++)
        {
            if(strcmp(if_name, sal_wan_get_ipv6_wan_if_t(i)) == 0)
            {
                sal_wan_set_ipv6_open_state_t(i, "1");
                break;
            }
        }
    }

}
