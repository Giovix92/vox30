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
#include <stddef.h>
#include <string.h>

#include "core.h"

/* Message functions */

struct nlmsghdr* nlmsg_init(void *buf, uint16_t type, uint16_t flags) {
	struct nlmsghdr *nh = buf;
	nh->nlmsg_seq = nh->nlmsg_pid = 0;
	nh->nlmsg_len = sizeof(*nh);
	nh->nlmsg_type = type;
	nh->nlmsg_flags = flags;
	return nh;
}

void* nlmsg_claim(struct nlmsghdr *nh, uint32_t datalen) {
	void *data = ((uint8_t*)nh) + NLMSG_ALIGN(nh->nlmsg_len);
	nh->nlmsg_len = NLMSG_ALIGN(nh->nlmsg_len) + datalen;
	return data;
}

void* nlmsg_claim_attr(struct nlmsghdr *nh, uint16_t type, uint32_t datalen) {
	struct nlattr *nla = nlmsg_claim(nh, NLA_HDRLEN + datalen);
	nla->nla_len = NLA_HDRLEN + datalen;
	nla->nla_type = type;
	return ((uint8_t*)nla) + NLA_HDRLEN;
}

struct nlattr* nla_init(void *buf, uint16_t type) {
	struct nlattr *nla = buf;
	nla->nla_len = sizeof(*nla);
	nla->nla_type = type;
	return nla;
}

void* nla_claim(struct nlattr *nla, size_t datalen) {
	void *data = ((uint8_t*)nla) + NLMSG_ALIGN(nla->nla_len);
	nla->nla_len = NLMSG_ALIGN(nla->nla_len) + datalen;
	return data;
}

void* nla_claim_attr(struct nlattr *nla, uint16_t type, size_t datalen) {
	nla = (struct nlattr*)nla_claim(nla, NLA_HDRLEN + datalen);
	nla->nla_type = type;
	nla->nla_len = NLA_HDRLEN + datalen;
	return ((uint8_t*)nla) + NLA_HDRLEN;
}
