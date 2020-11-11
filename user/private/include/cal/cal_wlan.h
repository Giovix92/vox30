#ifndef _CAL_WLAN_H_
#define _CAL_WLAN_H_
#include "utility.h"
#include "sc_drv/wifi_info.h"
#include <val_wifi.h>
#include "rcl/rcl_wlan.h"
#define WLAN_DEVICE_MAIN (1)

enum{
    WLAN_VAP_MAIN = 0,
    WLAN_VAP_GUEST = 2,
    WLAN_VAP_GUEST2,
    WLAN_VAP_GUEST3,
#ifdef CONFIG_SUPPORT_WIFI_5G
    WLAN_5G_VAP_MAIN,
    WLAN_5G_VAP_GUEST,
    WLAN_5G_VAP_GUEST2,
    WLAN_5G_VAP_GUEST3,
#endif
    WLAN_VAP_END,
};
#define GUEST_VAP_MAP_INSTANCE_ID(id) (id)
#define GUEST_INSTANCE_MAP_VAP(id) (atoi(id))
#ifdef CONFIG_SUPPORT_5G_QD
#define QD_WIFI_PHY "eth3"
#define QD_WIFI_BASE_VLAN 100
#define QD_WIFI_CONTROL_VLAN 10
#define QD_WIFI_CONTROL_BR "br10"
#endif
/* WirelessMode
  value 0: b & g
        1: b only
        2: a only
        3: a & b & g
        4: g only
        5: a/b/g/n
        6: n only
        7: g & n
        8: a & n
        9: b & g & n
        10: a/g/n
        64: ac
        72: a/n/ac
        76: n/ac */
enum{
    WLAN_MODE_BG = 0,   //0
    WLAN_MODE_B,        //1
    WLAN_MODE_A,        //2
    WLAN_MODE_ABG,      //3
    WLAN_MODE_G,        //4
    WLAN_MODE_ABGN,     //5
    WLAN_MODE_N,        //6
    WLAN_MODE_GN,       //7
    WLAN_MODE_AN,       //8
    WLAN_MODE_BGN,      //9
    WLAN_MODE_AGN,      //10
    WLAN_MODE_AC = 64,  //64
    WLAN_MODE_ANAC = 72,//72
    WLAN_MODE_ANC = 76, //76
};

enum{
    WLAN_BANDWIDTH_AUTO = 0,
    WLAN_BANDWIDTH_20 = 2,
    WLAN_BANDWIDTH_40 = 4,
    WLAN_BANDWIDTH_80 = 8,
    WLAN_BANDWIDTH_8080 = 8080,
    WLAN_BANDWIDTH_160 = 16,
};

enum{
    WLAN_EXTCHANNEL_LOW = 0,
    WLAN_EXTCHANNEL_HIGH,
};

enum{
    WLAN_AUTH_NONE = 0, //0
    WLAN_AUTH_WEP64,    //1
    WLAN_AUTH_WEP128,   //2
    WLAN_AUTH_WPA,      //3
    WLAN_AUTH_WPA2,     //4
    WLAN_AUTH_WAP_WPA2, //5
    WLAN_AUTH_END
};

enum{
    WLAN_AUTH_ATTR_OPEN = 1<<0,         //1
    WLAN_AUTH_ATTR_SHARE = 1<<1,        //2
    WLAN_AUTH_ATTR_OPEN_SHARE = 1<<2,   //4
    WLAN_AUTH_ATTR_WEP_INDEX1 = 1<<3,   //8
    WLAN_AUTH_ATTR_WEP_INDEX2 = 1<<4,   //16
    WLAN_AUTH_ATTR_WEP_INDEX3 = 1<<5,   //32
    WLAN_AUTH_ATTR_WEP_INDEX4 = 1<<6,   //64
    WLAN_AUTH_ATTR_PSK = 1<<7,          //128
    WLAN_AUTH_ATTR_TKIP = 1<<8,         //256
    WLAN_AUTH_ATTR_AES = 1<<9,          //512
    WLAN_AUTH_ATTR_EAP = 1<<10,      //1024
};
#ifdef CONFIG_SUPPORT_WIFI_MSSID
#define WL_MAX_NUM_SSID  4
#else
#define WL_MAX_NUM_SSID  1
#endif
#ifdef CONFIG_SUPPORT_WIFI_5G
#define WLAN_DEVICE_SECONDARY (WLAN_DEVICE_MAIN+WL_MAX_NUM_SSID)
#define WLAN_DEVICE_SECONDARY_GUEST1 (WLAN_DEVICE_SECONDARY+1)
#define WLAN_DEVICE_SECONDARY_GUEST2 (WLAN_DEVICE_SECONDARY+2)
#define WLAN_DEVICE_SECONDARY_GUEST3 (WLAN_DEVICE_SECONDARY+3)
#endif
#define MAX_WLAN_FILTER_NUM (32)
#define WLAN_FILTER_LEN (32) //xx:xx:xx:xx:xx:xx

