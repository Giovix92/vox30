#ifndef _CAL_PLUME_H_
#define _CAL_PLUME_H_
char *cal_services_plume_get_controlmode(void); 
int cal_services_plume_set_controlmode(char *value);
char *cal_services_plume_get_cloudaddress(void);
int cal_services_plume_set_cloudaddress(char *value);
char *cal_services_plume_get_restart(void);
int cal_services_plume_set_restart(char *value);
char *cal_services_plume_get_extender_allowed(void); 
int cal_services_plume_set_extender_allowed(char *value);
#endif
