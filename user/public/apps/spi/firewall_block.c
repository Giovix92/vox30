#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/spinlock.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/ppp_defs.h>
#include <sc/sc_spi.h>
#include "common.h"

#define ENABLE_QUICK_FILTER_SOURCE 1 //check_block_src_protonum
//#define SEQ_OPERATION
MODULE_LICENSE("GPL");

struct action{
	struct list_head list;
	struct nf_conntrack_tuple	match_tuple, mask;
	unsigned long ts;
	int type; /* 0 or 1: 0 means black list, 1 means white list */
	__u16 dport_min, dport_max; /* host format */
	unsigned long count;
};

spinlock_t	action_lock;
static struct timer_list timeout_action;

struct list_head action_list = LIST_HEAD_INIT(action_list);

/****************************************************************
	** every packet from internet should be enter this place.
	** INPUT parameters
	**		tuple -- incoming packet's tuple, which will be 
	**				 compared with action list.
	** RETURN : 0 or 1
	** 		0 means do nothing.
	**		1 means drop this packet
****************************************************************/
int __sc_check_block_src_protonum(const struct sk_buff *skb)
{
    const struct iphdr *iph;
	struct action * pos;
	int ret = DETECT_NOT_MATCH;
	int nhoff;
	struct iphdr _iph;
    struct {
        struct pppoe_hdr hdr;
        __be16 proto;
    }*hdr, _hdr;
    struct vlan_hdr *vlan;
    struct vlan_hdr _vlan;
    if(!skb->dev || (skb->dev->priv_flags & IFF_WANDEV) == 0)
    {
	    return ret;
    }
    if(nf_conntrack_dmz_enable == 1)
        return DETECT_PASS;
    if(0 == nf_conntrack_fw_block_enable)
        return ret;
	nhoff = skb_network_offset(skb);
	if (skb->protocol == cpu_to_be16(ETH_P_IP))
    {
__ip:
		iph = skb_header_pointer(skb, nhoff, sizeof(_iph), &_iph);
		if (!iph)
            return ret;
    }
    else if (skb->protocol == cpu_to_be16(ETH_P_PPP_SES)) 
    {

__ppp_ses:
        hdr = skb_header_pointer(skb, nhoff, sizeof(_hdr), &_hdr);
		if (!hdr)
			return ret;
		nhoff += PPPOE_SES_HLEN;
	    if (hdr->proto == cpu_to_be16(PPP_IP))
        {
		    iph = skb_header_pointer(skb, nhoff, sizeof(_iph), &_iph);
            if (!iph)
                return ret;
        }
        else
            return ret;
    }
    else if (skb->protocol == cpu_to_be16(ETH_P_8021Q))
    {
__vlan:
		vlan = skb_header_pointer(skb, nhoff, sizeof(_vlan), &_vlan);
		if (!vlan)
			return ret;

		nhoff += sizeof(*vlan);
		if (cpu_to_be16(ETH_P_8021Q) == vlan->h_vlan_encapsulated_proto)
            goto __vlan;
        else if (cpu_to_be16(ETH_P_PPP_SES) == vlan->h_vlan_encapsulated_proto)
            goto __ppp_ses;
        else if (cpu_to_be16(ETH_P_IP) == vlan->h_vlan_encapsulated_proto)
            goto __ip;
        else
            return ret;
    }
    else
        return ret;
	spin_lock_bh(&action_lock);
	list_for_each_entry(pos, &action_list, list)
	{
        if ((iph->saddr ^ pos->match_tuple.src.u3.ip) || (pos->match_tuple.dst.protonum ^ iph->protocol))
            continue;
        if(pos->type == BLACK_ENTRY)
        {
			ret = DETECT_BLOCK;
            break;
        }
    }
	spin_unlock_bh(&action_lock);
    return ret;
}
int __sc_check_and_block_hook(struct sk_buff *skb, const struct nf_conntrack_tuple *tuple)
{
	struct action * pos;
	int ret = DETECT_NOT_MATCH;
    if(!skb->dev || (skb->dev->priv_flags & IFF_WANDEV) == 0)
    {
	    return ret;
    }
    if(nf_conntrack_dmz_enable == 1)
        return DETECT_PASS;
    if(0 == nf_conntrack_fw_block_enable)
        return ret;
	spin_lock_bh(&action_lock);
	list_for_each_entry(pos, &action_list, list)
	{
		if(sc_nf_ct_tuple_mask_cmp(tuple, &pos->match_tuple, &pos->mask))
		{
			if(((!(skb->dev->dos_flags & IFF_DOS_ENABLE) && (pos->type == WHITE_ENTRY)) ||
				pos->type == LOCAL_SERVICE_ENTRY) 
			   && ((ntohs(tuple->dst.u.all) >= pos->dport_min) 
					&& (ntohs(tuple->dst.u.all) <= pos->dport_max)))
			{
				pos->count++;
				if(pos->type == LOCAL_SERVICE_ENTRY)
					ret = DETECT_LOCAL_SERVICE;
				else
					ret = DETECT_PASS;
				
				break;
			}
			else if(pos->type == BLACK_ENTRY && ret == DETECT_NOT_MATCH)
			{
				pos->count++;
				//match it do corresponding action;
				if(printk_ratelimit())
					DEBUG("*******************  find block item *********\n");
				ret = DETECT_BLOCK;
				break;
			}
		}
	}
	spin_unlock_bh(&action_lock);
	return ret;
}