#define WLAN_WEP_KEYS		    4	/* 4 keys */
#define WLAN_WEP_64KEY_SIZE	    5	/* 5 bytes per key ( 5x8= 40 bit) */
#define WLAN_WEP_128KEY_SIZE	13	/* 13 bytes per key (13x8=104 bit) */
#define WLAN_MAX_ESSID_LEN	    32	/* Linux supports up to 34 char */
#define WLAN_MAX_WEP_KEY	    32	/* String Format Length */
#define WLAN_SECURITY_KEY_LEN       (128)

#define WDS_MAC_MAX_NUM 32
#define WDS_STATUS_CONNECTED "Connected"
#define WDS_STATUS_DISCONNECTED "Disconnected"
#define WDS_STATUS_OFFLINE "Offline"
typedef struct WLANWEPConfig_s
{
    int wep_mode;		/* disable, 64bit, 128bit */
    unsigned char wep_keys[WLAN_WEP_KEYS][WLAN_MAX_WEP_KEY];
} __attribute__ ((packed)) WLANWEPConfig;

void get_wep_key(WLANWEPConfig *info, char *passphrase);

/* Data structure for MD5 (Message-Digest) computation */
typedef struct
{
    DWORD i[2];			/* number of _bits_ handled mod 2^64 */
    DWORD buf[4];		/* scratch buffer */
    BYTE in[64];		/* input buffer */
    BYTE digest[16];		/* actual digest after MD5Final_wifi call */
} MD5_CTX_WIFI;
typedef struct cal_associate_t
{
    char hostname[64];
    char mac[18];
    char ip[16];
    char auth[2];
    char caps[156];
    char active_caps[156];
}cal_associate;

typedef struct cal_schedule_t
{
    char enable[2];
    char name[64];
    char day_list[64];
    char timeframe[64];
#ifdef CONFIG_SUPPORT_WEBAPI
    char weekday[16];/*for web api*/
#endif
    char stime[16];
    char etime[16];
}cal_schedule_entry;
 
typedef struct cal_wds_entry_t
{
    char key[2];
    char mac[18];
    char ssid[32];
    char mode[16];
    char status[16];
    char remove[8];
    char enable[2];
}cal_wds_entry;

VOID MD5Init_wifi(MD5_CTX_WIFI * mdContext);
VOID MD5Update_wifi(MD5_CTX_WIFI * mdContext, BYTE * inBuf, WORD inLen);
VOID MD5Final_wifi(MD5_CTX_WIFI * mdContext);

enum
{
    WLAN_FILTER_DISABLE = 0,
    WLAN_FILTER_ALLOW,
    WLAN_FILTER_DENY,
    WLAN_FILTER_END
};

#define H_CAL_WLAN_FUNC0(funcname, uri) \
char *cal_wlan_get_##funcname(void);\
int cal_wlan_set_##funcname(char *value)

#define H_CAL_WLAN_FUNC1(funcname, uri) \
char *cal_wlan_get_##funcname(int id1); \
int cal_wlan_set_##funcname(char *value, int id1) \

#define H_CAL_WLAN_FUNC2(funcname, uri) \
char *cal_wlan_get_##funcname(int id1, int id2); \
int cal_wlan_set_##funcname(char *value, int id1, int id2) \

#define H_CAL_WLAN_FUNC3(funcname, uri) \
char *cal_wlan_get_##funcname(int id1, int id2, int id3); \
int cal_wlan_set_##funcname(char *value, int id1, int id2, int id3) \

