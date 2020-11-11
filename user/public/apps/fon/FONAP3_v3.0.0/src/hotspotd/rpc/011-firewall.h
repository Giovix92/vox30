#ifndef RPC_FIREWALL_H_
#define RPC_FIREWALL_H_

enum rpc_fw {
	FRT_FW_NEWBLOCK	= 0x110,	// Returns current process state
	FRT_FW_DELBLOCK	= 0x111,	// Returns current process state
};

enum rpc_fw_attr {
	FRA_FW_UNKNOWN,
	FRA_FW_HWADDR,		// uint8_t[6]: MAC-address
	_FRA_FW_SIZE,
};

#endif /* RPC_FIREWALL_H_ */
