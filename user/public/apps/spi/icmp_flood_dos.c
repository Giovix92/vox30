#include <linux/init.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/icmp.h>
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
#include <linux/icmpv6.h>
#endif
MODULE_LICENSE("GPL");

//#define LIST_SIZE			MAX_CONNTRACK_COUNT
#define LIST_SIZE			20
#define HASH_LOG			9
#define HASH_SIZE			(1 << HASH_LOG)
#define	TIMEOUT_PERIOD	(1*HZ)
#define ICMP_MAX_ENTRY	100
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
static DEFINE_SPINLOCK(free_list_lock);
#else
static spinlock_t 	free_list_lock = SPIN_LOCK_UNLOCKED;
#endif

static int timeout = TIMEOUT_PERIOD;  
static int threshhold;

/*
 * Information we keep per each source address.
 */
struct host {
    struct host *next;		/* Next entry with the same hash */
    struct list_head	list;
    unsigned long timestamp;		/* Last update time */
    u_int8_t pf;
    struct in_addr src_addr;	/* Source address */
#ifdef CONFIG_IPV6
    struct in6_addr src_addr6;	/* Source address */
#endif
    unsigned long times[ICMP_MAX_ENTRY];	/* List of ports */
    queue_t queue;
};

/*
 * State information.
 */
static struct {
	spinlock_t lock;
	struct host list_entry[LIST_SIZE];	/* save source addresses array */
	struct host *icmp_hash[HASH_SIZE];	/* point to icmp hash list */
	struct host **hash;
} state;


atomic_t total_entry = ATOMIC_INIT(0);
struct list_head free_list = LIST_HEAD_INIT(free_list);

static void clear_oldest_entry(void)
{
    struct host *curr, *prev, **head;
    int i;
    unsigned long now = jiffies;

    prev = NULL;
    state.hash = &state.icmp_hash[0];

    for(i = 0; i < HASH_SIZE; i++)
    {
        prev = NULL;
        if((curr = *(head = &state.hash[i])))
            do {
                // timeout
                if(time_after(now, (curr->timestamp + timeout)))
                {
                    curr->timestamp = 0;
                    atomic_dec(&total_entry);
                    if (prev)
                        prev->next = prev->next->next;
                    else if (*head)
                        *head = (*head)->next;	
                    spin_lock_bh(&free_list_lock);
                    list_add(&curr->list, &free_list);
                    spin_unlock_bh(&free_list_lock);
                }
                else if (curr->next)
                {
                    prev = curr;
                }
            } while ((curr = curr->next));
    }	
}
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

#ifdef CONFIG_IPV6
static inline int hashfunc6(struct in6_addr addr)
{
    unsigned int value;
    int hash;

    value = addr.s6_addr32[3];
    hash = 0;
    do {
        hash ^= value;
    } while ((value >>= HASH_LOG));

    return hash & (HASH_SIZE - 1);
}
#endif

