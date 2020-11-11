#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/ip.h>
#include <net/udp.h>
#include <linux/list.h>

#include <sc/sc_spi.h>
#include <linux/slog.h>
#include "common.h"

MODULE_LICENSE("GPL");

#define UDP_FLOOD_BLOCK_TIMEOUT (10*60*HZ)
#define LIST_ENTRY_SIZE		600
#define ECHO_PORT		7
#define CHARGEN_PORT	19

#define HASH_LOG			9
#define HASH_SIZE			(1 << HASH_LOG)

static struct udp_bysrc_ip_s{
	struct udp_bysrc_ip_s *next;
	struct list_head queue_list;
	struct list_head udp_bysrc_ip;
	struct in_addr src_addr;	/* Source address */
	atomic_t count;
} udp_bysrc_ip_item[LIST_ENTRY_SIZE]={{0,},};

static struct list_head free_queue = LIST_HEAD_INIT(free_queue);
static struct udp_bysrc_ip_s  *bysrc_ip_hash_array[HASH_SIZE];
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
static DEFINE_SPINLOCK(udp_lock);
static DEFINE_SPINLOCK(udp_hash_table_lock);
#else
static spinlock_t 	udp_lock = SPIN_LOCK_UNLOCKED;
static spinlock_t 	udp_hash_table_lock = SPIN_LOCK_UNLOCKED;
#endif



static atomic_t total_count  = ATOMIC_INIT(0);


/*
 * Convert an IP address into a hash table index.
 */
static inline int hashfunc(struct in_addr addr)
{
	unsigned int value;
	int hash;

	value = addr.s_addr;
	hash = 0;
	do {
		hash ^= value;
	} while ((value >>= HASH_LOG));

	return hash & (HASH_SIZE - 1);
}

//void remove_session_idled_few_secs(unsigned long delta)
//{
//	spin_lock(&udp_lock);
	//list each entry and remove the entry that time idled than delta
//	spin_unlock(&udp_lock);
//}

static int compare_ct_by_tuple(struct nf_conn *i, void *data)
{
	struct nf_conntrack_tuple *tuple = (struct nf_conntrack_tuple *)data;
	if(nf_ct_tuple_equal(&i->tuplehash[IP_CT_DIR_ORIGINAL].tuple, tuple))
	{
		return 1;
	}
	return 0;
}


void delete_all_session_from_same_src(struct udp_bysrc_ip_s *curr)
{
	struct nf_conn *ct;
	struct nf_conntrack_tuple tuple;
	
restart:
	spin_lock_bh(&udp_hash_table_lock);
	list_for_each_entry(ct, &curr->udp_bysrc_ip, udp_bysrc_ip)
	{
		tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
		
		list_del_init(&ct->udp_bysrc_ip);
		spin_unlock_bh(&udp_hash_table_lock);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
		nf_ct_iterate_cleanup(&init_net, compare_ct_by_tuple, &tuple);
#else
		nf_ct_iterate_cleanup(&init_net, compare_ct_by_tuple, &tuple, 0 ,0);
#endif
		goto restart;
	}
	spin_unlock_bh(&udp_hash_table_lock);
	
}

void ___sc_udp_delete_reference_fun(struct nf_conn *ct)
{
	struct udp_bysrc_ip_s *lookup, *curr, **head, *prev;
	if(ct->udp_head)
	{
		lookup = ct->udp_head;
		
		atomic_dec(&ct->udp_head->count);//same src ip conunt increase.

		/* remove unused item from hash table, but be dec by tcp_packet*/
		if( atomic_read(&lookup->count) == 0)
		{
			prev = NULL;
		

			curr = *(head = &bysrc_ip_hash_array[hashfunc(lookup->src_addr)]);
			if( curr )
			{
				do {
					/*find if has the same src ip has come here*/
					if (curr == lookup) 
						break;
					prev = curr;
				} while ((curr = curr->next));
			}


			/* Then, remove it */
			if (curr) {
				if (prev)
					prev->next = prev->next->next;
				else if (*head)
					*head = (*head)->next;
			}
			else
			{
				if(printk_ratelimit())
					DEBUG("something wrong, did't find this tcp entry from the list\n");
			}

			list_add(&curr->queue_list, &free_queue);
			curr->src_addr.s_addr = 0;
			INIT_LIST_HEAD(&curr->udp_bysrc_ip);
		}

		list_del(&ct->udp_bysrc_ip);
		ct->udp_head = NULL;
		INIT_LIST_HEAD(&ct->udp_bysrc_ip);

					
		atomic_dec(&total_count);//total conunt increase.
	}
	
}

void  __sc_udp_deal_establish_hook(struct nf_conn *ct)
{
	spin_lock_bh(&udp_hash_table_lock);
	___sc_udp_delete_reference_fun(ct);
	spin_unlock_bh(&udp_hash_table_lock);
}

void __sc_udp_destroy_hook(struct nf_conn *ct)
{
	spin_lock_bh(&udp_hash_table_lock);
	___sc_udp_delete_reference_fun(ct);
	spin_unlock_bh(&udp_hash_table_lock);
}