H_CAL_WLAN_FUNC0(counts, WLAN_CONFIGURATION_NUMBER);
H_CAL_WLAN_FUNC1(enable, WLAN_ENABLE);
H_CAL_WLAN_FUNC1(last_enable, WLAN_ENABLE);
H_CAL_WLAN_FUNC1(bssid, WLAN_BSSID);
H_CAL_WLAN_FUNC1(radio_enabled, WLAN_RADIO_ENABLED);
H_CAL_WLAN_FUNC1(channel, WLAN_CHANNEL);
H_CAL_WLAN_FUNC1(auto_channel_e, WLAN_AUTO_CHANNEL_ENABLE);
H_CAL_WLAN_FUNC1(channel_in_use, WLAN_CHANNEL_IN_USE);
H_CAL_WLAN_FUNC1(possible_channels, WLAN_POSSIBLE_CHANNEL);
H_CAL_WLAN_FUNC1(name, WLAN_NAME);
H_CAL_WLAN_FUNC1(ssid, WLAN_SSID);
H_CAL_WLAN_FUNC1(msglevel, WLAN_MSGLEVEL);
H_CAL_WLAN_FUNC1(operation_mode, WLAN_OPERATION_MODE);
H_CAL_WLAN_FUNC1(basic_data_transmit_rates, WLAN_BASIC_DATA_TRANSMIT_RATES);
H_CAL_WLAN_FUNC1(operational_data_transmit_rates, WLAN_OPERATIONAL_DATA_TRANSMIT_RATES);
H_CAL_WLAN_FUNC1(standard_p, WLAN_STANDARD_P);
H_CAL_WLAN_FUNC1(transmit_power, WLAN_TRANSMIT_POWER);
H_CAL_WLAN_FUNC1(rekey_interval, WLAN_REKEY_INTERVAL);
H_CAL_WLAN_FUNC1(beacon_t, WLAN_BEACON_TYPE);
H_CAL_WLAN_FUNC1(wep_idx, WLAN_WEP_KEY_INDEX);
H_CAL_WLAN_FUNC1(wep_encryptlevel, WLAN_WEP_ENCRYPTION_LEVEL);
H_CAL_WLAN_FUNC1(wep_encrypt_m, WLAN_BASIC_ENCRYPTION_MODES);
H_CAL_WLAN_FUNC1(basic_auth_m, WLAN_BASIC_AUTHENTICATION_MODE);
H_CAL_WLAN_FUNC1(wpa_encrypt_m, WLAN_WPA_ENCRYPTION_MODES);
H_CAL_WLAN_FUNC1(wpa_auth_m, WLAN_WPA_AUTHENTICATION_MODE);
H_CAL_WLAN_FUNC1(wpa2_encrypt_m, WLAN_WPA2_ENCRYPTION_MODES);
H_CAL_WLAN_FUNC1(wpa2_auth_m, WLAN_WPA2_AUTHENTICATION_MODE);
H_CAL_WLAN_FUNC1(wmm_support, WLAN_WMM_SUPPORTED);
H_CAL_WLAN_FUNC1(wmm_e, WLAN_WMM_ENABLE);
H_CAL_WLAN_FUNC1(wmm_noack, WLAN_WMM_WMM_NOACK);
H_CAL_WLAN_FUNC1(client_num, WLAN_CLIENT_NUM);
H_CAL_WLAN_FUNC1(broadcast_ssid, WLAN_BROADCAST_SSID);
H_CAL_WLAN_FUNC1(client_isolation, WLAN_CLIENT_ISOLATION);
H_CAL_WLAN_FUNC1(wpa_gtk_rekey, WLAN_WPA_GTK_REKEY);
H_CAL_WLAN_FUNC1(radius_key, WLAN_RADIUS_KEY);
H_CAL_WLAN_FUNC1(radius_server_ip, WLAN_RADIUS_SERVER_IP);
H_CAL_WLAN_FUNC1(radius_port, WLAN_RADIUS_PORT);
H_CAL_WLAN_FUNC1(preauth, WLAN_PREAUTH);
H_CAL_WLAN_FUNC1(net_preauth, WLAN_NET_REAUTH);
H_CAL_WLAN_FUNC1(hw_accelerate, WLAN_HW_ACCELERATE);
H_CAL_WLAN_FUNC1(up_stream_limit, WLAN_UP_STREAM_LIMIT);
H_CAL_WLAN_FUNC1(ampdu_en, WLAN_AMPDU_ENABLE);
H_CAL_WLAN_FUNC1(amsdu_en, WLAN_AMSDU_ENABLE);
H_CAL_WLAN_FUNC1(nondfs_pref, WLAN_ACS_NONDFS_PREF);
#ifdef CONFIG_BRCM_SUPPORT
H_CAL_WLAN_FUNC1(autochannel_reselect_en, WLAN_AUTOCHANNEL_RESELECTION_ENABLE);
H_CAL_WLAN_FUNC1(autochannel_reselect_timeout, WLAN_AUTOCHANNEL_RESELECTION_TIMEOUT);
H_CAL_WLAN_FUNC1(autochannel_restrict, WLAN_AUTOCHANNEL_RESTRICT);
#endif
#ifdef CONFIG_SUPPORT_PRPL_HL_API
H_CAL_WLAN_FUNC1(acs_chan_preferred, WLAN_ACS_CHANPREFERRED);
H_CAL_WLAN_FUNC1(bss_id, WLAN_BSS_ID);
H_CAL_WLAN_FUNC1(bss_name, WLAN_BSS_NAME);
H_CAL_WLAN_FUNC1(bss_del_flag, WLAN_BSS_DELETE_FLAG);
#endif
H_CAL_WLAN_FUNC1(scan_time, WLAN_MONITOR_SCANTIME);