unsigned int __sc_icmp_check_hook(struct sk_buff *skb, u_int8_t pf, struct nf_conn *ct)
{
    struct iphdr *iph = NULL;
    struct in_addr addr;
    unsigned long now;
    struct host *curr, *prev, *next, **head;
    int hash;
    unsigned int ret = NF_ACCEPT;
    struct nf_conntrack_tuple tuple;
    struct nf_conntrack_tuple mask;
#ifdef CONFIG_IPV6
    struct ipv6hdr *ipv6h = NULL;
    struct in6_addr addr6;
    struct icmp6hdr *hdr;
    int type;
#endif

    if(((skb->dev->priv_flags & IFF_WANDEV) == 0)
            || (nf_ct_icmp_flood_enable == 0)
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
        hdr = icmp6_hdr(skb);
        type = hdr->icmp6_type;
        if(type != ICMPV6_ECHO_REQUEST)
            return ret;
        memcpy(&addr6, &ipv6h->saddr, sizeof(struct in6_addr));
    }
    else
#endif
    {
        iph = ip_hdr(skb);
        addr.s_addr = iph->saddr;

        /* We're using IP address 0.0.0.0 for a special purpose here, so don't let
         * them spoof us. [DHCP needs this feature - HW] */
        if (!addr.s_addr) {
            return ret;
        }
    }

    /* Use jiffies here not to depend on someone setting the time while we're
     * running; we need to be careful with possible return value overflows. */
    now = jiffies;
    spin_lock_bh(&state.lock);
    state.hash = &state.icmp_hash[0];
    /* Do we know this source address already? */
    prev = NULL;
#ifdef CONFIG_IPV6
    if(pf == PF_INET6)
        hash = hashfunc6(addr6);
    else
#endif
        hash = hashfunc(addr);

    if ((curr = *(head = &state.hash[hash])))
        do {
#ifdef CONFIG_IPV6
            if(pf == PF_INET6 && curr->pf == PF_INET6)
            {
                if(0 == memcmp(&addr6, &(curr->src_addr6), sizeof(struct in6_addr)))
                {
                    break;
                }
            }
            else if(pf == PF_INET && curr->pf == PF_INET)
#endif
            {
                if (curr->src_addr.s_addr == addr.s_addr) break;
            }
            if (curr->next) prev = curr;
        } while ((curr = curr->next));

    if (curr) 
    {
        /* We know this src address, and the entry isn't too old. Update it. */
        /* Just update the appropriate list entry if we've seen this port already */
        if(enqueue_and_check(&curr->queue, now, timeout))
        {
            memcpy(&tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, sizeof(tuple));
            /* delete this entry */
#ifdef CONFIG_IPV6
            if(pf == PF_INET6)
            {
                memset(&(curr->src_addr6), 0, sizeof(struct in6_addr));
            }
            else
#endif
            {
                curr->src_addr.s_addr = 0;
            }
            atomic_dec(&total_entry);
            curr->timestamp = 0;
            if (prev)
                prev->next = prev->next->next;
            else if (*head)
                *head = (*head)->next;	
            spin_lock_bh(&free_list_lock);	
            list_add(&curr->list, &free_list);
            spin_unlock_bh(&free_list_lock);

            if(sc_add_block_pattern_hook)
            {
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
                                "DoS attack: ICMP Flood from source: "NIP6_FMT"\n",  
                                NIP6(ipv6h->saddr));
                    }
                    else
#endif
                    {
                        LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                "DoS attack: ICMP Flood Attck from source: %u.%u.%u.%u\n",  
                                NIPQUAD(iph->saddr));
                    }
                }
                ret = NF_DROP;
                goto out;
            }
            if (net_ratelimit())
            {
#ifdef CONFIG_IPV6
                if(pf == PF_INET6)
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: TCP FIN Flood from source: "NIP6_FMT"\n",  
                            NIP6(ipv6h->saddr));
                }
                else
#endif
                {
                    LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                            "DoS attack: ICMP Flood Attck from source: %u.%u.%u.%u\n",  
                            NIPQUAD(iph->saddr));
                }
            }
            ret = NF_DROP;
            goto out;
        }
        else
        {
            curr->timestamp = now;
            goto out;
        }
    }

    /* We're going to re-use the oldest list entry, when the entry is exhausted, 
     * so remove it from the hash table first. */
    if(atomic_read(&total_entry) >= LIST_SIZE)
    {
        /* which means there is no list entry available */
        clear_oldest_entry();
    }

    if(list_empty(&free_list))
    {
        // it should be this case. Just in case.
        goto out;
    }
    /* Get our list entry */
    spin_lock_bh(&free_list_lock);
    list_for_each_entry_safe(curr, next, &free_list, list)
    {
        list_del(&curr->list);
        break;
    }
    spin_unlock_bh(&free_list_lock);

    /* Link it into the hash table */
    head = &state.hash[hash];
    curr->next = *head;
    *head = curr;

    if(nf_ct_icmp_flood_speed > ICMP_MAX_ENTRY)
    {
        timeout = ICMP_MAX_ENTRY * HZ / nf_ct_icmp_flood_speed;
        threshhold = ICMP_MAX_ENTRY; 
    }
    else
    {
        threshhold = nf_ct_icmp_flood_speed; 
    }

    /* init queue list */
    curr->queue.base = &curr->times[0];
    curr->queue.front = 0;
    curr->queue.rear = 0;
    curr->queue.size = threshhold;

    /* And fill in the fields */
    enqueue(&curr->queue, now);
#ifdef CONFIG_IPV6
    if(pf == PF_INET6)
    {
        memcpy(&curr->src_addr6, &addr6, sizeof(struct in6_addr));
    }
    else
#endif
    {
        curr->src_addr = addr;
    }
    curr->pf = pf;
    curr->timestamp = now;
    atomic_inc(&total_entry);

out:
    spin_unlock_bh(&state.lock);
    return ret;
}
static int __init icmp_dos_init(void)
{
	int i;

	memset(&state, 0, sizeof(state));

	spin_lock_bh(&free_list_lock);
	for(i = 0; i < LIST_SIZE; i++)
	{
		list_add(&state.list_entry[i].list, &free_list);
	}
	spin_unlock_bh(&free_list_lock);
	
	spin_lock_init(&(state.lock));

	sc_icmp_check_hook = __sc_icmp_check_hook;

	return 0;
}

static void __exit icmp_dos_exit(void)
{
	sc_icmp_check_hook = NULL;
}

module_init(icmp_dos_init);
module_exit(icmp_dos_exit);
