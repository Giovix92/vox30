#ifndef __SAL_IPV6_H__
#define __SAL_IPV6_H__
typedef struct tag_IPV6_LAN_NEIGH{
    char ip[64];
    char mac[18];
    char state[16];
} IPV6_LAN_NEIGH_t;

int sal_ipv6_get_lan_neigh(IPV6_LAN_NEIGH_t *ipv6_neigh);

#endif
