/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "core.h"

struct nlmsghdr* genlmsg_init
(void *buf, uint16_t family, uint16_t flags, uint8_t cmd, uint8_t version) {
 	struct nlmsghdr *nh = nlmsg_init(buf, family, flags);
 	struct genlmsghdr *gmh = nlmsg_claim(nh, sizeof(*gmh));
 	gmh->cmd = cmd;
 	gmh->version = version;
 	return nh;
}

static struct nlmsghdr* get_family
(struct unl *hndl, const char *family) {
	if (strlen(family) >= GENL_NAMSIZ){
		errno = EINVAL;
		return NULL;
	}
	uint8_t buffer[64] = {0};
	struct nlmsghdr *nh =
		genlmsg_init(buffer, GENL_ID_CTRL, 0, CTRL_CMD_GETFAMILY, 0);
	nlmsg_put_string(nh, CTRL_ATTR_FAMILY_NAME, family);
	return (unl_request(hndl, nh) < 0) ? NULL : unl_receive_response(hndl);
}

uint16_t unl_genl_resolve(struct unl *hndl, const char *family) {
	struct nlmsghdr *nh = get_family(hndl, family);
	if (!nh) return -1;
	return nla_get_u16(genlmsg_find(nh, 0, CTRL_ATTR_FAMILY_ID), 0);
}


#ifdef CTRL_ATTR_MCAST_GRP_MAX
static int subscribe
(struct unl *hndl, const char *family, const char *group, int subscribe) {
	struct nlmsghdr *nh = get_family(hndl, family);
	if (!nh) return -1;
	struct nlattr *grps = genlmsg_find(nh, 0, CTRL_ATTR_MCAST_GROUPS);
	if (!grps) return -1;

	uint32_t id = 0;
	struct nlattr *grp;
	nla_foreach_attr(grp, grps) {
		struct nlattr *nla = nla_find(grp, CTRL_ATTR_MCAST_GRP_NAME);
		if (!strcmp(group, nla_get_string(nla, ""))
		&& (nla = nla_find(grp, CTRL_ATTR_MCAST_GRP_ID))) {
			id = nla_get_u32(nla, 0);
			break;
		}
	}
	if (!id) {
		errno = ENOENT;
		return -1;
	}
	return (subscribe) ? unl_subscribe(hndl, id) : unl_unsubscribe(hndl, id);
}

int unl_genl_subscribe(struct unl *hndl, const char *family, const char *group) {
	return subscribe(hndl, family, group, 1);
}

int unl_genl_unsubscribe(struct unl *hndl, const char *family, const char *group) {
	return subscribe(hndl, family, group, 0);
}
#endif
