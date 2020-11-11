/*****************************************************
copyright 2017 SerComm Corporation.ALL Rights Reserved. 
File name:
Author: Lip ling  Version:0.1    Date: 
Description:
Funcion List: 
*****************************************************/
#ifndef _CAL_SMS_H_
#define _CAL_SMS_H_

#define SMS_WHITELIST_DIRECTION_REPLY    (0x1)
#define SMS_WHITELIST_DIRECTION_SENT     (0x1 << 1)
#define SMS_WHITELIST_DIRECTION_RECEIVE  (0x1 << 2)
typedef struct cal_sms_whitelist_entry_t
{
    char channel[32];
    char direction[32];
    char name[256];
    char phone_num[32];
}cal_sms_whitelist;

typedef struct cal_sms_list_entry_t
{
    char index[32];
    char unread[2];
    char time_stamp[64];
    char phone_number[128];
    char msg_data[256];
}cal_sms_list;

char *cal_sms_whitelist_get_channel(int id);
int cal_sms_whitelist_set_channel(char *value,int id);
char *cal_sms_whitelist_get_direction(int id);
int cal_sms_whitelist_set_direction(char *value,int id);
char *cal_sms_whitelist_get_name(int id);
int cal_sms_whitelist_set_name(char *value,int id);
char *cal_sms_whitelist_get_phone_num(int id);
int cal_sms_whitelist_set_phone_num(char *value,int id);
int cal_sms_whitelist_addonce_entry(cal_sms_whitelist* value);
int cal_sms_whitelist_get_entry(int **array);
int cal_whitelist_del_index(int index);

int cal_sms_list_add(cal_sms_list* sms);
int cal_sms_get_entry(int **array);

#endif

