/**
 * @file   libevent_client.h 
 * @author Soohoo_Hu@sdc.sercomm.com Leon_Gao@sdc.sercomm.com
 * @version 1.0
 * @date   2017-03-21
 * @brief  headfile event management client api
 *
 * Copyright - 2017 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */
#ifndef _EVENT_CLI_H_
#define _EVENT_CLI_H_
#define MAX_LAN_CLIENT 256
typedef struct device_access_info_s{
	char hostname[256];
	char ipaddress[128];
	char hosttype[128];
	char sharedstate[128];
	char interfacetype[64];
	char macaddress[64];
    char layer2interface[256];
	char addresssource[256];
	char active[4];
#ifdef CONFIG_SUPPORT_PRPL_HL_API
    char alias[256];
    char vendorclass[256];
    char userclass[256];
    char sendoptions[256];
    char requestoptions[256];
#endif
    long first_see_time;            // the first time when device get updated, seconds
    long last_see_time;            // the first time when device get updated, seconds
}device_access_info_t;

typedef struct device_access_info_req
{
	int count;
	device_access_info_t client[MAX_LAN_CLIENT];
}device_access_info_req;

int event_notify_guest_wifi_expired(char *expired, char *ssid);
int event_notify_new_device_connected(const unsigned char *mac, char *lintype, char *devtype, char *name);
int event_notify_device_connected(const unsigned char *mac, char *linktype, char *devtype, char *name);
int event_notify_device_dhcp_opt(const unsigned char *mac);
int event_notify_wps_pairing(const unsigned char *status, char *mac);
int event_notify_ntp_status(int status);
int event_notify_wan_status(int status, int bandwidth, char *access_type);
int event_notify_wan_change(int wan_id);
#ifdef CONFIG_SUPPORT_PRPL_HL_API
int event_notify_host_needupdate(int needupdate);
int event_notify_button_action(char *action);
#endif
#endif
