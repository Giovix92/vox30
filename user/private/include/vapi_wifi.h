#ifndef __VAPI_WIFI_H__
#define __VAPI_WIFI_H__
#include <plugin_wifi.h>
typedef struct{                                        
           unsigned int counter;                                   
#define MACLIST_NUMBER 128                             
#define MAC_LEN 18                                 
                        char mac_list[MACLIST_NUMBER][MAC_LEN];
}client_macinfo;                                       

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

int vapi_wifi_get_status(char *device, int *value);
int vapi_wifi_get_mac(char *device, char *value);
//
int vapi_wifi_get_cnu(char *device, char *value);
int vapi_wifi_get_sec(char *device, char *value);
int vapi_wifi_get_enc(char *device, char *value);
int vapi_wifi_client_stats_info(char *mac, client_info_t* client_info);
int vapi_wifi_get_statistics(char *device, wifi_info_t* info);
int vapi_wifi_get_stats(char *device, int stats_member);
int vapi_wifi_get_info(char *device, int wifi_info_type, char **outbuf);
int vapi_wifi_client_check_by_mac(char *mac);
int vapi_wifi_check_client_mac(char *device, char *mac);
//int vapi_wifi_get_auth_mac(int type, char **outbuf);
int vapi_wifi_get_ver(char **ver);
int vapi_wifi_clear_counter();
//int vapi_get_wds_station_info(char *mac, wds_station_info_t station_info[]);
//int vapi_get_wds_mac_list(char mac[], int len);
//int vapi_get_channel_imstate(char *device);
void vapi_wifi_get_wifi_max_speed(double *value);
void vapi_wifi_get_5g_max_speed(double *value);
int vapi_wifi_get_radio_enable(char *device);
void vapi_wifi_get_maxbitrate(char *device,double *value);
void vapi_get_all_client_mac(client_macinfo* mac_client, char *device);
int vapi_get_all_client_info(char *device, char *mac, client_info_t *client_info);
int vapi_wifi_get_capabilities(char *device, unsigned int *ht_capabilities);
int vapi_wifi_get_vht_capabilities(char *device, int *vht_flags);
int vapi_wifi_get_channel(char *device);
int vapi_get_neighbor_info(int *count, neighbor_info_t neighbor_info[], char *device);
//
int vapi_get_5g_wifi_init_status(void);
//
int vapi_get_5g_init_wifi_env(void);
#endif
