#ifndef __CAL_PCP_H_
#define __CAL_PCP_H_

struct pcp_sinfo
{
    int sid;
    int enable;
    char ip[254];
    char username[254];
    char password[254];
};

int cal_pcp_get_enable(void);
int cal_pcp_set_enable(int status);
char* cal_pcp_get_auth_method(void);
int cal_pcp_set_auth_method(char *value);
int cal_pcp_add_one_server_entry(struct pcp_sinfo *entry);
int cal_pcp_del_one_server_entry(int index);
int cal_pcp_del_all_server_entry(void);
int cal_pcp_get_all_server_entry(struct pcp_sinfo **entry);

#endif
