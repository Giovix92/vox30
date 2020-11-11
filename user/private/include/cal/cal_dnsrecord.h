#define CAL_DNSRECORD_TMP_MAX_CHAR     8
#define CAL_DNSRECORD_MAC_MAX_CHAR     18
#define CAL_DNSRECORD_DOMAIN_MAX_CHAR  128 
#define CAL_DNSRECORD_MAX_ENTRY_NUM    20 


typedef struct
{
    char key[CAL_DNSRECORD_TMP_MAX_CHAR];
    char enable[CAL_DNSRECORD_TMP_MAX_CHAR];
    char domain[CAL_DNSRECORD_DOMAIN_MAX_CHAR];
    char mac[CAL_DNSRECORD_MAC_MAX_CHAR];
}cal_dns_record; 




int cal_dr_add_one_entry(cal_dns_record* entry);
int cal_dr_get_one_entry(cal_dns_record* entry);
int cal_dr_get_all_entry(cal_dns_record** entry);
int cal_dr_del_one_entry( cal_dns_record* entry);
int cal_dr_del_all_entry(void);
int cal_dr_del_entry_array(int** index_array);
int cal_dr_modify_one_entry( cal_dns_record* entry);    
