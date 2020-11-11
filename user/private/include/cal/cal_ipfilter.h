#ifndef _CAL_IPFILTER_H
#define _CAL_IPFILTER_H


typedef struct
{
    char key[8];
    int  is_input;
    int  is_ipv4;
    char name[64];
    char enable[8];
    char policy[8];
    char srcip[64];
    char srcmask[64];
    char srcport_range[16];
    char dstip[64];
    char dstmask[64];
    char dstport_range[16];
    char protocol[8];
    char tos_range[8];    
}cal_ipfilter_rule;


typedef struct
{
    char key[8];
    char name[64];
    char if_list[1024]; 
}cal_ipfilter_set;

int cal_ipfilter_add_rule(cal_ipfilter_rule* rule, int set_index);
int cal_ipfilter_insert_rule(cal_ipfilter_rule* rule, int set_index);
int cal_ipfilter_modify_rule(cal_ipfilter_rule* rule, int set_index);
int cal_ipfilter_del_rule(cal_ipfilter_rule* rule, int set_index);
int cal_ipfilter_get_rule_num(int set_index);
int cal_ipfilter_get_one_rule(cal_ipfilter_rule* rule, int set_index);
int cal_ipfilter_get_ipv4_input_rules(cal_ipfilter_rule** p_rule, int set_index);
int cal_ipfilter_get_ipv4_output_rules(cal_ipfilter_rule** p_rule, int set_index);
int cal_ipfilter_get_ipv6_input_rules(cal_ipfilter_rule** p_rule, int set_index);
int cal_ipfilter_get_ipv6_output_rules(cal_ipfilter_rule** p_rule, int set_index);
char* cal_ipfilter_get_max_num(void);

/** set **/
int cal_ipfilter_get_set_index(char *pattern);
int cal_ipfilter_add_set(cal_ipfilter_set* p_set);
int cal_ipfilter_modify_set(int set_index, cal_ipfilter_set* p_set);
int cal_ipfilter_del_set(int set_index);
int cal_ipfilter_get_one_set(int set_index, cal_ipfilter_set* p_set);
int cal_ipfilter_get_all_set(cal_ipfilter_set** p_set);

#endif

