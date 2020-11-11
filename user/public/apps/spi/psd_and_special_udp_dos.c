#include <linux/init.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/tcp.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <sc/sc_spi.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include "common.h"
#include <linux/slog.h>
#include <linux/param.h>
#include <linux/types.h>
#include <linux/version.h>
#ifdef CONFIG_IPV6
#include <linux/in6.h>
#include <linux/ipv6.h>
#endif
//#define IPPROTO_BOTH  255
MODULE_LICENSE("GPL");

#define SUPPORT_ECHO_CHARGEN_DETECT
#ifdef	SUPPORT_ECHO_CHARGEN_DETECT

#define MAX_ECHO_CHARGEN	100
#define MAX_USER_SET	4

#define ECHO_PORT		7
#define CHARGEN_PORT	19


#define UDP_ECHO		0
#define UDP_CHARGEN		1
#define TCP_ECHO		2
#define TCP_CHARGEN		3
#define UDP_TCP_ECHO	0
#define UDP_TCP_CHARGEN	1

#define TIMEOUT_PERIOD  (1*HZ)

static int dmz = 0;
static int timeout = TIMEOUT_PERIOD;  
struct user_set
{
	u_int8_t 	proto_user;
	u_int16_t 	port_user;		    /* network byte order*/
	queue_t		queue;
};

#endif

/*
 * State information.
 */
static struct {
	spinlock_t lock;
#ifdef	SUPPORT_ECHO_CHARGEN_DETECT
	struct user_set user_set[MAX_USER_SET];
	unsigned long echo_udp[MAX_ECHO_CHARGEN];
	unsigned long chargen_udp[MAX_ECHO_CHARGEN];
	unsigned long echo_tcp[MAX_ECHO_CHARGEN];
	unsigned long chargen_tcp[MAX_ECHO_CHARGEN];
#endif
} state;

unsigned int __sc_psd_and_special_udp_detect_hook(struct sk_buff *skb, struct nf_conn *ct, u_int8_t protonum, unsigned int dataoff)

{
	struct iphdr *iph = NULL;
	struct in_addr addr;
	u_int16_t src_port;
	unsigned long now;
	u_int8_t proto;
	unsigned int ret = NF_ACCEPT;
	struct nf_conntrack_tuple tuple;
	struct nf_conntrack_tuple mask;
	u_int8_t pf = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
        struct udphdr *u_hp;
        struct udphdr u_hdr;
        struct tcphdr *t_hp;
        struct tcphdr t_hdr;
        queue_t *action_queue;

#ifdef CONFIG_IPV6
        struct ipv6hdr *ipv6h = NULL;
        struct in6_addr addr6;
#endif

	if((pf != PF_INET)
#ifdef CONFIG_IPV6
            && (pf != PF_INET6)
#endif
            )
	{
		DEBUG("psd just support ipv4 now\n");
		return ret;
	}
	
	if(ct->master)
	{
		DEBUG("psd return, because it is a ALG expect packet.\n");
		return ret;
	}

        if((nf_ct_tcp_port_scan_enable == 0 && nf_ct_udp_port_scan_enable == 0
            && nf_ct_udp_flood_enable == 0)
            || ((skb->dev->priv_flags & IFF_WANDEV) == 0)
            || ((skb->dev->dos_flags & IFF_DOS_ENABLE) == 0))
	{
		return ret;
	}
	
        addr.s_addr = 0;
	/* IP header */
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
           /* Sanity check */
           if (ntohs(iph->frag_off) & IP_OFFSET) {
            DEBUG("PSD: sanity check failed\n");
               return ret;
           }
           /* TCP or UDP ? */
           proto = iph->protocol;
    }
    if (proto != IPPROTO_TCP && proto != IPPROTO_UDP) {
        DEBUG("PSD: protocol not supported\n");
        return ret;
    }

    /* Get the source address, source & destination ports, and TCP flags */
#ifdef CONFIG_IPV6
    if(pf == PF_INET6)
    {
        memcpy(&addr6, &ipv6h->saddr, sizeof(struct in6_addr));
    }
    else
#endif
    {
        addr.s_addr = iph->saddr;
        /* We're using IP address 0.0.0.0 for a special purpose here, so don't let
         * them spoof us. [DHCP needs this feature - HW] */
        if (!addr.s_addr) {
            DEBUG("PSD: spoofed source address (0.0.0.0)\n");
            return ret;
        }
    }

    memcpy(&tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, sizeof(tuple));
    if(proto == IPPROTO_UDP)
    {
        u_hp = skb_header_pointer(skb, dataoff, sizeof(u_hdr), &u_hdr);
        if(u_hp)
        {
            src_port = u_hp->source;
        }
	else 
            return ret;
    }
    else
    {
        t_hp = skb_header_pointer(skb, dataoff, sizeof(t_hdr), &t_hdr);
        if(t_hp)
        {
            src_port = t_hp->source;
        }
        else 
            return ret;
    }

	/* Use jiffies here not to depend on someone setting the time while we're
	 * running; we need to be careful with possible return value overflows. */
	now = jiffies;
	spin_lock_bh(&state.lock);

