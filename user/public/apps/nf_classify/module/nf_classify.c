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
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/arp.h>
#include <linux/inetdevice.h>
#include <net/route.h>

//MODULE_LICENSE("Proprietary");
MODULE_LICENSE("GPL");

extern rwlock_t nf_conntrack_lock ;

extern volatile  int (*nf_packet_classify) (struct sk_buff *skb, void *ct, int ip_hooknum);

int sc_nf_packet_classify(struct sk_buff *skb, void *ct, int ip_hooknum)
{
    struct nf_conn *pct = NULL;
    struct nf_conn_help *help;
    int ret = 0;
    
    write_lock_bh(&nf_conntrack_lock);

    if(!(pct = (struct nf_conn *)ct))
        goto nf_unlock;

    if(get_sc_conntrack_prio(ct) > SC_NF_PKT_PRIO_FIRST)
        goto nf_unlock;

    if(ip_hooknum == NF_INET_LOCAL_OUT)
    {
        set_sc_conntrack_prio(ct, SC_NF_PKT_PRIO_HIGH);
    }
    else if(ip_hooknum == NF_INET_LOCAL_IN)
    {
        int prio = get_sc_pkt_prio(skb->sercomm_header);     

        if(prio >= SC_NF_PKT_PRIO_FIRST)
        {
            set_sc_conntrack_prio(ct, prio);
        }
    }
    else if(ip_hooknum == NF_INET_POST_ROUTING)
    {
        int prio = get_sc_pkt_prio(skb->sercomm_header);     

        if(prio >= SC_NF_PKT_PRIO_FIRST)
        {
            set_sc_conntrack_prio(ct, prio);
        }
        
        if(((help = nfct_help(pct)) && help->helper)
            || (pct->master && (help = nfct_help(pct->master)) && help->helper))
        {
            if(!strcmp(help->helper->name, "pptp")
                || strstr(help->helper->name, "sip") //sip
                || !strcmp(help->helper->name, "RAS")  //sip
                || !strcmp(help->helper->name, "Q.931")) //sip
            {
                set_sc_conntrack_prio(ct, SC_NF_PKT_PRIO_HIGH);
            }
        }
    }
    else
    {
        //do nothing
    }
    
    ret = 1;

nf_unlock:
    write_unlock_bh(&nf_conntrack_lock);    

    return ret;
}
        
static int __init nf_classify_init_module(void)
{
    nf_packet_classify = sc_nf_packet_classify;
    
    return 0;
}

static void __exit nf_classify_cleanup_module(void)
{
    write_lock_bh(&nf_conntrack_lock);
    nf_packet_classify = NULL;
    write_unlock_bh(&nf_conntrack_lock);    
}

module_init(nf_classify_init_module);
module_exit(nf_classify_cleanup_module);