/******************************************************************
	** Insert point. Insert a pattern to action list.
	** INPUT parameters
	**		tuple means pattern tuple
	**		mask means that tuple & mask is the entire condition
	**		type means black entry(0) or white entry(1)
	**		timeout means this action will take effect how long
	** RETURN : 0 or 1
	** 		0 means insert successed
	**		1 means insert failed
******************************************************************/
int __sc_add_block_pattern_hook(struct nf_conntrack_tuple *tuple,
								struct nf_conntrack_tuple *mask,
								int dport_min,
								int dport_max,
								int type,
								int	timeout)

{
	struct action *action_pt, *pos, *prev_pos = NULL;
	int ret = 0;
	spin_lock_bh(&action_lock);
	list_for_each_entry(pos, &action_list, list)
	{
		if(sc_nf_ct_tuple_mask_cmp(tuple, &pos->match_tuple, &pos->mask)
		   && sc_nf_ct_tuple_mask_cmp(tuple, &pos->match_tuple, mask)
		   && nf_ct_tuple_equal(&pos->mask, mask)
		   && (type == pos->type)
		   && (pos->dport_max == dport_max) && (pos->dport_min == dport_min))
		{
			//match it do corresponding action;
			if(printk_ratelimit())
				DEBUG("same block entry --- do nothing. But it should be block before reach here!!!!!\n");
			ret = 1;
			goto exit;
		}
	}
	
	action_pt = kmalloc(sizeof(struct action), GFP_ATOMIC);
	if(action_pt)
	{
		memset(action_pt, 0, sizeof(struct action));
		memcpy(&action_pt->match_tuple, tuple, sizeof(struct nf_conntrack_tuple));
		memcpy(&action_pt->mask, mask, sizeof(struct nf_conntrack_tuple));
		if(timeout == BLOCK_INFINITE_TIME)
			action_pt->ts = BLOCK_INFINITE_TIME;
		else
			action_pt->ts = (jiffies + timeout)&0xFFFFFFFE; /* let it not same as BLOCK_INFINITE_TIME*/
		action_pt->type = type;
		action_pt->dport_min = dport_min;
		action_pt->dport_max = dport_max;
		
		/* must order the action list. 1st LOCAL_SERVICE_ENTRY, 
		   2nd WHITE_ENTRY, 3rd BLACK_ENTRY */
		   
		if(type == BLACK_ENTRY)
			list_add_tail(&action_pt->list, &action_list);
		else if (type == LOCAL_SERVICE_ENTRY)
			list_add(&action_pt->list, &action_list);
		else
		{
			list_for_each_entry(pos, &action_list, list)
			{
				if( LOCAL_SERVICE_ENTRY == pos->type )
				{
					prev_pos = pos;
					continue;
				}
				else
				{
					break;
				}
			}
			if(prev_pos != NULL)
				list_add(&action_pt->list, &prev_pos->list);
			else
				list_add(&action_pt->list, &action_list);
			
		}

	}
	else
		ret = 1;

exit:
	spin_unlock_bh(&action_lock);
	return ret;
}

