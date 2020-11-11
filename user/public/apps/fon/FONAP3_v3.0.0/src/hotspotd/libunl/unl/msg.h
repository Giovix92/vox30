/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#ifndef UNL_MSG_H_
#define UNL_MSG_H_

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>


/* Compiler magic */
#define UNL_FORCE_ALIGNMENT(n) __attribute__((packed,aligned(n)))
#define UNL_ALIGNED UNL_FORCE_ALIGNMENT(NLMSG_ALIGNTO)


/****** NETLINK message construction ******/

/**
 * Initializes a buffer for use as a netlink message and sets type and flags.
 *
 * buf:		your buffer (must be at least sizeof(struct nlmsghdr))
 * type:	netlink message type
 * flags:	netlink message flags
 *
 * Returns a pointer to buf as a nlmsghdr.
 */
struct nlmsghdr* nlmsg_init(void *buf, uint16_t type, uint16_t flags);

/**
 * Same as nlmsg_init but initializes a netlink attribute
 */
struct nlattr* nla_init(void *buf, uint16_t type);


/**
 * Claims an aligned amount of data from the message buffer
 *
 * buf:		your buffer
 * len:		length of your buffer
 * datalen:	data length
 *
 * Returns a pointer to the claimed memory
 */
void* nlmsg_claim(struct nlmsghdr *nh, uint32_t datalen);

/**
 * Same as nlmsg_claim but claims from a netlink buffer
 */
void* nla_claim(struct nlattr *nla, size_t datalen);


/**
 * Same as nlmsg_claim but inserts a netlink attribute header before the pointer
 * type: netlink attribute type
 */
void* nlmsg_claim_attr(struct nlmsghdr *nh, uint16_t type, uint32_t datalen);

/**
 * Same as nlmsg_claim but claims from a netlink buffer
 */
void* nla_claim_attr(struct nlattr *nla, uint16_t type, size_t datalen);



/**
 * Starts a new attribute at the the end of the message
 *
 * Use nla_* functions on the returned attribute to append data to it
 *
 * NOTE: Any call appending to the same messages before a call to
 * nlmsg_commit_attr overwrites the new attribute
 */
static inline struct nlattr* nlmsg_start_attr(struct nlmsghdr *nh, uint16_t type) {
	return nla_init(((uint8_t*)nh) + NLMSG_ALIGN(nh->nlmsg_len), type);
}

/**
 * Same as nlmsg_start_attr but starts inside an attribute
 */
static inline struct nlattr* nla_start_attr(struct nlattr *outer, uint16_t type) {
	return nla_init(((uint8_t*)outer) + NLMSG_ALIGN(outer->nla_len), type);
}


/**
 * Finishes an attribute started with nlmsg_start_attr and commits it to the message
 */
static inline void nlmsg_commit_attr(struct nlmsghdr *nh, struct nlattr *nla) {
	nlmsg_claim(nh, nla->nla_len);
}

/**
 * Finishes an attribute started with nla_start_attr and commits it to the outer attribute
 */
static inline void nla_commit_attr(struct nlattr *outer, struct nlattr *nla) {
	nla_claim(outer, nla->nla_len);
}


/**
 * This is basically nlmsg_claim with a memcpy afterwards
 */
static inline void* nlmsg_append(struct nlmsghdr *nh, void *data, size_t len) {
	return memcpy(nlmsg_claim(nh, len), data, len);
}

/**
 * This is basically nla_claim with a memcpy afterwards
 */
static inline void* nla_append(struct nlattr *nla, void *data, size_t len) {
	return memcpy(nla_claim(nla, len), data, len);
}


/**
 * These functions append an attribute of type "type" to the current message
 */

static inline void* nlmsg_put(struct nlmsghdr *nh, uint16_t type, const void *data, size_t len) {
	return memcpy(nlmsg_claim_attr(nh, type, len), data, len);
}

static inline void* nlmsg_put_flag(struct nlmsghdr *nh, uint16_t type) {
	return nlmsg_claim_attr(nh, type, 0);
}

static inline void* nlmsg_put_string(struct nlmsghdr *nh, uint16_t type, const char *data) {
	return nlmsg_put(nh, type, data, strlen(data) + 1);
}

#define nlmsg_put_type(nh, type, datatype, value) (*((datatype*)nlmsg_claim_attr(nh, type, sizeof(datatype))) = value)
#define nlmsg_put_u8(nh, type, value) nlmsg_put_type(nh, type, uint8_t, value)
#define nlmsg_put_u16(nh, type, value) nlmsg_put_type(nh, type, uint16_t, value)
#define nlmsg_put_u32(nh, type, value) nlmsg_put_type(nh, type, uint32_t, value)
#define nlmsg_put_u64(nh, type, value) nlmsg_put_type(nh, type, uint64_t, value)


/**
 * These functions append an attribute of type "type" to the current attribute
 */

static inline void* nla_put(struct nlattr *nla, uint16_t type, const void *data, size_t len) {
	return memcpy(nla_claim_attr(nla, type, len), data, len);
}

static inline void* nla_put_flag(struct nlattr *nla, uint16_t type) {
	return nla_claim_attr(nla, type, 0);
}

static inline void* nla_put_string(struct nlattr *nla, uint16_t type, const char *data) {
	return nla_put(nla, type, data, strlen(data) + 1);
}

#define nla_put_type(nh, type, datatype, value) (*((datatype*)nla_claim_attr(nh, type, sizeof(datatype))) = value)
#define nla_put_u8(nh, type, value) nla_put_type(nh, type, uint8_t, value)
#define nla_put_u16(nh, type, value) nla_put_type(nh, type, uint16_t, value)
#define nla_put_u32(nh, type, value) nla_put_type(nh, type, uint32_t, value)
#define nla_put_u64(nh, type, value) nla_put_type(nh, type, uint64_t, value)


#endif /* UNL_MSG_H_ */
