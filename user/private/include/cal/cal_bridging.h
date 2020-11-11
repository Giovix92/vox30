/**
 * @file   cal_bridging.h
 * @author Martin_Huang@sdc.sercomm.com
 * @date   2013-01-02
 * @brief
 *
 *   Limimations:
 *      1. Don't support filter per ether type or vlan
 * 
 * Copyright - 2013 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 *
 *
 */
#ifndef __CAL_BRIDGING_H__
#define __CAL_BRIDGING_H__

#define IS_WAN_INTERFACE(interface) (strstr(interface, "WANConnectionDevice"))
#define IS_L2_WAN_INTERFACE(interface) (strstr(interface, "WANConnectionDevice")\
                    && !(strstr(interface, "WANPPPConnection") || strstr(interface, "WANIPConnection")))

#define IS_L3_WAN_INTERFACE(interface) ((strstr(interface, "WANPPPConnection") || strstr(interface, "WANIPConnection")))
#define IS_L3_WAN_IP_INTERFACE(interface) ((strstr(interface, "WANIPConnection")))
#define IS_L3_WAN_PPP_INTERFACE(interface) ((strstr(interface, "WANPPPConnection")))

#define IS_L2_ELAN_INTERFACE(interface) (strstr(interface, "LANEthernetInterfaceConfig"))
#define IS_L2_WLAN_INTERFACE(interface) (strstr(interface, "WLANConfiguration"))
#define IS_L2_LAN_INTERFACE(interface) (strstr(interface, "WLANConfiguration") || strstr(interface, "LANEthernetInterfaceConfig"))
#define IS_L3_LAN_INTERFACE(interface) (strstr(interface, "IPInterface"))
#define BGM_L2_BRIDGING_BRIDGE_IF_NAME_PREFIX  "gr"
#define BGM_L2_ROUTING_BRIDGE_IF_NAME_PREFIX   "br"
#define BGM_L2_BRIDGING_WAN_VDSL_IF_NAME_PREFIX     "ptm0"
#define BGM_L2_BRIDGING_WAN_ADSL_IF_NAME_PREFIX     "nas"
#ifdef VD1018
#define BGM_L2_BRIDGING_WAN_ETHER_IF_NAME_PREFIX     "eth3"
#else
#if defined(FD1018) || defined(VOX25) || defined(ESSENTIAL) 
#define BGM_L2_BRIDGING_WAN_ETHER_IF_NAME_PREFIX     "eth4"
#else
#define BGM_L2_BRIDGING_WAN_ETHER_IF_NAME_PREFIX     "eth0"
#endif
#endif
#define BGM_L2_BRIDGING_ETHER_IF_NAME_PREFIX   "eth"
#define BGM_L2_BRIDGING_WLAN_IF_NAME_PREFIX   "wl0"

#define BRIDGE_MAX_VALUE_LENGTH 		512
typedef struct tag_BRIDGING_MARKING_ENTRY{
    int key;
    int enable;
    int bridgeKey;
    char interface[BRIDGE_MAX_VALUE_LENGTH];
    int pbit;
    int unTag;
    int vlan;
    int override;
    int prio_override;
}BRIDGING_MARKING_ENTRY;
typedef struct tag_BRIDGING_FILTER_ENTRY{
    int key;
    int enable;
    int bridgeKey;
    char interface[BRIDGE_MAX_VALUE_LENGTH];
    int vlan;
    int onlyTagged;
    char ethertype[256];
    char sip[32];
}BRIDGING_FILTER_ENTRY;