unsigned int __sc_udp_check_hook(struct sk_buff *skb, struct nf_conn *ct)
{
	struct udp_bysrc_ip_s *curr, **head, *prev, *next;
	struct in_addr addr;
	int srchash = 0;
	int count_per_src = 0;
        int speed_limit = 200;
        struct nf_conntrack_tuple mask;
        struct nf_conntrack_tuple tuple;
	struct iphdr *iph;
	struct udphdr *udp_hdr;

	int count = 0;

	prev = NULL;

        if(!skb->dev || (skb->dev->dos_flags & IFF_DOS_ENABLE) == 0 
                || (skb->dev->priv_flags & IFF_WANDEV) == 0
                || nf_ct_udp_flood_enable== 0)
            goto out;

	/* IP header */
        iph = ip_hdr(skb);
        udp_hdr = (struct udphdr*)((u_int32_t *)iph + iph->ihl);
#if 0
	src_port = udp_hdr->source;
	if((src_port != htons(ECHO_PORT)) && (src_port != htons(CHARGEN_PORT)))
        goto out;
#endif
    /* ct in white list will not be scaned */
    if(sc_check_and_block_hook)
    {
        if(sc_check_and_block_hook(skb, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple) == DETECT_PASS)
        {
            return NF_ACCEPT;
        }
    }
#if 0
    if(atomic_read(&total_count) >= 600)
    {
        			if(printk_ratelimit())
        				printk("block ..... because total_count = %d\n", atomic_read(&total_count));
        return NF_DROP;
    }
#endif
    addr.s_addr = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
    // src_hash function should hash any src to 0 ~ HASH_SIZE -1
    srchash = hashfunc(addr);

    spin_lock_bh(&udp_lock);
    spin_lock_bh(&udp_hash_table_lock);
    curr = *(head = &bysrc_ip_hash_array[srchash]);
    if(curr)
    {
        do {
            /*find if has the same src ip has come here*/
            if(curr->src_addr.s_addr == addr.s_addr)
                break;
            count++;


        } while ((curr = curr->next));
    }

    if(curr)
    {	/*find out a match record*/
        /* add ct to the item */
        list_add(&ct->udp_bysrc_ip, &curr->udp_bysrc_ip);
        ct->udp_head = curr;
        atomic_inc(&total_count);//total conunt increase.
        atomic_inc(&curr->count);//same src ip increase
        count_per_src = atomic_read(&curr->count);
        if(nf_ct_udp_timeout > 0)
            speed_limit = nf_ct_udp_timeout/1000*nf_ct_udp_flood_speed;
        memcpy(&tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, sizeof(tuple));

        spin_unlock_bh(&udp_hash_table_lock);
        spin_unlock_bh(&udp_lock);

        if(count_per_src > speed_limit)
        {
            delete_all_session_from_same_src(curr);
            if(sc_add_block_pattern_hook)
            {
                memset(&mask, 0, sizeof(mask));
                memset(&mask.src.u3.all, 0xFF, sizeof(mask.src.u3.all));
                mask.dst.protonum = 0xFF;

                sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, nf_conntrack_block_time);
			    if (net_ratelimit())
                LOG_FIREWALL(KERN_WARNING,NORM_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                    "DoS attack: UDP Flood Attack from source: %u.%u.%u.%u:%hu\n",
                        NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all));
            }

        }

        goto out;

    }

    spin_unlock_bh(&udp_hash_table_lock);
    /* new incoming session */
    //this is a new session. find the record��insert in hash table, and count it

    if(list_empty(&free_queue))
    {
        // it should not be this case. Just in case.
        spin_unlock_bh(&udp_lock);
        return NF_DROP;
    }
    list_for_each_entry_safe(curr, next, &free_queue, queue_list)
    {
        list_del(&curr->queue_list);
        break;
    }

    curr->src_addr.s_addr = addr.s_addr;

    /* insert it into the hash table */
    spin_lock_bh(&udp_hash_table_lock);

    head = & bysrc_ip_hash_array [srchash];
    curr->next = *head;
    *head = curr;

    /* add ct to the item */
    list_add(&ct->udp_bysrc_ip, &curr->udp_bysrc_ip);
    ct->udp_head = curr;
    atomic_inc(&total_count);	//total conunt increase.
    atomic_set(&curr->count, 1); //same src ip increase

    spin_unlock_bh(&udp_hash_table_lock);
    spin_unlock_bh(&udp_lock);

out:
	return NF_ACCEPT;

}

#ifdef SEQ_OPERATION
struct ct_iter_state {
	unsigned int bucket;
};

static int first = 0;

static struct udp_bysrc_ip_s *udp_get_first(struct seq_file *seq)
{
	struct ct_iter_state *st = seq->private;

	for (st->bucket = 0;
	     st->bucket < HASH_SIZE;
	     st->bucket++) {
		if (bysrc_ip_hash_array[st->bucket])
			return bysrc_ip_hash_array[st->bucket];
	}
	return NULL;
}

