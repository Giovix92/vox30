#ifndef _SAL_SYSTEM_INFO_H
#define _SAL_SYSTEM_INFO_H

#define SAL_SYSINFO "/tmp/sal/sysinfo.sal"
#define SAL_FLASH_SYSINFO "/mnt/1/sysinfo.sal"
double sal_get_mem_utilization(void);
int sal_get_mem_total(void);
int sal_get_mem_free(void);
char *sal_flash_sysinfo_get_uptime_start(void);
int sal_flash_sysinfo_set_uptime_start(char *value);

/* interval means  time interval of cpu utilization test*/
double sal_get_cpu_utilization(int interval);
int sal_get_cpuusage(void);

#endif
