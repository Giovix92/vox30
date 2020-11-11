#ifndef __LINUX_BRIDGE_EBT_TCPMSS_H
#define __LINUX_BRIDGE_EBT_TCPMSS_H
#include <linux/types.h>

struct ebt_tcpmss_info {
    __u16 mss;
};
#define XT_TCPMSS_CLAMP_PMTU 0xffff

#define EBT_TCPMSS_TARGET "TCPMSS"

#endif
