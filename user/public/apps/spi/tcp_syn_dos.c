#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/ip.h>
#include <net/tcp.h>
#include <linux/list.h>
#include <linux/netfilter/nf_conntrack_tcp.h>

#include <sc/sc_spi.h>
#include <linux/slog.h>
#include <linux/version.h>
#include "common.h"
#ifdef CONFIG_IPV6
#include <linux/in6.h>
#include <linux/ipv6.h>
#endif

MODULE_LICENSE("GPL");
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
static DEFINE_SPINLOCK(tcp_hash_table_lock);
static DEFINE_SPINLOCK(free_list_lock);
static DEFINE_SPINLOCK(free_list_lock_s);
#else
static spinlock_t 	tcp_hash_table_lock = SPIN_LOCK_UNLOCKED;
static spinlock_t free_list_lock = SPIN_LOCK_UNLOCKED;
static spinlock_t free_list_lock_s = SPIN_LOCK_UNLOCKED;
#endif
static atomic_t total_entry = ATOMIC_INIT(0);
static atomic_t total_entry_s = ATOMIC_INIT(0);
static struct list_head free_list = LIST_HEAD_INIT(free_list);
static struct list_head free_list_s = LIST_HEAD_INIT(free_list_s);

#define SCAN_MAX_COUNT     21
#define SCAN_MAX_COUNT_S   2048
#define LIST_SIZE           MAX_CONNTRACK_COUNT
#define LIST_SIZE_S         400
#define HASH_LOG            9
#define HASH_SIZE           (1 << HASH_LOG)

#define	TH_PUSH	0x08
#define	TH_SYN  0x02

#define TCP_SYN_FLOOD_BLOCK_TIMEOUT (10*60*HZ)
#define LIST_ENTRY_SIZE		600

#define HASH_LOG			9
#define HASH_SIZE			(1 << HASH_LOG)


/*
 * Information we keep per each target port
 */
struct port {
	u_int16_t number;      /* port number */ 
	u_int8_t proto;        /* protocol number */
	unsigned long timestamp;	   /*record the timestamp*/
};

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
    struct port ports[SCAN_MAX_COUNT];	/* List of ports */
    queue_t queue;
    atomic_t portcount;			/* Number of ports in the list */
    atomic_t ipcount;           /*Number of same ip */
};

struct host_s {
	struct host_s *next;		/* Next entry with the same hash */
	struct list_head	list;
	unsigned long timestamp;		/* Last update time */
	u_int8_t pf;
	struct in_addr src_addr;	/* Source address */
#ifdef CONFIG_IPV6
	struct in6_addr src_addr6;	/* Source address */
#endif
	unsigned int ports[SCAN_MAX_COUNT_S];	/* List of ports */
	atomic_t portcount;			/* Number of ports in the list */
};

/*
 * State_s information.
 */
static struct {
	spinlock_t lock;
	struct host_s list_entry[LIST_SIZE_S];	/* save source addresses array */
	struct host_s *tcp_hash[HASH_SIZE];	/* point to tcp hash list */
	struct host_s *udp_hash[HASH_SIZE];	/* point to udp hash list */
} state_s;
/*
 * State information.
 */
static struct {
	spinlock_t lock;
	struct host list_entry[LIST_SIZE];	/* save source addresses array */
	struct host *tcp_hash[HASH_SIZE];	/* point to tcp hash list */
	struct host *udp_hash[HASH_SIZE];	/* point to udp hash list */
} state;

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

