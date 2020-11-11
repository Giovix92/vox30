#ifndef _CAL_UDP_ECHO_H_
#define _CAL_UDP_ECHO_H_

int cal_udp_echo_set_enable(char * value);
char* cal_udp_echo_get_enable(void);
int  cal_udp_echo_set_echo_plus_enable(char * value);
char* cal_udp_echo_get_echo_plus_enable(void);
int  cal_udp_echo_set_echo_plus_supported(char * value);
char* cal_udp_echo_get_echo_plus_supported(void);
int cal_udp_echo_set_interface(char * value);
char* cal_udp_echo_get_interface(void);
int cal_udp_echo_set_source_ip_address(char * value);
char* cal_udp_echo_get_source_ip_address(void);
int cal_udp_echo_set_udp_port(char * value);
char* cal_udp_echo_get_udp_port(void);

#endif