#ifdef SUPPORT_ECHO_CHARGEN_DETECT
	//  udp echo and chargen detect is embedded in udp_tcp_port_scan module
	if(nf_ct_udp_flood_enable == 1 && ((src_port == htons(ECHO_PORT)) 
             || (src_port == htons(CHARGEN_PORT)))
            )
	{
	    /*  udp echo and chargen detect */
		if(proto == IPPROTO_UDP)
		{
			if(src_port  == htons(ECHO_PORT))
				action_queue = &state.user_set[UDP_ECHO].queue;
			else
				action_queue = &state.user_set[UDP_CHARGEN].queue;

		//	ct->need_recheck = 1;
						
			if(enqueue_and_check(action_queue, now, timeout))
			{
				if(sc_add_block_pattern_hook)
				{
					tuple.dst.protonum = IPPROTO_UDP;

					memset(&mask, 0, sizeof(mask));
                    memset(&mask.src.u3.all, 0xFF, sizeof(mask.src.u3.all));
                    mask.dst.protonum = 0xFF;
					sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, nf_conntrack_block_time);
                    if (net_ratelimit())
                    {
#ifdef CONFIG_IPV6
                        if(pf == PF_INET6)
                        {
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                    "DoS attack: UDP Flood Attack from source: "NIP6_FMT"\n",  
                                    NIP6(ipv6h->saddr));
                        }
                        else
#endif
                        {
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                    "DoS attack: UDP Flood Attack from source: %u.%u.%u.%u:%hu\n",
                                    NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all));
                        }
                    }
				}
				ret = NF_DROP;
			}
		}
	        /* tcp echo and chargen detect */
                else if(proto == IPPROTO_TCP)
		{
			if(src_port  == htons(ECHO_PORT))
				action_queue = &state.user_set[TCP_ECHO].queue;
			else
				action_queue = &state.user_set[TCP_CHARGEN].queue;

			if(enqueue_and_check(action_queue, now, timeout))
			{
				if(sc_add_block_pattern_hook)
				{
					tuple.dst.protonum = IPPROTO_TCP;

					memset(&mask, 0, sizeof(mask));
                                        memset(&mask.src.u3.all, 0xFF, sizeof(mask.src.u3.all));
                                        mask.dst.protonum = 0xFF;
					sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, nf_conntrack_block_time);
                                        if (net_ratelimit())
                                        {
#ifdef CONFIG_IPV6
                                            if(pf == PF_INET6)
                                            {
                                                LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                                 "DoS attack: Echo/Chargen Attack from source: "NIP6_FMT"\n", NIP6(ipv6h->saddr));
                                            }
                                            else
#endif
                                            {
                                                LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                                "DoS attack: Echo/Chargen Attack from source: %u.%u.%u.%u:%hu\n",
                                                NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all));
                                            }
                                        }
				}
				ret = NF_DROP;
			}
		}
	}

#endif
	spin_unlock_bh(&state.lock);
	return ret;
}

#ifdef SUPPORT_ECHO_CHARGEN_DETECT