/******************************************************************
	** Insert point. Insert a pattern to action list.
	** INPUT parameters
	**		tuple means pattern tuple
	** RETURN : 0 or 1
	** 		0 means remove successed
	**		1 means remove failed
******************************************************************/
int __sc_delete_block_pattern_hook(struct nf_conntrack_tuple *tuple,
									int dport_min,
									int dport_max,
									int type)

{
	int ret = 0;
	struct action *pos, *next;
	spin_lock_bh(&action_lock);
	list_for_each_entry_safe(pos, next, &action_list, list)
	{
		if(sc_nf_ct_tuple_mask_cmp(tuple, &pos->match_tuple, &pos->mask)
			&& (pos->type == type)
		   && (pos->dport_max == dport_max) && (pos->dport_min == dport_min))
		{
			DEBUG("find the block item, delete it\n");
			list_del(&pos->list);
			kfree(pos);
			break;
		}
		else
		{
			struct nf_conntrack_tuple * tuple_t;
			tuple_t = &pos->match_tuple;
			DEBUG("tuple : %u.%u.%u.%u:%hu -> %u.%u.%u.%u:%hu, protonum = 0x%X\n", 
							NIPQUAD((tuple_t->src.u3.ip)),ntohs(tuple_t->src.u.all),
							NIPQUAD((tuple_t->dst.u3.ip)),ntohs(tuple_t->dst.u.all), tuple_t->dst.protonum);
			tuple_t = &pos->mask;
			DEBUG("mask : %u.%u.%u.%u:%hu -> %u.%u.%u.%u:%hu, protonum = 0x%X\n", 
							NIPQUAD((tuple_t->src.u3.ip)),ntohs(tuple_t->src.u.all),
							NIPQUAD((tuple_t->dst.u3.ip)),ntohs(tuple_t->dst.u.all), tuple_t->dst.protonum);
			DEBUG("tuple : %u.%u.%u.%u:%hu -> %u.%u.%u.%u:%hu, protonum = 0x%X\n\n\n", 
							NIPQUAD((tuple->src.u3.ip)),ntohs(tuple->src.u.all),
							NIPQUAD((tuple->dst.u3.ip)),ntohs(tuple->dst.u.all), tuple->dst.protonum);
		}
	}	
	spin_unlock_bh(&action_lock);
	return ret;
}


/******************************************************************
	** Insert special block pattern. ie. port trigger
	** INPUT parameters
	**		dport_min
	**		dport_max
	** 		protonum
	** RETURN: 0 or 1
	**		0 means insert successed
	**		1 means insert failed
******************************************************************/
int __sc_add_special_block_pattern_hook(int dport_min, int dport_max, 
										int type, u_int8_t protonum)
{
	int ret = 0;
	struct nf_conntrack_tuple tuple, mask;
	
	memset(&tuple, 0, sizeof(tuple));
	memset(&mask, 0, sizeof(mask));
	
   	tuple.src.l3num = PF_INET;
   	mask.src.l3num = 0xFF;
   	
   	if(protonum != IPPROTO_BOTH)
	{
   		tuple.dst.protonum = protonum;
   		mask.dst.protonum = 0xFF; 
   	}
   	
	ret = __sc_add_block_pattern_hook(&tuple, &mask, dport_min, 
										dport_max, type, BLOCK_INFINITE_TIME);
   	
   	return ret;

}

/******************************************************************
	** Detete special block pattern. ie. port trigger 
	** INPUT parameters
	**		dport_min
	**		dport_max
	** RETURN: 0 or 1
	**		0 means delete successed
	**		1 means delete failed
******************************************************************/
int __sc_delete_special_block_pattern_hook(int dport_min, int dport_max,
										int type, u_int8_t protonum)
{
	int ret = 0;
	struct nf_conntrack_tuple tuple;
	
	memset(&tuple, 0, sizeof(tuple));
	
   	tuple.src.l3num = PF_INET;
   	
   	if(protonum != IPPROTO_BOTH)
	{
   		tuple.dst.protonum = protonum;
   	}
   	
	ret = __sc_delete_block_pattern_hook(&tuple, dport_min, 
										dport_max, type);
   	
   	return ret;
	
}

