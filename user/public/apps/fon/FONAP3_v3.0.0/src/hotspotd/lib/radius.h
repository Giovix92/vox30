#ifndef RADIUS_H_
#define RADIUS_H_

#include <stddef.h>
#include <stdint.h>

#define RADIUS_AUTH_PORT "1812"
#define RADIUS_ACCT_PORT "1813"

enum radius_code {
	RADIUS_ACCESS_REQUEST = 1,
	RADIUS_ACCESS_ACCEPT = 2,
	RADIUS_ACCESS_REJECT = 3,
	RADIUS_ACCOUNTING_REQUEST = 4,
	RADIUS_ACCOUNTING_RESPONSE = 5,
	RADIUS_ACCESS_CHALLENGE = 11
};

enum radius_vendor {
	RADIUS_VENDOR_MS = 311,
	RADIUS_VENDOR_WISPR = 14122,
	RADIUS_VENDOR_CHILLISPOT = 14559,
	RADIUS_VENDOR_FON = 41100,
	RADIUS_VENDOR_TMOBILE = 3414,
};


enum radius_attr_type {
	RADIUS_USER_NAME = 1,
	RADIUS_USER_PASSWORD = 2,
	RADIUS_CHAP_PASSWORD = 3,
	RADIUS_NAS_IP = 4,
#ifdef __SC_BUILD__
	RADIUS_NAS_PORT = 5,
#endif
	RADIUS_SERVICE_TYPE = 6,
	RADIUS_CHILLISPOT_CONFIG = 6,
	RADIUS_FRAMED_IP_ADDRESS = 8,
#ifdef __SC_BUILD__
	RADIUS_FRAMED_MTU = 12,
#endif
	RADIUS_MS_MPPE_SEND_KEY = 16,
	RADIUS_MS_MPPE_RECV_KEY = 17,
	RADIUS_REPLY_MSG = 18,
	RADIUS_STATE = 24,
	RADIUS_CLASS = 25,
	RADIUS_VENDOR = 26,
	RADIUS_SESSION_TIMEOUT = 27,
	RADIUS_IDLE_TIMEOUT = 28,
	RADIUS_TERMINATION_ACTION = 29,
	RADIUS_CALLED_STATION_ID = 30,
	RADIUS_CALLING_STATION_ID = 31,
	RADIUS_NAS_IDENTIFIER = 32,
	RADIUS_ACCT_STATUS_TYPE = 40,
	RADIUS_ACCT_INPUT_OCTETS = 42,
	RADIUS_ACCT_OUTPUT_OCTETS = 43,
	RADIUS_ACCT_SESSION_ID = 44,
	RADIUS_ACCT_SESSION_TIME = 46,
	RADIUS_ACCT_INPUT_PACKETS = 47,
	RADIUS_ACCT_OUTPUT_PACKETS = 48,
	RADIUS_ACCT_TERMINATE_CAUSE = 49,
	RADIUS_ACCT_INPUT_GIGAWORDS = 52,
	RADIUS_ACCT_OUTPUT_GIGAWORDS = 53,
	RADIUS_ATTR_EVENT_TIMESTAMP = 55,
	RADIUS_NAS_PORT_TYPE = 61,
	RADIUS_EAP_MSG = 79,
	RADIUS_MSG_AUTHENTICATOR = 80,
	RADIUS_ACCT_INTERIM_INTERVAL = 85,
#ifdef __SC_BUILD__
	RADIUS_NAS_PORT_ID = 87,
#endif
	RADIUS_CUI = 89,
	RADIUS_FRAMED_IPV6_PREFIX = 97,
	RADIUS_CHILLISPOT_MAX_INPUT_OCTETS = 1,
	RADIUS_CHILLISPOT_MAX_OUTPUT_OCTETS = 2,
	RADIUS_CHILLISPOT_MAX_TOTAL_OCTETS = 3,
	RADIUS_WISPR_LOCATION_ID = 1,
	RADIUS_WISPR_LOCATION_NAME = 2,
	RADIUS_WISPR_LOGOFF_URL = 3,
	RADIUS_FON_TUNNEL_USER = 8,
	RADIUS_FON_TUNNEL_PASS = 4,
	RADIUS_OPERATOR_NAME = 126,
	RADIUS_LOCATION_INFORMATION = 127,
	RADIUS_LOCATION_DATA = 128,
	RADIUS_BASIC_POLICY_RULES = 129,
	RADIUS_EXTENDED_POLICY_RULES = 130,
	RADIUS_LOCATION_CAPABLE = 131,
	RADIUS_REQUESTED_LOCATION = 132,
	RADIUS_TMOBILE_VENUE_CLASS = 40,
};

enum radius_target {
	RADIUS_RADCONF = 0,
	RADIUS_PROVISION = 1
};

#define RADIUS_NAS_PORT_TYPE_80211 19

struct radius_attr {
	uint8_t type;
	uint8_t length;
	uint8_t payload[];
};

struct radius_vattr {
	uint32_t vendor;
	uint8_t vtype;
	uint8_t vlength;
	uint8_t vpayload[];
} __attribute__((packed));

struct radius_pkt {
	uint8_t code;
	uint8_t identifier;
	uint16_t length;
	uint8_t authenticator[16];
	uint8_t attributes[];
};

struct radius_cluster;

typedef void (radius_cb)(struct radius_pkt *pkt, void *context);

#define radius_foreach_attr(attr, pkt) \
	attr = (struct radius_attr*)pkt->attributes; \
	radius_foreach_attr_continue(attr, pkt)

#define radius_foreach_attr_continue(attr, pkt) \
	for (ssize_t _len = pkt->length - ((uint8_t*)attr - (uint8_t*)pkt); \
	_len >= 2 && _len >= attr->length; \
	_len -= attr->length, attr = (void*)(((uint8_t*)attr) + attr->length))

static inline struct radius_attr* radius_vattr_to_attr(struct radius_attr *a) {
	struct radius_vattr *vattr = (void*)a->payload;
	return (void*)&vattr->vtype;
}

struct radius_pkt* radius_pkt_init(void* buffer, uint8_t code);
void radius_pkt_append
(struct radius_pkt *pkt, uint8_t type, const void *payload, uint32_t len);

void radius_pkt_append_vendor(struct radius_pkt *pkt, uint32_t vendor,
		uint8_t type, const void *payload, uint32_t len);

int radius_pkt_match(struct radius_attr *attr, uint8_t type, uint32_t vendor);
uint32_t radius_pkt_get_u32(struct radius_attr *attr);

size_t radius_finalize(struct radius_pkt *npkt, const char *secret);

struct radius_cluster* radius_create(const char *section, const char *port,
			const char *templatefill);
void radius_destroy(struct radius_cluster *cl);

void radius_init_default_servers();
void radius_clear_default_servers();

int radius_request(struct radius_cluster *cl, const struct radius_pkt *pkt,
						radius_cb *cb, void *ctx);
int radius_cancel(struct radius_cluster *cl, int handle);
int radius_sock(struct radius_cluster *cl);

#endif /* RADIUS_H_ */
