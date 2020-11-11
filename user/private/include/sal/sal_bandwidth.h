#ifndef __SAL_BANDWIDTH_H__
#define __SAL_BANDWIDTH_H__
#include "common/list.h"

typedef struct {
    unsigned long int tx_bytes;
    unsigned long int rx_bytes;
} speed_t;
struct BDList_s{
    struct list_head list;
    speed_t data;
};
enum
{
    BW_TYPE_TX,
    BW_TYPE_RX
};
enum
{
    BW_ACTION_RESET,
    BW_ACTION_GET,
    BW_ACTION_WAN_START,
    BW_ACTION_WAN_STOP,
    LED_ACTION_WIFI
};
enum
{
    BW_ITEM_LAN,
    BW_ITEM_WAN,
    BW_ITEM_PEAK_TX_WAN,
    BW_ITEM_PEAK_RX_WAN,
    BW_ITEM_WIFI,
    LED_ITEM_OFF,
    LED_ITEM_ON,
    LED_ITEM_OFF_SCHE,
    LED_ITEM_WPS,
    LED_ITEM_WPSEND,
    LED_ITEM_WPS_FAIL,
    LED_ITEM_WPS_FAILEND
};
struct bw_packet{
    int item;
    int action;
    int id;
    speed_t data;
};
#ifdef CONFIG_SUPPORT_UBUS
#define UBUS_LEDAP_OBJECT     "ledap"
#define UBUS_LEDAP_METHOD_WAN_START "wan_start"
#define UBUS_LEDAP_METHOD_WAN_STOP "wan_stop"
#define UBUS_LEDAP_METHOD_RESET "reset"
#define UBUS_LEDAP_METHOD_GET "get"
#define UBUS_LEDAP_METHOD_LED_WIFIBUTTON "led_wifibutton"
#define UBUS_LEDAP_METHOD_TIMER_ACTION  "timer_action"
#endif
#define BW_SOCKETNAME_S "/tmp/4/bw_server.socket"
#define BW_SOCKETNAME_C "/tmp/4/bw_client.socket"
int sal_bw_set_wifi_led_on(void);
int sal_bw_set_wifi_led_off(void);
#if defined(VOX_LED_SPEC)
int sal_bw_set_wifi_led_off_sche(void);
int sal_bw_set_wifi_led_wps(void);
int sal_bw_set_wifi_led_wps_end(void);
int sal_bw_set_wifi_led_wps_fail(void);
int sal_bw_set_wifi_led_wps_failend(void);
#endif
int sal_bw_reset_lan(int lan_id);
int sal_bw_reset_wan(int wan_id);
int sal_bw_reset_wifi(void);
int sal_bw_notify_wan_start(int wan_id);
int sal_bw_notify_wan_stop(int wan_id);
int sal_bw_get_lan(int lan_id, speed_t *speed, int time_m);
int sal_bw_get_wan(int wan_id, speed_t *speed, int time_m);
int sal_bw_get_wifi(speed_t *speed, int time_m);
int sal_bw_tx_get_wan(int wan_id, speed_t *speed, int time_m);
int sal_bw_rx_get_wan(int wan_id, speed_t *speed, int time_m);
#endif
