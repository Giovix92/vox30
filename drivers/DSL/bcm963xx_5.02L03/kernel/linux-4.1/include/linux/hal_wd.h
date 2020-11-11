#ifndef __CONFIG_CRASH_LOG__
#define __CONFIG_CRASH_LOG__

#define KERNEL_SDRAM_IMAGE_SIZE					0x100000

/*BOOT_FREE_ADDRESS get address from kerSysGetKernelCrashMemory()*/
extern unsigned long boot_free_address;
#define BOOT_FREE_ADDRESS       (boot_free_address)
#define BOOT_LOG_ADDRESS        (BOOT_FREE_ADDRESS  +  (1 << 11))  
#define SC_DMESG_LOG_SIZE		8192  //8k
#define BOOT_STACK_ADDRESS      (BOOT_LOG_ADDRESS + (1 << 13))
#define SC_STACK_LOG_SIZE       6144 //6k
extern int  sc_boot_flag;
extern int  sc_boot_flag_copy;
extern char stack_log_buf[SC_STACK_LOG_SIZE];
extern unsigned int stack_buf_len;

#define SC_LOG_MSG_TO_BUFF(fmt, args...) do{               \
    if(stack_buf_len <= (SC_STACK_LOG_SIZE-256))\
    {\
        stack_buf_len += sprintf(stack_log_buf+stack_buf_len,fmt,##args);    \
    }\
}while(0)
typedef enum{	
	REBOOT_POWER_OFF = 0,	
	REBOOT_TRIGGER_BY_USER,	
	REBOOT_HW_WATCH_DOG,
	REBOOT_PANIC_TIMEOUT,
	REBOOT_KERNEL_CRASH,
	REBOOT_FROM_ALLOC_PAGE_FAILED,
	REBOOT_FROM_OUT_OF_MEMORY,
	REBOOT_FROM_KILL_OTHER_PROGRESS,
	REBOOT_UNALIGNED_ACCESS,
	REBOOT_CPU_TIMEUP,
	REBOOT_CPU_LOCKUP,
	REBOOT_APP_HANGUP,
	REBOOT_UNKNOW,
}HAL_REBOOT_REASON;

#endif
