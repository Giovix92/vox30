#ifndef __SAL_BGM_H__
#define __SAL_BGM_H__

int sal_bgm_set_configured(char *value);
char *sal_bgm_get_configured(void);

char *sal_bgm_get_port_pvid(int port);
char *sal_bgm_get_port_pri(int port);
#endif