static void clear_oldest_entry_s(int type, u_int8_t pf)
{
    struct host_s *curr, *prev, **head;
    int i;
    unsigned long now = jiffies;
    unsigned long timeout;
    struct host_s **hash;
    prev = NULL;
    
    if(type == IPPROTO_TCP)
        hash = &state_s.tcp_hash[0];
    else
        hash = &state_s.udp_hash[0];
    
    for(i = 0; i < HASH_SIZE; i++)
    {
        prev = NULL;
        if((curr = *(head = &hash[i])))
        {
            do 
            {
                timeout = curr->timestamp + 4*HZ;
                if(time_after(now, timeout))
                {
#ifdef CONFIG_IPV6
                    if(pf == PF_INET6)
                        memset(&(curr->src_addr6), 0, sizeof(struct in6_addr));
                    else
#endif
                        curr->src_addr.s_addr = 0;
                    curr->timestamp = 0;
                    atomic_set(&curr->portcount,0);
                    memset(&curr->ports,0,sizeof(curr->ports));
                    atomic_dec(&total_entry_s);
                    if (prev)
                        prev->next = prev->next->next;
                    else if (*head)
                        *head = (*head)->next;	
                    spin_lock_bh(&free_list_lock_s);
                    list_add(&curr->list, &free_list);
                    spin_unlock_bh(&free_list_lock_s);
                }
                else if (curr->next)
                {
                    prev = curr;
                }
            } while ((curr = curr->next));
        }
    }
}

