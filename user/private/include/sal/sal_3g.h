#ifndef __SAL_3G_H__
#define __SAL_3G_H__

#define XML_PATH "/mnt/1/sms/"
#define SAL_FLASH_MISC "/mnt/1/misc.sal"
#ifdef CONFIG_SUPPORT_HA
char *sal_3g_get_dngl_current_apn(void);
int sal_3g_set_dngl_current_apn(char *value);
#endif
char *sal_3g_get_conn_status(void);
int sal_3g_set_conn_status(char *value);
char *sal_3g_get_dngl_plug_time(void);
int sal_3g_set_dngl_plug_time(char *value);
char *sal_3g_get_dngl_status_file(void);
int sal_3g_set_dngl_status_file(char *value);
char *sal_3g_get_dngl_config_file(void);
int sal_3g_set_dngl_config_file(char *value);
char *sal_3g_get_dngl_save_pin(void);
int sal_3g_set_dngl_save_pin(char *value);
char *sal_3g_get_dngl_disable_pin(void);
int sal_3g_set_dngl_disable_pin(char *value);
char *sal_3g_get_dngl_pin(void);
int sal_3g_set_dngl_pin(char *value);
char *sal_3g_get_phy_status(void);
int sal_3g_set_phy_status(char *value);
char* sal_3g_get_unsupported_stick(void);
int sal_3g_set_unsupported_stick(char *value);
char *sal_3g_get_dngl_module_name(void);
int sal_3g_set_dngl_module_name(char *value);
char *sal_3g_get_dngl_data_if(void);
int sal_3g_set_dngl_data_if(char *value);
char *sal_3g_get_time_wait(void);
int sal_3g_set_time_wait(char *value);
char *sal_3g_get_page_id(void);
int sal_3g_set_page_id(char *value);
char *sal_3g_get_dngl_at_port(void);
int sal_3g_set_dngl_at_port(char *value);
char *sal_3g_get_supported_dngl_once_plugged(void);
int sal_3g_set_supported_dngl_once_plugged(char *value);
char *sal_3g_get_puk(void);
int sal_3g_set_puk(char *value);
char *sal_3g_get_backup_mode(void);
int sal_3g_set_backup_mode(char *value);
char *sal_3g_get_support_4g(void);
int sal_3g_set_support_4g(char *value);
char *sal_3g_get_support_voice(void);

int sal_3g_set_support_voice(char *value);
int sal_3g_set_showtime_dataps2G(char *);
int sal_3g_set_showtime_dataps3G(char *);
int sal_3g_set_showtime_dataps4G(char *);
int sal_3g_set_showtime_voiceps3G(char *);
int sal_3g_set_showtime_voiceps4G(char *);
char* sal_3g_get_showtime_dataps2G(void);
char* sal_3g_get_showtime_dataps3G(void);
char* sal_3g_get_showtime_dataps4G(void);
char* sal_3g_get_showtime_voiceps3G(void);
char* sal_3g_get_showtime_voiceps4G(void);

int sal_3g_set_showtime_voicecs2G(char *);
char* sal_3g_get_showtime_voicecs2G(void);
int sal_3g_set_showtime_voicecs3G(char *);
char* sal_3g_get_showtime_voicecs3G(void);

int sal_flash_3g_set_4gdata_yesterday(char *);
char* sal_flash_3g_get_4gdata_yesterday(void);
int sal_flash_3g_set_4gvoice_yesterday(char *);
char* sal_flash_3g_get_4gvoice_yesterday(void);
int sal_flash_3g_set_3gdata_yesterday(char *);
char* sal_flash_3g_get_3gdata_yesterday(void);
int sal_flash_3g_set_3gvoice_yesterday(char *);
char* sal_flash_3g_get_3gvoice_yesterday(void);
int sal_flash_3g_set_signal_strength_yesterday(char *);
char* sal_flash_3g_get_signal_strength_yesterday(void);
int sal_flash_3g_set_bearer_yesterday(char *);
char* sal_flash_3g_get_bearer_yesterday(void);
int sal_flash_3g_set_availability_dataps3g(char *);
char* sal_flash_3g_get_availability_dataps3g(void);
int sal_flash_3g_set_availability_voiceps3g(char *);
char* sal_flash_3g_get_availability_voiceps3g(void);
int sal_flash_3g_set_availability_dataps4g(char *);
char* sal_flash_3g_get_availability_dataps4g(void);
int sal_flash_3g_set_availability_voiceps4g(char *);
char* sal_flash_3g_get_availability_voiceps4g(void);
int sal_flash_3g_set_3glast_registered_time(char *);
char* sal_flash_3g_get_3glast_registered_time(void);
int sal_flash_3g_set_4glast_registered_time(char *);
char* sal_flash_3g_get_4glast_registered_time(void);
#endif

