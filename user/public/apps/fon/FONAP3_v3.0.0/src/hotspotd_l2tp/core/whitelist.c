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
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unl.h>
#include <unistd.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log.h>
#endif
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netdb.h>

// ... no comment
#include <linux/types.h>
#ifndef aligned_be64
#define aligned_be64 __be64 __attribute__((aligned(8)))
#endif
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink_log.h>
#include <linux/netfilter/nfnetlink_queue.h>

#include "lib/event.h"
#include "lib/list.h"
#include "lib/config.h"
#include "lib/dns.h"
#include "routing.h"
#include "firewall.h"
#include "hotspotd.h"

static struct list_head hosts = LIST_HEAD_INIT(hosts);	/* Hosts */
static struct whitelist_route *routes = NULL;		/* Addresses */
static struct unl *unl_nfqueue = NULL;
static bool use_nfqueue = true;

static struct whitelist_config {
	uint32_t timeout;		/* Rule timeout */
	uint16_t nfqueueid;
} cfg = { .timeout = 0 };

enum whitelist_type {
	WL_NONE,		/* Remove */
	WL_STATIC,		/* Static host */
	WL_HOST,		/* Single host */
	WL_ZONE,		/* This host and all children */
	WL_EXCLUDE,		/* Excluded (blacklisted) */
	WL_DOMAIN		/* host, zone or exclude */
};

struct whitelist_host {
	struct list_head _head;
	uint16_t type;
	uint16_t len;
	char name[];
};

struct whitelist_route {
	struct whitelist_route *next;
	int32_t timeout;
	struct in6_addr addr;
	uint8_t prefix;
};

#define DNS_BUFSIZE	1500

static void whitelist_nfqueue_process(struct event_epoll *event,
						uint32_t revents);
static struct whitelist_host* whitelist_get(const char *hostname,
						uint16_t len);
static void whitelist_table_clean(struct event_timer *timer, int64_t now);
static int whitelist_table_add(int af, const void *addr, uint8_t prefix,
						int32_t timeout);
static int whitelist_table_matches(const char *name, int len);
static void whitelist_parse_config(const char *host, void *ctx);
static void whitelist_parse_dns(const char *host, void *ctx);
static void whitelist_flush();
static void whitelist_deinit();
static int whitelist_nfqueue(void);
static int whitelist_set(const char *hostname, enum whitelist_type type);

static struct event_epoll event_nflog = {
	.events = EPOLLIN | EPOLLET,
	.handler = whitelist_nfqueue_process,
};

static struct event_timer event_timer_routes = {
	.interval = 0,
	.handler = whitelist_table_clean,
};

static int whitelist_apply() {
	int ret = 0;

	// Flush the hostname entries
	whitelist_flush();

	// Static IP-based whitelisting entries are stored with timeout INT32_MAX
	// so expire them to let GC collect them
	struct whitelist_route *r;
	for (r = routes; r; r = r->next)
		if (r->timeout == INT32_MAX)
			r->timeout = 0;

	// Read whitelist config
	enum whitelist_type t = WL_HOST;
	config_foreach_list("whitelist", "host", whitelist_parse_config, &t);
	t = WL_STATIC;
	config_foreach_list("whitelist", "static", whitelist_parse_config, &t);
	t = WL_ZONE;
	config_foreach_list("whitelist", "zone", whitelist_parse_config, &t);
	t = WL_EXCLUDE;
	config_foreach_list("whitelist", "exclude", whitelist_parse_config, &t);
	t = WL_DOMAIN;
	config_foreach_list("whitelist", "domain", whitelist_parse_config, &t);


	const char *redir = config_get_string("redirect", "url", NULL);
	if(!redir) {
		redir = redirect_url_default;
	}

	if(hotspot_assertconf_string("redirect.url", redir)) {
		char buffer[280];
		size_t redirlen = strlen(redir);
		if (redirlen >= sizeof(buffer))
			redirlen = sizeof(buffer) - 1;
		memcpy(buffer, redir, redirlen);
		buffer[redirlen] = 0;

		char *host = NULL, *saveptr;
		char *http = strtok_r(buffer, "/", &saveptr);
		if (http)
			host = strtok_r(NULL, "/", &saveptr);

		if (host)
			whitelist_set(host, WL_STATIC);
	} else {
		ret = -1;
	}

	return ret;
}

