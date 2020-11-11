#ifndef _CAL_NOTIFICATION_H
#define _CAL_NOTIFICATION_H


#define H_DEF_FUN_NOTIFICATION(para) \
char *cal_notification_get_##para(void);\
int cal_notification_set_##para(char *value)

H_DEF_FUN_NOTIFICATION(cloudURL);
H_DEF_FUN_NOTIFICATION(accessToken);
/******************************public api***********************************************/
char *cal_get_event_type(int id);
int cal_set_event_type(char *value, int id);
char *cal_get_event_enable(int id);
int cal_set_event_enable(char *value, int id);
char *cal_get_event_url(int id);
int cal_set_event_url(char *value, int id);
int cal_add_event_index(char *enable, char *type, char *url);
char *cal_get_event_incomingcall_notifylist_deviceid(int id);
int cal_set_event_incomingcall_notifylist_deviceid(char *value,int id);
int cal_add_event_incomingcall_notifylist(char *deviceid);
int cal_get_incomingcall_notifylist_number();
int cal_del_event_incomingcall_notifylist(int id);
int cal_get_event_index_available(int **array);
int cal_del_event_index(int id);
char *cal_get_event_pushnotification_enable(void);
int cal_set_event_pushnotification_enable(char *value);
#endif

