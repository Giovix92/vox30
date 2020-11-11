#ifndef _CAL_PCTRL_H
#define _CAL_PCTRL_H

typedef struct _pctrl
{
    char key[8];
    char name[64];
#ifdef VFIE
    char *mac; /* 11:22:33:44:55:66 */
#else
    char mac[18]; /* 11:22:33:44:55:66 */
#endif
    char device_name[256];
    char mode[16];
    char week[256];
    char stime[8];
    char etime[8];
    char policy[8];
}cal_pctrl_entry;

int pctrl_del_one_rule(int index);
char *cal_pctrl_get_enable(void);
int cal_pctrl_set_enable(char *value);
char *cal_pctrl_get_default_policy(void);
int cal_pctrl_set_default_policy(char *value);
int cal_pctrl_get_one_entry(cal_pctrl_entry *p_entry);
int cal_pctrl_get_all_entry(cal_pctrl_entry **p_entry);
int cal_pctrl_add_entry(cal_pctrl_entry *p_entry);
int cal_pctrl_modify_entry(cal_pctrl_entry *p_entry);
int cal_pctrl_del_entry(cal_pctrl_entry *p_entry);
int cal_pctrl_del_all_entry(void);

#endif


