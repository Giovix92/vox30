#ifndef __SAL_TR069_H__
#define __SAL_TR069_H__

char *sal_tr069_get_acs_url(void);
int sal_tr069_set_acs_url(char *value);
char *sal_tr069_get_trml(int index);
int sal_tr069_set_trml(char *value, int index);
char *sal_http_get_trml(int index);
int sal_http_set_trml(char *value, int index);
#endif
