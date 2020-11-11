#ifndef FONRPC_LOCAL_H_
#define FONRPC_LOCAL_H_

#include "fonrpc.h"

enum frm_flags {
	FRM_F_REQUEST	= 0x001,	// a request
	FRM_F_MULTI	= 0x002,	// multipart message

	FRM_F_DUMP	= 0x200,	// dump all entries (expect multipart)

	FRM_F_REPLACE	= 0x100,	// replace existing
	FRM_F_EXCL	= 0x200,	// create only if non-existant
	FRM_F_CREATE    = 0x400,	// create if not existing
	FRM_F_APPEND    = 0x800,	// append to a list
};

/* FRMSG_ERROR attributes */
enum frm_errtypes {
	FRE_UNKNOWN,
	FRE_CODE,
	_FRE_SIZE,
};

/* Internal Implementation */

struct frmsg {
	uint32_t frm_flags;	/* Additional flags */
	uint32_t frm_seq;	/* Sequence number */
	struct frattr fra;
};


#define frm_align(len) FRA_ALIGN(len)
#define frm_length(frm) (sizeof(*(frm)) + FRA_PAYLOAD(&(frm)->fra))
#define frm_flags(frm) (fswap32((frm)->frm_flags))
#define frm_type(frm) (FRA_TYPE(&(frm)->fra))
#define frm_parse(frm, attrs, size) fra_parse(&(frm)->fra, attrs, size)

#define frm_put_buffer(frm, type, data, len) fra_put(&(frm)->fra, type, data, len)
#define frm_put_string(frm, type, str) fra_put_string(&(frm)->fra, type, str)
#define frm_put_flag(frm, type) fra_put_flag(&(frm)->fra, (type))
#define frm_put_u32(frm, type, val) fra_put_u32(&(frm)->fra, type, val)
#define frm_put_u64(frm, type, val) fra_put_u64(&(frm)->fra, type, val)

#define fra_length(fra) ((!fra || FRA_LENGTH(fra) < FRA_HDRLEN) ? 0 : FRA_PAYLOAD(fra))
#define fra_data FRA_DATA

#define FRM_NEXT(frm,len) ((len) -= FRA_ALIGN(frm_length((frm))), \
                 (struct frmsg*)(((uint8_t*)(frm)) + FRA_ALIGN(frm_length((frm)))))

#ifdef __SC_BUILD__
static inline struct frmsg* frm_init(void *buffer, uint16_t type, uint32_t flags) {
	struct frmsg *frm = buffer;
	frm->frm_flags = fswap32(flags);
	fra_init(&frm->fra, type);
	return frm;
}
#else
static inline struct frmsg* frm_init(void *buffer, uint16_t type, uint16_t flags) {
	struct frmsg *frm = buffer;
	frm->frm_flags = fswap16(flags);
	fra_init(&frm->fra, type);
	return frm;
}
#endif

static inline struct frmsg* frm_init_status(void *buffer, uint32_t code) {
	struct frmsg *frm = frm_init(buffer, FRMSG_ERROR, 0);
	frm_put_u32(frm, FRE_CODE, code);
	return frm;
}

static inline uint32_t frm_status(struct frmsg *frm) {
	if (!frm || frm_type(frm) != FRMSG_ERROR)
		return 0;
	struct frattr *fra[_FRE_SIZE];
	frm_parse(frm, fra, _FRE_SIZE);
	return fra_to_u32(fra[FRE_CODE], 0);
}

static inline struct frmsg* frm_load(void *buffer, size_t len) {
	struct frmsg *frm = buffer;
	return (len >= sizeof(*frm) && frm_length(frm) <= len) ? frm : NULL;
}



#endif /* FONRPC_LOCAL_H_ */
