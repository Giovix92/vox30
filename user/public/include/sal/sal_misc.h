#ifndef __SAL_MISC_H__
#define __SAL_MISC_H__

#define DNS_RESOLVE "1"
#define PING_TRY    "2"
#define PING_EXITE  "0"
#define SAL_WPS_STATUS_SUCCESS  "2"
#define WPS_STATUS_FAILED  "3"
char *sal_misc_get_ipping_diag_succ_count(void);
int sal_misc_set_ipping_diag_succ_count(char *value);
char *sal_misc_get_ipping_diag_fail_count(void);
int sal_misc_set_ipping_diag_fail_count(char *value);
char *sal_misc_get_ipping_diag_avg_resp_time(void);
int sal_misc_set_ipping_diag_avg_resp_time(char *value);
char *sal_misc_get_ipping_diag_min_resp_time(void);
int sal_misc_set_ipping_diag_min_resp_time(char *value);
char *sal_misc_get_ipping_diag_max_resp_time(void);
int sal_misc_set_ipping_diag_max_resp_time(char *value);

char *sal_misc_get_board_cpu_info(void);
int sal_misc_set_board_cpu_info(char *value);
char *sal_misc_get_board_cpu(void);
int sal_misc_set_board_cpu(char *value);
char *sal_misc_get_board_manufacture(void);
int sal_misc_set_board_manufacture(char *value);
char *sal_misc_get_board_wifi_psk(void);
int sal_misc_set_board_wifi_psk(char *value);
char *sal_misc_get_board_pin(void);
int sal_misc_set_board_pin(char *value);
char *sal_misc_get_board_model_des(void);
int sal_misc_set_board_model_des(char *value);
char *sal_misc_get_board_model_name(void);
int sal_misc_set_board_model_name(char *value);
char *sal_misc_get_board_model_url(void);
int sal_misc_set_board_model_url(char *value);
char *sal_misc_get_board_soft_vendor(void);
int sal_misc_set_board_soft_vendor(char *value);
char *sal_misc_get_board_manufacture_url(void);
int sal_misc_set_board_manufacture_url(char *value);
char *sal_misc_get_board_pid(void);
int sal_misc_set_board_pid(char *value);
char *sal_misc_get_board_csn(void);
int sal_misc_set_board_csn(char *value);
char *sal_misc_get_board_pcba_sn(void);
int sal_misc_set_board_pcba_sn(char *value);
char *sal_misc_get_board_hw_version(void);
int sal_misc_set_board_hw_version(char *value);
char *sal_misc_get_board_fw_version(void);
int sal_misc_set_board_fw_version(char *value);
char *sal_misc_get_board_lib_version(void);
int sal_misc_set_board_lib_version(char *value);
char *sal_misc_get_board_wifi_ssid(void);
int sal_misc_set_board_wifi_ssid(char *value);
char *sal_misc_get_board_boot_version(void);
int sal_misc_set_board_boot_version(char *value);
char *sal_misc_get_board_hw_mac(void);
int sal_misc_set_board_hw_mac(char *value);
char *sal_misc_get_board_hw_id(void);
int sal_misc_set_board_hw_id(char *value);
char *sal_misc_get_board_manufacture_oui(void);
int sal_misc_set_board_manufacture_oui(char *value);
char *sal_misc_get_board_model_number(void);
int sal_misc_set_board_model_number(char *value);
char *sal_misc_get_board_product_class(void);
int sal_misc_set_board_product_class(char *value);
char *sal_misc_get_board_build_time(void);
int sal_misc_set_board_build_time(char *value);
char *sal_misc_get_board_build_tag(void);
int sal_misc_set_board_build_tag(char *value);
char *sal_misc_get_wps_client_pin(void);
int sal_misc_set_wps_client_pin(char *value);
char *sal_misc_get_wps_status(void);
int sal_misc_set_wps_status(char *value);
#ifdef CONFIG_SUPPORT_WEBAPI
char *sal_misc_get_speed_test_upload_speed(void);
int sal_misc_set_speed_test_upload_speed(char *value);
char *sal_misc_get_speed_test_download_speed(void);
int sal_misc_set_speed_test_download_speed(char *value);
char *sal_misc_get_speed_test_status(void);
int sal_misc_set_speed_test_status(char *value);
#endif

char *sal_misc_get_ddns_status(void);
int sal_misc_set_ddns_status(char *value);
char *sal_misc_get_upgrade_status(void);
int sal_misc_set_upgrade_status(char *value);
#ifdef CONFIG_SUPPORT_MEDIA_SERVER
char *sal_misc_get_media_status(void);
int sal_misc_set_media_status(char *value);
#endif
#ifdef CONFIG_SUPPORT_BBU
char *sal_misc_get_bbu_status(void);
int sal_misc_set_bbu_status(char *value);
#endif
char *sal_gpon_get_vif_status(void);
int sal_gpon_set_vif_status(char *value);

