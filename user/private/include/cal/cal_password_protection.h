#ifndef __CAL_PASSWORD_PROTECTION_H__
#define __CAL_PASSWORD_PROTECTION_H__

int cal_password_protection_set_enable(char *value);
char *cal_password_protection_get_enable(void);
int cal_password_protection_set_pageblock(char *value);
char *cal_password_protection_get_pageblock(void);
int cal_password_protection_set_useremailaddress(char *value);
char *cal_password_protection_get_useremailaddress(void);
int cal_password_protection_set_localemailaddress(char *value);
char *cal_password_protection_get_localemailaddress(void);
int cal_password_protection_set_localemailusername(char *value);
char *cal_password_protection_get_localemailusername(void);

int cal_password_protection_set_localemailport(char *value);
char *cal_password_protection_get_localemailport(void);
int cal_password_protection_set_localemailserver(char *value);
char *cal_password_protection_get_localemailserver(void);
#endif
