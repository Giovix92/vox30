#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "sal_route.h"
#define _PATH_PROCNET_ROUTE  "/proc/net/route"
#ifdef CONFIG_SUPPORT_IPV6
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <linux/in_route.h>
#include <linux/ip_mp_alg.h>

#include "rt_names.h"
#include "utils.h"
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
#define MAX_ROUTE_LINE 1024
static int line = 0;
struct rtnl_handle rth;
static sal_ipv6_route route[SAL_ROUTE_MAX_ENTRY];

static inline int rtm_get_table(struct rtmsg *r, struct rtattr **tb)
{
	__u32 table = r->rtm_table;
	if (tb[RTA_TABLE])
		table = *(__u32*) RTA_DATA(tb[RTA_TABLE]);
	return table;
}
static int rtnl_rtcache_request(struct rtnl_handle *rth, int family)
{
	struct {
		struct nlmsghdr nlh;
		struct rtmsg rtm;
	} req;
	struct sockaddr_nl nladdr;

	memset(&nladdr, 0, sizeof(nladdr));
	memset(&req, 0, sizeof(req));
	nladdr.nl_family = AF_NETLINK;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = RTM_GETROUTE;
	req.nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = rth->dump = ++rth->seq;
	req.rtm.rtm_family = family;
	req.rtm.rtm_flags |= RTM_F_CLONED;

	return sendto(rth->fd, (void*)&req, sizeof(req), 0, (struct sockaddr*)&nladdr, sizeof(nladdr));
}
char *rtnl_rtntype_n2a(int id, char *buf, int len)
{
	switch (id) {
	case RTN_UNSPEC:
		return "none";
	case RTN_UNICAST:
		return "unicast";
	case RTN_LOCAL:
		return "local";
	case RTN_BROADCAST:
		return "broadcast";
	case RTN_ANYCAST:
		return "anycast";
	case RTN_MULTICAST:
		return "multicast";
	case RTN_BLACKHOLE:
		return "blackhole";
	case RTN_UNREACHABLE:
		return "unreachable";
	case RTN_PROHIBIT:
		return "prohibit";
	case RTN_THROW:
		return "throw";
	case RTN_NAT:
		return "nat";
	case RTN_XRESOLVE:
		return "xresolve";
	default:
		snprintf(buf, len, "%d", id);
		return buf;
	}
}
#endif

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001	/* route usable                 */
#define RTF_GATEWAY     0x0002	/* destination is a gateway     */
#define RTF_HOST        0x0004	/* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008	/* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010	/* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020	/* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040	/* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU	/* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080	/* per route window clamping    */
#define RTF_IRTT        0x0100	/* Initial round trip time      */
#define RTF_REJECT      0x0200	/* Reject route                 */
#endif

static const unsigned int flagvals[] = { /* Must agree with flagchars[]. */
    RTF_GATEWAY,
    RTF_HOST,
    RTF_REINSTATE,
    RTF_DYNAMIC,
    RTF_MODIFIED,
#ifdef CONFIG_FEATURE_IPV6
    RTF_DEFAULT,
    RTF_ADDRCONF,
    RTF_CACHE
#endif
};

#define IPV4_MASK (RTF_GATEWAY|RTF_HOST|RTF_REINSTATE|RTF_DYNAMIC|RTF_MODIFIED)
#define IPV6_MASK (RTF_GATEWAY|RTF_HOST|RTF_DEFAULT|RTF_ADDRCONF|RTF_CACHE)

