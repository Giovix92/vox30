#ifndef QOS_CONFIG_H
#define QOS_CONFIG_H

#include "rcl/rcl_qos.h"

#define INGRESS_SYMBOL  "ALL,LAN"
#define EGRESS_SYMBOL   "LAN,Local"
#define ALL_SYMBOL      "ALL,LAN,Local"

#define CF_TYPE_USR "user"
#define CF_TYPE_APP "app"

#define APP_NAME_SIP "sip"
#define APP_NAME_H323 "h323"

#define SRC_MAC_TYPE_ETH "eth"
#define SRC_MAC_TYPE_VANEDOR "vendor"
#define SRC_MAC_TYPE_CLIENT "client"
#define SRC_MAC_TYPE_USER "user"

#define SERVICE_DSCP "dscp"
#define SERVICE_TOS "tos"

#define ALL_LAN_PORTS "LAN"
#define ALL_WAN_PORTS "WAN"
#define LOCAL_ROUTER  "Local"
#ifdef VD1018
#define DEFAULT_ETH_WAN_PORT_ID 3
#else
#if defined FD1018 || defined(VOX25) || defined(ESSENTIAL)
#define DEFAULT_ETH_WAN_PORT_ID 4
#else
#define DEFAULT_ETH_WAN_PORT_ID 0
#endif
#endif
#define MAX_ETH_QUEUE 8
#define MAX_DSL_QUEUE 5
#define MAX_DSL_HW_QUEUE 16
#define MAX_DSL_QUEUE_PRI 7
#define MAX_QOS_BUF 128
typedef struct
{
    int key;
        char rule_name[MAX_QOS_BUF];
        char rule_order[MAX_QOS_BUF];
        char rule_en[MAX_QOS_BUF];
        char mc_interface[MAX_QOS_BUF];
        char mc_outgoing_interface[MAX_QOS_BUF];
        char mc_ethernet_protocol[MAX_QOS_BUF];
        char mc_protocol[MAX_QOS_BUF];
        char mc_protocol_exclude[MAX_QOS_BUF];
        char mc_src_ip[MAX_QOS_BUF];
        char mc_src_ip_mask[MAX_QOS_BUF];
        char src_ip_exclude[MAX_QOS_BUF];
        char mc_des_ip[MAX_QOS_BUF];
        char mc_des_ip_mask[MAX_QOS_BUF];
        char mc_des_ip_exclude[MAX_QOS_BUF];
        char mc_source_port[MAX_QOS_BUF];
        char mc_destination_port[MAX_QOS_BUF];
        char mc_src_mac[MAX_QOS_BUF];
        char mc_src_mac_mask[MAX_QOS_BUF];
        char mc_src_mac_exclude[MAX_QOS_BUF];
        char mc_des_mac[MAX_QOS_BUF];
        char mc_des_mac_mask[MAX_QOS_BUF];
        char mc_des_mac_exclude[MAX_QOS_BUF];
        char mc_ip_length[MAX_QOS_BUF];
        char mc_dscp_check[MAX_QOS_BUF];
        char mc_priority_check[MAX_QOS_BUF];
        char mc_vlan_id_check[MAX_QOS_BUF];
        char action_queue[MAX_QOS_BUF];
        char action_priority[MAX_QOS_BUF];
        char action_ingress_priority[MAX_QOS_BUF];
        char action_dscp[MAX_QOS_BUF];
        char action_app[MAX_QOS_BUF];
        
}cal_qos_cf;
/*------------get---------*/
const char *qos_get_cf_enable(int index);
const char *qos_get_cf_status(int index);
const char *qos_get_cf_name(int index);
const char *qos_get_cf_order(int index);
const char *qos_get_cf_interface(int index);
const char *qos_get_cf_ingress_interface(int index);
const char *qos_get_cf_ingress_type(int index);
const char *qos_get_cf_ingress_priority(int index);
const char *qos_get_cf_egress_interface(int index);
const char *qos_get_cf_type(int index);
const char *qos_get_cf_app_name(int index);
const char *qos_get_cf_ether_type(int index);
const char *qos_get_cf_protocol(int index);
const char *qos_get_cf_protocol_exclude(int index);
const char *qos_get_cf_dst_ip(int index);
const char *qos_get_cf_dst_mask(int index);
const char *qos_get_cf_dst_exclude(int index);
const char *qos_get_cf_src_ip(int index);
const char *qos_get_cf_src_mask(int index);
const char *qos_get_cf_src_exclude(int index);
const char *qos_get_cf_dst_port_start(int index);
const char *qos_get_cf_dst_port_end(int index);
const char *qos_get_cf_dst_port_exclude(int index);
const char *qos_get_cf_src_port_start(int index);
const char *qos_get_cf_src_port_end(int index);
const char *qos_get_cf_src_port_exclude(int index);
const char *qos_get_cf_src_mac_type(int index);
const char *qos_get_cf_src_mac_addr(int index);
const char *qos_get_cf_dest_mac_addr(int index);
const char *qos_get_cf_src_mac_mask(int index);
const char *qos_get_cf_src_mac_exclude(int index);
const char *qos_get_cf_dest_mac_mask(int index);
const char *qos_get_cf_dest_mac_exclude(int index);
const char *qos_get_cf_src_vendor_id(int index);
const char *qos_get_cf_src_vendor_id_exclude(int index);
const char *qos_get_cf_dst_vendor_id(int index);
const char *qos_get_cf_dst_vendor_id_exclude(int index);
const char *qos_get_cf_src_client_id(int index);
const char *qos_get_cf_src_usr_id(int index);
const char *qos_get_cf_dscp_tos(int index);
const char *qos_get_cf_dscp_check(int index);
const char *qos_get_cf_dscp_start(int index);
const char *qos_get_cf_dscp_end(int index);
const char *qos_get_cf_dscp_exclude(int index);
const char *qos_get_cf_tos(int index);
const char *qos_get_cf_dscp_check(int index);
const char *qos_get_cf_ip_precedence(int index);
const char *qos_get_cf_eth_pri_check(int index);
const char *qos_get_cf_eth_pri_exclude(int index);
const char *qos_get_cf_vlan_id_check(int index);
const char *qos_get_cf_ip_len_min(int index);
const char *qos_get_cf_ip_len_max(int index);
const char *qos_get_cf_tcp_ack(int index);
const char *qos_get_cf_tcp_ack_exclude(int index);
const char *qos_get_cf_tcp_syn(int index);
const char *qos_get_cf_tcp_syn_exclude(int index);
const char *qos_get_cf_tcp_fin(int index);
const char *qos_get_cf_tcp_fin_exclude(int index);
const char *qos_get_cf_eth_pri_mark(int index);
const char *qos_get_cf_dscp_mark(int index);
const char *qos_get_cf_class_queue(int index);
const char *qos_get_cf_forwarding_policy(int index);
const char *qos_get_tcpack_prio_enable(void);
const char *qos_get_tcpack_prio_xtm(void);
const char *qos_get_tcpack_prio_eth(void);
#ifdef CONFIG_SUPPORT_QOS_DHCP_OPTION121
const char *qos_get_cf_dest_option_id(int index);

