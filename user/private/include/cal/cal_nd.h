#ifndef _CAL_ND_H_
#define _CAL_ND_H_

char *cal_nd_get_nd_enable(void);
int cal_nd_set_nd_enable(char *value);

/*********************NeighborDiscovery.InterfaceSetting***************/
char *cal_nd_get_if_setting_en(int wan_id);
int cal_nd_set_if_setting_en(int wan_id, char *value);

char *cal_nd_get_if_setting_if(int nd_id);
int cal_nd_set_if_setting_if(int nd_id, char *value);

char *cal_nd_get_if_setting_rtr_timer(int wan_id);
int cal_nd_set_if_setting_rtr_timer(int wan_id, char *value);

char *cal_nd_get_if_setting_rtr_solicit_interval(int wan_id);
int cal_nd_set_if_setting_rtr_solicit_interval(int wan_id, char *value);

char *cal_nd_get_if_setting_max_rtr_solicits(int wan_id);
int cal_nd_set_if_setting_max_rtr_solicits(int wan_id, char *value);

char *cal_nd_get_if_setting_nud_en(int wan_id);
int cal_nd_set_if_setting_nud_en(int wan_id, char *value);

char *cal_nd_get_if_setting_rs_en(int wan_id);
int cal_nd_set_if_setting_rs_en(int wan_id, char *value);

#endif