static void clear_oldest_entry(int type,u_int8_t pf)
{
    struct host *curr, *prev, **head;
    int i;
    unsigned long now = jiffies;
    unsigned long timeout;
    struct host **hash;
    prev = NULL;
    
    if(type == IPPROTO_TCP)
        hash = &state.tcp_hash[0];
    else
        hash = &state.udp_hash[0];
    
    for(i = 0; i < HASH_SIZE; i++)
    {
        prev = NULL;
        if((curr = *(head = &hash[i])))
        {
            do 
            {
                timeout = curr->timestamp + 4*HZ;
                if(time_after(now, timeout))
                {
#ifdef CONFIG_IPV6
                    if(pf == PF_INET6)
                        memset(&(curr->src_addr6), 0, sizeof(struct in6_addr));
                    else
#endif
                        curr->src_addr.s_addr = 0;
                    atomic_set(&curr->ipcount,0);
                    atomic_set(&curr->portcount,0);
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
}

void ___sc_tcp_delete_reference_fun(struct nf_conn *ct)
{
    struct in_addr addr;
#ifdef CONFIG_IPV6
    struct in6_addr addr6;
#endif
    struct host **hash_p;
    struct host *curr, *prev,**head;
    int hash;
    u_int8_t proto;
    int count_per_src = 0;
    struct nf_conntrack_tuple tuple;
    u_int8_t pf = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
    
    addr.s_addr = 0;
    memcpy(&tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, sizeof(tuple));
    
    proto = tuple.dst.protonum; 
#ifdef CONFIG_IPV6
    if(pf == PF_INET6)
    {
        memcpy(&addr6, &tuple.src.u3.ip6, sizeof(struct in6_addr));
    }
    else
#endif
    {
        addr.s_addr = tuple.src.u3.ip;
    }

    
    spin_lock_bh(&state_s.lock);
    if (proto == IPPROTO_TCP)
    {
        hash_p = &state.tcp_hash[0];
    }
    else
    {
        hash_p = &state.udp_hash[0];
    }
	
    prev = NULL;
#ifdef CONFIG_IPV6
    if(pf == PF_INET6)
        hash = hashfunc6(addr6);
    else
#endif
        hash = hashfunc(addr);
    if ((curr = *(head = &hash_p[hash])))
    {
        do 
        {
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
                if (curr->src_addr.s_addr == addr.s_addr) 
                {
                    break;
                }
            }
		    if (curr->next) 
            {
                prev = curr;
            }
        } while ((curr = curr->next));
    }
    if(curr)
    {
        atomic_dec(&curr->ipcount);//same src ip conunt increase.
        count_per_src = atomic_read(&curr->ipcount);
        if(count_per_src == 0)
        {
#ifdef CONFIG_IPV6
            if(pf == PF_INET6)
                memset(&(curr->src_addr6), 0, sizeof(struct in6_addr));
            else
#endif
                curr->src_addr.s_addr = 0;
            atomic_set(&curr->ipcount,0);
            atomic_set(&curr->portcount,0);
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
    }
    spin_unlock_bh(&state_s.lock);
    return;
}

void  __sc_tcp_deal_establish_hook(struct nf_conn *ct)
{
    spin_lock_bh(&tcp_hash_table_lock);
    ___sc_tcp_delete_reference_fun(ct);
    spin_unlock_bh(&tcp_hash_table_lock);
}

unsigned int __sc_port_scan_check(struct sk_buff *skb, struct nf_conn *ct, u_int8_t pf, unsigned int dataoff)
{
    struct iphdr *iph = NULL;
    struct in_addr addr;
    u_int16_t dest_port;
#ifdef CONFIG_IPV6
    struct ipv6hdr *ipv6h = NULL;
    struct in6_addr addr6;
#endif
    struct udphdr *u_hp;
    struct udphdr u_hdr;
    struct tcphdr *t_hp;
    struct tcphdr t_hdr;
    unsigned long now;
    struct host_s **hash_p;
    struct host_s *curr, *prev, *next, **head;
    int hash;
    u_int8_t proto;
    unsigned int ret = NF_ACCEPT;
    struct nf_conntrack_tuple tuple;
    struct nf_conntrack_tuple mask;
    unsigned int port = 0;
    int count_per_src = 0;
    
    addr.s_addr = 0;
    
    if(!skb->dev || (skb->dev->dos_flags & IFF_DOS_ENABLE) == 0 
                || (skb->dev->priv_flags & IFF_WANDEV) == 0
            ||((pf != PF_INET)
#ifdef CONFIG_IPV6
                && (pf != PF_INET6)
#endif
              )
            )
    {
        return ret;
    }
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
    }

    memcpy(&tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, sizeof(tuple));
    if(proto == IPPROTO_UDP)
    {
        u_hp = skb_header_pointer(skb, dataoff, sizeof(u_hdr), &u_hdr);
        if(u_hp)
        {
            dest_port = u_hp->dest;
        }
        else 
            return ret;
    }
    else
    {
        t_hp = skb_header_pointer(skb, dataoff, sizeof(t_hdr), &t_hdr);
        if(t_hp)
        {
            dest_port = t_hp->dest;
        }
        else 
            return ret;
    }
    
    port = (unsigned int)(ntohs(dest_port));
    now = jiffies;
    spin_lock_bh(&state_s.lock);
    
    if (proto == IPPROTO_TCP)
    {
        hash_p = &state_s.tcp_hash[0];
    }
    else
    {
        hash_p = &state_s.udp_hash[0];
    }
	
    prev = NULL;
#ifdef CONFIG_IPV6
    if(pf == PF_INET6)
        hash = hashfunc6(addr6);
    else
#endif
        hash = hashfunc(addr);
    if ((curr = *(head = &hash_p[hash])))
    {
        do 
        {
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
                if (curr->src_addr.s_addr == addr.s_addr) 
                {
                    break;
                }
            }
            if (curr->next) 
            {
                prev = curr;
            }
        } while ((curr = curr->next));
    }
    if(curr)
    {
        
        if(time_after(now, (curr->timestamp + 60*HZ)))
        {
            memset(&curr->ports,0,sizeof(curr->ports));
            curr->timestamp = now;
            curr->ports[port>>5] |= (1 <<(port&0X1F));
            atomic_set(&curr->portcount,1);
            goto out_s;
        }
        /*port exict*/
        if(curr->ports[port>>5] & (1 <<(port&0X1F)))
        {
            goto out_s;
        }
        
        /*set port list*/
        curr->ports[port>>5] |= (1 <<(port&0X1F));
        atomic_inc(&curr->portcount);
        count_per_src = atomic_read(&curr->portcount);

        if (count_per_src > nf_conntrack_port_scan_max) 
        {
            if(proto == IPPROTO_UDP)
            {
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
                                    "DoS attack: UDP Port Scan from source: "NIP6_FMT"\n",  NIP6(ipv6h->saddr));
                        }
                        else
#endif
                        {
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                        "DoS attack: UDP Port Scan from source: %u.%u.%u.%u:%hu\n",
                                        NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all));
                        }
                    }
                }
                ret = NF_DROP;
            }
            else if(proto == IPPROTO_TCP)
            {
                    /*
                       if(close port)
                       should be take care by syn flood.
						else if (opend port)
					*/
                DEBUG("tcp port scan detect ---- %u:%u:%u:%u:%u -> %u:%u:%u:%u:%u\n",
                        NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all),
                        NIPQUAD((tuple.dst.u3.ip)),ntohs(tuple.dst.u.all));
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
                                        "DoS attack: TCP Port Scan from source: "NIP6_FMT"\n",  
                                        NIP6(ipv6h->saddr));
                        }
                        else
