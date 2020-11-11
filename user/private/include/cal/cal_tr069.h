#ifndef _CAL_TR069_H_
#define _CAL_TR069_H_
#ifndef CWMP_DLS_CA_PATH
#define CWMP_DLS_CA_PATH "/mnt/1/dls_ca.crt"
#endif
#ifndef CWMP_ACS_CA_PATH
#define CWMP_ACS_CA_PATH "/tmp/1/acs_ca.crt"
#endif
#ifndef SSH_ROOT_CA_KEY
#define SSH_ROOT_CA_KEY "/mnt/1/.ssh/.sshcakeys"
#endif
int is_cpe_provisioned(void);
int cal_tr069_get_enable(void);
int cal_tr069_set_enable(char *value);
int cal_tr069_get_wan_id(void);
int cal_tr069_set_wan_id(int wanId);
int cal_tr069_set_acs_url(char *value);
char *cal_tr069_get_acs_url(void);
int cal_tr069_set_con_url(char *value);
char *cal_tr069_get_con_url(void);
char *cal_tr069_get_acs_backup_url(void);
int cal_tr069_get_option43_override(void);
int cal_tr069_set_option43_override(char *value);
int cal_tr069_set_acs_user(char *value);
char *cal_tr069_get_acs_user(void);
int cal_tr069_set_acs_pwd(char *value);
char *cal_tr069_get_acs_pwd(void);
int cal_tr069_set_cpe_user(char *value);
char *cal_tr069_get_cpe_user(void);
int cal_tr069_set_cpe_pwd(char *value);
char *cal_tr069_get_cpe_pwd(void);
int cal_tr069_set_cpe_port(char *value);
int cal_tr069_get_cpe_port(void);
int cal_tr069_set_inform_enable(char *value);
int cal_tr069_get_inform_enable(void);
int cal_tr069_set_inform_interval(char *value);
int cal_tr069_get_inform_interval(void);
int cal_tr069_set_inform_time(char *value);
char *cal_tr069_get_inform_time(void);
int cal_tr069_wan_port_valid_check(int port);//0:invalid 1:valid
int cal_tr069_get_reserved_port(void);
int cal_tr069_get_reserved_port_range(void);
int cal_tr069_set_cpe_external_port(char* value);
int cal_tr069_get_cpe_external_port(void);
int cal_tr069_set_policy_basic(char* value);
int cal_tr069_get_policy_basic(void);
int cal_tr069_set_policy_custom(char* value);
int cal_tr069_get_policy_custom(void);
int cal_tr069_set_upgrade_not_needed(char* value);
int cal_tr069_get_upgrade_not_needed(void);
#endif /*_CAL_TR069_H_*/


