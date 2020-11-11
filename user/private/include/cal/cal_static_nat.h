
#define CAL_STATICNAT_TMP_MAX_CHAR     8
#define CAL_STATICNAT_IP_MAX_CHAR      18
#define CAL_STATICNAT_PORT_MAX_CHAR    8 
#define CAL_STATICNAT_STRING_MAX_CHAR  128 


typedef struct
{
    char key[CAL_STATICNAT_TMP_MAX_CHAR];
    char enable[CAL_STATICNAT_TMP_MAX_CHAR];
    char servicename[CAL_STATICNAT_STRING_MAX_CHAR];
    char protocol[CAL_STATICNAT_TMP_MAX_CHAR];
    char publicip[CAL_STATICNAT_IP_MAX_CHAR];
    char publicport[CAL_STATICNAT_PORT_MAX_CHAR];
    char publicport_end[CAL_STATICNAT_PORT_MAX_CHAR];
    char privateip[CAL_STATICNAT_IP_MAX_CHAR];
    char privateport[CAL_STATICNAT_PORT_MAX_CHAR];
    char privateport_end[CAL_STATICNAT_PORT_MAX_CHAR];
}cal_static_nat; 




int cal_static_nat_add_one_entry(cal_static_nat* entry);
int cal_static_nat_get_one_entry(cal_static_nat* entry);
int cal_static_nat_get_all_entry(cal_static_nat** entry);
int cal_static_nat_del_one_entry(cal_static_nat* entry);
int cal_static_nat_del_all_entry(void);
int cal_static_nat_del_entry_array(int** index_array);
int cal_static_nat_modify_one_entry(cal_static_nat* entry);
char* cal_static_nat_get_enable(void);
int cal_static_nat_set_enable(char* value);
int cal_static_nat_set_rule_enable(char* value);
char* cal_static_nat_get_host(void);
int cal_static_nat_set_host(char* value);
int cal_static_nat_get_bind_if(void);
