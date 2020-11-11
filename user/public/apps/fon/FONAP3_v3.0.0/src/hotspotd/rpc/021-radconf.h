#ifndef RPC_RADCONF_H_
#define RPC_RADCONF_H_

enum rpc_rc {
	FRT_RC_CALL	= 0x210,	// Do radconf call
};

enum rpc_rc_attr {
	FRA_RC_UNKNOWN,
			// BEGIN: Identificators (unique session keys)
	FRA_RC_USER,	// string username
	_FRA_RC_SIZE,
};

#endif /* RPC_RADCONF_H_ */