H_CAL_WLAN_FUNC1(frameburst, WLAN_FRAME_BURST);
H_CAL_WLAN_FUNC1(obss_en, WLAN_OBSS_COEX_ENABLE);
H_CAL_WLAN_FUNC1(im_beamforming, WLAN_IBEAMFORMING);
H_CAL_WLAN_FUNC1(ex_beamforming, WLAN_EBEAMFORMING);
H_CAL_WLAN_FUNC1(mitigation, WLAN_MITIGATION);
H_CAL_WLAN_FUNC1(wps_v2_en, WLAN_WPS_V2_ENABLE);
H_CAL_WLAN_FUNC1(wps_e, WLAN_WPS_ENABLE);
H_CAL_WLAN_FUNC1(wps_pbc_e, WLAN_WPS_PBCENABLE);
H_CAL_WLAN_FUNC1(wps_passwd, WLAN_WPS_DEVICE_PASSWORD);
H_CAL_WLAN_FUNC1(wps_config_s, WLAN_WPS_CONFIGURATION_STATE);
H_CAL_WLAN_FUNC1(alert_dismiss, WLAN_ALERT_DISMISS);
H_CAL_WLAN_FUNC1(mfp, WLAN_MFP);
H_CAL_WLAN_FUNC1(acs_exclude_list, WLAN_ACS_EXCLUDELIST);
H_CAL_WLAN_FUNC1(dfschanautomode, WLAN_DFSCHANAUTOMODE);
H_CAL_WLAN_FUNC1(maui, WLAN_MAUI);
H_CAL_WLAN_FUNC2(wep_key, WLAN_WEP_KEY_WEP_KEY);
H_CAL_WLAN_FUNC2(pre_key, WLAN_PRESHARED_KEY_PRESHAREDKEY);
H_CAL_WLAN_FUNC2(ap_wmm_aifsn, WLAN_APWMM_PARAMETER_AIFSN);
H_CAL_WLAN_FUNC2(ap_wmm_ecwmin, WLAN_APWMM_PARAMETER_ECWMin);
H_CAL_WLAN_FUNC2(ap_wmm_ecwmax, WLAN_APWMM_PARAMETER_ECWMax);
H_CAL_WLAN_FUNC2(ap_wmm_txop, WLAN_APWMM_PARAMETER_TXOP);
H_CAL_WLAN_FUNC2(ap_wmm_ackpolicy, WLAN_APWMM_PARAMETER_AckPolicy);
H_CAL_WLAN_FUNC2(sta_wmm_aifsn, WLAN_STWMM_PARAMETER_AIFSN);
H_CAL_WLAN_FUNC2(sta_wmm_ecwmin, WLAN_STWMM_PARAMETER_ECWMin);
H_CAL_WLAN_FUNC2(sta_wmm_ecwmax, WLAN_STWMM_PARAMETER_ECWMax);
H_CAL_WLAN_FUNC2(sta_wmm_txop, WLAN_STWMM_PARAMETER_TXOP);
H_CAL_WLAN_FUNC2(sta_wmm_ackpolicy, WLAN_STWMM_PARAMETER_AckPolicy);
H_CAL_WLAN_FUNC1(phy_band, WLAN_BAND);
H_CAL_WLAN_FUNC1(phy_side_band, WLAN_SIDE_BAND);
H_CAL_WLAN_FUNC1(phy_standard, WLAN_STANDARD);
H_CAL_WLAN_FUNC1(phy_bandw, WLAN_BAND_WIDTH);
H_CAL_WLAN_FUNC1(phy_domain, WLAN_REGULATORY_DOMAIN);
H_CAL_WLAN_FUNC1(phy_short_slot, WLAN_SHORT_SLOT_TIMING_MODE);
H_CAL_WLAN_FUNC1(phy_mcs_idx, WLAN_MCSIDX);
H_CAL_WLAN_FUNC1(phy_stream_number, WLAN_SPATIALSTREAMSNUMBER);
H_CAL_WLAN_FUNC1(phy_abg_rate, WLAN_ABG_RATE);
H_CAL_WLAN_FUNC1(phy_beacon_interval, WLAN_BEACON_INTERVAL);
H_CAL_WLAN_FUNC1(phy_dtim_interval, WLAN_DTIM_INTERVAL);
H_CAL_WLAN_FUNC1(phy_frag, WLAN_FRAG_THRESH);
H_CAL_WLAN_FUNC1(phy_rts, WLAN_RTS_THRESH);
H_CAL_WLAN_FUNC1(phy_txpower, WLAN_TXPOWER_PERCENT);
H_CAL_WLAN_FUNC1(phy_txpower_level, WLAN_TXPOWER_LEVEL);
H_CAL_WLAN_FUNC1(phy_frame_burst, WLAN_FRAME_BURST);
H_CAL_WLAN_FUNC1(phy_wmm_apsd, WLAN_WMM_APSD);
H_CAL_WLAN_FUNC1(phy_tx_preamble, WLAN_TX_PREAMBLE);
H_CAL_WLAN_FUNC1(mac_filter_m, WLAN_MAC_FILTER_MODE);
#ifdef CONFIG_SUPPORT_BLOCKING_ACCESS_TO_NEW_DEVICES 
H_CAL_WLAN_FUNC1(wifi_band_e, WLAN_WIFI_BLOCK_NEW_DEVICES);
#endif
H_CAL_WLAN_FUNC1(mac_filter_n, WLAN_MAC_FILTER_NUMBER);
H_CAL_WLAN_FUNC2(mac_filter_mac, WLAN_MAC_FILTER_FILTER_MAC);
H_CAL_WLAN_FUNC2(mac_filter_name, WLAN_MAC_FILTER_FILTER_NAME);
#ifdef CONFIG_SUPPORT_PRPL_HL_API
H_CAL_WLAN_FUNC2(mac_filter_filter_enable, WLAN_MAC_FILTER_FILTER_ENABLE);
H_CAL_WLAN_FUNC2(mac_filter_filter_notify, WLAN_MAC_FILTER_FILTER_NOTIFY);
H_CAL_WLAN_FUNC2(mac_filter_filter_id, WLAN_MAC_FILTER_FILTER_ID);
H_CAL_WLAN_FUNC1(mac_filter_id, WLAN_MAC_FILTER_ID);
H_CAL_WLAN_FUNC1(mac_filter_fname, WLAN_MAC_FILTER_NAME);
H_CAL_WLAN_FUNC1(mac_filter_notify, WLAN_MAC_FILTER_NOTIFY);
#endif
H_CAL_WLAN_FUNC1(mac_filter_enable, WLAN_MAC_FILTER_ENABLE);
//H_CAL_WLAN_FUNC1(guest_n, WLAN_GUEST_NUM);
H_CAL_WLAN_FUNC1(guest_e, WLAN_GUEST_ENABLE);
H_CAL_WLAN_FUNC1(guest_ssid, WLAN_GUEST_SSID);
H_CAL_WLAN_FUNC1(guest_beacon_t, WLAN_GUEST_BEACON_TYPE);
H_CAL_WLAN_FUNC1(guest_wep_idx, WLAN_GUEST_WEP_KEY_INDEX);
H_CAL_WLAN_FUNC1(guest_wep_encryptlevel, WLAN_GUEST_WEP_ENCRYPTION_LEVEL);
H_CAL_WLAN_FUNC1(guest_wep_encrypt_m, WLAN_GUEST_BASIC_ENCRYPTION_MODES);
H_CAL_WLAN_FUNC1(guest_basic_auth_m, WLAN_GUEST_BASIC_AUTHENTICATION_MODE);
H_CAL_WLAN_FUNC1(guest_wpa_encrypt_m, WLAN_GUEST_WPA_ENCRYPTION_MODES);
H_CAL_WLAN_FUNC1(guest_wpa_auth_m, WLAN_GUEST_WPA_AUTHENTICATION_MODE);
H_CAL_WLAN_FUNC1(guest_wpa2_encrypt_m, WLAN_GUEST_WPA2_ENCRYPTION_MODES);
H_CAL_WLAN_FUNC1(guest_wpa2_auth_m, WLAN_GUEST_WPA2_AUTHENTICATION_MODE);
H_CAL_WLAN_FUNC1(guest_broadcast_ssid, WLAN_GUEST_BROADCAST_SSID);
H_CAL_WLAN_FUNC1(guest_client_isolation, WLAN_GUEST_CLIENT_ISOLATION);
H_CAL_WLAN_FUNC1(guest_client_num, WLAN_GUEST_CLIENT_NUM);
H_CAL_WLAN_FUNC1(guest_wpa_gtk_rekey, WLAN_GUEST_WPA_GTK_REKEY);
H_CAL_WLAN_FUNC1(guest_radius_key, WLAN_GUEST_RADIUS_KEY);
H_CAL_WLAN_FUNC1(guest_radius_server_ip, WLAN_GUEST_RADIUS_SERVER_IP);
H_CAL_WLAN_FUNC1(guest_radius_port, WLAN_GUEST_RADIUS_PORT);
H_CAL_WLAN_FUNC1(guest_preauth, WLAN_GUEST_PREAUTH);
H_CAL_WLAN_FUNC1(guest_net_preauth, WLAN_GUEST_NET_REAUTH);
H_CAL_WLAN_FUNC1(shortGI_en, WLAN_GUEST_NET_REAUTH);
H_CAL_WLAN_FUNC1(airtime_enable, WLAN_GUEST_NET_REAUTH);
H_CAL_WLAN_FUNC1(iaf_enable, WLAN_GUEST_NET_REAUTH);
H_CAL_WLAN_FUNC1(autochannel_reselect_state, WLAN_GUEST_NET_REAUTH);
H_CAL_WLAN_FUNC1(acs_intf_delay_period, WLAN_GUEST_NET_REAUTH);
H_CAL_WLAN_FUNC1(iaf_type, WLAN_GUEST_NET_REAUTH);
H_CAL_WLAN_FUNC2(guest_wep_key, WLAN_GUEST_WEP_KEY_WEP_KEY);
H_CAL_WLAN_FUNC2(guest_pre_key, WLAN_GUEST_PRESHARED_KEY_PRESHAREDKEY);
H_CAL_WLAN_FUNC1(scheduler_enable, WLAN_SCHEDULER_ENABLE);
H_CAL_WLAN_FUNC1(scheduler_ssid_bitmask, WLAN_SCHEDULER_SSID_BITMASK);
H_CAL_WLAN_FUNC1(scheduler_wifi_enable, WLAN_SCHEDULER_WIFI_ENABLE);
H_CAL_WLAN_FUNC1(scheduler_entries, WLAN_SCHEDULER_ENTRIESNUM);
H_CAL_WLAN_FUNC2(scheduler_list_wifi_enable, WLAN_SCHEDULER_LIST_WIFI_ENABLE);
H_CAL_WLAN_FUNC2(scheduler_list_wifi_name, WLAN_SCHEDULER_LIST_WIFI_NAME);
H_CAL_WLAN_FUNC2(scheduler_day, WLAN_SCHEDULER_DAY);
H_CAL_WLAN_FUNC2(scheduler_timeframe, WLAN_SCHEDULER_TIMEFRAME);
H_CAL_WLAN_FUNC2(scheduler_start_time, WLAN_SCHEDULER_START_TIME);
H_CAL_WLAN_FUNC2(scheduler_end_time, WLAN_SCHEDULER_END_TIME);

