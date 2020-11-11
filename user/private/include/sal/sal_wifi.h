#ifndef _SAL_WIFI_H_
#define _SAL_WIFI_H_

#ifdef CONFIG_SUPPORT_BANDSTEERING
#define SAL_WIFI_BANDSTEERING  "statsd.wifi.bandsteering"
#endif
#define SAL_WIFI_GENERAL  "statsd.wifi.general"
#define SAL_WIFI_CLIENT  "statsd.wifi.client"

int sal_get_wifi_main_ssid_client_list(char *client_list);
#ifdef CONFIG_SUPPORT_BANDSTEERING
char* sal_wifi_bs_get_log(char *mac, int member);
#endif
char* sal_wifi_client_get_stats(char *ifname, int table_id, char *mac, int stats_member);
#endif
