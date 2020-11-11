#ifndef __CAL_MISC_H__
#define __CAL_MISC_H__
#ifdef CONFIG_SUPPORT_WOLAN
char *cal_misc_get_WoLAN_enable();
int cal_misc_set_WoLAN_enable(char *value);
char *cal_misc_get_WoLANListenPort();
int cal_misc_set_WoLANListenPort(char *value);
char *cal_misc_get_WoLANSendPort();
int cal_misc_set_WoLANSendPort(char *value);
#endif
#ifdef CONFIG_SUPPORT_AUTO_SENSE
int cal_autosense_set_enable(char *value);
char *cal_autosense_get_enable(void);
int cal_autosense_set_force_link(char *value);
char *cal_autosense_get_force_link(void);
#endif
int cal_misc_revert_old_encryption(void);
#ifdef CONFIG_SUPPORT_BOOSTER_MODE
int cal_get_booster_mode_addrlist_aviable_index(int **array);
char *cal_get_booster_mode_enable(void);
char *cal_get_booster_mode_addrlist_num(void);
char *cal_get_booster_mode_addrlist_ip(int index);
char *cal_get_booster_mode_addrlist_mask(int index);
#endif

#ifdef VOX25
char *cal_get_power_led_enable(void);
char *cal_get_status_led_enable(void);
char *cal_get_power_led_brightness(void);
int cal_set_power_led_enable(char *value);
int cal_set_status_led_enable(char *value);
int cal_set_power_led_brightness(char *value);
#endif
char *cal_misc_get_networkmap_enable(void);
int   cal_misc_set_networkmap_enable(char *value);
char *cal_misc_get_networkmap_vif_enable(void);
int   cal_misc_set_networkmap_vif_enable(char *value);
#ifdef CONFIG_SUPPORT_PRPL_HL_API
char *cal_misc_get_networkmap_history_enable(void);
int cal_misc_set_networkmap_history_enable(char *value);
#endif

int cal_gpon_set_debug_mode(char *value);
char *cal_gpon_get_debug_mode(void);
int cal_gpon_set_trace_enable(char *value);
char *cal_gpon_get_trace_enable(void);
int cal_gpon_set_log_enable(char *value);
char *cal_gpon_get_log_enable(void);
int cal_gpon_set_compat_OLT(char *value);
char *cal_gpon_get_compat_OLT(void);
int cal_gpon_set_olt_permanent_sense(char *value);
char *cal_gpon_get_olt_permanent_sense(void);
int cal_gpon_set_tr247_enable(char *value);
char *cal_gpon_get_tr247_enable(void);
int cal_gpon_set_rogueont(char *value);
char *cal_gpon_get_rogueont(void);
int cal_gpon_set_rx_threshold(char *value);
char *cal_gpon_get_rx_threshold(void);
int cal_gpon_set_ont_mode(char *value);
char *cal_gpon_get_ont_mode(void);
int cal_gpon_set_ericsson_ont_type(char *value);
char *cal_gpon_get_ericsson_ont_type(void);
int cal_gpon_set_huawei_ont_type(char *value);
char *cal_gpon_get_huawei_ont_type(void);
int cal_gpon_set_zte_ont_type(char *value);
char *cal_gpon_get_zte_ont_type(void);
int cal_gpon_set_traffic_mgmt_opt(char *value);
char *cal_gpon_get_traffic_mgmt_opt(void);
int cal_gpon_set_single_uni_enable(char *value);
char *cal_gpon_get_single_uni_enable(void);

char *cal_nat_get_hw_enable(void);
int cal_nat_set_hw_enable(char *value);
char *cal_nat_get_loose_enable(void);
int cal_nat_set_loose_enable(char *value);
char *cal_nat_get_fullcone_enable(void);
int cal_nat_set_fullcone_enable(char *value);


#ifdef CONFIG_SUPPORT_SECURE_NET
char *cal_get_dns_redirect_enable(void);
int cal_set_dns_redirect_enable(char *value);
#endif
char *cal_get_sdns_enable(void);
int cal_set_sdns_enable(char *value);
char *cal_get_sdns_configurealldevices(void);
int cal_set_sdns_configurealldevices(char *value);

