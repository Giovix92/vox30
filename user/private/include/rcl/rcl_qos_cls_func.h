/**
 * @file   
 * @author 
 * @date   2010-08-30
 * @brief  
 *
 * Copyright - 2009 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */
#ifndef _QOS_CLS_FUNC_H__
#define _QOS_CLS_FUNC_H__
#define MAX_FLOW_NUM 4
typedef struct flow_policy {
    int flow_index;         //flow number
    int flow_enable; 
    int flow_status;        //disabled ,enable
    char flow_type[128];    //flow urn
    char flow_type_para[256];
    char flow_name[64];
    int flow_key;           //link app table and flow table 
    int flow_fw_policy;
    int flow_trf_class;     //-1 no traffic class.
    int flow_policer;       //-1 no policer.
    int flow_queue;         //-1 no queue
    int flow_dscp_mark;     //-1 no change,-2 ethprio:[0-7]
    int flow_ethprio_mark;  //-1 no change,-2 dscp prio.  
    int flow_id;            // match the flow_id of urn_table
}flow_policy;
typedef struct app_policy 
{   
    int app_key;
    int  app_enable; 
    int app_status;             //disabled ,error
    char app_urn[256];          //app urn :urn:***
    char app_name[64]; 
    unsigned int app_def_fw_policy; //appdefaultforwardingpolicy
    int app_def_trf_class;          //appdefaulttrafficclass
    int app_def_policer;            //appdefaultpolicer
    int app_def_queue;              //appdefaultqueue
    int app_def_dscp_mark;
    int app_def_ethprio_mark; 
    int app_phy_index;
    int app_module;
    int app_id;                     //match the app_id of urn_table
    int flow_num;
    struct app_policy * next;
    flow_policy flow_tab[MAX_FLOW_NUM];
}app_policy;



typedef struct qos_cls_policy_entry
{
    struct qos_cls_policy_entry *next;

    int  policy_enable;      // ClsEn:1
    int  policy_index;       // 0-RULE_NUM_MAX
    int  queue_index;        // 1-8

    char s_cls_owner[16];      // ingress, egress
    char s_cls_type[16];      // user, app
    char s_cls_app[16];      // app name: sip, h323
    char s_cls_src[256];      // LAN,LOCAL...
    char o_cls_destiface[128];      // wan0, wan1...

    char s_ethertype[16];    // ARP,PPP_DISC,PPP_SES,ipv4
    char s_srcmac[18];       // xx:xx:xx:xx:xx:xx
    char s_srcmac_mask[18];       // xx:xx:xx:xx:xx:xx
    int  s_srcmac_exclude;
    char s_destmac[18];       // xx:xx:xx:xx:xx:xx
    char s_destmac_mask[18];       // xx:xx:xx:xx:xx:xx
    int  s_destmac_exclude;
    int  ether_prio_mode;    // include,exclude
    int  ether_prio;         // 0-7
    int  vlanid;             // 1-4094

    int  proto_mode;         // 0: Ignored,1: include,2: exclude
    int  protocol;           // ICMP: 1,IGMP: 2,TCP: 6,UDP: 17,/etc/protocol
    int  dstip_mode;         // 0: Ignored,1: include,2: exclude
    char dstip[64];          // xxx.xxx.xxx.xxx
    char dstip_mask[64];     // xxx.xxx.xxx.xxx
    int  dport_mode;         // 0: Ignored,1: include,2: exclude
    int  dport_start;        // 0-65535
    int  dport_end;          // 0-65535
    int  srcip_mode;         // 0: Ignored,1: include,2: exclude
    char srcip[64];          // xxx.xxx.xxx.xxx
    char srcip_mask[64];     // xxx.xxx.xxx.xxx
    int  sport_mode;         // 0: Ignored,1: include,2: exclude
    int  sport_start;        // 0-65535
    int  sport_end;          // 0-65535
    int  SVdClsIDmode;
    int  DVdClsIDmode;
    char SVdClassID[256];
    char DVdClassID[256];
    char SClientID[256];
    char SUClassID[256];
    int  dscp_tos_sel;       // 0: Ignore,1: DSCP,2: ToS
    int  dscp_mode;          // 0: Ignored,1: include,2: exclude
    int  dscp;               // dscpcheck
    int  dscp_end;
    int  tos_mode;           // 0: Ignored,1: include,2: exclude
    int  tos;
    int  ip_precedence;
    int  packet_length_min;
    int  packet_length_max;
    int  tcp_ctrl_enable;    // 0: disable,1: enable
    int  tcp_ctrl_mode;      // 0: Ignored,1: include,2: exclude
    int  tcp_syn_mode;       // 0: Ignored,1: include,2: exclude
    char tcp_syn_ctrl[8];    // disable,enable
    int  tcp_ack_mode;       // 0: Ignored,1: include,2: exclude
    char tcp_ack_ctrl[8];    // disable,enable
    int  tcp_fin_mode;       // 0: Ignored,1: include,2: exclude
    char tcp_fin_ctrl[8];    // disable,enable

    int  ethprio_remarkto;
    int  dscp_remarkto;

    int cls_module; //what place to process this rule
    int phy_index; //instance number in classify rule
    int classapp; //classapp = 1, enable enter app table
#ifdef CONFIG_SUPPORT_QOS_DHCP_OPTION121
    int destOptionID; //Dest info from DHCP option id
#endif
}qos_cls_policy_entry;