#endif
                        {
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                        "DoS attack: TCP Port Scan from source: %u.%u.%u.%u:%hu\n",
                                        NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all));
                        }
                    }
                }
                ret = NF_DROP;
            }
            /* delete this entry */
#ifdef CONFIG_IPV6
            if(pf == PF_INET6)
                memset(&(curr->src_addr6), 0, sizeof(struct in6_addr));
            else
#endif
                curr->src_addr.s_addr = 0;
            atomic_set(&curr->portcount,0);
            curr->timestamp = 0;
            atomic_dec(&total_entry_s);
            if (prev)
                prev->next = prev->next->next;
            else if (*head)
                *head = (*head)->next;	
            memset(&curr->ports,0,sizeof(curr->ports));
            spin_lock_bh(&free_list_lock_s);	
            list_add(&curr->list, &free_list_s);
            spin_unlock_bh(&free_list_lock_s);
        }
    }
    else
    {
	    if(atomic_read(&total_entry_s) >= LIST_SIZE_S)
	    {
		    clear_oldest_entry_s(IPPROTO_TCP,pf);
		    clear_oldest_entry_s(IPPROTO_UDP,pf);
	    }
	
        /* Get our list entry */
        spin_lock_bh(&free_list_lock_s);
        if(list_empty(&free_list_s))
        {
		    // it should be this case. Just in case.
	        spin_unlock_bh(&free_list_lock_s);
	        spin_unlock_bh(&state_s.lock);
	        return ret;
        }
        list_for_each_entry_safe(curr, next, &free_list_s, list)
        {
            list_del(&curr->list);
            break;
        }
        spin_unlock_bh(&free_list_lock_s);

        /* Link it into the hash table */
        head = &hash_p[hash];
        curr->next = *head;
        *head = curr;
		
        /* And fill in the fields */
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
        atomic_set(&curr->portcount,1);
        curr->timestamp = now;
        memset(&curr->ports,0,sizeof(curr->ports));
        curr->ports[port>>5] |= (1 <<(port&0X1F));
        atomic_inc(&total_entry_s);
    }

out_s:
    spin_unlock_bh(&state_s.lock);
    return ret;
}

