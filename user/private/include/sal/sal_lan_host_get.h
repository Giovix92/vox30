#ifndef __SAL_LAN_HOST_TABLE_H__
#define __SAL_LAN_HOST_TABLE_H__
typedef struct _sal_host_get
{
    char mac[18];
    char ip[16];
}sal_host_get;
         

int sal_get_lan_host_table (sal_host_get ** buf,int * host_num);
int sal_get_lan_has_active_host(void);
#endif
