#ifndef _CAL_PT_H_
#define _CAL_PT_H_

#define PT_ENTRY_DES_MAX_SIZE    512
#define PT_ENTRY_TMP_MAX_CHAR    8
#define PT_ENTRY_IP_MAX_CHAR     16
#define PT_ENTRY_IF_MAX_CHAR     512  /* interface */

typedef struct
{
    char key[PT_ENTRY_TMP_MAX_CHAR];
    char enable[PT_ENTRY_TMP_MAX_CHAR];     /* boolean */
    char interface[PT_ENTRY_IF_MAX_CHAR];
    char des[PT_ENTRY_DES_MAX_SIZE];
    char client_ip[PT_ENTRY_IP_MAX_CHAR];   /* string  */
    char out_proto[PT_ENTRY_TMP_MAX_CHAR];   /* TCP, UDP */
    char out_sp[PT_ENTRY_TMP_MAX_CHAR];     /* 0-65535 */
    char out_ep[PT_ENTRY_TMP_MAX_CHAR];
    char in_proto[PT_ENTRY_TMP_MAX_CHAR];
    char in_sp[PT_ENTRY_TMP_MAX_CHAR];
    char in_ep[PT_ENTRY_TMP_MAX_CHAR];
}cal_pt_entry;


int cal_pt_add_one_entry(cal_pt_entry* entry);
int cal_pt_del_one_entry(cal_pt_entry* entry);
int cal_pt_get_one_entry(cal_pt_entry* entry);
int cal_pt_get_all_entry(void* id, cal_pt_entry** entry);
int cal_pt_del_all_entry(void* id);
int cal_pt_get_entry_num(void);
int cal_pt_get_entry_array(int ** index_list);
int cal_pt_modify_one_entry(cal_pt_entry* entry);
char * cal_pt_get_uri(void);
#endif