int set_policy_enable(char *value,qos_cls_policy_entry *p);
int get_policy_enable(char *buf,qos_cls_policy_entry *p);
int set_policy_order(char *value,qos_cls_policy_entry *p);
int get_policy_order(char *buf,qos_cls_policy_entry *p);
int set_cls_type(char *value,qos_cls_policy_entry *p);
int get_cls_type(char *buf,qos_cls_policy_entry *p);
int set_cls_app(char *value,qos_cls_policy_entry *p);
int get_cls_app(char *buf,qos_cls_policy_entry *p);
int set_cls_src(char *value,qos_cls_policy_entry *p);
int get_cls_src(char *buf,qos_cls_policy_entry *p);
int set_cls_destiface(char *value,qos_cls_policy_entry *p);
int get_cls_destiface(char *buf,qos_cls_policy_entry *p);
int set_dst_ip(char *value,qos_cls_policy_entry *p);
int get_dst_ip(char *buf,qos_cls_policy_entry *p);
int set_dstip_mask(char *value,qos_cls_policy_entry *p);
int get_dstip_mask(char *buf,qos_cls_policy_entry *p);
int set_dstip_mode(char *value,qos_cls_policy_entry *p);
int get_dstip_mode(char *buf,qos_cls_policy_entry *p);
int set_src_ip(char *value,qos_cls_policy_entry *p);
int get_src_ip(char *buf,qos_cls_policy_entry *p);
int set_srcip_mask(char *value,qos_cls_policy_entry *p);
int get_srcip_mask(char *buf,qos_cls_policy_entry *p);
int set_srcip_mode(char *value,qos_cls_policy_entry *p);
int get_srcip_mode(char *buf,qos_cls_policy_entry *p);
int set_protocol(char *value,qos_cls_policy_entry *p);
int get_protocol(char *buf,qos_cls_policy_entry *p);
int set_proto_mode(char *value,qos_cls_policy_entry *p);
int get_proto_mode(char *buf,qos_cls_policy_entry *p);
int set_dport_start(char *value,qos_cls_policy_entry *p);
int get_dport_start(char *buf,qos_cls_policy_entry *p);
int set_dport_end(char *value,qos_cls_policy_entry *p);
int get_dport_end(char *buf,qos_cls_policy_entry *p);
int set_dport_mode(char *value,qos_cls_policy_entry *p);
int get_dport_mode(char *buf,qos_cls_policy_entry *p);
int set_sport_start(char *value,qos_cls_policy_entry *p);
int get_sport_start(char *buf,qos_cls_policy_entry *p);
int set_sport_end(char *value,qos_cls_policy_entry *p);
int get_sport_end(char *buf,qos_cls_policy_entry *p);
int set_sport_mode(char *value,qos_cls_policy_entry *p);
int get_sport_mode(char *buf,qos_cls_policy_entry *p);
int set_svdclsid(char *value,qos_cls_policy_entry *p);
int get_svdclsid(char *buf,qos_cls_policy_entry *p);
int set_svdclsid_mode(char *value,qos_cls_policy_entry *p);
int get_svdclsid_mode(char *buf,qos_cls_policy_entry *p);
int set_dvdclsid(char *value,qos_cls_policy_entry *p);
int get_dvdclsid(char *buf,qos_cls_policy_entry *p);
int set_dvdclsid_mode(char *value,qos_cls_policy_entry *p);
int get_dvdclsid_mode(char *buf,qos_cls_policy_entry *p);
int set_tcpack_ctrl(char *value,qos_cls_policy_entry *p);
int get_tcpack_ctrl(char *buf,qos_cls_policy_entry *p);
int set_tcpack_ctrlmode(char *value,qos_cls_policy_entry *p);
int get_tcpack_ctrlmode(char *buf,qos_cls_policy_entry *p);
int set_dscp(char *value,qos_cls_policy_entry *p);
int get_dscp(char *buf,qos_cls_policy_entry *p);
int set_dscp_end(char *value,qos_cls_policy_entry *p);
int get_dscp_end(char *buf,qos_cls_policy_entry *p);
int set_dscp_mode(char *value,qos_cls_policy_entry *p);
int get_dscp_mode(char *buf,qos_cls_policy_entry *p);
int set_dscp_remarkto(char *value,qos_cls_policy_entry *p);
int get_dscp_remarkto(char *buf,qos_cls_policy_entry *p);
int set_8021p(char *value,qos_cls_policy_entry *p);
int get_8021p(char *buf,qos_cls_policy_entry *p);
int set_8021p_mode(char *value,qos_cls_policy_entry *p);
int get_8021p_mode(char *buf,qos_cls_policy_entry *p);
int set_ethprio_remarkto(char *value,qos_cls_policy_entry *p);
int get_ethprio_remarkto(char *buf,qos_cls_policy_entry *p);
int set_queue_index(char *value,qos_cls_policy_entry *p);
int get_queue_index(char *buf,qos_cls_policy_entry *p);
int set_sclientid(char *value,qos_cls_policy_entry *p);
int get_sclientid(char *buf,qos_cls_policy_entry *p);
int set_suclsid(char *value,qos_cls_policy_entry *p);
int get_suclsid(char *buf,qos_cls_policy_entry *p);
int set_vlanid(char *value,qos_cls_policy_entry *p);
int get_vlanid(char *buf,qos_cls_policy_entry *p);
int set_iplength_min(char *value,qos_cls_policy_entry *p);
int get_iplength_min(char *buf,qos_cls_policy_entry *p);
int set_iplength_max(char *value,qos_cls_policy_entry *p);
int get_iplength_max(char *buf,qos_cls_policy_entry *p);
int set_tcpsyn_ctrl(char *value,qos_cls_policy_entry *p);
int get_tcpsyn_ctrl(char *buf,qos_cls_policy_entry *p);
int set_tcpsyn_ctrlmode(char *value,qos_cls_policy_entry *p);
int get_tcpsyn_ctrlmode(char *buf,qos_cls_policy_entry *p);
int set_tcpfin_ctrl(char *value,qos_cls_policy_entry *p);
int get_tcpfin_ctrl(char *buf,qos_cls_policy_entry *p);
int set_tcpfin_ctrlmode(char *value,qos_cls_policy_entry *p);
int get_tcpfin_ctrlmode(char *buf,qos_cls_policy_entry *p);
int set_tos(char *value,qos_cls_policy_entry *p);
int get_tos(char *buf,qos_cls_policy_entry *p);
int set_ipprecedence(char *value,qos_cls_policy_entry *p);
int get_ipprecedence(char *buf,qos_cls_policy_entry *p);
int set_src_mac(char *value,qos_cls_policy_entry *p);
int get_src_mac(char *buf,qos_cls_policy_entry *p);
int set_src_mac_mask(char *value,qos_cls_policy_entry *p);
int get_src_mac_mask(char *buf,qos_cls_policy_entry *p);
int set_src_mac_exclude(char *value,qos_cls_policy_entry *p);
int get_src_mac_exclude(char *buf,qos_cls_policy_entry *p);
int set_dest_mac(char *value,qos_cls_policy_entry *p);
int get_dest_mac(char *buf,qos_cls_policy_entry *p);
int set_dest_mac_mask(char *value,qos_cls_policy_entry *p);
int get_dest_mac_mask(char *buf,qos_cls_policy_entry *p);
int set_dest_mac_exclude(char *value,qos_cls_policy_entry *p);
int get_dest_mac_exclude(char *buf,qos_cls_policy_entry *p);
int set_ethtype(char *value,qos_cls_policy_entry *p);
int get_ethtype(char *buf,qos_cls_policy_entry *p);
int set_cls_owner(char *value,qos_cls_policy_entry *p);
int get_cls_owner(char *buf,qos_cls_policy_entry *p);
void add_qos_app_policy(struct app_policy **head, struct app_policy *a);
#ifdef CONFIG_SUPPORT_QOS_DHCP_OPTION121
int set_dest_option_id(char *value,qos_cls_policy_entry *p);
int get_dest_option_id(char *buf,qos_cls_policy_entry *p);
#endif