int sal_route_get_route_entry(sal_route** route_info)
{
    FILE *fp = NULL;
    static sal_route route[SAL_ROUTE_MAX_ENTRY];
    char line[200];
    int route_num = 0;
    char *dest, *gw, *dev, *metric, *s, *p;
    int mask_num;
    int i;
    struct in_addr mask;

    fp = popen("/usr/sbin/ip route list table main", "r");
    if (!fp)
        return 0;
   
    while(fgets(line, sizeof(line), fp) && route_num < SAL_ROUTE_MAX_ENTRY)
    {
        memset(&route[route_num], sizeof(route[route_num]), 0);

        s = line;
        if(strstr(s, "local"))
            continue;
        dest = s;
        if(!(p = strchr(dest, ' ')))
            continue;
        *p = '\0';
        s = p + 1;

        strcpy(route[route_num].flags, "U");
        if(*(dest + strspn(dest, "0123456789./")) != '\0')
        {
            if(strcmp(dest, "default"))
                continue;

            /* default here */
            if(strstr(s, "table"))
                continue;
            
            strcpy(route[route_num].dst, "*");
            strcpy(route[route_num].mask, "0.0.0.0");
        }
        else
        {
            if(strstr(s, "table") && strstr(s, "link"))
                continue;

            if((p = strchr(dest, '/')))
            {
                *p = '\0';
                mask_num = atoi(p + 1);
                for(mask.s_addr = 0, i = 0; i < 31; i++)
                {
                    mask.s_addr = (mask.s_addr | (mask_num ? 1 : 0)) << 1;
                    if(mask_num)
                        mask_num--;
                }
                mask.s_addr = htonl(mask.s_addr);
                strcpy(route[route_num].mask, inet_ntoa(mask));
            }
            else
            {
                strcat(route[route_num].flags, "H");
                strcpy(route[route_num].mask, "255.255.255.255");
            }

            strcpy(route[route_num].dst, dest);
        }

        if((p = strstr(s, "via")))
        {
            p += strcspn(p, " ");
            gw = p + strspn(p, " ");
            s = gw + strcspn(gw, " ");
            *s = '\0';
            s++;

            strcpy(route[route_num].gw, gw);
            strcat(route[route_num].flags, "G");
        }
        else
        {
            strcpy(route[route_num].gw, "*");
        }

        if((p = strstr(s, "dev")))
        {
            p += strcspn(p, " ");
            dev = p + strspn(p, " ");
            s = dev + strcspn(dev, " ");
            *s = '\0';
            s++;
            strcpy(route[route_num].dev, dev);
        }
        else
        {
            continue;
        }
        
        if((p = strstr(s, "metric")))
        {
            p += strcspn(p, " ");
            metric = p + strspn(p, " ");
            s = metric + strcspn(metric, " ");
            *s = '\0';
            s++;
            strcpy(route[route_num].metric, metric);
        }
        else
        {
            strcpy(route[route_num].metric, "0");
        }
        
        /* remove reduntant items */
       // for(i = 0; i < route_num; i++)
       // {
       //     if(!memcmp(&route[route_num], &route[i], sizeof(route[route_num])))
       //         break;
       // }
       // if(i == route_num)
            route_num++;
    }

    pclose(fp);
    *route_info = route;
    return route_num;
}
#ifdef CONFIG_SUPPORT_IPV6
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
static int flush_update(void)
{
    if (rtnl_send(&rth, filter.flushb, filter.flushp) < 0) {
        return -1;
    }
    filter.flushp = 0;
    return 0;
}

