/*
 * (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 * (C) 2011 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_nat_core.h>
#ifdef __SC_BUILD__
#ifdef CONFIG_CNAPT
#include <sc/cnapt/nf_cnapt.h>
#endif /* CONFIG_CNAPT */
#ifdef CONFIG_SUPPORT_SPI_FIREWALL
#include <sc/sc_spi.h>
#endif
#endif /* _SC_BUILD_ */
static int xt_nat_checkentry_v0(const struct xt_tgchk_param *par)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;

	if (mr->rangesize != 1) {
		pr_info("%s: multiple ranges no longer supported\n",
			par->target->name);
		return -EINVAL;
	}
	return 0;
}

static void xt_nat_convert_range(struct nf_nat_range *dst,
				 const struct nf_nat_ipv4_range *src)
{
	memset(&dst->min_addr, 0, sizeof(dst->min_addr));
	memset(&dst->max_addr, 0, sizeof(dst->max_addr));

	dst->flags	 = src->flags;
	dst->min_addr.ip = src->min_ip;
	dst->max_addr.ip = src->max_ip;
	dst->min_proto	 = src->min;
	dst->max_proto	 = src->max;
}

static unsigned int
xt_snat_target_v0(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;
	struct nf_nat_range range;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	NF_CT_ASSERT(ct != NULL &&
		     (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED ||
		      ctinfo == IP_CT_RELATED_REPLY));

	xt_nat_convert_range(&range, &mr->range[0]);
	return nf_nat_setup_info(ct, &range, NF_NAT_MANIP_SRC);
}

static unsigned int
xt_dnat_target_v0(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;
	struct nf_nat_range range;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	NF_CT_ASSERT(ct != NULL &&
		     (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

	xt_nat_convert_range(&range, &mr->range[0]);
#ifdef __SC_BUILD__
#ifdef CONFIG_CNAPT
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_TCP ||
		ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_UDP) {
		__be32 privip, pubip;
		__u16 privport, pubport;
		typeof(cnapt_get_mapping_info_hook) get_info;
		int bypass = 0;
	rcu_read_lock();
		get_info = rcu_dereference(cnapt_get_mapping_info_hook);
		if (get_info && 
			(*get_info)(ct, par->hooknum, 
					&privip, &privport, &pubip, &pubport) == 0) {	

			/* This must be fake. */
			if (pubip == 0)
				bypass = 1;

			if (!bypass) {
				if (mr->range[0].flags & IP_NAT_RANGE_MAP_IPS)
					bypass = (privip != mr->range[0].min_ip);
				else
					bypass = (privip != pubip);
			}

			if (!bypass) {
				if (mr->range[0].flags & IP_NAT_RANGE_PROTO_SPECIFIED) {
					if (mr->range[0].min.tcp.port != mr->range[0].max.tcp.port) {
			 			bypass = (privport != pubport || mr->range[0].min.tcp.port > privport 
			 				|| mr->range[0].max.tcp.port < privport);
			 		} else {
			 			bypass = (privport != mr->range[0].min.tcp.port);
			 		}
				} else
			 		bypass = (privport != pubport);
			 }
		}
		rcu_read_unlock();
		if (bypass)
			return XT_CONTINUE;
	}
#endif
#endif
#if defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_SPI_FIREWALL)
    if(ct->from_wan && sc_detect_total_session_for_one_host_hook)
    {
        if(sc_detect_total_session_for_one_host_hook(skb, ct) == NF_DROP)
        {
            if(printk_ratelimit())
                printk("DNAT total session number from one host is too much\n");
            return  NF_DROP;
        }
    }
#endif /* _SC_BUILD_ && CONFIG_SPI_FIREWALL */
	return nf_nat_setup_info(ct, &range, NF_NAT_MANIP_DST);
}

static unsigned int
xt_snat_target_v1(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct nf_nat_range *range = par->targinfo;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	NF_CT_ASSERT(ct != NULL &&
		     (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED ||
		      ctinfo == IP_CT_RELATED_REPLY));
#ifdef __SC_BUILD__
#ifdef CONFIG_CNAPT
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_TCP ||
		ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_UDP) {
		__be32 privip, pubip;
		__u16 privport, pubport;
		typeof(cnapt_get_mapping_info_hook) get_info;
		int bypass = 0;
	rcu_read_lock();
		get_info = rcu_dereference(cnapt_get_mapping_info_hook);
		if (get_info && 
			(*get_info)(ct, par->hooknum, 
					&privip, &privport, &pubip, &pubport) == 0) {	

			/* This must be fake. */
			if (pubip == 0)
				bypass = 1;

			if (!bypass) {
				if (mr->range[0].flags & IP_NAT_RANGE_MAP_IPS)
					bypass = (privip != mr->range[0].min_ip);
				else
					bypass = (privip != pubip);
			}

			if (!bypass) {
				if (mr->range[0].flags & IP_NAT_RANGE_PROTO_SPECIFIED) {
					if (mr->range[0].min.tcp.port != mr->range[0].max.tcp.port) {
			 			bypass = (privport != pubport || mr->range[0].min.tcp.port > privport 
			 				|| mr->range[0].max.tcp.port < privport);
			 		} else {
			 			bypass = (privport != mr->range[0].min.tcp.port);
			 		}
				} else
			 		bypass = (privport != pubport);
			 }
		}
		rcu_read_unlock();
		if (bypass)
			return XT_CONTINUE;
	}
#endif
#endif
#if defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_SPI_FIREWALL)
    if(ct->from_wan && sc_detect_total_session_for_one_host_hook)
    {
        if(sc_detect_total_session_for_one_host_hook(skb, ct) == NF_DROP)
        {
            if(printk_ratelimit())
                printk("DNAT total session number from one host is too much\n");
            return  NF_DROP;
        }
    }
#endif /* _SC_BUILD_ && CONFIG_SPI_FIREWALL */
	return nf_nat_setup_info(ct, range, NF_NAT_MANIP_SRC);
}

