#ifndef _CAL_CORE_DUMP_H_
#define _CAL_CORE_DUMP_H_


char *cal_coredump_get_enable(void);
char *cal_coredump_get_path(void);
char *cal_coredump_get_app_to_filter(void);
int cal_coredump_set_enable(char *value);
int cal_coredump_set_path(char *value);
int cal_coredump_set_app_to_filter(char *value);

#endif
