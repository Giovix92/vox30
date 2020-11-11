#ifndef __WIFI_SCH_H__
#define __WIFI_SCH_H__

#define WIFI_SCHEDULE_NAME "device.wifi.wifi_schedule"
#define GUEST_EXPIER_TIME 1

enum
{
    AC_DELAY_KEY,
    OFF_DELAY_KEY,
    GUEST_DURATION_KEY,
    GUEST_PRIOR_NOTIFY_KEY
};

int ws_delay_time_update(int key, int value);
int ws_guest_config_change(int guest_en, int guest_2g, int guest_5g, char *guest_ssid);
int ws_wifi_config_change(int wifi_all, int wifi_2g, int wifi_5g);
int ws_fon_config_change(int fon_en, int fon_ac, int wan_status, int min_band);
int ws_fon_eap_change(char *eap, char *eap5);
int wifi_schedule_get_guest_time(int type);
int wifi_schedule_get_fon_status(char *key);
#ifdef CONFIG_SUPPORT_DEBUG
char* wifi_schedule_get_parameters_value(void);
#endif

#endif
