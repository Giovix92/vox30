/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#ifndef UNL_PARSE_H_
#define UNL_PARSE_H_

#include <stdint.h>
#include <linux/netlink.h>

#ifndef NLA_TYPE_MASK
#define NLA_TYPE_MASK (~((1 << 15)|(1 << 14)))
#endif

enum {
	NLA_UNSPEC,
	NLA_U8,
	NLA_U16,
	NLA_U32,
	NLA_U64,
	NLA_STRING,
	NLA_FLAG,
	NLA_NESTED,
	NLA_AFTER_TYPE
};

struct nla_policy {
	uint16_t type;
	uint16_t minlen;
	uint16_t maxlen;
};

/* Basic NLA_ macros inspired by rtnetlink RTA_ macros from linux kernel headers */
#define NLA_DATA(nla)   ((void*)(((uint8_t*)(nla)) + NLA_HDRLEN))
#define NLA_PAYLOAD(nla) ((ssize_t)((nla)->nla_len) - NLA_HDRLEN)
#define NLA_OK(nla,len) ((len) >= (ssize_t)sizeof(struct nlattr) && \
                         (nla)->nla_len >= sizeof(struct nlattr) && \
                         (nla)->nla_len <= (len))
#define NLA_NEXT(nla,attrlen)   ((attrlen) -= NLA_ALIGN((nla)->nla_len), \
                                 (struct nlattr*)(((uint8_t*)(nla)) + NLA_ALIGN((nla)->nla_len)))

static inline int nla_type(const struct nlattr *nla) {
	return nla->nla_type & NLA_TYPE_MASK;
}


/****** NETLINK loop helpers ******/

/**
 * Iterates over each valid netlink message in buffer, using nh as entry pointer
 * Use NLMSG_DATA(nh) to get a pointer to the payload data
 * Use NLMSG_PAYLOAD(nh, 0) to get the size of the payload data
 *
 * nh:		entry pointer pointing to current struct nlmsghdr
 * buffer:	the message buffer
 * len:		the message buffer length
 */
#define unl_foreach_msg(nh, buffer, len) \
	(nh) = (struct nlmsghdr*)(buffer); \
	for (ssize_t _blen = (len); NLMSG_OK((nh), _blen); (nh) = NLMSG_NEXT((nh), _blen))

/**
 * Iterates over each valid netlink attribute using nla as entry pointer
 * Use NLA_DATA(nla) to get a pointer to the payload data
 * Use NLA_PAYLOAD(nla) to get the size of the payload data
 *
 * nla:		entry pointer pointing to current struct nlmsghdr
 * buffer:	buffer containing attributes
 * len:		buffer length
 */
#define unl_foreach_attr(nla, buffer, len) \
	(nla) = (struct nlattr*)(buffer); \
	for (ssize_t _rlen = (len); NLA_OK((nla), _rlen); (nla) = NLA_NEXT((nla), _rlen))


/**
 * Iterates over each valid netlink attribute in a netlink message, using nla as entry pointer
 * Use NLA_DATA(nla) to get a pointer to the payload data
 * Use NLA_PAYLOAD(nla) to get the size of the payload data
 *
 * nla:		entry pointer pointing to current struct nlmsghdr
 * nh:		netlink message
 * skip:	the length of the fixed message part - if any - to skip
 */
#define nlmsg_foreach_attr(nla, nh, skip) \
	unl_foreach_attr(nla, ((uint8_t*)NLMSG_DATA((nh))) + NLMSG_ALIGN((skip)), NLMSG_PAYLOAD((nh), (skip)))


/**
 * Same as unl_foreach_attr but more convenient for nested attributes
 *
 * nla:		entry pointer pointing to current struct nlmsghdr
 * outer:	outer netlink nlattribute
 */
#define nla_foreach_attr(nla, outer) \
	unl_foreach_attr(nla, NLA_DATA((outer)), (outer) ? NLA_PAYLOAD((outer)) : 0)


/**
 * Checks whether given netlink attribute's payload has given length and if so
 * returns its value after casting it it to a given type otherwise returns a
 * default value.
 *
 * nla:		netlink attribute
 * type:	type to cast to
 * def:		default value if attribute is invalid
 */
#define NLA_CAST_NUMBER(nla, type, def) (((nla) && NLA_PAYLOAD(nla) == sizeof(type)) \
	? *((type*)NLA_DATA(nla)) : (def))

/**
 * Same as NLA_CAST_NUMBER, but verifies that the attribute
 * carries a zero terminated string.
 */
#define NLA_CAST_STRING(nla, def) (((nla) && ((char*)NLA_DATA(nla))[NLA_PAYLOAD(nla) - 1] == 0) \
	? (char*)NLA_DATA(nla) : (def))

/**
 * Same as NLA_CAST_NUMBER but only checks whether the attribute has
 * given length.
 */
#define NLA_CAST_FIXED(nla, len, def) (((nla) && NLA_PAYLOAD(nla) == len) ? NLA_DATA(nla) : (def))