int get_wlan_auth_info_by_auth(int vap, int auth, int *auth_attr, char *key_buf, int key_len);
int get_wlan_auth_info(int vap, int *auth, int *auth_attr, char *key_buf, int key_len);
int set_wlan_auth_info(int vap, int auth, int auth_attr, char *key);
int cal_get_wep_key(int vap, int wep_type, int key_id, char *key_buf, int key_len);
char *cal_wlan_get_pre_passphrase(int vap, int id);
int cal_wlan_set_pre_passphrase(char *value, int vap, int id);

int get_wlan_filter_mode(int *mode, int id);
int set_wlan_filter_mode(int mode, int id);
int clear_wlan_filter_mac(int id);
int get_wlan_filter_maclist(char **buf_array, int buf_size, int array_size, int id);
int get_wlan_filter_namelist(char **buf_array, int buf_size, int array_size, int id);
int add_wlan_filter_mac(char *mac, char *name, int id);

char *hcal_wlan_patern_xxx_map_interface(char *value);
char *hcal_wlan_interface_map_patern_xxx(char *value);

void get_wsc_uuid(char *buf, int len);
char *get_wsc_model_number(void);
char *get_wsc_modelname(void);
char *get_wsc_model_url(void);
char *get_wsc_device_name(void);
char *get_wsc_manufacture(void);
char *get_wsc_manufacture_url(void);
char *get_wsc_model_number(void);
char *get_wsc_model_desc(void);
char *get_wsc_sn(void);
int get_wlan_wps_onoff(void);
void set_wlan_wps_onoff(int flag);

