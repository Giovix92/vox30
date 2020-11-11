#define EXPORT_SYMTAB

#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODEVERSIONS
#endif

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/skbuff.h>
#include <net/arp.h>
#include <linux/inetdevice.h>
#include <net/route.h>
#include <linux/if_vlan.h>
#include <linux/ppp_defs.h>
#include <net/dsfield.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/netfilter/xt_dscp.h>
#include <linux/sercomm.h>
#include <net/qos_cls.h>
#include <linux/version.h>

//MODULE_LICENSE("Proprietary");
MODULE_LICENSE("GPL");

extern void (*skb_remark)(struct sk_buff *skb);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
DEFINE_RWLOCK(qos_remark_lock);
#else
rwlock_t qos_remark_lock = RW_LOCK_UNLOCKED;
#endif
#define INVALID_REMARK_VALUE (-1)
#define IFID_NUM (8)
typedef struct l2l3_remark{
    int vid; // match vlan id which user space defined
    int new_8021p; // -1: no change, 0~7
    int new_dscp; // -1: no change, 0~63
}l2l3_remark_t;

static l2l3_remark_t l2l3_remark_db[IFID_NUM];


enum{
    REMARK_TYPE_DSCP = 0,
    REMARK_TYPE_PBIT
};

static int get_remark_value(struct sk_buff *skb, int type)
{
    struct vlan_ethhdr *veth = (struct vlan_ethhdr *)(skb->data);
    int vid, i, retv = INVALID_REMARK_VALUE;

    if(!skb->dev || !(skb->dev->name))
        return retv;
    
    if(!(skb->dev->priv_flags & IFF_WANDEV))
        return retv;
        
    if(veth->h_vlan_proto != htons(ETH_P_8021Q))
        return retv;
    
    vid = ntohs(veth->h_vlan_TCI)&0xfff;
    
    read_lock_bh(&qos_remark_lock);
    for(i = 0; i < IFID_NUM; i++)
    {
        if(vid != l2l3_remark_db[i].vid)
            continue;
        
        if(type == REMARK_TYPE_DSCP)
            retv = l2l3_remark_db[i].new_dscp;
        else if(type == REMARK_TYPE_PBIT)  
            retv = l2l3_remark_db[i].new_8021p; 
        else            
            retv = INVALID_REMARK_VALUE; 
        
        break;                   
    } 
    read_unlock_bh(&qos_remark_lock);
    
    return retv;   
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
static int qos_remark_read_proc(char *buffer, char **start, off_t offset, int length, int *eof, void *data)
{
    int i = 0, len = 0;

    len += sprintf(buffer + len, "############REMARK CONFIG DUMP############\n");

    read_lock_bh(&qos_remark_lock);

    for(i = 0; i < IFID_NUM; i++)
    {
        if(l2l3_remark_db[i].vid != INVALID_REMARK_VALUE)
            len += sprintf(buffer + len, "WAN ID=%d\tVLAN ID=%d\tNEW 802.1p=%d\tNEW DSCP=%d\n",
                i+1,
                l2l3_remark_db[i].vid,
                l2l3_remark_db[i].new_8021p,
                l2l3_remark_db[i].new_dscp);
    }
    
    read_unlock_bh(&qos_remark_lock);
        
    return len;
}

static int qos_remark_write_proc(struct file *filp, const char *buffer,
        unsigned long count , void *offp)
{
    char line[128];
    int size, i = 0;
    l2l3_remark_t t;

    size = (count >= sizeof(line)) ? (sizeof(line) - 1) : count;
    copy_from_user(line, buffer, size);
    line[size] = '\0';
    
    sscanf(line, "%d;%d;%d;%d", &i, &(t.vid), &(t.new_8021p), &(t.new_dscp));

    write_lock_bh(&qos_remark_lock);
    
    if((i >= 0) && (i < IFID_NUM))
    {        
        l2l3_remark_db[i].vid = t.vid;
        l2l3_remark_db[i].new_8021p = t.new_8021p;
        l2l3_remark_db[i].new_dscp = t.new_dscp;
    }

    write_unlock_bh(&qos_remark_lock);

    return size;
}
#endif
static int skb_remark_needed(struct sk_buff *skb, int type)
{
    int ret = 0;
    struct sercomm_head *psh = NULL;
    unsigned int mark = 0;
    /*
    if(get_remark_value(skb, type) != INVALID_REMARK_VALUE)
        return 1;
    */
    psh = (struct sercomm_head *)&skb->sercomm_header[0];
    mark = psh->egress_mark;
    if(REMARK_TYPE_DSCP == type)  
        ret = GET_DSCP_REMARK_FLAG(mark);
    else if(REMARK_TYPE_PBIT == type)           
        ret = GET_8021P_REMARK_FLAG(mark);
    else
        ret = 0;        

    return ret;        
}

static int skb_remark_value_and_clear_flag(struct sk_buff *skb, int type)
{
    int retv = 0;
    struct sercomm_head *psh = NULL;
    unsigned int mark = 0;
    
    if((retv = get_remark_value(skb, type)) != INVALID_REMARK_VALUE)
        return retv;

    psh = (struct sercomm_head *)&skb->sercomm_header[0];
    mark = psh->egress_mark;
    
    if(REMARK_TYPE_DSCP == type) 
    {         
        retv = GET_DSCP_REMARK_VALUE(mark);
        psh->egress_mark = CLR_DSCP_REMARK_FLAG(&(psh->egress_mark));
    }
    else if(REMARK_TYPE_PBIT == type) 
    {                  
        retv = GET_8021P_REMARK_VALUE(mark);
        psh->egress_mark = CLR_8021P_REMARK_FLAG(&(psh->egress_mark));
    }
    else
    {        
        retv = 0;  
    }      
        
    return retv;        
}

static void my_skb_remark(struct sk_buff *skb)
{
    if(skb_remark_needed(skb, REMARK_TYPE_DSCP))
    {
        if(skb->dev)
        {
            struct iphdr *iph = NULL;
 
            if(skb->protocol == htons(ETH_P_8021Q))
            {
                struct vlan_ethhdr *veth = (struct vlan_ethhdr *)(skb->data);
                char *p = NULL;
                
                p = skb->data + sizeof(struct vlan_ethhdr);
    
                if(veth->h_vlan_encapsulated_proto == htons(ETH_P_IP)) 
                {         
                    iph = (struct iphdr *)p;
                }
                else if(veth->h_vlan_encapsulated_proto == htons(ETH_P_PPP_SES))  
                {
                    p += 6; /* pppoe header len */
                    if(*((short *)p) == htons(PPP_IP))
                        iph = (struct iphdr *)((char *)p + 2);
                }             
            }                
            else if(skb->protocol == htons(ETH_P_IP))
            {
                iph = ip_hdr(skb);
            }
            else if(skb->protocol == htons(ETH_P_PPP_SES))              
            {
                char *p = (char *)(ip_hdr(skb)) + 6/* pppoe header len */;
                printk(KERN_ERR "ip header %02x,%02x\n", p[0], p[1]);
                if(ntohs(*((short *)p)) == PPP_IP)
                    iph = (struct iphdr *)(p + 2);
            }
            
            if(iph)
            { 
                ipv4_change_dsfield(iph, (__u8)(~XT_DSCP_MASK),
                            (skb_remark_value_and_clear_flag(skb, REMARK_TYPE_DSCP))<<XT_DSCP_SHIFT);
            }
        }
    }
    if( skb_remark_needed(skb, REMARK_TYPE_PBIT))
    {
        struct vlan_ethhdr *veth = (struct vlan_ethhdr *)(skb->data);
    
        if(!skb->dev)
            return;

        if(veth->h_vlan_proto != htons(ETH_P_8021Q))
        {
            return;
        }
        else
        {
            unsigned short veth_TCI = ntohs(veth->h_vlan_TCI);

            veth_TCI &= ~(0x7<<13);
            veth_TCI |= (skb_remark_value_and_clear_flag(skb, REMARK_TYPE_PBIT))<<13;
            veth->h_vlan_TCI = htons(veth_TCI);
        }                        
    }
}

static int __init qos_remark_init_module(void)
{
    static struct proc_dir_entry *proc_qos_remark;
    int i;
    
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
    rwlock_init(&qos_remark_lock);
#else
    qos_remark_lock = RW_LOCK_UNLOCKED;
#endif
    
    for(i = 0; i < IFID_NUM; i++)
    {        
        l2l3_remark_db[i].vid = -1;
        l2l3_remark_db[i].new_8021p = -1;
        l2l3_remark_db[i].new_dscp = -1;
    }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
    if((proc_qos_remark = create_proc_entry("qos_remark", 0666, init_net.proc_net)))
    {
        proc_qos_remark->read_proc = qos_remark_read_proc;
        proc_qos_remark->write_proc = qos_remark_write_proc;
       // proc_qos_remark->owner = THIS_MODULE;
    }
#endif
    skb_remark = my_skb_remark;
    
    return 0;
}

static void __exit qos_remark_cleanup_module(void)
{
    skb_remark = NULL;
    remove_proc_entry("qos_remark", init_net.proc_net);
}

module_init(qos_remark_init_module);
module_exit(qos_remark_cleanup_module);