int show_route(sal_ipv6_route** route_info, const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
    char * _SL_ = NULL;
    int show_stats = 0;
//    FILE *fp = (FILE*)arg;
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

    //if (n->nlmsg_type == RTM_DELROUTE)
    //	fprintf(fp, "Deleted ");
    //if (r->rtm_type != RTN_UNICAST && !filter.type)
    //    fprintf(fp, "%s ", rtnl_rtntype_n2a(r->rtm_type, b1, sizeof(b1)));
    
    if (tb[RTA_DST]) {
        if (r->rtm_dst_len != host_len) {
            sprintf(route[line].dest, "%s/%u ", rt_addr_n2a(r->rtm_family,
                        RTA_PAYLOAD(tb[RTA_DST]),
                        RTA_DATA(tb[RTA_DST]),
                        abuf, sizeof(abuf)),
                    r->rtm_dst_len
                    );
        } else {
            sprintf(route[line].dest, "%s ", format_host(r->rtm_family,
                        RTA_PAYLOAD(tb[RTA_DST]),
                        RTA_DATA(tb[RTA_DST]),
                        abuf, sizeof(abuf))
                    );
        }
    } else if (r->rtm_dst_len) {
        sprintf(route[line].dest, "0/%d ", r->rtm_dst_len);
    } else {
        sprintf(route[line].dest, "default ");
    }
/*    if (tb[RTA_SRC]) {
        if (r->rtm_src_len != host_len) {
            fprintf(fp, "from %s/%u ", rt_addr_n2a(r->rtm_family,
                        RTA_PAYLOAD(tb[RTA_SRC]),
                        RTA_DATA(tb[RTA_SRC]),
                        abuf, sizeof(abuf)),
                    r->rtm_src_len
                    );
        } else {
            fprintf(fp, "from %s ", format_host(r->rtm_family,
                        RTA_PAYLOAD(tb[RTA_SRC]),
                        RTA_DATA(tb[RTA_SRC]),
                        abuf, sizeof(abuf))
                    );
        }
    } else if (r->rtm_src_len) {
        fprintf(fp, "from 0/%u ", r->rtm_src_len);
    }
    if (r->rtm_tos && filter.tosmask != -1) {
        SPRINT_BUF(b1);
        fprintf(fp, "tos %s ", rtnl_dsfield_n2a(r->rtm_tos, b1, sizeof(b1)));
    }

	if (tb[RTA_MP_ALGO]) {
		__u32 mp_alg = *(__u32*) RTA_DATA(tb[RTA_MP_ALGO]);
		if (mp_alg > IP_MP_ALG_NONE) {
			fprintf(fp, "mpath %s ",
			    mp_alg < IP_MP_ALG_MAX ? mp_alg_names[mp_alg] : "unknown");
		}
	}
*/
    if (tb[RTA_GATEWAY] && filter.rvia.bitlen != host_len) {
        sprintf(route[line].gw, "%s ",
                format_host(r->rtm_family,
                    RTA_PAYLOAD(tb[RTA_GATEWAY]),
                    RTA_DATA(tb[RTA_GATEWAY]),
                    abuf, sizeof(abuf)));
    }
    else
    {
        strcpy(route[line].gw, "*");
    }
    if (tb[RTA_OIF] && filter.oifmask != -1)
        sprintf(route[line].dev, "%s ", ll_index_to_name(*(int*)RTA_DATA(tb[RTA_OIF])));
#if 0
    if (!(r->rtm_flags&RTM_F_CLONED)) {
        if (table != RT_TABLE_MAIN && !filter.tb)
            fprintf(fp, " table %s ", rtnl_rttable_n2a(table, b1, sizeof(b1)));
        if (r->rtm_protocol != RTPROT_BOOT && filter.protocolmask != -1)
            fprintf(fp, " proto %s ", rtnl_rtprot_n2a(r->rtm_protocol, b1, sizeof(b1)));
        if (r->rtm_scope != RT_SCOPE_UNIVERSE && filter.scopemask != -1)
            fprintf(fp, " scope %s ", rtnl_rtscope_n2a(r->rtm_scope, b1, sizeof(b1)));
    }
    if (tb[RTA_PREFSRC] && filter.rprefsrc.bitlen != host_len) {
        /* Do not use format_host(). It is our local addr
         * and symbolic name will not be useful.
         * */
        fprintf(fp, " src %s ",
                rt_addr_n2a(r->rtm_family,
                    RTA_PAYLOAD(tb[RTA_PREFSRC]),
                    RTA_DATA(tb[RTA_PREFSRC]),
                    abuf, sizeof(abuf)));
    }
#endif
    if (tb[RTA_PRIORITY])
    {
        sprintf(route[line].metric, "%d ", *(__u32*)RTA_DATA(tb[RTA_PRIORITY]));
        line++;
    }
#if 0
    if (r->rtm_flags & RTNH_F_DEAD)
        fprintf(fp, "dead ");
    if (r->rtm_flags & RTNH_F_ONLINK)
        fprintf(fp, "onlink ");
    if (r->rtm_flags & RTNH_F_PERVASIVE)
        fprintf(fp, "pervasive ");
    if (r->rtm_flags & RTM_F_EQUALIZE)
        fprintf(fp, "equalize ");
    if (r->rtm_flags & RTM_F_NOTIFY)
        fprintf(fp, "notify ");
    
    if (tb[RTA_FLOW] && filter.realmmask != ~0U) {
        __u32 to = *(__u32*)RTA_DATA(tb[RTA_FLOW]);
        __u32 from = to>>16;
        to &= 0xFFFF;
        fprintf(fp, "realm%s ", from ? "s" : "");
        if (from) {
            fprintf(fp, "%s/",
                    rtnl_rtrealm_n2a(from, b1, sizeof(b1)));
        }
        fprintf(fp, "%s ",
                rtnl_rtrealm_n2a(to, b1, sizeof(b1)));
    }
    if ((r->rtm_flags&RTM_F_CLONED) && r->rtm_family == AF_INET) {
        __u32 flags = r->rtm_flags&~0xFFFF;
        int first = 1;
        
        fprintf(fp, "%s    cache ", _SL_);

#define PRTFL(fl,flname) if (flags&RTCF_##fl) { \
  flags &= ~RTCF_##fl; \
  fprintf(fp, "%s" flname "%s", first ? "<" : "", flags ? "," : "> "); \
  first = 0; }
		PRTFL(LOCAL, "local");
		PRTFL(REJECT, "reject");
		PRTFL(MULTICAST, "mc");
		PRTFL(BROADCAST, "brd");
		PRTFL(DNAT, "dst-nat");
		PRTFL(SNAT, "src-nat");
		PRTFL(MASQ, "masq");
		PRTFL(DIRECTDST, "dst-direct");
		PRTFL(DIRECTSRC, "src-direct");
		PRTFL(REDIRECTED, "redirected");
		PRTFL(DOREDIRECT, "redirect");
		PRTFL(FAST, "fastroute");
		PRTFL(NOTIFY, "notify");
		PRTFL(TPROXY, "proxy");
#ifdef RTCF_EQUALIZE
		PRTFL(EQUALIZE, "equalize");
#endif
		if (flags)
			fprintf(fp, "%s%x> ", first ? "<" : "", flags);
		if (tb[RTA_CACHEINFO]) {
			struct rta_cacheinfo *ci = RTA_DATA(tb[RTA_CACHEINFO]);
			if (!hz)
				hz = get_user_hz();
			if (ci->rta_expires != 0)
				fprintf(fp, " expires %dsec", ci->rta_expires/hz);
			if (ci->rta_error != 0)
				fprintf(fp, " error %d", ci->rta_error);
			if (show_stats) {
				if (ci->rta_clntref)
					fprintf(fp, " users %d", ci->rta_clntref);
				if (ci->rta_used != 0)
					fprintf(fp, " used %d", ci->rta_used);
				if (ci->rta_lastuse != 0)
					fprintf(fp, " age %dsec", ci->rta_lastuse/hz);
			}
#ifdef RTNETLINK_HAVE_PEERINFO
			if (ci->rta_id)
				fprintf(fp, " ipid 0x%04x", ci->rta_id);
			if (ci->rta_ts || ci->rta_tsage)
				fprintf(fp, " ts 0x%x tsage %dsec", ci->rta_ts, ci->rta_tsage);
#endif
		}
	} else if (r->rtm_family == AF_INET6) {
		struct rta_cacheinfo *ci = NULL;
		if (tb[RTA_CACHEINFO])
			ci = RTA_DATA(tb[RTA_CACHEINFO]);
		if ((r->rtm_flags & RTM_F_CLONED) || (ci && ci->rta_expires)) {
			if (!hz)
				hz = get_user_hz();
			if (r->rtm_flags & RTM_F_CLONED)
				fprintf(fp, "%s    cache ", _SL_);
			if (ci->rta_expires)
				fprintf(fp, " expires %dsec", ci->rta_expires/hz);
			if (ci->rta_error != 0)
				fprintf(fp, " error %d", ci->rta_error);
			if (show_stats) {
				if (ci->rta_clntref)
					fprintf(fp, " users %d", ci->rta_clntref);
				if (ci->rta_used != 0)
					fprintf(fp, " used %d", ci->rta_used);
				if (ci->rta_lastuse != 0)
					fprintf(fp, " age %dsec", ci->rta_lastuse/hz);
			}
		} else if (ci) {
			if (ci->rta_error != 0)
				fprintf(fp, " error %d", ci->rta_error);
		}
	}
	if (tb[RTA_METRICS]) {
		int i;
		unsigned mxlock = 0;
		struct rtattr *mxrta[RTAX_MAX+1];

		parse_rtattr(mxrta, RTAX_MAX, RTA_DATA(tb[RTA_METRICS]),
			    RTA_PAYLOAD(tb[RTA_METRICS]));
		if (mxrta[RTAX_LOCK])
			mxlock = *(unsigned*)RTA_DATA(mxrta[RTAX_LOCK]);

		for (i=2; i<= RTAX_MAX; i++) {
			if (mxrta[i] == NULL)
				continue;
			if (!hz)
				hz = get_hz();

			if (i < sizeof(mx_names)/sizeof(char*) && mx_names[i])
				fprintf(fp, " %s", mx_names[i]);
			else
				fprintf(fp, " metric %d", i);
			if (mxlock & (1<<i))
				fprintf(fp, " lock");

			if (i != RTAX_RTT && i != RTAX_RTTVAR)
				fprintf(fp, " %u", *(unsigned*)RTA_DATA(mxrta[i]));
			else {
				unsigned val = *(unsigned*)RTA_DATA(mxrta[i]);

				val *= 1000;
				if (i == RTAX_RTT)
					val /= 8;
				else
					val /= 4;
				if (val >= hz)
					fprintf(fp, " %ums", val/hz);
				else
					fprintf(fp, " %.2fms", (float)val/hz);
			}
		}
	}
	if (tb[RTA_IIF] && filter.iifmask != -1) {
		fprintf(fp, " iif %s", ll_index_to_name(*(int*)RTA_DATA(tb[RTA_IIF])));
	}
	if (tb[RTA_MULTIPATH]) {
		struct rtnexthop *nh = RTA_DATA(tb[RTA_MULTIPATH]);
		int first = 0;

		len = RTA_PAYLOAD(tb[RTA_MULTIPATH]);

		for (;;) {
			if (len < sizeof(*nh))
				break;
			if (nh->rtnh_len > len)
				break;
			if (r->rtm_flags&RTM_F_CLONED && r->rtm_type == RTN_MULTICAST) {
				if (first)
					fprintf(fp, " Oifs:");
				else
					fprintf(fp, " ");
			} else
				fprintf(fp, "%s\tnexthop", _SL_);
			if (nh->rtnh_len > sizeof(*nh)) {
				parse_rtattr(tb, RTA_MAX, RTNH_DATA(nh), nh->rtnh_len - sizeof(*nh));
				if (tb[RTA_GATEWAY]) {
					fprintf(fp, " via %s ",
						format_host(r->rtm_family,
							    RTA_PAYLOAD(tb[RTA_GATEWAY]),
							    RTA_DATA(tb[RTA_GATEWAY]),
							    abuf, sizeof(abuf)));
				}
				if (tb[RTA_FLOW]) {
					__u32 to = *(__u32*)RTA_DATA(tb[RTA_FLOW]);
					__u32 from = to>>16;
					to &= 0xFFFF;
					fprintf(fp, " realm%s ", from ? "s" : "");
					if (from) {
						fprintf(fp, "%s/",
							rtnl_rtrealm_n2a(from, b1, sizeof(b1)));
					}
					fprintf(fp, "%s",
						rtnl_rtrealm_n2a(to, b1, sizeof(b1)));
				}
			}
			if (r->rtm_flags&RTM_F_CLONED && r->rtm_type == RTN_MULTICAST) {
				fprintf(fp, " %s", ll_index_to_name(nh->rtnh_ifindex));
				if (nh->rtnh_hops != 1)
					fprintf(fp, "(ttl>%d)", nh->rtnh_hops);
			} else {
				fprintf(fp, " dev %s", ll_index_to_name(nh->rtnh_ifindex));
				fprintf(fp, " weight %d", nh->rtnh_hops+1);
			}
			if (nh->rtnh_flags & RTNH_F_DEAD)
				fprintf(fp, " dead");
			if (nh->rtnh_flags & RTNH_F_ONLINK)
				fprintf(fp, " onlink");
			if (nh->rtnh_flags & RTNH_F_PERVASIVE)
				fprintf(fp, " pervasive");
			len -= NLMSG_ALIGN(nh->rtnh_len);
			nh = RTNH_NEXT(nh);
		}
	}
	fprintf(fp, "\n");
	fflush(fp);