/*------------ set ------------*/
int qos_set_cf_dest_option_id(char *value, int index);
#endif
int cal_qos_get_entry_array(int ** index_list);
int qos_set_cf_enable(char *value, int index);
int qos_set_cf_name(char *value, int index);
int qos_set_cf_order(char *value, int index);
int qos_set_cf_interface(char *value, int index);
int qos_set_cf_ingress_interface(char *value, int index);
int qos_set_cf_ingress_type(char *value, int index);
int qos_set_cf_type(char *value, int index);
int qos_set_cf_ingress_priority(char *value, int index);
int qos_set_cf_egress_interface(char *value, int index);
int qos_set_cf_app_name(char *value, int index);
int qos_set_cf_ether_type(char *value, int index);
int qos_set_cf_protocol(char *value, int index);
int qos_set_cf_protocol_exclude(char *value, int index);
int qos_set_cf_dst_ip(char *value, int index);
int qos_set_cf_dst_mask(char *value, int index);
int qos_set_cf_dst_exclude(char *value, int index);
int qos_set_cf_src_ip(char *value, int index);
int qos_set_cf_src_mask(char *value, int index);
int qos_set_cf_src_exclude(char *value, int index);
int qos_set_cf_dst_port_start(char *value, int index);
int qos_set_cf_dst_port_end(char *value, int index);
int qos_set_cf_dst_port_exclude(char *value, int index);
int qos_set_cf_src_port_start(char *value, int index);
int qos_set_cf_src_port_end(char *value, int index);
int qos_set_cf_src_port_exclude(char *value, int index);
int qos_set_cf_src_mac_type(char *value, int index);
int qos_set_cf_src_mac_addr(char *value, int index);
int qos_set_cf_dest_mac_addr(char *value, int index);
int qos_set_cf_src_mac_mask(char *value, int index);
int qos_set_cf_src_mac_exclude(char *value, int index);
int qos_set_cf_dest_mac_mask(char *value, int index);
int qos_set_cf_dest_mac_exclude(char *value, int index);
int qos_set_cf_src_vendor_id(char *value, int index);
int qos_set_cf_src_vendor_id_exclude(char *value, int index);
int qos_set_cf_src_client_id(char *value, int index);
int qos_set_cf_src_usr_id(char *value, int index);
int qos_set_cf_dscp_tos(char *value, int index);
int qos_set_cf_dscp_start(char *value, int index);
int qos_set_cf_dscp_end(char *value, int index);
int qos_set_cf_dscp_exclude(char *value, int index);
int qos_set_cf_tos(char *value, int index);
int qos_set_cf_dscp_check(char *value, int index);
int qos_set_cf_ip_precedence(char *value, int index);
int qos_set_cf_eth_pri_check(char *value, int index);
int qos_set_cf_eth_pri_exclude(char *value, int index);
int qos_set_cf_vlan_id_check(char *value, int index);
int qos_set_cf_ip_len_min(char *value, int index);
int qos_set_cf_ip_len_max(char *value, int index);
int qos_set_cf_tcp_ack(char *value, int index);
int qos_set_cf_tcp_ack_exclude(char *value, int index);
int qos_set_cf_tcp_syn(char *value, int index);
int qos_set_cf_tcp_syn_exclude(char *value, int index);
int qos_set_cf_tcp_fin(char *value, int index);
int qos_set_cf_tcp_fin_exclude(char *value, int index);
int qos_set_cf_eth_pri_mark(char *value, int index);
int qos_set_cf_dscp_mark(char *value, int index);
int qos_set_cf_class_queue(char *value, int index);

