/**
 * @file   sal_wan.h
 * @author Martin_Huang@sdc.sercomm.com
 * @date   2011-09-01
 * @brief  state abstract layer
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 *
 *
 */
#ifndef __SALX_WAN_H__
#define __SALX_WAN_H__

#include <netinet/in.h>
#include <statistics.h>

typedef struct tag_WAN_CP_INFO{
	struct in_addr ip;
	struct in_addr ipmask;
	struct in_addr gw;
	struct in_addr dns1;
	struct in_addr dns2;
	char *uptime;
	char *state;
	char *gw_mac;
	char *opt43;
	char *opt120;
	char *opt121;
    char *opt249;
    char *opt33;
    char *opt42;
	char *opt12;
    char *opt2;
	char *lease_time;
	int ppp_session;
}WAN_CP_INFO_t;

typedef struct tag_WAN_PPP_TAG{
    char *hurl;
    char *ntp[5];
    char *provcode;
    char *ip_route;
}WAN_PPP_TAG_t;
#define    PT_TR069              (1 << 0)
#define    PT_NTP                (1 << 1)
#define    PT_PORTMAP            (1 << 2)
#define    PT_PORTTRIGGER        (1 << 3)
#define    PT_HTTP               (1 << 4)
#define    PT_HTTPS              (1 << 5)
#define    PT_TELNET             (1 << 6)
#define    PT_SSH                (1 << 7)
#define    PT_TR069_D            (1 << 8)
#define    PT_SNMP               (1 << 9)
#define    PT_RTP_D              (1 << 10)
#define    PT_VOIP_D             (1 << 11)
#define    PROTOCOL_TCP          (1 << 0)
#define    PROTOCOL_UDP          (1 << 1)
#define GPON_IPOE_CLOSED    "0"
#define GPON_IPOE_IPV4_OPENED    "1"
#define GPON_IPOE_IPV6_OPENED    "2"
#define GPON_IPOE_BOTH_OPENED    "3"

#define ERROR_NONE    "0"
#define ERROR_IDLE_DISCONNECT    "1"
#define ERROR_AUTHERTICATION_FAILURE    "2"
#define ERROR_IP_CONFIGURATION      "3"

void sal_wan_store_cp_info(int wan, WAN_CP_INFO_t *info);
int sal_wan_load_cp_info(int wan, WAN_CP_INFO_t *info);
char *sal_wan_get_lease_expires(int wan);
char *sal_wan_get_connection_time(int wan);
char *sal_wan_get_lease_remain(int wan);


char *sal_wan_get_con_lcp_state_t(int wan);
int sal_wan_set_con_lcp_state_t(int wan, char *value);
char *sal_wan_get_con_mode_t(int wan);
int sal_wan_set_con_mode_t(int wan, char *value);

char *sal_wan_get_con_type_t(int wan);
int sal_wan_set_con_type_t(int wan, char *value);

char *sal_wan_get_l2_if_name_t(int wan);
int sal_wan_set_l2_if_name_t(int wan, char *value);

char *sal_wan_get_con_gw_mac_t(int wan);
int sal_wan_set_con_gw_mac_t(int wan, char *value);
char *sal_wan_get_con_ppp_session_t(int wan);
int sal_wan_set_con_ppp_session_t(int wan, char *value);
char *sal_wan_get_con_ppp_trigger_t(int wan);
int sal_wan_set_con_ppp_trigger_t(int wan, char *value);

char *sal_wan_get_con_ppp_hurl_t(int wan);
int sal_wan_set_con_ppp_hurl_t(int wan, char *value);
char *sal_wan_get_con_ppp_ntp1_t(int wan);
int sal_wan_set_con_ppp_ntp1_t(int wan, char *value);
char *sal_wan_get_con_ppp_ntp2_t(int wan);
int sal_wan_set_con_ppp_ntp2_t(int wan, char *value);
char *sal_wan_get_con_ppp_ntp3_t(int wan);
int sal_wan_set_con_ppp_ntp3_t(int wan, char *value);
char *sal_wan_get_con_ppp_ntp4_t(int wan);
int sal_wan_set_con_ppp_ntp4_t(int wan, char *value);
char *sal_wan_get_con_ppp_ntp5_t(int wan);
int sal_wan_set_con_ppp_ntp5_t(int wan, char *value);
char *sal_wan_get_con_ppp_provcode_t(int wan);
int sal_wan_set_con_ppp_provcode_t(int wan, char *value);
char *sal_wan_get_con_ppp_ip_route_add_t(int wan);
int sal_wan_set_con_ppp_ip_route_add_t(int wan, char *value);

void sal_wan_save_ppp_tags(int wan, WAN_PPP_TAG_t *tags);
void sal_wan_clean_ppp_tags(int wan);