unsigned int __sc_tcp_check_hook(struct sk_buff *skb, struct nf_conn *ct, u_int8_t pf, unsigned int dataoff)
{
    struct iphdr *iph = NULL;
    struct in_addr addr;
    u_int16_t dest_port;
    struct tcphdr *th;
    struct tcphdr _tcph;
#ifdef CONFIG_IPV6
    struct ipv6hdr *ipv6h = NULL;
    struct in6_addr addr6;
#endif
    struct udphdr *u_hp;
    struct udphdr u_hdr;
    struct tcphdr *t_hp;
    struct tcphdr t_hdr;
    unsigned long now;
    unsigned long timeout = 5*HZ;
    struct host **hash_p;
    struct host *curr, *prev, *next, **head;
    int hash;
    u_int8_t proto;
    unsigned int ret = NF_ACCEPT;
    int count_per_src = 0;
    int speed_limit = 200;
    struct nf_conntrack_tuple tuple;
    struct nf_conntrack_tuple mask;
    int index;
    
    
    if(!skb->dev || (skb->dev->dos_flags & IFF_DOS_ENABLE) == 0 
                || (skb->dev->priv_flags & IFF_WANDEV) == 0
            ||((pf != PF_INET)
#ifdef CONFIG_IPV6
                && (pf != PF_INET6)
#endif
              )
            )
    {
        return ret;
    }
    
    /* ct in white list will not be scaned */
    if(sc_check_and_block_hook)
    {
        if(sc_check_and_block_hook(skb, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple) == DETECT_PASS)
            return NF_ACCEPT;
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
            dest_port = u_hp->dest;
        }
        else 
            return ret;
    }
    else
    {
        t_hp = skb_header_pointer(skb, dataoff, sizeof(t_hdr), &t_hdr);
        if(t_hp)
        {
            dest_port = t_hp->dest;
        }
        else 
            return ret;
    }
    
    /* SYN with Data Detect */
    if(nf_ct_tcp_syn_with_data_enable && ct->proto.tcp.state == TCP_CONNTRACK_NONE/*TCP_CONNTRACK_SYN_SENT*/)
    {
        th = skb_header_pointer(skb, dataoff, sizeof(_tcph), &_tcph);
        if(th)
        {
            if((((u_int8_t *)th)[13] & TH_PUSH) && (((u_int8_t *)th)[13] & TH_SYN))
            {
                memcpy(&tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, sizeof(tuple));
                if (net_ratelimit())
                {
#ifdef CONFIG_IPV6
                    if(pf == PF_INET6)
                    {
                        LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                "DoS attack: SYN With Data Attack from source: "NIP6_FMT"\n",NIP6(ipv6h->saddr));
                    }
                    else
#endif
                    {
                        LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                "DoS attack: SYN With Data Attack from source: %u.%u.%u.%u:%hu\n",
                                NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all));
                    }
                }
                return NF_DROP;
            }
        }
    }
   
    /* Use jiffies here not to depend on someone setting the time while we're
	* running; we need to be careful with possible return value overflows. */
    now = jiffies;
    spin_lock_bh(&state.lock);
    
    if (proto == IPPROTO_TCP)
    {
        if(nf_ct_tcp_port_scan_enable == 0)
            goto out;
        hash_p = &state.tcp_hash[0];
    }
    else
    {
        if(nf_ct_udp_port_scan_enable == 0)
            goto out;
        hash_p = &state.udp_hash[0];
    }
	
    prev = NULL;
#ifdef CONFIG_IPV6
    if(pf == PF_INET6)
        hash = hashfunc6(addr6);
    else
#endif
        hash = hashfunc(addr);
    if ((curr = *(head = &hash_p[hash])))
    {
        do 
        {
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
                if (curr->src_addr.s_addr == addr.s_addr) 
                {
                    break;
                }
            }
		    if (curr->next) 
            {
                prev = curr;
            }
        } while ((curr = curr->next));
    }
    if(curr)
    {
        /*Flood attack detect*/
        if(proto == IPPROTO_TCP)
        {
            if(TCP_SYN_RCV_TIMEOUT > 0)
                timeout = TCP_SYN_RCV_TIMEOUT/1000;
        }
        else
        {
            if(nf_ct_udp_timeout > 0)
                timeout = nf_ct_udp_timeout/1000;
        }
        if(time_after(now, (curr->timestamp + timeout)))
        {
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                                    "session timeout: %u.%u.%u.%u:%hu,count_per_src%d,speed_limit:%d\n",
                                                    NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all),count_per_src,speed_limit);
#ifdef CONFIG_IPV6
            if(pf == PF_INET6)
                memset(&(curr->src_addr6), 0, sizeof(struct in6_addr));
            else
