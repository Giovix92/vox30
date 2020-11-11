/**
 * @file   cal_wan.h
 * @author Martin_Huang@sdc.sercomm.com
 * @date   2011-09-01
 * @brief
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
#ifndef __CAL_WAN_H__
#define __CAL_WAN_H__

#include <netinet/in.h>
#define DEFAULT_WANDEV_ID	1
#if defined(VOX25) || defined(ESSENTIAL)
#define CAL_WAN_MAX_NUM		16
#else
#ifdef VOX30
#define CAL_WAN_MAX_NUM		32
#else
#define CAL_WAN_MAX_NUM		8
#endif
#endif
#ifdef CONFIG_SUPPORT_VDSL
#define CAL_WAN_LINK_MAX_NUM	5
#else
#define CAL_WAN_LINK_MAX_NUM	4
#endif
#define CAL_WAN_SERVICE_NUM		4

#define WAN_CFG_BASE				"/tmp/0/"
#define CAL_WAN_ACCESS_TYPE_GPON 	"GPON"
#define CAL_WAN_ACCESS_TYPE_ETHER 	"Ethernet"
#define CAL_WAN_ACCESS_TYPE_DSL 	"DSL"
#define CAL_WAN_ACCESS_TYPE_VIB 	"VIB"

#define CAL_WAN_HOST				"Unconfigured"
#define CAL_WAN_ROUTE				"IP_Routed"
#define CAL_WAN_PPPOE_BRIDGE		"PPPoE_Bridged"
#define CAL_WAN_IPOE_BRIDGE			"IP_Bridged"

#define CAL_WAN_CON_MODE_PPPOE		"PPPoE"
#define CAL_WAN_CON_MODE_PPPOA		"PPPoA"
#define CAL_WAN_CON_MODE_PPPOS		"PPPoS"
#define CAL_WAN_CON_MODE_L2TP		"L2TP"
#define CAL_WAN_CON_MODE_STATIC		"Static"
#define CAL_WAN_CON_MODE_DHCP		"DHCP"
#define CAL_WAN_CON_MODE_BRIDGE		"Bridge"

#ifdef CONFIG_SUPPORT_DSL
#define DSL_LINK_TYPE_EOA             "EoA"
#define DSL_LINK_TYPE_IPOA            "IPoA"
#define DSL_LINK_TYPE_IPOE            "IPoE"
#define DSL_LINK_TYPE_PPPOE           "PPPoE"
#define DSL_LINK_TYPE_PPPOA           "PPPoA"
#define DSL_LINK_TYPE_CIP             "CIP"
#define DSL_LINK_TYPE_UNCONF          "Unconfigured"
#endif
#ifdef CONFIG_SUPPORT_VDSL
#define DSL_MODE_PTM             1
#define DSL_MODE_ATM             2
#define DSL_MODE_UNKNOWN         -1
#endif
#define CAL_ENABLE			"1"
#define CAL_DISABLE			"0"
#define CAL_WAN_PPP_DOD			"OnDemand"
#define CAL_WAN_PPP_ALWAYSON		"AlwaysOn"
#define CAL_WAN_PPP_MANUAL		"Manual"

#define CAL_WAN_PPP_AUTH_AUTO	"AUTO"
#define CAL_WAN_PPP_AUTH_PAP	"PAP"
#define CAL_WAN_PPP_AUTH_CHAP	"CHAP"
#define CAL_WAN_PPP_AUTH_MSCHAP	"MS-CHAP"

#ifdef CONFIG_SUPPORT_IPV6
#define PREFIX_ORIGIN_AUTOCONF "AutoConfigured"
#define PREFIX_ORIGIN_DHCPV6 "DHCPv6"
#define PREFIX_ORIGIN_RA "RouterAdvertisement"
#define PREFIX_ORIGIN_WELLKNOWN "WellKnown"
#define PREFIX_ORIGIN_STATIC "Static"

#define CAL_WAN_TMP_DUID_NAME			"dhcp6c_duid"	
#define CAL_WAN_IPV6_ID_DEFAULT     1
#define CAL_LAN_IPV6_ID_DEFAULT     1
#endif

#define CAL_WAN_INVALID_PBIT		"-1"

#ifdef CONFIG_SUPPORT_OPENVSWITCH
#define OVSCTL              "/usr/bin/ovs-vsctl"
#define OFCTL               "/usr/bin/ovs-ofctl"
#else
#define BRCTL               "/usr/sbin/brctl"
#endif
#define IFCONFIG		    "/sbin/ifconfig"
#define VCONFIG				"/sbin/vconfig"
#define DHCPD				"/usr/sbin/udhcpd"
#define KILLALL				"/usr/bin/killall"
#define PPPD				"/usr/sbin/pppd"
#define RC				"/usr/sbin/rc"
#define IPROUTE2_CONFIG			"/usr/sbin/iproute2_config"
#ifdef CONFIG_SUPPORT_L2TP_CLIENT
#define L2TPD                "/usr/sbin/openl2tpd"
#endif
#define DHCPC				"udhcpc"
#ifdef CONFIG_SUPPORT_IPV6
#define DHCP6C              "dhcp6c"
#define DHCP6D				"/usr/sbin/dhcp6c"
#define DHCP6CONF           "/var/dhcpd/dhcp6c.conf"
#endif

#define CAL_WAN_SERVICE_TYPE_DATA	"DATA"
#define CAL_WAN_SERVICE_TYPE_VOIP	"VOIP"
#define CAL_WAN_SERVICE_TYPE_MGNT	"MGNT"
#define CAL_WAN_SERVICE_TYPE_OTHER	"IPTV"
#define CAL_WAN_SERVICE_TYPE_IPTV	"IPTV"

#ifndef SERVICE_DATA

#define SERVICE_DATA		(0x1 << 0)
#define SERVICE_VOIP		(0x1 << 1)
#define SERVICE_MGNT		(0x1 << 2)
#define SERVICE_OTHER		(0x1 << 3)
#define SERVICE_IPTV		(0x1 << 3)
#endif

#define CAL_WAN_ADD_OK                 0
#define CAL_WAN_ADD_DUP_NAME_FAILED    -1
#define CAL_WAN_ADD_DUP_VLAN_FAILED    -2
#define CAL_WAN_ADD_DUP_PVC_FAILED     -3
#define CAL_WAN_ADD_NO_TYPE_FAILED     -4
#define CAL_WAN_ADD_MAX_NUM_FAILED     -5
#define CAL_WAN_ADD_NO_DEV_FAILED      -6
#define CAL_WAN_ADD_SOCKET_FAILED      -7

#ifdef CONFIG_SUPPORT_FWA
#define FWA_DEFAULT_DATA_WANID 14
#define FWA_DEFAULT_VOICE_WANID 15
#endif

typedef struct tag_PPP_CON_PROFILE{
	char *l2_type;
	char *l2_if_name;
	char *enable;
#ifdef CONFIG_SUPPORT_IPV6
	char *ipv6_enable;
#endif
	char *connection_type;
	char *name;
	char *transport_type;
	char *fw_enable;
	char *nat_enable;
	char *user_name;
	char *password;
	char *max_mru;
	char *mac;
	char *ac_name;
    char *auth;
	char *service_name;
	char *connection_trigger;
	char *idle_time;
	char *lcp_echo;
	char *lcp_retry;
	char *ip;
	char *dns_enable;
	char *dns_override;
	char *dns_servers;
	char *if_name;
	char *vlan_id;
	char *tag;
	char *cid;
    int service_type;
} PPP_CON_PROFILE_t;

typedef struct tag_IP_CON_PROFILE{
	char *l2_type;
	char *l2_if_name;
	char *enable;
#ifdef CONFIG_SUPPORT_IPV6
	char *ipv6_enable;
#endif
	char *connection_type;
	char *name;
	char *address_type;
	char *fw_enable;
	char *nat_enable;
	char *ip;
	char *subnetmask;
	char *gateway;
	char *max_mtu;
	char *mac;
	char *dns_enable;
	char *dns_override;
	char *dns_servers;
	char *if_name;
	char *vlan_id;
	char *tag;
	char *cid;
	char *host_name;
	char *client_id;
	char *vendor_id;
	char *authentication;
	char *secret_id;
	char *user_class;
    char *option2_source_ntp;
    char *option42_source_ntp;
    char *option2_en;
    char *option6_en;
	char *option12_en;
	char *option43_en;
	char *option51_en;
	char *option121_en;
    char *option249_en;
    char *option33_en;
	char *option120_en;
    char *option42_en;
	char *option125_en;
	char *option58_en;
	char *option59_en;
    int service_type;
} IP_CON_PROFILE_t;

typedef	struct {
	int path;
	/* adsl mode: ADSL_2plus/ADSL_2/ADSL_G.dmt/ADSL_G.lite/ADSL_G.dmt.bis */
	char *adsl_mode;
	/* CBR/VBR-nrt/VBR-rt/UBR */
	char *atm_qos;
	char *atm_pcr;
	char *atm_scr;
	char *atm_mbs;
	/* STR_IPOA/STR_PPPOE/STR_EOA/STR_PPPOA/STR_IPOA/STR_UNCONF */
	char *dslLinkType;
	int vpi;
	int vci;
	/* LLC/VC */
	char *encap;
	char *mac_nv;
	char *L2_ifname;
    int ptm_pri;
} L2_DSL_PARAMS_t;

