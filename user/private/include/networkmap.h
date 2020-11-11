/**
 * @file   
 * @author 
 * @date   2010-08-30
 * @brief  
 *
 * Copyright - 2009 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */
#ifndef _NETWORKMAP_H_
#define _NETWORKMAP_H_

#include "libevent_client.h"

/********************************* common public api *********************************/

char *networkmap_work_dir(void);

/* networkmap_get_result, return 0 on success, -1 on error */
int networkmap_get_result(char *result_file);

/* networkmap_detect_is_running -- return 1 if in detecting, else return 0 */
int networkmap_detect_is_running(void);

int networkmap_detect_signal(void);
int networkmap_save_info_signal(void);
int networkmap_exit_signal(void);

/* networkmap_output_xml_done -- return 1 if done, else return 0 */
int networkmap_output_xml_done(void);
char networkmap_get_xml_data_separator(void);

/* networkmap_update_lan_hosts_info -- return 0 on success, -1 on error */
int networkmap_update_lan_hosts_info(void);

/* nmap_update_dev_list_info */
int nmap_update_dev_list_info(unsigned char mac[], unsigned int ip);
int nmap_update_dev_list_send_options_info(unsigned char mac[],char option[]);
int nmap_update_dev_list_request_options_info(unsigned char mac[],char option[]);
/* nmap update macfilter */
int nmap_update_wifi_macfilter_list(char *mode, char *list);
int nmap_update_guest_wifi_macfilter_list(char *mode, char *list);
int nmap_get_csrf_seed(unsigned int *seed);
#ifdef CONFIG_SUPPORT_UBUS
int nmap_get_guest_device_info(device_access_info_req ** client);
int nmap_get_main_device_info(device_access_info_req ** client);
int nmap_get_all_device_info(device_access_info_req ** client);
#endif
/********************************* networkmap client *********************************/

#define NETWORKMAP_DOMAIN   "/tmp/lan_host.domain"
#define NETWORKMAP_BUFFER   1024


typedef enum
{
    NETWORKMAP_KEY_TYPE_NONE = 0,
    NETWORKMAP_KEY_TYPE_MAC,
    NETWORKMAP_KEY_TYPE_IP,
    NETWORKMAP_KEY_TYPE_INDEX, // index start from 1
#ifdef CONFIG_SUPPORT_IPV6
    NETWORKMAP_KEY_TYPE_GADDR,
#endif
    NETWORKMAP_KEY_TYPE_MACFILTER,
    NETWORKMAP_KEY_TYPE_END
}e_networkmap_key_type;