#endif
                curr->src_addr.s_addr = 0;
            atomic_set(&curr->ipcount,0);
            atomic_set(&curr->portcount,0);
            curr->timestamp = 0;
            atomic_dec(&total_entry);
            if (prev)
                prev->next = prev->next->next;
            else if (*head)
                *head = (*head)->next;	
            spin_lock_bh(&free_list_lock);	
            list_add(&curr->list, &free_list);
            spin_unlock_bh(&free_list_lock);
            goto out;
        }
        curr->timestamp = now;
        atomic_inc(&curr->ipcount);//same src ip increase
        count_per_src = atomic_read(&curr->ipcount);
        memcpy(&tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, sizeof(tuple));
        if(proto == IPPROTO_TCP)
        {
            if(TCP_SYN_RCV_TIMEOUT > 0)
                speed_limit = TCP_SYN_RCV_TIMEOUT/1000 * nf_ct_tcp_syn_flood_speed;
        }
        else
        {
            if(nf_ct_udp_timeout > 0)
                speed_limit = nf_ct_udp_timeout/1000*nf_ct_udp_flood_speed;
        }
        
        if(((count_per_src > speed_limit) && (skb->dev->priv_flags & IFF_WANDEV) != 0)
                || (count_per_src > (nf_conntrack_max>>1)))
        {
            if(sc_add_block_pattern_hook)
            {
                memset(&mask, 0, sizeof(mask));
                memset(&mask.src.u3.all, 0xFF, sizeof(mask.src.u3.all));
                mask.dst.protonum = 0xFF;
                sc_add_block_pattern_hook(&tuple, &mask, 0, 0, BLACK_ENTRY, nf_conntrack_block_time);
                if(proto == IPPROTO_TCP)
                {
                    if (net_ratelimit())
                    {
#ifdef CONFIG_IPV6
                        if(pf == PF_INET6)
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                                    "DoS attack: SYN Flood Attack from source: "NIP6_FMT"\n",NIP6(addr6));
                        else
#endif
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                                    "DoS attack: SYN Flood Attack from source: %u.%u.%u.%u:%hu\n",
                                                    NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all));
                    }
                }
                else
                {
                    if (net_ratelimit())
                    {
#ifdef CONFIG_IPV6
                        if(pf == PF_INET6)
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                                    "DoS attack: UDP Flood Attack from source: "NIP6_FMT"\n",NIP6(addr6));
                        else
#endif
                            LOG_FIREWALL(KERN_WARNING,CRIT_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,
                                                    "DoS attack: UDP Flood Attack from source: %u.%u.%u.%u:%hu\n",
                                                    NIPQUAD((tuple.src.u3.ip)),ntohs(tuple.src.u.all));
                    }
                }
            }
            /* delete this entry */
#ifdef CONFIG_IPV6
            if(pf == PF_INET6)
                memset(&(curr->src_addr6), 0, sizeof(struct in6_addr));
            else
