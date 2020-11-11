#ifndef _CAL_NSLOOKUP_H_
#define _CAL_NSLOOKUP_H_

#define CAL_NSLOOKUP_MAX     256
#define CAL_NSLOOKUP_TMP_MAX_CHAR 8

int cal_nslookup_get_result_id(char *uri);
char* cal_nslookup_get_state(void);
int cal_nslookup_set_state(char *value);
char* cal_nslookup_get_interface(void);
int cat_nslookup_set_interface(char* value);
char* cal_nslookup_get_host(void);
int cat_nslookup_set_host(char* value);
char* cal_nslookup_get_dns_server(void);
int cat_nslookup_set_dns_server(char* value);
char* cal_nslookup_get_timeout(void);
int cat_nslookup_set_timeout(char* value);
char* cal_nslookup_get_repet_num(void);
int cat_nslookup_set_repet_num(char* value);
char* cal_nslookup_get_success_count(void);
char* cal_nslookup_get_result_num(void);

char* cal_nslookup_get_result_status(int id);
char* cal_nslookup_get_result_answer_type(int id);
char* cal_nslookup_get_result_hop_name(int id);
char* cal_nslookup_get_result_ip(int id);
char* cal_nslookup_get_result_dns_ip(int id);
char* cal_nslookup_get_result_response_time(int id);

int cal_nslookup_add_one_result(int index);
int cal_nslookup_delete_one_result(int index);
int cal_nslookup_delete_all_result(void);

#endif
