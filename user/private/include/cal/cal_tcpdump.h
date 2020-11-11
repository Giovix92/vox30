#ifndef __CAL_TCPDUMP_H__
#define __CAL_TCPDUMP_H__
int cal_tcpdump_set_stat(char *value);

char *cal_tcpdump_get_stat(void);



int cal_tcpdump_set_filter_rule(char *value);

char *cal_tcpdump_get_filter_rule(void);
int cal_tcpdump_set_max_ram_size(char *value);

char *cal_tcpdump_get_max_ram_size(void);

int cal_tcpdump_set_capture_file_size(char *value);

char *cal_tcpdump_get_capture_file_size(void);
int cal_tcpdump_set_capture_file_time(char *value);

char *cal_tcpdump_get_capture_file_time(void);
int cal_tcpdump_set_time_start(char *value);

char *cal_tcpdump_get_time_start(void);
int cal_tcpdump_set_time_stop(char *value);
char *cal_tcpdump_get_time_stop(void);

int cal_tcpdump_set_day_list(char *value);
char *cal_tcpdump_get_day_list(void);

int cal_tcpdump_set_upload_url(char *value);
char *cal_tcpdump_get_upload_url(void);


int cal_tcpdump_set_upload_user(char *value);
char *cal_tcpdump_get_upload_user(void);

int cal_tcpdump_set_upload_passwd(char *value);
char *cal_tcpdump_get_upload_passwd(void);

int cal_tcpdump_set_retry(char *value);
char *cal_tcpdump_get_retry(void);

int cal_tcpdump_schedule_day_to_number(char *day);

#endif