static struct udp_bysrc_ip_s *udp_get_next(struct seq_file *seq, struct udp_bysrc_ip_s *head)
{
	struct ct_iter_state *st = seq->private;

	head = head->next;
	while (head == NULL) {
		if (++st->bucket >= HASH_SIZE)
			return NULL;
		head = bysrc_ip_hash_array[st->bucket];
	}
	return head;
}

static struct udp_bysrc_ip_s *udp_get_idx(struct seq_file *seq, loff_t pos)
{
	struct udp_bysrc_ip_s *head = udp_get_first(seq);

	if (head)
		while (pos && (head = udp_get_next(seq, head)))
			pos--;
	return pos ? NULL : head;
}

static void *udp_seq_start(struct seq_file *seq, loff_t *pos)
{
	spin_lock_bh(&udp_lock);
	spin_lock_bh(&udp_hash_table_lock);
	return udp_get_idx(seq, *pos);
}

static void *udp_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return udp_get_next(s, v);
}

static void udp_seq_stop(struct seq_file *s, void *v)
{
	spin_unlock_bh(&udp_hash_table_lock);
	spin_unlock_bh(&udp_lock);
}

/* return 0 on success, 1 in case of error */
static int udp_seq_show(struct seq_file *s, void *v)
{
	const struct udp_bysrc_ip_s *curr = v;

	if(first == 1)
	{
		first = 0;
		if(seq_printf(s, "UDP statistic:\n"))
			return -ENOSPC;
		if(seq_printf(s, "\ttotal count:%d\n", atomic_read(&total_count)))
			return -ENOSPC;
	}	
	if (seq_printf(s, "src ip %u:%u:%u:%u count:%d\n",
										NIPQUAD((curr->src_addr.s_addr)), atomic_read(&curr->count)))
		return -ENOSPC;

	return 0;
}


static struct seq_operations udp_seq_fops = {
	.start = udp_seq_start,
	.next  = udp_seq_next,
	.stop  = udp_seq_stop,
	.show  = udp_seq_show
};


static int udp_open(struct inode *inode, struct file *file)
{
	struct seq_file *seq;
	struct ct_iter_state *st;
	int ret;

	st = kmalloc(sizeof(struct ct_iter_state), GFP_KERNEL);
	if (st == NULL)
		return -ENOMEM;
	ret = seq_open(file, &udp_seq_fops);
	if (ret)
		goto out_free;
	seq          = file->private_data;
	seq->private = st;
	memset(st, 0, sizeof(struct ct_iter_state));
	first = 1;
	return ret;
out_free:
	kfree(st);
	return ret;
}



static const struct file_operations udp_fops = {
	.owner   = THIS_MODULE,
	.open    = udp_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release_private,
};
#endif

static int __init udp_flood_init(void)
{
	int i;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
#ifdef SEQ_OPERATION
	struct proc_dir_entry *proc;
#endif
#endif
	
	for(i = 0; i < LIST_ENTRY_SIZE; i++)
	{
		list_add(&udp_bysrc_ip_item[i].queue_list, &free_queue);
		INIT_LIST_HEAD(&udp_bysrc_ip_item[i].udp_bysrc_ip);
	}
	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
#ifdef SEQ_OPERATION
	proc = proc_net_fops_create (&init_net, "udp_statistic", 0, &udp_fops);
	if (!proc) 
	{
		printk("udp_statistic  proc create failed\n");
	}
#endif
#endif
	
	sc_udp_check_hook = __sc_udp_check_hook;
	sc_udp_destroy_hook = __sc_udp_destroy_hook;
	//sc_udp_deal_establish_hook = __sc_udp_deal_establish_hook;

	printk("netfilter UDP flood module loaded \n");
	return 0;
}

static void __exit udp_flood_fini(void)
{
	
	struct nf_conn *ct, *n;
	int bucket;
	struct udp_bysrc_ip_s *curr;
	
	spin_lock_bh(&udp_lock);

	spin_lock_bh(&udp_hash_table_lock);
	for (bucket = 0; bucket < HASH_SIZE; bucket++)
	{
		if ((curr = bysrc_ip_hash_array[bucket]))
		{
			do {
					list_for_each_entry_safe(ct, n, &curr->udp_bysrc_ip, udp_bysrc_ip)
					{
						___sc_udp_delete_reference_fun(ct);
					}
			} while ((curr = curr->next));	
		}
	}
	spin_unlock_bh(&udp_hash_table_lock);

	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
#ifdef SEQ_OPERATION
	proc_net_remove(&init_net, "udp_statistic");
#endif
#endif

	sc_udp_check_hook = NULL;
	sc_udp_destroy_hook = NULL;
	//sc_udp_deal_establish_hook = NULL;
	
	spin_unlock_bh(&udp_lock);
	
	printk("netfilter UDP flood module unloaded \n");
}

module_init(udp_flood_init);
module_exit(udp_flood_fini);

