#ifndef __CAL_MGMT_H__
#define __CAL_MGMT_H__

enum{
    LOGIN_ACCOUNT_ADMIN=1,
    LOGIN_ACCOUNT_ISP=2,
    LOGIN_ACCOUNT_DEBUG=3,
};

#define HTTP_PASSWD_FILE            "/tmp/htpasswd"
#define HTTP_PASSWD_FILE_REMOTE     "/tmp/htpasswdremote"

#define LOGIN_ACCOUNT_DEFAULT LOGIN_ACCOUNT_ADMIN

char *cal_mgmt_httpd_get_top_uri(void);
int cal_mgmt_get_default_wan_id(void);
int cal_mgmt_set_default_wan_id(int wanId);
char *cal_mgmt_upnp_get_enable(void);
int cal_mgmt_upnp_set_enable(char *value);
char *cal_mgmt_upnp_get_rw_mode(void);
int cal_mgmt_upnp_set_rw_mode(char *value);
char *cal_mgmt_upnp_get_nat_tranversal_mode(void);
int cal_mgmt_upnp_set_nat_traversal_mode(char *value);

char *cal_mgmt_upnp_get_igd1_mac_prefix(void);
int cal_mgmt_upnp_set_igd1_mac_prefix(char *value);


char *cal_mgmt_sshd_get_enable(void);
int cal_mgmt_sshd_set_enable(char *value);
int cal_mgmt_sshd_set_access_port(char *value);
char *cal_mgmt_sshd_get_access_port(void);
int cal_mgmt_sshd_set_external_access_port(char *value);
char *cal_mgmt_sshd_get_external_access_port(void);
char *cal_mgmt_sshd_get_interface(void);
int cal_mgmt_sshd_set_interface(char *value);

char *cal_mgmt_telnetd_get_enable(void);
int cal_mgmt_telnetd_set_enable(char *value);
int cal_mgmt_telnetd_set_access_port(char *value);
char *cal_mgmt_telnetd_get_access_port(void);
char *cal_mgmt_telnetd_get_interface(void);
int cal_mgmt_telnetd_set_interface(char *value);
int cal_mgmt_telnetd_set_external_access_port(char *value);
char *cal_mgmt_telnetd_get_external_access_port(void);
char *cal_mgmt_telnetd_get_prompt(void);
int cal_mgmt_telnetd_set_prompt(char *value);
char *cal_mgmt_console_get_enable(void);
int cal_mgmt_console_set_enable(char *value);
char *cal_mgmt_openmodem_get_enable(void);
int cal_mgmt_openmodem_set_enable(char *value);
char *cal_mgmt_openmodem_get_gui_enable(void);
int cal_mgmt_openmodem_set_gui_enable(char *value);
int cal_mgmt_shell_set_enable(char *value);
char *cal_mgmt_shell_get_enable(void);
int cal_mgmt_tcpdump_set_enable(char *value);
char *cal_mgmt_tcpdump_get_enable(void);

char *cal_mgmt_httpd_get_enable(void);
int cal_mgmt_httpd_set_enable(char *value);
int cal_mgmt_httpd_set_access_port(char *value);
char *cal_mgmt_httpd_get_access_port(void);
int cal_mgmt_httpd_set_external_access_port(char *value);
char *cal_mgmt_httpd_get_external_access_port(void);
char *cal_mgmt_httpd_get_interface(void);
int cal_mgmt_httpd_set_interface(char *value);

char *cal_mgmt_get_account_num(void);
char *cal_mgmt_httpd_get_user_name(int id);
int cal_mgmt_httpd_get_user_list(int **user_list);
int cal_mgmt_httpd_set_user_name(int id, char *value);
char *cal_mgmt_httpd_get_password(int id);
int cal_mgmt_httpd_set_password(int id, char *value);

char *cal_mgmt_httpsd_get_enable(void);
int cal_mgmt_httpsd_set_enable(char *value);
int cal_mgmt_httpsd_set_access_port(char *value);
char *cal_mgmt_httpsd_get_access_port(void);
char *cal_mgmt_httpsd_get_external_access_port(void);
char *cal_mgmt_httpsd_get_interface(void);
int cal_mgmt_httpsd_set_interface(char *value);

char *cal_mgmt_get_autosave_timeout(void);
char *cal_mgmt_http_get_timeout_interval(void);
int cal_mgmt_http_set_timeout_interval(char* value);
#ifdef SUPPORT_SNMP
char* cal_mgmt_snmp_get_server_access_port(void);
char* cal_mgmt_snmp_get_server_external_access_port(void);
char* cal_mgmt_snmp_get_inform_access_port(void);
char* cal_mgmt_snmp_get_inform_external_access_port(void);
int cal_mgmt_snmp_set_server_access_port(char* value);
int cal_mgmt_snmp_set_server_external_access_port(char* value);
int cal_mgmt_snmp_set_inform_access_port(char* value);
int cal_mgmt_snmp_set_inform_external_access_port(char* value);
#endif
#endif
