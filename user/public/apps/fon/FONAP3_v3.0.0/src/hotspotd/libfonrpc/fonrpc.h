#ifndef LIBFONRPC_H_
#define LIBFONRPC_H_

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#ifdef __SC_BUILD__
#include <linux/byteorder/little_endian.h>
#endif

# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define fswap64(x)	x
#  define fswap32(x)	x
#  define fswap16(x)	x
# else
#  define fswap64(x)	bswap_64(x)
#  define fswap32(x)	bswap_32(x)
#  define fswap16(x)	bswap_16(x)
# endif

struct frattr {
	uint16_t fra_len;
	uint16_t fra_type;
};

/* Standard Types */
enum frm_types {
	FRMSG_NOOP	= 0x01,
	FRMSG_ERROR	= 0x02,
	FRMSG_DONE	= 0x03,
	FRMSG_MIN_TYPE	= 0x10,
};

#define FRA_ALIGNTO 4
#define FRA_ALIGN(len) (((len) + FRA_ALIGNTO - 1) & ~(FRA_ALIGNTO - 1))

#define FRA_HDRLEN		sizeof(struct frattr)
#define FRA_TYPE(fra)	(fswap16((fra)->fra_type))
#define FRA_DATA(fra)   ((void*)(((uint8_t*)(fra)) + FRA_HDRLEN))
#define FRA_LENGTH(fra) ((ssize_t)(fswap16((fra)->fra_len)))
#define FRA_PAYLOAD(fra) (FRA_LENGTH((fra)) - FRA_HDRLEN)
#define FRA_OK(fra,len) ((len) >= ((ssize_t)sizeof(struct frattr)) && \
                         fswap16((fra)->fra_len) >= sizeof(struct frattr) && \
                         fswap16((fra)->fra_len) <= (len))
#define FRA_NEXT(fra,attrlen)   ((attrlen) -= FRA_ALIGN(fswap16((fra)->fra_len)), \
                                 (struct frattr*)(((uint8_t*)(fra)) + FRA_ALIGN(fswap16((fra)->fra_len))))

// Iterate over all attributes "fra" of an attribute stream
#define fr_foreach_attr(fra, buffer, len) \
	(fra) = (struct frattr*)(buffer); \
	for (ssize_t _rlen = (len); FRA_OK((fra), _rlen); (fra) = FRA_NEXT((fra), _rlen))

// Iterate over all attributes "fra" of a message
#define fra_foreach_attr(frac, fra) \
	fr_foreach_attr(frac, FRA_DATA((fra)), FRA_PAYLOAD((fra)))


void fra_parse(struct frattr *fra, struct frattr *fratbl[], size_t size);

/**
 * Validates the payload of an attribute and returns its converted value.
 * If fra is NULL or the payload doesn't match, def is returned instead.
 */
const char *fra_to_string(struct frattr *fra);
uint32_t fra_to_u32(struct frattr *fra, uint32_t def);
uint64_t fra_to_u64(struct frattr *fra, uint64_t def);




struct frattr* fra_init(void *buffer, uint16_t type);

void* fra_claim(struct frattr *fra, uint16_t type, uint16_t len);

void fra_put(struct frattr *fra, uint16_t type, const void *data, uint16_t len);

#define fra_put_type(frm, type, datatype, value) \
	(*((datatype*)fra_claim(frm, type, sizeof(value))) = value)

static inline void fra_put_string(struct frattr *fra,
			uint16_t type, const char *value)
{
	fra_put(fra, type, value, strlen(value) + 1);
}

static inline void fra_put_flag(struct frattr *fra, uint16_t type)
{
	fra_claim(fra, type, 0);
}

static inline void fra_put_u32(struct frattr *fra,
			uint16_t type, uint32_t value)
{
	fra_put_type(fra, type, uint32_t, fswap32(value));
}

static inline void fra_put_u64(struct frattr *fra,
			uint16_t type, uint64_t value)
{
	fra_put_type(fra, type, uint64_t, fswap64(value));
}



/**
 * Start a new attr at the end of the stream without increasing the stream size
 */
static inline struct frattr* fra_start_attr(struct frattr *outer, uint16_t type)
{
	return fra_init(((uint8_t*)outer) + FRA_ALIGN(FRA_LENGTH(outer)), type);
}

/**
 * Finishes an attribute started with nla_start_attr and commits it to the outer attribute
 */
static inline void fra_commit_attr(struct frattr *outer, struct frattr *fra)
{
	fra_claim(outer, FRA_TYPE(fra), FRA_LENGTH(fra));
}

#endif /* LIBFONRPC_H_ */
