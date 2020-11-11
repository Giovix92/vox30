#ifndef _CAL_DEVINFO_H_
#define _CAL_DEVINFO_H_

#include "utility.h"
#include "sal/sal_process.h"

char *cal_dev_get_name(void);
char *cal_dev_get_manufacturer(void);
char *cal_dev_get_manufaurl(void);
char *cal_dev_get_manufaoui(void);
char *cal_dev_get_modelname(void);
char *cal_dev_get_modelnum(void);
char *cal_dev_get_modelurl(void);
char *cal_dev_get_descrip(void);
char *cal_dev_get_sn(void);
char *cal_dev_get_hwver(void);
char *cal_dev_get_swver(void);
char *cal_dev_get_fwver(void);
char *cal_dev_get_provision(void);
int cal_dev_set_provision(char *value);
char *cal_dev_get_uptime(void);
char *cal_dev_get_memtotal(void);
char *cal_dev_get_memfree(void);
char *cal_dev_get_cpuusage(void);
char *cal_dev_get_processnum(void);
char *cal_dev_get_reboot_cause(void);
int cal_dev_set_reboot_cause(char *value);
char * cal_dev_get_first_use_date(void);
int cal_dev_set_first_use_date(char *value);
#ifdef CONFIG_SUPPORT_WIZARD
int cal_dev_get_provisioned(void);
#endif
char *cal_dev_get_remote_dnsalias(void);
char *cal_dev_get_remote_access_enabled(void);
int cal_dev_set_one_process(int index,pro_t *pro);
int cal_dev_process_add_entry(pro_t * pro);
int cal_dev_process_del_entry(void);
#ifdef CONFIG_SUPPORT_FWA
char *cal_dev_get_imsi(void);
int cal_dev_set_imsi(char *value);
#endif
#endif
