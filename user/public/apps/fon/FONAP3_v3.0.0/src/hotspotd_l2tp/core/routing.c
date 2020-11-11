/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <stdint.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log.h>
#endif
#include <unistd.h>
#include <byteswap.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/fib_rules.h>
#include <linux/rtnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#include "lib/list.h"
#include "lib/event.h"
#include "lib/insmod.h"
#include "lib/config.h"
#include "lib/hexlify.h"
#include "client.h"
#include "routing.h"
#include "trigger.h"
#include "hotspotd.h"

#define ROUTING_TBLCNT (HOTSPOT_LIMIT_RES * 2)

static struct unl *unl_rt = NULL, *unl_nf = NULL;
static int ifindex_lo = -1;
static uint32_t rttbl_offset = 0;
static uint32_t rttbl_alloc[ROUTING_TBLCNT] = {0};
static const char *tr_disconnect = NULL;

struct routing_cfg routing_cfg = {.iface_index = 0};


static void routing_deinit();

static int routing_module_apply() {
	tr_disconnect = config_get_string("main", "trigger_disconnect", NULL);
	return 0;
}

static int routing_module_init()
{
	int iosock = socket(AF_UNIX, SOCK_DGRAM, 0);
	struct ifreq ifr;

	/* Detect ifindex of main interface */
	const char *iface = config_get_string("main", "iface", NULL);
	if(!hotspot_assertconf_string("main.iface", iface)) {
		return -1;
	}

	strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
	if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unable to resolve iface: %s", ifr.ifr_name);
#else
		syslog(LOG_ERR, "Unable to resolve iface: %s", ifr.ifr_name);
#endif
		close(iosock);
		return -1;
	}

	routing_cfg.iface_index = ifr.ifr_ifindex;
	strncpy(routing_cfg.iface_name, ifr.ifr_name,
			sizeof(routing_cfg.iface_name) - 1);

	if (!ioctl(iosock, SIOCGIFHWADDR, &ifr)) {
		memcpy(routing_cfg.iface_addr, ifr.ifr_hwaddr.sa_data, 6);
	} else {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Error setting hotspotd iface hw address\n");
#else
		syslog(LOG_ERR, "Error setting hotspotd iface hw address");
#endif
	}


	/* Detect ifindex of eap interface */
	const char *iface_eap = config_get_string("main", "iface_eap", NULL);
	if(!hotspot_assertconf_string("main.iface_eap", iface_eap)) {
		return -1;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, iface_eap, sizeof(ifr.ifr_name) - 1);
	routing_cfg.iface_eap_index = 0;
	if (!ioctl(iosock, SIOCGIFINDEX, &ifr)) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Resolve eap iface: %s\n", ifr.ifr_name);
#else
		syslog(LOG_INFO, "Resolve eap iface: %s", ifr.ifr_name);
#endif
		routing_cfg.iface_eap_index = ifr.ifr_ifindex;
		strncpy(routing_cfg.iface_eap_name, ifr.ifr_name,
				sizeof(routing_cfg.iface_eap_name) - 1);
	} else {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Error trying to resolve eap iface %s",  ifr.ifr_name);
#else
		syslog(LOG_ERR, "Error trying to resolve eap iface %s",  ifr.ifr_name);
		syslog(LOG_ERR, "ioctl returns %s", strerror(errno));
#endif
	}

	close(iosock);
	return routing_module_apply();
}

static void routing_module_deinit()
{
	for (int i = 0; i < ROUTING_TBLCNT; ++i)
		if (rttbl_alloc[i])
			routing_policy_del(i);
}

static int routing_apply()
{
	return routing_module_apply();
}

