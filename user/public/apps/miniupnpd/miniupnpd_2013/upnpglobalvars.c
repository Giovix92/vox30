/* $Id: upnpglobalvars.c,v 1.30 2013/06/13 13:21:30 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2012 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include "config.h"
#include "upnpglobalvars.h"

/* network interface for internet */
const char * ext_if_name = 0;

/* file to store leases */
#ifdef ENABLE_LEASEFILE
const char* lease_file = 0;
#endif

/* forced ip address to use for this interface
 * when NULL, getifaddr() is used */
const char * use_ext_ip_addr = 0;

/* LAN address */
/*const char * listen_addr = 0;*/

unsigned long downstream_bitrate = 0;
unsigned long upstream_bitrate = 0;

/* startup time */
time_t startup_time = 0;

int runtime_flags = 0;

const char * pidfilename = "/var/run/miniupnpd.pid";
#ifdef __SC_BUILD__
char uuidvalue_igd[] = "uuid:fc4ec57e-28b1-11db-88f8-a16830956233";
char uuidvalue_wan[] = "uuid:fc4ec57e-2753-11db-88f8-0060085db3f6";
char uuidvalue_wcd[] = "uuid:fc4ec57e-092a-11db-88f8-0578ab52457c";
#else
char uuidvalue_igd[] = "uuid:00000000-0000-0000-0000-000000000000";
char uuidvalue_wan[] = "uuid:00000000-0000-0000-0000-000000000000";
char uuidvalue_wcd[] = "uuid:00000000-0000-0000-0000-000000000000";
#endif
char serialnumber[SERIALNUMBER_MAX_LEN] = "00000000";

char modelnumber[MODELNUMBER_MAX_LEN] = "1";

/* presentation url :
 * http://nnn.nnn.nnn.nnn:ppppp/  => max 30 bytes including terminating 0 */
char presentationurl[PRESENTATIONURL_MAX_LEN];

/* friendly name for root devices in XML description */
char friendly_name[FRIENDLY_NAME_MAX_LEN] = OS_NAME " router";
#ifdef __SC_BUILD__
int ext_ifid = -1;
char manufacture[NAME_MAX_LEN];
char module_number[NAME_MAX_LEN];
char device_name[NAME_MAX_LEN];
int upnp_mode = 0x3;
char ext_if_actype[NAME_MAX_LEN];
char module_des[NAME_MAX_LEN];
char module_name[NAME_MAX_LEN];
char module_url[NAME_MAX_LEN];
char connectionType[NAME_MAX_LEN];
#ifdef IGD_V2
char igd1_mac_prefix[9];
#endif
#endif
/* UPnP permission rules : */
struct upnpperm * upnppermlist = 0;
unsigned int num_upnpperm = 0;

#ifdef ENABLE_NATPMP
/* NAT-PMP */
#if 0
unsigned int nextnatpmptoclean_timestamp = 0;
unsigned short nextnatpmptoclean_eport = 0;
unsigned short nextnatpmptoclean_proto = 0;
#endif
#endif

/* For automatic removal of expired rules (with LeaseDuration) */
unsigned int nextruletoclean_timestamp = 0;

#ifdef USE_PF
const char * anchor_name = "miniupnpd";
const char * queue = 0;
const char * tag = 0;
#endif

#ifdef USE_NETFILTER
/* chain name to use, both in the nat table
 * and the filter table */
#ifdef __SC_BUILD__
const char * miniupnpd_nat_chain = "PRE_UPNP";
const char * miniupnpd_forward_chain = "FWD_UPNP";
#else
const char * miniupnpd_nat_chain = "MINIUPNPD";
const char * miniupnpd_forward_chain = "MINIUPNPD";
#ifdef ENABLE_6FC_SERVICE
const char * miniupnpd_v6_filter_chain = "MINIUPNPD";
#endif
#endif


#endif
#ifdef ENABLE_NFQUEUE
int nfqueue = -1;
int n_nfqix = 0;
unsigned nfqix[MAX_LAN_ADDR];
#endif
struct lan_addr_list lan_addrs;

#ifdef ENABLE_IPV6
/* ipv6 address used for HTTP */
char ipv6_addr_for_http_with_brackets[64];
#endif

/* Path of the Unix socket used to communicate with MiniSSDPd */
const char * minissdpdsocketpath = "/var/run/minissdpd.sock";

/* BOOTID.UPNP.ORG and CONFIGID.UPNP.ORG */
unsigned int upnp_bootid = 1;
unsigned int upnp_configid = 1337;

#ifdef ENABLE_6FC_SERVICE
int ipv6fc_firewall_enabled = 1;
int ipv6fc_inbound_pinhole_allowed = 1;
#endif

#ifdef CONFIG_SUPPORT_PCPD
int enable_pcpd = 0;
#endif
#ifdef __SC_BUILD__
#ifdef IGD_V2
int arraynumber=0;
struct mac_ip_addr devicearray[DEVICE_ARRAY_SIZE] = {0};
#endif
#endif
