/* vi: set sw=4 ts=4: */
/*
 * iproute.c		"ip route".
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
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/in_route.h>
#include <linux/ip_mp_alg.h>
#include <stdarg.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"
#include <sal/sal_wan.h>
#include "log/slog.h"

#ifndef RTAX_RTTVAR
#define RTAX_RTTVAR RTAX_HOPS
#endif

static const char *mx_names[RTAX_MAX+1] = {
	[RTAX_MTU]	= "mtu",
	[RTAX_WINDOW]	= "window",
	[RTAX_RTT]	= "rtt",
	[RTAX_RTTVAR]	= "rttvar",
	[RTAX_SSTHRESH] = "ssthresh",
	[RTAX_CWND]	= "cwnd",
	[RTAX_ADVMSS]	= "advmss",
	[RTAX_REORDERING]="reordering",
	[RTAX_HOPLIMIT] = "hoplimit",
	[RTAX_INITCWND] = "initcwnd",
	[RTAX_FEATURES] = "features",
};

static struct
{
	int tb;
	int cloned;
	int flushed;
	char *flushb;
	int flushp;
	int flushe;
	int protocol, protocolmask;
	int scope, scopemask;
	int type, typemask;
	int tos, tosmask;
	int iif, iifmask;
	int oif, oifmask;
	int realm, realmmask;
	inet_prefix rprefsrc;
	inet_prefix rvia;
	inet_prefix rdst;
	inet_prefix mdst;
	inet_prefix rsrc;
	inet_prefix msrc;
} filter;

static char *mp_alg_names[IP_MP_ALG_MAX+1] = {
	[IP_MP_ALG_NONE] = "none",
	[IP_MP_ALG_RR] = "rr",
	[IP_MP_ALG_DRR] = "drr",
	[IP_MP_ALG_RANDOM] = "random",
	[IP_MP_ALG_WRANDOM] = "wrandom"
};

static int flush_update(void)
{
	if (rtnl_send(&rth, filter.flushb, filter.flushp) < 0) {
		perror("Failed to send flush request\n");
		return -1;
	}
	filter.flushp = 0;
	return 0;
}
static inline int rtm_get_table(struct rtmsg *r, struct rtattr **tb)
{
	__u32 table = r->rtm_table;
	if (tb[RTA_TABLE])
		table = *(__u32*) RTA_DATA(tb[RTA_TABLE]);
	return table;
}

int handle_route(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE*)arg;
	struct rtmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr * tb[RTA_MAX+1];
	char abuf[256];
	inet_prefix dst;
	inet_prefix src;
	inet_prefix prefsrc;
	inet_prefix via;
	int host_len = -1;
	static int ip6_multiple_tables;
	__u32 table;
	SPRINT_BUF(b1);
	static int hz;

	if (n->nlmsg_type != RTM_NEWROUTE && n->nlmsg_type != RTM_DELROUTE) {
		return 0;
	}
	if (filter.flushb && n->nlmsg_type != RTM_NEWROUTE)
		return 0;
	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		return -1;
	}

	if (r->rtm_family == AF_INET6)
		host_len = 128;
	else if (r->rtm_family == AF_INET)
		host_len = 32;
	else if (r->rtm_family == AF_DECnet)
		host_len = 16;
	else if (r->rtm_family == AF_IPX)
		host_len = 80;

	parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
	table = rtm_get_table(r, tb);

	if (r->rtm_family == AF_INET6 && table != RT_TABLE_MAIN)
		ip6_multiple_tables = 1;

	if (r->rtm_family == AF_INET6 && !ip6_multiple_tables) {
		if (filter.cloned) {
			if (!(r->rtm_flags&RTM_F_CLONED))
				return 0;
		}
		if (filter.tb) {
			if (r->rtm_flags&RTM_F_CLONED)
				return 0;
			if (filter.tb == RT_TABLE_LOCAL) {
				if (r->rtm_type != RTN_LOCAL)
					return 0;
			} else if (filter.tb == RT_TABLE_MAIN) {
				if (r->rtm_type == RTN_LOCAL)
					return 0;
			} else {
				return 0;
			}
		}
	} else {
		if (filter.tb > 0 && filter.tb != table)
			return 0;
	}
	if ((filter.protocol^r->rtm_protocol)&filter.protocolmask)
		return 0;
	if ((filter.scope^r->rtm_scope)&filter.scopemask)
		return 0;
	if ((filter.type^r->rtm_type)&filter.typemask)
		return 0;
	if ((filter.tos^r->rtm_tos)&filter.tosmask)
		return 0;
	if (filter.rdst.family &&
	    (r->rtm_family != filter.rdst.family || filter.rdst.bitlen > r->rtm_dst_len))
		return 0;
	if (filter.mdst.family &&
	    (r->rtm_family != filter.mdst.family ||
	     (filter.mdst.bitlen >= 0 && filter.mdst.bitlen < r->rtm_dst_len)))
		return 0;
	if (filter.rsrc.family &&
	    (r->rtm_family != filter.rsrc.family || filter.rsrc.bitlen > r->rtm_src_len))
		return 0;
	if (filter.msrc.family &&
	    (r->rtm_family != filter.msrc.family ||
	     (filter.msrc.bitlen >= 0 && filter.msrc.bitlen < r->rtm_src_len)))
		return 0;
	if (filter.rvia.family && r->rtm_family != filter.rvia.family)
		return 0;
	if (filter.rprefsrc.family && r->rtm_family != filter.rprefsrc.family)
		return 0;

	memset(&dst, 0, sizeof(dst));
	dst.family = r->rtm_family;
	if (tb[RTA_DST])
		memcpy(&dst.data, RTA_DATA(tb[RTA_DST]), (r->rtm_dst_len+7)/8);
	if (filter.rsrc.family || filter.msrc.family) {
		memset(&src, 0, sizeof(src));
		src.family = r->rtm_family;
		if (tb[RTA_SRC])
			memcpy(&src.data, RTA_DATA(tb[RTA_SRC]), (r->rtm_src_len+7)/8);
	}
	if (filter.rvia.bitlen>0) {
		memset(&via, 0, sizeof(via));
		via.family = r->rtm_family;
		if (tb[RTA_GATEWAY])
			memcpy(&via.data, RTA_DATA(tb[RTA_GATEWAY]), host_len/8);
	}
	if (filter.rprefsrc.bitlen>0) {
		memset(&prefsrc, 0, sizeof(prefsrc));
		prefsrc.family = r->rtm_family;
		if (tb[RTA_PREFSRC])
			memcpy(&prefsrc.data, RTA_DATA(tb[RTA_PREFSRC]), host_len/8);
	}

	if (filter.rdst.family && inet_addr_match(&dst, &filter.rdst, filter.rdst.bitlen))
		return 0;
	if (filter.mdst.family && filter.mdst.bitlen >= 0 &&
	    inet_addr_match(&dst, &filter.mdst, r->rtm_dst_len))
		return 0;

	if (filter.rsrc.family && inet_addr_match(&src, &filter.rsrc, filter.rsrc.bitlen))
		return 0;
	if (filter.msrc.family && filter.msrc.bitlen >= 0 &&
	    inet_addr_match(&src, &filter.msrc, r->rtm_src_len))
		return 0;

	if (filter.rvia.family && inet_addr_match(&via, &filter.rvia, filter.rvia.bitlen))
		return 0;
	if (filter.rprefsrc.family && inet_addr_match(&prefsrc, &filter.rprefsrc, filter.rprefsrc.bitlen))
		return 0;
	if (filter.realmmask) {
		__u32 realms = 0;
		if (tb[RTA_FLOW])
			realms = *(__u32*)RTA_DATA(tb[RTA_FLOW]);
		if ((realms^filter.realm)&filter.realmmask)
			return 0;
	}
	if (filter.iifmask) {
		int iif = 0;
		if (tb[RTA_IIF])
			iif = *(int*)RTA_DATA(tb[RTA_IIF]);
		if ((iif^filter.iif)&filter.iifmask)
			return 0;
	}
	if (filter.oifmask) {
		int oif = 0;
		if (tb[RTA_OIF])
			oif = *(int*)RTA_DATA(tb[RTA_OIF]);
		if ((oif^filter.oif)&filter.oifmask)
			return 0;
	}
	if (filter.flushb &&
	    r->rtm_family == AF_INET6 &&
	    r->rtm_dst_len == 0 &&
	    r->rtm_type == RTN_UNREACHABLE &&
	    tb[RTA_PRIORITY] &&
	    *(int*)RTA_DATA(tb[RTA_PRIORITY]) == -1)
		return 0;

	if (filter.flushb) {
		struct nlmsghdr *fn;
		if (NLMSG_ALIGN(filter.flushp) + n->nlmsg_len > filter.flushe) {
			if (flush_update())
				return -1;
		}
		fn = (struct nlmsghdr*)(filter.flushb + NLMSG_ALIGN(filter.flushp));
		memcpy(fn, n, n->nlmsg_len);
		fn->nlmsg_type = RTM_DELROUTE;
		fn->nlmsg_flags = NLM_F_REQUEST;
		fn->nlmsg_seq = ++rth.seq;
		filter.flushp = (((char*)fn) + n->nlmsg_len) - filter.flushb;
		filter.flushed++;
		if (show_stats < 2)
			return 0;
	}

    // detect default router
    if(!tb[RTA_DST] && (0 == r->rtm_dst_len))
    {
        char *if_name = NULL;
        int i;
        if_name = ll_index_to_name(*(int*)RTA_DATA(tb[RTA_OIF]));

        if(n->nlmsg_type == RTM_DELROUTE)
        {
            for(i=0;i<WAN_MAX_NUM;i++)
            {
                if(strcmp(if_name, sal_wan_get_ipv6_wan_if_t(i)) == 0)
                {
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Default route %s has been deleted on WAN%d\n", 
                            format_host(r->rtm_family, RTA_PAYLOAD(tb[RTA_GATEWAY]),
                                RTA_DATA(tb[RTA_GATEWAY]), abuf, sizeof(abuf)), i+1);
                    sal_wan_set_ipv6_gw_exist_t(i, "0");
                    system("/usr/sbin/rc radvd restart");
                    break;
                }
            }
        }
        else if(n->nlmsg_type == RTM_NEWROUTE)
        {
            for(i=0;i<WAN_MAX_NUM;i++)
            {
                if(strcmp(if_name, sal_wan_get_ipv6_wan_if_t(i)) == 0)
                {
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Default route %s has been detected on WAN%d\n", 
                            format_host(r->rtm_family, RTA_PAYLOAD(tb[RTA_GATEWAY]),
                                RTA_DATA(tb[RTA_GATEWAY]), abuf, sizeof(abuf)), i+1);
                    sal_wan_set_ipv6_gw_exist_t(i, "1");
                    system("/usr/sbin/rc radvd restart");
                    break;
                }
            }
        }
        return 0;
    }

    return 0;
}