static int routing_init() {
	syslog(LOG_INFO, "Connecting to rtnetlink kernel interface");
	if (!(unl_rt = unl_open(NETLINK_ROUTE, NULL)))
		goto err;
	syslog(LOG_INFO, "Connecting to nfnetlink kernel interface");
	if (!(unl_nf = unl_open(NETLINK_NETFILTER, NULL)))
		goto err;

	unl_timeout(unl_rt, 1000);
	unl_timeout(unl_nf, 1000);

	memset(&routing_cfg, 0, sizeof(routing_cfg));
	rttbl_offset = config_get_int("main", "rttbl_offset", 10000);

	int iosock = socket(AF_UNIX, SOCK_DGRAM, 0);
	/* Detect ifindex of loopback */
	struct ifreq ifr = {.ifr_name = "lo"};
	if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unable to resolve loopback interface\n");
#else
		syslog(LOG_ERR, "Unable to resolve loopback interface");
#endif
		close(iosock);
		goto err;
	}
	ifindex_lo = ifr.ifr_ifindex;

	/* Detect ifindex of priv interface */
	const char *iface_priv = config_get_string("main", "iface_priv", NULL);
	if(!hotspot_assertconf_string("main.iface_priv", iface_priv)) {
		goto err;
	}
	strncpy(ifr.ifr_name, iface_priv, sizeof(ifr.ifr_name) - 1);
	routing_cfg.iface_priv_index = 0;
	if (!ioctl(iosock, SIOCGIFINDEX, &ifr)) {
		syslog(LOG_INFO, "Resolve priv iface: %s", ifr.ifr_name);
		routing_cfg.iface_priv_index = ifr.ifr_ifindex;
		strncpy(routing_cfg.iface_priv_name, ifr.ifr_name,
				sizeof(routing_cfg.iface_priv_name) - 1);
	}

	/* Detect ifindex of ifb interface */
	strncpy(ifr.ifr_name, config_get_string("main", "ifb", "<unset>"),
		sizeof(ifr.ifr_name) - 1);
	if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
		if (!strncmp(ifr.ifr_name, "imq", 3))
			insmod("imq", "");
		else
			insmod("ifb", "");

		if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
#ifdef __SC_BUILD__
            log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unable to resolve ifb interface: %s. "
			"You might need to load the ifb kernel module.",
								ifr.ifr_name);
#else
			syslog(LOG_ERR, "Unable to resolve ifb interface: %s. "
			"You might need to load the ifb kernel module.",
								ifr.ifr_name);
#endif
			close(iosock);
			goto err;
		}
	}
	routing_cfg.ifb_index = ifr.ifr_ifindex;
	strncpy(routing_cfg.ifb_name, ifr.ifr_name,
		sizeof(routing_cfg.ifb_name) - 1);

	/* Bring up ifb */
	ioctl(iosock, SIOCGIFFLAGS, &ifr);
	if (!(ifr.ifr_flags & IFF_UP)) {
		ifr.ifr_flags |= IFF_UP;
		ioctl(iosock, SIOCSIFFLAGS, &ifr);
	}

	strncpy(ifr.ifr_name, config_get_string("main", "ifb2", "<unset>"),
			sizeof(ifr.ifr_name) - 1);
	if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unable to resolve secondary ifb interface: "
				"%s", ifr.ifr_name);
#else
		syslog(LOG_ERR, "Unable to resolve secondary ifb interface: "
				"%s", ifr.ifr_name);
#endif
		close(iosock);
		goto err;
	}

	routing_cfg.ifb2_index = ifr.ifr_ifindex;
	strncpy(routing_cfg.ifb2_name, ifr.ifr_name,
		sizeof(routing_cfg.ifb2_name) - 1);

	/* Bring up ifb2 */
	ioctl(iosock, SIOCGIFFLAGS, &ifr);
	if (!(ifr.ifr_flags & IFF_UP)) {
		ifr.ifr_flags |= IFF_UP;
		ioctl(iosock, SIOCSIFFLAGS, &ifr);
	}

	close(iosock);

	// Enable kernel flow-accounting
	int fd = open("/proc/sys/net/netfilter/nf_conntrack_acct", O_WRONLY);
	if (write(fd, "1\n", 2)) {
		// Dummy
	}
	close(fd);

	return routing_apply();

	int errno_preserve;
err:
	errno_preserve = errno;
	routing_deinit();
	errno = errno_preserve;
	return -1;
}

static void routing_deinit() {
	if (unl_nf) {
		unl_close(unl_nf);
		unl_nf = NULL;
	}
	if (unl_rt) {
		unl_close(unl_rt);
		unl_rt = NULL;
	}
}

