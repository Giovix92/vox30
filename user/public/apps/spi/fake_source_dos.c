
#include <linux/init.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/param.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/spinlock.h>
#include <sc/sc_spi.h>
#include <linux/slog.h>
#include "common.h"


MODULE_LICENSE("GPL");
#define	TIMEOUT_PERIOD	(10*HZ)
    
#define FAKE_SOURCE_ENTRY	51

#define FAKE_SOURCE_BLOCK_TIME (1*60*HZ)

unsigned long icmp_entry[FAKE_SOURCE_ENTRY] = {0};
unsigned long special_udp_entry[FAKE_SOURCE_ENTRY] = {0};

queue_t icmp_queue;
queue_t special_udp_queue;
spinlock_t	queue_lock;

#if 0
/*gordon add count all fregment */
int count_freg(int *icmp, int * echo_chargen)
{
	struct ipq *qp;
	struct hlist_node *n;
	unsigned long now = jiffies

	read_lock(&ipfrag_lock);
	for (i = 0; i < IPQ_HASHSZ; i++) {
		struct hlist_node *p, *n;

		hlist_for_each_entry_safe(qp, p, n, &ipq_hash[i], list) {
		
			spin_lock_bh(&qp->lock);
			if(qp->protocol == icmp && qp->be_count == 1
				&& (now - qp->ts <= 10*HZ 
				    && time_after_eq(now, curr->timestamp)))
				icmp += atomic_read(&qp->refcnt);
			else
				qp->be_count = 0;
			if(qp->protocol == tcp/udp && qp->be_count == 1)
				&& (now - qp->ts <= 10*HZ 
				    && time_after_eq(now, curr->timestamp)))
				echo_chargen += atomic_read(&qp->refcnt);
			else
				qp->be_count = 0;
				
			spin_unlock_bh(&qp->lock);
		}
		read_unlock(&ipfrag_lock)
		return 0;
		
	}
	
	read_unlock(&ipfrag_lock)
	return 0;
	
}
#endif

unsigned int __sc_fake_source_detect_hook(struct sk_buff *skb, struct nf_conn *ct, u_int8_t protonum)
{
//	int icmp_count_total = 0;
//	int special_udp_count_total = 0;
//	int icmp_no_freg_count = 0, icmp_freg_count = 0;
//	int special_udp_no_freg_count = 0, special_udp_freg_count = 0;

	struct nf_conntrack_tuple tuple;
	struct nf_conntrack_tuple mask;
	int ret = NF_ACCEPT;

	
	unsigned long now = jiffies;
		
//	count_freg(&icmp_freg_count, &special_udp_freg_count);
	
	spin_lock_bh(&queue_lock);
	if(skb && (protonum == IPPROTO_ICMP || protonum == IPPROTO_UDP))
	{
		/* As some existed conntrack need block, some needn't block */
		ct->need_recheck = 1;
		memcpy(&tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, sizeof(tuple));
		if (protonum == IPPROTO_ICMP)
		{
			if(enqueue_and_check(&icmp_queue, now, TIMEOUT_PERIOD))
			{
//				block 1;
				if(sc_add_block_pattern_hook)
				{
					memset(&mask, 0, sizeof(mask));
					mask.dst.protonum = 0xFF;
					sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, nf_conntrack_block_time);
                    LOG_FIREWALL(KERN_WARNING,NORM_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                "DoS attack: ICMP Flood from source: %u.%u.%u.%u\n", 
                                                NIPQUAD((tuple.src.u3.ip)));
					ret = NF_DROP;
					goto end;
				}
			}
			
		}
		else if(protonum == IPPROTO_UDP)
		{
			__u16	sport = tuple.src.u.udp.port;
			__u16	dport = tuple.dst.u.udp.port;
			if((sport == htons(7) && dport == htons(19))
			    || (sport == htons(19) && dport == htons(7)))
//			if(dport == htons(7) || dport == htons(19))
			{
				if(enqueue_and_check(&special_udp_queue, now, TIMEOUT_PERIOD))
				{
//					block 2;
					if(sc_add_block_pattern_hook)
					{
						tuple.src.u.udp.port = htons(7);
						memset(&mask, 0, sizeof(mask));
						mask.src.u.udp.port = 0xFFFF;
						mask.dst.protonum = 0xFF;
						sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, nf_conntrack_block_time);
						tuple.src.u.udp.port = htons(19);
						sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, nf_conntrack_block_time);
                        LOG_FIREWALL(KERN_WARNING,NORM_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
						            "DoS attack: Echo/Chargen Attack from source: %u.%u.%u.%u:%hu\n",
                                                    NIPQUAD((tuple.src.u3.ip)),ntohs(sport));
						ret = NF_DROP;
						goto end;
					}
				}
			}
		}

		
	}

#if 0	
	icmp_no_freg_count = queue_len(&icmp_queue);
	special_udp_no_freg_count = queue_len(&special_udp_queue);
	
	spin_unlock_bh(&queue_lock);
	
	if(icmp_no_freg_count > 50 || (icmp_freg_count +  icmp_no_freg_count) > 1000)
	{
				if(sc_add_block_pattern_hook)
				{
					mask = ((struct ip_conntrack_tuple)
							{ { 0x0, { 0 } },
		  					{ 0x0, { .icmp = { 0x0, 0x0 } }, 0xFF }});
					sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, 1*60*HZ);
				}
	}
	
	if(special_udp_no_freg_count > 50 || (special_udp_freg_count +  special_udp_no_freg_count) > 1000)
	{
				if(sc_add_block_pattern_hook)
				{
					tuple.dst.u.udp.port = htons(7);
					mask = ((struct ip_conntrack_tuple)
							{ { 0x0, { 0 } },
		  					{ 0x0, { .udp = { 0xFFFF } }, 0xFF }});
					sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, 1*60*HZ);
					tuple.dst.u.udp.port = htons(19);
					sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, 1*60*HZ);
				}
	}

	return;
#endif
	
end:
	spin_unlock_bh(&queue_lock);
	return ret;		
	
}

static int sourcedos_init(void)
{
	
	/* init icmp queue */
	icmp_queue.base = &icmp_entry[0];
	icmp_queue.rear = icmp_queue.front = 0;
	icmp_queue.size = FAKE_SOURCE_ENTRY;
	
	/* init special udp queue */
	special_udp_queue.base = &special_udp_entry[0];
	special_udp_queue.rear = special_udp_queue.front = 0;
	special_udp_queue.size = FAKE_SOURCE_ENTRY;
	
	spin_lock_init(&queue_lock);

	sc_fake_source_detect_hook = __sc_fake_source_detect_hook;
	return 0;
}

void sourcedos_exit(void)
{
	sc_fake_source_detect_hook = NULL;
}

module_init(sourcedos_init);
module_exit(sourcedos_exit);
