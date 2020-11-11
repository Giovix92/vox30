
#ifndef __SAL_WIZARD_H__
#define __SAL_WIZARD_H__

char *sal_wizard_get_user_choose_t(void);
int sal_wizard_set_user_choose_t(char *value);

char *sal_wizard_get_user_start_t(void);
int sal_wizard_set_user_start_t(char *value);

char *sal_wizard_get_ppp_start_t(void);
int sal_wizard_set_ppp_start_t(char *value);

char *sal_wizard_get_wait_time_t(void);
int sal_wizard_set_wait_time_t(char *value);

char *sal_wizard_get_old_client_t(void);
int sal_wizard_set_old_client_t(char *value);

char *sal_wizard_get_key_checking_t(void);
int sal_wizard_set_key_checking_t(char *value);
char *sal_wizard_get_redirected_once(void);
int sal_wizard_set_redirected_once(char *value);
char *sal_wizard_get_activation_dismiss(void);
int sal_wizard_set_activation_dismiss(char *value);
#endif
