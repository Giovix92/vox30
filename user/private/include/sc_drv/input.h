#ifndef __SC_DRV_INPUT_H__
#define __SC_DRV_INPUT_H__

enum
{
    INPUT_START_KEY,
    INPUT_RESET_KEY,
    INPUT_WPS_KEY,
    INPUT_WIFI_KEY,
    INPUT_AC220_OFF_KEY,
    INPUT_LOW_VOLTAGE_KEY,
    INPUT_BATTERY_ON_KEY,
    INPUT_BBU_AIN_KEY,
    INPUT_BBU_BIN_KEY,
    INPUT_LAST_KEY,
};

typedef struct tag_INPUT_KEY {
    int key;
    int state;
}INPUT_KEY;
struct QoS_DEV
{
#define MAX_INFO_LENGTH 64
#define ETH_ALEN 6
    unsigned char mac[ETH_ALEN];
    int dhcp_opt_code;
    char opt_value[MAX_INFO_LENGTH];
    unsigned int ip_addr;
};
#define MAX_KEY_NUM INPUT_LAST_KEY
typedef struct tag_INPUT_EVENT {
    int num;
    int key[MAX_KEY_NUM];
    int timeout;  //seconds
}INPUT_EVENT;

enum
{
    OUTPUT_FIRST_KEY,
    OUTPUT_LED_POWER,
#if !defined(VOX_LED_SPEC)
#if (defined(CONFIG_SUPPORT_DT_TEST) || defined(VD1018) || defined(CONFIG_BCM963268))
#ifndef FD1018
#ifndef VOX25
    OUTPUT_LED_LAN4,
    OUTPUT_LED_LAN3,
    OUTPUT_LED_LAN2,
#endif
#endif
#endif
    OUTPUT_LED_LAN1,
#endif
    OUTPUT_LED_INTERNET,
    OUTPUT_LED_PHONE,
#if !defined(VOX_LED_SPEC)
    OUTPUT_LED_DSL,
#endif
    OUTPUT_LED_USB,
    OUTPUT_LED_WIFI,
#define OUTPUT_LED_WPS OUTPUT_LED_WIFI
#if !defined(VOX_LED_SPEC)
    OUTPUT_LED_WPS,
#endif
    OUTPUT_LED_REINJECTION,
#define OUTPUT_LAST_LED_KEY OUTPUT_LED_REINJECTION 
    OUTPUT_LED_UPGRADE,
#if defined(VOX_LED_SPEC)
    OUTPUT_LED_RESTORE,
    OUTPUT_LED_FWDL,
    OUTPUT_LED_FWDLFIN,
#endif
    OUTPUT_LAST_KEY,
};
enum
{
    OUTPUT_LED_OFF,
    OUTPUT_LED_ON,
    OUTPUT_LED_BLINK,
    OUTPUT_LED_STORE,
    OUTPUT_LED_LOCK,
    OUTPUT_LED_UNLOCK,
    OUTPUT_LED_PULSING,
    OUTPUT_LED_FADE_IN,
    OUTPUT_LED_FADE_OUT,
};
enum
{
    OUTPUT_LED_GREEN,
    OUTPUT_LED_RED,
    OUTPUT_LED_BLUE,
#ifdef ESSENTIAL
    OUTPUT_LED_PURPLE,
#endif
};

typedef struct tag_OUTPUT_BLINK {
    unsigned int count;
    int speed;
    int last_stat;
}OUTPUT_BLINK;
typedef struct tag_OUTPUT_KEY {
    int key;
    int state;
    int color;  //0: Green; 1: Red
    OUTPUT_BLINK blink; 
}OUTPUT_KEY;

#if defined(SUPPORT_USB_STORAGE) || defined(SUPPORT_USB_PRINTER) || defined(CONFIG_SUPPORT_3G)  
enum
{
    OUTPUT_USB_STATE_ADD,
    OUTPUT_USB_STATE_UP,
    OUTPUT_USB_STATE_DEL,
};
typedef struct tag_OUTPUT_USB_STATE_DEV{
    int bus;
    char port[16];
    int support;
    int is_printer;
    char name[64];
}OUTPUT_USB_STATE_DEV;
typedef struct tag_OUTPUT_USB_STATE {
    OUTPUT_USB_STATE_DEV dev;
    int used;
}OUTPUT_USB_STATE;
typedef struct tag_OUTPUT_USB_STATE_UP
{
    OUTPUT_USB_STATE usb_state;
    int action;
}OUTPUT_USB_STATE_CMD;
#endif
typedef struct tag_ETH_PORT_CHANGE{
    int timeout;
    int flag;
}ETH_PORT_CHANGE;

#ifdef VOX25
enum {
    LED_FROM_INTERNET_MODULE=(1<<16),
    LED_FROM_TELEPHONE_MODULE=(2<<16),
    LED_FROM_WIFI_MODULE=(3<<16),
    LED_FROM_MOBILE_MODULE=(4<<16),
    //add new module in here
	LED_FROM_LAST_MODULE,
};
#endif

#ifdef CONFIG_SUPPORT_ENERGY_SAVING
typedef struct {
int status;
int power;
int brightness;
}STR_ENERGY_SAVING;
#endif
#endif
