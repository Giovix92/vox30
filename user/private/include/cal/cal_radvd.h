
#ifndef _CAL_RADVD_H_
#define _CAL_RADVD_H_

#define RADVD_OPTION_ERR "0"
#define RADVD_OPTION_DNS "2"

#define RADVD_ROUTER_PREFERENCE_HIGH    "High"
#define RADVD_ROUTER_PREFERENCE_Medium  "Medium"
#define RADVD_ROUTER_PREFERENCE_Low     "Low"

/************************RouterAdvertisement*****************************/
char *cal_ra_get_ra_enable(void);
int cal_ra_set_ra_enable(char *value);

/*********************RouterAdvertisement.InterfaceSetting***************/
char *cal_ra_get_if_setting_en(int lan_id);
int cal_ra_set_if_setting_en(int lan_id, char *value);

char *cal_ra_get_if_setting_if(int if_id);
int cal_ra_set_if_setting_if(int if_id, char *value);

char *cal_ra_get_if_setting_manual_prefix(int lan_id);
int cal_ra_set_if_setting_manual_prefix(int lan_id, char *value);

char *cal_ra_get_if_setting_prefix(int lan_id);
int cal_ra_set_if_setting_prefix(int lan_id, char *value);

char *cal_ra_get_if_setting_max_rtr(int lan_id);
int cal_ra_set_if_setting_max_rtr(int lan_id, char *value);

char *cal_ra_get_if_setting_min_rtr(int lan_id);
int cal_ra_set_if_setting_min_rtr(int lan_id, char *value);

char *cal_ra_get_if_setting_life_time(int lan_id);
int cal_ra_set_if_setting_life_time(int lan_id, char *value);

char *cal_ra_get_if_setting_preferred_router(int lan_id);
int cal_ra_set_if_setting_preferred_router(int lan_id, char *value);

char *cal_ra_get_if_setting_managed_flag(int lan_id);
int cal_ra_set_if_setting_managed_flag(int lan_id, char *value);

char *cal_ra_get_if_setting_other_flag(int lan_id);
int cal_ra_set_if_setting_other_flag(int lan_id, char *value);

char *cal_ra_get_if_setting_proxy_flag(int lan_id);
int cal_ra_set_if_setting_proxy_flag(int lan_id, char *value);

char *cal_ra_get_if_setting_link_mtu(int lan_id);
int cal_ra_set_if_setting_link_mtu(int lan_id, char *value);

char *cal_ra_get_if_setting_reachable_time(int lan_id);
int cal_ra_set_if_setting_reachable_time(int lan_id, char *value);

char *cal_ra_get_if_setting_retrans_time(int lan_id);
int cal_ra_set_if_setting_retrans_time(int lan_id, char *value);

char *cal_ra_get_if_setting_hop_limit(int lan_id);
int cal_ra_set_if_setting_hop_limit(int lan_id, char *value);

char *cal_ra_get_if_setting_pref_lft(int lan_id);
int cal_ra_set_if_setting_pref_lft(int lan_id, char *value);
char *cal_ra_get_if_setting_valid_lft(int lan_id);
int cal_ra_set_if_setting_valid_lft(int lan_id, char *value);

/******************RouterAdvertisement.InterfaceSetting.Option***************/
int cal_ra_add_if_setting_option_entry(int lan_id, char *tag_value);
int cal_ra_del_if_setting_option_entry(int lan_id, char *tag_value);

char *cal_ra_get_if_setting_option_tag(int if_id, int option_id);
int cal_ra_set_if_setting_option_tag(int if_id, int option_id, char *value);

char *cal_ra_get_if_setting_option_value(int lan_id, char *tag_value);
int cal_ra_set_if_setting_option_value(int lan_id, char *tag_value, char *value);

#endif /* _CAL_RADVD_H_ */
