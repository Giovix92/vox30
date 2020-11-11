/*
 *  ebt_sc_mark
 *
 *	Authors:
 *	martin_huang@sdc.sercomm.com to support sercomm mark
 *  based on ebt_mark
 *  March, 2011
 *
 */

/* The mark target can be used in any chain,
 * I believe adding a mangle table just for marking is total overkill.
 * Marking a frame doesn't really change anything in the frame anyway.
 */

#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_mark_t.h>
#include <linux/sercomm.h>
#include <linux/version.h>

MODULE_DESCRIPTION("Ebtables: Packet sc_mark modification");
MODULE_AUTHOR("Martin Huang <martin_huang@sdc.sercomm.com.cn>");
MODULE_LICENSE("GPL");


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
static unsigned int
ebt_mark_tg(struct sk_buff *skb, const struct xt_action_param *par)
#else
static unsigned int
ebt_mark_tg(struct sk_buff *skb, const struct xt_target_param *par)
#endif
{
	const struct ebt_mark_t_info *info = par->targinfo;
	int action = info->target & -16;

    unsigned int *mark;
    struct sercomm_head *psh;
    psh = (struct sercomm_head *)&skb->sercomm_header[0];
    mark = &psh->egress_mark;

	if (action == MARK_SET_VALUE)
		*mark = info->mark;
	else if (action == MARK_OR_VALUE)
		*mark |= info->mark;
	else if (action == MARK_AND_VALUE)
		*mark &= info->mark;
	else if (action == MARK_XOR_VALUE)
		*mark ^= info->mark;

	return info->target | ~EBT_VERDICT_BITS;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
static int ebt_mark_tg_check(const struct xt_tgchk_param *par)
#else
static bool ebt_mark_tg_check(const struct xt_tgchk_param *par)
#endif
{
	const struct ebt_mark_t_info *info = par->targinfo;
	int tmp;

	tmp = info->target | ~EBT_VERDICT_BITS;
	if (BASE_CHAIN && tmp == EBT_RETURN)
		return false;
	if (tmp < -NUM_STANDARD_TARGETS || tmp >= 0)
		return false;
	tmp = info->target & ~EBT_VERDICT_BITS;
	if (tmp != MARK_SET_VALUE && tmp != MARK_OR_VALUE &&
	    tmp != MARK_AND_VALUE && tmp != MARK_XOR_VALUE &&
       tmp != VTAG_SET_VALUE)    /* brcm */
		return false;
	return true;
}

static struct xt_target ebt_mark_tg_reg __read_mostly = {
	.name		= "SC_MARK",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_mark_tg,
	.checkentry	= ebt_mark_tg_check,
	.targetsize	= XT_ALIGN(sizeof(struct ebt_mark_t_info)),
	.me		= THIS_MODULE,
};

static int __init sc_ebt_mark_init(void)
{
	return xt_register_target(&ebt_mark_tg_reg);
}

static void __exit sc_ebt_mark_fini(void)
{
	xt_unregister_target(&ebt_mark_tg_reg);
}

module_init(sc_ebt_mark_init);
module_exit(sc_ebt_mark_fini);

