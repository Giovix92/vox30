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
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>
#include <sys/utsname.h>

#include "libunl/unl.h"

#include "client.h"
#ifndef __SC_BUILD__
#include "routing.h"
#endif
#include "hotspotd.h"
#include "neigh.h"

#include "lib/hexlify.h"
#include "lib/event.h"
#include "lib/list.h"

static struct unl *unl = NULL;
static struct list_head entries = LIST_HEAD_INIT(entries);
static struct list_head handlers = LIST_HEAD_INIT(handlers);
static bool old_neigh = false;
static void neigh_event(struct event_epoll *ev, uint32_t revents);
static void neigh_sync(struct event_timer *ev, int64_t now);

static struct event_epoll ev_neigh = {
	.fd = -1,
	.events = EPOLLIN | EPOLLET,
	.handler = neigh_event
};

static struct event_timer ev_sync = {
	.value = 1000,
	.interval = 1000,
	.handler = neigh_sync
};


struct neigh_entry {
	struct list_head head;
	int ifindex;
	bool marked;
	uint8_t family;
	uint8_t mac[6];
	union {
		struct in_addr in;
		struct in6_addr in6;
	} addr;
};


static void neigh_synthesize(void)
{
	// Synthesize initial events
	char buffer[128] = {0};
	struct nlmsghdr *nh = nlmsg_init(buffer, RTM_GETNEIGH, NLM_F_DUMP);
	struct ndmsg *ndm = nlmsg_claim(nh, sizeof(*ndm));
	ndm->ndm_ifindex = routing_cfg.iface_index;

	ndm->ndm_family = AF_INET;
	unl_request(unl, nh);
	ndm->ndm_family = AF_INET6;
	unl_request(unl, nh);

	if (routing_cfg.iface_eap_index && routing_cfg.iface_eap_index !=
			routing_cfg.iface_index) {
		ndm->ndm_ifindex = routing_cfg.iface_eap_index;
		unl_request(unl, nh);
		ndm->ndm_family = AF_INET;
		unl_request(unl, nh);
	}

	if (routing_cfg.iface_priv_index && routing_cfg.iface_priv_index !=
			routing_cfg.iface_index) {
		ndm->ndm_ifindex = routing_cfg.iface_priv_index;
		unl_request(unl, nh);
		ndm->ndm_family = AF_INET6;
		unl_request(unl, nh);
	}
}

static int neigh_module_init()
{
	neigh_synthesize();
	return 0;
}

static int neigh_module_apply()
{
	return 0;
}

static void neigh_module_deinit()
{
	return;
}

static int neigh_apply()
{
	return 0;
}


static int neigh_init()
{
	unl = unl_open(NETLINK_ROUTE, NULL);
	if (!unl)
		return -1;

	unl_subscribe(unl, RTNLGRP_NEIGH);

	struct utsname uts;
	uname(&uts);
	old_neigh = !strncmp(uts.release, "2.6.21", strlen("2.6.21")) ||
				!strncmp(uts.release, "2.6.22", strlen("2.6.22")) ||
				!strncmp(uts.release, "2.6.23", strlen("2.6.23"));

	neigh_synthesize();

	ev_neigh.fd = event_nonblock(unl_fd(unl));
	event_ctl(EVENT_EPOLL_ADD, &ev_neigh);

	if (old_neigh)
		event_ctl(EVENT_TIMER_ADD, &ev_sync);

	return 0;
}


static void neigh_deinit()
{
	while (!list_empty(&entries)) {
		struct neigh_entry *e = list_first_entry(&entries,
				struct neigh_entry, head);
		list_del(&e->head);
		free(e);
	}

	if (old_neigh)
		event_ctl(EVENT_TIMER_DEL, &ev_sync);

	unl_close(unl);
	unl = NULL;
}


int neigh_ip2mac(int ifindex, uint8_t hwaddr[6], int af, const void *addr)
{
	struct neigh_entry *e;
	size_t addrlen = (af == AF_INET6) ? 16 : 4;

	list_for_each_entry(e, &entries, head) {
		if (e->ifindex == ifindex && e->family == af &&
				!memcmp(&e->addr, addr, addrlen)) {
			memcpy(hwaddr, e->mac, sizeof(e->mac));
			return 0;
		}
	}

	return -1;
}


ssize_t neigh_mac2ip(int ifindex, int af, void *addr, size_t len,
		const uint8_t hwaddr[6])
{
	struct neigh_entry *e;
	struct in_addr *in = addr;
	struct in6_addr *in6 = addr;

	ssize_t i = 0;
	list_for_each_entry(e, &entries, head) {
		if (i >= len || e->family != af || e->ifindex != ifindex ||
				memcmp(e->mac, hwaddr, sizeof(e->mac)))
			continue;

		if (af == AF_INET)
			in[i++] = e->addr.in;
		else if (af == AF_INET6)
			in6[i++] = e->addr.in6;
	}
	return i;
}

