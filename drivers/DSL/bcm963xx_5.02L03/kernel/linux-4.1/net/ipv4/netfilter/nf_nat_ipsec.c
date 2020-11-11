#if defined(CONFIG_BCM_KF_PROTO_IPSEC)
/*
<:copyright-BRCM:2012:GPL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/

#include <linux/module.h>
#include <linux/udp.h>
#include <linux/ip.h>

#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <linux/netfilter/nf_conntrack_ipsec.h>

#ifdef __SC_BUILD__
#include <net/route.h>
#endif
MODULE_AUTHOR("Pavan Kumar <pavank@broadcom.com>");
MODULE_DESCRIPTION("Netfilter connection tracking module for ipsec");
MODULE_LICENSE("GPL");
MODULE_ALIAS("nf_nat_ipsec");

/* outbound packets == from LAN to WAN */
static int
ipsec_outbound_pkt(struct sk_buff *skb,
                   struct nf_conn *ct, enum ip_conntrack_info ctinfo)

{
#ifndef __SC_BUILD__
   struct iphdr *iph = ip_hdr(skb);
   struct udphdr *udph = (void *)iph + iph->ihl * 4;

   /* make sure source port is 500 */
   udph->source = htons(IPSEC_PORT);
   udph->check = 0;
#endif
   return NF_ACCEPT;
}


/* inbound packets == from WAN to LAN */
static int
ipsec_inbound_pkt(struct sk_buff *skb, struct nf_conn *ct,
                  enum ip_conntrack_info ctinfo, __be32 lan_ip)
{
   struct iphdr *iph = ip_hdr(skb);
   struct udphdr *udph = (void *)iph + iph->ihl * 4;

   iph->daddr = lan_ip;
   udph->check = 0;
   iph->check = 0;
   iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
   
#ifdef __SC_BUILD__
	 if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip != lan_ip)
	 {
		int err;
		if(skb_dst(skb))
		    dst_release(skb_dst(skb));
		err = ip_route_input(skb, iph->daddr, iph->saddr, iph->tos,
					 (skb)->dev);
		if (unlikely(err)) {
                    printk("WARNNING: Packet can not be routed\n");
   			return NF_DROP;

		}
	 }
#endif
   return NF_ACCEPT;
}

static int __init nf_nat_helper_ipsec_init(void)
{
   BUG_ON(nf_nat_ipsec_hook_outbound != NULL);
   RCU_INIT_POINTER(nf_nat_ipsec_hook_outbound, ipsec_outbound_pkt);

   BUG_ON(nf_nat_ipsec_hook_inbound != NULL);
   RCU_INIT_POINTER(nf_nat_ipsec_hook_inbound, ipsec_inbound_pkt);

   return 0;
}

static void __exit nf_nat_helper_ipsec_fini(void)
{
	RCU_INIT_POINTER(nf_nat_ipsec_hook_inbound, NULL);
	RCU_INIT_POINTER(nf_nat_ipsec_hook_outbound, NULL);

	synchronize_rcu();
}

module_init(nf_nat_helper_ipsec_init);
module_exit(nf_nat_helper_ipsec_fini);
#endif
