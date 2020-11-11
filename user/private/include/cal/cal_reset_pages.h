#ifndef __CAL_RESET_PAGES_H__
#define __CAL_RESET_PAGES_H__

/* ---------------- Internet -------------------- */
int cal_internet_fireware_page_reset();
int cal_internet_portmapping_page_reset();
int cal_internet_exposedhost_page_reset();
int cal_internet_dns_page_reset();
/* ---------------- Wi-Fi -------------------- */
int cal_wifi_general_page_reset();
int cal_wifi_scheduler_page_reset();
int cal_wifi_wps_page_reset();
int cal_wifi_macfilter_page_reset();
int cal_wifi_bandsteering_page_reset();
int cal_wifi_settings_page_reset();
/* ---------------- Settings -------------------- */
int cal_setting_energy_settings_page_reset();
int cal_setting_configuration_page_reset();
int cal_setting_lan_page_reset();
/* ---------------- function for config backuop ---------------- */
char *cal_setting_config_overview_restore(int is_admin);
char *cal_setting_config_phone_restore(int is_admin);
char *cal_setting_config_internet_restore(int is_admin);
char *cal_setting_config_wifi_restore(int is_admin);
char *cal_setting_config_sharing_restore(int is_admin);
char *cal_setting_config_settings_restore(int is_admin);
#endif
