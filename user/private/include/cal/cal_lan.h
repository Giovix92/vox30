
#ifndef _CAL_LAN_H_
#define _CAL_LAN_H_

typedef enum
{
    CAL_LAN_DEVICE_ID_DEFAULT = 1,
    CAL_LAN_DEVICE_ID_MAX = 2,
} CAL_LAN_DEVICE_ID;

typedef enum
{
    CAL_LAN_SUBNET_ID_MAIN = 0,
    CAL_LAN_SUBNET_ID_2,
    CAL_LAN_SUBNET_ID_3,
    CAL_LAN_SUBNET_ID_4,
    CAL_LAN_SUBNET_ID_5,
    CAL_LAN_SUBNET_ID_END
}CAL_LAN_SUBNET_ID;

typedef enum
{
    CAL_LAN_DHCP_OPTION_ID_6 = 6,
    CAL_LAN_DHCP_OPTION_ID_12 = 12,
    CAL_LAN_DHCP_OPTION_ID_15 = 15,
    CAL_LAN_DHCP_OPTION_ID_42 = 42,
    CAL_LAN_DHCP_OPTION_ID_43 = 43,
    CAL_LAN_DHCP_OPTION_ID_58 = 58,
    CAL_LAN_DHCP_OPTION_ID_59 = 59,
    CAL_LAN_DHCP_OPTION_ID_66 = 66,
    CAL_LAN_DHCP_OPTION_ID_67 = 67,
    CAL_LAN_DHCP_OPTION_ID_114 = 114,
    CAL_LAN_DHCP_OPTION_ID_120 = 120,
    CAL_LAN_DHCP_OPTION_ID_125 = 125,
    CAL_LAN_DHCP_OPTION_ID_150 = 150,
    CAL_LAN_DHCP_OPTION_ID_160 = 160,
    CAL_LAN_DHCP_OPTION_ID_240 = 240,
    CAL_LAN_DHCP_OPTION_ID_241 = 241,
    CAL_LAN_DHCP_OPTION_ID_242 = 242,
    CAL_LAN_DHCP_OPTION_ID_243 = 243,
    CAL_LAN_DHCP_OPTION_ID_244 = 244,
    CAL_LAN_DHCP_OPTION_ID_245 = 245,
    CAL_LAN_DHCP_OPTION_ID_246 = 246,
    CAL_LAN_DHCP_OPTION_ID_247 = 247,
    CAL_LAN_DHCP_OPTION_ID_248 = 248,
    CAL_LAN_DHCP_OPTION_ID_250 = 250,
    CAL_LAN_DHCP_OPTION_ID_END
}CAL_LAN_DHCP_OPTION_ID;
#ifdef CONFIG_SUPPORT_IPV6
#define ULA_AUTOCONFIGURED          "AutoConfigured"
#define MANUAL_PREFIX_STATIC        "Static"
#define DEFAULT_IPINTERFACE         1
#endif
#define CAL_LAN_CONFIG_LEN_MAX 1024
#define CAL_LAN_MAX_NUM 4
#define CAL_LAN_ETHER_IF_NAME_PREFIX   "eth"
#define MAX_HOST_NUM  256

