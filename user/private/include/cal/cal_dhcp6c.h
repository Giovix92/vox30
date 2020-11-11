#ifndef _CAL_DHCP6C_H
#define _CAL_DHCP6C_H

char *cal_dhcp6c_get_client_uri(int id);
char *cal_dhcp6c_get_client_en(int wan_id);
int cal_dhcp6c_set_client_en(int wan_id, char *value);

char *cal_dhcp6c_get_client_if(int if_id);
int cal_dhcp6c_set_client_if(int if_id, char *value);

char *cal_dhcp6c_get_client_duid(int wan_id);
int cal_dhcp6c_set_client_duid(int wan_id, char *value);

char *cal_dhcp6c_get_client_request_ip(int wan_id);
int cal_dhcp6c_set_client_request_ip(int wan_id, char *value);

char *cal_dhcp6c_get_client_request_prefix(int wan_id);
int cal_dhcp6c_set_client_request_prefix(int wan_id, char *value);

char *cal_dhcp6c_get_client_rapid_commit(int wan_id);
int cal_dhcp6c_set_client_rapid_commit(int wan_id, char *value);

char *cal_dhcp6c_get_client_renew(int wan_id);
int cal_dhcp6c_set_client_renew(int wan_id, char *value);

char *cal_dhcp6c_get_client_suggested_t1(int wan_id);
int cal_dhcp6c_set_client_suggested_t1(int wan_id, char *value);

char *cal_dhcp6c_get_client_suggested_t2(int wan_id);
int cal_dhcp6c_set_client_suggested_t2(int wan_id, char *value);

char *cal_dhcp6c_get_client_supported_options(int wan_id);
int cal_dhcp6c_set_client_supported_options(int wan_id, char *value);

char *cal_dhcp6c_get_client_requested_options(int wan_id);
int cal_dhcp6c_set_client_requested_options(int wan_id, char *value);
int cal_dhcp6c_add_client_requested_options(int wan_id, char *tag);
int cal_dhcp6c_del_client_requested_options(int wan_id, char *tag);

char *cal_dhcp6c_get_client_prefix_mode(int wan_id);
int cal_dhcp6c_set_client_prefix_mode(int wan_id, char *value);
char *cal_dhcp6c_get_client_random_method(int wan_id);
int cal_dhcp6c_set_client_random_method(int wan_id, char *value);
char *cal_dhcp6c_get_client_time_from(int wan_id);
int cal_dhcp6c_set_client_time_from(int wan_id, char *value);
char *cal_dhcp6c_get_client_time_to(int wan_id);
int cal_dhcp6c_set_client_time_to(int wan_id, char *value);

/******************Device.DHCPv6.Client.Option*********************/
char *cal_dhcp6c_get_client_option_tag(int if_id, int option_id);
int cal_dhcp6c_set_client_option_tag(int if_id, int option_id, char *value);

char *cal_dhcp6c_get_client_option_value(int wan_id, char *tag_value);
int cal_dhcp6c_set_client_option_value(int wan_id, char *tag_value, char *value);

#endif