typedef	struct {
	char *mac_nv;
	char *L2_ifname;
} L2_ETH_PARAMS_t;

typedef	union {
	L2_DSL_PARAMS_t	dsl;
	L2_ETH_PARAMS_t	eth;
} L2_PARAMS_t;

typedef	union {
	PPP_CON_PROFILE_t ppp;
	IP_CON_PROFILE_t ip;
} L3_PARAMS_t;

struct wanParam_t {
	/* uniqeue id. */
	int idWan;
	/* WANDevice.[0].WanConnectionDevice.[1].PPP/IPConnection.[2]. */
	int idWanL[3];
	/* idWan_org index, which indicate this structure's original copy*/
	//int idWan_org;
	/* PPTP/PPPOE/IPOA/DHCP/BRIDGE/BPA */
	int conType;
	/* ipoa/pppoe/pppoa/dhcp/bridge */
	char *wan_mode;
	/* firewall: wan state */
	int fw_event;
	
	/* inited */
	//int initialized;
	/* wanAcType: DSL/Ethernet */
	char *wanAcType;
	/* enable/disable */
	char *link_enable;
	/* L2 Parameters */
	L2_PARAMS_t	L2;
	/* L3 Parameters */
	L3_PARAMS_t	L3;
};


#define HCAL_WAN_IS_VALID_WAN_ID(wan_id) (((wan_id) >=0 && (wan_id) < CAL_WAN_MAX_NUM) ? 1 : 0)
#define HCAL_WAN_IS_VALID_WAN_PBIT(pbit) (((pbit) >=0 && (pbit) <= 7) ? 1 : 0)
#define HCAL_WAN_IS_VALID_WAN_VLAN(vlan) (((vlan) >=0 && (vlan) <= 4094) ? 1 : 0)

