#ifndef __SAL_NSLOOKUP_H__
#define __SAL_NSLOOKUP_H__

#define DNS_RESOLVE   "1"
#define NSLOOKUP_EXIT "0"

#define DNSSERVER_NOTALAILABLE "Error_DNSServerNotAvailable"
#define HOSTNAME_NOTRESESOLVED "Error_HostNameNotResolved"
#define SUCCESS "Success"
#define ERROR_OTHER "Error_Other"

char *sal_nslookup_get_pid(void);
int sal_nslookup_set_pid(char *value);

char *sal_nslookup_get_status(void);
int sal_nslookup_set_status(char *value);
char *sal_nslookup_get_success_count(void);
int sal_nslookup_set_success_count(char *value);
char *sal_nslookup_get_result_num(void);
int sal_nslookup_set_result_num(char *value);
char *sal_nslookup_get_result_status(int id);
int sal_nslookup_set_result_status(char *value, int id);
char *sal_nslookup_get_result_answer_type(int id);
int sal_nslookup_set_result_answer_type(char *value, int id);
char *sal_nslookup_get_result_host_name(int id);
int sal_nslookup_set_result_host_name(char *value, int id);
char *sal_nslookup_get_result_ip(int id);
int sal_nslookup_set_result_ip(char *value, int id);
char *sal_nslookup_get_result_dns_server(int id);
int sal_nslookup_set_result_dns_server(char *value, int id);
char *sal_nslookup_get_result_response_time(int id);
int sal_nslookup_set_result_response_time(char *value, int id);

int sal_nslookup_check_running(void);
#endif
