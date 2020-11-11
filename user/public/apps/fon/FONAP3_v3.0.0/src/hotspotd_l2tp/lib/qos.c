/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <net/ethernet.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <linux/gen_stats.h>
#include <linux/tc_act/tc_mirred.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unl.h>

#include "qos.h"

static const char *kind = "hfsc";

// This equals to:
// tc qdisc add dev srcif ingress
// tc filter add dev srcif parent ffff: u32 match mark 0 0 action mirred egress redirect dev dstif
// or if dstif == 0:  tc qdisc del dev srcif parent ffff:
int qos_ingress(struct unl *unl, int srcif, int dstif, uint32_t ehandle) {
	uint8_t buffer[256];
	struct tcmsg tcm = {
		.tcm_ifindex = srcif,
		.tcm_parent = (!ehandle) ? TC_H_INGRESS : TC_H_ROOT,
		.tcm_info = htons(ETH_P_ALL),
		.tcm_handle = ehandle,
	};

	// Delete the ingress qdisc if we have no target iface
	if (!dstif) {
		struct nlmsghdr *nh = nlmsg_init(buffer, RTM_DELQDISC, 0);
		nlmsg_append(nh, &tcm, sizeof(tcm));
		return unl_call(unl, nh);
	}

	// Create the ingress qdisc
	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_NEWQDISC, NLM_F_CREATE);
	nlmsg_append(nh, &tcm, sizeof(tcm));
	if (!ehandle) {
		nlmsg_put_string(nh, TCA_KIND, "ingress");
		nlmsg_claim_attr(nh, TCA_OPTIONS, 0);
	} else {
		struct tc_hfsc_qopt qopt = { .defcls = 0 };
		nlmsg_put_string(nh, TCA_KIND, kind);
		nlmsg_put(nh, TCA_OPTIONS, &qopt, sizeof(qopt));
	}

	if (unl_call(unl, nh))
		return -1;

	tcm.tcm_parent = (!ehandle) ? TC_H_MAJ(TC_H_INGRESS) : ehandle;

	struct tc_mirred m = {
		.eaction = TCA_EGRESS_REDIR,
		.action = TC_ACT_STOLEN,
		.ifindex = dstif,
	};

	// Nesting... FUCK YEAH!
	nh = nlmsg_init(buffer, RTM_NEWTFILTER, NLM_F_CREATE);
	nlmsg_append(nh, &tcm, sizeof(tcm));
	nlmsg_put_string(nh, TCA_KIND, "basic");
	struct nlattr *nla = nlmsg_start_attr(nh, TCA_OPTIONS);
		struct nlattr *nla_act = nla_start_attr(nla, TCA_BASIC_ACT);
#ifndef QOS_NO_CONNMARK
			struct nlattr *nla_act1 = nla_start_attr(nla_act, 1);
				nla_put_string(nla_act1, TCA_ACT_KIND, "connmark");
				nla_claim_attr(nla_act1, TCA_ACT_OPTIONS, 0);
			nla_commit_attr(nla_act, nla_act1);
			struct nlattr *nla_act2 = nla_start_attr(nla_act, 2);
#else
			struct nlattr *nla_act2 = nla_start_attr(nla_act, 1);
#endif
				nla_put_string(nla_act2, TCA_ACT_KIND, "mirred");
				struct nlattr *nla_mirr = nla_start_attr(nla_act2, TCA_ACT_OPTIONS);
					nla_put(nla_mirr, TCA_MIRRED_PARMS, &m, sizeof(m));
				nla_commit_attr(nla_act2, nla_mirr);
			nla_commit_attr(nla_act, nla_act2);
		nla_commit_attr(nla, nla_act);
	nlmsg_commit_attr(nh, nla);

	return unl_call(unl, nh);
}

int qos_stat(struct unl *unl, int iface, uint32_t handle, uint64_t *bytes, uint32_t *packets) {
	uint8_t buffer[128];
	struct tcmsg tcm = {
		.tcm_ifindex = iface,
		.tcm_handle = handle
	};

	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_GETTCLASS, NLM_F_ECHO);
	nlmsg_append(nh, &tcm, sizeof(tcm));
	if (unl_request(unl, nh) < 0 || !(nh = unl_receive_response(unl)))
		return -1;

	struct nlattr *nla = nlmsg_find(nh, sizeof(tcm), TCA_STATS2);
	if (nla && (nla = nla_find(nla, TCA_STATS_APP))
	&& NLA_PAYLOAD(nla) >= sizeof(struct tc_hfsc_stats)) {
		struct tc_hfsc_stats *tcs = NLA_DATA(nla);
		*bytes = tcs->work;
		*packets = tcs->period;
		return 0;
	}
	return -1;
}

