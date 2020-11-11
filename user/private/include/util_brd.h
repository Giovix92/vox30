#ifndef _UTIL_BRD_H_
#define _UTIL_BRD_H_


int util_read_board_info_to_ram(void);

int util_get_hw_mac_from_flash(unsigned char *buf, int buf_len);
int util_get_hw_csn_from_flash(char *buf, int buf_len);
int util_get_hw_pin_from_flash(char *buf, int buf_len);
int util_get_hw_pcba_sn_from_flash(char *buf, int buf_len);
int util_get_hw_pid_from_flash(unsigned char *buf, int buf_len);
int util_get_hw_pskkey_from_flash(char *buf, int buf_len);
int util_get_hw_ssid_from_flash(char *buf, int buf_len);
int util_get_end_pwd_from_flash(char *end_pwd, int str_len);
int util_get_hw_id_from_flash(char *hwid, int str_len);
int util_set_hw_mac_to_flash(unsigned char *mac, int mac_len);
int util_set_hw_pin_to_flash(char *pin_str, int str_len);
int util_set_hw_pcba_sn_to_flash(unsigned char *pcbasn_str, int str_len);
int util_set_hw_pskkey_to_flash(char *pskkey_str, int str_len);
int util_set_hw_id_to_flash(char *hwid_str, int str_len);
int util_set_hw_endpwd_to_flash(char *endpwd_str, int str_len);
int util_set_hw_usbupgrade_to_flash(char *endpwd_str, int str_len);
int util_set_hw_csn_to_flash(char *csn, int str_len);
int util_set_hw_ssid_to_flash(char *ssid, int str_len);
int util_set_tm_size_to_flash(unsigned int tm);
int util_get_tm_size_from_flash(unsigned int *value);
int util_set_value_to_flash(unsigned int offset, unsigned int length, unsigned int value);
int util_get_value_from_flash(unsigned int offset, unsigned int length, unsigned int *value);

#define HW_ID_LEN 			32
int util_get_hw_id(char *buf, int buf_len);
int util_get_hw_boot_version(char *buf, int buf_len);
int util_get_hw_mac(char *buf, int buf_len);
int util_get_hw_sn(char *buf, int buf_len);
int util_get_hw_version(char *buf, int buf_len);
int util_get_hw_pin(char *buf, int buf_len);
int util_get_hw_pcba_sn(char *buf, int buf_len);
int util_get_hw_pid(char *buf, int buf_len);
int util_get_friendly_name(char *buf);
int util_get_manu_facturer(char *buf);
int util_get_manu_facturerurl(char *buf);
int util_get_manu_description(char *buf);
int util_get_modelurl(char *buf);
int util_get_fw_ver(char *buf);
int util_get_product_name(char *buf);
int util_get_product_sn(char *buf);


void util_set_all_red_led_off(void);
void util_set_all_red_led_on(void);
void util_set_all_green_led_off(void);
void util_set_all_green_led_on(void);
void util_set_all_led_off(void);
void util_set_all_green_led_blink_slow(void);
void util_set_all_green_led_blink_fast(void);
void util_set_all_led_on(void);
int util_set_voip_ring(char *buf);
int util_init_sys_time(void);
#if defined(VOX25) || defined(ESSENTIAL) || defined(VOX30) 
void util_set_all_blue_led_off(void);
void util_set_all_blue_led_on(void);
#endif
void libGetAPMacAddress(char *pMac);
void libGetLanMacAddress(char *pMac);
void libGetWanMacAddress(int wanId, char *mac_str);
void libGetWanMacAddress_X(int wanId,char *mac_str, char *sep);
int  lib_get_wan_mac_address(int wanId,char *mac);
int  lib_get_lan_mac_address(char *mac);
#ifdef CONFIG_SUPPORT_UNIQUE_ADMIN_PWD
int util_set_hw_adminpwd_to_flash(char *endpwd_str, int str_len);
int util_get_admin_pwd_from_flash(char *end_pwd, int str_len);
#endif
#endif /* _UTIL_BRD_H_ */

