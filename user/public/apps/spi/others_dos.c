#include <linux/init.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/proc_fs.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#include <sc/sc_spi.h>
#include <linux/slog.h>
#ifdef CONFIG_IPV6
#include <linux/in6.h>
#include <linux/ipv6.h>
#endif
#include "common.h"
MODULE_LICENSE("GPL");

unsigned int __sc_packet_ip_check_hook(struct sk_buff *skb, u_int8_t pf)
{
    struct iphdr *iph = NULL;
    unsigned short sport, dport;
#ifdef CONFIG_IPV6
    struct ipv6hdr *ipv6h = NULL;
#endif
    u_int8_t proto = 0;

    if(nf_conntrack_ip_land == 0)
		return NF_ACCEPT;

#ifdef CONFIG_IPV6
    if(pf == PF_INET6)
    {
        ipv6h = ipv6_hdr(skb);
        proto = ipv6h->nexthdr;
    }
    else
#endif
    {
        iph = ip_hdr(skb);
        proto = iph->protocol;
    }
    if(pf == PF_INET)
    {
        if(iph->saddr != iph->daddr)
            return NF_ACCEPT;
    }
#ifdef CONFIG_IPV6
    else if(pf == PF_INET6)
    {
        if(0 != memcmp(&(ipv6h->saddr), &(ipv6h->daddr), sizeof(struct in6_addr)))
            return NF_ACCEPT;
    }
#endif
	if(skb->dev && strncmp(skb->dev->name, "lo", 2))
    {
        sport = dport = 0;
        if(skb->len - ip_hdrlen(skb) < 4)
        	return NF_DROP;
        switch (proto) {
        case IPPROTO_TCP: {
            struct tcphdr *tcph = (struct tcphdr *)(skb_network_header(skb) + ip_hdrlen(skb));

            if (ntohs(iph->frag_off) & IP_OFFSET) {
                break;
            }
            sport = ntohs(tcph->source);
            dport = ntohs(tcph->dest);
            break;
        }
        case IPPROTO_UDP: {
            struct udphdr *udph = (struct udphdr *)(skb_network_header(skb) + ip_hdrlen(skb));

            if (ntohs(iph->frag_off) & IP_OFFSET) {
                break;
            }
            sport = ntohs(udph->source);
            dport = ntohs(udph->dest);
            break;
        }
        default:
            break;
        }
        if(proto == IPPROTO_TCP)
        {
            if (net_ratelimit())
            {
#ifdef CONFIG_IPV6
                if(pf == PF_INET6)
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: TCP Land Attack from source: "NIP6_FMT"\n",  
                            NIP6(ipv6h->saddr));
                }
                else
#endif
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: TCP Land Attack from source: %u.%u.%u.%u:%hu\n",
                            NIPQUAD(iph->saddr),ntohs(sport));
                }
            }
        }
        else if(proto == IPPROTO_UDP)
        {
            if (net_ratelimit())
            {
#ifdef CONFIG_IPV6
                if(pf == PF_INET6)
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: UDP Land Attack from source: "NIP6_FMT"\n",  
                            NIP6(ipv6h->saddr));
                }
                else
#endif
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: UDP Land Attack from source: %u.%u.%u.%u:%hu\n",
                            NIPQUAD(iph->saddr),ntohs(sport));
                }
            }
        }
        else if((proto == IPPROTO_ICMP)
#ifdef CONFIG_IPV6
                || (proto == IPPROTO_ICMPV6)
#endif
                )
        {
            if (net_ratelimit())
            {
#ifdef CONFIG_IPV6
                if(pf == PF_INET6)
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: ICMP Land Attack from source: "NIP6_FMT"\n",  
                            NIP6(ipv6h->saddr));
                }
                else
#endif
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: ICMP Land Attack from source: %u.%u.%u.%u\n",
                            NIPQUAD(iph->saddr));
                }
            }
        }
        else		
        {
            if (net_ratelimit())
            {
#ifdef CONFIG_IPV6
                if(pf == PF_INET6)
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: Land Attack from source: "NIP6_FMT"\n",  
                            NIP6(ipv6h->saddr));
                }
                else
#endif
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: Land Attack from source: %u.%u.%u.%u\n",
                            NIPQUAD(iph->saddr));
                }
            }
        }
        return NF_DROP;
    }
    else
        return NF_ACCEPT;
}


static int __init otherdos_init(void)
{
	
	sc_packet_ip_check_hook = __sc_packet_ip_check_hook;

	printk("netfilter other dos module loaded \n");
	return 0;
}

static void __exit otherdos_fini(void)
{
	sc_packet_ip_check_hook = NULL;
	
	printk("netfilter other dos module unloaded \n");
}

module_init(otherdos_init);
module_exit(otherdos_fini);