char *sal_wan_get_con_uptime_t(int wan);
int sal_wan_set_con_uptime_t(int wan, char *value);
char *sal_wan_get_con_pasttime_t(int wan);
int sal_wan_set_con_pasttime_t(int wan, char *value);
char *sal_wan_get_con_state_t(int wan);
int sal_wan_set_con_state_t(int wan, char *value);
int sal_wan_set_con_client_pid_t(int wan, char *value);
char *sal_wan_get_con_client_pid_t(int wan);
int sal_wan_set_con_auth_failed_t(int wan, char *value);
char *sal_wan_get_con_auth_failed_t(int wan);

char *sal_wan_get_con_hw_idle_t(int wan);
int sal_wan_set_con_hw_idle_t(int wan, char *value);

char *sal_wan_get_con_alive_t(int wan);
int sal_wan_set_con_alive_t(int wan, char *value);

char *sal_wan_get_con_ip_t(int wan);
int sal_wan_set_con_ip_t(int wan, char *value);
char *sal_wan_get_con_gw_t(int wan);
int sal_wan_set_con_gw_t(int wan, char *value);
char *sal_wan_get_con_ipmask_t(int wan);
int sal_wan_set_con_ipmask_t(int wan, char *value);
char *sal_wan_get_con_dns1_t(int wan);
int sal_wan_set_con_dns1_t(int wan, char *value);
char *sal_wan_get_con_dns2_t(int wan);
int sal_wan_set_con_dns2_t(int wan, char *value);
char *sal_wan_get_con_opt12_t(int wan);
int sal_wan_set_con_opt12_t(int wan, char *value);
char *sal_wan_get_con_opt42_t(int wan);
int sal_wan_set_con_opt42_t(int wan, char *value);
char *sal_wan_get_con_opt43_t(int wan);
int sal_wan_set_con_opt43_t(int wan, char *value);
#ifdef CONFIG_SUPPORT_FWA
char *sal_wan_get_fwa_imsi(int wan);
int sal_wan_set_fwa_imsi(int wan, char *value);
#endif
char *sal_wan_get_con_opt120_t(int wan);
int sal_wan_set_con_opt120_t(int wan, char *value);
char *sal_wan_get_con_opt121_t(int wan);
int sal_wan_set_con_opt121_t(int wan, char *value);
char *sal_wan_get_con_opt121p_t(int wan);
int sal_wan_set_con_opt121p_t(int wan, char *value);
char *sal_wan_get_con_opt121n_t(int wan);
int sal_wan_set_con_opt121n_t(int wan, char *value);

char *sal_wan_get_con_opt249_t(int wan);
int sal_wan_set_con_opt249_t(int wan, char *value);
char *sal_wan_get_con_opt249p_t(int wan);
int sal_wan_set_con_opt249p_t(int wan, char *value);
char *sal_wan_get_con_opt249n_t(int wan);
int sal_wan_set_con_opt249n_t(int wan, char *value);

char *sal_wan_get_con_opt33_t(int wan);
int sal_wan_set_con_opt33_t(int wan, char *value);
char *sal_wan_get_con_opt33p_t(int wan);
int sal_wan_set_con_opt33p_t(int wan, char *value);
char *sal_wan_get_con_opt33n_t(int wan);
int sal_wan_set_con_opt33n_t(int wan, char *value);
char *sal_wan_get_con_opt2_t(int wan);
int sal_wan_set_con_opt2_t(int wan, char *value);

char *sal_wan_get_con_lease_t(int wan);
int sal_wan_set_con_lease_t(int wan, char *value);
char *sal_wan_get_con_dhcp_server_t(int wan);
int sal_wan_set_con_dhcp_server_t(int wan, char *value);

char *sal_wan_get_con_phy_init_t(int wan);
int sal_wan_set_con_phy_init_t(int wan, char *value);

char *sal_wan_get_con_vlanid_t(int wan);
int sal_wan_set_con_vlanid_t(int wan, char *value);

char *sal_wan_get_con_bridgeid_t(int wan);
int sal_wan_set_con_bridgeid_t(int wan, char *value);

#ifdef CONFIG_SUPPORT_IPV6
char *sal_wan_get_ipv6_wan_if_t(int wan);
int sal_wan_set_ipv6_wan_if_t(int wan, char *value);
char *sal_wan_get_ipv6_wan_ip_t(int wan);
int sal_wan_set_ipv6_wan_ip_t(int wan, char *value);
char *sal_wan_get_ipv6_prefix_t(int wan);
int sal_wan_set_ipv6_prefix_t(int wan, char *value);
char *sal_wan_get_ipv6_prefix_len_t(int wan);
int sal_wan_set_ipv6_prefix_len_t(int wan, char *value);
char *sal_wan_get_ipv6_device_t(int wan);
int sal_wan_set_ipv6_device_t(int wan, char *value);
char *sal_wan_get_ipv6_prefix_pltime_t(int wan);
int sal_wan_set_ipv6_prefix_pltime_t(int wan, char *value);
char *sal_wan_get_ipv6_prefix_vltime_t(int wan);
int sal_wan_set_ipv6_prefix_vltime_t(int wan, char *value);
char *sal_wan_get_ipv6_dns_t(int wan);
int sal_wan_set_ipv6_dns_t(int wan, char *value);

