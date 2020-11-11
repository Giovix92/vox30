#ifndef _WIZARD_H_
#define _WIZARD_H_

#include "wizard.h"

//wan status
typedef enum
{
    SW_WAN_PROCESSING = 0,
    SW_WAN_NO_LINK,
    SW_WAN_FTTH_IPoE,
    SW_WAN_FTTH_PPPoE,
    SW_WAN_DSL_PPPoE,
    SW_WAN_DSL_IPoE,
#ifdef CONFIG_SUPPORT_VDSL
    SW_WAN_VDSL_IPoE,
    SW_WAN_VDSL_PPPoE,
#endif
} SW_WAN_TYPE_E;

SW_WAN_TYPE_E sw_get_wan_service_type(int *wan_id);

typedef enum
{
    SW_WAN_NO_PHY_LINK,
    SW_WAN_NO_CONNECTION,
    SW_WAN_NO_IP,
    SW_WAN_UP
} SW_WAN_CONNECTION_E;

void sw_restart_autosense(void);
#ifdef CONFIG_SUPPORT_VDSL
SW_WAN_CONNECTION_E sw_get_wan_status(char *link, char *mode, SW_WAN_TYPE_E phy);
#else
SW_WAN_CONNECTION_E sw_get_wan_status(char *link, char *mode);
#endif
void sw_restart_dsl_pppoe(void);
void sw_restart_dsl(void);
void sw_restart_dsl_ipoe(void);
void sw_restart_ether(void);
void sw_restart_ether_pppoe(void);
int sw_get_pppoe_wanid(char *link);

typedef enum
{
    SW_WIFI_PROCESS,
    SW_WIFI_NO_BINDING,
    SW_WIFI_BIND_FAIL,
    SW_WIFI_BIND_SUCCESS
} SW_WIFI_BINDING_E;

typedef enum
{
    WSC_PROC_IDLE,
    WSC_PROC_WAITING,
    WSC_PROC_SUCC,
    WSC_PROC_TIMEOUT,
    WSC_PROC_FAIL,
    WSC_PROC_M2_SENT,
    WSC_PROC_M7_SENT,
    WSC_PROC_MSG_DONE,
    WSC_PROC_PBC_OVERLAP
} WSC_PROC_STATE_E;

//wifi status
void sw_wireless_pbc_start(void);
void sw_wireless_pbc_stop(void);
WSC_PROC_STATE_E sw_wireless_get_wps_process_status(void);

void sw_wireless_binding_start(void);
SW_WIFI_BINDING_E sw_wireless_check_binding_status(void);

//installing status
int sw_is_cpe_configured(void);
int sw_is_cpe_under_installation_process(void);
#endif