void action_timeout_check(void)
{
	struct action *pos, *next;
	unsigned long now = jiffies;
	spin_lock_bh(&action_lock);
	list_for_each_entry_safe(pos, next, &action_list, list)
	{
		if(pos->ts == BLOCK_INFINITE_TIME)
			continue;
			
		if(time_after_eq(now,pos->ts))
		{
			list_del(&pos->list);
			kfree(pos);
		}
	}	
	spin_unlock_bh(&action_lock);
	mod_timer(&timeout_action, jiffies + 1*HZ);

}


void usage(void)
{
	printk(KERN_ERR "usage:\n");
	printk(KERN_ERR "command srcip:sport dstip:dport_min-dport_max protocol type\n");
	printk(KERN_ERR "\tcommand: add\n");
	printk(KERN_ERR "\tsrcip and dstip: 0x01020304 means 1.2.3.4,  0x0 means wildcard\n");
	printk(KERN_ERR "\tsport dport_min and dport_max: 0~65535\n");
	printk(KERN_ERR "\tprotocol: udp , tcp or tcp/udp\n");
	printk(KERN_ERR "\ttype: lacal or lan\n");
}

#ifdef SEQ_OPERATION

struct ct_iter_state {
	unsigned int bucket;
};

static struct list_head *firewall_block_get_first(struct seq_file *seq)
{

	if(list_empty(&action_list))
		return NULL;
	else
		return &action_list;
}

static struct list_head *firewall_block_get_next(struct seq_file *seq, struct list_head *head)
{
	struct ct_iter_state *st = seq->private;

	head = head->next;
	while (head == &action_list) {
			return NULL;
	}
	st->bucket++;
	return head;
}

static struct list_head *firewall_block_get_idx(struct seq_file *seq, loff_t pos)
{
	struct list_head *head = firewall_block_get_first(seq);
	struct ct_iter_state *st = seq->private;

	st->bucket = 1;
	
	if (head)
		while (pos && (head = firewall_block_get_next(seq, head)))
			pos--;
	return pos ? NULL : head;
}

static void *firewall_block_seq_start(struct seq_file *seq, loff_t *pos)
{
	spin_lock_bh(&action_lock);
	return firewall_block_get_idx(seq, *pos);
}

static void *firewall_block_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return firewall_block_get_next(s, v);
}

static void firewall_block_seq_stop(struct seq_file *s, void *v)
{
	spin_unlock_bh(&action_lock);
}

/* return 0 on success, 1 in case of error */
static int firewall_block_seq_show(struct seq_file *s, void *v)
{
	const struct list_head *curr = v;
	struct action *pos;
	struct nf_conntrack_tuple	*tuple;
	struct ct_iter_state *st = s->private;
	
	pos = list_entry((curr)->next, struct action, list);

	if(pos->match_tuple.src.l3num == PF_INET)
	{
		tuple = &pos->match_tuple;
//			proto = __ip_ct_find_proto(tuple->dst.protonum);
		if (seq_printf(s, "(%d) [%-8s] [%s]:\n", st->bucket, /*(char *)proto->name*/
						(tuple->dst.protonum==IPPROTO_ICMP)?"ICMP":((tuple->dst.protonum==IPPROTO_UDP)?"UDP":
							((tuple->dst.protonum==IPPROTO_TCP)?"TCP":"Unknown")),
						(pos->type == BLACK_ENTRY)?"block entry":((pos->type == WHITE_ENTRY)?"white entry":"local service entry")))
			return -ENOSPC;

		if (seq_printf(s, "tuple : %u.%u.%u.%u:%hu -> %u.%u.%u.%u:%hu, protonum = 0x%X\n", 
						NIPQUAD((tuple->src.u3.ip)),ntohs(tuple->src.u.all),
						NIPQUAD((tuple->dst.u3.ip)),ntohs(tuple->dst.u.all), tuple->dst.protonum))
			return -ENOSPC;
		tuple = &pos->mask;
		if (seq_printf(s, "mask : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u, protonum = 0x%X\n", 
						NIPQUAD((tuple->src.u3.ip)),ntohs(tuple->src.u.all),
						NIPQUAD((tuple->dst.u3.ip)),ntohs(tuple->dst.u.all), tuple->dst.protonum))
			return -ENOSPC;

		if(seq_printf(s, "packet number %lu\n", pos->count))
			return -ENOSPC;

		if(pos->type == WHITE_ENTRY || pos->type == LOCAL_SERVICE_ENTRY)
		{
			if (seq_printf(s, "dst port %d~%d\n", pos->dport_min, pos->dport_max))
				return -ENOSPC;
		}
		if(pos->type == BLACK_ENTRY && pos->ts != BLOCK_INFINITE_TIME)
		{
			if (seq_printf(s, "left time %ld seconds\n\n", ((long)pos->ts - (long)jiffies)/HZ))
				return -ENOSPC;
		}
		else
		{
			if (seq_printf(s, "left time INFINITE seconds\n\n"))
				return -ENOSPC;
		}
	}

	return 0;
}