char *cal_get_ddns_enable(void);
int cal_set_ddns_enable(char *value);
char *cal_get_ddns_wildcard_enable(void);
int cal_set_ddns_wildcard_enable(char *value);
char *cal_get_ddns_service_provide(void);
int cal_set_ddns_service_provide(char *value);
char *cal_get_ddns_supported_providers(void);
char *cal_get_dyndns_user(void);
int cal_set_dyndns_user(char *value);
char *cal_get_dyndns_pwd(void);
int cal_set_dyndns_pwd(char *value);
char *cal_get_dyndns_hostname(void);
int cal_set_dyndns_hostname(char *value);
char *cal_get_tzo_user(void);
int cal_set_tzo_user(char *value);
char *cal_get_tzo_pwd(void);
int cal_set_tzo_pwd(char *value);
char *cal_get_tzo_hostname(void);
int cal_set_tzo_hostname(char *value);
char *cal_get_dynip_key(void);
int cal_set_dynip_key(char *value);
char *cal_get_dynip_hostname(void);
int cal_set_dynip_hostname(char *value);
char *cal_get_dynip_interface(void);
int cal_set_ddns_interface(int wan_id);
char *cal_get_custom_user(void);
int cal_set_custom_user(char *value);
char *cal_get_custom_pwd(void);
int cal_set_custom_pwd(char *value);
char *cal_get_custom_hostname(void);
int cal_set_custom_hostname(char *value);

char *cal_get_noip_user(void);
int cal_set_noip_user(char *value);
char *cal_get_noip_pwd(void);
int cal_set_noip_pwd(char *value);
char *cal_get_noip_hostname(void);
int cal_set_noip_hostname(char *value);
char * cal_get_ddns_uri(void);

char *cal_catv_get_enable(void);
int cal_catv_set_enable(char *value);
void cal_catv_hide_parameter(void);
char *cal_catv_get_mode(void);
int cal_catv_set_mode(char *value);
char *cal_catv_get_filter(void);
int cal_catv_set_filter(char *value);

int cal_UIface_set_current_language(char *value);
char *cal_UIface_get_current_language(void);

char *cal_get_boost_enable(void);
int cal_set_boost_enable(char *value);
char *cal_get_setup_wizard_finished(void);
int cal_set_setup_wizard_finished(char *value);
char *cal_get_setup_wizard_enable(void);
int cal_set_setup_wizard_enable(char *value);

char *cal_get_fail_login_attempts(void);
int cal_set_fail_login_attempts(char *value);
char *cal_get_fail_login_timeoutmax(void);
int cal_set_fail_login_timeoutmax(char *value);
char *cal_get_fail_login_timeout_increment(void);
int cal_set_fail_login_timeout_increment(char *value);
#ifdef CONFIG_SUPPORT_WEBAPI
char *cal_get_boost_mac(void);
int cal_set_boost_mac(char *value);
char *cal_get_boost_duration(void);
int cal_set_boost_duration(char *value);
#endif
#ifdef CONFIG_SUPPORT_BRIDGE_MODE
char *cal_get_bridge_mode_enable(void);
int cal_set_bridge_mode_enable(char *value);
#endif
#ifdef CONFIG_SUPPORT_NEW_LED_BEHAV
char *cal_get_new_led_behav_enable(void);
char *cal_get_new_led_behav_customer(void);
#endif
#ifdef CONFIG_SUPPORT_PRPL_HL_API
char *cal_get_sysled_name(int s_id);
int cal_set_sysled_name(char *value, int index);
char *cal_get_sysled_enable(int s_id);
int cal_set_sysled_enable(char *value, int index);
char *cal_get_sysbutton_id(int s_id);
int cal_set_sysbutton_id(char *value, int index);
char *cal_get_sysbutton_name(int s_id);
int cal_set_sysbutton_name(char *value, int index);
char *cal_get_sysbutton_enable(int s_id);
int cal_set_sysbutton_enable(char *value, int index);
char *cal_get_sysbutton_click_object(int s_id);
int cal_set_sysbutton_click_object(char *value, int index);
char *cal_get_sysbutton_click_method(int s_id);
int cal_set_sysbutton_click_method(char *value, int index);
char *cal_get_sysbutton_click_argument(int s_id);
int cal_set_sysbutton_click_argument(char *value, int index);
char *cal_get_sysbutton_press_object(int s_id);
int cal_set_sysbutton_press_object(char *value, int index);
char *cal_get_sysbutton_press_method(int s_id);
int cal_set_sysbutton_press_method(char *value, int index);
char *cal_get_sysbutton_press_argument(int s_id);
int cal_set_sysbutton_press_argument(char *value, int index);
#endif
char *cal_get_statsd_update_flash_interval(void);
#ifdef CONFIG_SUPPORT_SECURE_NET
char *cal_get_secure_net_enable(void);
int cal_set_secure_net_enable(char *value);
char *cal_get_edns0_interfaces(void);
void cal_set_edns0_interfaces(char *value);
char *cal_get_edns0_macaddrtagging(void);
void cal_set_edns0_macaddrtagging(char *value);
#endif
#endif /* _CAL_MISC_H_ */

