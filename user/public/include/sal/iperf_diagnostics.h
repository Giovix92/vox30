#ifndef _SAL_DIAGNOSTICS_H_
#define _SAL_DIAGNOSTICS_H_

char *sal_diag_get_upload_rom_time(void);
int sal_diag_set_upload_rom_time(char *value);
char *sal_diag_get_upload_bom_time(void);
int sal_diag_set_upload_bom_time(char *value);
char *sal_diag_get_upload_eom_time(void);
int sal_diag_set_upload_eom_time(char *value);
char *sal_diag_get_upload_test_bs(void);
int sal_diag_set_upload_test_bs(char *value);
char *sal_diag_get_upload_total_bs(void);
int sal_diag_set_upload_total_bs(char *value);
char *sal_diag_get_upload_total_br(void);
int sal_diag_set_upload_total_br(char *value);
char *sal_diag_get_upload_tcpopen_reqtime(void);
int sal_diag_set_upload_tcpopen_reqtime(char *value);
char *sal_diag_get_upload_tcpopen_restime(void);
int sal_diag_set_upload_tcpopen_restime(char *value);
char *sal_diag_get_upload_testbs_fl(void);
int sal_diag_set_upload_testbs_fl(char *value);
char *sal_diag_get_upload_totalbs_fl(void);
int sal_diag_set_upload_totalbs_fl(char *value);
char *sal_diag_get_upload_totalbr_fl(void);
int sal_diag_set_upload_totalbr_fl(char *value);
char *sal_diag_get_upload_period_fl(void);
int sal_diag_set_upload_period_fl(char *value);

char *sal_diag_get_download_rom_time(void);
int sal_diag_set_download_rom_time(char *value);
char *sal_diag_get_download_bom_time(void);
int sal_diag_set_download_bom_time(char *value);
char *sal_diag_get_download_eom_time(void);
int sal_diag_set_download_eom_time(char *value);
char *sal_diag_get_download_test_br(void);
int sal_diag_set_download_test_br(char *value);
char *sal_diag_get_download_total_br(void);
int sal_diag_set_download_total_br(char *value);
char *sal_diag_get_download_total_bs(void);
int sal_diag_set_download_total_bs(char *value);
char *sal_diag_get_download_tcpopen_reqtime(void);
int sal_diag_set_download_tcpopen_reqtime(char *value);
char *sal_diag_get_download_tcpopen_restime(void);
int sal_diag_set_download_tcpopen_restime(char *value);
char *sal_diag_get_download_testbr_fl(void);
int sal_diag_set_download_testbr_fl(char *value);
char *sal_diag_get_download_totalbr_fl(void);
int sal_diag_set_download_totalbr_fl(char *value);
char *sal_diag_get_download_totalbs_fl(void);
int sal_diag_set_download_totalbs_fl(char *value);
char *sal_diag_get_download_period_fl(void);
int sal_diag_set_download_period_fl(char *value);

char* sal_diag_get_ipping_complete(void);
int sal_diag_set_ipping_complete(char* value);
char* sal_diag_get_wifidump_complete(void);
int sal_diag_set_wifidump_complete(char* value);


char* sal_diag_get_download_complete(void);
int sal_diag_set_download_complete(char* value);
char* sal_diag_get_upload_complete(void);
int sal_diag_set_upload_complete(char* value);
char* sal_diag_get_traceroute_complete(void);
int sal_diag_set_traceroute_complete(char* value);
char* sal_diag_get_wifiradar_complete(void);
int sal_diag_set_wifiradar_complete(char* value);
char* sal_diag_get_nslookup_complete(void);
int sal_diag_set_nslookup_complete(char* value);
#ifdef CONFIG_SUPPORT_DSL
char* sal_diag_get_dsl_complete(void);
int sal_diag_set_dsl_complete(char* value);
char* sal_diag_get_atmf5_complete(void);
int sal_diag_set_atmf5_complete(char* value);
#endif

