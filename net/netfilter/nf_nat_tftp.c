/* (C) 2001-2002 Magnus Boden <mb@ozaba.mine.nu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/udp.h>

#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_nat_helper.h>
#include <linux/netfilter/nf_conntrack_tftp.h>

#ifdef __SC_BUILD__
#ifdef CONFIG_CNAPT
#include <sc/cnapt/nf_cnapt.h>
#endif
#endif
MODULE_AUTHOR("Magnus Boden <mb@ozaba.mine.nu>");
MODULE_DESCRIPTION("TFTP NAT helper");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_nat_tftp");

static unsigned int help(struct sk_buff *skb,
			 enum ip_conntrack_info ctinfo,
			 struct nf_conntrack_expect *exp)
{
	const struct nf_conn *ct = exp->master;

	exp->saved_proto.udp.port
		= ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port;
	exp->dir = IP_CT_DIR_REPLY;
	exp->expectfn = nf_nat_follow_master;
#ifdef __SC_BUILD__
#ifdef CONFIG_CNAPT
	/* are we going to expect a connection dst to pubip ? */

	if (hooks && hooks->pubip_related((struct nf_conn *)ct, exp->tuple.dst.u3.ip)) {
		void *calg;
		u_int16_t algport, nport;

		calg = hooks->alloc(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip, // privip
						exp->tuple.dst.u3.ip, // pubip
						ntohs(exp->saved_proto.udp.port), // privport
						0, // pubport
						exp->tuple.dst.protonum // protocol
						);
		if (!calg)
			return NF_DROP;

		algport = ntohs(exp->tuple.dst.u.udp.port);
		nport = algport + 1;
		if (hooks->add_expect(calg, exp, &algport, &nport) == 0) {
			if (nf_ct_expect_related(exp) != 0) {
			    hooks->del_expect(exp);
			    algport = 0;
			}
		} else {
		    algport = 0;
		}
		hooks->put(calg);

		if (algport == 0)
		    return NF_DROP;

	} else
#endif
#endif
	if (nf_ct_expect_related(exp) != 0) {
		nf_ct_helper_log(skb, exp->master, "cannot add expectation");
		return NF_DROP;
	}
	return NF_ACCEPT;
}

static void __exit nf_nat_tftp_fini(void)
{
	RCU_INIT_POINTER(nf_nat_tftp_hook, NULL);
	synchronize_rcu();
}

static int __init nf_nat_tftp_init(void)
{
	BUG_ON(nf_nat_tftp_hook != NULL);
	RCU_INIT_POINTER(nf_nat_tftp_hook, help);
	return 0;
}

module_init(nf_nat_tftp_init);
module_exit(nf_nat_tftp_fini);