int routing_policy_new(const char *iface, int af, uint32_t fwmark,
		uint32_t fwmask, uint32_t ifindex) {
	int policy = -1, s1, s2;
	for (int i = 0; i < ROUTING_TBLCNT; ++i) {
		if (!rttbl_alloc[i]) {
			policy = i;
			break;
		}
	}
	if (policy < 0) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Out of routing policy slots. Leak?\n");
#else
		syslog(LOG_WARNING, "Out of routing policy slots. Leak?");
#endif
		return -1;
	}

	uint8_t addrbuf[16];
	routing_addresses(ifindex, af, addrbuf, 1);

	/* Create default route specifying destination iface */
	uint8_t buffer[128] = {0};
	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_NEWROUTE,
					NLM_F_CREATE | NLM_F_REPLACE);
	struct rtmsg *rtm = nlmsg_claim(nh, sizeof(*rtm));
	rtm->rtm_family = af;
	rtm->rtm_protocol = RTPROT_BOOT;
	nlmsg_put_u32(nh, RTA_TABLE, policy + rttbl_offset);
	if (ifindex == 0) {
		ifindex = ifindex_lo;
		rtm->rtm_scope = RT_SCOPE_HOST;
		rtm->rtm_type = RTN_LOCAL;
	} else {
		rtm->rtm_scope = RT_SCOPE_UNIVERSE;
		rtm->rtm_type = RTN_UNICAST;
		nlmsg_put(nh, RTA_GATEWAY, addrbuf, (af == AF_INET6) ? 16 : 4);
	}
	nlmsg_put_u32(nh, RTA_OIF, ifindex);
	if ((s1 = unl_call(unl_rt, nh)))
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to setup routing table %i: %s",
			policy + rttbl_offset, strerror(errno));
#else
		syslog(LOG_WARNING, "Failed to setup routing table %i: %s",
			policy + rttbl_offset, strerror(errno));
#endif
	/* Create policy rule using firewall mark */
	nh = nlmsg_init(buffer, RTM_NEWRULE, NLM_F_CREATE | NLM_F_REPLACE);
	rtm = nlmsg_claim(nh, sizeof(*rtm));

	nlmsg_put_string(nh, FRA_IFNAME, iface);
	nlmsg_put_u32(nh, FRA_FWMARK, fwmark);
	nlmsg_put_u32(nh, FRA_FWMASK, fwmask);

	rtm->rtm_type = RTN_UNICAST;
	nlmsg_put_u32(nh, FRA_TABLE, policy + rttbl_offset);
	if ((s2 = unl_call(unl_rt, nh)))
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to setup routing policy %i: %s",
			policy + rttbl_offset, strerror(errno));
#else
		syslog(LOG_WARNING, "Failed to setup routing policy %i: %s",
			policy + rttbl_offset, strerror(errno));
#endif

	rttbl_alloc[policy] = 1;
	if (!s1 && !s2) {
#ifdef __SC_BUILD__
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Created routing policy %i for target %u",
			policy, (unsigned)ifindex);
#else
		syslog(LOG_INFO, "Created routing policy %i for target %u",
			policy, (unsigned)ifindex);
#endif
		return policy;
	} else {
		routing_policy_del(policy);
		return -1;
	}
}

int routing_policy_del(int policy) {
	if (policy < 0 || policy >= ROUTING_TBLCNT || !rttbl_alloc[policy])
		return -1;
	syslog(LOG_INFO, "Deleting routing policy %i", policy);
	static int afs[2] = {AF_INET, AF_INET6};

	for (int i = 0; i < sizeof(afs) / sizeof(*afs); ++i) {
		/* Delete default route */
		uint8_t buffer[64] = {0};
		struct nlmsghdr *nh = nlmsg_init(buffer, RTM_DELROUTE, 0);
		struct rtmsg *rtm = nlmsg_claim(nh, sizeof(*rtm));
		rtm->rtm_family = afs[i];
		rtm->rtm_type = RTN_UNICAST;
		nlmsg_put_u32(nh, RTA_TABLE, policy + rttbl_offset);
		unl_call(unl_rt, nh);

		/* Delete policy rule for table */
		nh = nlmsg_init(buffer, RTM_GETRULE, NLM_F_DUMP);
		rtm = nlmsg_claim(nh, sizeof(*rtm));

		if (unl_request(unl_rt, nh) > 0) {
			unl_foreach_response(nh, unl_rt) {
				uint32_t table = nla_get_u32(nlmsg_find(nh,
					sizeof(*rtm), FRA_TABLE), 0);
				if (table == policy + rttbl_offset) {
					nh->nlmsg_type = RTM_DELRULE;
					unl_request(unl_rt, nh);
					unl_flush(unl_rt);
					break;
				}
			}
		}
	}

	rttbl_alloc[policy] = 0;
	return 0;
}

