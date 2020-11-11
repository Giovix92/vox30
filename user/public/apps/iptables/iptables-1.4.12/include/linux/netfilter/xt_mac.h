#ifndef _XT_MAC_H
#define _XT_MAC_H

struct xt_mac_info {
    unsigned char srcaddr[ETH_ALEN];
#ifdef CONFIG_SCM_SUPPORT
    unsigned char srcaddrmsk[ETH_ALEN]; 
#endif
    int invert;
};
#endif /*_XT_MAC_H*/
