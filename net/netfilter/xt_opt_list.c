/**
 * @file ipt_opt_list.c
 * @author West Zhou
 * @date   2017-06-09
 * @brief  do match the MAC list of DHCP optin [60|61|77].   
 * 
 *
 * Copyright - 2017 SerComm Corporation. All Rights Reserved. 
 * SerComm Corporation reserves the right to make changes to this document without notice. 
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability 
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising 
 * out of the application or use of any product or circuit. SerComm Corporation specifically 
 * disclaims any and all liability, including without limitation consequential or incidental 
 * damages; neither does it convey any license under its patent rights, nor the rights of others.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter/x_tables.h>
#include <net/tcp.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/spinlock.h>
#include <linux/etherdevice.h>
#include <linux/netfilter/xt_opt_list.h>

MODULE_AUTHOR("West Zhou <west_zhou@sdc.sercomm.com>");
MODULE_DESCRIPTION("Match xt_opt_list");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_opt_list");
MODULE_ALIAS("ip6t_opt_list");
MODULE_ALIAS("ipt_opt_list");
MODULE_ALIAS("ip6t_opt_list");


//#define TOUPPER(c)	(c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c)
#define TOLOWER(c)	(c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c)

#define HEX2VAL(s) 	((isalpha(s) ? (TOLOWER(s)-'a'+10) : (TOLOWER(s)-'0')) & 0xf)
enum
{
    XT_DHCP_OPTION_60 = 0,
    XT_DHCP_OPTION_61,
    XT_DHCP_OPTION_77,
    XT_DEVICE_TYPE,
    XT_DHCP_OPTION_MAX
};
typedef struct dhcp_opt_t
{
    int dhcp_opt_code;
    int xt_option_code;
}dhcp_opt_t;
dhcp_opt_t xt_opt_t[] = 
{
    {60, XT_DHCP_OPTION_60},
    {61, XT_DHCP_OPTION_61},
    {77, XT_DHCP_OPTION_77},
    {256, XT_DEVICE_TYPE},
    {0,  0}
};
typedef struct xt_opt_device
{
    struct list_head list;
    unsigned char mac[ETH_ALEN];
    char attr[XT_DHCP_OPTION_MAX][MAX_INFO_LENGTH];
    unsigned int ip_addr;
}xt_opt_device_t;

static LIST_HEAD(xt_opt_device_list_head);
DEFINE_RWLOCK(xt_opt_device_list_lock);

static xt_opt_device_t* xt_opt_alloc_new_device(void)
{
    xt_opt_device_t* new = NULL;

    new = (xt_opt_device_t *) kmalloc(sizeof(xt_opt_device_t), GFP_KERNEL);
    if(!new)
        return NULL;
    memset(new, 0, sizeof(xt_opt_device_t));
    INIT_LIST_HEAD(&new->list);
    return new;
}

static int xt_opt_get_option_by_dhcp_id(int dhcp_option)
{
    int ret = -1;
    dhcp_opt_t *q = NULL;
    for(q = xt_opt_t; q->dhcp_opt_code != 0; q++)
    {
        if(dhcp_option == q->dhcp_opt_code)
        {
            ret = q->xt_option_code;
            break;
        }
    }

    return ret;
}
static struct xt_opt_device * xt_opt_find_device_entry(unsigned char* mac)
{
    struct xt_opt_device *entry, *tmp;
    struct xt_opt_device * found = NULL;
    list_for_each_entry_safe(entry, tmp, &xt_opt_device_list_head, list)
    {
        if(ether_addr_equal(mac, entry->mac))
        {
            found = entry;
            break;
        }
    }
    return found;
}
static struct xt_opt_device * xt_opt_find_device_entry_by_ip_addr(unsigned int ip)
{
    struct xt_opt_device *entry, *tmp;
    struct xt_opt_device * found = NULL;
    list_for_each_entry_safe(entry, tmp, &xt_opt_device_list_head, list)
    {
        if(ip == entry->ip_addr)
        {
            found = entry;
            break;
        }
    }
    return found;
}
int xt_opt_add_device_attr(unsigned char* mac, int dhcp_id, char* value, unsigned int ip_addr)
{
    struct xt_opt_device  *new = NULL;
    int ret = -1;
    int opt_id = -1;
    struct xt_opt_device * found = NULL;

    write_lock_bh(&xt_opt_device_list_lock);
    found = xt_opt_find_device_entry(mac);

    if(!found) //new add device
    {
        new = xt_opt_alloc_new_device();
        if(!new)
        {
            write_unlock_bh(&xt_opt_device_list_lock);
            return -1;
        }
        memcpy(new->mac, mac, ETH_ALEN);
        list_add(&new->list, &xt_opt_device_list_head);
        found = new;
    }
    
    opt_id = xt_opt_get_option_by_dhcp_id(dhcp_id);
    if(opt_id >= 0)
    {
        snprintf(found->attr[opt_id], sizeof(found->attr[0]), "%s", value);
        ret = 0;
    }
    if(ip_addr)
    {
        found->ip_addr = ip_addr;
    }
    write_unlock_bh(&xt_opt_device_list_lock);
    return ret;
}
EXPORT_SYMBOL(xt_opt_add_device_attr);
int xt_opt_clear_device_list(void)
{
    struct xt_opt_device *entry, *tmp;

    write_lock_bh(&xt_opt_device_list_lock);
    list_for_each_entry_safe(entry, tmp, &xt_opt_device_list_head, list)
    {
        if (0 == entry->attr[XT_DEVICE_TYPE][0])
        {
        list_del(&entry->list);
        kfree(entry);
        }
    }
    write_unlock_bh(&xt_opt_device_list_lock);
    return 0;
}
EXPORT_SYMBOL(xt_opt_clear_device_list);
  
static bool opt_list_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
    const struct xt_opt_list_info *info = par->matchinfo;
    struct xt_opt_device * found = NULL;
    int opt_id = -1, ret=0;
    if(info)
    {	
        read_lock_bh(&xt_opt_device_list_lock);
        if(info->is_src)
        {
            if (skb_mac_header(skb) < skb->head)
            {
                read_unlock_bh(&xt_opt_device_list_lock);
                return false;
            }
            if (skb_mac_header(skb) + ETH_HLEN > skb->data)
            {
                read_unlock_bh(&xt_opt_device_list_lock);
                return false;
            }
            if(eth_hdr(skb))
                found = xt_opt_find_device_entry(eth_hdr(skb)->h_source);
        }
        else
        {
            if(ip_hdr(skb))
            {
                found = xt_opt_find_device_entry_by_ip_addr(ip_hdr(skb)->daddr);
            }
        }
        if(!found)
        {
            read_unlock_bh(&xt_opt_device_list_lock);
            return false;
        }
        opt_id = xt_opt_get_option_by_dhcp_id(info->match_option);

        if(opt_id >= 0)
        {
            ret = (strcmp(found->attr[opt_id], info->match_info) == 0);
            if(info->invert)
                ret = !ret;
            read_unlock_bh(&xt_opt_device_list_lock);
            return ret;
        }
        else
        {
            read_unlock_bh(&xt_opt_device_list_lock);
            return false;
        }
    }
    return false;
}

static int opt_list_mt_check(const struct xt_mtchk_param *par)
{
    const struct xt_opt_list_info *info = par->matchinfo;
    if(!info || strlen(info->match_info) == 0 || info->match_option <= 0)
        return 1;
    return 0;
}
static struct xt_match opt_list_mt_reg[] __read_mostly = {
	{
		.name		= "opt_list",
		.family		= NFPROTO_UNSPEC,
		.checkentry	= opt_list_mt_check,
		.match		= opt_list_mt,
		.matchsize	= sizeof(struct xt_opt_list_info),
		.me		= THIS_MODULE,
	},
};


static int __init opt_list_init(void)
{
    return xt_register_matches(opt_list_mt_reg, ARRAY_SIZE(opt_list_mt_reg));
}

static void __exit opt_list_fini(void)
{
    xt_unregister_matches(opt_list_mt_reg, ARRAY_SIZE(opt_list_mt_reg));
}

module_init(opt_list_init);
module_exit(opt_list_fini);

