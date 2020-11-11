#ifndef _CAL_URL_FILTER_H_
#define _CAL_URL_FILTER_H_
#define MAX_URL_NUM 16
#define MAX_URL_LENGTH 64
#define CAL_URL_FILTER_TMP_MAX_CHAR 8
typedef struct
{
    char url[64];
    char name[64];
    char key[16];
    char enable[8];
    char *mac;
}cal_url_filter;

char *cal_url_filter_get_enable(void);
int cal_url_filter_set_enable(char *value);
int cal_url_filter_get_one_entry(cal_url_filter *entry);
int cal_url_filter_get_all_entry(cal_url_filter **entry);
int cal_url_filter_add_one_entry(cal_url_filter *entry);
int cal_url_filter_del_one_entry(cal_url_filter *entry);
int cal_url_filter_modify_one_entry(cal_url_filter *entry);
int cal_url_filter_del_all_entry(void);
#endif

