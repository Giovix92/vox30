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
#include <errno.h>

#include "core.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static uint16_t types_min[] = {
	[NLA_UNSPEC] = 0,
	[NLA_U8] = 1,
	[NLA_U16] = 2,
	[NLA_U32] = 4,
	[NLA_U64] = 8,
	[NLA_STRING] = 1,
};

int unl_validate(const void *buf, size_t len,
const struct nla_policy pol[], uint16_t maxtype) {
	struct nlattr *nla;
	unl_foreach_attr(nla, buf, len) {
		uint16_t type = nla_type(nla);
		if (type > maxtype) continue;
		const struct nla_policy *p = &pol[type];
		uint16_t maxlen = (p->maxlen) ? p->maxlen : UINT16_MAX;
		uint16_t minlen = (p->minlen) ? p->minlen :
			(p->type < ARRAY_SIZE(types_min)) ? types_min[p->type] : 0;
		if (nla->nla_len > maxlen || nla->nla_len < minlen) return -1;
		if (p->type == NLA_STRING
		&& ((char*)NLA_DATA(nla))[NLA_PAYLOAD(nla) - 1] != 0) return -1;
	}
	return 0;
}

struct nlattr* unl_find(const void *buf, size_t len, uint16_t type) {
	struct nlattr *nla;
	unl_foreach_attr(nla, buf, len)
		if (nla_type(nla) == type)
			return nla;
	return NULL;
}

int unl_parse(const void *buf, size_t len, struct nlattr *nlatbl[],
uint16_t maxtype, const struct nla_policy *pol) {
	memset(nlatbl, 0, (maxtype + 1) * sizeof(*nlatbl));
	if (pol && unl_validate(buf, len, pol, maxtype)) return -1;
	struct nlattr *nla;
	unl_foreach_attr(nla, buf, len) {
		uint16_t type = nla_type(nla);
		if (type <= maxtype)
			nlatbl[type] = nla;
	}
	return 0;
}

int nlmsg_errno(const struct nlmsghdr *nh) {
	return (!nh) ? -1 : (nh->nlmsg_type != NLMSG_ERROR) ? 0 :
	(NLMSG_PAYLOAD(nh, 0) < sizeof(struct nlmsgerr)) ? -1 :
	-((struct nlmsgerr*)NLMSG_DATA(nh))->error;
}

int nlmsg_success(const struct nlmsghdr *nh) {
	int err = nlmsg_errno(nh);
	if (err > 0) errno = err;
	return -(!!err);
}
