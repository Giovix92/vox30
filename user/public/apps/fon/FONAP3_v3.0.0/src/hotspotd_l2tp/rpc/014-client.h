#ifndef RPC_CLIENT_H_
#define RPC_CLIENT_H_

enum rpc_cl {
	FRT_CL_GET	= 0x140,	// Get session information
					// FR_F_DUMP: Get all sessions
	FRT_CL_SET	= 0x141,	// Change session options
	FRT_CL_DEL	= 0x142,	// Logout a user
	FRT_CL_LOGIN	= 0x143,	// Login a user
#ifdef __SC_BUILD__
	FRT_CL_GET_INFO	= 0x144,	// Get clients information
    FRT_CL_ADD_AUTH_TO_DNRD = 0x145, //Update authenticated client to dnrd
#endif
					// FR_F_REPLACE: override current login
					// EBUSY when already authenticated
};

enum rpc_cl_attr {
	FRA_CL_UNKNOWN,
			// BEGIN: Identificators (unique session keys)
	FRA_CL_HWADDR,	// uint8_t[6]	mac-address
	FRA_CL_SESSID,	// uint32_t	session id
	FRA_CL_IPADDR,	// uint8_t[4] / uint8_t[16] layer 3 address
	FRA_CL_IFINDEX,	// uint32_t	interface index
			// END: Identificators
			// BEGIN: Logout options
	FRA_CL_REASON,	// uint32_t logout reason (see enum rpc_cl_attr_reason)
			// END: Logout options
			// BEGIN: Login options
	FRA_CL_METHOD,	// uint32_t	auth method (see enum rpc_cl_attr_method)
	FRA_CL_USERNAME,// string	username
	FRA_CL_KEY,	// buffer	authentication key (e.g. password if PAP)
	FRA_CL_BACKEND,	// string	alternative authentication backend
			// causes EBUSY when already authenticated or in process
			// END: Login options
			// BEGIN: Session stats
	FRA_CL_STATUS,	// uint32_t	auth state (see enum rpc_cl_attr_status)
#ifdef __SC_BUILD__
	FRA_CL_AUTH_COUNT,// uint32_t authed client
	FRA_CL_TOTAL_COUNT,// uint32_t total client
	FRA_CL_AUTH_SUM,// uint32_t authed sum
#endif
	FRA_CL_TIME,	// uint32_t session time in seconds
	FRA_CL_IDLE,	// uint32_t idle time in seconds
	FRA_CL_BIN,	// uint64_t bytes incoming
	FRA_CL_BOUT,	// uint64_t bytes outgoing
			// END: Session stats
			// BEGIN: Session options
	FRA_CL_TIME_MAX,// uint32_t session time limit in seconds
	FRA_CL_IDLE_MAX,// uint32_t idle time limit in seconds
	FRA_CL_INTERIM,	// uint32_t accounting interim interval in seconds
	FRA_CL_BIN_MAX,	// uint32_t	bytes incoming (limit)
	FRA_CL_BOUT_MAX,// uint32_t bytes outgoing (limit)
	FRA_CL_BOTH_MAX,// uint32_t bytes overall (limit)
	FRA_CL_NOACCT,	// uint32_t disable accounting (bool)
			// END: Session options
	_FRA_CL_SIZE,
};

enum rpc_cl_attr_method {
	CL_CLIENT_GRANT = 1,	// Authorize without authentication
	CL_CLIENT_PAP,		// PAP authentication
	CL_CLIENT_CHAP,		// CHAP authentication
	CL_CLIENT_EAP,		// EAP authentication
};

enum rpc_cl_attr_status {
	CL_CLSTAT_NOAUTH = 1,	// Unauthenticated
	CL_CLSTAT_LOGIN,	// Login in process
	CL_CLSTAT_AUTHED,	// Authenticated
	CL_CLSTAT_LOGOUT,	// Logout in process
};

enum rpc_cl_attr_reason {
	CL_LOGIN_SUCCESS		= 0,
	CL_LOGIN_PENDING		= CL_LOGIN_SUCCESS,
	CL_LOGOUT_USER 			= 1,
	CL_LOGOUT_DISCONNECT 		= 2,
	CL_LOGOUT_LOST_SERVICE		= 3,
	CL_LOGOUT_IDLE 			= 4,
	CL_LOGOUT_SESSION_LIMIT		= 5,
	CL_LOGOUT_ADMIN_RESET		= 6,
	CL_LOGOUT_ADMIN_REBOOT		= 7,
	CL_LOGOUT_NAS_ERROR		= 9,
	CL_LOGIN_ALREADY		= 50, // Chillispot madness compatibility...
	CL_LOGIN_DENIED			= 51,
	CL_LOGIN_ERROR			= 52,
	CL_LOGIN_NOTYET			= 55,
	CL_LOGIN_TIMEOUT		= 58,
	CL_LOGIN_INVALID_REQUEST	= 61,
	CL_LOGIN_CHALLENGE		= 62,
	CL_LOGIN_BUSY			= 63,
	CL_LOGIN_INSUFFICIENT_PAYMENT,
	CL_LOGIN_INSUFFICIENT_SESSIONS,
	CL_LOGIN_ISPFAIL_AUTHSERVER,
	CL_LOGIN_ISPFAIL_OVERLOAD,
};


#endif /* RPC_CLIENT_H_ */
