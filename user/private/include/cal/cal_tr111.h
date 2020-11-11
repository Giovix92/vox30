#ifndef __CAL_TR111_H__
#define __CAL_TR111_H__
int cal_tr111_route_set_entries_num(char* value);
char* cal_tr111_route_get_entries_num(void);
int cal_tr111_route_add_one_hop(int index);
int cal_tr111_route_del_all_hop(void);

char* cal_tr111_route_get_udp_req_addrss(void);
int cal_tr111_route_set_udp_req_address(char* value);
char* cal_tr111_route_get_udp_req_address_notification_limit(void);
int cal_tr111_route_set_udp_req_address_notification_limit(char* value);
char* cal_tr111_route_get_stun_enable(void);
int cal_tr111_route_set_stun_enable(char* value);
char* cal_tr111_route_get_stun_serveraddress(void);
int cal_tr111_route_set_stun_serveraddress(char* value);
char* cal_tr111_route_get_stun_serverport(void);
int cal_tr111_route_set_stun_serverport(char* value);

char* cal_tr111_route_get_stun_username(void);
int cal_tr111_route_set_stun_username(char* value);
char* cal_tr111_route_get_stun_password(void);
int cal_tr111_route_set_stun_password(char* value);
char* cal_tr111_route_get_stun_maxkeepaliveperiod(void);
int cal_tr111_route_set_stun_maxkeepaliveperiod(char* value);
char* cal_tr111_route_get_stun_minkeepaliveperiod(void);
int cal_tr111_route_set_stun_minkeepaliveperiod(char* value);
char* cal_tr111_route_get_stun_natdetected(void);
int cal_tr111_route_set_stun_natdetected(char* value);

int cal_tr111_get_hop_id(char *uri);
#endif
