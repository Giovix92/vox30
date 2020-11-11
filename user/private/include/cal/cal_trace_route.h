#ifndef _CAL_TRACE_ROUTE_H_
#define _CAL_TRACE_ROUTE_H_

#define CAL_TRACE_ROUTE_MAX         256
#define CAL_TRACE_HOPS_MAX_NUM      30
#define CAL_TRACE_TMP_MAX_CHAR      8

typedef struct
{
    char key[CAL_TRACE_TMP_MAX_CHAR];
    char host[CAL_TRACE_ROUTE_MAX];
    char host_addr[CAL_TRACE_ROUTE_MAX];
    char err_code[CAL_TRACE_TMP_MAX_CHAR];
    char rt_times[CAL_TRACE_ROUTE_MAX];
} route_hop;

typedef struct
{
	char state[CAL_TRACE_ROUTE_MAX];
	char interface[CAL_TRACE_ROUTE_MAX];
	char host[CAL_TRACE_ROUTE_MAX];
	char try_num[CAL_TRACE_TMP_MAX_CHAR];
	char timeout[CAL_TRACE_TMP_MAX_CHAR];
	char block_size[CAL_TRACE_TMP_MAX_CHAR];
	char dscp[CAL_TRACE_TMP_MAX_CHAR];
	char hop_count[CAL_TRACE_TMP_MAX_CHAR];
	char resp_time[CAL_TRACE_TMP_MAX_CHAR];
	char hops_num[CAL_TRACE_TMP_MAX_CHAR];
	route_hop hop[CAL_TRACE_HOPS_MAX_NUM];
} cal_trace_route;

int cal_trace_route_get_hop_id(char *uri);
char* cal_trace_route_get_state(void);
int cal_trace_route_set_state(char* value);
char* cal_trace_route_get_interface(void);
int cal_trace_route_set_interface(char* value);
char* cal_trace_route_get_host(void);
int cal_trace_route_set_host(char* value);
char* cal_trace_route_get_try_num(void);
int cal_trace_route_set_try_num(char* value);
char* cal_trace_route_get_timeout(void);
int cal_trace_route_set_timeout(char* value);
char* cal_trace_route_get_data_size(void);
int cal_trace_route_set_data_size(char* value);
char* cal_trace_route_get_dscp(void);
int cal_trace_route_set_dscp(char* value);
char* cal_trace_route_get_max_hop_count(void);
int cal_trace_route_set_max_hop_count(char* value);
char* cal_trace_route_get_resp_time(void);
int cal_trace_route_set_resp_time(char* value);
char* cal_trace_route_get_hops_num(void);
int cal_trace_route_set_hops_num(char* value);

char* cal_trace_route_get_hop_host(int id);
int cal_trace_route_set_hop_host(char* value, int id);
char* cal_trace_route_get_hop_host_addr(int id);
int cal_trace_route_set_hop_host_addr(char* value, int id);
char* cal_trace_route_get_hop_err_code(int id);
int cal_trace_route_set_hop_err_code(char* value, int id);
char* cal_trace_route_get_hop_rt_times(int id);
int cal_trace_route_set_hop_rt_times(char* value, int id);

int cal_trace_route_add_one_hop(int index);
int cal_trace_route_del_all_hop(void);
#endif
