
#ifndef	_SC_DRV_ETH_H_
#define	_SC_DRV_ETH_H_

#include "sc_drv_types.h"

typedef struct _ScDrv_Eth_Port_Stats
{
    int port_id; /* SC_DRV_ETH_PORT_ID */
    unsigned long rx_bytes;
    unsigned long rx_pkts;
    unsigned long rx_err_pkts;
    unsigned long rx_mcast_pkts;
    unsigned long rx_bcast_pkts;
    unsigned long tx_bytes;
    unsigned long tx_pkts;
    unsigned long tx_err_pkts;
    unsigned long tx_mcast_pkts;
    unsigned long tx_bcast_pkts;
} ScDrv_Eth_Port_Stats_t;

typedef struct _ScDrv_Eth_Port_Flow_Control
{
    int port_id; /* SC_DRV_ETH_PORT_ID */
    int tx_en; /* 0 Disable | 1 Enable */
    int rx_en; /* 0 Disable | 1 Enable */
} ScDrv_Eth_Port_Flow_Control_t;


#endif /* _SC_DRV_ETH_H_ */