char *sal_misc_get_ping_status(void);
int sal_misc_set_ping_status(char *value);
char *sal_misc_get_ping_result_line(void);
int sal_misc_set_ping_result_line(char *value);
char *sal_misc_get_ping_pid(void);
int sal_misc_set_ping_pid(char *value);
char *sal_misc_get_autosense_start_time(void);
int sal_misc_set_autosense_start_time(char *value);
int sal_misc_get_autosense_running_time(void);
char *sal_misc_get_reboot_cause(void);
#ifdef CONFIG_SUPPORT_ADMIN_BACK_DOOR
char *sal_misc_get_superuser_language(void);
int sal_misc_set_superuser_language(char *value);
#endif
char *sal_misc_get_current_user(char *remote_ip);
int sal_misc_set_current_user(char *remote_ip, char *value);
char *sal_misc_get_logout_user(char *remote_ip);
int sal_misc_set_logout_user(char *remote_ip, char *value);
int sal_misc_set_rc_status(char *value, const char *format, ...);
char *sal_misc_get_rc_status(const char *format, ...);

char *sal_misc_get_reset_button_status(void);
int sal_misc_set_reset_button_status(char *value);
char *sal_misc_get_reset_status(void);
int sal_misc_set_reset_status(char *value);
char *sal_misc_get_wps_button_status(void);
int sal_misc_set_wps_button_status(char *value);
char *sal_misc_get_wifi_button_status(void);
int sal_misc_set_wifi_button_status(char *value);
char *sal_misc_get_ipping_diag_succ_count_by_wan(char* wan_id);
int sal_misc_set_ipping_diag_succ_count_by_wan(char* wan_id, char *value);

char *sal_misc_get_config_computer_restore(void);
int sal_misc_set_config_computer_restore(char *value);
char *sal_misc_get_config_computer_restore_type(void);
int sal_misc_set_config_computer_restore_type(char *value);
char *sal_misc_get_board_modem_fw_version(void);
int sal_misc_set_board_modem_fw_version(char *value);
char *sal_login_get_session_id(char* remote_ip);
int sal_login_set_session_id(char* remote_ip, char *value);
char *sal_login_get_last_session_id1(char* remote_ip);
int sal_login_set_last_session_id1(char* remote_ip, char *value);
char *sal_login_get_last_session_id2(char* remote_ip);
int sal_login_set_last_session_id2(char* remote_ip, char *value);
char *sal_login_get_last_session_id3(char* remote_ip);
int sal_login_set_last_session_id3(char* remote_ip, char *value);
char *sal_login_get_last_session_id4(char* remote_ip);
int sal_login_set_last_session_id4(char* remote_ip, char *value);
char *sal_login_get_last_session_id5(char* remote_ip);
int sal_login_set_last_session_id5(char* remote_ip, char *value);
char *sal_login_get_last_session_id1_uptime(char* remote_ip);
int sal_login_set_last_session_id1_uptime(char* remote_ip, char *value);
char *sal_login_get_last_session_id2_uptime(char* remote_ip);
int sal_login_set_last_session_id2_uptime(char* remote_ip, char *value);
char *sal_login_get_last_session_id3_uptime(char* remote_ip);
int sal_login_set_last_session_id3_uptime(char* remote_ip, char *value);
char *sal_login_get_last_session_id4_uptime(char* remote_ip);
int sal_login_set_last_session_id4_uptime(char* remote_ip, char *value);
char *sal_login_get_last_session_id5_uptime(char* remote_ip);
int sal_login_set_last_session_id5_uptime(char* remote_ip, char *value);
char *sal_login_get_csrf_token(char* remote_ip);
int sal_login_set_csrf_token(char* remote_ip, char *value);
char *sal_login_get_last_csrf_token(char* remote_ip);
int sal_login_set_last_csrf_token(char* remote_ip, char *value);
char *sal_login_get_csrf_token_hidden(char* remote_ip);
int sal_login_set_csrf_token_hidden(char* remote_ip, char *value);
int sal_login_set_failed_times(char *value);
char *sal_login_get_failed_times(void);
int sal_login_set_ssh_failed_times(char *value);
char *sal_login_get_ssh_failed_times(void);
char *sal_misc_http_session_id_generate(char* remote_ip);


#ifdef SUPPORT_USB_STORAGE
char *sal_misc_get_twonky_server_db_dir(void);
int sal_misc_set_twonky_server_db_dir(char *value);
#endif
char *sal_misc_get_tr069_boot_event(void);
int sal_misc_set_tr069_boot_event(char *value);
char *sal_misc_get_tr069_connect_to_acs_server(void);
int sal_misc_set_tr069_connect_to_acs_server(char *value);

char *sal_misc_get_auto_diag_result(void);
int sal_misc_set_auto_diag_result(char *value);
char *sal_misc_get_auto_diag_busy(void);
int sal_misc_set_auto_diag_busy(char *value);

char *sal_misc_get_auto_ping_result(void);
int sal_misc_set_auto_ping_result(char *value);
char *sal_misc_get_auto_ping_busy(void);
int sal_misc_set_auto_ping_busy(char *value);
#ifdef CONFIG_SUPPORT_HA
char *sal_misc_get_ha_state(void);
char *sal_misc_get_ha_status(void);
char *sal_misc_set_ha_status(char *value);
#endif
#ifdef CONFIG_SUPPORT_IPV6
char *sal_misc_get_ipv6_prefix_changed_t(void);
int sal_misc_set_ipv6_prefix_changed_t(char *value);
#endif
#endif /* __SAL_MISC_H__ */
