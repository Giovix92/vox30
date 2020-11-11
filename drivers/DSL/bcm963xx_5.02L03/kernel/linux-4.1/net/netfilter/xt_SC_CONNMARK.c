/*
 *	xt_connmark - Netfilter module to operate on connection marks
 *
 *	Copyright (C) 2002,2004 MARA Systems AB <http://www.marasystems.com>
 *	by Henrik Nordstrom <hno@marasystems.com>
 *	Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *	Jan Engelhardt <jengelh@medozas.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_connmark.h>

MODULE_AUTHOR("Henrik Nordstrom <hno@marasystems.com>");
MODULE_DESCRIPTION("Xtables: connection mark operations");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_SC_CONNMARK");
MODULE_ALIAS("ip6t_SC_CONNMARK");
MODULE_ALIAS("ipt_sc_connmark");
MODULE_ALIAS("ip6t_sc_connmark");
static unsigned int sc_mask_len(unsigned int mask)
{
    unsigned int len = 0;
    while((mask&0x1) == 0)
    {
        len++;
        mask = mask >> 1;
    }
    return len;
}

static unsigned int
sc_connmark_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_connmark_tginfo1 *info = par->targinfo;
	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct;
	u_int32_t newmark;
	u_int32_t ct_len = 0;
	u_int32_t nf_len = 0;
	struct sercomm_head *psh;
	psh = (struct sercomm_head *)&((skb)->sercomm_header[0]);

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return XT_CONTINUE;

	ct_len = sc_mask_len(info->ctmask);
	nf_len = sc_mask_len(info->nfmask);

	switch (info->mode) {
	case XT_CONNMARK_SAVE:
		newmark = ((ct->mark & ~info->ctmask)>>ct_len) ^
		          ((psh->mark & info->nfmask)>>nf_len);
		if (newmark) {
			ct->mark = (ct->mark & ~info->ctmask) ^ (newmark<<ct_len);
			nf_conntrack_event_cache(IPCT_MARK, ct);
		}
		break;
	case XT_CONNMARK_RESTORE:
		newmark = ((psh->mark & ~info->nfmask)>>nf_len) ^
		          ((ct->mark & info->ctmask)>>ct_len);
		if (newmark) {
			psh->mark = (psh->mark & ~info->nfmask) ^ (newmark<<nf_len);
		}
		break;
	}

	return XT_CONTINUE;
}

static int sc_connmark_tg_check(const struct xt_tgchk_param *par)
{
	int ret;

	ret = nf_ct_l3proto_try_module_get(par->family);
	if (ret < 0)
		pr_info("cannot load conntrack support for proto=%u\n",
			par->family);
	return ret;
}

static void sc_connmark_tg_destroy(const struct xt_tgdtor_param *par)
{
	nf_ct_l3proto_module_put(par->family);
}

static bool
sc_connmark_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_connmark_mtinfo1 *info = par->matchinfo;
	enum ip_conntrack_info ctinfo;
	const struct nf_conn *ct;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return false;

	return ((ct->mark & info->mask) == info->mark) ^ info->invert;
}

static int sc_connmark_mt_check(const struct xt_mtchk_param *par)
{
	int ret;

	ret = nf_ct_l3proto_try_module_get(par->family);
	if (ret < 0)
		pr_info("cannot load conntrack support for proto=%u\n",
			par->family);
	return ret;
}

static void sc_connmark_mt_destroy(const struct xt_mtdtor_param *par)
{
	nf_ct_l3proto_module_put(par->family);
}

static struct xt_target sc_connmark_tg_reg __read_mostly = {
	.name           = "SC_CONNMARK",
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.checkentry     = sc_connmark_tg_check,
	.target         = sc_connmark_tg,
	.targetsize     = sizeof(struct xt_connmark_tginfo1),
	.destroy        = sc_connmark_tg_destroy,
	.me             = THIS_MODULE,
};

static struct xt_match sc_connmark_mt_reg __read_mostly = {
	.name           = "connmark",
	.revision       = 1,
	.family         = NFPROTO_UNSPEC,
	.checkentry     = sc_connmark_mt_check,
	.match          = sc_connmark_mt,
	.matchsize      = sizeof(struct xt_connmark_mtinfo1),
	.destroy        = sc_connmark_mt_destroy,
	.me             = THIS_MODULE,
};

static int __init sc_connmark_mt_init(void)
{
	int ret;

	ret = xt_register_target(&sc_connmark_tg_reg);
	if (ret < 0)
		return ret;
	ret = xt_register_match(&sc_connmark_mt_reg);
	if (ret < 0) {
		xt_unregister_target(&sc_connmark_tg_reg);
		return ret;
	}
	return 0;
}

static void __exit sc_connmark_mt_exit(void)
{
	xt_unregister_match(&sc_connmark_mt_reg);
	xt_unregister_target(&sc_connmark_tg_reg);
}

module_init(sc_connmark_mt_init);
module_exit(sc_connmark_mt_exit);
