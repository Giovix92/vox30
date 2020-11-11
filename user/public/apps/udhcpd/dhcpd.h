/* dhcpd.h */
#ifndef _DHCPD_H
#define _DHCPD_H

#include <netinet/ip.h>
#include <netinet/udp.h>

#include "leases.h"
#ifdef CONFIG_SUPPORT_PLUME
extern int save_to_flash;
#endif

/************************************/
/* Defaults _you_ may want to tweak */
/************************************/

#define DHCPD_LAN_IFNAME         "br0"
/* the period of time the client is allowed to use that address */
#define LEASE_TIME              (60*60*24*10) /* 10 days of seconds */

/* where to find the DHCP server configuration file */
#define DHCPD_CONF_FILE         "/var/udhcpd.conf"
#ifdef __SC_BUILD__
/* where to find the DHCP Vendor info*/
#define VENDOR_FILE             "/var/udhcpd.vendor"

/* Only when default subnet dhcp server enable, it works. */
#define DEFAULT_SERVER_CONFIG  0

/* If there is no netmask, use it */
#define SUBNET_MASK 0xFFFFFF00
#endif

/*****************************************************************/
/* Do not modify below here unless you know what you are doing!! */
/*****************************************************************/

/* DHCP protocol -- see RFC 2131 */
#define SERVER_PORT		67
#define CLIENT_PORT		68

#define DHCP_MAGIC		0x63825363

/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET		0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER		0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL		0x17
#define DHCP_MTU		0x1a
#define DHCP_BROADCAST		0x1c
#define DHCP_NTP_SERVER		0x2a
#ifdef __SC_BUILD__
#define DHCP_VENDOR_INFO    0x2b
#endif
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1			0x3a
#define DHCP_T2			0x3b
#define DHCP_VENDOR		0x3c
#define DHCP_CLIENT_ID		0x3d
#ifdef __SC_BUILD__
#define DHCP_TFTP       0x42
#define DHCP_BOOT_FILE          0x43
#define DHCP_USER_CLASS_ID	0x4d
#define DHCP_PROVISIONING_SERVER_URL    0x72
#define DHCP_SIP_SERVERS    0x78
#define DHCP_CLASSLESS_ROUTE    0x79
#define DHCP_VENDOR_SPECIFIC_INFO    0x7d
#define DHCP_MS_CLASSLESS_ROUTE 0xF9
#define DHCP_STATIC_ROUTE    0x21
#define DHCP_LONG_OPTION    0xFA
#define DHCP_OPTION_57      0x39
#define DHCP_OPTION_90      0x5a
#define DHCP_OPTION_160      0xa0
#define DHCP_OPTION_150      0x96
#endif

#define DHCP_END		0xFF


#define BOOTREQUEST		1
#define BOOTREPLY		2

#define ETH_10MB		1
#define ETH_10MB_LEN		6

#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8
#ifdef __SC_BUILD__
#define DHCPFORCERENEW  9
#endif

#define BROADCAST_FLAG		0x8000

#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2

/* miscellaneous defines */
#ifdef __SC_BUILD__
#define TRUE			1
#define FALSE			0
#define MAC_LENGTH      6
#define MAC2_OFFSET     8
#endif
#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"
#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2

#if defined(__SC_BUILD__)
/* Depend on kernel define */
#define BRIDGE_PORT_NONE    (0) // private define, value should be different with other ports define in kernel
#define BRIDGE_PORT_LAN_1   (1)
#define BRIDGE_PORT_LAN_2   (1<<1)
#define BRIDGE_PORT_LAN_3   (1<<2)
#define BRIDGE_PORT_LAN_4   (1<<3)
#define BRIDGE_PORT_WLAN_1  (1<<10)
#define BRIDGE_PORT_WLAN_2  (1<<11)
#define BRIDGE_PORT_WLAN_3  (1<<12)
#define BRIDGE_PORT_WLAN_4  (1<<13)
#define BRIDGE_PORT_WLAN_5  (1<<14)
#define BRIDGE_PORT_WLAN_6  (1<<15)
#define BRIDGE_PORT_WLAN_7  (1<<16)
#define BRIDGE_PORT_WLAN_8  (1<<17)
#define BRIDGE_PORT_IPPONE_VLAN  (1<<18)
/* Depend on config define */
#define SC_PORT_NONE    (0)
#define SC_PORT_LAN_1   (1)
#define SC_PORT_LAN_2   (1<<1)
#define SC_PORT_LAN_3   (1<<2)
#define SC_PORT_LAN_4   (1<<3)
#define SC_PORT_WLAN_1  (1<<4)
#define SC_PORT_WLAN_2  (1<<5)
#define SC_PORT_WLAN_3  (1<<6)
#define SC_PORT_WLAN_4  (1<<7)
#define SC_PORT_WLAN_5  (1<<8)
#define SC_PORT_WLAN_6  (1<<9)
#define SC_PORT_WLAN_7  (1<<10)
#define SC_PORT_WLAN_8  (1<<11)
#define SC_PORT_IPPHONE  (1<<12)