static ssize_t proc_write_echo_chargen (struct file *file, const char __user *input,
				size_t size, loff_t *ofs){
	char buffer[64];
	int len, i;

	if(size > sizeof(buffer))
		len = sizeof(buffer);
	else
		len = size;

	memset(buffer, 0, sizeof(buffer));
	if(copy_from_user(buffer,input,len))
		return -EFAULT;

	buffer[len - 1] = '\0';

	printk(KERN_INFO " proc_write_echo_chargen receive: len: %d, input: %s\n",len, buffer);

	spin_lock_bh(&state.lock);
	
	/* Check if we are asked to flush the entire table */
	if(!memcmp(buffer,"clear",5)) {
		dmz = 0;
		printk(KERN_INFO " proc_write_echo_chargen receive: clear list\n");
		for(i = 0; i < MAX_USER_SET; i++)
		{
			state.user_set[i].proto_user = 0;
			state.user_set[i].port_user = 0;
			state.user_set[i].queue.base = NULL;
			
		}
	}
    else if(!memcmp(buffer,"dirq",4)) {
        local_irq_disable();
        while(1);
	    return size;	
    }
    else if(!memcmp(buffer,"dirqs",5)) {
        local_irq_disable();
        schedule();
	    return size;	
    }
    else if(!memcmp(buffer,"dsirq",5)) {
        while(1);
	    return size;	
    }
    else if(!memcmp(buffer,"dsirqs",6)) {
        schedule();
	    return size;	
    }
	else if (!memcmp(buffer,"dmz",3))
	{
		dmz = 1;
		/*clear first*/
		for(i = 0; i < MAX_USER_SET; i++)
		{
			state.user_set[i].proto_user = 0;
			state.user_set[i].port_user = 0;
			state.user_set[i].queue.base = NULL;
			
		}

		state.user_set[UDP_TCP_ECHO].proto_user = IPPROTO_BOTH/*IPPROTO_UDP | IPPROTO_TCP*/;
		state.user_set[UDP_TCP_ECHO].port_user = __constant_htons(ECHO_PORT);
		queue_init(&state.user_set[UDP_TCP_ECHO].queue,  &state.echo_udp[0], MAX_ECHO_CHARGEN);

		state.user_set[UDP_TCP_CHARGEN].proto_user = IPPROTO_BOTH/*IPPROTO_UDP | IPPROTO_TCP*/;
		state.user_set[UDP_TCP_CHARGEN].port_user = __constant_htons(CHARGEN_PORT);
		queue_init(&state.user_set[UDP_TCP_CHARGEN].queue,  &state.echo_udp[0], MAX_ECHO_CHARGEN);

		printk(KERN_INFO " proc_write_echo_chargen receive: dmz\n");
	}
	else if(!dmz)
	{
		char protocol[16] = "";
		int  port = 0;

		sscanf(buffer, "%s %d", protocol, &port);

		if((strncasecmp(protocol, "udp", strlen("udp"))
		   		&& strncasecmp(protocol, "tcp", strlen("tcp"))
		   		&& strncasecmp(protocol, "tcp/udp", strlen("tcp/udp")))
		   || (port < 0 || port > 65535))
		{
			printk("input format is wrong.\n");
			printk("[protocol] [port_number]\n");
			printk("\tprotocol: udp tcp tcp/udp\n");
			printk("\tport_number: 7 or 19\n");
			goto end;
		}

		if((strncasecmp(protocol, "tcp/udp", strlen("tcp/udp")) == 0) && port == (ECHO_PORT))
		{
			printk("UDP_TCP_ECHO queue\n");
			state.user_set[UDP_TCP_ECHO].proto_user = IPPROTO_BOTH/*(IPPROTO_UDP|| IPPROTO_TCP)*/;
			state.user_set[UDP_TCP_ECHO].port_user = __constant_htons(ECHO_PORT);
			queue_init(&state.user_set[UDP_TCP_ECHO].queue,  &state.echo_udp[0], MAX_ECHO_CHARGEN);
		}
		else if((strncasecmp(protocol, "tcp/udp", strlen("tcp/udp")) == 0) && port == (CHARGEN_PORT))
		{
			printk("UDP_TCP_CHARGEN queue\n");
			state.user_set[UDP_TCP_CHARGEN].proto_user = IPPROTO_BOTH/*(IPPROTO_UDP|| IPPROTO_TCP)*/;
			state.user_set[UDP_TCP_CHARGEN].port_user = __constant_htons(CHARGEN_PORT);
			queue_init(&state.user_set[UDP_TCP_CHARGEN].queue,  &state.chargen_udp[0], MAX_ECHO_CHARGEN);
		}
		else if((strncasecmp(protocol, "udp", strlen("udp")) == 0) && port == (ECHO_PORT))
		{
			printk("UDP_ECHO queue\n");
			state.user_set[UDP_ECHO].proto_user = IPPROTO_UDP;
			state.user_set[UDP_ECHO].port_user = __constant_htons(ECHO_PORT);
			queue_init(&state.user_set[UDP_ECHO].queue,  &state.echo_udp[0], MAX_ECHO_CHARGEN);
		}
		else if((strncasecmp(protocol, "udp", strlen("udp")) == 0) && port == (CHARGEN_PORT))
		{
			printk("UDP_CHARGEN queue\n");
			state.user_set[UDP_CHARGEN].proto_user = IPPROTO_UDP;
			state.user_set[UDP_CHARGEN].port_user = __constant_htons(CHARGEN_PORT);
			queue_init(&state.user_set[UDP_CHARGEN].queue,  &state.chargen_udp[0], MAX_ECHO_CHARGEN);

		}
		else if((strncasecmp(protocol, "tcp", strlen("tcp")) == 0) && port == (ECHO_PORT))
		{
			printk("TCP_ECHO queue\n");
			state.user_set[TCP_ECHO].proto_user = IPPROTO_TCP;
			state.user_set[TCP_ECHO].port_user = __constant_htons(ECHO_PORT);
			queue_init(&state.user_set[TCP_ECHO].queue,  &state.echo_tcp[0], MAX_ECHO_CHARGEN);
		}
		else if((strncasecmp(protocol, "tcp", strlen("tcp")) == 0) && port == (CHARGEN_PORT))
		{
			printk("TCP_CHARGEN queue\n");
			state.user_set[TCP_CHARGEN].proto_user = IPPROTO_TCP;
			state.user_set[TCP_CHARGEN].port_user = __constant_htons(CHARGEN_PORT);
			queue_init(&state.user_set[TCP_CHARGEN].queue,  &state.chargen_tcp[0], MAX_ECHO_CHARGEN);

		}
		else
		{
			printk("something is wrong when setting\n");
			printk("valid input is:\n");
			printk("[clear | dmz | (udp|tcp|tcp/udp) (port)]\n");
		}

	}
	else
	{
		printk(KERN_ERR "Can not accept this command(%s) (dmz = %d)\n", buffer, dmz);
		printk("valid input is:\n");
		printk("[clear | dmz | (udp|tcp|tcp/udp) (port)]\n");
	}

	printk(KERN_INFO " Leaving proc_write_echo_chargen : size: %u\n", size);

end:	
	spin_unlock_bh(&state.lock);

	return size;	
}

