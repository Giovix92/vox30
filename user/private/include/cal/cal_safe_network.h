#ifndef _CAL_SAFE_NETWORK_H_
#define _CAL_SAFE_NETWORK_H_

int cal_comvrs_set_link_account(char *value);
char *cal_comvrs_get_link_account(void);
int cal_comvrs_set_urlportal(char *value);
char *cal_comvrs_get_urlportal(void);
int cal_comvrs_set_active(char *value);	
char *cal_comvrs_get_active(void);
int cal_comvrs_set_market(char *value);
char *cal_comvrs_get_market(void);
int cal_comvrs_set_wanmode(char *value);
char *cal_comvrs_get_wanmode(void);
int cal_comvrs_set_wan_lastmode(char *value);
char *cal_comvrs_get_wan_lastmode(void);
int cal_comvrs_set_premium(char *value);
char *cal_comvrs_get_premium(void);
int cal_comvrs_set_msisdn(char *value);
char *cal_comvrs_get_msisdn(void);
int cal_comvrs_set_enable(char *value);
char *cal_comvrs_get_enable(void);
#ifdef CONFIG_SUPPORT_FWA
int cal_comvrs_set_fwa_used(char *value);
char *cal_comvrs_get_fwa_used(void);
#endif
#endif

