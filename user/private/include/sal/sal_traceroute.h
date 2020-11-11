#ifndef __SAL_TRACEROUTE_H__
#define __SAL_TRACEROUTE_H__

#define DNS_RESOLVE       "1"
#define TRACEROUTE_TRY    "2"
#define TRACEROUTE_EXITE  "0"

char *sal_traceroute_get_status(void);
int sal_traceroute_set_status(char *value);
char *sal_traceroute_get_pid(void);
int sal_traceroute_set_pid(char *value);

char *sal_traceroute_get_resp_time(void);
int sal_traceroute_set_resp_time(char *value);

char *sal_traceroute_get_hop_host(int id);
int sal_traceroute_set_hop_host(char *value, int id);
char *sal_traceroute_get_hop_host_addr(int id);
int sal_traceroute_set_hop_host_addr(char *value, int id);
char *sal_traceroute_get_hop_err_code(int id);
int sal_traceroute_set_hop_err_code(char *value, int id);
char *sal_traceroute_get_hop_rt_time(int id);
int sal_traceroute_set_hop_rt_time(char *value, int id);

int sal_traceroute_check_running(void);

char *sal_traceroute_get_result(void);
int sal_traceroute_set_result(char *value);
char *sal_traceroute_get_busy(void);
int sal_traceroute_set_busy(char *value);
char *sal_traceroute_get_gui_pid(void);
int sal_traceroute_set_gui_pid(char *value);
#endif /* __SAL_TRACEROUTE_H__ */
