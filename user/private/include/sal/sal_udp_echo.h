#ifndef _SAL_UDP_ECHO_H_
#define _SAL_UDP_ECHO_H_


char *sal_udp_echo_get_packets_received(void);
int sal_udp_echo_set_packets_received(char *value);
char *sal_udp_echo_get_packets_responded(void);
int sal_udp_echo_set_packets_responded(char *value);
char *sal_udp_echo_get_bytes_received(void);
int sal_udp_echo_set_bytes_received(char *value);
char *sal_udp_echo_get_bytes_responded(void);
int sal_udp_echo_set_bytes_responded(char *value);
char *sal_udp_echo_get_time_first_packet_received(void);
int sal_udp_echo_set_time_first_packet_received(char *value);
char *sal_udp_echo_get_time_last_packet_received(void);
int sal_udp_echo_set_time_last_packet_received(char *value);
char *sal_udp_echo_get_wan_id(void);
int sal_udp_echo_set_wan_id(char *value);

#endif
