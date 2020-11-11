#ifndef _SAL_ROUTE_H_ 
#define _SAL_ROUTE_H_


#define SAL_ROUTE_MAX_ENTRY 512

typedef struct 
{
    char dst[64];
    char gw[64];
    char mask[64];
    char flags[8];
    char metric[8];
    char ref[8];
    char use[8];
	char dev[64];
}sal_route;

int sal_route_get_route_entry(sal_route** route);
#ifdef CONFIG_SUPPORT_IPV6
int sal_route_ipv6_get_route_entry(sal_route** route_info);
#endif

#endif
