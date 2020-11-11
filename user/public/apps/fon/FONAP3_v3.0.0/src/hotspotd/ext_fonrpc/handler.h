#ifndef HANDLER_FONRPC_H_
#define HANDLER_FONRPC_H_

#include <time.h>
#include "libfonrpc/fonrpc.h"

#define FR0_VALIDITY 30

#define FR_BUFFER_SIZE	65536
#define FR_CONFIG_SIZE	32768


enum frmsg_fonrpc {
	FRMSG_CHECKIN = FRMSG_MIN_TYPE,
};

struct frmsghdr {
	uint32_t time;
	uint32_t seq;
	struct frattr fra;
};

struct fr0hdr {
	uint8_t version;
	uint8_t code;
	uint8_t nodeid[6];
	uint8_t aesiv[16];
};

struct fr0hdr_sync {
	uint8_t version;
	uint8_t code;
	uint8_t pad[2];
	uint32_t time;
	uint8_t aesiv[16];
};

enum fr0_code {
	FR0_E_SUCCESS,
	FR0_E_MAXERROR = 239,
	FR0_E_TIMESYNC = 254,
	FR0_E_NOACCESS = 255,
};

enum fra_checkin {
	FRA_CI_UNKNOWN,
	FRA_CI_CFGTIME,	// uint32_t
	FRA_CI_CFGDATA,	// string
	FRA_CI_USERS,	// uint32_t
	FRA_CI_FWVER,	// uint8_t[4]
	FRA_CI_UPTIME,	// uint32_t
	FRA_CI_BYTESIN,	// uint64_t
	FRA_CI_BYTESOUT,// uint64_t
	FRA_CI_SESSID,	// uint32_t
	FRA_CI_FWURL,	// string
	FRA_CI_MAX
};

#define AES_ALIGNTO 16
#define AES_ALIGN(len) (((len) + AES_ALIGNTO - 1) & ~(AES_ALIGNTO - 1))

#endif /* HANDLER_FONRPC_H_ */