/*********************CLASSIFY POLICY GENERIC FUNC*****************************/
void add_qos_cls_policy(struct qos_cls_policy_entry **head, struct qos_cls_policy_entry *a);
void free_qos_cls_policy(struct qos_cls_policy_entry *head);
int dump_qos_cls_all_policy(struct qos_cls_policy_entry *head);
int dump_qos_cls_single_policy(qos_cls_policy_entry *p);

enum{
    IFACE_NONE = 0,
    IFACE_LAN,
    IFACE_LAN2,
    IFACE_WLAN,
    IFACE_WAN
};

typedef struct iface_id
{
    int iface;
    int id;
}iface_id_t;

void resolve_iface_id(char *iface, iface_id_t *t);
void resolve_class_iface_id(char *iface, iface_id_t *t);

void update_conf_QoS_part(FILE *fp_conf);
int set_clsapp(char * value,qos_cls_policy_entry *p);
int get_clsapp(char * buf,qos_cls_policy_entry*p);
int set_appenable( char * value,app_policy *p);
int get_appenable(char * buf,app_policy *p);
int set_appkey( char * value,app_policy *p);
int get_appkey(char * buf,app_policy *p);
int set_appurn(char * value,app_policy *p);
int get_appurn(char * buf,app_policy *p);
int set_flownum(char * value,app_policy *p);
int get_flownum(char * buf,app_policy *p);
int set_app_eth_mark( char * value,app_policy *p);
int get_app_eth_mark(char * buf,app_policy *p);
int set_app_dscp_mark( char * value,app_policy *p);
int get_app_dscp_mark(char * buf,app_policy *p);
int set_app_queue(char * value,app_policy * p);
int get_app_queue(char * buf,app_policy *p);


#endif
