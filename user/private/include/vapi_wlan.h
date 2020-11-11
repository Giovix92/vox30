#ifndef __VAPI_WLAN_H__
#define __VAPI_WLAN_H__
enum
{
    QD_SCHE_INVALID = 0,
    QD_SCHE_WATCHDOG,
    QD_SCHE_ASSOCIATION,
    QD_SCHE_NEIBOURINFO,
    QD_SCHE_END,
};
enum
{
    WLAN_CHANNEL = 0,
    WLAN_MAC,
    WLAN_VLAN_CONFIG,
    WLAN_BSS,
    WLAN_SSID,
    WLAN_BROADCAST,
    WLAN_REGION,
    WLAN_PHY_MODE,
    WLAN_BANDWIDTH,
    WLAN_VHT,
    WLAN_TX_POWER,
    WLAN_ISOLATE,
    WLAN_RTS_THRESHOLD,
    WLAN_DTIM,
    WLAN_BEACON_INTERVAL,
    WLAN_KEY_INTERVAL,
    WLAN_POWER_SAVE,
    WLAN_AMSDU,
    WLAN_IBEAMFORMING,
    WLAN_EBEAMFORMING,
    WLAN_AIRTIME,
    WLAN_DEV_ASSOC_LIMIT,
    WLAN_BSS_ASSOC_LIMIT,
    WLAN_BEACON_TYPE,
    WLAN_ENCRYPTION_MODE,
    WLAN_AUTHENTICATION_MODE,
    WLAN_PASSPHRASE,
    WLAN_PRIORITY,
    WLAN_ASSOCIATIONS,
    WLAN_COUNTER,
    WLAN_FW_VERSION,
    WLAN_RESET_CONTER,
    WLAN_RADIO_STATUS,
    WLAN_MAXBITRATE,
    WLAN_NEIGHBOR,
    WLAN_IFSTATUS,
    WLAN_PROD,
    WLAN_MACFILTER_MODE,
    WLAN_ALLOW_MACFILTER_LIST,
    WLAN_DENY_MACFILTER_LIST,
    WLAN_WPS,
    WLAN_WPS_PIN,
    WLAN_CHANNEL_LIST,
    WLAN_APPLY,
};
char* vapi_wlan_get_channel(int bss_id, char* value);
int vapi_wlan_set_channel(int bss_id, char* value);
char* vapi_wlan_get_mac(int bss_id, char* value);
int vapi_wlan_set_mac(int bss_id, char* value);
int vapi_wlan_set_vlan_config(int bss_id, char* value);
int vapi_wlan_create_bss(int bss_id, char* value);
int vapi_wlan_remove_bss(int bss_id, char* value);
char* vapi_wlan_get_ssid(int bss_id, char* value);
int vapi_wlan_set_ssid(int bss_id, char* value);
char* vapi_wlan_get_broadcast(int bss_id, char* value);
int vapi_wlan_set_broadcast(int bss_id, char* value);
char* vapi_wlan_get_region(int bss_id, char* value);
int vapi_wlan_set_region(int bss_id, char* value);
char* vapi_wlan_get_phy_mode(int bss_id, char* value);
int vapi_wlan_set_phy_mode(int bss_id, char* value);
char* vapi_wlan_get_bandwidth(int bss_id, char* value);
int vapi_wlan_set_bandwidth(int bss_id, char* value);
char* vapi_wlan_get_vht(int bss_id, char* value);
int vapi_wlan_set_vht(int bss_id, char* value);
char* vapi_wlan_get_tx_power(int bss_id, char* value);
int vapi_wlan_set_tx_power(int bss_id, char* value);
char* vapi_wlan_get_isolate(int bss_id, char* value);
int vapi_wlan_set_isolate(int bss_id, char* value);
char* vapi_wlan_get_rts_threshold(int bss_id, char* value);
int vapi_wlan_set_rts_threshold(int bss_id, char* value);
char* vapi_wlan_get_dtim(int bss_id, char* value);
int vapi_wlan_set_dtim(int bss_id, char* value);
char* vapi_wlan_get_beacon_interval(int bss_id, char* value);
int vapi_wlan_set_beacon_interval(int bss_id, char* value);
char* vapi_wlan_get_key_interval(int bss_id, char* value);
int vapi_wlan_set_key_interval(int bss_id, char* value);
char* vapi_wlan_get_power_save(int bss_id, char* value);
int vapi_wlan_set_power_save(int bss_id, char* value);
char* vapi_wlan_get_amsdu(int bss_id, char* value);
int vapi_wlan_set_amsdu(int bss_id, char* value);
char* vapi_wlan_get_im_beamforming(int bss_id, char* value);
int vapi_wlan_set_im_beamforming(int bss_id, char* value);
char* vapi_wlan_get_ex_beamforming(int bss_id, char* value);
int vapi_wlan_set_ex_beamforming(int bss_id, char* value);
char* vapi_wlan_get_airtime(int bss_id, char* value);
int vapi_wlan_set_airtime(int bss_id, char* value);
char* vapi_wlan_get_dev_assoc_limit(int bss_id, char* value);
int vapi_wlan_set_dev_assoc_limit(int bss_id, char* value);
char* vapi_wlan_get_bss_assoc_limit(int bss_id, char* value);
int vapi_wlan_set_bss_assoc_limit(int bss_id, char* value);
char* vapi_wlan_get_beacon_type(int bss_id, char* value);
int vapi_wlan_set_beacon_type(int bss_id, char* value);
char* vapi_wlan_get_encryption_mode(int bss_id, char* value);
int vapi_wlan_set_encryption_mode(int bss_id, char* value);
char* vapi_wlan_get_authentication_mode(int bss_id, char* value);
int vapi_wlan_set_authentication_mode(int bss_id, char* value);
char* vapi_wlan_get_passphrase(int bss_id, char* value);
int vapi_wlan_set_passphrase(int bss_id, char* value);
char* vapi_wlan_get_priority(int bss_id, char* value);
int vapi_wlan_set_priority(int bss_id, char* value);
char* vapi_wlan_get_associations(int bss_id, char* value);
char* vapi_wlan_get_counter(int bss_id, char* value);
char* vapi_wlan_get_fw_version(int bss_id, char* value);
int vapi_wlan_reset_conter(int bss_id, char* value);
char* vapi_wlan_get_radio_status(int bss_id, char* value);
int vapi_wlan_set_radio_status(int bss_id, char* value);
char* vapi_wlan_get_maxbitrate(int bss_id, char* value);
int vapi_wlan_set_maxbitrate(int bss_id, char* value);
char* vapi_wlan_get_ifstatus(int bss_id, char* value);
int vapi_wlan_set_ifstatus(int bss_id, char* value);
char* vapi_wlan_get_prod(int bss_id, char* value);
int vapi_wlan_set_prod(int bss_id, char* value);
char* vapi_wlan_get_macfilter_mode(int bss_id, char* value);
int vapi_wlan_set_macfilter_mode(int bss_id, char* value);
char* vapi_wlan_get_allow_macfilter_list(int bss_id, char* value);
int vapi_wlan_set_allow_macfilter_list(int bss_id, char* value);
char* vapi_wlan_get_deny_macfilter_list(int bss_id, char* value);
int vapi_wlan_set_deny_macfilter_list(int bss_id, char* value);
int vapi_wlan_set_wps(int bss_id, char* value);
char* vapi_wlan_get_wps_pin(int bss_id, char* value);
int vapi_wlan_set_wps_pin(int bss_id, char* value);
char* vapi_wlan_get_channel_list(int bss_id, char* value);
int vapi_wlan_apply_security_config(int bss_id, char* value);
int vapi_wlan_sche_regist(int id, int interval);
int vapi_wlan_sche_unregist(int id);
#endif