#define BRIDGE_LAN_PORT(port) (BRIDGE_PORT_LAN_1 == port || BRIDGE_PORT_LAN_2 == port || BRIDGE_PORT_LAN_3 == port || BRIDGE_PORT_LAN_4 == port)
#endif

struct option_set {
	unsigned char *data;
	struct option_set *next;
};
#ifdef __SC_BUILD__
struct static_lease {
	uint8_t *mac;
	uint32_t *ip;
	struct static_lease *next;
};

struct  var_list_s{
	uint8_t len;
	uint8_t *value;
};

struct pipe_option_s{
	unsigned char *name;
	unsigned char *session;
	unsigned char *mode;
	unsigned char *cmd;
	struct var_list_s clientID;
	struct var_list_s clientMask;
	struct var_list_s dns;
	struct option_arg_s *arg;
	struct pipe_option_s *next;
};

struct option_arg_s{
	unsigned char *value;
	struct option_arg_s *next;
};

struct option_script_s{
	unsigned char *file;
	unsigned int  arg_num;
	struct option_arg_s *arg;
};

struct client_option_s{
	unsigned char *offer;
	unsigned char *file;
	struct option_script_s script;
	unsigned char *opt_value;
	struct client_option_s *next;
};

struct dhcp_ip_list_s{
    unsigned int ip;    // network order
    struct dhcp_ip_list_s *next;
};

struct lan_port_info_s{
    int port_bit;
    int distribute_ip_enable;
    int distribute_gw_enable;
    int distribute_dns_enable;
    struct lan_port_info_s *next;
};
// If distribute_gw_enable is 0, it will use the PROVISIONAL_LEASE_TIME.
#define PROVISIONAL_LEASE_TIME 120

#endif

struct server_config_t {
	u_int32_t server;		/* Our IP, in network order */
	u_int32_t start;		/* Start address of leases, network order */
	u_int32_t end;			/* End of leases, network order */
	struct option_set *options;	/* List of DHCP options loaded from the config file */
	char *interface;		/* The name of the interface to use */
	int ifindex;			/* Index number of the interface to use */
#ifdef __SC_BUILD__
	u_int32_t netmask;
	char *cds_enable;       /* Conditional Serving on/off, for other subnet, always "1" */
	char *vendor_bind;      /* Conditional Serving */
	char *mac_bind;         /* Conditional Serving */
	char *host_bind;         /* Conditional Serving */
#if defined(__SC_BUILD__) && defined(__SC_CONDITIONAL_SERVING_PER_IF__)
    u_int32_t brg_port_bind;    /* Conditional Serving */
#endif
	char *serialnumber;
	char *manufacture_oui;
	char *product_class;
	char *wan_bind;         /* for other subnet, wan local bind */
	struct dhcp_ip_list_s *dns_server;       /* user input */
	char *dns_proxy;        /* enable 1 and disable 0 */
#endif
	unsigned char arp[6];		/* Our arp address */
	unsigned long lease;		/* lease time in seconds (host order) */
	unsigned long max_leases; 	/* maximum number of leases (including reserved address) */
	char remaining; 		/* should the lease file be interpreted as lease time remaining, or
			 		 * as the time the lease expires */
	unsigned long auto_time; 	/* how long should udhcpd wait before writing a config file.
					 * if this is zero, it will only write one on SIGUSR1 */
	unsigned long decline_time; 	/* how long an address is reserved if a client returns a
				    	 * decline message */
	unsigned long conflict_time; 	/* how long an arp conflict offender is leased for */
	unsigned long offer_time; 	/* how long an offered address is reserved */
	unsigned long min_lease; 	/* minimum lease a client can request*/
	char *lease_file;
	char *pidfile;
	char *notify_file;		/* What to run whenever leases are written */
	u_int32_t siaddr;		/* next server bootp option */
	char *sname;			/* bootp server name */
	char *boot_file;		/* bootp boot file option */
#ifdef __SC_BUILD__
	int  active;
	struct static_lease *static_leases;
	struct pipe_option_s *pipe;
#endif
};	
#ifdef __SC_BUILD__
extern struct server_config_t server_config[];
extern struct dhcpOfferedAddr *leases[];
#else
extern struct server_config_t server_config;
extern struct dhcpOfferedAddr *leases;
#endif
#ifdef __SC_BUILD__
extern int no_of_ifaces;
extern int default_enable;
extern struct client_option_s *vendor_header;
extern struct client_option_s *client_header;
extern struct client_option_s *user_class_header;
extern unsigned int lan_port_info_enable;
extern struct lan_port_info_s *lan_port_header;
extern int gw_option_enable;
extern int dns_option_enable;
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#endif
#endif