static unsigned int
xt_dnat_target_v1(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct nf_nat_range *range = par->targinfo;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	NF_CT_ASSERT(ct != NULL &&
		     (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

	return nf_nat_setup_info(ct, range, NF_NAT_MANIP_DST);
}

static struct xt_target xt_nat_target_reg[] __read_mostly = {
	{
		.name		= "SNAT",
		.revision	= 0,
		.checkentry	= xt_nat_checkentry_v0,
		.target		= xt_snat_target_v0,
		.targetsize	= sizeof(struct nf_nat_ipv4_multi_range_compat),
		.family		= NFPROTO_IPV4,
		.table		= "nat",
		.hooks		= (1 << NF_INET_POST_ROUTING) |
				  (1 << NF_INET_LOCAL_IN),
		.me		= THIS_MODULE,
	},
	{
		.name		= "DNAT",
		.revision	= 0,
		.checkentry	= xt_nat_checkentry_v0,
		.target		= xt_dnat_target_v0,
		.targetsize	= sizeof(struct nf_nat_ipv4_multi_range_compat),
		.family		= NFPROTO_IPV4,
		.table		= "nat",
		.hooks		= (1 << NF_INET_PRE_ROUTING) |
				  (1 << NF_INET_LOCAL_OUT),
		.me		= THIS_MODULE,
	},
	{
		.name		= "SNAT",
		.revision	= 1,
		.target		= xt_snat_target_v1,
		.targetsize	= sizeof(struct nf_nat_range),
		.table		= "nat",
		.hooks		= (1 << NF_INET_POST_ROUTING) |
				  (1 << NF_INET_LOCAL_IN),
		.me		= THIS_MODULE,
	},
	{
		.name		= "DNAT",
		.revision	= 1,
		.target		= xt_dnat_target_v1,
		.targetsize	= sizeof(struct nf_nat_range),
		.table		= "nat",
		.hooks		= (1 << NF_INET_PRE_ROUTING) |
				  (1 << NF_INET_LOCAL_OUT),
		.me		= THIS_MODULE,
	},
};

static int __init xt_nat_init(void)
{
	return xt_register_targets(xt_nat_target_reg,
				   ARRAY_SIZE(xt_nat_target_reg));
}

static void __exit xt_nat_exit(void)
{
	xt_unregister_targets(xt_nat_target_reg, ARRAY_SIZE(xt_nat_target_reg));
}

module_init(xt_nat_init);
module_exit(xt_nat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_ALIAS("ipt_SNAT");
MODULE_ALIAS("ipt_DNAT");
MODULE_ALIAS("ip6t_SNAT");
MODULE_ALIAS("ip6t_DNAT");