typedef enum
{
    NETWORKMAP_REQUEST_TYPE_NONE = 0,
    NETWORKMAP_GET_DEVICE_NUM, /* NETWORKMAP_GET_DEVICE_NUM is special, not need to set key type and value */
    NETWORKMAP_GET_DEVICE_MAC,
    NETWORKMAP_GET_DEVICE_IP,
    NETWORKMAP_GET_DEVICE_USERNAME,
    NETWORKMAP_GET_DEVICE_HOSTNAME,
    NETWORKMAP_GET_DEVICE_LINKTYPE,
    NETWORKMAP_GET_DEVICE_ALIVE,
    NETWORKMAP_GET_DEVICE_TYPE,
    NETWORKMAP_GET_DEVICE_USERTYPE,
    NETWORKMAP_GET_DEVICE_SHARE,
    NETWORKMAP_GET_DEVICE_TYPE_NUM,
    NETWORKMAP_GET_DEVICE_DHCP_OPT,
    NETWORKMAP_GET_DEVICE_DHCP_USERCLASS_OPT,
    NETWORKMAP_GET_DEVICE_DHCP_VENDOR_OPT,
    NETWORKMAP_GET_DEVICE_DHCP_SEND_OPTS,
    NETWORKMAP_GET_DEVICE_DHCP_REQ_OPTS,
#ifdef CONFIG_SUPPORT_WEBAPI
    NETWORKMAP_GET_DEVICE_FIRST_SEE_TIME,
#endif
#ifdef CONFIG_SUPPORT_IPV6
    NETWORKMAP_GET_DEVICE_IPV6_GADDR,
    NETWORKMAP_GET_DEVICE_IPV6_LADDR,
    NETWORKMAP_GET_DEVICE_ALIVE6,
#endif
    NETWORKMAP_GET_DEVICE_SPEED,
    NETWORKMAP_REQUEST_TYPE_GET_END,
    /* For set, now not use, and if will use it someday, need to think it carefully.
       Here not deal if not find device, and whether need to add a new device */
    NETWORKMAP_SET_DEVICE_IP,
    NETWORKMAP_SET_DEVICE_USERNAME,
    NETWORKMAP_SET_DEVICE_HOSTNAME,
    NETWORKMAP_SET_DEVICE_TYPE,
    NETWORKMAP_SET_DEVICE_USERTYPE,
    NETWORKMAP_SET_DMZ_IP,
    NETWORKMAP_SET_DEVICE_DHCP_OPT,
    NETWORKMAP_SET_DEVICE_DHCP_SEND_OPTS,
    NETWORKMAP_SET_DEVICE_DHCP_REQ_OPTS,
    NETWORKMAP_REQUEST_TYPE_SET_END,
    NETWORKMAP_GET_CSRF_SEED,
    NETWORKMAP_UPDATE_MACFILTER,
    NETWORKMAP_UPDATE_GUEST_MACFILTER,
    NETWORKMAP_REQUEST_TYPE_END
}e_networkmap_request_type;
struct dhcp_t
{
#define OPT_MAX_LEN 256
    int opt_code;
    char opt_value[OPT_MAX_LEN];
};
struct nmapreq
{
#define MAC_ADDR_LEN        6
#define INFO_MAX_LEN	    256
#define IPV6_MAX_LEN	    16
    unsigned int    nmr_dev_num;  // total number of devices
    int             nmr_key_type; // find the device with this type info "e_networkmap_key_type"
    int             nmr_err_code; // when error, it records the error code
	char    wifi_access_control[INFO_MAX_LEN];             //filter mode
	char    wifi_access_list[INFO_MAX_LEN];                //filter list
    union
    {
        unsigned char   keyu_mac[MAC_ADDR_LEN];
		unsigned int    keyu_ip;    // host order
		unsigned int    keyu_index; // show pc list, index start from 1
#ifdef CONFIG_SUPPORT_IPV6
        unsigned char   keyu_ipv6_gaddr[IPV6_MAX_LEN];
#endif
    } nmr_keyu;
	union
	{
		unsigned char   nmru_mac[MAC_ADDR_LEN];
		unsigned int    nmru_ip;    // host order
        unsigned char   nmru_username[INFO_MAX_LEN];
		unsigned char   nmru_hostname[INFO_MAX_LEN];
		unsigned char   nmru_devtype[INFO_MAX_LEN];
		unsigned char   nmru_user_devtype[INFO_MAX_LEN];
		unsigned char   nmru_typenum[INFO_MAX_LEN];
		unsigned char   nmru_share[INFO_MAX_LEN];
		unsigned int    nmru_dmz_ip_addr; // host order
		unsigned char   nmru_link_type[INFO_MAX_LEN]; // return string
		unsigned int    nmru_alive; // nmru_alive will return 1 or 0
		unsigned char   nmru_sendoptions[INFO_MAX_LEN];//send options list 
		unsigned char   nmru_reqoptions[INFO_MAX_LEN]; //request options list
#ifdef CONFIG_SUPPORT_WEBAPI
		long int        nmru_first_see_time;    //new dev first connect's time
#endif
#ifdef CONFIG_SUPPORT_IPV6
		unsigned char    nmru_ipv6_gaddr[IPV6_MAX_LEN];    // ipv6 global address
		unsigned char    nmru_ipv6_laddr[IPV6_MAX_LEN];    // ipv6 local address
		unsigned int    nmru_alive6; 
#endif
        unsigned char    nmru_speed[INFO_MAX_LEN];
		unsigned int    nmru_csrf_seed; 
        struct dhcp_t  nmru_dhcp;
	} nmr_nmru;
}__attribute__((packed));

typedef enum
{
    NETWORKMAP_REQUEST_SUCCESSFULLY = 0,
    NETWORKMAP_REQUEST_FAILED
}e_networkmap_request_result;

/* in response packet, status: NETWORKMAP_REQUEST_SUCCESSFULLY or NETWORKMAP_REQUEST_FAILED
   in request packet, status: e_networkmap_request_type */
struct nmr_packet
{
    int status;
    struct nmapreq nmr;
};

/* return 0, successful; return -1, failed, error message will be saved in nmr->nmr_err_code */
int networkmap_ioctl(e_networkmap_request_type request_type, struct nmapreq *nmr);
int networkmap_non_block_ioctl(e_networkmap_request_type request_type, struct nmapreq *nmr);


#endif /* _NETWORKMAP_H_ */
