#ifndef __CAL_WIFIDUMP_H__
#define __CAL_WIFIDUMP_H__

int cal_wifi_dump_set_state(char *value);
char *cal_wifi_dump_get_state(void);
int cal_wifi_dump_set_link_sampling_period(char *value);
char *cal_wifi_dump_get_link_sampling_period(void);
int cal_wifi_dump_set_ap_sampling_period(char *value);
char *cal_wifi_dump_get_ap_sampling_period(void);
int cal_wifi_dump_set_air_sampling_period(char *value);
char *cal_wifi_dump_get_air_sampling_period(void);
int cal_wifi_dump_set_chan_info_sampling_period(char *value);
char *cal_wifi_dump_get_chan_info_sampling_period(void);
int cal_wifi_dump_set_spect_sampling_period(char *value);
char *cal_wifi_dump_get_spect_sampling_period(void);
int cal_wifi_dump_set_file_size(char *value);
char *cal_wifi_dump_get_file_size(void);
int cal_wifi_dump_set_file_time(char *value);
char *cal_wifi_dump_get_file_time(void);
int cal_wifi_dump_set_total_time(char *value);
char *cal_wifi_dump_get_total_time(void);
int cal_wifi_dump_set_upload_url(char *value);
char *cal_wifi_dump_get_upload_url(void);
int cal_wifi_dump_set_upload_user(char *value);
char *cal_wifi_dump_get_upload_user(void);
int cal_wifi_dump_set_upload_passwd(char *value);
char *cal_wifi_dump_get_upload_passwd(void);
int cal_wifi_dump_set_retry(char *value);
char *cal_wifi_dump_get_retry(void);
#endif