enum{
    WIFI_TXPW_AUTO = 0,
    WIFI_TXPW_HIGH,
    WIFI_TXPW_MEDIUM,
    WIFI_TXPW_LOW,
};

enum{
    WIFI_WMM_AP = 0,
    WIFI_WMM_STA
};

enum{
    WIFI_AC_BE = 0,
    WIFI_AC_BK,
    WIFI_AC_VI,
    WIFI_AC_VO
};

typedef struct wifi_wmm_cfg_s
{
    char cwmin[8];
    char cwmax[8];
    char aifs[8];
    char txoplimit[8];
}wifi_wmm_cfg_t;

int cal_wlan_get_maxclient_num(int vap);
int cal_wlan_set_maxclient_num(int vap, char *value);
int cal_wlan_wmm_get(int type, wifi_wmm_cfg_t w[]);
int cal_wlan_wmm_set(int type, wifi_wmm_cfg_t w[]);

enum{
    WIFI_TXPW_CCK_A = 0,
    WIFI_TXPW_CCK_B,
    WIFI_TXPW_HT40_1S_A,
    WIFI_TXPW_HT40_1S_B,
    WIFI_TXPW_HT40_2S,
    WIFI_TXPW_HT20,
};

#define WIFI_MP_TXPW_NUM    (14)
#define WIFI_INVALID_TXPW    "FFFFFFFFFFFFFFFFFFFFFFFFFFFF"

typedef struct wifi_mp_txpw_s
{
    unsigned char pwrlevelCCK_A[WIFI_MP_TXPW_NUM];
    unsigned char pwrlevelCCK_B[WIFI_MP_TXPW_NUM];
    unsigned char pwrlevelHT40_1S_A[WIFI_MP_TXPW_NUM];
    unsigned char pwrlevelHT40_1S_B[WIFI_MP_TXPW_NUM];
    unsigned char pwrdiffHT40_2S[WIFI_MP_TXPW_NUM];
    unsigned char pwrdiffHT20[WIFI_MP_TXPW_NUM];
}wifi_mp_txpw_t;

typedef struct wifi_mp_txpw_str_s
{
    unsigned char pwrlevelCCK_A[2*WIFI_MP_TXPW_NUM+1];
    unsigned char pwrlevelCCK_B[2*WIFI_MP_TXPW_NUM+1];
    unsigned char pwrlevelHT40_1S_A[2*WIFI_MP_TXPW_NUM+1];
    unsigned char pwrlevelHT40_1S_B[2*WIFI_MP_TXPW_NUM+1];
    unsigned char pwrdiffHT40_2S[2*WIFI_MP_TXPW_NUM+1];
    unsigned char pwrdiffHT20[2*WIFI_MP_TXPW_NUM+1];
}wifi_mp_txpw_str_t;

