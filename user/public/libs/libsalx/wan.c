/**
 * @file   wan_t.c
 * @author Martin Huang martin_huang@sdc.sercomm.com
 * @date   2011-08-11
 * @brief  support multiple wan configuration abstract layer API.
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifisally
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
//#include <linux/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <nvram.h>
#include <pathnames.h>
#include <cal/cal_wan.h>
#include <sal/sal_wan.h>
#define SAL_WAN_TMP_CFG_NAME			"s000"	
#define SAL_WAN_PORT_INFO_FILE		    WAN_CFG_BASE"%d/port_info"
#define SAL_WAN_PORT_INFO_LOCK_FILE		"/var/lock/port_info_%d.lock"

#define SAL_WAN_TMP_L2_IF_CFG_NAME		"l2_ifName"
#ifdef CONFIG_SUPPORT_AUTO_DETECT
#define SAL_WAN_TMP_L2_IF_ACTIVE        "l2_if_active"
#endif
#define SAL_WAN_TMP_CON_MODE_CFG_NAME	"con_mode"
#define SAL_WAN_TMP_CON_TYPE_CFG_NAME	"con_type"
#ifdef CONFIG_SUPPORT_HA
#define SAL_WAN_TMP_HA_STATE	        "ha_state"
#endif
#define SAL_WAN_TMP_IP_ST_NAME			"con_ip"
#define SAL_WAN_TMP_GW_ST_NAME			"con_gw"
#define SAL_WAN_TMP_IP_MASK_ST_NAME		"con_ipmask"
#define SAL_WAN_TMP_DNS1_ST_NAME		"con_dns1"
#define SAL_WAN_TMP_DNS2_ST_NAME		"con_dns2"
#define SAL_WAN_TMP_LEASE_ST_NAME		"con_lease"
#define SAL_WAN_TMP_OPT12_ST_NAME		"con_option12"
#define SAL_WAN_TMP_OPT42_ST_NAME		"con_option42"
#define SAL_WAN_TMP_OPT43_ST_NAME		"con_option43"
#ifdef CONFIG_SUPPORT_FWA
#define SAL_WAN_TMP_FWA_IMSI           "fwa_imsi"
#endif
#define SAL_WAN_TMP_OPT120_ST_NAME		"con_option120"
#define SAL_WAN_TMP_OPT121_ST_NAME		"con_option121"
#define SAL_WAN_TMP_OPT121P_ST_NAME		"con_option121p"
#define SAL_WAN_TMP_OPT121N_ST_NAME		"con_option121n"
#define SAL_WAN_TMP_OPT249_ST_NAME		"con_option249"
#define SAL_WAN_TMP_OPT249P_ST_NAME		"con_option249p"
#define SAL_WAN_TMP_OPT249N_ST_NAME		"con_option249n"
#define SAL_WAN_TMP_OPT33_ST_NAME		"con_option33"
#define SAL_WAN_TMP_OPT33P_ST_NAME		"con_option33p"
#define SAL_WAN_TMP_OPT33N_ST_NAME		"con_option33n"
#define SAL_WAN_TMP_OPT2_ST_NAME		"con_option2"

#define SAL_WAN_TMP_DHCP_SERVER_ST_NAME		"con_dhcp_server"

#define SAL_WAN_TMP_GW_MAC_ST_NAME		"con_gw_mac"

#define SAL_WAN_TMP_PPP_SESSION_ST_NAME	    "con_ppp_session"
#define SAL_WAN_TMP_PPP_TRIGGER_ST_NAME	    "con_ppp_trigger"
#ifdef CONFIG_PPP_SUPPORT_PADM_PADN
#define SAL_WAN_TMP_PPP_TAG_HRUL	        "con_ppp_hurl"
#define SAL_WAN_TMP_PPP_TAG_MOTM_NTP1	    "con_ppp_ntp1"
#define SAL_WAN_TMP_PPP_TAG_MOTM_NTP2	    "con_ppp_ntp2"
#define SAL_WAN_TMP_PPP_TAG_MOTM_NTP3	    "con_ppp_ntp3"
#define SAL_WAN_TMP_PPP_TAG_MOTM_NTP4	    "con_ppp_ntp4"
#define SAL_WAN_TMP_PPP_TAG_MOTM_NTP5	    "con_ppp_ntp5"
#define SAL_WAN_TMP_PPP_TAG_MOTM_PROVCODE	"con_ppp_provcode"
#define SAL_WAN_TMP_PPP_TAG_IP_ROUTE_ADD	"con_ppp_ip_route_add"
#ifdef CONFIG_SUPPORT_IPV6
#define SAL_WAN_TMP_MOTM_MNG_IPV6          "motm_mng_ipv6"
#endif
#endif
#define SAL_WAN_TMP_ALIVE_ST_NAME	        "con_alive"
#define SAL_WAN_TMP_UPTIME_ST_NAME		    "con_uptime"
#define SAL_WAN_TMP_PASTTIME_ST_NAME		"con_pasttime"
#define SAL_WAN_TMP_STATE_ST_NAME		    "con_state"
#ifdef CONFIG_SUPPORT_WAN_BACKUP
#define SAL_WAN_TMP_CHECK_ST_NAME		    "check_state"
#endif
#define SAL_WAN_TMP_RSTATE_ST_NAME		    "con_rstate"
#define SAL_WAN_TMP_CLIENT_PID_ST_NAME		"con_client_pid"
#define SAL_WAN_TMP_AUTH_FAILED_ST_NAME		"con_auth_failed"

#define SAL_WAN_TMP_HW_IDLE_TIME_ST_NAME	"con_hw_idle_time"
#define SAL_WAN_TMP_PPP_LCP_ST_NAME		"con_lcp_state"
#define SAL_WAN_TMP_PPP_LCP_TIME		"con_lcp_time"
#define SAL_WAN_TMP_PHY_INIT_ST_NAME		"con_phy_init"
#define SAL_WAN_TMP_CON_VLANID		        "con_vlanid"
#define SAL_WAN_TMP_CON_BRIDGEID            "con_bridgeid"
#define SAL_WAN_TMP_DEL_OBJ                 "del_obj"
#define SAL_WAN_TMP_DEL_WAN                 "del_wan"
#define SAL_WAN_TMP_MANUAL_TRIGGERED        "manu_trigger"
#define SAL_WAN_TMP_PPP_IDLE_TIME           "ppp_idle"
#define SAL_WAN_TMP_AUTO_DISCONNECT         "auto_disconnect"
#define SAL_WAN_TMP_AUTO_TRIGGERED          "auto_trigger"
#ifdef AD1018
#define SAL_WAN_TMP_SOFTIRQ_PREEMPT         "softirq_preempt"
#endif

#ifdef CONFIG_SUPPORT_IPV6
#define SAL_WAN_IPV6_TMP_WAN_IF              "wan_ipv6_wan_if"
#define SAL_WAN_IPV6_TMP_WAN_IP              "wan_ipv6_wan_ip"
#define SAL_WAN_IPV6_TMP_PREFIX_NAME         "wan_ipv6_prefix"
#define SAL_WAN_IPV6_TMP_PREFIX_LENGTH_NAME  "wan_ipv6_prefix_length"
#define SAL_WAN_IPV6_TMP_PREFIX_PREF_TIME    "wan_ipv6_prefix_pltime"
#define SAL_WAN_IPV6_TMP_PREFIX_VALID_TIME   "wan_ipv6_prefix_vltime"
#define SAL_WAN_IPV6_TMP_DEVICE_NAME         "wan_ipv6_device"
#define SAL_WAN_IPV6_TMP_DNS                 "wan_ipv6_dns"
#define SAL_WAN_IPV6_TMP_IPV6CP_STATE        "wan_ipv6_ipv6cp_state"
#define SAL_WAN_IPV6_LLADDR                  "wan_ipv6_lladdr"
#define SAL_WAN_IPV6_RLADDR                  "wan_ipv6_rladdr"
#define SAL_WAN_IPV6_TMP_NTP_NAME            "wan_ipv6_ntp"
#define SAL_WAN_IPV6_TMP_DOMAIN_NAME         "wan_ipv6_domain_name"
#define SAL_WAN_IPV6_TMP_ULA_PREFIX          "wan_ipv6_ula_prefix"
#define SAL_WAN_TMP_IPV6_OPEN_STATE          "wan_ipv6_open_state"
#define SAL_WAN_IPV6_TMP_SIP_NAME            "wan_ipv6_sip_name"
#define SAL_WAN_IPV6_TMP_SIP                 "wan_ipv6_sip"
#define SAL_WAN_IPV6_DEFAULT_ROUTER_EXIST    "wan_ipv6_default_route"
#define SAL_WAN_VOIP_STATUS                  "wan_voip_status"
#endif
#ifdef CONFIG_SUPPORT_DSL
#define SAL_WAN_TMP_PHY_INIT_DSL            "con_phy_dsl"
#define SAL_WAN_TMP_PHY_DSL_UPTIME          "phy_dsl_uptime"
#define SAL_WAN_TMP_DSL_LINK_ST_NAME		"dsl_link"
#define SAL_WAN_TMP_DSL_PVC_ST_NAME         "dsl_pvc"
#define SAL_WAN_TMP_DSL_PORT_ST_NAME        "dsl_port"
#endif
#ifdef CONFIG_SUPPORT_VDSL
#define SAL_WAN_TMP_DSL_LINK_PTM_ST_NAME	"dsl_link_ptm"
#define SAL_WAN_TMP_DSL_LINK_MODE_ST_NAME	"dsl_link_mode"
#endif
#define SAL_WAN_TMP_GPON_IPOE_OPEN_STATE     "gpon_ipoe_open_state"
#define SAL_WAN_TMP_LAST_CONN_ERROR          "last_conn_error"

#define SAL_WAN_TMP_VALUE_MAX_LENGTH	     512
#define SAL_WAN_TMP_PATH_MAX_LENGTH		     256
#define SAL_WAN_TMP_LAST_WANMODE             "last_wanmode"
#ifdef CONFIG_SUPPORT_CGN
#define SAL_WAN_TMP_UPNP_PORT_MAPPING          "upnp_port_mapping"
#endif
#ifdef CONFIG_SUPPORT_AUTO_DETECT
#define SAL_WAN_TMP_CPM_PID                  "cpm_pid"
#endif
#define NVRAN_GET_WAN_FUNC(funcname, name, buffer, bufflen)\
char *funcname(int wan)\
{\
	{\
		static char buf_##buffer[bufflen == 0 ? SAL_WAN_TMP_VALUE_MAX_LENGTH : bufflen];\
		char *p;\
		char wan_nvram_path[bufflen == 0 ? SAL_WAN_TMP_PATH_MAX_LENGTH : bufflen];\
    	snprintf(wan_nvram_path, sizeof(wan_nvram_path), WAN_CFG_BASE"%d/%s", wan, SAL_WAN_TMP_CFG_NAME);\
        buf_##buffer[0] = '\0';\
		p = nvram_get_fun(name, wan_nvram_path);\
		if(p)\
		{\
			snprintf(buf_##buffer, bufflen == 0 ? SAL_WAN_TMP_VALUE_MAX_LENGTH : bufflen, "%s", p);\
			free(p);\
		}\
		return buf_##buffer;\
	}\
}

#define NVRAN_SET_WAN_FUNC(funcname, name, bufflen)\
int funcname(int wan, char *value)\
{\
	{\
		char wan_nvram_path[bufflen == 0 ? SAL_WAN_TMP_PATH_MAX_LENGTH : bufflen];\
		if(!value)\
			return -1;\
    	snprintf(wan_nvram_path, sizeof(wan_nvram_path), WAN_CFG_BASE"%d/%s", wan, SAL_WAN_TMP_CFG_NAME);\
		return nvram_set_p(wan_nvram_path, name, value);\
	}\
}

#define NVRAN_GET_PHY_FUNC(funcname, name, buffer)\
char *funcname(int wan)\
{\
	{\
		static char buf_##buffer[SAL_WAN_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char wan_nvram_path[SAL_WAN_TMP_PATH_MAX_LENGTH];\
    	snprintf(wan_nvram_path, sizeof(wan_nvram_path), WAN_CFG_BASE"%s", SAL_WAN_TMP_CFG_NAME);\
        buf_##buffer[0] = '\0';\
		p = nvram_get_fun(name, wan_nvram_path);\
		if(p)\
		{\
			snprintf(buf_##buffer, SAL_WAN_TMP_VALUE_MAX_LENGTH, "%s", p);\
			free(p);\
		}\
		return buf_##buffer;\
	}\
}

#define NVRAN_SET_PHY_FUNC(funcname, name)\
int funcname(int wan, char *value)\
{\
	{\
		char wan_nvram_path[SAL_WAN_TMP_PATH_MAX_LENGTH];\
		if(!value)\
			return -1;\
    	snprintf(wan_nvram_path, sizeof(wan_nvram_path), WAN_CFG_BASE"%s",  SAL_WAN_TMP_CFG_NAME);\
		return nvram_set_p(wan_nvram_path, name, value);\
	}\
}
	
NVRAN_GET_WAN_FUNC(sal_wan_get_l2_if_name_t, SAL_WAN_TMP_L2_IF_CFG_NAME, l2_ifName, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_l2_if_name_t, SAL_WAN_TMP_L2_IF_CFG_NAME, 0)
#ifdef CONFIG_SUPPORT_AUTO_DETECT
NVRAN_GET_WAN_FUNC(sal_wan_get_l2_if_active_t, SAL_WAN_TMP_L2_IF_ACTIVE, l2_if_active, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_l2_if_active_t, SAL_WAN_TMP_L2_IF_ACTIVE, 0)
#endif
NVRAN_GET_WAN_FUNC(sal_wan_get_con_mode_t, SAL_WAN_TMP_CON_MODE_CFG_NAME, con_mode, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_mode_t, SAL_WAN_TMP_CON_MODE_CFG_NAME, 0)

#ifdef CONFIG_SUPPORT_HA
NVRAN_GET_WAN_FUNC(sal_wan_get_ha_state_t, SAL_WAN_TMP_HA_STATE, ha_state, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ha_state_t, SAL_WAN_TMP_HA_STATE, 0)
#endif
NVRAN_GET_WAN_FUNC(sal_wan_get_con_type_t, SAL_WAN_TMP_CON_TYPE_CFG_NAME, con_type, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_type_t, SAL_WAN_TMP_CON_TYPE_CFG_NAME, 0)
 
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ip_t, SAL_WAN_TMP_IP_ST_NAME, con_ip, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ip_t, SAL_WAN_TMP_IP_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_gw_t, SAL_WAN_TMP_GW_ST_NAME, con_gw, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_gw_t, SAL_WAN_TMP_GW_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ipmask_t, SAL_WAN_TMP_IP_MASK_ST_NAME, con_ipmask, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ipmask_t, SAL_WAN_TMP_IP_MASK_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_dns1_t, SAL_WAN_TMP_DNS1_ST_NAME, con_dns1, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_dns1_t, SAL_WAN_TMP_DNS1_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_dns2_t, SAL_WAN_TMP_DNS2_ST_NAME, con_dns2, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_dns2_t, SAL_WAN_TMP_DNS2_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt12_t, SAL_WAN_TMP_OPT12_ST_NAME, con_opt12, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt12_t, SAL_WAN_TMP_OPT12_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt2_t, SAL_WAN_TMP_OPT2_ST_NAME, con_opt2, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt2_t, SAL_WAN_TMP_OPT2_ST_NAME, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt42_t, SAL_WAN_TMP_OPT42_ST_NAME, con_opt42, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt42_t, SAL_WAN_TMP_OPT42_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt43_t, SAL_WAN_TMP_OPT43_ST_NAME, con_opt43, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt43_t, SAL_WAN_TMP_OPT43_ST_NAME, 0)
#ifdef CONFIG_SUPPORT_FWA
NVRAN_GET_WAN_FUNC(sal_wan_get_fwa_imsi, SAL_WAN_TMP_FWA_IMSI, fwa_imsi, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_fwa_imsi, SAL_WAN_TMP_FWA_IMSI, 0)
#endif
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt120_t, SAL_WAN_TMP_OPT120_ST_NAME, con_opt120, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt120_t, SAL_WAN_TMP_OPT120_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt121_t, SAL_WAN_TMP_OPT121_ST_NAME, con_opt121, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt121_t, SAL_WAN_TMP_OPT121_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt121p_t, SAL_WAN_TMP_OPT121P_ST_NAME, con_opt121p, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt121p_t, SAL_WAN_TMP_OPT121P_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt121n_t, SAL_WAN_TMP_OPT121N_ST_NAME, con_opt121n, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt121n_t, SAL_WAN_TMP_OPT121N_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt249_t, SAL_WAN_TMP_OPT249_ST_NAME, con_opt249, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt249_t, SAL_WAN_TMP_OPT249_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt249p_t, SAL_WAN_TMP_OPT249P_ST_NAME, con_opt249p, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt249p_t, SAL_WAN_TMP_OPT249P_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt249n_t, SAL_WAN_TMP_OPT249N_ST_NAME, con_opt249n, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt249n_t, SAL_WAN_TMP_OPT249N_ST_NAME, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt33_t, SAL_WAN_TMP_OPT33_ST_NAME, con_opt33, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt33_t, SAL_WAN_TMP_OPT33_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt33p_t, SAL_WAN_TMP_OPT33P_ST_NAME, con_opt33p, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt33p_t, SAL_WAN_TMP_OPT33P_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_opt33n_t, SAL_WAN_TMP_OPT33N_ST_NAME, con_opt33n, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_opt33n_t, SAL_WAN_TMP_OPT33N_ST_NAME, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_lease_t, SAL_WAN_TMP_LEASE_ST_NAME, con_leas, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_lease_t, SAL_WAN_TMP_LEASE_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_dhcp_server_t, SAL_WAN_TMP_DHCP_SERVER_ST_NAME, con_dhcp_server, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_dhcp_server_t, SAL_WAN_TMP_DHCP_SERVER_ST_NAME, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_gw_mac_t, SAL_WAN_TMP_GW_MAC_ST_NAME, con_gw_mac, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_gw_mac_t, SAL_WAN_TMP_GW_MAC_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ppp_session_t, SAL_WAN_TMP_PPP_SESSION_ST_NAME, con_session, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ppp_session_t, SAL_WAN_TMP_PPP_SESSION_ST_NAME, 0)
#ifdef CONFIG_PPP_SUPPORT_PADM_PADN
NVRAN_GET_WAN_FUNC(sal_wan_get_con_hurl_t, SAL_WAN_TMP_PPP_TAG_HRUL, con_ppp_hurl, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_hurl_t, SAL_WAN_TMP_PPP_TAG_HRUL, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ppp_ntp1_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP1, con_ppp_ntp1, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ppp_ntp1_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP1, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ppp_ntp2_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP2, con_ppp_ntp1, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ppp_ntp2_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP2, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ppp_ntp3_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP3, con_ppp_ntp1, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ppp_ntp3_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP3, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ppp_ntp4_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP4, con_ppp_ntp1, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ppp_ntp4_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP4, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ppp_ntp5_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP5, con_ppp_ntp1, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ppp_ntp5_t, SAL_WAN_TMP_PPP_TAG_MOTM_NTP5, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_provcode_t, SAL_WAN_TMP_PPP_TAG_MOTM_PROVCODE, con_ppp_provcode, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_provcode_t, SAL_WAN_TMP_PPP_TAG_MOTM_PROVCODE, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ppp_ip_route_add_t, SAL_WAN_TMP_PPP_TAG_IP_ROUTE_ADD, con_ppp_ip_route_add, 2048)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ppp_ip_route_add_t, SAL_WAN_TMP_PPP_TAG_IP_ROUTE_ADD, 0)
#ifdef CONFIG_SUPPORT_IPV6
NVRAN_GET_WAN_FUNC(sal_wan_get_con_motm_mng_ipv6, SAL_WAN_TMP_MOTM_MNG_IPV6, con_motm_mng_ipv6, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_motm_mng_ipv6, SAL_WAN_TMP_MOTM_MNG_IPV6, 0)
#endif
#endif
NVRAN_GET_WAN_FUNC(sal_wan_get_con_ppp_trigger_t, SAL_WAN_TMP_PPP_TRIGGER_ST_NAME, con_trigger, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_ppp_trigger_t, SAL_WAN_TMP_PPP_TRIGGER_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_alive_t, SAL_WAN_TMP_ALIVE_ST_NAME, con_alive, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_alive_t, SAL_WAN_TMP_ALIVE_ST_NAME, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_uptime_t, SAL_WAN_TMP_UPTIME_ST_NAME, con_uptime, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_uptime_t, SAL_WAN_TMP_UPTIME_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_pasttime_t, SAL_WAN_TMP_PASTTIME_ST_NAME, con_pasttime, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_pasttime_t, SAL_WAN_TMP_PASTTIME_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_state_t, SAL_WAN_TMP_STATE_ST_NAME, con_state, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_state_t, SAL_WAN_TMP_STATE_ST_NAME, 0)
#ifdef CONFIG_SUPPORT_WAN_BACKUP
NVRAN_GET_WAN_FUNC(sal_wan_get_no_active_host_time, SAL_WAN_TMP_PPP_IDLE_TIME, ppp_idle, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_no_active_host_time, SAL_WAN_TMP_PPP_IDLE_TIME, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_check_state_t, SAL_WAN_TMP_CHECK_ST_NAME, check_state, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_check_state_t, SAL_WAN_TMP_CHECK_ST_NAME, 0)
#endif
NVRAN_GET_WAN_FUNC(sal_wan_get_con_rstate_t, SAL_WAN_TMP_RSTATE_ST_NAME, con_rstate, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_rstate_t, SAL_WAN_TMP_RSTATE_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_client_pid_t, SAL_WAN_TMP_CLIENT_PID_ST_NAME, con_client_pid, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_client_pid_t, SAL_WAN_TMP_CLIENT_PID_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_auth_failed_t, SAL_WAN_TMP_AUTH_FAILED_ST_NAME, con_auth_failed, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_auth_failed_t, SAL_WAN_TMP_AUTH_FAILED_ST_NAME, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_hw_idle_t, SAL_WAN_TMP_HW_IDLE_TIME_ST_NAME, con_hw_idle, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_hw_idle_t, SAL_WAN_TMP_HW_IDLE_TIME_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_lcp_state_t, SAL_WAN_TMP_PPP_LCP_ST_NAME, con_lcp_state, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_lcp_state_t, SAL_WAN_TMP_PPP_LCP_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_con_last_lcp_time, SAL_WAN_TMP_PPP_LCP_TIME, con_lcp_time, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_last_lcp_time, SAL_WAN_TMP_PPP_LCP_TIME, 0)

NVRAN_GET_PHY_FUNC(sal_wan_get_con_phy_init_t, SAL_WAN_TMP_PHY_INIT_ST_NAME, con_phy_init_state)
NVRAN_SET_PHY_FUNC(sal_wan_set_con_phy_init_t, SAL_WAN_TMP_PHY_INIT_ST_NAME)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_vlanid_t, SAL_WAN_TMP_CON_VLANID, con_vlanid, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_vlanid_t, SAL_WAN_TMP_CON_VLANID, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_bridgeid_t, SAL_WAN_TMP_CON_BRIDGEID, con_bridgeid, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_bridgeid_t, SAL_WAN_TMP_CON_BRIDGEID, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_manual_triggered_t, SAL_WAN_TMP_MANUAL_TRIGGERED, con_triggered, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_manual_triggered_t, SAL_WAN_TMP_MANUAL_TRIGGERED, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_auto_disconnect_t, SAL_WAN_TMP_AUTO_DISCONNECT, auto_disconnect, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_auto_disconnect_t, SAL_WAN_TMP_AUTO_DISCONNECT, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_con_auto_triggered_t, SAL_WAN_TMP_AUTO_TRIGGERED, auto_triggered, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_con_auto_triggered_t, SAL_WAN_TMP_AUTO_TRIGGERED, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_del_obj_t, SAL_WAN_TMP_DEL_OBJ, del_obj, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_del_obj_t, SAL_WAN_TMP_DEL_OBJ, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_del_wan_t, SAL_WAN_TMP_DEL_WAN, del_wan, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_del_wan_t, SAL_WAN_TMP_DEL_WAN, 0)
#ifdef AD1018
NVRAN_GET_WAN_FUNC(sal_wan_get_softirq_preempt_t, SAL_WAN_TMP_SOFTIRQ_PREEMPT, softirq_preempt, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_softirq_preempt_t, SAL_WAN_TMP_SOFTIRQ_PREEMPT, 0)
#endif

#ifdef CONFIG_SUPPORT_DSL
NVRAN_GET_PHY_FUNC(sal_wan_get_con_phy_dsl_t, SAL_WAN_TMP_PHY_INIT_DSL, con_phy_dsl_state)
NVRAN_SET_PHY_FUNC(sal_wan_set_con_phy_dsl_t, SAL_WAN_TMP_PHY_INIT_DSL)
NVRAN_GET_PHY_FUNC(sal_wan_get_con_phy_dsl_uptime_t, SAL_WAN_TMP_PHY_DSL_UPTIME, con_phy_dsl_uptime)
NVRAN_SET_PHY_FUNC(sal_wan_set_con_phy_dsl_uptime_t, SAL_WAN_TMP_PHY_DSL_UPTIME)

NVRAN_GET_WAN_FUNC(sal_wan_get_dsl_link_t, SAL_WAN_TMP_DSL_LINK_ST_NAME, dsl_link, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_dsl_link_t, SAL_WAN_TMP_DSL_LINK_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_dsl_pvc_t, SAL_WAN_TMP_DSL_PVC_ST_NAME, dsl_pvc, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_dsl_pvc_t, SAL_WAN_TMP_DSL_PVC_ST_NAME, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_dsl_port_t, SAL_WAN_TMP_DSL_PORT_ST_NAME, dsl_port, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_dsl_port_t, SAL_WAN_TMP_DSL_PORT_ST_NAME, 0)
#endif
#ifdef CONFIG_SUPPORT_VDSL
NVRAN_GET_WAN_FUNC(sal_wan_get_dsl_link_ptm_t, SAL_WAN_TMP_DSL_LINK_PTM_ST_NAME, dsl_link_ptm, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_dsl_link_ptm_t, SAL_WAN_TMP_DSL_LINK_PTM_ST_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_dsl_link_mode_t, SAL_WAN_TMP_DSL_LINK_MODE_ST_NAME, dsl_link_mode, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_dsl_link_mode_t, SAL_WAN_TMP_DSL_LINK_MODE_ST_NAME, 0)
#endif
/* IPv6 get/set wan configure */
#ifdef CONFIG_SUPPORT_IPV6
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_wan_if_t, SAL_WAN_IPV6_TMP_WAN_IF, wan_if, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_wan_if_t, SAL_WAN_IPV6_TMP_WAN_IF, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_wan_ip_t, SAL_WAN_IPV6_TMP_WAN_IP, wan_ip, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_wan_ip_t, SAL_WAN_IPV6_TMP_WAN_IP, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_prefix_t, SAL_WAN_IPV6_TMP_PREFIX_NAME, wan_prefix, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_prefix_t, SAL_WAN_IPV6_TMP_PREFIX_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_prefix_len_t, SAL_WAN_IPV6_TMP_PREFIX_LENGTH_NAME, wan_prefix_len, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_prefix_len_t, SAL_WAN_IPV6_TMP_PREFIX_LENGTH_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_device_t, SAL_WAN_IPV6_TMP_DEVICE_NAME, wan_device, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_device_t, SAL_WAN_IPV6_TMP_DEVICE_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_prefix_pltime_t, SAL_WAN_IPV6_TMP_PREFIX_PREF_TIME, wan_pltime, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_prefix_pltime_t, SAL_WAN_IPV6_TMP_PREFIX_PREF_TIME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_prefix_vltime_t, SAL_WAN_IPV6_TMP_PREFIX_VALID_TIME, wan_pltime, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_prefix_vltime_t, SAL_WAN_IPV6_TMP_PREFIX_VALID_TIME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_dns_t, SAL_WAN_IPV6_TMP_DNS, wan_dns, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_dns_t, SAL_WAN_IPV6_TMP_DNS, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_lladdr_t, SAL_WAN_IPV6_LLADDR, wan_lladdr, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_lladdr_t, SAL_WAN_IPV6_LLADDR, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_rladdr_t, SAL_WAN_IPV6_RLADDR, wan_rladdr, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_rladdr_t, SAL_WAN_IPV6_RLADDR, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_ipv6cp_state_t, SAL_WAN_IPV6_TMP_IPV6CP_STATE, wan_rladdr, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_ipv6cp_state_t, SAL_WAN_IPV6_TMP_IPV6CP_STATE, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_ntp_t, SAL_WAN_IPV6_TMP_NTP_NAME, wan_ntp, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_ntp_t, SAL_WAN_IPV6_TMP_NTP_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_domain_t, SAL_WAN_IPV6_TMP_DOMAIN_NAME, wan_domain, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_domain_t, SAL_WAN_IPV6_TMP_DOMAIN_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_ula_prefix_t, SAL_WAN_IPV6_TMP_ULA_PREFIX, wan_ula_prefix, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_ula_prefix_t, SAL_WAN_IPV6_TMP_ULA_PREFIX, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_open_state_t, SAL_WAN_TMP_IPV6_OPEN_STATE, ipv6_open_state, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_open_state_t, SAL_WAN_TMP_IPV6_OPEN_STATE, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_sip_name_t, SAL_WAN_IPV6_TMP_SIP_NAME, wan_sip_name, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_sip_name_t, SAL_WAN_IPV6_TMP_SIP_NAME, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_sip_t, SAL_WAN_IPV6_TMP_SIP, wan_sip, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_sip_t, SAL_WAN_IPV6_TMP_SIP, 0)

