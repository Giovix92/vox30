#ifndef _CAL_HPSA_H_
#define _CAL_HPSA_H_

char *cal_hpsa_get_cfg_ava_time_define(void);
void cal_hpsa_set_cfg_ava_time_define(char *value);
char *cal_hpsa_get_cfg_pin_code(void);
void cal_hpsa_set_cfg_pin_code(char *value);
char *cal_hpsa_get_dev_imsi(void);
void cal_hpsa_set_dev_imsi(char *value);
char *cal_hpsa_get_msisdn(void);
void cal_hpsa_set_msisdn(char *value);
char *cal_hpsa_get_cfg_3g_enable(void);
void cal_hpsa_set_cfg_3g_enable(char *value);
char *cal_hpsa_get_cfg_prefer_wireless_mode(void);
void cal_hpsa_set_cfg_prefer_wireless_mode(char *value);
#if 0
char *cal_hpsa_get_cfg_pin_enable(void);
void cal_hpsa_set_cfg_pin_enable(char *value);
#endif
char *cal_hpsa_get_cfg_save_pin_enable(void);
void cal_hpsa_set_cfg_save_pin_enable(char *value);
char *cal_hpsa_get_cfg_network_select(void);
void cal_hpsa_set_cfg_network_select(char *value);
char *cal_hpsa_get_cfg_network_operator(void);
void cal_hpsa_set_cfg_network_operator(char *value);
#ifdef CONFIG_SUPPORT_WAN_BACKUP
#ifdef CONFIG_SUPPORT_3G_BACKUP
char *cal_hpsa_get_switch_instant(void);
void cal_hpsa_set_switch_instant(char *value);
char *cal_hpsa_get_voice_switch_instant(void);
void cal_hpsa_set_voice_switch_instant(char *value);
char *cal_hpsa_get_switch_confirm(void);
void cal_hpsa_set_switch_confirm(char *value);
char *cal_hpsa_get_switch_d2h_cs(void);
void cal_hpsa_set_switch_d2h_cs(char *value);
char *cal_hpsa_get_switch_d2h_ps(void);
void cal_hpsa_set_switch_d2h_ps(char *value);
char *cal_hpsa_get_switch_h2d_cs(void);
void cal_hpsa_set_switch_h2d_cs(char *value);
char *cal_hpsa_get_switch_h2d_ps(void);
void cal_hpsa_set_switch_h2d_ps(char *value);
char *cal_hpsa_get_switch_icmp_times(void);
void cal_hpsa_set_switch_icmp_times(char *value);
char *cal_hpsa_get_switch_dns_times(void);
void cal_hpsa_set_switch_dns_times(char *value);
char *cal_hpsa_get_switch_icmp_timer(void);
void cal_hpsa_set_switch_icmp_timer(char *value);
char *cal_hpsa_get_switch_dns_timer(void);
void cal_hpsa_set_switch_dns_timer(char *value);
char *cal_hpsa_get_switch_voiceoverps(void);
char *cal_hpsa_get_switch_voiceoverps_sip_domain(void);
char *cal_hpsa_get_switch_sip_lookup_query_name(void);
void cal_hpsa_set_switch_voiceoverps(char *value);
char *cal_hpsa_get_gui_3g_enable(void);
void cal_hpsa_set_gui_3g_enable(char *value);
char* cal_hpsa_get_3g_switch_uri(void);
char* cal_hpsa_get_3g_apn_uri(void);

char *cal_sms_get_msisdn(void);
int cal_sms_set_msisdn(char *value);
char *cal_sms_get_phonenumber(void);
int cal_sms_set_phonenumber(char *value);
#endif
#endif
#endif
