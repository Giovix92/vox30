#ifndef _RCL_WLAN_H_
#define _RCL_WLAN_H_
#include "utility.h"
#include "sc_drv/wifi_info.h"
#include <val_wifi.h>
//#include "../../../../drivers/DSL/bcm963xx/bcmdrivers/broadcom/net/wl/impl14/include/sc_nvram.h"
#include "nvram.h"
int rcl_init_default_wlan_settings(void);
int rcl_init_default_guest_wlan_settings(void);
int rcl_default_wlan_wep128key(char *key, int len);
int wlan_init_check_start(void);
#ifdef CONFIG_SUPPORT_5G_QD
int rcl_init_wlan_env(void);
#endif
int check_wlan_if_up(void);

enum wps_action{
    WPS_ACTION_PBC = 0,
    WPS_ACTION_PIN,
    WPS_ACTION_CANCEL,
    WPS_ACTION_SAVECFG
};

#define WPS_UUID_LEN    (32)
enum
{
    WLAN_WPS_UNCONFIGURED = 0,
    WLAN_WPS_CONFIGURED,
};

#ifdef CONFIG_SUPPORT_BLOCKING_ACCESS_TO_NEW_DEVICES 
#define WIFI_ACCESSED_CLIENT_LIST   "/mnt/1/client_list"
#endif
#define WPS_PIN_LEN     8
#define WPS_WALK_TIME   (120)

#define SC_WPS_MONITOR_ON      "/var/wsc_monitor_on"
#define SC_WPS_MONITOR      "wsc_monitor"
#define SC_WPS_MONITOR_P      "/usr/sbin/wsc_monitor"

#ifdef SUPPORT_WIFI_RTL
#define RTL_WSC_SCRIPT    "/var/rtl_wsc_script.sh"

#define RTL_WIFI_MAIN_IF    "wlan0"
#define RTL_WIFI_MAIN_IF_FP    "wifi_fp0"
#define RTL_WIFI_MAIN_IF_ALL RTL_WIFI_MAIN_IF","RTL_WIFI_MAIN_IF_FP

#define RTL_WIFI_GUEST_IF_GROUP "wlan0-va0,wlan0-va1,wlan0-va2"
#define RTL_WIFI_GUEST_IF_GROUP_FP "wifi_fp1,wifi_fp2,wifi_fp3"
#define RTL_WIFI_GUEST_IF_GROUP_ALL RTL_WIFI_GUEST_IF_GROUP","RTL_WIFI_GUEST_IF_GROUP_FP

#define RTL_WIFI_ROOT    "/var/rtl8192c"
#define RTL_WSC_ROOT    "/var/wps"
#define RTL_WSC_WLAN    RTL_WIFI_ROOT"/wlan0"

#define RTL_WSC_SIMPLECONF_FILE   "/etc/simplecfgservice.xml"

#define RTL_WSC_CONF_FILE   "/var/wsc-wlan0.conf"
#define RTL_WSC_CANCEL_FILE   "/tmp/wscd_cancel"
#define RTL_WSC_FIFO_FILE   "/var/wscd-wlan0.fifo"

#define WSC_STATUS_FILE "/tmp/wscd_status"

#define RTL_TOOL_FLASH      "flash"
#define RTL_TOOL_FLASH_P      "/bin/flash"
#define RTL_TOOL_WSCD       "wscd"
#define RTL_TOOL_WSCD_P       "/bin/wscd"
#define RTL_TOOL_IWCONTROL  "iwcontrol"
#define RTL_TOOL_IWCONTROL_P  "/bin/iwcontrol"
#define RTL_TOOL_IWPRIV  "iwpriv"
#define RTL_TOOL_IWPRIV_P  "/sbin/iwpriv"
#endif

#ifdef SUPPORT_WIFI_RTL
#define SC_WIFI_MAIN_IF_ALL RTL_WIFI_MAIN_IF_ALL
#define SC_WIFI_GUEST_IF_GROUP_ALL RTL_WIFI_GUEST_IF_GROUP_ALL
#else
#define SC_WIFI_MAIN_IF_ALL NULL
#define SC_WIFI_GUEST_IF_GROUP_ALL NULL
#endif