char *sal_wan_get_ipv6_lladdr_t(int wan);
int sal_wan_set_ipv6_lladdr_t(int wan, char *value);
char *sal_wan_get_ipv6_rladdr_t(int wan);
int sal_wan_set_ipv6_rladdr_t(int wan, char *value);
char *sal_wan_get_ipv6_ipv6cp_state_t(int wan);
int sal_wan_set_ipv6_ipv6cp_state_t(int wan, char *value);

char *sal_wan_get_ipv6_ntp_t(int wan);
int sal_wan_set_ipv6_ntp_t(int wan, char *value);
char *sal_wan_get_ipv6_domain_t(int wan);
int sal_wan_set_ipv6_domain_t(int wan, char *value);
char *sal_wan_get_ipv6_ula_prefix_t(int wan);
int sal_wan_set_ipv6_ula_prefix_t(int wan, char *value);

char *sal_wan_get_ipv6_open_state_t(int wan);
int sal_wan_set_ipv6_open_state_t(int wan, char *value);
char *sal_wan_get_ipv6_sip_t(int wan);
int sal_wan_set_ipv6_sip_t(int wan, char *value);
char *sal_wan_get_ipv6_sip_name_t(int wan);
int sal_wan_set_ipv6_sip_name_t(int wan, char *value);
char *sal_wan_get_ipv6_gw_exist_t(int wan);
int sal_wan_set_ipv6_gw_exist_t(int wan, char *value);
int sal_wan_get_sub_prefix_delegation(int wan, char *prefix);
#endif
#ifdef CONFIG_SUPPORT_DSL
#define WAN_DSL_LINK_ON  "1"
#define WAN_DSL_LINK_OFF "0"
#define WAN_DSL_STATUS_SHOWTIME	1
#define WAN_DSL_STATUS_INIT		2
#define WAN_DSL_STATUS_TRAINING	3
#define WAN_DSL_STATUS_IDLE		4
#define WAN_DSL_STATUS_UNAVAILABLE 5

char *sal_wan_get_con_phy_dsl_t(int wan);
int sal_wan_set_con_phy_dsl_t(int wan, char *value);
char *sal_wan_get_con_phy_dsl_uptime_t(int wan);
int sal_wan_set_con_phy_dsl_uptime_t(int wan, char *value);

char *sal_wan_get_dsl_link_t(int wan);
int sal_wan_set_dsl_link_t(int wan, char *value);
int sal_wan_get_dsl_phy_status();
char *sal_wan_get_dsl_pvc_t(int wan);
int sal_wan_set_dsl_pvc_t(int wan, char *value);

char *sal_wan_get_dsl_port_t(int wan);
int sal_wan_set_dsl_port_t(int wan, char *value);
#endif
#ifdef CONFIG_SUPPORT_VDSL
char *sal_wan_get_dsl_link_ptm_t(int wan);
int sal_wan_set_dsl_link_ptm_t(int wan, char *value);
char *sal_wan_get_dsl_link_mode_t(int wan);
int sal_wan_set_dsl_link_mode_t(int wan, char *value);
int sal_wan_get_ptm_link_up_before(int wan_id);
int sal_wan_get_ptm_link_all_down(int wan_id);
#endif
char *sal_wan_get_con_manual_triggered_t(int wan);
int sal_wan_set_con_manual_triggered_t(int wan, char *value);
char *sal_wan_get_con_auto_triggered_t(int wan);
int sal_wan_set_con_auto_triggered_t(int wan, char *value);
char *sal_wan_get_con_auto_disconnect_t(int wan);
int sal_wan_set_con_auto_disconnect_t(int wan, char *value);

char* sal_wan_get_gpon_ipoe_open_state_t(int wan);
int sal_wan_set_gpon_ipoe_open_state_t(int wan, char *value);
char *sal_wan_get_last_conn_error_t(int wan);
int sal_wan_set_last_conn_error_t(int wan, char *value);

char *sal_wan_get_del_obj_t(int wan);
int sal_wan_set_del_obj_t(int wan, char *value);
char *sal_wan_get_del_wan_t(int wan);
int sal_wan_set_del_wan_t(int wan, char *value);

#endif