int hcal_wan_map_wandev_id_str_to_uwan_id(char *id_stack_str, int is_ppp);
int hcal_wan_map_cid_to_uwan(int cid);

int hcal_revert_connection(int wan, void *profile, int is_ppp);
#ifdef CONFIG_SUPPORT_IPV6
int hcal_sense_ipv6_profile(int wan);
#endif

int hcal_wan_set_default_con_service(int wan);
/*
 *  success wanid
 *  otherwise -1
 *
 * */
int hcal_wan_get_default_con_service(void);
int hcal_wan_is_data_wan_non_bridge(void);

/*
 *  success wanid
 *  otherwise -1
 *
 * */
int hcal_get_default_wan_id(void);
int hcal_get_default_wan_id_p(void);
int hcal_get_active_voice_wan_id(void);

char *hcal_wan_get_basic_if(int wan);
int hcal_is_wan_if_unUsed(char *ifName);

int hcal_load_ppp_profile(int wan, PPP_CON_PROFILE_t *ppp_con_profile);
int hcal_load_ip_profile(int wan, IP_CON_PROFILE_t *ip_con_profile);


char *hcal_wan_get_name(int wan);
void hcal_wan_set_name(int wan, char *value);

char *hcal_wan_get_connection_name_cached(int wan);

char *hcal_wan_get_enable(int wan);
void hcal_wan_set_enable(int wan, char *value);
#ifdef CONFIG_SUPPORT_IPV6
/* ipv6 start */
char *hcal_wan_get_ipv6_enable(int wan);
void hcal_wan_set_ipv6_enable(int wan, char *value);
char *hcal_wan_get_ula_enable(int wan);
void hcal_wan_set_ula_enable(int wan, char *value);
char *hcal_wan_get_ppp_ipcp_enable(int wan);
void hcal_wan_set_ppp_ipcp_enable(int wan, char *value);
char *hcal_wan_get_ppp_ipv6cp_enable(int wan);
void hcal_wan_set_ppp_ipv6cp_enable(int wan, char *value);
char *hcal_wan_get_ppp_local_id(int wan);
void hcal_wan_set_ppp_local_id(int wan, char *value);
char *hcal_wan_get_ppp_remote_id(int wan);
void hcal_wan_set_ppp_remote_id(int wan, char *value);
char *hcal_wan_get_ip_ipv6_gw(int wan);
void hcal_wan_set_ip_ipv6_gw(int wan, char *value);
char *hcal_wan_get_ip_ipv6_dns(int wan);
void hcal_wan_set_ip_ipv6_dns(int wan, char *value);