/* Some more convenience functions */
static inline uint8_t nla_get_u8(struct nlattr *nla, uint8_t def) {
	return NLA_CAST_NUMBER(nla, uint8_t, def);
}

static inline uint16_t nla_get_u16(struct nlattr *nla, uint16_t def) {
	return NLA_CAST_NUMBER(nla, uint16_t, def);
}

static inline uint32_t nla_get_u32(struct nlattr *nla, uint32_t def) {
	return NLA_CAST_NUMBER(nla, uint32_t, def);
}

static inline uint64_t nla_get_u64(struct nlattr *nla, uint64_t def) {
	return NLA_CAST_NUMBER(nla, uint64_t, def);
}

static inline char* nla_get_string(struct nlattr *nla, char *def) {
	return NLA_CAST_STRING(nla, def);
}

static inline int nla_get_flag(struct nlattr *nla) {
	return (nla) ? 1 : 0;
}


/****** NETLINK parser ******/

/**
 * Test whether more messages can be expected.
 */
static inline int nlmsg_complete(const struct nlmsghdr *nh) {
	return nh && (!(nh->nlmsg_flags & NLM_F_MULTI) || nh->nlmsg_type == NLMSG_DONE);
}

/**
 * Return an error code associated with given message.
 */
int nlmsg_errno(const struct nlmsghdr *nh);

/**
 * Check whether the message is a non-error message
 * and set errno if not.
 */
int nlmsg_success(const struct nlmsghdr *nh);


/**
 * Searches a buffer for a netlink attribute with given type
 *
 * buf:		netlink message
 * len:		the length of the buffer
 * type:	attribute type
 *
 */
struct nlattr* unl_find(const void *buf, size_t len, uint16_t type);

/**
 * Searches a buffer for all netlink attribute upto a given type
 * and put them into an array if they are valid.
 *
 * buf:		buffer
 * len:		the length of the buffer
 * nlatbl:	an array where pointers to the attributes
 * maxtype:	the highest attribute type id to parse
 * pol:		Policy array to validate attributes (optional)
 *
 * NOTE: nlatbl must have space for at least max + 1 entries
 * The same applies to pol if it is not NULL
 *
 * Retruns 0 on success
 */
int unl_parse(const void *buf, size_t len, struct nlattr *nlatbl[],
		uint16_t maxtype, const struct nla_policy *pol);

/**
 * Validates a buffer of netlink attributes using a given policy
 *
 * buf:		buffer
 * len:		the length of the buffer
 * pol:		Policy array to validate attributes (optional)
 * maxtype:	the highest attribute type id to parse
 *
 * Returns 0 on success, -1 otherwise
 */
int unl_validate(const void *buf, size_t len,
const struct nla_policy pol[], uint16_t maxtype);


/**
 * Same as unl_find but more convenient for netlink msgs containing attributes
 */
static inline struct nlattr* nlmsg_find(const struct nlmsghdr *nh,
size_t skip, uint16_t type) {
	return unl_find(((uint8_t*)NLMSG_DATA(nh)) + NLMSG_ALIGN(skip),
		NLMSG_PAYLOAD(nh, skip), type);
}

/**
 * Same as unl_parse but more convenient for netlink msgs containing attributes
 */
static inline int nlmsg_parse(const struct nlmsghdr *nh, size_t skip,
struct nlattr *nlatbl[], uint16_t maxtype, const struct nla_policy *pol) {
	return unl_parse(((uint8_t*)NLMSG_DATA(nh)) + NLMSG_ALIGN(skip),
		NLMSG_PAYLOAD(nh, skip), nlatbl, maxtype, pol);
}

/**
 * Same as unl_validate but more convenient for netlink msgs containing attributes
 */
static inline int nlmsg_validate(const struct nlmsghdr *nh, size_t skip,
const struct nla_policy *pol, uint16_t maxtype) {
	return unl_validate(((uint8_t*)NLMSG_DATA(nh)) + NLMSG_ALIGN(skip),
		NLMSG_PAYLOAD(nh, skip), pol, maxtype);
}


/**
 * Same as nlmsg_find but searches inside a netlink attribute
 */
static inline struct nlattr* nla_find(const struct nlattr *nla, uint16_t type) {
	return unl_find(NLA_DATA(nla), NLA_PAYLOAD(nla), type);
}


/**
 * Same as nlmsg_parse but searches inside a netlink attribute
 */
static inline int nla_parse(const struct nlattr *nla, struct nlattr *nlatbl[],
uint16_t maxtype, const struct nla_policy *pol) {
	return unl_parse(NLA_DATA(nla), NLA_PAYLOAD(nla), nlatbl, maxtype, pol);
}


/**
 * Same as nlmsg_validate but validates nested attributes
 */
static inline int nla_validate(const struct nlattr *nla,
const struct nla_policy *pol, uint16_t maxtype) {
	return unl_validate(NLA_DATA(nla), NLA_PAYLOAD(nla), pol, maxtype);
}



#endif /* UNL_PARSE_H_ */