static int whitelist_init() {
	cfg.timeout = config_get_int("whitelist", "timeout", 600);
	cfg.nfqueueid = config_get_int("whitelist", "nflogid",
					routing_cfg.iface_index);

	// The timer for the route garbage collector
	event_timer_routes.interval = 10000;
	event_ctl(EVENT_TIMER_ADD, &event_timer_routes);
	use_nfqueue = !!strncmp(routing_cfg.ifb_name, "imq", 3);

	if (!(unl_nfqueue = unl_open(NETLINK_NETFILTER, NULL)) ||
			unl_timeout(unl_nfqueue, 1000) || whitelist_nfqueue())
		goto err;

	if (firewall_set_nfqueue_out(IPPROTO_UDP, 53, cfg.nfqueueid, true,
			use_nfqueue))
		goto err;

	bool enable = true;
	config_foreach_list("whitelist", "dns", whitelist_parse_dns, &enable);

	event_nflog.fd = event_nonblock(unl_fd(unl_nfqueue));
	event_ctl(EVENT_EPOLL_ADD, &event_nflog);

	return whitelist_apply();

	int preserve_errno;
err:
	preserve_errno = errno;
	whitelist_deinit();
	errno = preserve_errno;
	return -1;
}

static void whitelist_deinit() {
#ifdef __SC_BUILD__
    char buffer[128] = {0};
	int subsys = (use_nfqueue) ?
			((NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_CONFIG):
			((NFNL_SUBSYS_ULOG << 8) | NFULNL_MSG_CONFIG);

	struct nlmsghdr *nh = genlmsg_init(buffer, subsys, 0, 0, 0);
	struct nfgenmsg *ng = NLMSG_DATA(nh);
	ng->nfgen_family = AF_UNSPEC;
	ng->res_id = 0;

	if (unl_nfqueue)
    {
        if (use_nfqueue) {
            struct nfqnl_msg_config_cmd *cmd = nlmsg_claim_attr(
                 nh, NFQA_CFG_CMD, sizeof(*cmd));

            cmd->command = NFQNL_CFG_CMD_PF_UNBIND;
            // un-Register AF_INET log handler
            cmd->pf = htons(AF_INET);
            unl_call(unl_nfqueue, nh);
               
            // un-Register AF_INET6 log handler
            cmd->pf = htons(AF_INET6);
            unl_call(unl_nfqueue, nh);

            // un-Register queue
            cmd->command = NFQNL_CFG_CMD_UNBIND;
            cmd->pf = 0;
            ng->res_id = htons(cfg.nfqueueid);
            unl_call(unl_nfqueue, nh);

        } else {
            // un-Register AF_INET log handler
            ng->nfgen_family = AF_INET;
            nlmsg_put_u8(nh, NFULA_CFG_CMD, NFULNL_CFG_CMD_PF_UNBIND);
            unl_call(unl_nfqueue, nh);

            // Register AF_INET6 log handler
            ng->nfgen_family = AF_INET6;
            unl_call(unl_nfqueue, nh);

            nh = genlmsg_init(buffer,
            (NFNL_SUBSYS_ULOG << 8) | NFULNL_MSG_CONFIG, 0, 0, 0);
            ng->res_id = htons(cfg.nfqueueid);
            ng->nfgen_family = 0;
            nlmsg_put_u8(nh, NFULA_CFG_CMD, NFULNL_CFG_CMD_UNBIND);
            unl_call(unl_nfqueue, nh);
        }
    }
#endif
	if (unl_nfqueue) {
		unl_close(unl_nfqueue);
		unl_nfqueue = NULL;
	}

	whitelist_table_clean(NULL, 0);
	whitelist_flush();

	bool enable = false;
	config_foreach_list("whitelist", "dns", whitelist_parse_dns, &enable);

	firewall_set_nfqueue_out(IPPROTO_UDP, 53, cfg.nfqueueid, false,
			use_nfqueue);

	if (event_timer_routes.interval) {
		event_ctl(EVENT_TIMER_DEL, &event_timer_routes);
		event_timer_routes.interval = 0;
	}
}

// The whitelist control "frontend"
static int whitelist_set(const char *hostname, enum whitelist_type type) {
	const size_t hostlen = strlen(hostname);
	uint8_t prefix = 255;
	char buffer[INET6_ADDRSTRLEN] = {0}, *p;
	struct in_addr addr;
	struct in6_addr addr6;

	// If we have a prefix, remove it and put IP-address in buffer
	if ((p = strchr(hostname, '/')) && (p - hostname) < sizeof(buffer)) {
		prefix = atoi(p + 1);
		memcpy(buffer, hostname, p - hostname);
		hostname = buffer;
	}

	if (inet_pton(AF_INET6, hostname, &addr6) == 1) {
		if (prefix > 128)
			prefix = 128;
		whitelist_table_add(AF_INET6, &addr6, prefix, (type) ? INT32_MAX : -1);
	} else if (inet_pton(AF_INET, hostname, &addr) == 1) {
		if (prefix > 32)
			prefix = 32;
		whitelist_table_add(AF_INET, &addr, prefix, (type) ? INT32_MAX : -1);
	} else if (type == WL_STATIC) {
		whitelist_set(hostname, WL_HOST);

		struct addrinfo *result, *rp;
		struct addrinfo hints = {
			.ai_family = AF_UNSPEC,
			.ai_socktype = SOCK_STREAM,
			.ai_flags = AI_ADDRCONFIG
#ifndef __UCLIBC__
				| AI_NUMERICSERV
#endif
		};

// (older) uclibc versions do not handle changes in resolv.conf correctly
#ifdef __UCLIBC__
#include <features.h>
#if __UCLIBC_MAJOR__ == 0 && __UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ <= 29
#include <resolv.h>
		res_init();
#endif
#endif

		if (getaddrinfo(hostname, "80", &hints, &result))
			return -1;

		for (rp = result; rp != NULL; rp = rp->ai_next) {
			struct sockaddr_in *in = (void*)rp->ai_addr;
			struct sockaddr_in6 *in6 = (void*)rp->ai_addr;
			if (rp->ai_family == AF_INET6)
				whitelist_table_add(AF_INET6, &in6->sin6_addr, 128, INT32_MAX);
			else if (rp->ai_family == AF_INET)
				whitelist_table_add(AF_INET, &in->sin_addr, 32, INT32_MAX);
		}

		freeaddrinfo(result);
	} else {
		struct whitelist_host *host = whitelist_get(hostname, hostlen);
		if (type == WL_NONE && host) { // Delete whitelist entry
			list_del(&host->_head);
			free(host);
		} else { // Add / replace whitelist entry
			if (!host) {
				if (!(host = malloc(sizeof(*host) + hostlen)))
					return -1;
				memcpy(host->name, hostname, hostlen);
				host->len = hostlen;
				list_add(&host->_head, &hosts);
			}
			host->type = type;
		}
	}
	return 0;
}

static void whitelist_flush() {
	while (!list_empty(&hosts)) {
		struct whitelist_host *h =
			list_first_entry(&hosts, struct whitelist_host, _head);
		list_del(&h->_head);
		free(h);
	}
}

// Config parsing callback
static void whitelist_parse_config(const char *host, void *ctx) {
	enum whitelist_type *ptype = ctx;
	enum whitelist_type type = *ptype;

	if (type == WL_DOMAIN) {
		if (host[0] == '!') {
			host = &host[1];
			type = WL_EXCLUDE;
		} else if (host[0] == '*' && host[1] == '.') {
			host = &host[2];
			type = WL_ZONE;
		} else {
			type = WL_HOST;
		}
	}
	whitelist_set(host, type);
}


// Config parsing DNS servers
static void whitelist_parse_dns(const char *host, void *ctx) {
	bool *enable = ctx;
	uint8_t buf[16];
	int family = strchr(host, ':') ? AF_INET6 : AF_INET;
	inet_pton(family, host, buf);
	firewall_set_nfqueue_src(family, buf, IPPROTO_UDP, 53,
			cfg.nfqueueid, *enable, use_nfqueue);
}

#ifdef __SC_BUILD__
static void scToLows(char *charStr)
{
    int i,len = strlen((char*)charStr);
    for(i=0;i<len;i++){
        if(charStr[i]>='A' && charStr[i]<='Z')
            charStr[i]+= 'a'-'A';
    }
}
#endif

// Analyze a DNS packet and evaluate its contents
static int whitelist_dns_process(struct dnspkt *dns, ssize_t len) {
	uint16_t ac; // answer count
	if (len < sizeof(*dns) || ntohs(dns->header[DNS_QDCOUNT]) != 1
	|| (ac = ntohs(dns->header[DNS_ANCOUNT])) < 1)
		return -1;

	uint8_t *data = (uint8_t*)dns;
	size_t off = offsetof(struct dnspkt, payload);

	char buf[255]; // read queried domain
	int size = dns_getname(data, &off, len, buf);
	if (size < 0 || len <= (off + 4))
		return -1;

	uint16_t qtype = dns_get16(data, &off);
	if (qtype != DNS_T_A && qtype != DNS_T_AAAA)
		return -1;

#ifdef __SC_BUILD__
    scToLows(buf);
#endif
	if (!whitelist_table_matches(buf, size))
		return -1;

	off += 2; /* Skip qclass, offset is now at first answer */
	while (ac-- > 0) { /* Iterate answers */
		if ((size = dns_getname(data, &off, len, buf)) < 0 || len <= (off + 10))
			break;

		qtype = dns_get16(data, &off);
		off += 6;
		uint16_t qlen = dns_get16(data, &off);

		if (qtype == DNS_T_A && qlen == 4 && len >= off + 4) {
			whitelist_table_add(AF_INET, &data[off], 32, cfg.timeout);
		} else if (qtype == DNS_T_AAAA && qlen == 16 && len >= off + 16) {
			whitelist_table_add(AF_INET6, &data[off], 128, cfg.timeout);
		}
		off += qlen;
	}

	return 0;
}

// get whitelist entry for domain
static struct whitelist_host* whitelist_get(const char *hostname,
							uint16_t len) {
	struct whitelist_host *c;
	list_for_each_entry(c, &hosts, _head)
		if (c->len == len && !memcmp(hostname, c->name, len))
			return c;
	return NULL;
}

// check whether a given domain is whitelisted
static int whitelist_table_matches(const char *name, int len) {
	for (const char *c = name; c && (c == name || *c++); c = strchr(c, '.')) {
		struct whitelist_host *host = whitelist_get(c, len - (c - name));
		if (!host)
			continue;
		else if (host->type == WL_EXCLUDE)
			break;
		else if (host->type == WL_ZONE || (host->type == WL_HOST && c == name))
			return 1;
	}
	return 0;
}

// add an IP address to the whitelist
static int whitelist_table_add(int af, const void *addr, uint8_t prefix,
							int32_t timeout) {
	const struct in6_addr *addrp = addr;
	struct in6_addr addrm = IN6ADDR_ANY_INIT;
	if (af == AF_INET) {
		addrm.s6_addr16[5] = 0xffff;
		addrm.s6_addr32[3] = *((uint32_t*)addr);
		addrp = &addrm;
	}
	// We store addresses as IPv6 internally

	struct whitelist_route *r;
	for (r = routes; r; r = r->next)
		if (!memcmp(&r->addr, addrp, sizeof(*addrp)) && r->prefix == prefix)
			break;

	if (!r) { /* No route yet, create on */
		if (!(r = malloc(sizeof(*r))))
			return -1;

		r->addr = *addrp;
		r->next = routes;
		r->prefix = prefix;
		r->timeout = 0;
		routes = r;

		if (IN6_IS_ADDR_V4MAPPED(&r->addr)) { /* IPv4 */
			firewall_set_whitelist(AF_INET, &addrp->s6_addr32[3], prefix, 1);
		} else { /* IPv6 */
			firewall_set_whitelist(AF_INET6, addrp, prefix, 1);
		}
	}

	// set the new route timeout
	timeout = (timeout == INT32_MAX) ? INT32_MAX : (event_time() / 1000 + timeout);
	if (r->timeout < timeout)
		r->timeout = timeout;

	return 0;
}

static void whitelist_table_clean(struct event_timer *timer, int64_t now) {
	/* If timer is NULL, do the final cleanup */
	int32_t tnow = (timer) ? now / 1000 : INT32_MAX;
	struct whitelist_route *r = routes, **o = &routes;
	int collected = 0;

	while (r) {
		if (r->timeout <= tnow) {
			if (IN6_IS_ADDR_V4MAPPED(&r->addr)) { /* IPv4 */
				firewall_set_whitelist(AF_INET,
				  &r->addr.s6_addr32[3], r->prefix, false);
			} else { /* IPv6 */
				firewall_set_whitelist(AF_INET6, &r->addr,
							r->prefix, false);
			}
			*o = r->next;
			free(r);
			collected++;
		} else {
			o = &r->next;
		}
		r = *o;
	}

	if (collected)
    {
		syslog(LOG_INFO, "Whitelist GC collected %i items", collected);
    }
}

#ifdef __SC_BUILD__
static unsigned int convert_ip_to_unsignedint(const char *value)
{
    struct in_addr ip_addr;
    unsigned int *buf;
    inet_pton(AF_INET,value,(void *)&ip_addr);
    buf = (unsigned int *)&ip_addr;
    return *buf;
}
static int _check_ip_is_in_same_subnet(const char *ip_addr1, const char *ip_addr2, const char *submask)
{
    int ret = 0;
    unsigned int ip1 = convert_ip_to_unsignedint(ip_addr1);
    unsigned int ip2 = convert_ip_to_unsignedint(ip_addr2);
    unsigned int mask = convert_ip_to_unsignedint(submask);
    if((ip1 & mask) == (ip2 & mask))
        ret = 1;
    return ret;
}
#endif
static void whitelist_nfqueue_process(struct event_epoll *event,
		uint32_t revents) {
	uint8_t buffer[32];
	struct nlmsghdr *nh_verdict = genlmsg_init(buffer,
			(NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_VERDICT, 0, 0, 0);
	struct nfgenmsg *ng = NLMSG_DATA(nh_verdict);
#ifdef __SC_BUILD__
    const char *ip = NULL;
    const char *mask = NULL;
    ip = config_get_string("main", "lan_ip", "192.168.0.1");
    mask = config_get_string("main", "lan_mask", "255.255.255.0");
#endif
	ng->version = 0;
	ng->nfgen_family = AF_UNSPEC;
	ng->res_id = htons(cfg.nfqueueid);

	struct nfqnl_msg_verdict_hdr *verdict = nlmsg_claim_attr(nh_verdict,
			NFQA_VERDICT_HDR, sizeof(*verdict));
	verdict->verdict = htonl(NF_ACCEPT);

	for (;;) {
		uint8_t *buffer = NULL;
		ssize_t len = 0;

		struct nlmsghdr *nh = unl_receive(unl_nfqueue);
		if (!nh && errno == EWOULDBLOCK) {
			break;
		} else if (!nh) {
			continue;
		}

		if (use_nfqueue) {
			struct nlattr *nla_hdr = genlmsg_find(nh, 0, NFQA_PACKET_HDR);
			struct nlattr *nla_pal = genlmsg_find(nh, 0, NFQA_PAYLOAD);

			if (!nla_pal || !nla_hdr)
            {
#ifdef __SC_BUILD__
                unl_buffer_flush(unl_nfqueue);
#endif
				continue;
            }
			struct nfqnl_msg_packet_hdr *hdr = NLA_DATA(nla_hdr);
			verdict->id = hdr->packet_id;
			buffer = NLA_DATA(nla_pal);
			len = NLA_PAYLOAD(nla_pal);
		} else {
			struct nlattr *nla_pal = genlmsg_find(nh, 0, NFULA_PAYLOAD);
			if (!nla_pal)
				continue;

			buffer = NLA_DATA(nla_pal);
			len = NLA_PAYLOAD(nla_pal);
		}

		if (len <= 0)
			continue;

		uint8_t ipver = buffer[0] >> 4; // Hack to get IP-Version
		if (ipver == 4 && len >= 28) {
			const struct iphdr *iph = (struct iphdr*)buffer;
#ifdef __SC_BUILD__
            struct in_addr addr;
            addr.s_addr = iph->daddr;
            if(_check_ip_is_in_same_subnet(inet_ntoa(addr), ip, mask))
            {
                goto accept;
            }
#endif
			uint8_t *dnspkt = buffer + (4 * iph->ihl + 8); // Skip IP/UDP header
			len -= (dnspkt - buffer);
			whitelist_dns_process((struct dnspkt*)dnspkt, len);
		} else if (ipver == 6 && len >= 48) {
			uint8_t *dnspkt = buffer + 48; // Skip IPv6/UDP header
			len -= (dnspkt - buffer);
			whitelist_dns_process((struct dnspkt*)dnspkt, len);
		}

#ifdef __SC_BUILD__
accept:
#endif
		// Accept packet after we processed it
		if (use_nfqueue)
			unl_request(unl_nfqueue, nh_verdict);
	}
}

static int whitelist_nfqueue(void) {
	char buffer[128] = {0};
	int subsys = (use_nfqueue) ?
			((NFNL_SUBSYS_QUEUE << 8) | NFQNL_MSG_CONFIG):
			((NFNL_SUBSYS_ULOG << 8) | NFULNL_MSG_CONFIG);

	struct nlmsghdr *nh = genlmsg_init(buffer, subsys, 0, 0, 0);
	struct nfgenmsg *ng = NLMSG_DATA(nh);
	ng->nfgen_family = AF_UNSPEC;
	ng->res_id = 0;

	if (use_nfqueue) {
		static bool first = true;
		struct nfqnl_msg_config_cmd *cmd = nlmsg_claim_attr(
			 nh, NFQA_CFG_CMD, sizeof(*cmd));

		cmd->command = NFQNL_CFG_CMD_PF_BIND;
		// Register AF_INET log handler
		cmd->pf = htons(AF_INET);
		if (unl_call(unl_nfqueue, nh))
			if (first)
            {
#ifdef __SC_BUILD__
            log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "whitelist: unable to register "
					"nfqueue: conflicting modules?");
#else
			syslog(LOG_WARNING, "whitelist: unable to register "
					"nfqueue: conflicting modules?");
#endif
            }
		first = false;

		// Register AF_INET6 log handler
		cmd->pf = htons(AF_INET6);
		unl_call(unl_nfqueue, nh);

		// Register queue
		cmd->command = NFQNL_CFG_CMD_BIND;
		cmd->pf = 0;
		ng->res_id = htons(cfg.nfqueueid);
		if (unl_call(unl_nfqueue, nh))
			return -1;

		nh = genlmsg_init(buffer, subsys, 0, 0, 0);
		ng = NLMSG_DATA(nh);
		ng->nfgen_family = AF_UNSPEC;
		ng->res_id = htons(cfg.nfqueueid);

		struct nfqnl_msg_config_params params = {
			.copy_range = htonl(2560),
			.copy_mode = NFQNL_COPY_PACKET,
		};

		nlmsg_put(nh, NFQA_CFG_PARAMS, &params, sizeof(params));
		return unl_call(unl_nfqueue, nh);
	} else {
		// Register AF_INET log handler
		ng->nfgen_family = AF_INET;
		nlmsg_put_u8(nh, NFULA_CFG_CMD, NFULNL_CFG_CMD_PF_BIND);
		unl_call(unl_nfqueue, nh);

		// Register AF_INET6 log handler
		ng->nfgen_family = AF_INET6;
		unl_call(unl_nfqueue, nh);

		nh = genlmsg_init(buffer,
		(NFNL_SUBSYS_ULOG << 8) | NFULNL_MSG_CONFIG, 0, 0, 0);
		ng->res_id = htons(cfg.nfqueueid);
		ng->nfgen_family = 0;

		nlmsg_put_u8(nh, NFULA_CFG_CMD, NFULNL_CFG_CMD_BIND);
		struct nfulnl_msg_config_mode mode = {
			.copy_range = htonl(1500),
			.copy_mode = NFULNL_COPY_PACKET,
		};
		nlmsg_put(nh, NFULA_CFG_MODE, &mode, sizeof(mode));
		nlmsg_put_u32(nh, NFULA_CFG_TIMEOUT, 0);
		return unl_call(unl_nfqueue, nh);
	}
}

MODULE_REGISTER(whitelist, 480)

#ifdef HOTSPOTD_RPC
#include "ext_rpc/rpc.h"
#include "rpc/012-whitelist.h"

static int whitelist_rpc_getactive(struct rpc_handle *hndl, struct frmsg *frr) {
	if (!(frm_flags(frr) & FRM_F_DUMP))
		return -EINVAL;

	struct frmsg *frm;
	uint8_t buffer[8192];
	size_t len = 0;
	int32_t now = event_time() / 1000;

	struct whitelist_route *r;
	for (r = routes; r; r = r->next) {
		// Build message for 1 entry
		frm = frm_init(buffer + len, FRT_WL_GETACTIVE, FRM_F_MULTI);
		frm->frm_seq = frr->frm_seq;

		if (IN6_IS_ADDR_V4MAPPED(&r->addr)) {
			frm_put_buffer(frm, FRA_WL_IPV4, &r->addr.s6_addr32[3], 4);
		} else {
			frm_put_buffer(frm, FRA_WL_IPV6, &r->addr, 16);
		}
		frm_put_u32(frm, FRA_WL_EXPIRES, r->timeout - now);
		frm_put_u32(frm, FRA_WL_PREFIX, r->prefix);

		len += frm_align(frm_length(frm));
		if (sizeof(buffer) - len < 64) {
			rpc_send(hndl, buffer, len);
			len = 0;
		}
	}

	frm = frm_init(buffer + len, FRMSG_DONE, 0);
	frm->frm_seq = frr->frm_seq;
	rpc_send(hndl, buffer, len + frm_length(frm));
	return 1;
}

static struct rpc_handler _rpcs[] = {
	{FRT_WL_GETACTIVE, whitelist_rpc_getactive},
	{0},
};

RPC_REGISTER(_rpcs)
#endif
