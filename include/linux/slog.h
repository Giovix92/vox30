#ifndef __SLOG_H__
#define __SLOG_H__


#define SYNC_CRIT_FLAG (1 << 16)
enum
{
    UNKNOWN_LOG = 0,
    CRIT_LOG,
    NORM_LOG
};

#define LOG_NONUSE_BLOCK_TIME  0
#define LOG_NONUSE_ID          0

#define CRIT_PRE_SIGN (char)('{')
#define CRIT_SUF_SIGN (char)('}')
#define NORM_PRE_SIGN (char)('[')
#define NORM_SUF_SIGN (char)(']')
#define SYNC_CRIT_FLAG_SIGN (char)(';') 

#define MAX_SIZE_PER_LOG    (1024*4) 

#define LOG_FW_PORT_SCAN_ID     12
#define LOG_FW_HALF_FLOOD_ID    13 
#define LOG_FW_BLOCK_TIME       300

void log_module(char *pri, int type, int id, int sec, char *module, const char *fmt, ...);


/* Please add your module function and should keep the id base value no diffrence with application */

#define LOG_SYS(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (1 << 16); \
    log_module(pri, type, xid, time, "System", fmt, ##arg); \
}while(0)


#define LOG_GPON(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (2 << 16); \
    log_module(pri, type, xid, time, "GPON", fmt, ##arg); \
}while(0)

#define LOG_ETH(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (3 << 16); \
    log_module(pri, type, xid, time, "Ethernet", fmt, ##arg); \
}while(0)

#define LOG_WIFI(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (4 << 16); \
    log_module(pri, type, xid, time, "WIFI", fmt, ##arg); \
}while(0)

#define LOG_USB(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (5 << 16); \
    log_module(pri, type, xid, time, "USB", fmt, ##arg); \
}while(0)


#define LOG_VOIP(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (6 << 16); \
    log_module(pri, type, xid, time, "VoIP", fmt, ##arg); \
}while(0)

#define LOG_IPTV(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (7 << 16); \
    log_module(pri, type, xid, time, "IPTV", fmt, ##arg); \
}while(0)

#define LOG_OMCI(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (14 << 16); \
    log_module(pri, type, xid, time, "OMCI", fmt, ##arg); \
}while(0)

#define LOG_LAN(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (23 << 16); \
    log_module(pri, type, xid, time, "LAN", fmt, ##arg); \
}while(0)

#define LOG_WAN(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (24 << 16); \
    log_module(pri, type, xid, time, "WAN", fmt, ##arg); \
}while(0)

#define LOG_FIREWALL(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (25 << 16); \
    log_module(pri, type, xid, time, "Firewall", fmt, ##arg); \
}while(0)

#define LOG_UMTS(pri, type, id, time, fmt, arg...) \
do{\
    int xid = id; \
    if (id != LOG_NONUSE_ID) \
        xid = id + (27 << 16); \
    log_module(pri, type, xid, time, "UMTS", fmt, ##arg); \
}while(0)

#endif

