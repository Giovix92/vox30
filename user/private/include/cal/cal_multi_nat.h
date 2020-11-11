
#define CAL_MULTINAT_TMP_MAX_CHAR     8
#define CAL_MULTINAT_IP_MAX_CHAR      18
#define CAL_MULTINAT_PORT_MAX_CHAR    8 
#define CAL_MULTINAT_STRING_MAX_CHAR  128 


typedef struct
{
    char key[CAL_MULTINAT_TMP_MAX_CHAR];
    char enable[CAL_MULTINAT_TMP_MAX_CHAR];
    char interface[CAL_MULTINAT_STRING_MAX_CHAR];
    char type[CAL_MULTINAT_STRING_MAX_CHAR];
    char publicstartip[CAL_MULTINAT_IP_MAX_CHAR];
    char publicendip[CAL_MULTINAT_IP_MAX_CHAR];
    char privatestartip[CAL_MULTINAT_IP_MAX_CHAR];
    char privateendip[CAL_MULTINAT_IP_MAX_CHAR];
}cal_multi_nat; 




int cal_multi_nat_add_one_entry(cal_multi_nat* entry);
int cal_multi_nat_get_one_entry(cal_multi_nat* entry);
int cal_multi_nat_get_all_entry(cal_multi_nat** entry);
int cal_multi_nat_del_one_entry(cal_multi_nat* entry);
int cal_multi_nat_del_all_entry(void);
int cal_multi_nat_del_entry_array(int** index_array);
int cal_multi_nat_modify_one_entry(cal_multi_nat* entry);
char* cal_multi_nat_get_enable(void);
int cal_multi_nat_set_enable(char* value);
int cal_multi_nat_set_rule_enable(char* value);
char* cal_multi_nat_get_host(void);
int cal_multi_nat_set_host(char* value);
int cal_multi_nat_get_bind_if(void);
int cal_multi_nat_get_all_entry_x(cal_multi_nat** entry, int wan_id);