static struct seq_operations firewall_block_seq_fops = {
	.start = firewall_block_seq_start,
	.next  = firewall_block_seq_next,
	.stop  = firewall_block_seq_stop,
	.show  = firewall_block_seq_show
};


static int firewall_block_open(struct inode *inode, struct file *file)
{
	struct seq_file *seq;
	struct ct_iter_state *st;
	int ret;

	st = kmalloc(sizeof(struct ct_iter_state), GFP_KERNEL);
	if (st == NULL)
		return -ENOMEM;
	ret = seq_open(file, &firewall_block_seq_fops);
	if (ret)
		goto out_free;
		
	seq          = file->private_data;
	seq->private = st;
	memset(st, 0, sizeof(struct ct_iter_state));
	printk("block items:\n");

	return ret;
out_free:
	kfree(st);
	return ret;
}

static ssize_t firewall_block_write (struct file *file, const char __user *input,
				size_t size, loff_t *ofs)
{
	char buf[64];
	int len;
	struct action *pos, *next;
	struct nf_conntrack_tuple tuple, mask;
	int type;
	
	if(size > sizeof(buf))
		len = sizeof(buf);
	else
		len = size;
		
	memset(buf, 0, sizeof(buf));
	if(copy_from_user(buf,input,len))
		return -EFAULT;
		
	buf[len] = 0;
	
	printk("input:%s\n", buf);
	memset(&tuple, 0, sizeof(tuple));
	memset(&mask, 0, sizeof(mask));
	type = WHITE_ENTRY; /* this is a white entry */
	
	if(memcmp(buf, "clear", strlen("clear")) == 0)
	{
		spin_lock_bh(&action_lock);
		list_for_each_entry_safe(pos, next, &action_list, list)
		{
			list_del(&pos->list);
			kfree(pos);
		}	
		spin_unlock_bh(&action_lock);		
	}
	else if(memcmp(buf, "dmz", strlen("dmz")) == 0)
	{
		tuple.src.l3num = PF_INET;
		mask.src.l3num = 0xFF;
		printk(KERN_EMERG "type = %d\n", type);
		__sc_add_block_pattern_hook(&tuple, &mask, 1, 65535, type, BLOCK_INFINITE_TIME);
	}
	else
	{
		/* add srcip:port dstip:port protocol type
		 * add x.x.x.x:x y.y.y.y:y tcp/udp local/lan
		 * add 172.21.6.99:8080 x.x.x.x:x tcp/udp local/lan*/
		char command[64], protocol[64], type[64];
		int sport, dport_min, dport_max;
		struct in_addr sip;
		struct in_addr dip;
		int ret = 0;
	
		sscanf(buf, "%s %x:%d %x:%d-%d %s %s", command, &sip.s_addr, &sport, &dip.s_addr, &dport_min, &dport_max, protocol, type);
		
		if(memcmp(command, "add", strlen("add")) == 0)
		{
			if(sport < 0 || sport > 65535)
			{
				printk(KERN_ERR "bad source port (%d) in firewall block\n", sport);
				usage();
				goto end;
			}
			if((dport_min < 0 || dport_min > 65535)
				|| (dport_max < 0 || dport_max > 65535)
				|| dport_min > dport_max)
			{
				printk(KERN_ERR "bad dest port (%d-%d) in firewall block\n", dport_min, dport_max);
				usage();
				goto end;
			}
		   	if(sip.s_addr)
		   	{
		   		tuple.src.u3.ip = sip.s_addr;
				memset(&mask.src.u3.all, 0xFF, sizeof(mask.src.u3.all));
		   	}
		   	if(dip.s_addr)
		   	{
		   		tuple.dst.u3.ip = dip.s_addr;
				memset(&mask.dst.u3.all, 0xFF, sizeof(mask.src.u3.all));
			}
			if(sport)
			{
				tuple.src.u.all = htons(sport);
				mask.src.u.all = 0xFFFF;
			}
		   	tuple.src.l3num = PF_INET;
   			mask.src.l3num = 0xFF;
   	
#if 1
			printk(KERN_ERR " srcip(%u:%u:%u:%u:%u) -> dstip(%u:%u:%u:%u:%u-%u) in firewall block\n",
			 NIPQUAD(sip.s_addr), sport, NIPQUAD(dip.s_addr), dport_min, dport_max);
#endif
		   	if(strncasecmp(protocol, "tcp/udp", strlen("tcp/udp")) == 0)
		   	{
		   		/*nothing*/
		   	}
		   	else if(strncasecmp(protocol, "udp", strlen("udp")) == 0)
			{
		   		tuple.dst.protonum = IPPROTO_UDP;
		   		mask.dst.protonum = 0xFF; 
		   	}
		   	else if(strncasecmp(protocol, "tcp", strlen("tcp")) == 0)
		   	{
		   		tuple.dst.protonum = IPPROTO_TCP;
		   		mask.dst.protonum = 0xFF; 
		   	}
		   	else
		   	{
				printk(KERN_ERR "bad protocol (%s) in firewall block\n", protocol);
				usage();
				goto end;
		   	}
		   	
		   	if(strncasecmp(type, "local", strlen("local")) == 0)
		   	{
				ret = __sc_add_block_pattern_hook(&tuple, &mask, dport_min, 
										dport_max, LOCAL_SERVICE_ENTRY, BLOCK_INFINITE_TIME);
		   	}
		   	else if(strncasecmp(type, "lan", strlen("lan")) == 0)
		   	{
				ret = __sc_add_block_pattern_hook(&tuple, &mask, dport_min, 
										dport_max, WHITE_ENTRY, BLOCK_INFINITE_TIME);
   			}
   			else
   			{
				printk(KERN_ERR "bad type (%s) in firewall block\n", protocol);
				usage();
				goto end;
   			}
   			
			if(ret)
				printk(KERN_ERR "add command failed in firewall block\n");
		}
		else
		{
			printk(KERN_ERR "unknown command (%s) in firewall block\n", buf);
			usage();
		}
		
	}
	
end:
	return size;

}
static const struct file_operations firewall_block_fops = {
	.owner   = THIS_MODULE,
	.open    = firewall_block_open,
	.read    = seq_read,
	.write   = firewall_block_write,
	.llseek  = seq_lseek,
	.release = seq_release_private,
};
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
static int proc_read_spi_enable(char *buf, char **start, off_t offset,
							int count, int *eof, void *data)
