#include <linux/init.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/proc_fs.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#include <sc/sc_spi.h>
#include <linux/slog.h>
MODULE_LICENSE("GPL");

unsigned int __sc_brdcst_src_check_hook(struct sk_buff *skb)
{

	struct iphdr *iph;
    unsigned short sport, dport;
	
    iph = ip_hdr(skb);
    if(nf_conntrack_brdcst_src == 0)
		return NF_ACCEPT;

	if(skb->dev && strncmp(skb->dev->name, "lo", 2) && ipv4_is_lbcast(iph->saddr))
	{
        sport = dport = 0;
        if(skb->len - ip_hdrlen(skb) < 4)
        	return NF_DROP;
        switch (iph->protocol) {
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
		if(iph->protocol == IPPROTO_TCP)
        {
			if (net_ratelimit())
                LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                        "DoS attack: TCP Broadcast as Source Address Attack from source: %u.%u.%u.%u:%hu\n",
                                NIPQUAD(iph->saddr),ntohs(sport));
        }
		else if(iph->protocol == IPPROTO_UDP)
        {
			if (net_ratelimit())
                LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                        "DoS attack: UDP Broadcast as Source Address Attack from source: %u.%u.%u.%u:%hu\n",
                                NIPQUAD(iph->saddr),ntohs(sport));
        }
		else if(iph->protocol == IPPROTO_ICMP)
        {
			if (net_ratelimit())
                LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                        "DoS attack: ICMP Broadcast as Source Address Attack from source: %u.%u.%u.%u\n",
                                NIPQUAD(iph->saddr));
        }
		else		
        {
			if (net_ratelimit())
            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
        	        "DoS attack: Broadcast as Source Address Attack from source: %u.%u.%u.%u\n",
                                    NIPQUAD(iph->saddr));
        }
		return NF_DROP;
	}
	else
		return NF_ACCEPT;
}


static int __init brdcst_init(void)
{
	
	sc_brdcst_src_check_hook = __sc_brdcst_src_check_hook;

	printk("netfilter brdcstSrc dos module loaded \n");
	return 0;
}

static void __exit brdcst_fini(void)
{
	sc_brdcst_src_check_hook = NULL;
	
	printk("netfilter brdcstSrc dos module unloaded \n");
}

module_init(brdcst_init);
module_exit(brdcst_fini);
