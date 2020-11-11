#ifndef RPC_CORE_H_
#define RPC_CORE_H_

enum rpc_sys {
	FRT_SYS_INFO	= 0x100,	// Returns current process state */
#ifdef __SC_BUILD__
	FRT_SYS_START	= 0x10C,	
	FRT_SYS_STOP	= 0x10D,	
	FRT_SYS_RELOAD	= 0x10e,	
	FRT_SYS_STATUS	= 0x10f,
#endif
};

enum rpc_sys_attr {
	FRA_SYS_UNKNOWN,
	FRA_SYS_PID,			// uint32_t: pid
	FRA_SYS_DYNMEMORY,		// uint32_t: Dynamic memory allocated
#ifdef __SC_BUILD__
	FRA_SYS_STATUS,		
#endif
	_FRA_SYS_SIZE,
};

enum rpc_cfg {
	FRT_CFG_SET	= 0x108,	// Set the value of a configuration option
					// honors: FRM_F_APPEND
	FRT_CFG_COMMIT	= 0x109,	// Commit pending changes to configuration file
	FRT_CFG_APPLY	= 0x10A,	// Reapply configuration
	FRT_CFG_RESTART	= 0x10B,	// Issue a full restart
};

enum rpc_cfg_attr {
	FRA_CFG_UNKNOWN,
	FRA_CFG_SECTION,		// string: configuration section
	FRA_CFG_OPTION,			// string: configuration option
	FRA_CFG_VALUE,			// string: configuration value
	_FRA_CFG_SIZE,
};

#endif /* RPC_CORE_H_ */