char *sal_diag_get_nslookup_start(void);
int sal_diag_set_nslookup_start(char *value);
char *sal_diag_get_traceroute_start(void);
int sal_diag_set_traceroute_start(char *value);
char *sal_diag_get_wifiradar_start(void);
int sal_diag_set_wifiradar_start(char *value);
char *sal_diag_get_ipping_start(void);
int sal_diag_set_ipping_start(char *value);

char *sal_diag_get_wifidump_start(void);
int sal_diag_set_wifidump_start(char *value);

char *sal_diag_get_download_start(void);
int sal_diag_set_download_start(char *value);
char *sal_diag_get_upload_start(void);
int sal_diag_set_upload_start(char *value);
#ifdef CONFIG_SUPPORT_DSL
char *sal_diag_get_dsl_start(void);
int sal_diag_set_dsl_start(char *value);
char *sal_diag_get_atmf5_start(void);
int sal_diag_set_atmf5_start(char *value);
#endif

char *sal_diag_get_upload_per_rom_time(int idx);
int sal_diag_set_upload_per_rom_time(char *value, int idx);
char *sal_diag_get_upload_per_bom_time(int idx);
int sal_diag_set_upload_per_bom_time(char *value, int idx);
char *sal_diag_get_upload_per_eom_time(int idx);
int sal_diag_set_upload_per_eom_time(char *value, int idx);
char *sal_diag_get_upload_per_test_bs(int idx);
int sal_diag_set_upload_per_test_bs(char *value, int idx);
char *sal_diag_get_upload_per_total_br(int idx);
int sal_diag_set_upload_per_total_br(char *value, int idx);
char *sal_diag_get_upload_per_total_bs(int idx);
int sal_diag_set_upload_per_total_bs(char *value, int idx);
char *sal_diag_get_upload_per_tcpopen_reqtime(int idx);
int sal_diag_set_upload_per_tcpopen_reqtime(char *value, int idx);
char *sal_diag_get_upload_per_tcpopen_restime(int idx);
int sal_diag_set_upload_per_tcpopen_restime(char *value, int idx);

char *sal_diag_get_download_per_rom_time(int idx);
int sal_diag_set_download_per_rom_time(char *value, int idx);
char *sal_diag_get_download_per_bom_time(int idx);
int sal_diag_set_download_per_bom_time(char *value, int idx);
char *sal_diag_get_download_per_eom_time(int idx);
int sal_diag_set_download_per_eom_time(char *value, int idx);
char *sal_diag_get_download_per_test_br(int idx);
int sal_diag_set_download_per_test_br(char *value, int idx);
char *sal_diag_get_download_per_total_br(int idx);
int sal_diag_set_download_per_total_br(char *value, int idx);
char *sal_diag_get_download_per_total_bs(int idx);
int sal_diag_set_download_per_total_bs(char *value, int idx);
char *sal_diag_get_download_per_tcpopen_reqtime(int idx);
int sal_diag_set_download_per_tcpopen_reqtime(char *value, int idx);
char *sal_diag_get_download_per_tcpopen_restime(int idx);
int sal_diag_set_download_per_tcpopen_restime(char *value, int idx);

char *sal_diag_get_upload_ic_test_bs(int idx);
int sal_diag_set_upload_ic_test_bs(char *value, int idx);
char *sal_diag_get_upload_ic_total_bs(int idx);
int sal_diag_set_upload_ic_total_bs(char *value, int idx);
char *sal_diag_get_upload_ic_total_br(int idx);
int sal_diag_set_upload_ic_total_br(char *value, int idx);
char *sal_diag_get_upload_ic_start_time(int idx);
int sal_diag_set_upload_ic_start_time(char *value, int idx);
char *sal_diag_get_upload_ic_end_time(int idx);
int sal_diag_set_upload_ic_end_time(char *value, int idx);