#define SC_WPS_ON           "/var/sc_wps_on"
#define SC_WPS_APP_ON          "/var/sc_wps_app_on"
#define SC_WPS_FINISH_WAIT          "/var/sc_wps_finish_wait"
#define SC_WPS_APP          "wsc_app"
#define SC_WPS_APP_P      "/usr/sbin/wsc_app"
#define WLCTL     "/bin/wlctl"
int wps_get_status(void);
void start_wps_monitor(void);
void stop_wps_monitor(void);

int get_wlan_wps_config_state(int *mode);
int set_wlan_wps_config_state(int mode);

/* wps_set_device_pin: renew WPS device PIN active */
void wps_set_device_pin(char *pin);

/* wps_set_pbc: active push button mode */
void wps_set_pbc(void);

/* wps_cancel_pbc: stop push button mode */
void wps_cancel_pbc(void);

/* wps_set_enroll_pin: active enroll pin mode and set pin */
void wps_set_enroll_pin(char *pin);

/* wps_enroll_pin_mode_cancel: cancel enroll pin mode and */
void wps_enroll_pin_mode_cancel(void);

/* wps_pbc_mode_cancel: cancel push button mode */
void wps_pbc_mode_cancel(void);

void wps_user_stop(void);

/* stat_inprogress: set wps status between start and finish */
void stat_inprogress(void);

/* stat_error: set the wps status in error */
void stat_error(int, int);

/* stat_Timeout: within 2 mins, no client connect to AP */
void stat_Timeout(void);

/* stat_success: within 2 mins, the client has connect to AP */
void stat_success(void);

/* wps status file */
#define WPS_INPROCESS       "/var/wps_start"
#define WPS_ERR_DETECT      "/var/wps_error"
#define WPS_OVERLAP         "/var/wps_overlap"
#define WPS_SUCCESS         "/var/wps_success"
#define WPS_TIMEOUT         "/var/wps_timeout"
#define WPS_STOP            "/var/wps_stop"

#define PRIVACY_NONE            0
#define PRIVACY_WEP             1
#define PRIVACY_WPAPSK          2
#define PRIVACY_WPA2PSK         3
#define PRIVACY_WPAPSK_ANY      4
#define PRIVACY_WPA_8021X       5
#define PRIVACY_WPA2_8021X      6
#define PRIVACY_WPA_ANY_8021X   7
                                 
                                 
#define PRIVACY_CIPHER_TKIP     1
#define PRIVACY_CIPHER_AES      2
#define PRIVACY_CIPHER_AES_TKIP 3

enum{
    WPS_STATUS_NONE = 0,
    WPS_STATUS_STOPPED,
    WPS_STATUS_SUCCESS,
    WPS_STATUS_OVERLAP,
    WPS_STATUS_ERR,
    WPS_STATUS_IN_PROGESS,
    WPS_STATUS_TIMEOUT,
};

void wps_led_in_progress(void);
void wps_led_error(void);
void wps_led_timeout(void);
void wps_led_success(void);
void wps_led_start(void);
void wps_led_stop(void);

void wls_led_data_rxtx(void);
void wls_led_off(void);
void wls_led_on(void);

void start_wifi_led_monitor(void);
void stop_wifi_led_monitor(void);

enum{
    WIFI_INFO_RXTX_DATA = 0,
    WIFI_INFO_STATS,
    WIFI_INFO_CLIENT_STATS
};

int val_wifi_get_info(int wlan_no, int wifi_info_type, char **outbuf);
int get_wifi_stats(int wlan_no, sc_wifi_stats_t *stat);
int get_wifi_rxtx_data(void);

void clear_wifi_ac_chain(void);
void update_wifi_access_control(void);

#ifdef CONFIG_SUPPORT_BLOCKING_ACCESS_TO_NEW_DEVICES 
int rcl_start_wifi_band(void);
int rcl_stop_wifi_band(void);
#endif

int rcl_start_wlan(int option);
int rcl_stop_wlan(int is_sche_on);
int rcl_stop_wlan_2g(void);
#ifdef CONFIG_SUPPORT_WIFI_5G
int rcl_stop_wlan_5g(void);
#endif
int check_wifi_module(void);
int wlPipe(char *command, char **output);
#ifndef CONFIG_SUPPORT_WIFI_5G
void sc_NvramMapping(void);
#endif
#endif /* _RCL_WLAN_H_ */

