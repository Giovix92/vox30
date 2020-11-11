#ifndef _CAL_DNSR_H_
#define _CAL_DNSR_H_

typedef struct 
{
    char domain[256];
    char interface[128];
    char dtype[128];
}cal_dns_route;

int cal_dnsr_add_entry(cal_dns_route* entry);
int cal_dnsr_del_entry(cal_dns_route* entry);
int cal_dnsr_get_all_entry(cal_dns_route** entry);
int cal_dnsr_del_all_entry(void);

#endif // _CAL_DNSR_H_H