/*****************************************************************************

Function: cal_lan_get_xxx()
          Return: 1) NULL -- get error
                  2) A static buffer pointer, which include the value string.

Function: cal_lan_set_xxx()
          Return: 1) 0  -- success
                  2) !0 -- fail
                  
Parameters:
          int lanDev_id      -- defined in enum CAL_LAN_DEVICE_ID
                           
          int lanSub_id      -- defined in enum CAL_LAN_SUBNET_ID
          
          int dhcpTag_id     -- defined in enum CAL_LAN_DHCP_OPTION_ID
          
          int dhcpStatic_id  -- range: 1 to atoi(cal_lan_get_dhcp_static_address_num(...))
          
******************************************************************************/
int cal_lan_get_matched_object_id(int lanDev_id, int lanSub_id);
void cal_lan_check_if_object_and_add(int lanDev_id, int lanSub_id);
int cal_lan_map_sub_id_to_object_id(int lanSub_id);
char *cal_lan_get_enable(int lanDev_id, int lanSub_id);
int   cal_lan_set_enable(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_if_name(int lanDev_id, int lanSub_id);
int   cal_lan_set_if_name(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_ip_address(int lanDev_id, int lanSub_id);
int   cal_lan_set_ip_address(int lanDev_id, int lanSub_id, char *value);
int   cal_lan_set_guest_ip_address(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_subnet_mask(int lanDev_id, int lanSub_id);
int   cal_lan_set_subnet_mask(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_dns_servers(int lanDev_id, int lanSub_id);
int   cal_lan_set_dns_servers(int lanDev_id,int lanSub_id, char *value);
char *cal_lan_get_dns_proxy_enable(int lanDev_id, int lanSub_id);
int cal_lan_set_dns_proxy_enable(int lanDev_id,int lanSub_id, char *value);
char *cal_lan_get_ip_routers(int lanDev_id, int lanSub_id);
int  *cal_lan_set_ip_routers(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_dhcp_server_button_visible();
char *cal_lan_get_dhcp_server_enable(int lanDev_id, int lanSub_id);
int   cal_lan_set_dhcp_server_enable(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_dhcp_start_ip(int lanDev_id, int lanSub_id);
int   cal_lan_set_dhcp_start_ip(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_dhcp_end_ip(int lanDev_id, int lanSub_id);
int   cal_lan_set_dhcp_end_ip(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_dhcp_lease_time(int lanDev_id, int lanSub_id);
int   cal_lan_set_dhcp_lease_time(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_rip_version(int lanDev_id, int lanSub_id);
int   cal_lan_set_rip_version(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_rip_direction(int lanDev_id, int lanSub_id);
int   cal_lan_set_rip_direction(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_dhcp_option_tag(int lanDev_id, int lanSub_id, int dhcpTag_id);
char *cal_lan_get_dhcp_option_value(int lanDev_id, int lanSub_id, int dhcpTag_id);
char *cal_lan_get_dhcp_option_value_encoded(int lanDev_id, int lanSub_id, int dhcpTag_id);
int   cal_lan_set_dhcp_option_value(int lanDev_id, int lanSub_id, int dhcpTag_id, char *value);

char *cal_lan_get_conserv_vlanqosenable_ipphone(int lanDev_id);
int   cal_lan_set_conserv_vlanqosenable_ipphone(int lanDev_id,char *value);
char *cal_lan_get_conserv_vlan_ipphone(int lanDev_id);
int   cal_lan_set_conserv_vlan_ipphone(int lanDev_id,char *value);

char *cal_lan_get_conserv_enable(int lanDev_id, int lanSub_id);
int   cal_lan_set_conserv_enable(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_conserv_source_interface(int lanDev_id, int lanSub_id);
int   cal_lan_set_conserv_source_interface(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_conserv_vendor_id(int lanDev_id, int lanSub_id);
int   cal_lan_set_conserv_vendor_id(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_conserv_mac(int lanDev_id, int lanSub_id);
int   cal_lan_set_conserv_mac(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_conserv_host(int lanDev_id, int lanSub_id);
int   cal_lan_set_conserv_host(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_conserv_if_key(int lanDev_id, int lanSub_id);
int   cal_lan_set_conserv_if_key(int lanDev_id, int lanSub_id, char *value);

char *cal_lan_get_dhcp_static_address_num(int lanDev_id, int lanSub_id);
char *cal_lan_get_dhcp_static_address_id(int lanDev_id, int lanSub_id, int dhcpStatic_id);
int   cal_lan_set_dhcp_static_address_id(int lanDev_id, int lanSub_id, int dhcpStatic_id, char *value);
char *cal_lan_get_dhcp_static_address_device_name(int lanDev_id, int lanSub_id, int dhcpStatic_id);
int   cal_lan_set_dhcp_static_address_device_name(int lanDev_id, int lanSub_id, int dhcpStatic_id, char *value);
char *cal_lan_get_dhcp_static_address_mac(int lanDev_id, int lanSub_id, int dhcpStatic_id);
int   cal_lan_set_dhcp_static_address_mac(int lanDev_id, int lanSub_id, int dhcpStatic_id, char *value);
char *cal_lan_get_dhcp_static_address_ip(int lanDev_id, int lanSub_id, int dhcpStatic_id);
int   cal_lan_set_dhcp_static_address_ip(int lanDev_id, int lanSub_id, int dhcpStatic_id, char *value);
int   cal_lan_add_dhcp_static_address_entry(int lanDev_id, int lanSub_id, int* dhcpStatic_id);
int   cal_lan_del_dhcp_static_address_entry(int lanDev_id, int lanSub_id, int dhcpStatic_id);
int cal_lan_get_dhcp_static_address_array(int **array, int lanDev_id, int lanSub_id);
char *cal_lan_get_dhcpd_lease_file_name(int lanDev_id, int lanSub_id);

/* Instead of old api */
int cal_lan_get_ip_inet_addr(int lanSub_id);
int cal_lan_get_netmask_inet_addr(int lanSub_id);


/* lan hosts */
char *cal_lan_get_host_num(int lanDev_id);
char *cal_lan_get_host_ip_address(int lanDev_id, int host_id);
int   cal_lan_set_host_ip_address(int lanDev_id, int host_id, char *value);
char *cal_lan_get_host_address_source(int lanDev_id, int host_id);
int   cal_lan_set_host_address_source(int lanDev_id, int host_id, char *value);
char *cal_lan_get_host_lease_time_remaining(int lanDev_id, int host_id);
int   cal_lan_set_host_lease_time_remaining(int lanDev_id, int host_id, char *value);
char *cal_lan_get_host_mac_address(int lanDev_id, int host_id);
int   cal_lan_set_host_mac_address(int lanDev_id, int host_id, char *value);
char *cal_lan_get_host_name(int lanDev_id, int host_id);
int   cal_lan_set_host_name(int lanDev_id, int host_id, char *value);
char *cal_lan_get_host_active(int lanDev_id, int host_id);
int   cal_lan_set_host_active(int lanDev_id, int host_id, char *value);
char *cal_lan_get_host_host_type(int lanDev_id, int host_id);
int cal_lan_set_host_host_type(int lanDev_id, int host_id, char *value);
char *cal_lan_get_host_shared_state(int lanDev_id, int host_id);
int cal_lan_set_host_shared_state(int lanDev_id, int host_id, char *value);
int   cal_lan_add_host_entry(int lanDev_id, int *host_id);
int   cal_lan_del_host_entry(int lanDev_id, int host_id);
char *cal_lan_get_host_layer2_interface(int lanDev_id, int host_id);
int cal_lan_set_host_layer2_interface(int lanDev_id, int host_id, char *value);
char *cal_lan_get_host_interface_type(int lanDev_id, int host_id);
int cal_lan_set_host_interface_type(int lanDev_id, int host_id, char *value);
/*lan alias host*/
int cal_lan_get_alias_host_array(int **array);
char *cal_lan_get_alias_host_mac(int lanDev_id, int host_id);
int cal_lan_set_alias_host_mac(int lanDev_id, int host_id, char *value);
char *cal_lan_get_alias_host_name(int lanDev_id, int host_id);
int cal_lan_set_alias_host_name(int lanDev_id, int host_id, char *value);
char *cal_lan_get_alias_host_type(int lanDev_id, int host_id);
int cal_lan_set_alias_host_type(int lanDev_id, int host_id, char *value);
int cal_lan_add_alias_host_entry(int lanDev_id, int *host_id);
int cal_lan_del_alias_host_entry(int lanDev_id, int host_id);
int cal_lan_host_update_mac(char *mac);
int cal_lan_host_update_username(unsigned char *mac, char *name);
int cal_lan_host_update_usertype(unsigned char *mac, int type);
/*lan ports*/
typedef struct tag_LAN_PORTS_PARA{
    char enable;
    char multicast_vid_list[512];
}LAN_PORTS_PARA_t;
#if 0
int cal_lan_get_ports_paras(int port_num, LAN_PORTS_PARA_t *paras);
#endif
char *cal_lan_get_port_enable(int lanDev_id, int port_id);
int cal_lan_set_port_enable(int lanDev_id, int port_id, char *value);
char *cal_lan_get_port_multicast_vid_list(int lanDev_id, int port_id);
int cal_lan_set_port_multicast_vid_list(int lanDev_id, int port_id, char *value);
#ifdef CONFIG_SUPPORT_L2BRIDGING
char *hcal_lan_get_routing_wan_interface(int lanDev_id);
#endif
char *cal_get_lan_id_map_uri(int lan_id);
char *cal_get_lan_uri(void);
int hcal_lan_pattern_xxx_map_id(char *value);
char *hcal_lan_pattern_xxx_map_interface(char *value);
char *cal_lan_get_master_ip_connection_uri(void);
#ifdef CONFIG_SUPPORT_IPV6
char *cal_lan_get_dnsv6_proxy_enable(int lanDev_id, int lanSub_id);
int cal_lan_set_dnsv6_proxy_enable(int lanDev_id, int lanSub_id, char *value);

char *cal_lan_get_ipv6_en(int lanDev_id, int lanSub_id);
int cal_lan_set_ipv6_en(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_ula_en(int lanDev_id, int lanSub_id);
int cal_lan_set_ula_en(int lanDev_id, int lanSub_id, char *value);
char *cal_lan_get_ipv6_ula_prefix(int lanDev_id, int lanSub_id);
int cal_lan_set_ipv6_ula_prefix(int lanDev_id, int lanSub_id, char *value);
/* InternetGatewayDevice.LANDevice.%d.LANHostConfigManagement.IPInterface.%d. */
char *cal_lan_get_ipv6_if_ipv6_en(int lanDev_id, int if_id);
int cal_lan_set_ipv6_if_ipv6_en(int lanDev_id, int if_id, char *value);
/* InternetGatewayDevice.LANDevice.%d.LANHostConfigManagement.IPInterface.%d.IPv6Prefix.%d. */
int cal_lan_add_ipv6_if_prefix_entry(int lanDev_id, int if_id, char *origin);
int cal_lan_del_ipv6_if_prefix_entry(int lanDev_id, int if_id, char *origin);
char *cal_lan_get_ipv6_if_prefix_en(int lanDev_id, int if_id, char *origin);
int cal_lan_set_ipv6_if_prefix_en(int lanDev_id, int if_id, char *origin, char *value);
char *cal_lan_get_ipv6_if_prefix_origin(int lanDev_id, int if_id, int prefix_id);
int cal_lan_set_ipv6_if_prefix_origin(int lanDev_id, int if_id, int prefix_id, char *value);
char *cal_lan_get_ipv6_if_prefix_prefix(int lanDev_id, int if_id, char *origin);
int cal_lan_set_ipv6_if_prefix_prefix(int lanDev_id, int if_id, char *origin, char *value);
char *cal_lan_get_ipv6_if_prefix_plifetime(int lanDev_id, int if_id, char *origin);
int cal_lan_set_ipv6_if_prefix_plifetime(int lanDev_id, int if_id, char *origin, char *value);
char *cal_lan_get_ipv6_if_prefix_autonomous(int lanDev_id, int if_id);
int cal_lan_set_ipv6_if_prefix_autonomous(int lanDev_id, int if_id, char *value);
#endif

/* InternetGatewayDevice.TR181.Hosts*/
int cal_lan_get_dhcpfingerprint_list(int **fingerprint_list);
char *cal_lan_get_dhcpfingerprint_type(int index);
char *cal_lan_get_dhcpfingerprint_sendoptions(int index);
char *cal_lan_get_dhcpfingerprint_requestoptions(int index);
char *cal_lan_get_dhcpfingerprint_vendorclassid(int index);
char *cal_lan_get_dhcpfingerprint_hostname(int index);
char *cal_lan_get_interception_type();

#ifdef VFIE
/****************************** New API for LAN macfilter **************************************/
char *cal_lan_get_mac_filter_mode();
int cal_lan_set_mac_filter_mode(char *value);
char *cal_lan_get_mac_filter_enable();
int cal_lan_set_mac_filter_enable(char *value);
int cal_lan_add_one_mac_filter_rule(char *name, char *mac);
int cal_lan_clear_mac_filter_rule();
char *cal_lan_get_mac_filter_name(int id);
char *cal_lan_get_mac_filter_mac(int id);
int cal_lan_get_mac_filter_num(int **maclist);
#endif
#ifdef CONFIG_SUPPORT_PRPL_HL_API
char *cal_lan_get_interface_energyefficient(int lanDev_id, int laniface_id);
char *cal_lan_get_interface_alias(int lanDev_id, int laniface_id);
int cal_lan_set_interface_alias(int lanDev_id, int laniface_id, char *value);
int cal_lan_set_interface_energyefficient(int lanDev_id, int laniface_id, char *value);
char *cal_lan_get_interface_maxbitrate(int lanDev_id, int laniface_id);
int cal_lan_set_interface_maxbitrate(int lanDev_id, int laniface_id, char *value);
char *cal_lan_get_interface_duplexmode(int lanDev_id, int laniface_id);
int cal_lan_set_interface_duplexmode(int lanDev_id, int laniface_id, char *value);
#endif
#endif /* _CAL_LAN_H_ */