int cal_wlan_get_txpower_level(void);
int cal_wlan_set_txpower_level(char *value);
char *get_pwr_keyname(int type);
int wifi_tx_pwr_get(wifi_mp_txpw_t *t);
int pwr_adjust_by_level(wifi_mp_txpw_t *t, int level);
int fillin_txpw_str(wifi_mp_txpw_t *s, wifi_mp_txpw_str_t *t);
 
enum wifi_stats_member
{
    WIFI_STATS_ErrorsSent = 0,
    WIFI_STATS_ErrorsReceived,
    WIFI_STATS_UnicastPacketsSent,
    WIFI_STATS_UnicastPacketsReceived,
    WIFI_STATS_DiscardPacketsSent,
    WIFI_STATS_DiscardPacketsReceived,
    WIFI_STATS_MulticastPacketsSent,
    WIFI_STATS_MulticastPacketsReceived,
    WIFI_STATS_BroadcastPacketsSent,
    WIFI_STATS_BroadcastPacketsReceived,
    WIFI_STATS_UnknownProtoPacketsReceived,
    WIFI_STATS_TotalPacketsSent,
    WIFI_STATS_TotalPacketsReceived,
    WIFI_STATS_TotalBytesSent,
    WIFI_STATS_TotalBytesReceived,
    WIFI_STATS_CHANNEL,
};

enum wifi_chanim_stats_member
{
    WIFI_CHANIM_STATS_TX = 0,
    WIFI_CHANIM_STATS_Inbss,
    WIFI_CHANIM_STATS_Obss,
    WIFI_CHANIM_STATS_NOCat,
    WIFI_CHANIM_STATS_NOPkt,
    WIFI_CHANIM_STATS_Doze,
    WIFI_CHANIM_STATS_TXOP,
    WIFI_CHANIM_STATS_GoodTX,
    WIFI_CHANIM_STATS_BadTX,
    WIFI_CHANIM_STATS_Glitch,
    WIFI_CHANIM_STATS_BadPLCP,
    WIFI_CHANIM_STATS_KNoise,
    WIFI_CHANIM_STATS_Idle,
    WIFI_CHANIM_STATS_TimeStamp,
};
enum wifi_client_stats_member
{
    WIFI_CLIENT_TIMESTAMP,
    WIFI_CLIENT_AVR_TXBITRATE,
    WIFI_CLIENT_PEAK_TXBITRATE,
    WIFI_CLIENT_LOW_TXBITRATE,
    WIFI_CLIENT_AVR_RXBITRATE,
    WIFI_CLIENT_PEAK_RXBITRATE,
    WIFI_CLIENT_LOW_RXBITRATE,
    WIFI_CLIENT_AVR_PHYRATE,
    WIFI_CLIENT_PEAK_PHYRATE,
    WIFI_CLIENT_AVR_TXRETRY,
    WIFI_CLIENT_PEAK_TXRETRY,
    WIFI_CLIENT_RSSI,
};
#ifdef CONFIG_SUPPORT_BANDSTEERING
enum wifi_bandsteering_member
{
    BS_BOUNCE = 0,
    BS_PICKY,
    BS_PSTA,
    BS_DWDS,
    BS_DUALBAND,
    BS_EVENTLOG,
};
#endif

#ifdef CONFIG_SUPPORT_BLOCKING_ACCESS_TO_NEW_DEVICES 
int cal_wlan_clean_up_list(void);
#endif
char *cal_get_wlan_dev_id_map_uri(int wlandev_id);
char *cal_get_wlan_id_map_if(int wlan_id);
int cal_get_wlan_id_map_dev_id(int wlan_id);
char *cal_wlan_get_associate_num(int landev_id,int wlandev_id);
int cal_wlan_set_associate_entry(int landev_id, int wlandev_id, int index, cal_associate* value, client_info_t* client);
int cal_wlan_del_associate_entry(int landev_id, int wlandev_id, int index);
int cal_wlan_set_associate_entry_new(int landev_id, int wlandev_id, cal_associate* value, client_info_t* client);
int cal_wlan_del_all_associate_entry(void);
int cal_wlan_del_quality_stat_entry(int landev_id, int wlandev_id, int assoc_id, int index);
int cal_wlan_set_quality_stat_entry_new(int landev_id, int wlandev_id, int assoc_id);
int cal_wlan_addonce_schedule_entry(cal_schedule_entry* value);
int cal_wlan_del_schedule_entry(int landev_id, int wlandev_id, int index);
int cal_wlan_get_schedule_entry(cal_schedule_entry* entry, int index);
#ifdef CONFIG_SUPPORT_WEBAPI
int cal_wlan_get_schedule_rule_aviable_index(int **array);
#endif
#ifdef CONFIG_SUPPORT_PRPL_HL_API
int cal_wlan_get_macfilter_rule_aviable_index(int **array, int wlan_index);
#endif
int cal_wlan_del_all_scheduler_entry(void);
char *hcal_wlan_pattern_xxx_map_interface(char *value);