/* address */
char *hcal_wan_get_ipv6_address_enable(int wan, char *origin);
char *hcal_wan_get_ipv6_address_ip(int wan, char *origin);
void hcal_wan_set_ipv6_address_ip(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_address_prefix(int wan, char *origin);
void hcal_wan_set_ipv6_address_prefix(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_address_preferred_time(int wan, char *origin);
void hcal_wan_set_ipv6_address_preferred_time(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_address_valid_time(int wan, char *origin);
void hcal_wan_set_ipv6_address_valid_time(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_address_anycast(int wan, char *origin);
void hcal_wan_set_ipv6_address_anycast(int wan, char *origin, char *value);
/* prefix */
char *hcal_wan_get_ipv6_prefix_enable(int wan, char *origin);
char *hcal_wan_get_ipv6_prefix_prefix(int wan, char *origin);
void hcal_wan_set_ipv6_prefix_prefix(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_prefix_type(int wan, char *origin);
void hcal_wan_set_ipv6_prefix_type(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_prefix_pprefix(int wan, char *origin);
void hcal_wan_set_ipv6_prefix_pprefix(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_prefix_cprefix(int wan, char *origin);
void hcal_wan_set_ipv6_prefix_cprefix(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_prefix_onlink(int wan, char *origin);
void hcal_wan_set_ipv6_prefix_onlink(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_prefix_auto(int wan, char *origin);
void hcal_wan_set_ipv6_prefix_auto(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_prefix_preferred_time(int wan, char *origin);
void hcal_wan_set_ipv6_prefix_preferred_time(int wan, char *origin, char *value);
char *hcal_wan_get_ipv6_prefix_valid_time(int wan, char *origin);
void hcal_wan_set_ipv6_prefix_valid_time(int wan, char *origin, char *value);
/* ipv6 end */
#endif
char *hcal_wan_get_nat_enable(int wan);
void hcal_wan_set_nat_enable(int wan, char *value);
char *hcal_wan_get_mode(int wan);
void hcal_wan_set_mode(int wan, char *value);
char *hcal_wan_get_con_type(int wan);
void hcal_wan_set_con_type(int wan, char *value);
char *hcal_wan_get_if_name(int wan);
void hcal_wan_set_if_name(int wan, char *value);
char *hcal_wan_get_mtu(int wan);
void hcal_wan_set_mtu(int wan, char *value);
char *hcal_wan_get_cgn_enable(int wan);
void hcal_wan_set_cgn_enable(int wan, char *value);
char *hcal_wan_get_ip_vendor_id(int wan);
void hcal_wan_set_ip_vendor_id(int wan, char *value);
char *hcal_wan_get_ip_authentication(int wan);
void hcal_wan_set_ip_authentication(int wan, char *value);
char *hcal_wan_get_ip_secret_id(int wan);
void hcal_wan_set_ip_secret_id(int wan, char *value);

char *hcal_wan_get_ppp_service_name(int wan);
void hcal_wan_set_ppp_service_name(int wan, char *value);
char *hcal_wan_get_ppp_ac_name(int wan);
void hcal_wan_set_ppp_ac_name(int wan, char *value);
char *hcal_wan_get_ppp_trigger(int wan);
void hcal_wan_set_ppp_trigger(int wan, char *value);
char *hcal_wan_get_ppp_idle_time(int wan);
void hcal_wan_set_ppp_idle_time(int wan, char *value);
char *hcal_wan_get_ppp_user_name(int wan);
void hcal_wan_set_ppp_user_name(int wan, char *value);
char *hcal_wan_get_ppp_password(int wan);
void hcal_wan_set_ppp_password(int wan, char *value);
char *hcal_wan_get_ppp_auth(int wan);
void hcal_wan_set_ppp_auth(int wan, char *value);
#ifdef CONFIG_SUPPORT_FWA
int hcal_wan_get_fwa_wanid(void);
int hcal_wan_get_fwa_voice_wanid(void);
char* hcal_wan_get_conflict_wan_list(int wan_id);
#endif
#ifdef CONFIG_SUPPORT_3G
int hcal_get_3g_dongle_wanid(void);
int hcal_get_3g_dongle_voice_wanid();
char *hcal_wan_get_cfg_apn_name(int wan);
void hcal_wan_set_cfg_apn_name(int wan, char *value);
char *hcal_wan_get_cfg_4g_apn_name(int wan);
void hcal_wan_set_cfg_4g_apn_name(int wan, char *value);
#endif
char *hcal_wan_get_ppp_lcp_echo(int wan);
void hcal_wan_set_ppp_lcp_echo(int wan, char *value);
char *hcal_wan_get_ppp_lcp_retry(int wan);
void hcal_wan_set_ppp_lcp_retry(int wan, char *value);

char *hcal_wan_get_ip_addr(int wan);
void hcal_wan_set_ip_addr(int wan, char *value);
char *hcal_wan_get_gateway(int wan);
void hcal_wan_set_gateway(int wan, char *value);

char *hcal_wan_get_ip_net_mask(int wan);
void hcal_wan_set_ip_net_mask(int wan, char *value);
char *hcal_wan_get_ip_host_name(int wan);
void hcal_wan_set_ip_host_name(int wan, char *value);

char *hcal_wan_get_ip_client_id(int wan);
void hcal_wan_set_ip_client_id(int wan, char *value);

char *hcal_wan_get_ip_user_class(int wan);
void hcal_wan_set_ip_user_class(int wan, char *value);

char *hcal_wan_get_dns_enable(int wan);
void hcal_wan_set_dns_enable(int wan, char *value);
char *hcal_wan_get_dns_override(int wan);
void hcal_wan_set_dns_override(int wan, char *value);
#ifdef CONFIG_SUPPORT_SECURE_NET
char *hcal_wan_get_dns_override_lastset(int wan);
void hcal_wan_set_dns_override_lastset(int wan, char *value);
#endif


char *hcal_wan_get_general_name(int wan);
char* hcal_wan_get_general_name_list(void);
char *hcal_wan_get_dns_servers(int wan);
void hcal_wan_set_dns_servers(int wan, char *value);
char *hcal_wan_get_mac_addr(int wan);
void hcal_wan_set_mac_addr(int wan, char *value);
char *hcal_wan_get_termination_vlanid(int wan);
void hcal_wan_set_termination_vlanid(int wan, char *value);

char *hcal_wan_get_dscp(int wan);
void hcal_wan_set_dscp(int wan, char *value);

char *hcal_wan_get_ip_option2_source_ntp(int wan);
void hcal_wan_set_ip_option2_source_ntp(int wan, char *value);

char *hcal_wan_get_ip_option42_source_ntp(int wan);
void hcal_wan_set_ip_option42_source_ntp(int wan, char *value);

char *hcal_wan_get_ip_option2_enable(int wan);
void hcal_wan_set_ip_option2_enable(int wan, char *value);

char *hcal_wan_get_ip_option42_enable(int wan);
void hcal_wan_set_ip_option42_enable(int wan, char *value);

char *hcal_wan_get_ip_option43_enable(int wan);
void hcal_wan_set_ip_option43_enable(int wan, char *value);
char *hcal_wan_get_ip_option12_enable(int wan);
void hcal_wan_set_ip_option12_enable(int wan, char *value);

char *hcal_wan_get_ip_option120_enable(int wan);
void hcal_wan_set_ip_option120_enable(int wan, char *value);
char *hcal_wan_get_ip_option121_enable(int wan);
void hcal_wan_set_ip_option121_enable(int wan, char *value);
char *hcal_wan_get_ip_option249_enable(int wan);
void hcal_wan_set_ip_option249_enable(int wan, char *value);
char *hcal_wan_get_ip_option33_enable(int wan);
void hcal_wan_set_ip_option33_enable(int wan, char *value);
char *hcal_wan_get_ip_option6_enable(int wan);
void hcal_wan_set_ip_option6_enable(int wan, char *value);
char *hcal_wan_get_ip_option51_enable(int wan);
void hcal_wan_set_ip_option51_enable(int wan, char *value);
char *hcal_wan_get_ip_option125_enable(int wan);
void hcal_wan_set_ip_option125_enable(int wan, char *value);
char *hcal_wan_get_ip_option58_enable(int wan);
void hcal_wan_set_ip_option58_enable(int wan, char *value);
char *hcal_wan_get_ip_option59_enable(int wan);
void hcal_wan_set_ip_option59_enable(int wan, char *value);

char *hcal_wan_get_port_mirror_enable(int wan);
void hcal_wan_set_port_mirror_enable(int wan, char *value);
char *hcal_wan_get_port_mirror_interface(int wan);
char *hcal_wan_get_wizard_password_enable(int wan);
void hcal_wan_set_wizard_password_enable(int wan, char *value);
void hcal_wan_set_port_mirror_interface(int wan, char *value);
#ifdef CONFIG_SUPPORT_WAN_BACKUP
char *hcal_wan_get_backup_wan(int wan);
void hcal_wan_set_backup_wan(int wan, char *value);
char *hcal_wan_get_backup_priority(int wan);
void hcal_wan_set_backup_priority(int wan, char *value);
#define CAL_WAN_CON_CHECK_NIP   "NIP"
#define CAL_WAN_CON_CHECK_NLINE "NLINE"
#define CAL_WAN_CON_CHECK_NGW   "NGW"
#define CAL_WAN_CON_CHECK_NDNS  "NDNS"
#define CAL_WAN_CON_CHECK_NSRV  "NSRV"

char *hcal_wan_get_backup_connection_check(int wan);
char *hcal_wan_get_backup_lookup_domain(int wan);
void hcal_wan_set_backup_connection_check(int wan, char *value);
void hcal_wan_set_backup_lookup_domain(int wan, char *value);
int hcal_wan_is_backup(int cid);
#endif
int hcal_wan_get_service_list(int wan);
char *hcal_wan_get_service_name_string(int service);

char *hcal_wan_get_id(int wan);
void hcal_wan_set_id(int wan, char *value);

char *hcal_wan_get_ppp_max_auth(int wan);
void hcal_wan_set_ppp_max_auth(int wan, char *value);

char *hcal_wan_get_ppp_idle_time(int wan);
void hcal_wan_set_ppp_idle_time(int wan, char *value);

char *hcal_wan_get_access_type(int wan);
char *hcal_wan_get_shaping_rate(int wan);
void hcal_wan_set_shaping_rate(int wan, char *value);
char *hcal_wan_get_shaping_burstsize(int wan);
void hcal_wan_set_shaping_burstsize(int wan, char *value);

char *hcal_wan_get_maxbit_rate(int wan);
char *hcal_wan_get_duplex_mode(int wan);
//void hcal_wan_set_access_type(int wan, char *value);
char *hcal_wan_get_marking_pbit(int wan);
void hcal_wan_set_marking_pbit(int wan, char *value);
char *hcal_wan_get_arpping_enable(int wan);
void hcal_wan_set_arpping_enable(int wan, char *value);
char *hcal_wan_get_arpping_timeout(int wan);
void hcal_wan_set_arpping_timeout(int wan, char *value);
char *hcal_wan_get_arpping_maxfail(int wan);
void hcal_wan_set_arpping_maxfail(int wan, char *value);
 

char *cal_wan_get_connection_uri(void *id, int is_ppp);
char* hcal_wan_id_map_uri(int wan_id);
int hcal_wan_uri_map_id(char* value);
int hcal_wan_pattern_xxx_map_id(char *value);
int hcal_wan_pattern_map_id(char *value);

char* hcal_list_visible_wan_name(int serviceType);
char* hcal_list_visible_non_bridge_wan_name(int serviceType);

char *hcal_wan_gpon_get_serial_number(int wan_id);
char *hcal_wan_gpon_get_password(int wan_id);
char *hcal_wan_gpon_get_default_state(int wan_id);
int hcal_wan_gpon_set_serial_number(int wan_id, char *value);
int hcal_wan_gpon_set_password(int wan_id, char *value);
int hcal_wan_gpon_set_password_force(int wan_id, char *value);
int hcal_wan_gpon_set_default_state(int wan_id, char *value);

int hcal_wan_get_data_wan_id(void);
int hcal_wan_get_voip_wan_id(void);
int hcal_wan_get_num(void);
int hcal_wan_check_exist_by_wanid(int wanID);
int hcal_get_wanid_by_vendor_id(char * vendor);
void hcal_wan_add_object_hook(char *uri);
void hcal_wan_del_object_hook(char *uri);
void hcal_wan_pre_del_object_hook(char *uri);
int hcal_wan_add_link_object(char* access_type, char* tag, char* name, char* conn_mode, int is_ptm);
int hcal_wan_del_link_object(char* name);
int hcal_wan_set_value_hook(char* uri);
char* hcal_wan_get_wan_host_name(void);
int hcal_wan_get_first_phy_wan_id(char *phy);
int hcal_wan_set_wanParams(struct wanParam_t *wpar);
int hcal_wan_add_wanParams(struct wanParam_t *wpar);
#ifdef CONFIG_SUPPORT_DSL
int hcal_load_dsl_wanParams(int wan, struct wanParam_t *wpar);
char* hcal_wan_get_adsl_mode(int wan);
char *hcal_wan_get_dsl_link_en(int wan_id);
int hcal_wan_set_dsl_link_en(int wan_id, char *value);
char *hcal_wan_get_dsl_link_type(int wan_id);
int hcal_wan_set_dsl_link_type(int wan_id, char *value);
char *hcal_wan_get_dsl_link_dest_addr(int wan_id);
void hcal_wan_set_dsl_link_dest_addr(int wan_id, char *value);
char *hcal_wan_get_dsl_link_atm_encap(int wan_id);
void hcal_wan_set_dsl_link_atm_encap(int wan_id, char *value);
char *hcal_wan_get_dsl_link_atm_qos(int wan_id);
void hcal_wan_set_dsl_link_atm_qos(int wan_id, char *value);
char *hcal_wan_get_dsl_link_atm_pcr(int wan_id);
void hcal_wan_set_dsl_link_atm_pcr(int wan_id, char *value);
char *hcal_wan_get_dsl_link_atm_scr(int wan_id);
void hcal_wan_set_dsl_link_atm_scr(int wan_id, char *value);
char *hcal_wan_get_dsl_link_atm_mbs(int wan_id);
void hcal_wan_set_dsl_link_atm_mbs(int wan_id, char *value);

int hcal_wan_get_wan_id_list(char *id_stack_str, char *list);
#endif
int hcal_wan_get_dsl_wan(void);
char* hcal_wan_get_access_type_by_uri(char *uri);
char *hcal_wan_get_dsl_standard_supported(int wan_id);
int hcal_wan_set_dsl_standard_supported(int wan_id, char *value);
char *hcal_wan_get_dsl_bitswap(int wan_id);
int hcal_wan_set_dsl_bitswap(int wan_id, char *value);
char *hcal_wan_get_dsl_sra(int wan_id);
int hcal_wan_set_dsl_sra(int wan_id, char *value);
char *hcal_wan_get_dsl_allow_prof(int wan_id);
int hcal_wan_set_dsl_allow_prof(int wan_id, char *value);
char *hcal_wan_get_dsl_us0_en(int wan_id);
int hcal_wan_set_dsl_us0_en(int wan_id, char *value);
char *hcal_wan_get_dsl_dpath(int wan_id);
int hcal_wan_set_dsl_dpath(int wan_id, char *value);
#ifdef CONFIG_SUPPORT_VDSL
char *hcal_wan_get_dsl_link_ptm_en(int wan);
void hcal_wan_set_dsl_link_ptm_en(int wan, char *value);
char *hcal_wan_get_dsl_link_ptm_prio(int wan);
void hcal_wan_set_dsl_link_ptm_prio(int wan, char *value);
int hcal_wan_get_dsl_is_ptm_mode(int wan_id);
#endif
char *hcal_wan_get_firewall_enable(int wan);
void hcal_wan_set_firewall_enable(int wan, char *value);
char *hcal_wan_get_icmp_timer(int wan);
void hcal_wan_set_icmp_timer(int wan, char *value);
char *hcal_wan_get_icmp_times(int wan);
void hcal_wan_set_icmp_times(int wan, char *value);

int hcal_wan_get_the_max_connection_id(void);
int hcal_wan_get_cid_num(int* cid_array);
#ifdef CONFIG_SUPPORT_IPV6
int hcal_wan_get_ipv6_client_num(int* array);
int hcal_wan_map_client_id_to_uwan_id(char *id_str);
#endif
/*fix me */
#define DATA_WAN_VID hcal_wan_get_termination_vlanid(0)
#define IPTV_UNI_VID hcal_wan_get_termination_vlanid(3)
#define IPTV_STREAM_VID  99
#define UNUSED_VLAN    4094

#define GPON_PWD_SET_UNDO        (-99)
#ifdef CONFIG_SUPPORT_CGN
int hcal_wan_get_services_affected_bycgn(int wan);
#endif

#ifdef CONFIG_SUPPORT_PER_WAN_DOS
char *hcal_dos_get_enable(int wan);
void hcal_dos_set_enable(int wan, char *value);
char *hcal_dos_get_icmp_smurf(int wan);
void hcal_dos_set_icmp_smurf(int wan, char *value);
char *hcal_dos_get_port_scan(int wan);
void hcal_dos_set_port_scan(int wan, char *value);
char *hcal_dos_get_pingofdeath(int wan);
void hcal_dos_set_pingofdeath(int wan, char *value);
char *hcal_dos_get_icmp_flood(int wan);
void hcal_dos_set_icmp_flood(int wan, char *value);
char *hcal_dos_get_icmp_flood_speed(int wan);
void hcal_dos_set_icmp_flood_speed(int wan, char *value);
char *hcal_dos_get_icmp_flood_per_ip(int wan);
void hcal_dos_set_icmp_flood_per_ip(int wan, char *value);
char *hcal_dos_get_icmp_flood_per_ip_speed(int wan);
void hcal_dos_set_icmp_flood_per_ip_speed(int wan, char *value);
char *hcal_dos_get_tcp_syn_flood(int wan);
void hcal_dos_set_tcp_syn_flood(int wan, char *value);
char *hcal_dos_get_tcp_syn_flood_speed(int wan);
void hcal_dos_set_tcp_syn_flood_speed(int wan, char *value);
char *hcal_dos_get_tcp_syn_flood_per_ip(int wan);
void hcal_dos_set_tcp_syn_flood_per_ip(int wan, char *value);
char *hcal_dos_get_tcp_syn_flood_per_ip_speed(int wan);
void hcal_dos_set_tcp_syn_flood_per_ip_speed(int wan, char *value);
char *hcal_dos_get_tcp_fin_flood(int wan);
void hcal_dos_set_tcp_fin_flood(int wan, char *value);
char *hcal_dos_get_tcp_fin_flood_speed(int wan);
void hcal_dos_set_tcp_fin_flood_speed(int wan, char *value);
char *hcal_dos_get_tcp_fin_flood_per_ip(int wan);
void hcal_dos_set_tcp_fin_flood_per_ip(int wan, char *value);
char *hcal_dos_get_tcp_fin_flood_per_ip_speed(int wan);
void hcal_dos_set_tcp_fin_flood_per_ip_speed(int wan, char *value);
char *hcal_dos_get_udp_flood(int wan);
void hcal_dos_set_udp_flood(int wan, char *value);
char *hcal_dos_get_udp_flood_speed(int wan);
void hcal_dos_set_udp_flood_speed(int wan, char *value);
char *hcal_dos_get_udp_flood_per_ip(int wan);
void hcal_dos_set_udp_flood_per_ip(int wan, char *value);
char *hcal_dos_get_udp_flood_per_ip_speed(int wan);
void hcal_dos_set_udp_flood_per_ip_speed(int wan, char *value);
char *hcal_dos_get_frghdr(int wan);
void hcal_dos_set_frghdr(int wan, char *value);
#endif
#ifdef CONFIG_SUPPORT_AUTO_DETECT
int cal_wan_get_auto_detect_delay();
int cal_wan_set_auto_detect_delay(char *value);
char *cal_wan_get_auto_detect_order();
int cal_wan_set_auto_detect_order(char *value);
int hcal_wan_get_parter_index(int wan);
#endif
#ifdef CONFIG_SUPPORT_CGN
int cal_wan_set_cgn_notificationdelay(char *value);
int cal_wan_get_cgn_notificationdelay(void);
int cal_wan_set_cgn_notificationretrytimer(char *value);
int cal_wan_get_cgn_notificationretrytimer(void);
int cal_wan_set_cgn_retry(char *value);
int cal_wan_get_cgn_retry(void);
int cal_wan_set_cgn_threshold(char *value);
int cal_wan_get_cgn_threshold(void);
#endif
#ifdef CONFIG_SUPPORT_SECURE_NET
char *hcal_wan_get_dnscache_enable(int wan);
void hcal_wan_set_dnscache_enable(int wan, char *value);
char *hcal_wan_get_dnscache_size(int wan);
void hcal_wan_set_dnscache_size(int wan, char *value);
char *hcal_wan_get_dnscache_flush(int wan);
void hcal_wan_set_dnscache_flush(int wan, char *value);
#endif
#endif
