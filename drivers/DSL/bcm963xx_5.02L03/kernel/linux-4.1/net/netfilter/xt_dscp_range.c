/* IP tables module for matching the value of the IPv4/IPv6 DSCP field
 *
 * (C) 2002 by Harald Welte <laforge@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/dsfield.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_dscp_range.h>

MODULE_AUTHOR("Harald Welte <laforge@netfilter.org>");
MODULE_DESCRIPTION("Xtables: DSCP/TOS field match");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_dscps");
MODULE_ALIAS("ip6t_dscps");

static bool
dscps_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_dscps_info *info = par->matchinfo;
	u_int8_t dscp = ipv4_get_dsfield(ip_hdr(skb)) >> XT_DSCP_SHIFT;

	return ((dscp >= info->dscp) && (dscp <= info->dscp_e)) ^ !!info->invert;
}

static bool
dscps_mt6(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_dscps_info *info = par->matchinfo;
	u_int8_t dscp = ipv6_get_dsfield(ipv6_hdr(skb)) >> XT_DSCP_SHIFT;

	return ((dscp >= info->dscp) && (dscp <= info->dscp_e)) ^ !!info->invert;
}

static int dscps_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_dscps_info *info = par->matchinfo;

	if (info->dscp > XT_DSCP_MAX) {
		printk(KERN_ERR "xt_dscps: dscps %x out of range\n", info->dscp);
		return -1;
	}
        if (info->dscp_e > XT_DSCP_MAX) {
		printk(KERN_ERR "xt_dscps: dscps %x out of range\n", info->dscp);
		return -1;
	}


	return 0;
}



static struct xt_match dscps_mt_reg[] __read_mostly = {
	{
		.name		= "dscps",
		.family		= NFPROTO_IPV4,
		.checkentry	= dscps_mt_check,
		.match		= dscps_mt,
		.matchsize	= sizeof(struct xt_dscps_info),
		.me		= THIS_MODULE,
	},
	{
		.name		= "dscps",
		.family		= NFPROTO_IPV6,
		.checkentry	= dscps_mt_check,
		.match		= dscps_mt6,
		.matchsize	= sizeof(struct xt_dscps_info),
		.me		= THIS_MODULE,
	},
};

static int __init dscps_mt_init(void)
{
	return xt_register_matches(dscps_mt_reg, ARRAY_SIZE(dscps_mt_reg));
}

static void __exit dscps_mt_exit(void)
{
	xt_unregister_matches(dscps_mt_reg, ARRAY_SIZE(dscps_mt_reg));
}

module_init(dscps_mt_init);
module_exit(dscps_mt_exit);