NVRAN_GET_WAN_FUNC(sal_wan_get_ipv6_gw_exist_t, SAL_WAN_IPV6_DEFAULT_ROUTER_EXIST, wan_ipv6_gw_exist, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_ipv6_gw_exist_t, SAL_WAN_IPV6_DEFAULT_ROUTER_EXIST, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_voip_service_status, SAL_WAN_VOIP_STATUS, wan_voip_status, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_voip_service_status, SAL_WAN_VOIP_STATUS, 0)

NVRAN_GET_PHY_FUNC(sal_wan_get_last_wanmode_t, SAL_WAN_TMP_LAST_WANMODE, last_wanmode)
NVRAN_SET_PHY_FUNC(sal_wan_set_last_wanmode_t, SAL_WAN_TMP_LAST_WANMODE)

int sal_wan_get_sub_prefix_delegation(int wan, char *prefix)
{
    int prelen = 0;
    char *p = NULL;
    sprintf(prefix, "%s", sal_wan_get_ipv6_prefix_t(wan));
    p = strrchr(prefix, ':');
    prelen = atoi(sal_wan_get_ipv6_prefix_len_t(wan));
    if (p && *p && prelen < 64)
    {
        p--; // skip ::
        p--;
        // use first subnet of prefix
        *p = (*p) | 0x1;
        prelen = 64;
    }
    return prelen;
}
#endif   
NVRAN_GET_WAN_FUNC(sal_wan_get_gpon_ipoe_open_state_t, SAL_WAN_TMP_GPON_IPOE_OPEN_STATE, gpon_ipoe_open_state, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_gpon_ipoe_open_state_t, SAL_WAN_TMP_GPON_IPOE_OPEN_STATE, 0)
NVRAN_GET_WAN_FUNC(sal_wan_get_last_conn_error_t, SAL_WAN_TMP_LAST_CONN_ERROR, last_conn_error, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_last_conn_error_t, SAL_WAN_TMP_LAST_CONN_ERROR, 0)
#ifdef CONFIG_SUPPORT_CGN
NVRAN_GET_WAN_FUNC(sal_wan_get_upnp_port_mapping_t, SAL_WAN_TMP_UPNP_PORT_MAPPING, last_conn_error, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_upnp_port_mapping_t, SAL_WAN_TMP_UPNP_PORT_MAPPING, 0)
#endif
#ifdef CONFIG_SUPPORT_AUTO_DETECT
NVRAN_GET_WAN_FUNC(sal_wan_get_cpm_pid_t, SAL_WAN_TMP_CPM_PID, cpm_pid, 0)
NVRAN_SET_WAN_FUNC(sal_wan_set_cpm_pid_t, SAL_WAN_TMP_CPM_PID, 0)
#endif
void sal_wan_store_cp_info_x(int wan, WAN_CP_INFO_t *info)
{
    char *dns1;
    char *dns2;
	char session_number[16];
    {
        sal_wan_set_con_dns1_t(wan, inet_ntoa(info->dns1));
        sal_wan_set_con_dns2_t(wan, inet_ntoa(info->dns2));
    }
	sal_wan_set_con_ip_t(wan, inet_ntoa(info->ip));
	sal_wan_set_con_gw_t(wan, inet_ntoa(info->gw));
	sal_wan_set_con_ipmask_t(wan, inet_ntoa(info->ipmask));
    {
        sal_wan_set_con_lease_t(wan, info->lease_time);
        sal_wan_set_con_opt12_t(wan, info->opt12);
        sal_wan_set_con_opt42_t(wan, info->opt42);
        sal_wan_set_con_opt43_t(wan, info->opt43);
        sal_wan_set_con_opt120_t(wan, info->opt120);
        sal_wan_set_con_opt121_t(wan, info->opt121);
        sal_wan_set_con_opt249_t(wan, info->opt249);
        sal_wan_set_con_opt33_t(wan, info->opt33);
    }
	if(info->ppp_session)
	{
	     snprintf(session_number, sizeof(session_number), "%d", info->ppp_session);
	     sal_wan_set_con_ppp_session_t(wan, session_number);
	}
	sal_wan_set_con_gw_mac_t(wan, info->gw_mac);
	sal_wan_set_con_uptime_t(wan, info->uptime);
	sal_wan_set_con_state_t(wan, info->state);
}
int sal_wan_load_cp_info(int wan, WAN_CP_INFO_t *info)
{
	char *session;
	inet_aton(sal_wan_get_con_ip_t(wan), &info->ip);
	inet_aton(sal_wan_get_con_gw_t(wan), &info->gw);
	inet_aton(sal_wan_get_con_ipmask_t(wan),&info->ipmask);
	inet_aton(sal_wan_get_con_dns1_t(wan), &info->dns1);
	inet_aton(sal_wan_get_con_dns2_t(wan), &info->dns2);
	info->opt43 = sal_wan_get_con_opt43_t(wan);
	info->opt120 = sal_wan_get_con_opt120_t(wan);
	info->opt121 = sal_wan_get_con_opt121_t(wan);
    info->opt249 = sal_wan_get_con_opt249_t(wan);
    info->opt33 = sal_wan_get_con_opt33_t(wan);
	info->lease_time = sal_wan_get_con_lease_t(wan);
	info->gw_mac = sal_wan_get_con_gw_mac_t(wan);
	info->uptime = sal_wan_get_con_uptime_t(wan);
	info->state = sal_wan_get_con_state_t(wan);
	session = sal_wan_get_con_ppp_session_t(wan);
	if(strlen(session))
		info->ppp_session = atoi(sal_wan_get_con_ppp_session_t(wan));
	return 0;
}
#ifdef CONFIG_PPP_SUPPORT_PADM_PADN
void sal_wan_save_ppp_tags(int wan, WAN_PPP_TAG_t *tags)
{
    if(tags)
    {
        if(tags->hurl)
            sal_wan_set_con_hurl_t(wan, tags->hurl);
        if(tags->ntp[0])
            sal_wan_set_con_ppp_ntp1_t(wan, tags->ntp[0]);
        if(tags->ntp[1])
            sal_wan_set_con_ppp_ntp2_t(wan, tags->ntp[1]);
        if(tags->ntp[2])
            sal_wan_set_con_ppp_ntp3_t(wan, tags->ntp[2]);
        if(tags->ntp[3])
            sal_wan_set_con_ppp_ntp4_t(wan, tags->ntp[3]);
        if(tags->ntp[4])
            sal_wan_set_con_ppp_ntp5_t(wan, tags->ntp[4]);
        if(tags->provcode)
            sal_wan_set_con_provcode_t(wan, tags->provcode);
    }
}
void sal_wan_clean_ppp_tags(int wan)
{
    sal_wan_set_con_hurl_t(wan, "");
    sal_wan_set_con_ppp_ntp1_t(wan, "");
    sal_wan_set_con_ppp_ntp2_t(wan, "");
    sal_wan_set_con_ppp_ntp3_t(wan, "");
    sal_wan_set_con_ppp_ntp4_t(wan, "");
    sal_wan_set_con_ppp_ntp5_t(wan, "");
    sal_wan_set_con_provcode_t(wan, "");
    sal_wan_set_con_ppp_ip_route_add_t(wan, "");
}
#endif