int qos_class(struct unl *unl, int iface, uint32_t parent, uint32_t handle, struct qos_hfsc *curves) {
	uint8_t buffer[3096];
	struct tcmsg tcm = {
		.tcm_ifindex = iface,
		.tcm_handle = handle,
		.tcm_parent = parent,
	};

	if (!parent || !curves) {
		struct nlmsghdr *nh = nlmsg_init(buffer, RTM_DELTCLASS, 0);
		nlmsg_append(nh, &tcm, sizeof(tcm));
		return unl_call(unl, nh);
	}

	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_NEWTCLASS, NLM_F_CREATE | NLM_F_REPLACE);
	nlmsg_append(nh, &tcm, sizeof(tcm));
	nlmsg_put_string(nh, TCA_KIND, kind);
	struct nlattr *nla = nlmsg_start_attr(nh, TCA_OPTIONS);

	if (curves->limit.m2)
		nla_put(nla, TCA_HFSC_USC, &curves->limit, sizeof(curves->limit));
	if (curves->realt.m2)
		nla_put(nla, TCA_HFSC_RSC, &curves->realt, sizeof(curves->realt));
	if (curves->share.m2)
		nla_put(nla, TCA_HFSC_FSC, &curves->share, sizeof(curves->share));
	nlmsg_commit_attr(nh, nla);

	return unl_call(unl, nh);
}

int qos_qdisc(struct unl *unl, int iface, uint32_t parent, uint32_t handle, uint16_t defclass) {
	uint8_t buffer[128];
	struct tcmsg tcm = {
		.tcm_ifindex = iface,
		.tcm_handle = handle,
		.tcm_parent = (parent) ? parent : TC_H_ROOT,
	};

	struct tc_hfsc_qopt qopt = {
		.defcls = defclass,
	};

	if (!handle) {
		struct nlmsghdr *nh = nlmsg_init(buffer, RTM_DELQDISC, 0);
		nlmsg_append(nh, &tcm, sizeof(tcm));
		return unl_call(unl, nh);
	}

	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_NEWQDISC, NLM_F_CREATE | NLM_F_REPLACE);
	nlmsg_append(nh, &tcm, sizeof(tcm));
	nlmsg_put_string(nh, TCA_KIND, kind);
	nlmsg_put(nh, TCA_OPTIONS, &qopt, sizeof(qopt));

	return unl_call(unl, nh);
}

int qos_filter_mark(struct unl *unl, int iface, uint16_t prio, uint32_t parent, uint32_t mark, uint32_t mask, uint32_t flow) {
	uint8_t buffer[128];
	struct tcmsg tcm = {
		.tcm_ifindex = iface,
		.tcm_parent = parent,
		.tcm_info = (prio << 16) | htons(ETH_P_ALL),
		.tcm_handle = mark,
	};

	if (!flow) {
		struct nlmsghdr *nh = nlmsg_init(buffer, RTM_DELTFILTER, 0);
		nlmsg_append(nh, &tcm, sizeof(tcm));
		return unl_call(unl, nh);
	}

	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_NEWTFILTER, NLM_F_CREATE);
	nlmsg_append(nh, &tcm, sizeof(tcm));
	nlmsg_put_string(nh, TCA_KIND, "fw");
	struct nlattr *nla = nlmsg_start_attr(nh, TCA_OPTIONS);
	nla_put_u32(nla, TCA_FW_MASK, mask);
	nla_put_u32(nla, TCA_FW_CLASSID, flow);
	nlmsg_commit_attr(nh, nla);

	return unl_call(unl, nh);
}

int qos_filter_basic(struct unl *unl, int iface, uint16_t prio, uint32_t parent, uint32_t flow) {
	uint8_t buffer[128];
	struct tcmsg tcm = {
		.tcm_ifindex = iface,
		.tcm_parent = parent,
		.tcm_info = (prio << 16) | htons(ETH_P_ALL),
	};

	if (!flow) {
		struct nlmsghdr *nh = nlmsg_init(buffer, RTM_DELTFILTER, 0);
		nlmsg_append(nh, &tcm, sizeof(tcm));
		return unl_call(unl, nh);
	}

	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_NEWTFILTER, NLM_F_CREATE);
	nlmsg_append(nh, &tcm, sizeof(tcm));
	nlmsg_put_string(nh, TCA_KIND, "basic");
	struct nlattr *nla = nlmsg_start_attr(nh, TCA_OPTIONS);
	nla_put_u32(nla, TCA_BASIC_CLASSID, flow);
	nlmsg_commit_attr(nh, nla);

	return unl_call(unl, nh);
}
