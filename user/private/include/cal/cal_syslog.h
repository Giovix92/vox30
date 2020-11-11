#ifndef _CAL_SYSLOG_H
#define _CAL_SYSLOG_H


#define H_DEF_FUN_SYSLOG(para) \
char *cal_syslog_get_##para(void);\
int cal_syslog_set_##para(char *value)

H_DEF_FUN_SYSLOG(enable);
H_DEF_FUN_SYSLOG(system_en);
H_DEF_FUN_SYSLOG(link_en);
H_DEF_FUN_SYSLOG(service_en);
H_DEF_FUN_SYSLOG(management_en);
H_DEF_FUN_SYSLOG(firewall_en);
H_DEF_FUN_SYSLOG(provision_en);
H_DEF_FUN_SYSLOG(other_en);
H_DEF_FUN_SYSLOG(norm_max_size);
H_DEF_FUN_SYSLOG(crit_max_size);
H_DEF_FUN_SYSLOG(crit_sync_interval);
H_DEF_FUN_SYSLOG(filter_level);
H_DEF_FUN_SYSLOG(client_mode);
H_DEF_FUN_SYSLOG(server_ip);
H_DEF_FUN_SYSLOG(binding_if);
H_DEF_FUN_SYSLOG(crit_log_level);


#define SYSTEM_LAYER_STR        "System"    
#define LINK_LAYER_STR          "GPON,Ethernet,WIFI,USB"
#define SERVICE_LAYER_STR       "VoIP,IPTV,DNS,NTP,Samba,FTP,DDNS,UMTS,SafeNetwork"
#define MANAGEMENT_LAYER_STR    "UPnP,TR069,OMCI,GUI,SSH,TELNET,CONSOLE"
#define FIREWALL_LAYER_STR      "Firewall,ParentControl,MacFilter,IPFilter,WIFISchedule,PortMapping,PortTrigger,IPSec,DMZ"
#define PROVISION_LAYER_STR     "WAN,LAN"
int cal_syslog_get_log_category_by_module_str(char *module);
char* cal_syslog_get_module_str_by_log_category(int log_catagory_id);
char* cal_syslog_get_binding_if_name(void);
#endif