#endif
	return 0;
}
void iproute_reset_filter()
{
    memset(&filter, 0, sizeof(filter));
    filter.mdst.bitlen = -1;
    filter.msrc.bitlen = -1;
}
int sal_route_get_ipv6_route_entry(int preferred_family, sal_ipv6_route** route_info)
{
    // AF_INET6
    int do_ipv6 = preferred_family;
    unsigned groups = 0;

    if (!preferred_family || preferred_family == AF_INET)
        groups |= nl_mgrp(RTNLGRP_IPV4_ROUTE);
    if (!preferred_family || preferred_family == AF_INET6)
        groups |= nl_mgrp(RTNLGRP_IPV6_ROUTE);

    iproute_reset_filter();
    filter.tb = RT_TABLE_MAIN;

    if (do_ipv6 == AF_UNSPEC && filter.tb)
        do_ipv6 = AF_INET;
    line = 0;

    rtnl_open(&rth, groups);
    ll_init_map(&rth);

    if (!filter.cloned) {
        if (rtnl_wilddump_request(&rth, do_ipv6, RTM_GETROUTE) < 0) {
            return 0;
        }
    } else {
        if (rtnl_rtcache_request(&rth, do_ipv6) < 0) {
            return 0;
        }
    }

    if (rtnl_dump_filter2(route_info, &rth, show_route, NULL, NULL, NULL) < 0) {
        return 0;
    }

	rtnl_close(&rth);
	*route_info = route;
    return line;
    //exit(0);
}
#endif
