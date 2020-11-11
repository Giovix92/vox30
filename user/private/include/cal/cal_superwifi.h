#ifndef _CAL_SUPERWIFI_H_
#define _CAL_SUPERWIFI_H_
char *cal_services_superwifi_get_controlmode(void); 
int cal_services_superwifi_set_controlmode(char *value);
char *cal_services_superwifi_get_cloudaddress(void);
int cal_services_superwifi_set_cloudaddress(char *value);
char *cal_services_superwifi_get_restart(void);
int cal_services_superwifi_set_restart(char *value);
char *cal_services_superwifi_get_extender_allowed(void); 
#ifndef CONFIG_SUPPORT_SUPERWIFI_NEW_DATAMODEL
int cal_services_superwifi_set_extender_allowed(char *value);
#endif
char *cal_services_superwifi_get_uuid(void); 
int cal_services_superwifi_set_uuid(char *value);
char *cal_services_superwifi_get_unscr_lanaccess(void); 
int cal_services_superwifi_set_unscr_lanaccess(char *value);
char *cal_services_superwifi_get_loglevel(void); 
int cal_services_superwifi_set_loglevel(char *value);
#ifdef CONFIG_SUPPORT_REPEATER
char *cal_services_superwifi_get_extender_autoprovision_enable(void); 
char *cal_services_superwifi_get_extender_autoprovision_whitelist(void); 
#endif
#endif
