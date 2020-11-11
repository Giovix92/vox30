#ifndef _CAL_PM_H_
#define _CAL_PM_H_

#define PM_ENTRY_DES_MAX_SIZE    512
#define PM_ENTRY_TMP_MAX_CHAR    8
#define PM_ENTRY_IP_MAX_CHAR     16

typedef struct
{
    char key[PM_ENTRY_TMP_MAX_CHAR];
    char enable[PM_ENTRY_TMP_MAX_CHAR];     /* boolean */
    char des[PM_ENTRY_DES_MAX_SIZE];
    char client_ip[PM_ENTRY_IP_MAX_CHAR];   /* string */
    char protocol[PM_ENTRY_TMP_MAX_CHAR];   /* TCP, UDP, BOTH */
    char out_sp[PM_ENTRY_TMP_MAX_CHAR];     /* 0-65535 */
    char out_ep[PM_ENTRY_TMP_MAX_CHAR];
    char in_sp[PM_ENTRY_TMP_MAX_CHAR];
    char in_ep[PM_ENTRY_TMP_MAX_CHAR];
}cal_pm_entry;

typedef struct
{
    char key[PM_ENTRY_TMP_MAX_CHAR];
    char des[PM_ENTRY_DES_MAX_SIZE];
    char protocol[PM_ENTRY_TMP_MAX_CHAR];   /* TCP, UDP, BOTH == 0,1,2*/
    char sp[PM_ENTRY_TMP_MAX_CHAR];         /* 0-65535 */
    char ep[PM_ENTRY_TMP_MAX_CHAR];         /* endport*/
}cal_cs_entry;                          /* custon service */

int cal_pm_add_one_entry(int wan_id, cal_pm_entry* entry);
int cal_pm_get_one_entry(int wan_id, cal_pm_entry* entry);
int cal_pm_get_all_entry(int wan_id, cal_pm_entry** entry);
int cal_pm_del_one_entry(int wan_id, cal_pm_entry* entry);
int cal_pm_del_all_entry(int wan_id);
#ifdef CONFIG_SUPPORT_RESERVE_PORTMAPPING_FIRST_ENTRY
int  cal_pm_del_all_entry_for_gui(int wan_id);
#endif
int cal_pm_get_entry_num(int wan_id);
int cal_pm_get_entry_array(int wan_id, int ** index_list);
int cal_pm_modify_one_entry(int wan_id, cal_pm_entry* entry);
char * cal_pm_get_uri(void);
#ifdef VFIE
/******************TR181**************************/
int cal_pm_add_one_entry_tr181(cal_pm_entry* entry);
int cal_pm_get_one_entry_tr181(cal_pm_entry* entry);
int cal_pm_get_all_entry_tr181(cal_pm_entry** entry);
int cal_pm_del_one_entry_tr181(cal_pm_entry* entry);
int cal_pm_del_all_entry_tr181(void);
int cal_pm_get_entry_num_tr181(void);
int cal_pm_get_entry_array_tr181(int ** index_list);
int cal_pm_modify_one_entry_tr181(cal_pm_entry* entry);
/********************************************/
#endif
int cal_cs_add_one_entry(cal_cs_entry* entry);
int cal_cs_get_one_entry(cal_cs_entry* entry);
int cal_cs_get_all_entry(cal_cs_entry** entry);
int cal_cs_del_one_entry(cal_cs_entry* entry);
int cal_cs_del_all_entry(void);
int cal_cs_get_entry_num(void);
int cal_cs_get_entry_array(int ** index_list);
int cal_cs_modify_one_entry(cal_cs_entry* entry);
#endif

