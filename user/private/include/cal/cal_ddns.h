#ifndef __CAL_DDNS_H__
#define __CAL_DDNS_H__

#define H_CAL_DDNS_FUNC0(funcname) \
char *cal_ddns_get_##funcname(void); \
int cal_ddns_set_##funcname(char *value) \

#define H_CAL_DDNS_FUNC1(funcname) \
char *cal_ddns_get_##funcname(int id1); \
int cal_ddns_set_##funcname(char *value, int id1) \

int cal_ddns_get_client_list(int **client_list);

H_CAL_DDNS_FUNC0(enable);
H_CAL_DDNS_FUNC0(renewinterval);
H_CAL_DDNS_FUNC0(advertise);
H_CAL_DDNS_FUNC0(clientnum);
H_CAL_DDNS_FUNC1(client_enable);
H_CAL_DDNS_FUNC1(client_name);
H_CAL_DDNS_FUNC1(client_username);
H_CAL_DDNS_FUNC1(client_password);
H_CAL_DDNS_FUNC1(client_server);
#endif
