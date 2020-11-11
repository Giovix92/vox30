#ifndef _CAL_RING_SCHEDULE_H_
#define _CAL_RING_SCHEDULE_H_

#define CAL_RING_SCHEDULE_CONFIG_LEN_MAX     256

char *cal_ring_schedule_get_during_time_enable(void);
char *cal_ring_schedule_get_assign_number(int index);
char *cal_ring_schedule_get_time_frame(int index);
char *cal_ring_schedule_get_day_id_list(int index);
int cal_ring_schedule_set_ring_enable(char *value);

typedef struct _cal_ring_sch
{
    char id[64];
    char assign_number[64];
    char time_frame[64];
    char day_id_list[64];
    char time_from[64];
    char time_to[64];
}cal_ring_sch;
/*****************************************************************************

******************************************************************************/
int cal_ring_schedule_del_all(void);
int cal_ring_schedule_add(cal_ring_sch *sch);
char *cal_ring_schedule_get_time_from(int index);
char *cal_ring_schedule_get_time_to(int index);
char *cal_ring_schedule_get_enable();
int cal_ring_schedule_set_enable(char *value);
int cal_ring_schedule_get_num(void);

#endif