#endif
                curr->src_addr.s_addr = 0;
            atomic_set(&curr->ipcount,0);
            atomic_set(&curr->portcount,0);
            curr->timestamp = 0;
            atomic_dec(&total_entry);
            if (prev)
                prev->next = prev->next->next;
            else if (*head)
                *head = (*head)->next;	
            spin_lock_bh(&free_list_lock);	
            list_add(&curr->list, &free_list);
            spin_unlock_bh(&free_list_lock);
            spin_unlock_bh(&state.lock);
            return NF_DROP;
        }
		
        /*port scan detect*/
        /* Just update the port timestamp*/
        for (index = curr->queue.front; index != curr->queue.rear; index = (index+1)%SCAN_MAX_COUNT) 
        {
            if (curr->ports[index].number == dest_port) {
                curr->ports[index].timestamp = now;
                curr->ports[index].proto = proto;
                goto out;
            }
        }
        
        /* Packet to a new port, and not TCP/ACK: update the timestamp */
        /* Remember the new port */
        if (atomic_read(&curr->portcount) < (SCAN_MAX_COUNT-1)) 
        {
            /*Check the oldest entry firstly, if it timeouts, dequeue it and insert the new entry in the end*/
            if(time_after(now, (curr->ports[curr->queue.front].timestamp + 10*HZ)))
            {
                curr->queue.front = (curr->queue.front+1)%SCAN_MAX_COUNT;
                curr->ports[curr->queue.rear].number = dest_port;
                curr->ports[curr->queue.rear].timestamp = now;
                curr->ports[curr->queue.rear].proto = proto;
                curr->queue.rear = (curr->queue.rear +1)% SCAN_MAX_COUNT;
                curr->timestamp = now;
            }
            else
            {
                curr->ports[curr->queue.rear].number = dest_port;
                curr->ports[curr->queue.rear].timestamp = now;
                curr->ports[curr->queue.rear].proto = proto;
                curr->queue.rear = (curr->queue.rear +1)% SCAN_MAX_COUNT;
                curr->timestamp = now;
                atomic_inc(&curr->portcount);
            }
        }
        else
        {
            if(time_after((curr->ports[curr->queue.front].timestamp + 10*HZ), now)
			    && time_after_eq(now, (curr->ports[curr->queue.front].timestamp)))
            {
                ret = __sc_port_scan_check(skb,ct,pf,dataoff);/*second list*/ 
                /* delete this entry */
                if(ret == NF_DROP)
                {
#ifdef CONFIG_IPV6
                    if(pf == PF_INET6)
                        memset(&(curr->src_addr6), 0, sizeof(struct in6_addr));
                    else
#endif
                        curr->src_addr.s_addr = 0;
                    atomic_set(&curr->ipcount,0);
                    atomic_set(&curr->portcount,0);
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
            }
            else
            {
                //clean_old_entry_that_timeout_than_10s;
                for(;curr->queue.rear != curr->queue.front;)
                {
                    if(time_after(now, (curr->ports[curr->queue.front].timestamp + 10*HZ)))
                    {
                        /* dequeue */
                        if(curr->queue.front != curr->queue.rear)
                        {
                            curr->queue.front = (curr->queue.front+1)%SCAN_MAX_COUNT;
                            atomic_dec(&curr->portcount);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                //insert the new session;
                curr->ports[curr->queue.rear].number = dest_port;
                curr->ports[curr->queue.rear].timestamp = now;
                curr->ports[curr->queue.rear].proto = proto;
                curr->queue.rear = (curr->queue.rear +1)% SCAN_MAX_COUNT;
                atomic_inc(&curr->portcount);
            }
        }
        goto out;
    }


    if (proto == IPPROTO_TCP && ct->proto.tcp.state != TCP_CONNTRACK_NONE)
    {
        goto out;
    }
	
    /* We're going to re-use the oldest list entry, when the entry is exhausted, 
	 * so remove it from the hash table first. */
	if(atomic_read(&total_entry) >= LIST_SIZE)
	{
		/* which means there is no list entry available */
        clear_oldest_entry(IPPROTO_TCP,pf);
        clear_oldest_entry(IPPROTO_UDP,pf);
	}
	
    /* Get our list entry */
    spin_lock_bh(&free_list_lock);
    if(list_empty(&free_list))
    {
        spin_unlock_bh(&free_list_lock);
        spin_unlock_bh(&state.lock);
        return ret;
    }
    list_for_each_entry_safe(curr, next, &free_list, list)
    {
        list_del(&curr->list);
        break;
    }
    spin_unlock_bh(&free_list_lock);

    /* Link it into the hash table */
    head = &hash_p[hash];
    curr->next = *head;
    *head = curr;
		
    curr->queue.base = &curr->ports[0].timestamp;
    curr->queue.front = 0;
    curr->queue.rear = 0;

    /* And fill in the fields */
    curr->timestamp = now;
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
    atomic_set(&curr->ipcount, 1); 
    atomic_set(&curr->portcount, 1); 
    curr->ports[0].number = dest_port;
    curr->ports[0].proto = proto;
    curr->ports[0].timestamp = now;
    curr->queue.rear = curr->queue.rear + 1;
    atomic_inc(&total_entry);

out:
    spin_unlock_bh(&state.lock);
    return ret;
}

#ifdef SEQ_OPERATION
struct ct_iter_state {
	unsigned int bucket;
};

static int first = 0;

static struct host *tcp_syn_get_first(struct seq_file *seq)
{
	struct ct_iter_state *st = seq->private;

	for (st->bucket = 0;
	     st->bucket < HASH_SIZE;
	     st->bucket++) {
		if (state.tcp_hash[st->bucket])
			return state.tcp_hash[st->bucket];
	}
	return NULL;
}

static struct host *tcp_syn_get_next(struct seq_file *seq, struct host *head)
{
	struct ct_iter_state *st = seq->private;

	head = head->next;
	while (head == NULL) {
		if (++st->bucket >= HASH_SIZE)
			return NULL;
		head = state.tcp_hash[st->bucket];
	}
	return head;
}

static struct host *tcp_syn_get_idx(struct seq_file *seq, loff_t pos)
{
	struct host *head = tcp_syn_get_first(seq);

	if (head)
		while (pos && (head = tcp_syn_get_next(seq, head)))
			pos--;
	return pos ? NULL : head;
}

static void *tcp_syn_seq_start(struct seq_file *seq, loff_t *pos)
{
    spin_lock_bh(&state.lock);
    return tcp_syn_get_idx(seq, *pos);
}

static void *tcp_syn_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return tcp_syn_get_next(s, v);
}

static void tcp_syn_seq_stop(struct seq_file *s, void *v)
{
    spin_unlock_bh(&state.lock);
}

/* return 0 on success, 1 in case of error */
static int tcp_syn_seq_show(struct seq_file *s, void *v)
{
	const struct host *curr = v;

	if(first == 1)
	{
		first = 0;
		if(seq_printf(s, "tcp syn statistic:\n"))
			return -ENOSPC;
		if(seq_printf(s, "\ttotal count:%d\n", atomic_read(&total_entry)))
			return -ENOSPC;
	}	
	if (seq_printf(s, "src ip %u:%u:%u:%u count:%d\n",
										NIPQUAD((curr->src_addr.s_addr)), atomic_read(&curr->ipcount)))
		return -ENOSPC;

	return 0;
}


static struct seq_operations tcp_syn_seq_fops = {
	.start = tcp_syn_seq_start,
	.next  = tcp_syn_seq_next,
	.stop  = tcp_syn_seq_stop,
	.show  = tcp_syn_seq_show
};


static int tcp_syn_open(struct inode *inode, struct file *file)
{
	struct seq_file *seq;
	struct ct_iter_state *st;
	int ret;

	st = kmalloc(sizeof(struct ct_iter_state), GFP_KERNEL);
	if (st == NULL)
		return -ENOMEM;
	ret = seq_open(file, &tcp_syn_seq_fops);
	if (ret)
		goto out_free;
	seq          = file->private_data;
	seq->private = st;
	memset(st, 0, sizeof(struct ct_iter_state));
	first = 1;
	printk("tcp syn total count:%d\n", atomic_read(&total_entry));
	return ret;
out_free:
	kfree(st);
	return ret;
}

static const struct file_operations tcp_syn_fops = {
	.owner   = THIS_MODULE,
	.open    = tcp_syn_open,
    .read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release_private,
};
#endif

static int __init tcp_syn_init(void)
{
	int i;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
#ifdef SEQ_OPERATION
	struct proc_dir_entry *proc;
#endif
#endif
	
	memset(&state, 0, sizeof(state));
	memset(&state_s, 0, sizeof(state_s));
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
#ifdef SEQ_OPERATION
	proc = proc_net_fops_create (&init_net, "tcp_syn_statistic", 0, &tcp_syn_fops);
	if (!proc) 
	{
		printk("tcp_syn_statistic  proc create failed\n");
	}
#endif
#endif

    spin_lock_bh(&free_list_lock_s);
    for(i = 0; i < LIST_SIZE_S; i++)
    {
         list_add(&state_s.list_entry[i].list, &free_list_s);
    }
    spin_unlock_bh(&free_list_lock_s);

    spin_lock_bh(&free_list_lock);
    for(i = 0; i < LIST_SIZE; i++)
    {
        list_add(&state.list_entry[i].list, &free_list);
    }
    spin_unlock_bh(&free_list_lock);
    
    spin_lock_init(&(state.lock));
    spin_lock_init(&(state_s.lock));
    sc_tcp_check_hook = __sc_tcp_check_hook;
    sc_tcp_deal_establish_hook = __sc_tcp_deal_establish_hook;
	return 0;
}

static void __exit tcp_syn_fini(void)
{
    spin_lock_bh(&state.lock);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
#ifdef SEQ_OPERATION
    proc_net_remove(&init_net, "tcp_syn_statistic");
#endif
#endif
    sc_tcp_check_hook = NULL;
    sc_tcp_deal_establish_hook = NULL;
    spin_unlock_bh(&state.lock);
}

module_init(tcp_syn_init);
module_exit(tcp_syn_fini);

