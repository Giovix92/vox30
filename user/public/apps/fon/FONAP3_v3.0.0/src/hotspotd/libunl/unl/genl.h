/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#ifndef UNL_GENL_H_
#define UNL_GENL_H_

#include <linux/genetlink.h>
#include "io.h"
#include "parse.h"


/****** Generic netlink helpers *******/

/**
 * Drop in replacement for nlmsg_init for generic netlink.
 *
 * buf:		your buffer
 * family:	generic netlink family id
 * flags:	netlink message flags
 * cmd:		generic netlink commands
 * version:	generic netlink command version
 */
struct nlmsghdr* genlmsg_init
(void *buf, uint16_t family, uint16_t flags, uint8_t cmd, uint8_t version);

/**
 * Drop in replacement for nlmsg_find for generic netlink.
 * Automatically skips the generic netlink header.
 */
static inline struct nlattr* genlmsg_find(const struct nlmsghdr *nh, size_t skip, uint16_t type) {
	return nlmsg_find(nh, NLMSG_ALIGN(sizeof(struct genlmsghdr) + skip), type);
}

/**
 * Drop in replacement for nlmsg_parse for generic netlink.
 * Automatically skips the generic netlink header.
 */
static inline int genlmsg_parse(const struct nlmsghdr *nh, size_t skip,
struct nlattr *nlatbl[], uint16_t maxtype, struct nla_policy *pol) {
	return nlmsg_parse(nh, NLMSG_ALIGN(sizeof(struct genlmsghdr) + skip),
		nlatbl, maxtype, pol);
}

/**
 * Same as nlmsg_foreach_attr but skips generic netlink header automatically
 */
#define genlmsg_foreach_attr(nla, nh, skip) \
	nlmsg_foreach_attr(nla, nh, NLMSG_ALIGN(sizeof(struct genlmsghdr) + skip))


/*
 * Resolve the family id of given netlink family
 *
 * returns the generic netlink family id for given name or 0 on error
 */
uint16_t unl_genl_resolve(struct unl *hndl, const char *family);

/*
 * (Un)Subscribes a multicast group of a generic netlink socket family
 *
 * returns 0 on success, -1 on errror
 */
#ifdef CTRL_ATTR_MCAST_GRP_MAX
int unl_genl_subscribe(struct unl *hndl, const char *family, const char *group);
int unl_genl_unsubscribe(struct unl *hndl, const char *family, const char *group);
#endif

#endif /* UNL_GENL_H_ */