#else
static int proc_read_spi_enable(struct file * file, char *buf, size_t count, loff_t  *data)
#endif
{
    int len = 0;
    struct net_device *dev;

    dev = dev_get_by_index(&init_net, g_spi_br_index);
    if (dev) {
    	len += sprintf(buf+len,"LAN interface: %s\n", dev->name);
        dev_put(dev);
    }
    else{
    	len += sprintf(buf+len,"LAN index    : %d\n", g_spi_br_index);
    }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
    *eof = 1;
#endif
    return len;
}

static inline char * sc_trim(char *buf, int head)
{
    char *tmp;
    buf = buf + head;
    tmp = buf;
    /* echo command output the trailing newline*/
    while(*(tmp++) != '\0') {
        if (*tmp == '\n' || *tmp == '\r'){
            *tmp = '\0';
            break;
        }
    } 
    while(*buf == ' ' || *buf == '=')
        buf++;
    return buf;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
static int proc_write_spi_enable(struct file *file, const char __user *input,
			   unsigned long count, void *data)
#else
static ssize_t proc_write_spi_enable(struct file *file,const char *input,
			   size_t count, loff_t *data)
#endif
{

    char buf[64];
    int len;
    char *str;
    struct net_device *dev;
	
	if(count > sizeof(buf))
		len = sizeof(buf);
	else
		len = count;
		
	memset(buf, 0, sizeof(buf));
	if(copy_from_user(buf,input,len))
		return -EFAULT;
		
	buf[len] = 0;
    if (memcmp(buf, "spi", strlen("spi")) == 0) {
        str = sc_trim(buf, strlen("spi"));
        if (memcmp(str, "enable", strlen("enable")) == 0) {
            g_spi_enable = 1;
        }
        else{
            g_spi_enable = 0;
        }
    }
    else if (memcmp(buf, "lan_if", sizeof("lan_if") - 1) == 0) {
     	str = sc_trim(buf, sizeof("lan_if") - 1);
        dev = dev_get_by_name(&init_net, str);
        if (dev) {
            g_spi_br_index = dev->ifindex;
            dev_put(dev);
        }
        else{
            g_spi_br_index = -1;
        }
        
        printk("lan interface=[%s], index=%d\n", str, g_spi_br_index);
    }
    else{
        printk( "illegal input.\n"              \
                "Usage:\n"                      \
                "    spi=[enable|disable]\n\n");
    }
    return count;
}

static int __init block_init(void)
{
	struct proc_dir_entry *proc, *spi_enable_proc;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
	spi_enable_proc = create_proc_entry("spi_enable", 0, init_net.proc_net);
	if(spi_enable_proc)
	{
		spi_enable_proc->read_proc = proc_read_spi_enable;
		spi_enable_proc->write_proc = proc_write_spi_enable;
	}else
	{
	    printk("can't create net/spi_enable.\n");
	}
#else
    static struct file_operations spi_enable_fops = 
    {
        .read = proc_read_spi_enable,
        .write = proc_write_spi_enable,
    };
	spi_enable_proc = proc_create ("spi_enable", 0, init_net.proc_net, &spi_enable_fops);
    if(!spi_enable_proc)
	    printk("can't create net/spi_enable.\n");
#endif
#ifdef SEQ_OPERATION
	proc = proc_create ("firewall_block", 0, init_net.proc_net, &firewall_block_fops);
	if (!proc) 
	{
		printk("firewall_block  proc create failed\n");
	}
#endif

	init_timer(&timeout_action);
	timeout_action.data = 0;
	timeout_action.function = (void *)action_timeout_check;
	timeout_action.expires = jiffies + 1*HZ;
	add_timer(&timeout_action);
	
	spin_lock_init(&action_lock);
	sc_check_and_block_hook = __sc_check_and_block_hook;
	sc_add_block_pattern_hook = __sc_add_block_pattern_hook;
	sc_delete_block_pattern_hook = __sc_delete_block_pattern_hook;
	sc_delete_special_block_pattern_hook = __sc_delete_special_block_pattern_hook;
	sc_add_special_block_pattern_hook = __sc_add_special_block_pattern_hook;
#ifdef ENABLE_QUICK_FILTER_SOURCE
    sc_check_block_src_protonum = __sc_check_block_src_protonum;	
#endif
	printk("insert firewall block module success\n");
	return 0;
}

static void __exit block_exit(void)
{
	struct action *pos, *next;
	
	del_timer(&timeout_action);

	spin_lock_bh(&action_lock);
	list_for_each_entry_safe(pos, next, &action_list, list)
	{
		list_del(&pos->list);
		kfree(pos);
	}	
	sc_check_and_block_hook = NULL;
	sc_add_block_pattern_hook = NULL;
	sc_delete_block_pattern_hook = NULL;
	sc_add_special_block_pattern_hook = NULL;
	sc_delete_special_block_pattern_hook = NULL;
    sc_check_block_src_protonum = NULL;	
	
	spin_unlock_bh(&action_lock);
	
	remove_proc_entry("firewall_block", init_net.proc_net /* parent dir */);
	remove_proc_entry("spi_enable", init_net.proc_net /* parent dir */);
	
	printk("remove firewall block module success\n");
}

module_init(block_init);
module_exit(block_exit);
