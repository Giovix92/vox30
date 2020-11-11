/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <errno.h>
#include <syslog.h>
#include <stdio.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>

#include "lib/config.h"
#include "lib/event.h"

#include "hotspotd.h"
#include "unl.h"


static struct unl *unl_rt_event = NULL;
#ifdef __SC_BUILD__
static char *iface = NULL;
static char *iface_eap = NULL;
#ifdef CONFIG_SUPPORT_WIFI_5G
static char *iface_eap_5g = NULL;
static int eap_need_up = 0;
static int eap_5g_need_up = 0;
#endif
#else
static char iface[IFNAMSIZ] = {0};
static char iface_eap[IFNAMSIZ] = {0};
#endif
#ifdef __SC_BUILD__
static int ifindex = -1, ifindex_eap = -1,  iface_eap_up = 0;
#else
static int ifindex = -1, ifindex_eap = -1, iface_up = 0, iface_eap_up = 0;
#endif
static unsigned cfg_delay = 0;
static bool eap_ignore = true;
static void loader_event(struct event_epoll *event, uint32_t revents);
static void loader_callcontrol(struct event_timer *event, int64_t now);
static void loader_callreload(struct event_signal *ev, const siginfo_t *si);
static void loader_load(bool init);

static struct event_epoll event_rtnetlink = {
	.handler = loader_event,
	.events = EPOLLIN | EPOLLET,
};

static struct event_timer event_callcontrol = {
	.handler = loader_callcontrol,
};

static struct event_signal event_callreload = {
	.handler = loader_callreload,
	.signal = SIGUSR1,
};


static int loader_apply() {
	cfg_delay = config_get_int("main", "loader_delay", 2500);

	// loader module must be aware of eap status in order to consider iface_eap
	// for reapplying non-resident modules in following interface events. This
	// parameter may be moved from "eapradserv" section to "main" for semantic
	// coherence in the future.
	if (config_section_exists("eapradserv")) {
		const char *status = config_get_string("eapradserv", "eapstatus", NULL);
		eap_ignore = (status && (!strcmp(status, "visible") || !strcmp(status,
				"hidden")))?false:true;
	} else {
		eap_ignore = true;
	}

	// Synthesize initial interface events
	uint8_t buffer[64] = {0};
	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_GETLINK, NLM_F_DUMP);
	nlmsg_claim(nh, sizeof(struct rtgenmsg));
	unl_request(unl_rt_event, nh);

	return 0;
}

static int loader_init() {
	if (!(unl_rt_event = unl_open(NETLINK_ROUTE, NULL)))
		return -1;

	/* Subscribe to link events */
	event_rtnetlink.fd = event_nonblock(unl_fd(unl_rt_event));
	event_ctl(EVENT_EPOLL_ADD, &event_rtnetlink);
	unl_subscribe(unl_rt_event, RTNLGRP_LINK);

	/* Register disabled deinit timer. We need to call deinit from
	a timer as calling it from within an epoll event can be racy */
	event_callcontrol.value = UINT32_MAX;
	event_ctl(EVENT_TIMER_ADD, &event_callcontrol);

	/* Register SIGUSR1 handler */
	event_ctl(EVENT_SIGNAL_ADD, &event_callreload);
#ifndef __SC_BUILD__
	const char *cfg_iface = config_get_string("main", "iface", NULL);
	if(!hotspot_assertconf_string("main.iface", cfg_iface)) {
		return -1;
	}
#endif
#ifdef __SC_BUILD__
    iface = routing_cfg.iface;
#else
	strncpy(iface, cfg_iface, sizeof(iface) - 1);
#endif
#ifndef __SC_BUILD__
	const char *cfg_iface_eap = config_get_string("main", "iface_eap", NULL);
	if(!hotspot_assertconf_string("main.iface_eap", cfg_iface_eap)) {
		return -1;
	}

#endif
#ifdef __SC_BUILD__
    iface_eap = routing_cfg.iface_eap;
#ifdef CONFIG_SUPPORT_WIFI_5G
    iface_eap_5g = routing_cfg.iface_eap_5g;
#endif
#else
	strncpy(iface_eap, cfg_iface_eap, sizeof(iface_eap) - 1);
#endif
	return loader_apply();
}

static void loader_deinit() {
	event_ctl(EVENT_SIGNAL_DEL, &event_callreload);
	event_ctl(EVENT_TIMER_DEL, &event_callcontrol);
	if (unl_rt_event) {
		unl_close(unl_rt_event);
		unl_rt_event = NULL;
	}
}

