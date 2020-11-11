#ifndef _CAL_HYBRID_H_
#define _CAL_HYBRID_H_

#define HA_TUNNEL_IFNAME "dummy0"
#define HA_SAVED_CONF_FILE  "/mnt/1/sb_router.conf"

char *cal_services_ha_get_enable(void); 
int cal_services_ha_set_enable(char *value);
char *cal_services_ha_get_url(void); 
int cal_services_ha_set_url(char *value);
char *cal_services_ha_get_apn(void); 
int cal_services_ha_set_apn(char *value);
char *cal_services_ha_get_ip(void); 
int cal_services_ha_set_ip(char *value);
char *cal_services_ha_get_username(void); 
int cal_services_ha_set_username(char *value);
char *cal_services_ha_get_password(void); 
int cal_services_ha_set_password(char *value);
#endif