/*------------- General ------------*/
int qos_get_queue_entry_max(void);
int qos_get_cf_max(void);
int qos_get_cf_num(void);
int qos_set_cf_num(char *value);
int qos_get_df_dscp_value(void);
int qos_set_df_dscp_value(char *value);
int qos_get_df_8021p_value(void);
int qos_set_df_8021p_value(char *value);
int qos_get_max_priority(char *type);
int get_physical_id_by_order(int orderIndex, char *type);
int qos_add_entry(int *p_index);
int qos_del_entry(int index);
int qos_get_cf_rule_id(int index);

/* --------------ingress --------------*/
char* get_ingress_qos_enable(void);
int set_ingress_qos_enable(char *value);

/* ------------SW Queue----------------*/

typedef struct{
    int id; // 1,2,...
    int prio; //0,1,...
    int weight;// invalid if <=0
    char bindif[32]; //WAN1,WAN2,...
}qos_queue_mgmt_t;

/* software queue */
int qos_get_swq_num(void);
int qos_get_swq_max(void);
int qos_add_swq_general(int *pindex);
int qos_del_swq_all_general(void);

/* get */
const char *qos_get_swq_enable(int index);
const char *qos_get_swq_status(int index);
const char *qos_get_swq_traffic_classes(int index);
const char *qos_get_swq_interface(int index);
const char *qos_get_swq_precedence(int index);
const char *qos_get_swq_drop_algorithm(int index);
const char *qos_get_swq_scheduler_algorithm(int index);
const char *qos_get_swq_weight(int index);
const char *qos_get_swq_burst_size(int index);
const char *qos_get_swq_committed_rate(int index);
const char *qos_get_swq_shaping_rate(int index);
const char *qos_get_swq_buffer_length(int index);
const char *qos_get_swq_latency(int index);

/* set */
int qos_set_swq_enable(char *value, int index);
int qos_set_swq_traffic_classes(char *value, int index);
int qos_set_swq_interface(char *value, int index);
int qos_set_swq_precedence(char *value, int index);
int qos_set_swq_drop_algorithm(char *value, int index);
int qos_set_swq_scheduler_algorithm(char *value, int index);
int qos_set_swq_weight(char *value, int index);
int qos_set_swq_burst_size(char *value, int index);
int qos_set_swq_committed_rate(char *value, int index);
int qos_set_swq_shaping_rate(char *value, int index);
int qos_set_swq_buffer_length(char *value, int index);
int qos_set_swq_latency(char *value, int index);
int qos_set_swq_outiface(char *value, int index);
int qos_set_swq_id(char *value, int index);

int qos_get_swq_index(int rcl_wan, int id);
int qos_add_swq(qos_queue_mgmt_t *qm);
int qos_modify_swq(qos_queue_mgmt_t *qm);
int qos_del_swq(qos_queue_mgmt_t *qm);
int qos_del_swq_all(qos_queue_mgmt_t *qm);
int qos_get_swq_all(qos_queue_mgmt_t *qm, qos_queue_mgmt_t qm_out[], int qm_size);
int qos_dump_swq_all(qos_queue_mgmt_t *qm);

/* miscellaneous */

/* get */
const char *qos_get_queue_mgnt_enable(void);
const char *qos_get_egress_enable(void);
const char *qos_get_swq_uprate(void);
const char *qos_get_switch_frame_one(void);
const char *qos_get_switch_frame_two(void);
const char *qos_get_switch_frame_three(void);
const char *qos_get_switch_frame_four(void);
int cal_qos_cf_get_all_entries(cal_qos_cf** entry);
/* set */
int qos_set_queue_mgnt_enable(char *value);
int qos_set_egress_enable(char *value);
int qos_set_swq_uprate(char *value);
int qos_set_switch_frame_one(char *value);
int qos_set_switch_frame_two(char *value);
int qos_set_switch_frame_three(char *value);
int qos_set_switch_frame_four(char *value);
int cal_qos_get_wan_high_queue(int wan_id);
int qos_get_app_num(void);
int qos_get_app_rule_id(int index);
char *qos_get_app_rule_name(int index);
const char * qos_get_app_enable(int index);
const char *qos_get_app_urn(int index);
const char *qos_get_app_key(int index);
const char *qos_get_app_eth_mark(int index);
const char *qos_get_app_dscp_mark(int index);
const char *qos_get_app_queue(int index);
const char * qos_get_flow_num(int index);
const char * qos_get_flow_key(int index);
const char * qos_get_flow_appid(int index);
const char *qos_get_cf_clsapp(int index);

#endif