int routing_connkill(int af, const void *addr) {
	char buffer[128] = {0};
	struct nlmsghdr *nh = genlmsg_init(buffer,
		(NFNL_SUBSYS_CTNETLINK << 8) | IPCTNL_MSG_CT_GET,
		NLM_F_DUMP, af, 0);

	int req = unl_request(unl_nf, nh);
	if (req < 0) /* Dump connections */
		return -1;

	size_t len = (af == AF_INET) ? 4 : 16;
	uint16_t ip_attr = (af == AF_INET) ? CTA_IP_V4_SRC : CTA_IP_V6_SRC;

	while ((nh = unl_receive(unl_nf))
			&& (nh->nlmsg_seq != req || !nlmsg_complete(nh))) {
		if (nh->nlmsg_seq != req)
			continue;

		struct nlattr *attr = genlmsg_find(nh, 0, CTA_TUPLE_ORIG), *c;
		if (!attr || !(c = nla_find(attr, CTA_TUPLE_IP))
		|| !(c = nla_find(c, ip_attr)) || NLA_PAYLOAD(c) != len
		|| memcmp(NLA_DATA(c), addr, len))
			continue;

		/* construct mark request */
		nh = genlmsg_init(buffer, (NFNL_SUBSYS_CTNETLINK << 8)
			| IPCTNL_MSG_CT_DELETE, 0, af, 0);

		/* simply copy the orig tuple to select the right connection */
		nlmsg_append(nh, attr, attr->nla_len);
		unl_request(unl_nf, nh);
    };

	unl_flush(unl_nf);
    return 0;
}

int routing_disconnect(int iface, const uint8_t hwaddr[6])
{
	struct trigger_handle *hndl = NULL;

	if (!tr_disconnect)
		return 0;

	char macbuf[24];
	heXlify(macbuf, hwaddr, 6, ':');
	char *argv[3] = {routing_cfg.iface_name, macbuf, NULL};

	if (iface == routing_cfg.iface_eap_index)
		argv[0] = routing_cfg.iface_eap_name;

	if (iface == routing_cfg.iface_priv_index)
		argv[0] = routing_cfg.iface_priv_name;

	hndl = trigger_run(tr_disconnect, argv, NULL, NULL, NULL);
	if (hndl) {
		trigger_wait(hndl);
	}
	return -!hndl;
}

/* detect the primary address of the main interface */
ssize_t routing_addresses(int iface, int af, void *addr, size_t len)
{
	char buffer[64] = {0};
	size_t i = 0;
	struct in_addr *inaddr = addr;
	struct in6_addr *in6addr = addr;
	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_GETADDR, NLM_F_DUMP);
	struct ifaddrmsg *ifa = nlmsg_claim(nh, sizeof(*ifa));
	ifa->ifa_family = af;
	ifa->ifa_index = iface;

	if (unl_request(unl_rt, nh) < 0)
		return -1;

	unl_foreach_response(nh, unl_rt) {
		ifa = NLMSG_DATA(nh);
		if (ifa->ifa_index == iface
		&& ifa->ifa_scope == RT_SCOPE_UNIVERSE) {
			struct nlattr *nla = nlmsg_find(nh, sizeof(*ifa), IFA_ADDRESS);
			if (nla && NLA_PAYLOAD(nla) == ((af == AF_INET) ? 4 : 16)) {
				if (af == AF_INET && i < len)
					inaddr[i++] = *((struct in_addr*)NLA_DATA(nla));
				else if (af == AF_INET6 && i < len)
					in6addr[i++] = *((struct in6_addr*)NLA_DATA(nla));
			}
		}
	}

	return i;
}

RESIDENT_REGISTER(routing, 150)
MODULE_REGISTER(routing_module, 150)
