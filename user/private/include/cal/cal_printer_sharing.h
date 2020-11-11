#ifndef _CAL_PRINTER_SHARING_H_
#define _CAL_PRINTER_SHARING_H_


#define H_DEF_FUN_PSHARING(para) \
char *cal_psharing_get_##para(void);\
int cal_psharing_set_##para(char *value)

H_DEF_FUN_PSHARING(enable);
H_DEF_FUN_PSHARING(printer_name);

#endif