char *sal_diag_get_download_ic_test_br(int idx);
int sal_diag_set_download_ic_test_br(char *value, int idx);
char *sal_diag_get_download_ic_total_bs(int idx);
int sal_diag_set_download_ic_total_bs(char *value, int idx);
char *sal_diag_get_download_ic_total_br(int idx);
int sal_diag_set_download_ic_total_br(char *value, int idx);
char *sal_diag_get_download_ic_start_time(int idx);
int sal_diag_set_download_ic_start_time(char *value, int idx);
char *sal_diag_get_download_ic_end_time(int idx);
int sal_diag_set_download_ic_end_time(char *value, int idx);


char* sal_diag_get_udpecho_start(char* cb);
int sal_diag_set_udpecho_start(char* value, char* cb);
char* sal_diag_get_udpecho_complete(char* cb);
int sal_diag_set_udpecho_complete(char* value, char* cb);
char* sal_diag_get_udpecho_success_count(char* cb);
int sal_diag_set_udpecho_success_count(char* value, char* cb);
char* sal_diag_get_udpecho_failure_count(char* cb);
int sal_diag_set_udpecho_failure_count(char* value, char* cb);
char* sal_diag_get_udpecho_average_response_time(char* cb);
int sal_diag_set_udpecho_average_response_time(char* value, char* cb);
char* sal_diag_get_udpecho_min_response_time(char* cb);
int sal_diag_set_udpecho_min_response_time(char* value, char* cb);
char* sal_diag_get_udpecho_max_response_time(char* cb);
int sal_diag_set_udpecho_max_response_time(char* value, char* cb);


char *sal_diag_get_udpecho_per_packet_success(char* cb, int idx);
int sal_diag_set_udpecho_per_packet_success(char *value, char* cb, int idx);
char *sal_diag_get_udpecho_per_packet_send_time(char* cb, int idx);
int sal_diag_set_udpecho_per_packet_send_time(char *value, char* cb, int idx);
char *sal_diag_get_udpecho_per_packet_receive_time(char* cb, int idx);
int sal_diag_set_udpecho_per_packet_receive_time(char *value, char* cb, int idx);
char *sal_diag_get_udpecho_per_test_gensn(char* cb, int idx);
int sal_diag_set_udpecho_per_test_gensn(char *value, char* cb, int idx);
char *sal_diag_get_udpecho_per_test_respsn(char* cb, int idx);
int sal_diag_set_udpecho_per_test_respsn(char *value, char* cb, int idx);
char *sal_diag_get_udpecho_per_resprcv_timestamp(char* cb, int idx);
int sal_diag_set_udpecho_per_resprcv_timestamp(char *value, char* cb, int idx);
char *sal_diag_get_udpecho_per_respreply_timestamp(char* cb, int idx);
int sal_diag_set_udpecho_per_respreply_timestamp(char *value, char* cb, int idx);
char *sal_diag_get_udpecho_per_respreply_failurecount(char* cb, int idx);
int sal_diag_set_udpecho_per_respreply_failurecount(char *value, char* cb, int idx);


char *sal_diag_get_hw_wifi_state(int idx);
int sal_diag_set_hw_wifi_state(char *value, int idx);
char *sal_diag_get_hw_slic_state(int idx);
int sal_diag_set_hw_slic_state(char *value, int idx);
char *sal_diag_get_hw_usb_state(int idx);
int sal_diag_set_hw_usb_state(char *value, int idx);
char *sal_diag_get_hw_usb_hostctl(int idx);
int sal_diag_set_hw_usb_hostctl(char *value, int idx);
char *sal_diag_get_hw_dsl_state(int idx);
int sal_diag_set_hw_dsl_state(char *value, int idx);
char *sal_diag_get_hw_switch_state(int idx);
int sal_diag_set_hw_switch_state(char *value, int idx);
char *sal_diag_get_hw_sfp_state(int idx);
int sal_diag_set_hw_sfp_state(char *value, int idx);
char *sal_diag_get_hw_diag_start(void);
int sal_diag_set_hw_diag_start(char *value);
char *sal_diag_get_hw_diag_complete(void);
int sal_diag_set_hw_diag_complete(char *value);
#endif