H_CAL_WLAN_FUNC1(wds_enable, WLAN_WDS_ENABLE);
H_CAL_WLAN_FUNC2(wds_ssid, WLAN_WDS_LIST_SSID);
H_CAL_WLAN_FUNC2(wds_mac, WLAN_WDS_LIST_MAC);
H_CAL_WLAN_FUNC2(wds_link, WLAN_WDS_LIST_ENABLE);
int cal_wlan_add_wdsentry(cal_wds_entry *wds);
int cal_wlan_del_allwdsentry(void);
int cal_wlan_get_allwdsentry(cal_wds_entry **wds);
int cal_wlan_get_all_active_ssid(int * ssid);
char *cal_wlan_get_wifi_radar_num(int landev_id,int wlandev_id);
int cal_wlan_set_wifi_radar_entry(int landev_id, int wlandev_id, int index, neighbor_info_t *info);
int cal_wlan_del_wifi_radar_entry(int landev_id, int wlandev_id, int index);
int cal_wlan_set_wifi_radar_entry_new(int wlandev_id, neighbor_info_t *info);
int cal_wlan_del_all_wifi_radar_entry(int wlandev_id);
int cal_wlan_set_all_wifi_enable(char *value);
char *cal_wlan_get_all_wifi_enable(void);
int cal_wlan_set_recovery_timer(char *value);
char *cal_wlan_get_recovery_timer(void);
int cal_wlan_set_fast_recovery(char *value);
char *cal_wlan_get_fast_recovery(void);
#ifdef CONFIG_SUPPORT_PRPL_HL_API
char *cal_wlan_get_all_wifi_macfilter_enable(void);
int cal_wlan_set_all_wifi_macfilter_enable(char *value);
#endif
int cal_wlan_set_guest_wifi_enable(char *value);
char *cal_wlan_get_guest_wifi_enable(void);
char *cal_wlan_get_all_wifi_last_enable(void);
int cal_wlan_set_all_wifi_last_enable(char *value);
int cal_wlan_get_acs_policy(char* buf, int id);


#ifdef CONFIG_SUPPORT_BANDSTEERING
char *cal_wlan_get_common_ssid_enable(void);
int cal_wlan_set_common_ssid_enable(char *value);
char *cal_wlan_get_common_guest_ssid_enable(void);
int cal_wlan_set_common_guest_ssid_enable(char *value);
char *cal_wlan_get_bandsteering_enable(void);
int cal_wlan_set_bandsteering_enable(char *value);
char *cal_wlan_get_bs_sta_excluded_enable(void);
int cal_wlan_set_bs_sta_excluded_enable(char *value);
int cal_wlan_clear_bs_sta_excluded_mac(void);
int cal_wlan_get_bs_sta_excluded_maclist(char **buf_array, int buf_size, int array_size);
int cal_wlan_add_bs_sta_excluded_mac(char *name, char *mac);
char* cal_wlan_get_bs_sta_excluded_list_num(void);
char* cal_wlan_get_bs_sta_excluded_list_name(int mac_index);
char* cal_wlan_get_bs_sta_excluded_list_mac(int mac_index);
int cal_wlan_get_bandsteer_bounce_detect(char* buf);
int cal_wlan_get_bs_non11v_detect(char* buf);
int cal_wlan_get_bs_msglevel(char* buf);
int cal_wlan_get_bs_statuspoll(char* buf);
int cal_wlan_get_bs_steer_timeout(char* buf);
int cal_wlan_get_bs_steer_nodeauth(char* buf);
int cal_wlan_get_bs_steer_max_req_count(char* buf);
int cal_wlan_get_bs_steering_policy(char* buf,int idx);
int cal_wlan_get_bs_qualify_policy(char* buf,int idx);
int cal_wlan_get_bs_if_steer_to(char* buf,int idx_from,int idx_to);
int cal_wlan_get_bs_select_policy(char* buf,int idx_from,int idx_to);
#endif

#ifdef CONFIG_SUPPORT_5G_QD
char *cal_get_wlan_id_map_qd_if(int wlan_id);
#endif
char* cal_wifi_schedule_get_day_id_by_day_str(char *day);
char* cal_wifi_schedule_get_day_str_by_day_id(char *id);
char* cal_wifi_schedule_get_frame_id_by_frame_str(char *frame);
char* cal_wifi_schedule_get_frame_str_by_frame_id(char *id);
int cal_wifi_schedule_day_to_number(char *day);

char *cal_wlan_get_guest_duration(void);
int cal_wlan_set_guest_duration(char *value);
char *cal_wlan_get_geust_available_duration(void);
int cal_wlan_set_guest_available_duration(char *value);
char *cal_wlan_get_guest_prior_notify_time(void);
int cal_wlan_set_guest_prior_notify_time(char *value);

char *cal_wlan_get_assoc_mac(char *uri);
#endif