void neigh_handle_event(struct neigh_entry *e, enum neigh_event nevent)
{
	char ip[INET6_ADDRSTRLEN];
	struct neigh_handler *neh = NULL;
	list_for_each_entry(neh, &handlers, _head) {
		if (e->ifindex == neh->ifindex) {
			inet_ntop(e->family, &e->addr, ip, sizeof(ip));
			neh->cb(e->mac, ip, nevent);
			return;
		}
	}
}

static void neigh_event(struct event_epoll *ev, uint32_t revents)
{
	while (true) {
		struct nlmsghdr *nh = unl_receive(unl);
		if (!nh && errno == EWOULDBLOCK)
			break;
		else if (!nh)
			continue;

		struct ndmsg *ndm = NLMSG_DATA(nh);
		struct nlattr *dst = nlmsg_find(nh, sizeof(*ndm), NDA_DST);
		struct nlattr *ll = nlmsg_find(nh, sizeof(*ndm), NDA_LLADDR);

		if (!dst || (ndm->ndm_ifindex != routing_cfg.iface_eap_index &&
				ndm->ndm_ifindex != routing_cfg.iface_index &&
				ndm->ndm_ifindex != routing_cfg.iface_priv_index))
			continue;
	
		if (ndm->ndm_family == AF_INET6
				&& IN6_IS_ADDR_LINKLOCAL(NLA_DATA(dst)))
			continue;

		struct in_addr brd = {INADDR_BROADCAST};
		if (ndm->ndm_family == AF_INET && dst && NLA_PAYLOAD(dst) >= sizeof(brd) &&
				!memcmp(NLA_DATA(dst), &brd, sizeof(brd)))
			continue;

		bool add = (nh->nlmsg_type == RTM_NEWNEIGH && (ndm->ndm_state &
				(NUD_REACHABLE | NUD_STALE | NUD_DELAY | NUD_PROBE
						| NUD_PERMANENT | NUD_NOARP)));

		struct neigh_entry *c, *e = NULL;
		list_for_each_entry(c, &entries, head) {
			if (c->ifindex == ndm->ndm_ifindex && c->family
					== ndm->ndm_family && !memcmp(&c->addr,
					NLA_DATA(dst), NLA_PAYLOAD(dst))) {
				e = c;
				e->marked = true;
				break;
			}
		}


		char ip[INET6_ADDRSTRLEN];
		char macbuf[18];

		if (add && ll && !e) {
			if (!(e = malloc(sizeof(*e))))
				continue;

			e->family = ndm->ndm_family;
			e->ifindex = ndm->ndm_ifindex;
			e->marked = true;
			memcpy(e->mac, NLA_DATA(ll), NLA_PAYLOAD(ll));
			memcpy(&e->addr, NLA_DATA(dst), NLA_PAYLOAD(dst));

			list_add(&e->head, &entries);

			inet_ntop(e->family, &e->addr, ip, sizeof(ip));
			hexlify(macbuf, e->mac, sizeof(e->mac), '-');
#ifndef __SC_BUILD__
			syslog(LOG_NOTICE, "neigh: new address %s for %s",
					ip, macbuf);
#endif
			neigh_handle_event(e, NEIGH_ADD);
			client_inform_address_update(e->ifindex, e->mac);
		} else if (!add && e) {
			inet_ntop(e->family, &e->addr, ip, sizeof(ip));
			hexlify(macbuf, e->mac, sizeof(e->mac), '-');
#ifndef __SC_BUILD__
			syslog(LOG_NOTICE, "neigh: del address "
					"%s for %s", ip, macbuf);
#endif
			list_del(&e->head);
			neigh_handle_event(e, NEIGH_DEL);
			free(e);
		}
	}
}

void neigh_handler(struct neigh_handler *neh) {
	list_add(&neh->_head, &handlers);
}

void neigh_deregister_handler(struct neigh_handler *neh) {
	list_del(&neh->_head);
}

static void neigh_sync(struct event_timer *ev, int64_t now)
{
	struct neigh_entry *c, *n = NULL;
	list_for_each_entry_safe(c, n, &entries, head) {
		if (c->marked) {
			c->marked = false;
		} else {
			list_del(&c->head);
			neigh_handle_event(c, NEIGH_DEL);
			free(c);
		}
	}

	neigh_synthesize();
}


MODULE_REGISTER(neigh_module, 170)
RESIDENT_REGISTER(neigh, 170)
