#ifndef _CAL_IP_PING_H_
#define _CAL_IP_PING_H_

#define CAL_IP_PING_TMP_MAX_CHAR 	8	
#define CAL_IP_PING_MAX				256	

typedef struct
{
	char key[CAL_IP_PING_TMP_MAX_CHAR];
	char state[CAL_IP_PING_TMP_MAX_CHAR];
	char interface[CAL_IP_PING_MAX];
	char host[CAL_IP_PING_MAX];
    char repeat[CAL_IP_PING_TMP_MAX_CHAR];
	char num[CAL_IP_PING_TMP_MAX_CHAR];
	char timeout[CAL_IP_PING_TMP_MAX_CHAR];
	char block_size[CAL_IP_PING_TMP_MAX_CHAR];
	char dscp[CAL_IP_PING_TMP_MAX_CHAR];
	char succ_cnt[CAL_IP_PING_TMP_MAX_CHAR];
	char fail_cnt[CAL_IP_PING_TMP_MAX_CHAR];
	char ave_time[CAL_IP_PING_TMP_MAX_CHAR];
	char min_time[CAL_IP_PING_TMP_MAX_CHAR];
	char max_time[CAL_IP_PING_TMP_MAX_CHAR];  
} cal_ip_ping;

int cal_ip_ping_get(cal_ip_ping* ip_ping);
int cal_ip_ping_set(cal_ip_ping*  ip_ping);

char* cal_ip_ping_get_state(void);
int cal_ip_ping_set_state(char* value);
char* cal_ip_ping_get_interface(void);
int cal_ip_ping_set_interface(char* value);
char* cal_ip_ping_get_host(void);
int cal_ip_ping_set_host(char* value);
char* cal_ip_ping_get_repeat(void);
int cal_ip_ping_set_repeat(char* value);
char* cal_ip_ping_get_timeout(void);
int cal_ip_ping_set_timeout(char* value);
char* cal_ip_ping_get_data_size(void);
int cal_ip_ping_set_data_size(char* value);
char* cal_ip_ping_get_dscp(void);
int cal_ip_ping_set_dscp(char* value);
char* cal_ip_ping_get_succ_cnt(void);
int cal_ip_ping_set_succ_cnt(char* value);
char* cal_ip_ping_get_fail_cnt(void);
int cal_ip_ping_set_fail_cnt(char* value);
char* cal_ip_ping_get_aver_time(void);
int cal_ip_ping_set_aver_time(char* value);
char* cal_ip_ping_get_min_time(void);
int cal_ip_ping_set_min_time(char* value);
char* cal_ip_ping_get_max_time(void);
int cal_ip_ping_set_max_time(char* value);

#endif
