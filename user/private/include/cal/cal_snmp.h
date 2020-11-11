#ifndef _CAL_SNMP_H_
#define _CAL_SNMP_H_

char *cal_snmp_get_enable();
char *cal_snmp_get_version();
char *cal_snmp_get_readcomm();
char *cal_snmp_get_writecomm();
char *cal_snmp_get_sysname();
char *cal_snmp_get_syscontact();
char *cal_snmp_get_syslocation();
char *cal_snmp_get_sysdescr();
char *cal_snmp_get_trap_ip();
char *cal_snmp_get_username(void);
char *cal_snmp_get_auth_protocol(void);
char *cal_snmp_get_auth_key(void);
char *cal_snmp_get_privacy_protocol(void);
char *cal_snmp_get_privacy_key(void);

int cal_snmp_set_enable();
int cal_snmp_set_version();
int cal_snmp_set_readcomm();
int cal_snmp_set_writecomm();
int cal_snmp_set_sysname();
int cal_snmp_set_syscontact();
int cal_snmp_set_sysdescr();
int cal_snmp_set_syslocation();
int cal_snmp_set_trap_ip();
int cal_snmp_set_username(char *val);
int cal_snmp_set_auth_protocol(char *val);
int cal_snmp_set_auth_key(char *val);
int cal_snmp_set_privacy_protocol(char *val);
int cal_snmp_set_privacy_key(char *val);
#endif