static void loader_load(bool init) {
	if (init) {
		event_callcontrol.context = (void*)HOTSPOT_CTRL_INIT;
		event_callcontrol.value = cfg_delay;
		event_ctl(EVENT_TIMER_MOD, &event_callcontrol);
	} else if (!init) {
		event_callcontrol.context = (void*)HOTSPOT_CTRL_DEINIT;
		event_callcontrol.value = 0;
		event_ctl(EVENT_TIMER_MOD, &event_callcontrol);
	}
}

// This is called whenever a network interface changes it state
static void loader_event(struct event_epoll *event, uint32_t revents) {
	for (;;) {
		struct nlmsghdr *nh = unl_receive(unl_rt_event);
		if (!nh && errno == EWOULDBLOCK) {
			break;
		} else if (!nh) {
			continue;
		}
		if (nh->nlmsg_type == RTM_NEWADDR) {
            /*
			struct ifaddrmsg *ifa = NLMSG_DATA(nh);
			siginfo_t sig = {
				.si_signo = SIGNAL_ROUTING_IFSTATE,
				.si_errno = ifa->ifa_family,
				.si_code = ifa->ifa_index
			};

			event_trigger(&sig, EVENT_NOW);
			continue;
            */
		} else if (nh->nlmsg_type == RTM_NEWLINK) {
			struct ifinfomsg *ifi = NLMSG_DATA(nh);
			char *ifname = nla_get_string(nlmsg_find(nh,
					sizeof(*ifi), IFLA_IFNAME), NULL);

/*			siginfo_t sig = {
				.si_signo = SIGNAL_ROUTING_IFSTATE,
				.si_errno = ifi->ifi_flags,
				.si_ptr = ifname,
				.si_code = ifi->ifi_index
			};
            */

			DPRINTF("Interface event: %s (%i) state "
					"changed to %x", ifname,
					ifi->ifi_index, ifi->ifi_flags);
#ifndef __SC_BUILD__
			syslog(LOG_INFO, "Interface event: %s (%i) state "
					"changed to %x", ifname,
					ifi->ifi_index, ifi->ifi_flags);
#endif
#ifndef __SC_BUILD__
			if (ifname && !strcmp(iface, ifname)) {
				ifindex = ifi->ifi_index;
				iface_up = ifi->ifi_flags & IFF_UP;
				loader_load(iface_up && ((!eap_ignore && iface_eap_up) ||
						eap_ignore));
			} else if (!eap_ignore && ifname && !strcmp(iface_eap, ifname)) {
#else
#ifdef CONFIG_SUPPORT_WIFI_5G
            if(ifname && (!strcmp(iface_eap, ifname) || !strcmp(iface_eap_5g, ifname))){
#else
            if(ifname && !strcmp(iface_eap, ifname)){
#endif
#endif
				ifindex_eap = ifi->ifi_index;
				iface_eap_up = ifi->ifi_flags & IFF_UP;
#ifdef CONFIG_SUPPORT_WIFI_5G
                if(!strcmp(iface_eap_5g, ifname))
                    eap_5g_need_up = iface_eap_up;
                else
                    eap_need_up = iface_eap_up;
#endif
#ifndef __SC_BUILD__
				loader_load(iface_up && iface_eap_up);
#else
#ifdef CONFIG_SUPPORT_WIFI_5G
				loader_load(eap_need_up || eap_5g_need_up);
#else
				loader_load(iface_eap_up);
#endif
#endif
			}

			// We propagate the changed interface state internally
#ifndef __SC_BUILD__
			event_trigger(&sig, EVENT_NOW);
#endif
			continue;
		} else if (nh->nlmsg_type == RTM_DELLINK) {
			struct ifinfomsg *ifi = NLMSG_DATA(nh);
			if (ifi->ifi_index == ifindex || (!eap_ignore && (ifi->ifi_index ==
					ifindex_eap)) ) {
				ifindex = -1;
				ifindex_eap = -1;
				loader_load(0);
			}
		}
	}
}

// Indirection for hotspot state change
static void loader_callcontrol(struct event_timer *event, int64_t now) {
	enum hotspot_control cmd = (uintptr_t)event->context;
	int status = hotspot_control(cmd);
	if (status && cmd == HOTSPOT_CTRL_INIT) { // Check for failures in init
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "hotspot setup failed, rolling back...");
#else
		syslog(LOG_WARNING, "hotspot setup failed, rolling back...");
#endif
		hotspot_control(HOTSPOT_CTRL_DEINIT);
	}
}

// Allow soft-reload when SIGUSR1 is received
static void loader_callreload(struct event_signal *ev, const siginfo_t *si) {
	config_init(NULL);
	if (config_get_string("main", "restart", NULL)) {
		config_set_string("main", "restart", NULL);
		config_commit();
		raise(SIGHUP);
	} else {
		hotspot_control(HOTSPOT_CTRL_APPLY);
	}
}

RESIDENT_REGISTER(loader, 110)