struct file_operations proc_echo_chargen_fops =
{
	.owner 	= THIS_MODULE,
	.write 	= proc_write_echo_chargen,
};

#endif

static int __init specialdos_init(void)
{
	int i;
    int threshhold;
#ifdef SUPPORT_ECHO_CHARGEN_DETECT
	struct proc_dir_entry *proc;
#endif

	memset(&state, 0, sizeof(state));

	spin_lock_init(&(state.lock));

#ifdef SUPPORT_ECHO_CHARGEN_DETECT
	proc = proc_create ("echo_chargen_config", 0, init_net.proc_net, &proc_echo_chargen_fops);

	if (!proc) 
	{
		printk("echo_chargen_config  proc create failed\n");
	}
    /*clear first*/
    for(i = 0; i < MAX_USER_SET; i++)
    {
        state.user_set[i].proto_user = 0;
        state.user_set[i].port_user = 0;
        state.user_set[i].queue.base = NULL;

    }

    if(nf_ct_udp_flood_speed > MAX_ECHO_CHARGEN)
    {
        timeout = MAX_ECHO_CHARGEN * HZ / nf_ct_udp_flood_speed;
        threshhold = MAX_ECHO_CHARGEN; 
    }
    else
    {
        threshhold = nf_ct_udp_flood_speed; 
    }
    state.user_set[UDP_ECHO].proto_user = IPPROTO_UDP;
    state.user_set[UDP_ECHO].port_user = __constant_htons(ECHO_PORT);
    queue_init(&state.user_set[UDP_ECHO].queue,  &state.echo_udp[0], threshhold);
    state.user_set[UDP_CHARGEN].proto_user = IPPROTO_UDP;
    state.user_set[UDP_CHARGEN].port_user = __constant_htons(CHARGEN_PORT);
    queue_init(&state.user_set[UDP_CHARGEN].queue,  &state.chargen_udp[0], threshhold);
    state.user_set[TCP_ECHO].proto_user = IPPROTO_TCP;
    state.user_set[TCP_ECHO].port_user = __constant_htons(ECHO_PORT);
    queue_init(&state.user_set[TCP_ECHO].queue,  &state.echo_tcp[0], threshhold);
    state.user_set[TCP_CHARGEN].proto_user = IPPROTO_TCP;
    state.user_set[TCP_CHARGEN].port_user = __constant_htons(CHARGEN_PORT);
    queue_init(&state.user_set[TCP_CHARGEN].queue,  &state.chargen_tcp[0], threshhold);
#endif
	
	sc_psd_and_special_udp_detect_hook = __sc_psd_and_special_udp_detect_hook;
	return 0;
}

static void __exit specialdos_fini(void)
{

#ifdef SUPPORT_ECHO_CHARGEN_DETECT
	remove_proc_entry("echo_chargen_config", init_net.proc_net);
#endif
	sc_psd_and_special_udp_detect_hook = NULL;
}

module_init(specialdos_init);
module_exit(specialdos_fini);
