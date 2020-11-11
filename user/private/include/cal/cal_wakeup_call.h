#ifndef _CAL_WAKEUP_CALL_H_
#define _CAL_WAKEUP_CALL_H_

#define CAL_WAKEUP_CALL_CONFIG_LEN_MAX     256

typedef struct _cal_wakeup_call
{
    char name[64];
    char enable[64];
    char time[64];
    char fxs[64];
    char time_frame[64];
    char day_id_list[64];
}cal_wakeup_call;
/*****************************************************************************

******************************************************************************/
int cal_wakeup_call_del_all(void);
int cal_wakeup_call_add(cal_wakeup_call *item);
char *cal_wakeup_call_get_name(int index);
char *cal_wakeup_call_get_fxs(int index);
char *cal_wakeup_call_get_enable(int index);
char *cal_wakeup_call_get_time(int index);
char *cal_wakeup_call_get_time_frame(int index);
char *cal_wakeup_call_get_day_id_list(int index);
int cal_wakeup_call_get_num(void);
#endif
