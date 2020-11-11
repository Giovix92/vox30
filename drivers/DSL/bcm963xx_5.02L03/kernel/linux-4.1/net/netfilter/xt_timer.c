/**
 * @file xt_timer.c
 * @author Phil Zhang
 * @date   2009-11-22
 * @brief  do match the end time of a rule.   
 *
 * Copyright - 2009 SerComm Corporation.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_timer.h>

MODULE_AUTHOR("Phil Zhang <phil_zhang@sdc.sercomm.com>");
MODULE_DESCRIPTION("Match timer");
MODULE_ALIAS("xt_timer");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_timer");

static bool timer_mt(const struct sk_buff *skb,
        struct xt_action_param *par)
{
    const struct xt_timer_info *info = par->matchinfo;

    if(info && info->finish_time)
    {
        if(time_before(jiffies-INITIAL_JIFFIES, msecs_to_jiffies(info->finish_time*MSEC_PER_SEC)))
            return 1;
    }
    return 0;
}

static int timer_mt_check(const struct xt_mtchk_param *par)
{
	return 0;
}

static struct xt_match xt_timer_mt_reg __read_mostly = {
	.name       = "timer",
	.family		= AF_INET,
	.match      = timer_mt,
	.checkentry = timer_mt_check,
	.matchsize	= sizeof(struct xt_timer_info),
	.me         = THIS_MODULE,
};


static int __init timer_mt_init(void)
{
	return xt_register_match(&xt_timer_mt_reg);
}

static void __exit timer_mt_fini(void)
{
	xt_unregister_match(&xt_timer_mt_reg);
}

module_init(timer_mt_init);
module_exit(timer_mt_fini);
