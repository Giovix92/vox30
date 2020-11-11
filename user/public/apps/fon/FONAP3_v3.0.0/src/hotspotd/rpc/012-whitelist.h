#ifndef RPC_WHITELIST_H_
#define RPC_WHITELIST_H_

enum rpc_wl {
	FRT_WL_GETACTIVE	= 0x120,	// Return entries
						// requires: FR_F_DUMP
};

enum rpc_wl_attr {
	FRA_WL_UNKNOWN,
	FRA_WL_IPV4,		// uint32_t: IPv4 address
	FRA_WL_IPV6,		// uint128_t: IPv6 address
	FRA_WL_PREFIX,		// uint32_t: network prefix
	FRA_WL_EXPIRES,		// uint32_t: Expire time in seconds
	_FRA_WL_SIZE,
};

#endif /* RPC_WHITELIST_H_ */