typedef struct tag_L2BRIDGING_PORT_ENTRY{
#define L2BRIDGING_LAN 0
#define L2BRIDGING_WAN 1
#define L2BRIDGING_WLAN 2
#define L3ROUTING_LAN 3
#define L3ROUTING_WAN 4
    int type;
#define L2BRIDGING_BRIDGING_MODE 0
#define L2BRIDGING_HOST_MODE 1
#define L2BRIDGING_ROUTING_MODE 2
#define L2BRIDGING_MAX_VLAN_NUM_PER_BRIDGE 4
    int mode;
/*1,2,3,4, LAN port ID*/
/*1 * 1000 WAN port ID, 1~1000 connection id*/
    char port;
    char interface[256];
    char tag_num;
    int vlan;
    int pbit;
    int vlans[L2BRIDGING_MAX_VLAN_NUM_PER_BRIDGE];
    char ethertype[256];
    int snooping;
    char sip[32];
}L2BRIDGING_PORT_ENTRY;
int cal_bridging_get_bridge_num(void);
int cal_bridging_get_bridge_keys(int **key_array);
char  *cal_bridging_get_bridge_vlan_id(int bridge_key);
int cal_bridging_set_bridge_vlan_id(int bridge_key, char *value);
char  *cal_bridging_get_bridge_name(int bridge_key);
int cal_bridging_set_bridge_name(int bridge_key, char *value);
char  *cal_bridging_get_bridge_standard(int bridge_key);
int cal_bridging_set_bridge_stardard(int bridge_key, char *value);
char  *cal_bridging_get_bridge_enable(int bridge_key);
int cal_bridging_set_bridge_enable(int bridge_key, char *value);
int cal_bridging_get_wan_host_vlan_by_interface(char *interface);
int cal_bridging_get_wan_host_pbit_by_interface(char *interface);
int cal_bridging_get_bridge_port_entries(int bridge_key, L2BRIDGING_PORT_ENTRY **entry);
L2BRIDGING_PORT_ENTRY *cal_bridging_get_routing_wan_port(L2BRIDGING_PORT_ENTRY *entry, int entry_num);
L2BRIDGING_PORT_ENTRY *cal_bridging_get_bridge_wan_port(L2BRIDGING_PORT_ENTRY *entry, int entry_num);
char *cal_bridging_get_wan_routing_interface_by_lan_ip_interface(char *interface);
int cal_available_if_add_one_object(char* uri);
int cal_available_if_del_one_object(char* uri);
int cal_bridging_set_wan_host_vlan_by_interface(char *interface, char *value);
int cal_bridging_set_wan_host_pbit_by_interface(char *interface, char *value);
char* hcal_bridging_get_if_uri_by_key(int key);
int hcal_bridging_get_if_key_by_uri(char* uri);
int cal_bridging_del_object_hook(char * uri);
int cal_bridging_map_bridgekey_to_wanid(int bridge_key);
int cal_bridging_set_value_hook(char * uri);
char* cal_bridging_get_uri_by_interface_name(const char *iface);
int cal_bridging_get_bridge_key_by_name(const char *bridge_name);
int cal_bridging_add_bridge(const char *bridge_name);
int cal_bridging_set_bridge_enable_by_key(int id, char *value);
int cal_bridging_set_bridge_vlan_by_key(int bridge_key, char *value);
int cal_bridging_get_bridge_filter_entries_by_bridge_id(int bridge_key, BRIDGING_FILTER_ENTRY **entry);
int cal_bridging_add_bridge_filter_entry();
int cal_bridging_set_bridge_snooping_by_key(int id, char *value);
int cal_bridging_load_all_bridge_filter_entries(BRIDGING_FILTER_ENTRY **entry);
int cal_bridging_set_filter_admit_tagged(int filter_key, char *value);
int cal_bridging_set_filter_vlanid(int filter_key, char *value);
int cal_bridging_set_filter_interface(int filter_key, char *value);
int cal_bridging_set_filter_bridge_reference(int filter_key, char *value);
int cal_bridging_set_filter_enable(int filter_key, char *value);
int cal_bridging_set_filter_ethertype(int filter_key, char *value);
int cal_bridging_set_filter_sourceip(int filter_key, char *value);
int cal_bridging_load_all_bridge_marking_entries(BRIDGING_MARKING_ENTRY **entry);
int cal_bridging_get_bridge_marking_entries(int bridge_key, BRIDGING_MARKING_ENTRY **entry);
int cal_bridging_add_marking();
int cal_bridging_set_marking_bridge_reference(int marking_key, char *value);
int cal_bridging_set_marking_enable(int marking_key, char *value);
int cal_bridging_set_marking_interface(int marking_key, char *value);
int cal_bridging_set_marking_priority(int marking_key, char *value);
int cal_bridging_set_marking_vlaniduntag(int marking_key, char *value);
int cal_bridging_set_marking_vlanidmark(int marking_key, char *value);
int cal_bridging_set_marking_priorityoverride(int marking_key, char *value);
int cal_bridging_del_filter_by_bridge_interface(int bridge_key, int if_key);
int cal_bridging_del_bridge(int id);
int cal_bridging_del_filter_entry(int filter_key);
int cal_bridging_del_marking_entry(int marking_key);
#endif
