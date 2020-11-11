/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log.h>
#endif
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib/config.h"
#include "lib/event.h"
#include "lib/ipt.h"
#include "routing.h"
#include "firewall.h"
#include "hotspotd.h"
#ifdef __SC_BUILD__
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <utility.h>
#endif

#ifdef __SC_BUILD__
static char chain_input[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_output[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_service[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_forward[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_blocked[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_reject[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_proxy[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_garden[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_prert[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_postrt[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_qos[WIFI_IF_NUM][16 + IFNAMSIZ];
static char chain_auth[WIFI_IF_NUM][16 + IFNAMSIZ];
#else
static char chain_input[16 + IFNAMSIZ];
static char chain_output[16 + IFNAMSIZ];
static char chain_service[16 + IFNAMSIZ];
static char chain_forward[16 + IFNAMSIZ];
static char chain_blocked[16 + IFNAMSIZ];
static char chain_reject[16 + IFNAMSIZ];
static char chain_proxy[16 + IFNAMSIZ];
static char chain_garden[16 + IFNAMSIZ];
static char chain_prert[16 + IFNAMSIZ];
static char chain_postrt[16 + IFNAMSIZ];
static char chain_qos[16 + IFNAMSIZ];
static char chain_auth[16 + IFNAMSIZ];
#endif

static struct firewall_config {
	char iface[IFNAMSIZ];	/* Interface name */
#ifdef __SC_BUILD__
	char wifi_iface[WIFI_IF_NUM][IFNAMSIZ];	/* Interface name */
	char wifi_eap_iface[WIFI_IF_NUM][IFNAMSIZ];	/* Interface name */
	char filter_fwd_fon_ipv4[32];
	char filter_fwd_fon_ipv6[32];
#endif
	char filter_input_ipv4[32];
	char filter_forward_ipv4[32];
	char filter_output_ipv4[32];
	char mangle_prerouting_ipv4[32];
	char mangle_postrouting_ipv4[32];
	char filter_input_ipv6[32];
	char filter_forward_ipv6[32];
	char filter_output_ipv6[32];
	char mangle_prerouting_ipv6[32];
	char mangle_postrouting_ipv6[32];
	unsigned input_hammer_limit;
} cfg = { .iface = "" };

static int firewall_ipv4_init();
static int firewall_ipv4_deinit();
static void firewall_deinit();

#ifdef HOTSPOTD_IPV6
static int firewall_ipv6_init();
static int firewall_ipv6_deinit();
static int rt_policy_ipv6 = -1;
#endif

static bool fw_ipv4 = false, fw_ipv6 = false;
static bool fw_enable_ipfilter = false;
static int rt_policy_ipv4 = -1;


#define IPT_FILTER(...) { if (ipt_filter(__VA_ARGS__)) return -1; }
#define IPT_MANGLE(...) { if (ipt_mangle(__VA_ARGS__)) return -1; }
#define IPT_NAT(...) { if (ipt_nat(__VA_ARGS__)) return -1; }

#define IP6T_FILTER(...) { if (ip6t_filter(__VA_ARGS__)) return -1; }
#define IP6T_MANGLE(...) { if (ip6t_mangle(__VA_ARGS__)) return -1; }
#define IP6T_NAT(...) { if (ip6t_nat(__VA_ARGS__)) return -1; }

#ifdef __SC_BUILD__
#define SCMARK_PORT_S           22
#define SCMARK_PORT_B           (0x04 << SCMARK_PORT_S)
#define SCMARK_PORT_MASK        "0x3c00000"
static char* _get_fon_if_sc_mark_s(char *if_name)
{
    static char sc_mark[32] = {0};
    char *p = NULL;
    int port = 0;
    unsigned int mark = SCMARK_PORT_B;
    p = strstr(if_name, "wl");
    if(p)
    {
        p = p + 2;
        if(*p == '1')
            port = (*p-'0')*5;
        else
            port = (*p-'0')*5 + 1;
        p = strchr(if_name, '.');
        if(p)
        {
            p++;
            port += (*p - '0');
        }
        mark += port << SCMARK_PORT_S;
    }
    else
    {
        return "0x0/0x3c00000";
    }
    snprintf(sc_mark, sizeof(sc_mark), "0x%x/%s", mark, SCMARK_PORT_MASK);

    return sc_mark;
}
#endif

static void ifaddr_handler(struct event_signal *signal, const siginfo_t *info);
static struct event_signal ifaddr = {
	.handler = ifaddr_handler,
	.signal = SIGNAL_ROUTING_IFADDR,
};

// Resident firewall, blocking all traffic to hotspot interface
static int firewall_resident_apply() {
	return 0;
}

static int firewall_resident_init() {
	const char *ipt4path = config_get_string("firewall", "iptables", NULL);
	const char *ipt6path = config_get_string("firewall", "ip6tables", NULL);

	if (!ipt4path && !access("/usr/sbin/iptables", X_OK)) {
		ipt4path = "/usr/sbin/iptables";
	} else if (!ipt4path && !access("/sbin/iptables", X_OK)) {
		ipt4path = "/sbin/iptables";
	}

	if (!ipt6path && !access("/usr/sbin/ip6tables", X_OK)) {
		ipt6path = "/usr/sbin/ip6tables";
	} else if (!ipt6path && !access("/sbin/ip6tables", X_OK)) {
		ipt6path = "/sbin/ip6tables";
	}

	ipt_path(ipt4path, ipt6path);

	// Read config
	const char *iface = config_get_string("main", "iface", "");
	if(!hotspot_assertconf_string("main.iface", iface)) {
		return -1;
	}

	strncpy(cfg.iface, iface, sizeof(cfg.iface) - 1);

#ifdef __SC_BUILD__
    const char *fon_ip = config_get_string("redirect","addr", "192.168.182.1");
    const char *fon_prefix = config_get_string("redirect","prefix", "24");
    const char *fon_ip_secure  = config_get_string("main", "secure_lan_ip", "192.168.183.1");
    const char *fon_prefix_secure = config_get_string("main", "secure_lan_mask", "255.255.255.0");
	const char *iface_eap = config_get_string("main", "iface_eap", NULL);
    char fon_network[32];
    char fon_network_secure[32];
    sprintf(fon_network, "%s/%s", fon_ip, fon_prefix);
    sprintf(fon_network_secure, "%s/%s", fon_ip_secure, fon_prefix_secure);

	const char *wifi_iface = config_get_string("main", "wifi_iface", "");
	if(!hotspot_assertconf_string("main.wifi_iface", wifi_iface)) {
		return -1;
	}
	strncpy(cfg.wifi_iface[0], wifi_iface, sizeof(cfg.wifi_iface[0]) - 1);
	const char *wifi_eap_iface = config_get_string("main", "wifi_eap_iface", "");
	if(!hotspot_assertconf_string("main.wifi_eap_iface", wifi_eap_iface)) {
		return -1;
	}
	strncpy(cfg.wifi_eap_iface[0], wifi_eap_iface, sizeof(cfg.wifi_eap_iface[0]) - 1);
	const char *wifi_iface5 = config_get_string("main", "wifi_iface5", "");
	if(!hotspot_assertconf_string("main.wifi_iface5", wifi_iface5)) {
		return -1;
	}
	strncpy(cfg.wifi_iface[1], wifi_iface5, sizeof(cfg.wifi_iface[1]) - 1);
	const char *wifi_eap_iface5 = config_get_string("main", "wifi_eap_iface5", "");
	if(!hotspot_assertconf_string("main.wifi_eap_iface5", wifi_eap_iface5)) {
		return -1;
	}
	strncpy(cfg.wifi_eap_iface[1], wifi_eap_iface5, sizeof(cfg.wifi_eap_iface[1]) - 1);
	strncpy(cfg.filter_fwd_fon_ipv4,
		config_get_string("main", "ipv4_fwd_fon_chain", "FWD_FON"),
		sizeof(cfg.filter_fwd_fon_ipv4) - 1);
	strncpy(cfg.filter_fwd_fon_ipv6,
		config_get_string("main", "ipv6_fwd_fon_chain", "FWD_FON"),
		sizeof(cfg.filter_fwd_fon_ipv6) - 1);
#endif

	strncpy(cfg.filter_forward_ipv4,
		config_get_string("firewall", "ipv4_forward_chain", "FORWARD"),
		sizeof(cfg.filter_forward_ipv4) - 1);
	strncpy(cfg.filter_forward_ipv6,
		config_get_string("firewall", "ipv6_forward_chain", "FORWARD"),
		sizeof(cfg.filter_forward_ipv6) - 1);
	strncpy(cfg.filter_input_ipv4,
		config_get_string("firewall", "ipv4_input_chain", "INPUT"),
		sizeof(cfg.filter_input_ipv4) - 1);
	strncpy(cfg.filter_output_ipv4,
		config_get_string("firewall", "ipv4_output_chain", "OUTPUT"),
		sizeof(cfg.filter_output_ipv4) - 1);
	strncpy(cfg.filter_input_ipv6,
		config_get_string("firewall", "ipv6_input_chain", "INPUT"),
		sizeof(cfg.filter_input_ipv6) - 1);
	strncpy(cfg.filter_output_ipv6,
		config_get_string("firewall", "ipv6_output_chain", "OUTPUT"),
		sizeof(cfg.filter_output_ipv6) - 1);
	strncpy(cfg.mangle_prerouting_ipv4,
		config_get_string("firewall", "ipv4_prerouting_chain", "PREROUTING"),
		sizeof(cfg.mangle_prerouting_ipv4) - 1);
	strncpy(cfg.mangle_prerouting_ipv6,
		config_get_string("firewall", "ipv6_prerouting_chain", "PREROUTING"),
		sizeof(cfg.mangle_prerouting_ipv6) - 1);
	strncpy(cfg.mangle_postrouting_ipv4,
		config_get_string("firewall", "ipv4_postrouting_chain", "POSTROUTING"),
		sizeof(cfg.mangle_postrouting_ipv4) - 1);
	strncpy(cfg.mangle_postrouting_ipv6,
		config_get_string("firewall", "ipv6_postrouting_chain", "POSTROUTING"),
		sizeof(cfg.mangle_postrouting_ipv6) - 1);

	cfg.input_hammer_limit = config_get_int("firewall", "hammer_times", 0);

	// Make out fw rulenames unique
#ifdef __SC_BUILD__
    int i = 0;
    ipt_filter(IPT_NEWCHAIN, cfg.filter_fwd_fon_ipv4);
    ipt_filter(IPT_FLUSHCHAIN, cfg.filter_fwd_fon_ipv4);
    ipt_filter(IPT_PREPEND, cfg.filter_forward_ipv4, "-j", cfg.filter_fwd_fon_ipv4);
    ipt_filter(IPT_APPEND, cfg.filter_fwd_fon_ipv4, "-i", cfg.iface, "-d", fon_network, "-j", "DROP");
    ipt_filter(IPT_APPEND, cfg.filter_fwd_fon_ipv4, "-i", routing_cfg.iface_eap_name, "-d", fon_network_secure, "-j", "DROP");

    ip6t_filter(IPT_NEWCHAIN, cfg.filter_fwd_fon_ipv6);
    ip6t_filter(IPT_FLUSHCHAIN, cfg.filter_fwd_fon_ipv6);
    ip6t_filter(IPT_PREPEND, cfg.filter_forward_ipv6, "-j", cfg.filter_fwd_fon_ipv6);
    ip6t_filter(IPT_APPEND, cfg.filter_fwd_fon_ipv6, "-i", cfg.iface, "-d", fon_network, "-j", "DROP");
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        strcpy(chain_input[i], "hotspot_input_");
        strcat(chain_input[i], cfg.wifi_iface[i]);

        strcpy(chain_output[i], "hotspot_output_");
        strcat(chain_output[i], cfg.wifi_iface[i]);

        strcpy(chain_service[i], "hotspot_service_");
        strcat(chain_service[i], cfg.wifi_iface[i]);

        strcpy(chain_forward[i], "hotspot_forward_");
        strcat(chain_forward[i], cfg.wifi_iface[i]);

        strcpy(chain_blocked[i], "hotspot_blocked_");
        strcat(chain_blocked[i], cfg.wifi_iface[i]);

        strcpy(chain_reject[i], "hotspot_reject_");
        strcat(chain_reject[i], cfg.wifi_iface[i]);

        strcpy(chain_proxy[i], "hotspot_proxy_");
        strcat(chain_proxy[i], cfg.wifi_iface[i]);

        strcpy(chain_garden[i], "hotspot_garden_");
        strcat(chain_garden[i], cfg.wifi_iface[i]);

        strcpy(chain_prert[i], "hotspot_prert_");
        strcat(chain_prert[i], cfg.wifi_iface[i]);

        strcpy(chain_postrt[i], "hotspot_postrt_");
        strcat(chain_postrt[i], cfg.wifi_iface[i]);

        strcpy(chain_qos[i], "hotspot_qos_");
        strcat(chain_qos[i], cfg.wifi_iface[i]);

        strcpy(chain_auth[i], "hotspot_auth_");
        strcat(chain_auth[i], cfg.wifi_iface[i]);
#else
	strcpy(chain_input, "hotspot_input_");
	strcat(chain_input, cfg.iface);

	strcpy(chain_output, "hotspot_output_");
	strcat(chain_output, cfg.iface);

	strcpy(chain_service, "hotspot_service_");
	strcat(chain_service, cfg.iface);

	strcpy(chain_forward, "hotspot_forward_");
	strcat(chain_forward, cfg.iface);

	strcpy(chain_blocked, "hotspot_blocked_");
	strcat(chain_blocked, cfg.iface);

	strcpy(chain_reject, "hotspot_reject_");
	strcat(chain_reject, cfg.iface);

	strcpy(chain_proxy, "hotspot_proxy_");
	strcat(chain_proxy, cfg.iface);

	strcpy(chain_garden, "hotspot_garden_");
	strcat(chain_garden, cfg.iface);

	strcpy(chain_prert, "hotspot_prert_");
	strcat(chain_prert, cfg.iface);

	strcpy(chain_postrt, "hotspot_postrt_");
	strcat(chain_postrt, cfg.iface);

	strcpy(chain_qos, "hotspot_qos_");
	strcat(chain_qos, cfg.iface);

	strcpy(chain_auth, "hotspot_auth_");
	strcat(chain_auth, cfg.iface);
#endif

	if (cfg.iface[0]) {
#ifdef __SC_BUILD__
        ipt_filter(IPT_PREPEND, cfg.filter_fwd_fon_ipv4,"-o", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", "DROP");
        ipt_filter(IPT_PREPEND, cfg.filter_input_ipv4,"-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", "DROP");
        ip6t_filter(IPT_PREPEND, cfg.filter_fwd_fon_ipv6,"-o", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", "DROP");
        ip6t_filter(IPT_PREPEND, cfg.filter_input_ipv6,"-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", "DROP");
        if(iface_eap[0] && strcmp(iface_eap,cfg.iface))
        {
            ipt_filter(IPT_PREPEND, cfg.filter_fwd_fon_ipv4,"-o", iface_eap,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", "DROP");
            ipt_filter(IPT_PREPEND, cfg.filter_input_ipv4,"-i", iface_eap,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", "DROP");
            ip6t_filter(IPT_PREPEND, cfg.filter_fwd_fon_ipv6,"-o", iface_eap,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", "DROP");
            ip6t_filter(IPT_PREPEND, cfg.filter_input_ipv6,"-i", iface_eap,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", "DROP");
        }
#else
		ipt_filter(IPT_PREPEND, cfg.filter_forward_ipv4,
						"-i", cfg.iface, "-j", "DROP");
		ipt_filter(IPT_PREPEND, cfg.filter_input_ipv4,
						"-i", cfg.iface, "-j", "DROP");
		ip6t_filter(IPT_PREPEND, cfg.filter_forward_ipv6,
						"-i", cfg.iface, "-j", "DROP");
		ip6t_filter(IPT_PREPEND, cfg.filter_input_ipv6,
						"-i", cfg.iface, "-j", "DROP");
#endif
	}
#ifdef __SC_BUILD__
    }
#endif
	return 0;
}

static void firewall_resident_deinit() {
	if (cfg.iface[0]) {
#ifdef __SC_BUILD__
    
        ipt_filter(IPT_FLUSHCHAIN, cfg.filter_fwd_fon_ipv4);
        ip6t_filter(IPT_FLUSHCHAIN, cfg.filter_fwd_fon_ipv6);
        ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-j", cfg.filter_fwd_fon_ipv4);
        ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv6, "-j", cfg.filter_fwd_fon_ipv6);
        ipt_filter(IPT_DELETECHAIN, cfg.filter_fwd_fon_ipv4);
        ip6t_filter(IPT_DELETECHAIN, cfg.filter_fwd_fon_ipv6);
        int i = 0;
        for(i = 0; i < WIFI_IF_NUM; i++)
        {
            ipt_filter(IPT_DELETE, cfg.filter_input_ipv4,"-i", cfg.iface,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", "DROP");
            ip6t_filter(IPT_DELETE, cfg.filter_input_ipv6,"-i", cfg.iface,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", "DROP");
            ipt_filter(IPT_DELETE, cfg.filter_input_ipv4,"-i", routing_cfg.iface_eap_name,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", "DROP");
            ip6t_filter(IPT_DELETE, cfg.filter_input_ipv6,"-i", routing_cfg.iface_eap_name,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", "DROP");
        }
#else
		ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4,
						"-i", cfg.iface, "-j", "DROP");
		ipt_filter(IPT_DELETE, cfg.filter_input_ipv4,
						"-i", cfg.iface, "-j", "DROP");
		ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv6,
						"-i", cfg.iface, "-j", "DROP");
		ip6t_filter(IPT_DELETE, cfg.filter_input_ipv6,
						"-i", cfg.iface, "-j", "DROP");
#endif
	}
}


// Hotspot firewall, setup when hotspot is operating
static int firewall_apply() {
#ifdef __SC_BUILD__
    int i = 0;
#endif
	firewall_set_service(IPPROTO_UDP | FIREWALL_SERVICE_EXTERNAL, 53, false);
	if (!!config_get_int("firewall", "anydns", 0))
		firewall_set_service(IPPROTO_UDP | FIREWALL_SERVICE_EXTERNAL, 53, true);

#ifdef __SC_BUILD__
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        if (fw_ipv4)
            ipt_filter(IPT_FLUSHCHAIN, chain_service[i]);

        if (fw_ipv6) {
            ip6t_filter(IPT_FLUSHCHAIN, chain_service[i]);
            ip6t_filter(IPT_APPEND, chain_service[i], "-p", "icmpv6", "--icmpv6-type", "router-solicitation", "-j", "ACCEPT");
            ip6t_filter(IPT_APPEND, chain_service[i], "-p", "icmpv6", "--icmpv6-type", "neighbour-solicitation", "-j", "ACCEPT");
            ip6t_filter(IPT_APPEND, chain_service[i], "-p", "icmpv6", "--icmpv6-type", "neighbour-advertisement", "-j", "ACCEPT");
        }
    }
#else
    if (fw_ipv4)
        ipt_filter(IPT_FLUSHCHAIN, chain_service);

    if (fw_ipv6) {
        ip6t_filter(IPT_FLUSHCHAIN, chain_service);
        ip6t_filter(IPT_APPEND, chain_service, "-p", "icmpv6", "--icmpv6-type", "router-solicitation", "-j", "ACCEPT");
        ip6t_filter(IPT_APPEND, chain_service, "-p", "icmpv6", "--icmpv6-type", "neighbour-solicitation", "-j", "ACCEPT");
        ip6t_filter(IPT_APPEND, chain_service, "-p", "icmpv6", "--icmpv6-type", "neighbour-advertisement", "-j", "ACCEPT");
    }
#endif
	// Open predefined ports
	const char *ports = config_get_string("firewall", "services_udp", "");
	while (*ports) {
		while (*ports && (*ports < '0' || *ports > '9'))
			ports++;
		int port = strtol(ports, (char**)&ports, 10);
		if (port > 0 && port < 65536)
			firewall_set_service(IPPROTO_UDP, port, true);
	}

	// Open predefined ports
	ports = config_get_string("firewall", "services_tcp", "");
	while (*ports) {
		while (*ports && (*ports < '0' || *ports > '9'))
			ports++;
		int port = strtol(ports, (char**)&ports, 10);
		if (port > 0 && port < 65536)
			firewall_set_service(IPPROTO_TCP, port, true);
	}

	return 0;
}

static int firewall_init() {
	fw_ipv4 = config_get_int("main", "ipv4", 0);
#ifdef __SC_BUILD__
	int iosock = socket(AF_UNIX, SOCK_DGRAM, 0);
	struct ifreq ifr;

	/* Detect ifindex of main interface */
	const char *iface = config_get_string("main", "wifi_iface", NULL);

	strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
	if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "1Unable to resolve iface: %s\n", ifr.ifr_name);
		close(iosock);
		return -1;
	}
	routing_cfg.wifi_iface_index[0] = ifr.ifr_ifindex;
        close(iosock);

	iosock = socket(AF_UNIX, SOCK_DGRAM, 0);
	iface = config_get_string("main", "wifi_eap_iface", NULL);
	strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
	if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "2Unable to resolve iface: %s\n", ifr.ifr_name);
		close(iosock);
		return -1;
	}
	routing_cfg.wifi_eap_iface_index[0] = ifr.ifr_ifindex;
        close(iosock);

	iosock = socket(AF_UNIX, SOCK_DGRAM, 0);
	iface = config_get_string("main", "wifi_iface5", NULL);
	strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
	if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "3Unable to resolve iface: (%s)\n", ifr.ifr_name);
		close(iosock);
		return -1;
	}
	routing_cfg.wifi_iface_index[1] = ifr.ifr_ifindex;
        close(iosock);

	iosock = socket(AF_UNIX, SOCK_DGRAM, 0);
	iface = config_get_string("main", "wifi_eap_iface5", NULL);
	strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
	if (ioctl(iosock, SIOCGIFINDEX, &ifr)) {
        log_fon(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "4Unable to resolve iface: %s\n", ifr.ifr_name);
		close(iosock);
		return -1;
	}

	routing_cfg.wifi_eap_iface_index[1] = ifr.ifr_ifindex;
        close(iosock);
#endif

	if (!fw_ipv4)
		fw_ipv4 = !!config_get_string("main", "addr_ipv4", "")[0];

#ifdef HOTSPOTD_IPV6
	fw_ipv6 = config_get_int("main", "ipv6", 0);
	if (!fw_ipv6)
		fw_ipv6 = !!config_get_string("main", "addr_ipv6", "")[0];

	if (fw_ipv6) {
		firewall_ipv6_deinit(); // TODO: Remove, should not be necessary
		if (firewall_ipv6_init())
			goto err;
		rt_policy_ipv6 = routing_policy_new(cfg.iface, AF_INET6,
				FIREWALL_TPROXY, FIREWALL_AUTHMASK, 0);
	}
#endif

	if (fw_ipv4) {
		firewall_ipv4_deinit(); // TODO: Remove, should not be necessary
		if (firewall_ipv4_init())
			goto err;
		rt_policy_ipv4 = routing_policy_new(cfg.iface, AF_INET,
				FIREWALL_TPROXY, FIREWALL_AUTHMASK, 0);
	}

	event_ctl(EVENT_SIGNAL_ADD, &ifaddr);
	return firewall_apply();

err:
	firewall_deinit();
	return -1;
}

static void firewall_deinit() {
	event_ctl(EVENT_SIGNAL_DEL, &ifaddr);

	if (fw_ipv4) {
		firewall_ipv4_deinit();
		routing_policy_del(rt_policy_ipv4);
	}

#ifdef HOTSPOTD_IPV6
	if (fw_ipv6) {
		firewall_ipv6_deinit();
		routing_policy_del(rt_policy_ipv6);
	}
#endif
}


static void ifaddr_handler(struct event_signal *signal, const siginfo_t *info)
{
	char ipbuf[INET6_ADDRSTRLEN];
	int af = info->si_errno;
	int ifindex = info->si_code;
#ifdef __SC_BUILD__
    int j = 0;
#endif
	if (ifindex != routing_cfg.iface_eap_index
			&& ifindex != routing_cfg.iface_index)
		return;

	if (af == AF_INET && fw_ipv4) {
		struct in_addr addr[8];
		ssize_t c = routing_addresses(ifindex, af, addr, 8);
#ifdef __SC_BUILD__
        for(j = 0; j < WIFI_IF_NUM; j++)
        {
            ipt_filter(IPT_FLUSHCHAIN, chain_blocked[j]);
            for (ssize_t i = 0; i < c; ++i) {
                inet_ntop(AF_INET, &addr[i], ipbuf, sizeof(ipbuf));
                ipt_filter(IPT_APPEND, chain_blocked[j], "-d", ipbuf, chain_reject[j]);
            }
        }
#else
		ipt_filter(IPT_FLUSHCHAIN, chain_blocked);
		for (ssize_t i = 0; i < c; ++i) {
			inet_ntop(AF_INET, &addr[i], ipbuf, sizeof(ipbuf));
			ipt_filter(IPT_APPEND, chain_blocked, "-d", ipbuf, chain_reject);
		}
#endif

#ifdef HOTSPOTD_IPV6
	} else if (af == AF_INET6 && fw_ipv6) {
		struct in6_addr addr[8];
		ssize_t c = routing_addresses(ifindex, af, addr, 8);
#ifdef __SC_BUILD__
        for(j = 0; j < WIFI_IF_NUM; j++)
        {
            ip6t_filter(IPT_FLUSHCHAIN, chain_blocked[j]);
            for (ssize_t i = 0; i < c; ++i) {
                inet_ntop(AF_INET6, &addr[i], ipbuf, sizeof(ipbuf));
                ip6t_filter(IPT_APPEND, chain_blocked[j], "-d", ipbuf, chain_reject[j]);
            }
        }
#else
		ip6t_filter(IPT_FLUSHCHAIN, chain_blocked);
		for (ssize_t i = 0; i < c; ++i) {
			inet_ntop(AF_INET6, &addr[i], ipbuf, sizeof(ipbuf));
			ip6t_filter(IPT_APPEND, chain_blocked, "-d", ipbuf, chain_reject);
		}
#endif

#endif
	}
}

// Open ports in input chain
int firewall_set_service(int proto, int port, bool accept) {
	char buffer[32], markbuf[32];
	snprintf(buffer, sizeof(buffer), "%i", port);
	snprintf(markbuf, sizeof(markbuf), "0x%x", FIREWALL_WHITELIST);

	int action = (accept) ? IPT_APPEND : IPT_DELETE;

	const char *prot = NULL;
	switch (proto & 0xffff) {
		case IPPROTO_UDP:
			prot = "udp";
			break;

		case IPPROTO_TCP:
			prot = "tcp";
			break;

		default:
			return -1;
	}
#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        if (proto & FIREWALL_SERVICE_EXTERNAL) {
            if (fw_ipv4)
                IPT_MANGLE(action, chain_garden[i], "-p", prot, "--dport",
                        buffer, "-j", "MARK", "--or-mark", markbuf);

            if (fw_ipv6)
                IP6T_MANGLE(action, chain_garden[i], "-p", prot, "--dport",
                        buffer, "-j", "MARK", "--or-mark", markbuf);
        } else {
            if (fw_ipv4)
                IPT_FILTER(action, chain_service[i], "-p", prot, "--dport",
                        buffer, "-j", "ACCEPT");

            if (fw_ipv6)
                IP6T_FILTER(action, chain_service[i], "-p", prot, "--dport",
                        buffer, "-j", "ACCEPT");
        }
    }
#else
	if (proto & FIREWALL_SERVICE_EXTERNAL) {
		if (fw_ipv4)
			IPT_MANGLE(action, chain_garden, "-p", prot, "--dport",
				buffer, "-j", "MARK", "--or-mark", markbuf);

		if (fw_ipv6)
			IP6T_MANGLE(action, chain_garden, "-p", prot, "--dport",
				buffer, "-j", "MARK", "--or-mark", markbuf);
	} else {
		if (fw_ipv4)
			IPT_FILTER(action, chain_service, "-p", prot, "--dport",
							buffer, "-j", "ACCEPT");

		if (fw_ipv6)
			IP6T_FILTER(action, chain_service, "-p", prot, "--dport",
							buffer, "-j", "ACCEPT");
	}
#endif

	return 0;
}


// Open ports in input chain for specific authed client
int firewall_set_client_service(const uint32_t id, int proto, 
			const char *port, bool accept) {
	char idmark[32], markbuf[32];
#ifdef __SC_BUILD__
        int i = 0;
#endif
	snprintf(idmark, sizeof(idmark), "0/0x%x", firewall_id_to_authmark(id));
	snprintf(markbuf, sizeof(markbuf), "0x%x", FIREWALL_WHITELIST);

	int action = (accept) ? IPT_APPEND : IPT_DELETE;

	const char *prot = NULL;
	switch (proto & 0xffff) {
		case IPPROTO_UDP:
			prot = "udp";
			break;

		case IPPROTO_TCP:
			prot = "tcp";
			break;

		default:
			return -1;
	}

	if (proto & FIREWALL_SERVICE_EXTERNAL) {
#ifdef __SC_BUILD__
        for(i = 0; i < WIFI_IF_NUM; i++)
        {
            if (fw_ipv4)
                IPT_MANGLE(action, chain_garden[i], "-p", prot, "--dport",
                        port, "-m", "mark", "!", "--mark", idmark, "-j", "MARK",
                        "--or-mark", markbuf);

            if (fw_ipv6)
                IP6T_MANGLE(action, chain_garden[i], "-p", prot, "--dport",
                        port, "-m", "mark", "!", "--mark", idmark, "-j", "MARK",
                        "--or-mark", markbuf);
        }
#else
		if (fw_ipv4)
			IPT_MANGLE(action, chain_garden, "-p", prot, "--dport",
				port, "-m", "mark", "!", "--mark", idmark, "-j", "MARK",
				"--or-mark", markbuf);

		if (fw_ipv6)
			IP6T_MANGLE(action, chain_garden, "-p", prot, "--dport",
				port, "-m", "mark", "!", "--mark", idmark, "-j", "MARK",
				"--or-mark", markbuf);
#endif
	} else {
#ifdef __SC_BUILD__
        for(i = 0; i < WIFI_IF_NUM; i++)
        {
            if (fw_ipv4)
                IPT_FILTER(action, chain_service[i], "-p", prot, "--dport",
                        port, "-m", "mark", "!", "--mark", idmark, "-j", "ACCEPT");

            if (fw_ipv6)
                IP6T_FILTER(action, chain_service[i], "-p", prot, "--dport",
                        port, "-m", "mark", "!", "--mark", idmark, "-j", "ACCEPT");
        }
#else
		if (fw_ipv4)
			IPT_FILTER(action, chain_service, "-p", prot, "--dport",
				port, "-m", "mark", "!", "--mark", idmark, "-j", "ACCEPT");

		if (fw_ipv6)
			IP6T_FILTER(action, chain_service, "-p", prot, "--dport",
				port, "-m", "mark", "!", "--mark", idmark, "-j", "ACCEPT");
#endif
	}

	return 0;
}

// Add host to whitelist
int firewall_set_whitelist(int af, const void *addr,
						uint8_t prefix, bool accept) {
    char buffer[64] = {0}, markbuf[32];
    int action = (accept) ? IPT_APPEND : IPT_DELETE;
#ifdef __SC_BUILD__
    int i = 0;
    char privoxy_ip[64] = {"1.1.1.227/32"};
#endif
    inet_ntop(af, addr, buffer, sizeof(buffer));
    snprintf(buffer + strlen(buffer), 5, "/%u", (unsigned)prefix);
    snprintf(markbuf, sizeof(markbuf), "0x%x", FIREWALL_WHITELIST);

#ifdef __SC_BUILD__
    if(strncmp(buffer, privoxy_ip, sizeof(privoxy_ip)) == 0)
    {
        return 0;
    }
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        if (af == AF_INET && fw_ipv4) {
            IPT_MANGLE(action, chain_garden[i], "-d", buffer,
					"-j", "MARK", "--or-mark", markbuf);
        } else if (af == AF_INET6 && fw_ipv6) {
            IP6T_MANGLE(action, chain_garden[i], "-d", buffer,
                    "-j", "MARK", "--or-mark", markbuf);
        } else {
            return -1;
        }
    }
#else
	if (af == AF_INET && fw_ipv4) {
		IPT_MANGLE(action, chain_garden, "-d", buffer,
					"-j", "MARK", "--or-mark", markbuf);
	} else if (af == AF_INET6 && fw_ipv6) {
		IP6T_MANGLE(action, chain_garden, "-d", buffer,
					"-j", "MARK", "--or-mark", markbuf);
	} else {
		return -1;
	}
#endif
	return 0;
}

// Mark a client's traffic with its userid
int firewall_set_auth(int ifindex, const uint8_t *hwaddr, uint32_t id, bool set) {
	char macbuf[24];
	snprintf(macbuf, sizeof(macbuf), "%02x:%02x:%02x:%02x:%02x:%02x",
		hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);

	const char *iface = (ifindex == routing_cfg.iface_eap_index)
			? routing_cfg.iface_eap_name : routing_cfg.iface_name;

	char chain[48];
#ifndef __SC_BUILD__
	snprintf(chain, sizeof(chain), "%s_%u", chain_auth, id);
#else
    int i = 0;
#endif
	int action = (set) ? IPT_APPEND : IPT_DELETE;

	if (fw_enable_ipfilter) {
#ifdef __SC_BUILD__
        for(i = 0; i < WIFI_IF_NUM; i++)
        {
            snprintf(chain, sizeof(chain), "%s_%u", chain_auth[i], id);
            if (set) {
                if (fw_ipv4)
                    IPT_MANGLE(IPT_NEWCHAIN, chain);

                if (fw_ipv6)
                    IP6T_MANGLE(IPT_NEWCHAIN, chain);
            }


            if (fw_ipv4)
                IPT_MANGLE(action, chain_auth[i], "-i", iface, "-m", "mac",
                        "--mac-source", macbuf, "-j", chain);

            if (fw_ipv6)
                IP6T_MANGLE(action, chain_auth[i], "-i", iface, "-m", "mac",
                        "--mac-source", macbuf, "-j", chain);

            if (!set) {
                firewall_flush_auth_ips(id);

                if (fw_ipv4) {
                    IPT_MANGLE(IPT_DELETECHAIN, chain);
                }

                if (fw_ipv6) {
                    IP6T_MANGLE(IPT_DELETECHAIN, chain);
                }
            }
        }
#else
		if (set) {
			if (fw_ipv4)
				IPT_MANGLE(IPT_NEWCHAIN, chain);

			if (fw_ipv6)
				IP6T_MANGLE(IPT_NEWCHAIN, chain);
		}


		if (fw_ipv4)
			IPT_MANGLE(action, chain_auth, "-i", iface, "-m", "mac",
			"--mac-source", macbuf, "-j", chain);

		if (fw_ipv6)
			IP6T_MANGLE(action, chain_auth, "-i", iface, "-m", "mac",
			"--mac-source", macbuf, "-j", chain);

		if (!set) {
			firewall_flush_auth_ips(id);

			if (fw_ipv4) {
				IPT_MANGLE(IPT_DELETECHAIN, chain);
			}

			if (fw_ipv6) {
				IP6T_MANGLE(IPT_DELETECHAIN, chain);
			}
		}
#endif
	} else {
		char markbuf[32];
		snprintf(markbuf, sizeof(markbuf), "0x%x",
				firewall_id_to_authmark(id));

#ifdef __SC_BUILD__
        for(i = 0; i < WIFI_IF_NUM; i++)
        {
		if (fw_ipv4)
			IPT_MANGLE(action, chain_auth[i], "-i", iface, "-m", "mac",
					"--mac-source", macbuf, "-j", "MARK",
					"--or-mark", markbuf);

		if (fw_ipv6)
			IP6T_MANGLE(action, chain_auth[i], "-i", iface, "-m", "mac",
					"--mac-source", macbuf, "-j", "MARK",
					"--or-mark", markbuf);
        }
#else
		if (fw_ipv4)
			IPT_MANGLE(action, chain_auth, "-i", iface, "-m", "mac",
					"--mac-source", macbuf, "-j", "MARK",
					"--or-mark", markbuf);

		if (fw_ipv6)
			IP6T_MANGLE(action, chain_auth, "-i", iface, "-m", "mac",
					"--mac-source", macbuf, "-j", "MARK",
					"--or-mark", markbuf);
#endif
	}

	return 0;
}


void firewall_flush_auth_ips(uint32_t id) {
	if (!fw_enable_ipfilter)
		return;

	char chain[48];
#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        snprintf(chain, sizeof(chain), "%s_%u", chain_auth[i], id);
#else
	snprintf(chain, sizeof(chain), "%s_%u", chain_auth, id);
#endif

	if (fw_ipv4)
		ipt_mangle(IPT_FLUSHCHAIN, chain);

	if (fw_ipv6)
		ip6t_mangle(IPT_FLUSHCHAIN, chain);
#ifdef __SC_BUILD__
    }
#endif
}


int firewall_add_auth_ip(uint32_t id, int af, const void *addr) {
	if ((!fw_ipv4 && af == AF_INET) || (!fw_ipv6 && af == AF_INET6) || !fw_enable_ipfilter)
		return 0;

	char chain[48];
#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        snprintf(chain, sizeof(chain), "%s_%u", chain_auth[i], id);
#else
	snprintf(chain, sizeof(chain), "%s_%u", chain_auth, id);
#endif

	char ipaddr[INET6_ADDRSTRLEN];
	inet_ntop(af, addr, ipaddr, sizeof(ipaddr));

	if (addr) {
		char markbuf[32];
		snprintf(markbuf, sizeof(markbuf), "0x%x",
				firewall_id_to_authmark(id));

		if (af == AF_INET) {
			IPT_MANGLE(IPT_APPEND, chain, "-s", ipaddr,
					"-j", "MARK", "--or-mark", markbuf);
		} else if (af == AF_INET6) {
			IP6T_MANGLE(IPT_APPEND, chain, "-s", ipaddr,
					"-j", "MARK", "--or-mark", markbuf);
		}
	}
#ifdef __SC_BUILD__
    }
#endif
	return 0;
}

int firewall_set_nfqueue_out(int proto, int port, uint32_t grp, bool set, bool nfqueue) {
	char portbuf[16], groupbuf[16];
	snprintf(portbuf, sizeof(portbuf), "%i", port);
	snprintf(groupbuf, sizeof(groupbuf), "%u", grp);
	int action = (set) ? IPT_APPEND : IPT_DELETE;

	const char *prot = NULL;
	switch (proto) {
		case IPPROTO_UDP:
			prot = "udp";
			break;

		case IPPROTO_TCP:
			prot = "tcp";
			break;

		default:
			return -1;
	}

	const char *target = (nfqueue) ? "NFQUEUE" : "NFLOG";
	const char *groupname = (nfqueue) ? "--queue-num" : "--nflog-group";

#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        if (fw_ipv4)
            IPT_MANGLE(action, chain_output[i], "-p", prot,
                    "--sport", portbuf,
                    "-j", target, groupname, groupbuf);

        if (fw_ipv6)
            IP6T_MANGLE(action, chain_output[i], "-p", prot,
                    "--sport", portbuf,
                    "-j", target, groupname, groupbuf);
    }
#else
	if (fw_ipv4)
		IPT_MANGLE(action, chain_output, "-p", prot,
				"--sport", portbuf,
				"-j", target, groupname, groupbuf);

	if (fw_ipv6)
		IP6T_MANGLE(action, chain_output, "-p", prot,
				"--sport", portbuf,
				"-j", target, groupname, groupbuf);
#endif
	return 0;
}

int firewall_set_nfqueue_src(int family, const void *addr,
		int proto, int port, uint32_t grp, bool set, bool nfqueue) {
	char portbuf[16], groupbuf[16], addrbuf[INET6_ADDRSTRLEN];
	snprintf(portbuf, sizeof(portbuf), "%i", port);
	snprintf(groupbuf, sizeof(groupbuf), "%u", grp);
	int action = (set) ? IPT_APPEND : IPT_DELETE;

	const char *prot = NULL;
	switch (proto) {
		case IPPROTO_UDP:
			prot = "udp";
			break;

		case IPPROTO_TCP:
			prot = "tcp";
			break;

		default:
			return -1;
	}

	inet_ntop(family, addr, addrbuf, sizeof(addrbuf));

	const char *target = (nfqueue) ? "NFQUEUE" : "NFLOG";
	const char *groupname = (nfqueue) ? "--queue-num" : "--nflog-group";

#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        if (family == AF_INET6) {
            IP6T_MANGLE(action, chain_postrt[i], "-s", addrbuf, "-p", prot,
                    "--sport", portbuf, "-j", target,
                    groupname, groupbuf);
        } else if (family == AF_INET) {
            IPT_MANGLE(action, chain_postrt[i], "-s", addrbuf, "-p", prot,
                    "--sport", portbuf, "-j", target,
                    groupname, groupbuf);
        }
    }
#else
	if (family == AF_INET6) {
		IP6T_MANGLE(action, chain_postrt, "-s", addrbuf, "-p", prot,
				"--sport", portbuf, "-j", target,
				groupname, groupbuf);
	} else if (family == AF_INET) {
		IPT_MANGLE(action, chain_postrt, "-s", addrbuf, "-p", prot,
				"--sport", portbuf, "-j", target,
				groupname, groupbuf);
	}
#endif

	return 0;
}

// Create a transparent proxy for a port (captive feature)
int firewall_set_redirect(int proto, int port, int dport, bool redirect) {
	char buffer[16], dbuffer[16], markbuf[16], tmarkbuf[32];
	snprintf(buffer, sizeof(buffer), "%i", port);
	snprintf(dbuffer, sizeof(dbuffer), "%i", dport);
	snprintf(markbuf, sizeof(markbuf), "%u", FIREWALL_TPROXY);
	snprintf(tmarkbuf, sizeof(tmarkbuf), "%u/%u",
				FIREWALL_TPROXY, FIREWALL_AUTHMASK);

	int action = (redirect) ? IPT_APPEND : IPT_DELETE;
#ifdef __SC_BUILD__
#ifdef CONFIG_SUPPORT_WEB_PRIVOXY
    const char *privoxy_port = NULL;
    char destination[64] = {0};
    const char *ip = NULL;
    ip = config_get_string("main", "open_lan_ip", "192.168.182.1");
    privoxy_port = config_get_string("main", "privoxy_port", "8118");
    snprintf(destination, sizeof(destination), "%s:%i", ip,atoi(privoxy_port));
#endif
#endif

	const char *prot = NULL;
	switch (proto) {
		case IPPROTO_UDP:
			prot = "udp";
			break;

		case IPPROTO_TCP:
			prot = "tcp";
			break;

		default:
			return -1;
	}

#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        if (fw_ipv4) {
            if (ipt_mangle(action, chain_proxy[i], "-p", prot, "--dport",
                        buffer, "-j", "TPROXY", "--on-port",
                        dbuffer, "--tproxy-mark", tmarkbuf)) {
                IPT_MANGLE(action, chain_proxy[i], "-p", prot, "--dport",
                        buffer, "-j", "MARK", "--or-mark", markbuf);
#ifdef CONFIG_SUPPORT_WEB_PRIVOXY
                ipt_nat(action, chain_proxy[i], "-d", "1.1.1.227", "-p", prot, "--dport",
                            buffer, "-j", "DNAT", "--to-destination", destination);
                if (ipt_nat(action, chain_proxy[i], "-p", prot, "!", "-d", "1.1.1.227", "--dport",
                            buffer, "-j", "REDIRECT", "--to-ports", dbuffer)) {
#else
                if (ipt_nat(action, chain_proxy[i], "-p", prot, "--dport",
                            buffer, "-j", "REDIRECT", "--to-ports", dbuffer)) {
#endif
                    ipt_mangle(IPT_DELETE, chain_proxy[i], "-p", prot,
                            "--dport", buffer, "-j",
                            "MARK", "--or-mark", markbuf);
                    return -1;
                }
            }
        }

        if (fw_ipv6)
            IP6T_MANGLE(action, chain_proxy[i], "-p", prot, "--dport", buffer,
                    "-j", "TPROXY", "--on-port", dbuffer,
                    "--tproxy-mark", tmarkbuf);
    }
#else
	if (fw_ipv4) {
		if (ipt_mangle(action, chain_proxy, "-p", prot, "--dport",
					buffer, "-j", "TPROXY", "--on-port",
					dbuffer, "--tproxy-mark", tmarkbuf)) {
			IPT_MANGLE(action, chain_proxy, "-p", prot, "--dport",
				buffer, "-j", "MARK", "--or-mark", markbuf);
			if (ipt_nat(action, chain_proxy, "-p", prot, "--dport",
			buffer, "-j", "REDIRECT", "--to-ports", dbuffer)) {
				ipt_mangle(IPT_DELETE, chain_proxy, "-p", prot,
						"--dport", buffer, "-j",
						"MARK", "--or-mark", markbuf);
				return -1;
			}
		}
	}

	if (fw_ipv6)
		IP6T_MANGLE(action, chain_proxy, "-p", prot, "--dport", buffer,
					"-j", "TPROXY", "--on-port", dbuffer,
					"--tproxy-mark", tmarkbuf);
#endif
	return 0;
}

// NAT rules
int firewall_set_nat(const char *dev, bool set) {
#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        if (fw_ipv6) {
            ip6t_filter((set) ? IPT_PREPEND : IPT_DELETE, chain_forward[i], "-i", dev,
                    "-m", "state", "!", "--state", "ESTABLISHED,RELATED",
                    "-j", chain_reject[i]);

            IP6T_NAT((set) ? IPT_APPEND : IPT_DELETE, chain_postrt[i],
                    "-o", dev, "-j", "MASQUERADE");
        }

        if (fw_ipv4) {
            ipt_filter((set) ? IPT_PREPEND : IPT_DELETE, chain_forward[i], "-i", dev,
                    "-m", "state", "!", "--state", "ESTABLISHED,RELATED",
                    "-j", chain_reject[i]);
            ipt_filter((set) ? IPT_PREPEND : IPT_DELETE, cfg.filter_forward_ipv4, "-i", dev, "-o", cfg.iface,
                    "-j", chain_forward[i]);
            if (routing_cfg.iface_eap_index &&
                    routing_cfg.iface_eap_index != routing_cfg.iface_index) {
                ipt_filter((set) ? IPT_PREPEND : IPT_DELETE, cfg.filter_forward_ipv4, "-i", dev, "-o", routing_cfg.iface_eap_name,
                        "-j", chain_forward[i]);
            }
            IPT_NAT((set) ? IPT_APPEND : IPT_DELETE, chain_postrt[i],
                    "-o", dev, "-j", "MASQUERADE");
        }
    }
#else
	if (fw_ipv6) {
		ip6t_filter((set) ? IPT_PREPEND : IPT_DELETE, chain_forward, "-i", dev,
				"-m", "state", "!", "--state", "ESTABLISHED,RELATED",
				"-j", chain_reject);

		IP6T_NAT((set) ? IPT_APPEND : IPT_DELETE, chain_postrt,
					"-o", dev, "-j", "MASQUERADE");
	}

	if (fw_ipv4) {
		ipt_filter((set) ? IPT_PREPEND : IPT_DELETE, chain_forward, "-i", dev,
			"-m", "state", "!", "--state", "ESTABLISHED,RELATED",
			"-j", chain_reject);
		IPT_NAT((set) ? IPT_APPEND : IPT_DELETE, chain_postrt,
					"-o", dev, "-j", "MASQUERADE");
	}
#endif

	return 0;
}


static int firewall_ipv4_deinit() {
	syslog(LOG_INFO, "firewall: deinitializing IPv4 chains");
	int stderr = dup(STDERR_FILENO);

	int fd = open("/dev/null", O_WRONLY);
	dup2(fd, STDERR_FILENO);
	close(fd);

	char buffer[32];
#ifdef __SC_BUILD__
    const char *wan_iface = config_get_string("main", "wan_iface", "eth4");
    const char *ip = NULL;
    const char *mask = NULL;
    const char *ip_secure = NULL;
    const char *mask_secure = NULL;
    ip = config_get_string("main", "open_lan_ip", "192.168.182.1");
    mask = config_get_string("main", "open_lan_mask", "255.255.255.0");
    ip_secure = config_get_string("main", "secure_lan_ip", "192.168.183.1");
    mask_secure = config_get_string("main", "secure_lan_mask", "255.255.255.0");
	/* unhook external chains */
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        ipt_filter(IPT_DELETE, cfg.filter_input_ipv4, "-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_input[i]);
        if(i == 0)
        {
            snprintf(buffer, sizeof(buffer), "%s/%s", ip,mask);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "%s/%s", ip_secure, mask_secure);
        }
        ipt_filter(IPT_DELETE, cfg.filter_output_ipv4, "-o", cfg.iface, "-d", buffer, "-j", chain_output[i]);
        ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_forward[i]);
        ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-o", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_forward[i]);
        ipt_mangle(IPT_DELETE, cfg.filter_output_ipv4, "-o", cfg.iface, "-d", buffer, "-j", chain_output[i]);
        ipt_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_prert[i]);
        ipt_mangle(IPT_DELETE, cfg.mangle_postrouting_ipv4, "-o", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_postrt[i]);
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_TPROXY, FIREWALL_AUTHMASK);
        ipt_nat(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-m", "mark", "--mark", buffer, "-j", chain_proxy[i]);
        ipt_nat(IPT_DELETE, cfg.mangle_postrouting_ipv4, 
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_postrt[i]);

	/* hooks for eap interface */
        if (routing_cfg.wifi_eap_iface_index[i] &&
                routing_cfg.wifi_eap_iface_index[i] != routing_cfg.wifi_iface_index[i]) {
            if(i == 0)
            {
                snprintf(buffer, sizeof(buffer), "%s/%s", ip,mask);
            }
            else
            {
                snprintf(buffer, sizeof(buffer), "%s/%s", ip_secure, mask_secure);
            }
            ipt_nat(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-m", "mark", "--mark", buffer, "-j", chain_proxy[i]);
            ipt_nat(IPT_DELETE, cfg.mangle_postrouting_ipv4, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_postrt[i]);
            ipt_filter(IPT_DELETE, cfg.filter_input_ipv4, "-i", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_input[i]);
            if(strcmp(cfg.iface, routing_cfg.iface_eap_name))
                ipt_filter(IPT_DELETE, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name,"-d", buffer, "-j", chain_output[i]);
            ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-i", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_forward[i]);
            ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-o", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_forward[i]);
            ipt_mangle(IPT_DELETE, cfg.mangle_postrouting_ipv4, "-o", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_postrt[i]);
            ipt_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_prert[i]);
            ipt_mangle(IPT_DELETE, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name,"-d", buffer, "-j", chain_output[i]);
        }
    }
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
#ifdef CONFIG_SUPPORT_HA
        if(strcmp(wan_iface, "dummy0") == 0)       
        { 
            ipt_mangle(IPT_DELETE, "OUTPUT_FON",  "-j", "SC_CONNMARK", "--restore-mark",
                    "--ctmask", "0xf0000000", "--nfmask", SCMARK_PORT_MASK);
        }
        else
#endif
        {
            ipt_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", wan_iface, "-j", "SC_CONNMARK", "--restore-mark",
                    "--ctmask", "0xf0000000", "--nfmask", SCMARK_PORT_MASK);
        }
        ipt_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", cfg.iface, "-j", "SC_CONNMARK", "--save-mark",
                "--ctmask", "0xf0000000", "--nfmask", SCMARK_PORT_MASK);
        if (routing_cfg.iface_eap_index &&
                routing_cfg.iface_eap_index != routing_cfg.iface_index)
        {
            ipt_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name, "-j", "SC_CONNMARK", "--save-mark",
                    "--ctmask", "0xf0000000", "--nfmask", SCMARK_PORT_MASK);
        }
        ipt_filter(IPT_DELETE, chain_forward[i], "-j", chain_blocked[i]);

        ipt_filter(IPT_FLUSHCHAIN, chain_input[i]);
        ipt_filter(IPT_FLUSHCHAIN, chain_output[i]);
        ipt_filter(IPT_FLUSHCHAIN, chain_service[i]);
        ipt_filter(IPT_FLUSHCHAIN, chain_blocked[i]);
        ipt_filter(IPT_FLUSHCHAIN, chain_forward[i]);
        ipt_filter(IPT_FLUSHCHAIN, chain_reject[i]);
        ipt_mangle(IPT_FLUSHCHAIN, chain_garden[i]);
        ipt_mangle(IPT_FLUSHCHAIN, chain_proxy[i]);
        ipt_mangle(IPT_FLUSHCHAIN, chain_prert[i]);
        ipt_mangle(IPT_FLUSHCHAIN, chain_postrt[i]);
        ipt_mangle(IPT_FLUSHCHAIN, chain_qos[i]);
        ipt_mangle(IPT_FLUSHCHAIN, chain_auth[i]);
        ipt_mangle(IPT_FLUSHCHAIN, chain_output[i]);
        ipt_nat(IPT_FLUSHCHAIN, chain_proxy[i]);
        ipt_nat(IPT_FLUSHCHAIN, chain_postrt[i]);

        ipt_filter(IPT_DELETECHAIN, chain_input[i]);
        ipt_filter(IPT_DELETECHAIN, chain_output[i]);
        ipt_filter(IPT_DELETECHAIN, chain_service[i]);
        ipt_filter(IPT_DELETECHAIN, chain_blocked[i]);
        ipt_filter(IPT_DELETECHAIN, chain_forward[i]);
        ipt_filter(IPT_DELETECHAIN, chain_reject[i]);
        ipt_mangle(IPT_DELETECHAIN, chain_garden[i]);
        ipt_mangle(IPT_DELETECHAIN, chain_proxy[i]);
        ipt_mangle(IPT_DELETECHAIN, chain_prert[i]);
        ipt_mangle(IPT_DELETECHAIN, chain_postrt[i]);
        ipt_mangle(IPT_DELETECHAIN, chain_qos[i]);
        ipt_mangle(IPT_DELETECHAIN, chain_auth[i]);
        ipt_mangle(IPT_DELETECHAIN, chain_output[i]);
        ipt_nat(IPT_DELETECHAIN, chain_proxy[i]);
        ipt_nat(IPT_DELETECHAIN, chain_postrt[i]);
    }
#else
	ipt_filter(IPT_DELETE, cfg.filter_input_ipv4, "-i", cfg.iface, "-j", chain_input);
	ipt_filter(IPT_DELETE, cfg.filter_output_ipv4, "-o", cfg.iface, "-j", chain_output);
	ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-i", cfg.iface, "-j", chain_forward);
	ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-o", cfg.iface, "-j", chain_forward);
	ipt_mangle(IPT_DELETE, cfg.filter_output_ipv4, "-o", cfg.iface, "-j", chain_output);
	ipt_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", cfg.iface, "-j", chain_prert);
	ipt_mangle(IPT_DELETE, cfg.mangle_postrouting_ipv4, "-o", cfg.iface, "-j", chain_postrt);
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_TPROXY, FIREWALL_AUTHMASK);
	ipt_nat(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", cfg.iface, "-m", "mark", "--mark", buffer, "-j", chain_proxy);
	ipt_nat(IPT_DELETE, cfg.mangle_postrouting_ipv4, "-j", chain_postrt);

	/* hooks for eap interface */
	if (routing_cfg.iface_eap_index &&
			routing_cfg.iface_eap_index != routing_cfg.iface_index) {
		ipt_filter(IPT_DELETE, cfg.filter_input_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_input);
		ipt_filter(IPT_DELETE, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_output);
		ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_forward);
		ipt_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_forward);
		ipt_mangle(IPT_DELETE, cfg.mangle_postrouting_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_postrt);
		ipt_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_prert);
		ipt_mangle(IPT_DELETE, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_output);
	}
	ipt_filter(IPT_DELETE, chain_forward, "-j", chain_blocked);

	ipt_filter(IPT_FLUSHCHAIN, chain_input);
	ipt_filter(IPT_FLUSHCHAIN, chain_output);
	ipt_filter(IPT_FLUSHCHAIN, chain_service);
	ipt_filter(IPT_FLUSHCHAIN, chain_blocked);
	ipt_filter(IPT_FLUSHCHAIN, chain_forward);
	ipt_filter(IPT_FLUSHCHAIN, chain_reject);
	ipt_mangle(IPT_FLUSHCHAIN, chain_garden);
	ipt_mangle(IPT_FLUSHCHAIN, chain_proxy);
	ipt_mangle(IPT_FLUSHCHAIN, chain_prert);
	ipt_mangle(IPT_FLUSHCHAIN, chain_postrt);
	ipt_mangle(IPT_FLUSHCHAIN, chain_qos);
	ipt_mangle(IPT_FLUSHCHAIN, chain_auth);
	ipt_mangle(IPT_FLUSHCHAIN, chain_output);
	ipt_nat(IPT_FLUSHCHAIN, chain_proxy);
	ipt_nat(IPT_FLUSHCHAIN, chain_postrt);

	ipt_filter(IPT_DELETECHAIN, chain_input);
	ipt_filter(IPT_DELETECHAIN, chain_output);
	ipt_filter(IPT_DELETECHAIN, chain_service);
	ipt_filter(IPT_DELETECHAIN, chain_blocked);
	ipt_filter(IPT_DELETECHAIN, chain_forward);
	ipt_filter(IPT_DELETECHAIN, chain_reject);
	ipt_mangle(IPT_DELETECHAIN, chain_garden);
	ipt_mangle(IPT_DELETECHAIN, chain_proxy);
	ipt_mangle(IPT_DELETECHAIN, chain_prert);
	ipt_mangle(IPT_DELETECHAIN, chain_postrt);
	ipt_mangle(IPT_DELETECHAIN, chain_qos);
	ipt_mangle(IPT_DELETECHAIN, chain_auth);
	ipt_mangle(IPT_DELETECHAIN, chain_output);
	ipt_nat(IPT_DELETECHAIN, chain_proxy);
	ipt_nat(IPT_DELETECHAIN, chain_postrt);
#endif

	dup2(stderr, STDERR_FILENO);
	close(stderr);
	return 0;
}

static int firewall_ipv4_init() {
    char buffer[32], buffer2[32];
#ifdef __SC_BUILD__
    const char *wan_iface = config_get_string("main", "wan_iface", "eth4");
    const char *ip = NULL;
    const char *mask = NULL;
    const char *ip_secure = NULL;
    const char *mask_secure = NULL;
    ip = config_get_string("main", "open_lan_ip", "192.168.182.1");
    mask = config_get_string("main", "open_lan_mask", "255.255.255.0");
    ip_secure = config_get_string("main", "secure_lan_ip", "192.168.183.1");
    mask_secure = config_get_string("main", "secure_lan_mask", "255.255.255.0");
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        ipt_filter(IPT_NEWCHAIN, chain_input[i]);
        ipt_filter(IPT_NEWCHAIN, chain_output[i]);
        ipt_filter(IPT_NEWCHAIN, chain_service[i]);
        ipt_filter(IPT_NEWCHAIN, chain_forward[i]);
        ipt_filter(IPT_NEWCHAIN, chain_blocked[i]);
        ipt_filter(IPT_NEWCHAIN, chain_reject[i]);
        ipt_mangle(IPT_NEWCHAIN, chain_garden[i]);
        ipt_mangle(IPT_NEWCHAIN, chain_proxy[i]);
        ipt_mangle(IPT_NEWCHAIN, chain_prert[i]);
        ipt_mangle(IPT_NEWCHAIN, chain_postrt[i]);
        ipt_mangle(IPT_NEWCHAIN, chain_qos[i]);
        ipt_mangle(IPT_NEWCHAIN, chain_auth[i]);
        ipt_mangle(IPT_NEWCHAIN, chain_output[i]);
        ipt_nat(IPT_NEWCHAIN, chain_proxy[i]);
        ipt_nat(IPT_NEWCHAIN, chain_postrt[i]);

        IPT_FILTER(IPT_PREPEND, cfg.filter_input_ipv4, "-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_input[i]);
        if(i == 0)
        {
            snprintf(buffer, sizeof(buffer), "%s/%s", ip,mask);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "%s/%s", ip_secure, mask_secure);
        }
        IPT_FILTER(IPT_PREPEND, cfg.filter_output_ipv4, "-o", cfg.iface, "-d", buffer, "-j", chain_output[i]);
        IPT_FILTER(IPT_PREPEND, cfg.filter_forward_ipv4, "-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_forward[i]);
        IPT_FILTER(IPT_PREPEND, cfg.filter_forward_ipv4, "-o", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_forward[i]);
        IPT_MANGLE(IPT_PREPEND, cfg.mangle_postrouting_ipv4, "-o", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_postrt[i]);
        IPT_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_prert[i]);
        IPT_MANGLE(IPT_PREPEND, cfg.filter_output_ipv4, "-o", cfg.iface, "-d", buffer, "-j", chain_output[i]);
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_TPROXY, FIREWALL_AUTHMASK);
        ipt_nat(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", cfg.iface,
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-m", "mark", "--mark", buffer, "-j", chain_proxy[i]);
        ipt_nat(IPT_PREPEND, cfg.mangle_postrouting_ipv4, 
                "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_postrt[i]);

	/* hooks for eap interface */
        if (routing_cfg.wifi_eap_iface_index[i] &&
                routing_cfg.wifi_eap_iface_index[i] != routing_cfg.wifi_iface_index[i]) {
            if(i == 0)
            {
                snprintf(buffer, sizeof(buffer), "%s/%s", ip,mask);
            }
            else
            {
                snprintf(buffer, sizeof(buffer), "%s/%s", ip_secure, mask_secure);
            }
            ipt_nat(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name,
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-m", "mark", "--mark", buffer, "-j", chain_proxy[i]);
            ipt_nat(IPT_PREPEND, cfg.mangle_postrouting_ipv4, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_postrt[i]);
            IPT_FILTER(IPT_PREPEND, cfg.filter_input_ipv4, "-i", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_input[i]);
            if(strcmp(cfg.iface, routing_cfg.iface_eap_name))
                IPT_FILTER(IPT_PREPEND, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name,"-d", buffer, "-j", chain_output[i]);
            IPT_FILTER(IPT_PREPEND, cfg.filter_forward_ipv4, "-i", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_forward[i]);
            IPT_FILTER(IPT_PREPEND, cfg.filter_forward_ipv4, "-o", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_forward[i]);
            IPT_MANGLE(IPT_PREPEND, cfg.mangle_postrouting_ipv4, "-o", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_postrt[i]);
            IPT_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_prert[i]);
            if (routing_cfg.iface_eap_index &&
                    routing_cfg.iface_eap_index != routing_cfg.iface_index)
                IPT_MANGLE(IPT_PREPEND, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name,"-d", buffer, "-j", chain_output[i]);
        }
#ifdef CONFIG_SUPPORT_HA
        if(strcmp(wan_iface, "dummy0") == 0)       
        { 
            IPT_MANGLE(IPT_PREPEND, "OUTPUT_FON",  "-j", "SC_CONNMARK", "--restore-mark",
                    "--ctmask", "0xf0000000", "--nfmask", SCMARK_PORT_MASK);
        }
        else
#endif
        {
            IPT_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", wan_iface, "-j", "SC_CONNMARK", "--restore-mark",
                    "--ctmask", "0xf0000000", "--nfmask", SCMARK_PORT_MASK);
        }
        IPT_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", cfg.iface, "-j", "SC_CONNMARK", "--save-mark",
                "--ctmask", "0xf0000000", "--nfmask", SCMARK_PORT_MASK);
        if (routing_cfg.iface_eap_index &&
                routing_cfg.iface_eap_index != routing_cfg.iface_index)
        {
            IPT_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name, "-j", "SC_CONNMARK", "--save-mark",
                    "--ctmask", "0xf0000000", "--nfmask", SCMARK_PORT_MASK);
        }
        /* prert */
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
        IPT_MANGLE(IPT_APPEND, chain_prert[i], "-j", "CONNMARK", "--restore-mark", "--mask", buffer);
        snprintf(buffer, sizeof(buffer), "0/%u", FIREWALL_AUTHMASK);
        IPT_MANGLE(IPT_APPEND, chain_prert[i], "-m", "mark", "--mark", buffer, "-m", "state", "--state", "new", "-j", chain_auth[i]);
        IPT_MANGLE(IPT_APPEND, chain_prert[i], "-i", cfg.iface, "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), 
                "-m", "mark", "--mark", buffer, "-m", "state", "--state", "new", "-j", chain_garden[i]);
        IPT_MANGLE(IPT_APPEND, chain_prert[i], "-i", cfg.iface, "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), 
                "-m", "mark", "--mark", buffer, "-j", chain_proxy[i]);
        IPT_MANGLE(IPT_APPEND, chain_prert[i], "-j", chain_qos[i]);
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
        IPT_MANGLE(IPT_APPEND, chain_prert[i], "-j", "CONNMARK", "--save-mark", "--mask", buffer);

        if (!strncmp(routing_cfg.ifb_name, "imq", 3)) {
            IPT_MANGLE(IPT_APPEND, chain_prert[i], "-j", "IMQ", "--todev", routing_cfg.ifb_name + 3);
            IPT_MANGLE(IPT_APPEND, chain_postrt[i], "-j", "IMQ", "--todev", routing_cfg.ifb2_name + 3);
        }

        /* postrt */
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
        IPT_MANGLE(IPT_APPEND, chain_postrt[i], "-j", "CONNMARK", "--restore-mark", "--mask", buffer);
        IPT_MANGLE(IPT_APPEND, chain_postrt[i], "-j", chain_qos[i]);
        IPT_MANGLE(IPT_APPEND, chain_postrt[i], "-j", "CONNMARK", "--save-mark", "--mask", buffer);
        /* qos */
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_CLS_BULK, FIREWALL_CLASSMASK);
        IPT_MANGLE(IPT_APPEND, chain_qos[i], "-m", "mark", "--mark", buffer, "-j", "RETURN");

        snprintf(buffer, sizeof(buffer), "%u", ~((unsigned)FIREWALL_CLASSMASK));
        IPT_MANGLE(IPT_APPEND, chain_qos[i], "-j", "MARK", "--and-mark", buffer);
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_CLS_SMALL);
        IPT_MANGLE(IPT_APPEND, chain_qos[i], "-m", "connbytes", "--connbytes", "0:131071", "--connbytes-dir", "both", "--connbytes-mode", "bytes", "-j", "MARK", "--or-mark", buffer);
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_CLS_INTER);
        IPT_MANGLE(IPT_APPEND, chain_qos[i], "-p", "udp", "-m", "length", "--length", "0:1023", "-j", "MARK", "--or-mark", buffer);
        IPT_MANGLE(IPT_APPEND, chain_qos[i], "-m", "connbytes", "--connbytes", "0:511", "--connbytes-dir", "both", "--connbytes-mode", "avgpkt", "-j", "MARK", "--or-mark", buffer);

        snprintf(buffer, sizeof(buffer), "%u/%u", 0, FIREWALL_CLASSMASK);
        snprintf(buffer2, sizeof(buffer2), "%u", FIREWALL_CLS_BULK);
        IPT_MANGLE(IPT_APPEND, chain_qos[i], "-m", "mark", "--mark", buffer, "-j", "MARK", "--or-mark", buffer2);

        /* reject chain */
        IPT_FILTER(IPT_APPEND, chain_reject[i], "-p", "tcp", "-j", "REJECT", "--reject-with", "tcp-reset");
        IPT_FILTER(IPT_APPEND, chain_reject[i], "-j", "REJECT");
        IPT_FILTER(IPT_APPEND, chain_reject[i], "-j", "DROP");

        /* output chain */
        IPT_FILTER(IPT_APPEND, chain_output[i], "-j", "ACCEPT");

        /* input default policy is reject */
        IPT_FILTER(IPT_APPEND, chain_input[i], "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
        IPT_FILTER(IPT_APPEND, chain_input[i], "-m", "state", "--state", "INVALID", "-j", chain_reject[i]);

        /* detect connmark killbit */
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_KILL, FIREWALL_AUTHMASK);
        IPT_FILTER(IPT_APPEND, chain_input[i], "-m", "mark", "--mark", buffer, "-j", chain_reject[i]);
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_TPROXY, FIREWALL_AUTHMASK);
        IPT_FILTER(IPT_APPEND, chain_input[i], "-m", "mark", "--mark", buffer, "-j", "ACCEPT");

        /* prevent hammering */
        if (cfg.input_hammer_limit) {
            snprintf(buffer, sizeof(buffer), "%u", cfg.input_hammer_limit);
            IPT_FILTER(IPT_APPEND, chain_input[i], "-m", "recent", "--name", chain_input[i], "--update", "--seconds", "1", "--hitcount", buffer, "-j", chain_reject[i]);
            IPT_FILTER(IPT_APPEND, chain_input[i], "-m", "recent", "--name", chain_input[i], "--set");
        }

        /* allow proxied traffic, allowed services, reject remaining */
        IPT_FILTER(IPT_APPEND, chain_input[i], "-j", chain_service[i]);
        IPT_FILTER(IPT_APPEND, chain_input[i], "-j", chain_reject[i]);

        /* detect connmark killbit */
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_KILL, FIREWALL_AUTHMASK);
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-m", "mark", "--mark", buffer, "-j", chain_reject[i]);

        /* default forwarding */
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-m", "state", "--state", "INVALID", "-j", chain_reject[i]);

        /* drop old TCP connections from being reestablished */
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-p", "tcp", "!", "--syn", "-m", "state", "--state", "NEW", "-j", chain_reject[i]);
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-o", cfg.iface, "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_iface[i]), "-j", chain_reject[i]);
        if (routing_cfg.wifi_eap_iface_index[i] &&
                routing_cfg.wifi_eap_iface_index[i] != routing_cfg.wifi_iface_index[i])
        {
            IPT_FILTER(IPT_APPEND, chain_forward[i], "-o", routing_cfg.iface_eap_name, 
                    "-m", "sc_mark", "--sc_mark", _get_fon_if_sc_mark_s(cfg.wifi_eap_iface[i]), "-j", chain_reject[i]);
        }
        /* disallow private prefixes */
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-d", "192.168.0.0/16", "-j", chain_reject[i]);
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-d", "172.16.0.0/12", "-j", chain_reject[i]);
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-d", "10.0.0.0/8", "-j", chain_reject[i]);
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-j", chain_blocked[i]);

        /* allow authenticated and whitelisted traffic, drop remaining */
        snprintf(buffer, sizeof(buffer), "0/%u", FIREWALL_AUTHMASK);
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-m", "mark", "!", "--mark", buffer, "-j", "ACCEPT");
        IPT_FILTER(IPT_APPEND, chain_forward[i], "-j", "DROP");
    }
#else

	ipt_filter(IPT_NEWCHAIN, chain_input);
	ipt_filter(IPT_NEWCHAIN, chain_output);
	ipt_filter(IPT_NEWCHAIN, chain_service);
	ipt_filter(IPT_NEWCHAIN, chain_forward);
	ipt_filter(IPT_NEWCHAIN, chain_blocked);
	ipt_filter(IPT_NEWCHAIN, chain_reject);
	ipt_mangle(IPT_NEWCHAIN, chain_garden);
	ipt_mangle(IPT_NEWCHAIN, chain_proxy);
	ipt_mangle(IPT_NEWCHAIN, chain_prert);
	ipt_mangle(IPT_NEWCHAIN, chain_postrt);
	ipt_mangle(IPT_NEWCHAIN, chain_qos);
	ipt_mangle(IPT_NEWCHAIN, chain_auth);
	ipt_mangle(IPT_NEWCHAIN, chain_output);
	ipt_nat(IPT_NEWCHAIN, chain_proxy);
	ipt_nat(IPT_NEWCHAIN, chain_postrt);

	/* hook external chains */
    IPT_FILTER(IPT_PREPEND, cfg.filter_input_ipv4, "-i", cfg.iface, "-j", chain_input);
    IPT_FILTER(IPT_PREPEND, cfg.filter_output_ipv4, "-o", cfg.iface, "-j", chain_output);
    IPT_FILTER(IPT_PREPEND, cfg.filter_forward_ipv4, "-i", cfg.iface, "-j", chain_forward);
    IPT_FILTER(IPT_PREPEND, cfg.filter_forward_ipv4, "-o", cfg.iface, "-j", chain_forward);
    IPT_MANGLE(IPT_PREPEND, cfg.mangle_postrouting_ipv4, "-o", cfg.iface, "-j", chain_postrt);
    IPT_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", cfg.iface, "-j", chain_prert);
    IPT_MANGLE(IPT_PREPEND, cfg.filter_output_ipv4, "-o", cfg.iface, "-j", chain_output);
    snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_TPROXY, FIREWALL_AUTHMASK);
	ipt_nat(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", cfg.iface, "-m", "mark", "--mark", buffer, "-j", chain_proxy);
	ipt_nat(IPT_PREPEND, cfg.mangle_postrouting_ipv4, "-j", chain_postrt);

	/* hooks for eap interface */
	if (routing_cfg.iface_eap_index &&
			routing_cfg.iface_eap_index != routing_cfg.iface_index) {
		IPT_FILTER(IPT_PREPEND, cfg.filter_input_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_input);
		IPT_FILTER(IPT_PREPEND, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_output);
		IPT_FILTER(IPT_PREPEND, cfg.filter_forward_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_forward);
		IPT_FILTER(IPT_PREPEND, cfg.filter_forward_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_forward);
		IPT_MANGLE(IPT_PREPEND, cfg.mangle_postrouting_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_postrt);
		IPT_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_prert);
		IPT_MANGLE(IPT_PREPEND, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_output);
	}
	/* prert */
	snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
	IPT_MANGLE(IPT_APPEND, chain_prert, "-j", "CONNMARK", "--restore-mark", "--mask", buffer);
	snprintf(buffer, sizeof(buffer), "0/%u", FIREWALL_AUTHMASK);
	IPT_MANGLE(IPT_APPEND, chain_prert, "-m", "mark", "--mark", buffer, "-m", "state", "--state", "new", "-j", chain_auth);

	/* qos */
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_CLS_BULK, FIREWALL_CLASSMASK);
	IPT_MANGLE(IPT_APPEND, chain_qos, "-m", "mark", "--mark", buffer, "-j", "RETURN");

	snprintf(buffer, sizeof(buffer), "%u", ~((unsigned)FIREWALL_CLASSMASK));
	IPT_MANGLE(IPT_APPEND, chain_qos, "-j", "MARK", "--and-mark", buffer);
	snprintf(buffer, sizeof(buffer), "%u", FIREWALL_CLS_SMALL);
	IPT_MANGLE(IPT_APPEND, chain_qos, "-m", "connbytes", "--connbytes", "0:131071", "--connbytes-dir", "both", "--connbytes-mode", "bytes", "-j", "MARK", "--or-mark", buffer);
	snprintf(buffer, sizeof(buffer), "%u", FIREWALL_CLS_INTER);
	IPT_MANGLE(IPT_APPEND, chain_qos, "-p", "udp", "-m", "length", "--length", "0:1023", "-j", "MARK", "--or-mark", buffer);
	IPT_MANGLE(IPT_APPEND, chain_qos, "-m", "connbytes", "--connbytes", "0:511", "--connbytes-dir", "both", "--connbytes-mode", "avgpkt", "-j", "MARK", "--or-mark", buffer);

	snprintf(buffer, sizeof(buffer), "%u/%u", 0, FIREWALL_CLASSMASK);
	snprintf(buffer2, sizeof(buffer2), "%u", FIREWALL_CLS_BULK);
	IPT_MANGLE(IPT_APPEND, chain_qos, "-m", "mark", "--mark", buffer, "-j", "MARK", "--or-mark", buffer2);

	/* reject chain */
	IPT_FILTER(IPT_APPEND, chain_reject, "-p", "tcp", "-j", "REJECT", "--reject-with", "tcp-reset");
	IPT_FILTER(IPT_APPEND, chain_reject, "-j", "REJECT");
	IPT_FILTER(IPT_APPEND, chain_reject, "-j", "DROP");

	/* output chain */
	IPT_FILTER(IPT_APPEND, chain_output, "-j", "ACCEPT");

	/* input default policy is reject */
	IPT_FILTER(IPT_APPEND, chain_input, "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
	IPT_FILTER(IPT_APPEND, chain_input, "-m", "state", "--state", "INVALID", "-j", chain_reject);

	/* detect connmark killbit */
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_KILL, FIREWALL_AUTHMASK);
	IPT_FILTER(IPT_APPEND, chain_input, "-m", "mark", "--mark", buffer, "-j", chain_reject);
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_TPROXY, FIREWALL_AUTHMASK);
	IPT_FILTER(IPT_APPEND, chain_input, "-m", "mark", "--mark", buffer, "-j", "ACCEPT");

	/* prevent hammering */
	if (cfg.input_hammer_limit) {
		snprintf(buffer, sizeof(buffer), "%u", cfg.input_hammer_limit);
		IPT_FILTER(IPT_APPEND, chain_input, "-m", "recent", "--name", chain_input, "--update", "--seconds", "1", "--hitcount", buffer, "-j", chain_reject);
		IPT_FILTER(IPT_APPEND, chain_input, "-m", "recent", "--name", chain_input, "--set");
	}

	/* allow proxied traffic, allowed services, reject remaining */
	IPT_FILTER(IPT_APPEND, chain_input, "-j", chain_service);
	IPT_FILTER(IPT_APPEND, chain_input, "-j", chain_reject);

	/* detect connmark killbit */
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_KILL, FIREWALL_AUTHMASK);
	IPT_FILTER(IPT_APPEND, chain_forward, "-m", "mark", "--mark", buffer, "-j", chain_reject);

	/* default forwarding */
	IPT_FILTER(IPT_APPEND, chain_forward, "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
	IPT_FILTER(IPT_APPEND, chain_forward, "-m", "state", "--state", "INVALID", "-j", chain_reject);

	/* drop old TCP connections from being reestablished */
	IPT_FILTER(IPT_APPEND, chain_forward, "-p", "tcp", "!", "--syn", "-m", "state", "--state", "NEW", "-j", chain_reject);
	IPT_FILTER(IPT_APPEND, chain_forward, "-o", cfg.iface, "-j", chain_reject);

	if (routing_cfg.iface_eap_index &&
			routing_cfg.iface_eap_index != routing_cfg.iface_index)
		IPT_FILTER(IPT_APPEND, chain_forward, "-o", routing_cfg.iface_eap_name, "-j", chain_reject);

	/* disallow private prefixes */
	IPT_FILTER(IPT_APPEND, chain_forward, "-d", "192.168.0.0/16", "-j", chain_reject);
	IPT_FILTER(IPT_APPEND, chain_forward, "-d", "172.16.0.0/12", "-j", chain_reject);
	IPT_FILTER(IPT_APPEND, chain_forward, "-d", "10.0.0.0/8", "-j", chain_reject);
	IPT_FILTER(IPT_APPEND, chain_forward, "-j", chain_blocked);

	/* allow authenticated and whitelisted traffic, drop remaining */
	snprintf(buffer, sizeof(buffer), "0/%u", FIREWALL_AUTHMASK);
	IPT_FILTER(IPT_APPEND, chain_forward, "-m", "mark", "!", "--mark", buffer, "-j", "ACCEPT");
	IPT_FILTER(IPT_APPEND, chain_forward, "-j", "DROP");
#endif

	return 0;
}

#ifdef HOTSPOTD_IPV6
static int firewall_ipv6_deinit() {
	syslog(LOG_INFO, "firewall: deinitializing IPv6 chains");
	int stderr = dup(STDERR_FILENO);

	int fd = open("/dev/null", O_WRONLY);
	dup2(fd, STDERR_FILENO);
	close(fd);
#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        /* unhook external chains */
        ip6t_filter(IPT_DELETE, cfg.filter_input_ipv6, "-i", cfg.iface, "-j", chain_input[i]);
        ip6t_filter(IPT_DELETE, cfg.filter_output_ipv6, "-o", cfg.iface, "-j", chain_output[i]);
        ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv6, "-i", cfg.iface, "-j", chain_forward[i]);
        ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv6, "-o", cfg.iface, "-j", chain_forward[i]);
        ip6t_mangle(IPT_DELETE, cfg.mangle_postrouting_ipv6, "-o", cfg.iface, "-j", chain_postrt[i]);
        ip6t_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv6, "-i", cfg.iface, "-j", chain_prert[i]);
        ip6t_mangle(IPT_DELETE, cfg.filter_output_ipv6, "-o", cfg.iface, "-j", chain_output[i]);


        /* hooks for eap interface */
        if (routing_cfg.iface_eap_index &&
                routing_cfg.iface_eap_index != routing_cfg.iface_index) {
            ip6t_filter(IPT_DELETE, cfg.filter_input_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_input[i]);
            ip6t_filter(IPT_DELETE, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_output[i]);
            ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_forward[i]);
            ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_forward[i]);
            ip6t_mangle(IPT_DELETE, cfg.mangle_postrouting_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_postrt[i]);
            ip6t_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_prert[i]);
            ip6t_mangle(IPT_DELETE, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_output[i]);
        }

        ip6t_filter(IPT_DELETE, chain_forward[i], "-j", chain_blocked[i]);

        ip6t_filter(IPT_FLUSHCHAIN, chain_input[i]);
        ip6t_filter(IPT_FLUSHCHAIN, chain_output[i]);
        ip6t_filter(IPT_FLUSHCHAIN, chain_service[i]);
        ip6t_filter(IPT_FLUSHCHAIN, chain_blocked[i]);
        ip6t_filter(IPT_FLUSHCHAIN, chain_forward[i]);
        ip6t_filter(IPT_FLUSHCHAIN, chain_reject[i]);
        ip6t_mangle(IPT_FLUSHCHAIN, chain_garden[i]);
        ip6t_mangle(IPT_FLUSHCHAIN, chain_proxy[i]);
        ip6t_mangle(IPT_FLUSHCHAIN, chain_prert[i]);
        ip6t_mangle(IPT_FLUSHCHAIN, chain_postrt[i]);
        ip6t_mangle(IPT_FLUSHCHAIN, chain_qos[i]);
        ip6t_mangle(IPT_FLUSHCHAIN, chain_auth[i]);
        ip6t_mangle(IPT_FLUSHCHAIN, chain_output[i]);

        ip6t_filter(IPT_DELETECHAIN, chain_input[i]);
        ip6t_filter(IPT_DELETECHAIN, chain_output[i]);
        ip6t_filter(IPT_DELETECHAIN, chain_service[i]);
        ip6t_filter(IPT_DELETECHAIN, chain_blocked[i]);
        ip6t_filter(IPT_DELETECHAIN, chain_forward[i]);
        ip6t_filter(IPT_DELETECHAIN, chain_reject[i]);
        ip6t_mangle(IPT_DELETECHAIN, chain_garden[i]);
        ip6t_mangle(IPT_DELETECHAIN, chain_proxy[i]);
        ip6t_mangle(IPT_DELETECHAIN, chain_prert[i]);
        ip6t_mangle(IPT_DELETECHAIN, chain_postrt[i]);
        ip6t_mangle(IPT_DELETECHAIN, chain_qos[i]);
        ip6t_mangle(IPT_DELETECHAIN, chain_auth[i]);
        ip6t_mangle(IPT_DELETECHAIN, chain_output[i]);
    }
#else
	/* unhook external chains */
	ip6t_filter(IPT_DELETE, cfg.filter_input_ipv6, "-i", cfg.iface, "-j", chain_input);
	ip6t_filter(IPT_DELETE, cfg.filter_output_ipv6, "-o", cfg.iface, "-j", chain_output);
	ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv6, "-i", cfg.iface, "-j", chain_forward);
	ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv6, "-o", cfg.iface, "-j", chain_forward);
	ip6t_mangle(IPT_DELETE, cfg.mangle_postrouting_ipv6, "-o", cfg.iface, "-j", chain_postrt);
	ip6t_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv6, "-i", cfg.iface, "-j", chain_prert);
	ip6t_mangle(IPT_DELETE, cfg.filter_output_ipv6, "-o", cfg.iface, "-j", chain_output);


	/* hooks for eap interface */
	if (routing_cfg.iface_eap_index &&
			routing_cfg.iface_eap_index != routing_cfg.iface_index) {
		ip6t_filter(IPT_DELETE, cfg.filter_input_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_input);
		ip6t_filter(IPT_DELETE, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_output);
		ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_forward);
		ip6t_filter(IPT_DELETE, cfg.filter_forward_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_forward);
		ip6t_mangle(IPT_DELETE, cfg.mangle_postrouting_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_postrt);
		ip6t_mangle(IPT_DELETE, cfg.mangle_prerouting_ipv4, "-i", routing_cfg.iface_eap_name, "-j", chain_prert);
		ip6t_mangle(IPT_DELETE, cfg.filter_output_ipv4, "-o", routing_cfg.iface_eap_name, "-j", chain_output);
	}

	ip6t_filter(IPT_DELETE, chain_forward, "-j", chain_blocked);

	ip6t_filter(IPT_FLUSHCHAIN, chain_input);
	ip6t_filter(IPT_FLUSHCHAIN, chain_output);
	ip6t_filter(IPT_FLUSHCHAIN, chain_service);
	ip6t_filter(IPT_FLUSHCHAIN, chain_blocked);
	ip6t_filter(IPT_FLUSHCHAIN, chain_forward);
	ip6t_filter(IPT_FLUSHCHAIN, chain_reject);
	ip6t_mangle(IPT_FLUSHCHAIN, chain_garden);
	ip6t_mangle(IPT_FLUSHCHAIN, chain_proxy);
	ip6t_mangle(IPT_FLUSHCHAIN, chain_prert);
	ip6t_mangle(IPT_FLUSHCHAIN, chain_postrt);
	ip6t_mangle(IPT_FLUSHCHAIN, chain_qos);
	ip6t_mangle(IPT_FLUSHCHAIN, chain_auth);
	ip6t_mangle(IPT_FLUSHCHAIN, chain_output);

	ip6t_filter(IPT_DELETECHAIN, chain_input);
	ip6t_filter(IPT_DELETECHAIN, chain_output);
	ip6t_filter(IPT_DELETECHAIN, chain_service);
	ip6t_filter(IPT_DELETECHAIN, chain_blocked);
	ip6t_filter(IPT_DELETECHAIN, chain_forward);
	ip6t_filter(IPT_DELETECHAIN, chain_reject);
	ip6t_mangle(IPT_DELETECHAIN, chain_garden);
	ip6t_mangle(IPT_DELETECHAIN, chain_proxy);
	ip6t_mangle(IPT_DELETECHAIN, chain_prert);
	ip6t_mangle(IPT_DELETECHAIN, chain_postrt);
	ip6t_mangle(IPT_DELETECHAIN, chain_qos);
	ip6t_mangle(IPT_DELETECHAIN, chain_auth);
	ip6t_mangle(IPT_DELETECHAIN, chain_output);
#endif

	dup2(stderr, STDERR_FILENO);
	close(stderr);
	return 0;
}

static int firewall_ipv6_init() {
	char buffer[32], buffer2[32];

#ifdef __SC_BUILD__
    int i = 0;
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        ip6t_filter(IPT_NEWCHAIN, chain_input[i]);
        ip6t_filter(IPT_NEWCHAIN, chain_output[i]);
        ip6t_filter(IPT_NEWCHAIN, chain_service[i]);
        ip6t_filter(IPT_NEWCHAIN, chain_forward[i]);
        ip6t_filter(IPT_NEWCHAIN, chain_blocked[i]);
        ip6t_filter(IPT_NEWCHAIN, chain_reject[i]);
        ip6t_mangle(IPT_NEWCHAIN, chain_garden[i]);
        ip6t_mangle(IPT_NEWCHAIN, chain_proxy[i]);
        ip6t_mangle(IPT_NEWCHAIN, chain_prert[i]);
        ip6t_mangle(IPT_NEWCHAIN, chain_postrt[i]);
        ip6t_mangle(IPT_NEWCHAIN, chain_qos[i]);
        ip6t_mangle(IPT_NEWCHAIN, chain_auth[i]);
        ip6t_mangle(IPT_NEWCHAIN, chain_output[i]);

        /* hook external chains */
        IP6T_FILTER(IPT_PREPEND, cfg.filter_input_ipv6, "-i", cfg.iface, "-j", chain_input[i]);
        IP6T_FILTER(IPT_PREPEND, cfg.filter_output_ipv6, "-o", cfg.iface, "-j", chain_output[i]);
        IP6T_FILTER(IPT_PREPEND, cfg.filter_forward_ipv6, "-i", cfg.iface, "-j", chain_forward[i]);
        IP6T_FILTER(IPT_PREPEND, cfg.filter_forward_ipv6, "-o", cfg.iface, "-j", chain_forward[i]);
        IP6T_MANGLE(IPT_PREPEND, cfg.mangle_postrouting_ipv6, "-o", cfg.iface, "-j", chain_postrt[i]);
        IP6T_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv6, "-i", cfg.iface, "-j", chain_prert[i]);
        IP6T_MANGLE(IPT_PREPEND, cfg.filter_output_ipv6, "-o", cfg.iface, "-j", chain_output[i]);

        /* hooks for eap interface */
        if (routing_cfg.iface_eap_index &&
                routing_cfg.iface_eap_index != routing_cfg.iface_index) {
            IP6T_FILTER(IPT_PREPEND, cfg.filter_input_ipv6, "-i", routing_cfg.iface_eap_name, "-j", chain_input[i]);
            IP6T_FILTER(IPT_PREPEND, cfg.filter_output_ipv6, "-o", routing_cfg.iface_eap_name, "-j", chain_output[i]);
            IP6T_FILTER(IPT_PREPEND, cfg.filter_forward_ipv6, "-i", routing_cfg.iface_eap_name, "-j", chain_forward[i]);
            IP6T_FILTER(IPT_PREPEND, cfg.filter_forward_ipv6, "-o", routing_cfg.iface_eap_name, "-j", chain_forward[i]);
            IP6T_MANGLE(IPT_PREPEND, cfg.mangle_postrouting_ipv6, "-o", routing_cfg.iface_eap_name, "-j", chain_postrt[i]);
            IP6T_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv6, "-i", routing_cfg.iface_eap_name, "-j", chain_prert[i]);
            IP6T_MANGLE(IPT_PREPEND, cfg.filter_output_ipv6, "-o", routing_cfg.iface_eap_name, "-j", chain_output[i]);
        }

        /* prert */
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
        IP6T_MANGLE(IPT_APPEND, chain_prert[i], "-j", "CONNMARK", "--restore-mark", "--mask", buffer);
        snprintf(buffer, sizeof(buffer), "0/%u", FIREWALL_AUTHMASK);
        IP6T_MANGLE(IPT_APPEND, chain_prert[i], "-m", "mark", "--mark", buffer, "-m", "state", "--state", "new", "-j", chain_auth[i]);
        IP6T_MANGLE(IPT_APPEND, chain_prert[i], "-i", cfg.iface, "-m", "mark", "--mark", buffer, "-m", "state", "--state", "new", "-j", chain_garden[i]);
        IP6T_MANGLE(IPT_APPEND, chain_prert[i], "-i", cfg.iface, "-m", "mark", "--mark", buffer, "-j", chain_proxy[i]);
        IP6T_MANGLE(IPT_APPEND, chain_prert[i], "-j", chain_qos[i]);
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
        IP6T_MANGLE(IPT_APPEND, chain_prert[i], "-j", "CONNMARK", "--save-mark", "--mask", buffer);

        if (!strncmp(routing_cfg.ifb_name, "imq", 3)) {
            IP6T_MANGLE(IPT_APPEND, chain_prert[i], "-j", "IMQ", "--todev", routing_cfg.ifb_name + 3);
            IP6T_MANGLE(IPT_APPEND, chain_postrt[i], "-j", "IMQ", "--todev", routing_cfg.ifb2_name + 3);
        }

        /* postrt */
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
        IP6T_MANGLE(IPT_APPEND, chain_postrt[i], "-j", "CONNMARK", "--restore-mark", "--mask", buffer);
        IP6T_MANGLE(IPT_APPEND, chain_postrt[i], "-j", chain_qos[i]);
        IP6T_MANGLE(IPT_APPEND, chain_postrt[i], "-j", "CONNMARK", "--save-mark", "--mask", buffer);

        /* qos */
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_CLS_BULK, FIREWALL_CLASSMASK);
        IP6T_MANGLE(IPT_APPEND, chain_qos[i], "-m", "mark", "--mark", buffer, "-j", "RETURN");

        snprintf(buffer, sizeof(buffer), "%u", ~((unsigned)FIREWALL_CLASSMASK));
        IP6T_MANGLE(IPT_APPEND, chain_qos[i], "-j", "MARK", "--and-mark", buffer);
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_CLS_SMALL);
        IP6T_MANGLE(IPT_APPEND, chain_qos[i], "-m", "connbytes", "--connbytes", "0:131071", "--connbytes-dir", "both", "--connbytes-mode", "bytes", "-j", "MARK", "--or-mark", buffer);
        snprintf(buffer, sizeof(buffer), "%u", FIREWALL_CLS_INTER);
        IP6T_MANGLE(IPT_APPEND, chain_qos[i], "-p", "udp", "-m", "length", "--length", "0:1023", "-j", "MARK", "--or-mark", buffer);
        IP6T_MANGLE(IPT_APPEND, chain_qos[i], "-m", "connbytes", "--connbytes", "0:511", "--connbytes-dir", "both", "--connbytes-mode", "avgpkt", "-j", "MARK", "--or-mark", buffer);

        snprintf(buffer, sizeof(buffer), "%u/%u", 0, FIREWALL_CLASSMASK);
        snprintf(buffer2, sizeof(buffer2), "%u", FIREWALL_CLS_BULK);
        IP6T_MANGLE(IPT_APPEND, chain_qos[i], "-m", "mark", "--mark", buffer, "-j", "MARK", "--or-mark", buffer2);

        /* reject chain */
        IP6T_FILTER(IPT_APPEND, chain_reject[i], "-p", "tcp", "-j", "REJECT", "--reject-with", "tcp-reset");
        IP6T_FILTER(IPT_APPEND, chain_reject[i], "-j", "REJECT");
        IP6T_FILTER(IPT_APPEND, chain_reject[i], "-j", "DROP");

        /* output chain */
        IP6T_FILTER(IPT_APPEND, chain_output[i], "-j", "ACCEPT");

        /* output / input default policy is reject */
        IP6T_FILTER(IPT_APPEND, chain_input[i], "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
        IP6T_FILTER(IPT_APPEND, chain_input[i], "-m", "state", "--state", "INVALID", "-j", chain_reject[i]);

        /* detect connmark killbit */
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_KILL, FIREWALL_AUTHMASK);
        IP6T_FILTER(IPT_APPEND, chain_input[i], "-m", "mark", "--mark", buffer, "-j", chain_reject[i]);

        /* prevent hammering */
        if (cfg.input_hammer_limit) {
            snprintf(buffer, sizeof(buffer), "%u", cfg.input_hammer_limit);
            IP6T_FILTER(IPT_APPEND, chain_input[i], "-m", "recent", "--name", chain_input[i], "--update", "--seconds", "1", "--hitcount", buffer, "-j", chain_reject[i]);
            IP6T_FILTER(IPT_APPEND, chain_input[i], "-m", "recent", "--name", chain_input[i], "--set");
        }

        /* allow proxied traffic, allowed services, reject remaining */
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_TPROXY, FIREWALL_AUTHMASK);
        IP6T_FILTER(IPT_APPEND, chain_input[i], "-m", "mark", "--mark", buffer, "-j", "ACCEPT");
        IP6T_FILTER(IPT_APPEND, chain_input[i], "-j", chain_service[i]);
        IP6T_FILTER(IPT_APPEND, chain_input[i], "-j", chain_reject[i]);

        /* allow icmpv6 */
        IP6T_FILTER(IPT_APPEND, chain_service[i], "-p", "icmpv6", "--icmpv6-type", "router-solicitation", "-j", "ACCEPT");
        IP6T_FILTER(IPT_APPEND, chain_service[i], "-p", "icmpv6", "--icmpv6-type", "neighbour-solicitation", "-j", "ACCEPT");
        IP6T_FILTER(IPT_APPEND, chain_service[i], "-p", "icmpv6", "--icmpv6-type", "neighbour-advertisement", "-j", "ACCEPT");

        /* detect connmark killbit */
        snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_KILL, FIREWALL_AUTHMASK);
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-m", "mark", "--mark", buffer, "-j", chain_reject[i]);

        /* default forwarding */
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-m", "state", "--state", "INVALID", "-j", chain_reject[i]);

        /* drop old TCP connections from being reestablished */
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-p", "tcp", "!", "--syn", "-m", "state", "--state", "NEW", "-j", chain_reject[i]);
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-o", cfg.iface, "-j", chain_reject[i]);

        if (routing_cfg.iface_eap_index &&
                routing_cfg.iface_eap_index != routing_cfg.iface_index)
            IP6T_FILTER(IPT_APPEND, chain_forward[i], "-o", routing_cfg.iface_eap_name, "-j", chain_reject[i]);

        /* disallow private prefixes */
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-d", "fc00::/7", "-j", chain_reject[i]);
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-j", chain_blocked[i]);

        /* treat as not authed by default */
        snprintf(buffer, sizeof(buffer), "0/%u", FIREWALL_AUTHMASK);
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-m", "mark", "!", "--mark", buffer, "-j", "ACCEPT");
        IP6T_FILTER(IPT_APPEND, chain_forward[i], "-j", "DROP");
    }
#else
	ip6t_filter(IPT_NEWCHAIN, chain_input);
	ip6t_filter(IPT_NEWCHAIN, chain_output);
	ip6t_filter(IPT_NEWCHAIN, chain_service);
	ip6t_filter(IPT_NEWCHAIN, chain_forward);
	ip6t_filter(IPT_NEWCHAIN, chain_blocked);
	ip6t_filter(IPT_NEWCHAIN, chain_reject);
	ip6t_mangle(IPT_NEWCHAIN, chain_garden);
	ip6t_mangle(IPT_NEWCHAIN, chain_proxy);
	ip6t_mangle(IPT_NEWCHAIN, chain_prert);
	ip6t_mangle(IPT_NEWCHAIN, chain_postrt);
	ip6t_mangle(IPT_NEWCHAIN, chain_qos);
	ip6t_mangle(IPT_NEWCHAIN, chain_auth);
	ip6t_mangle(IPT_NEWCHAIN, chain_output);

	/* hook external chains */
	IP6T_FILTER(IPT_PREPEND, cfg.filter_input_ipv6, "-i", cfg.iface, "-j", chain_input);
	IP6T_FILTER(IPT_PREPEND, cfg.filter_output_ipv6, "-o", cfg.iface, "-j", chain_output);
	IP6T_FILTER(IPT_PREPEND, cfg.filter_forward_ipv6, "-i", cfg.iface, "-j", chain_forward);
	IP6T_FILTER(IPT_PREPEND, cfg.filter_forward_ipv6, "-o", cfg.iface, "-j", chain_forward);
	IP6T_MANGLE(IPT_PREPEND, cfg.mangle_postrouting_ipv6, "-o", cfg.iface, "-j", chain_postrt);
	IP6T_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv6, "-i", cfg.iface, "-j", chain_prert);
	IP6T_MANGLE(IPT_PREPEND, cfg.filter_output_ipv6, "-o", cfg.iface, "-j", chain_output);

	/* hooks for eap interface */
	if (routing_cfg.iface_eap_index &&
			routing_cfg.iface_eap_index != routing_cfg.iface_index) {
		IP6T_FILTER(IPT_PREPEND, cfg.filter_input_ipv6, "-i", routing_cfg.iface_eap_name, "-j", chain_input);
		IP6T_FILTER(IPT_PREPEND, cfg.filter_output_ipv6, "-o", routing_cfg.iface_eap_name, "-j", chain_output);
		IP6T_FILTER(IPT_PREPEND, cfg.filter_forward_ipv6, "-i", routing_cfg.iface_eap_name, "-j", chain_forward);
		IP6T_FILTER(IPT_PREPEND, cfg.filter_forward_ipv6, "-o", routing_cfg.iface_eap_name, "-j", chain_forward);
		IP6T_MANGLE(IPT_PREPEND, cfg.mangle_postrouting_ipv6, "-o", routing_cfg.iface_eap_name, "-j", chain_postrt);
		IP6T_MANGLE(IPT_PREPEND, cfg.mangle_prerouting_ipv6, "-i", routing_cfg.iface_eap_name, "-j", chain_prert);
		IP6T_MANGLE(IPT_PREPEND, cfg.filter_output_ipv6, "-o", routing_cfg.iface_eap_name, "-j", chain_output);
	}

	/* prert */
	snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
	IP6T_MANGLE(IPT_APPEND, chain_prert, "-j", "CONNMARK", "--restore-mark", "--mask", buffer);
	snprintf(buffer, sizeof(buffer), "0/%u", FIREWALL_AUTHMASK);
	IP6T_MANGLE(IPT_APPEND, chain_prert, "-m", "mark", "--mark", buffer, "-m", "state", "--state", "new", "-j", chain_auth);
	IP6T_MANGLE(IPT_APPEND, chain_prert, "-i", cfg.iface, "-m", "mark", "--mark", buffer, "-m", "state", "--state", "new", "-j", chain_garden);
	IP6T_MANGLE(IPT_APPEND, chain_prert, "-i", cfg.iface, "-m", "mark", "--mark", buffer, "-j", chain_proxy);
	IP6T_MANGLE(IPT_APPEND, chain_prert, "-j", chain_qos);
	snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
	IP6T_MANGLE(IPT_APPEND, chain_prert, "-j", "CONNMARK", "--save-mark", "--mask", buffer);

	if (!strncmp(routing_cfg.ifb_name, "imq", 3)) {
		IP6T_MANGLE(IPT_APPEND, chain_prert, "-j", "IMQ", "--todev", routing_cfg.ifb_name + 3);
		IP6T_MANGLE(IPT_APPEND, chain_postrt, "-j", "IMQ", "--todev", routing_cfg.ifb2_name + 3);
	}

	/* postrt */
	snprintf(buffer, sizeof(buffer), "%u", FIREWALL_AUTHMASK | FIREWALL_CLASSMASK);
	IP6T_MANGLE(IPT_APPEND, chain_postrt, "-j", "CONNMARK", "--restore-mark", "--mask", buffer);
	IP6T_MANGLE(IPT_APPEND, chain_postrt, "-j", chain_qos);
	IP6T_MANGLE(IPT_APPEND, chain_postrt, "-j", "CONNMARK", "--save-mark", "--mask", buffer);

	/* qos */
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_CLS_BULK, FIREWALL_CLASSMASK);
	IP6T_MANGLE(IPT_APPEND, chain_qos, "-m", "mark", "--mark", buffer, "-j", "RETURN");

	snprintf(buffer, sizeof(buffer), "%u", ~((unsigned)FIREWALL_CLASSMASK));
	IP6T_MANGLE(IPT_APPEND, chain_qos, "-j", "MARK", "--and-mark", buffer);
	snprintf(buffer, sizeof(buffer), "%u", FIREWALL_CLS_SMALL);
	IP6T_MANGLE(IPT_APPEND, chain_qos, "-m", "connbytes", "--connbytes", "0:131071", "--connbytes-dir", "both", "--connbytes-mode", "bytes", "-j", "MARK", "--or-mark", buffer);
	snprintf(buffer, sizeof(buffer), "%u", FIREWALL_CLS_INTER);
	IP6T_MANGLE(IPT_APPEND, chain_qos, "-p", "udp", "-m", "length", "--length", "0:1023", "-j", "MARK", "--or-mark", buffer);
	IP6T_MANGLE(IPT_APPEND, chain_qos, "-m", "connbytes", "--connbytes", "0:511", "--connbytes-dir", "both", "--connbytes-mode", "avgpkt", "-j", "MARK", "--or-mark", buffer);

	snprintf(buffer, sizeof(buffer), "%u/%u", 0, FIREWALL_CLASSMASK);
	snprintf(buffer2, sizeof(buffer2), "%u", FIREWALL_CLS_BULK);
	IP6T_MANGLE(IPT_APPEND, chain_qos, "-m", "mark", "--mark", buffer, "-j", "MARK", "--or-mark", buffer2);

	/* reject chain */
	IP6T_FILTER(IPT_APPEND, chain_reject, "-p", "tcp", "-j", "REJECT", "--reject-with", "tcp-reset");
	IP6T_FILTER(IPT_APPEND, chain_reject, "-j", "REJECT");
	IP6T_FILTER(IPT_APPEND, chain_reject, "-j", "DROP");

	/* output chain */
	IP6T_FILTER(IPT_APPEND, chain_output, "-j", "ACCEPT");

	/* output / input default policy is reject */
	IP6T_FILTER(IPT_APPEND, chain_input, "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
	IP6T_FILTER(IPT_APPEND, chain_input, "-m", "state", "--state", "INVALID", "-j", chain_reject);

	/* detect connmark killbit */
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_KILL, FIREWALL_AUTHMASK);
	IP6T_FILTER(IPT_APPEND, chain_input, "-m", "mark", "--mark", buffer, "-j", chain_reject);

	/* prevent hammering */
	if (cfg.input_hammer_limit) {
		snprintf(buffer, sizeof(buffer), "%u", cfg.input_hammer_limit);
		IP6T_FILTER(IPT_APPEND, chain_input, "-m", "recent", "--name", chain_input, "--update", "--seconds", "1", "--hitcount", buffer, "-j", chain_reject);
		IP6T_FILTER(IPT_APPEND, chain_input, "-m", "recent", "--name", chain_input, "--set");
	}

	/* allow proxied traffic, allowed services, reject remaining */
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_TPROXY, FIREWALL_AUTHMASK);
	IP6T_FILTER(IPT_APPEND, chain_input, "-m", "mark", "--mark", buffer, "-j", "ACCEPT");
	IP6T_FILTER(IPT_APPEND, chain_input, "-j", chain_service);
	IP6T_FILTER(IPT_APPEND, chain_input, "-j", chain_reject);

	/* allow icmpv6 */
	IP6T_FILTER(IPT_APPEND, chain_service, "-p", "icmpv6", "--icmpv6-type", "router-solicitation", "-j", "ACCEPT");
	IP6T_FILTER(IPT_APPEND, chain_service, "-p", "icmpv6", "--icmpv6-type", "neighbour-solicitation", "-j", "ACCEPT");
	IP6T_FILTER(IPT_APPEND, chain_service, "-p", "icmpv6", "--icmpv6-type", "neighbour-advertisement", "-j", "ACCEPT");

	/* detect connmark killbit */
	snprintf(buffer, sizeof(buffer), "%u/%u", FIREWALL_KILL, FIREWALL_AUTHMASK);
	IP6T_FILTER(IPT_APPEND, chain_forward, "-m", "mark", "--mark", buffer, "-j", chain_reject);

	/* default forwarding */
	IP6T_FILTER(IPT_APPEND, chain_forward, "-m", "state", "--state", "RELATED,ESTABLISHED", "-j", "ACCEPT");
	IP6T_FILTER(IPT_APPEND, chain_forward, "-m", "state", "--state", "INVALID", "-j", chain_reject);

	/* drop old TCP connections from being reestablished */
	IP6T_FILTER(IPT_APPEND, chain_forward, "-p", "tcp", "!", "--syn", "-m", "state", "--state", "NEW", "-j", chain_reject);
	IP6T_FILTER(IPT_APPEND, chain_forward, "-o", cfg.iface, "-j", chain_reject);

	if (routing_cfg.iface_eap_index &&
			routing_cfg.iface_eap_index != routing_cfg.iface_index)
		IP6T_FILTER(IPT_APPEND, chain_forward, "-o", routing_cfg.iface_eap_name, "-j", chain_reject);

	/* disallow private prefixes */
	IP6T_FILTER(IPT_APPEND, chain_forward, "-d", "fc00::/7", "-j", chain_reject);
	IP6T_FILTER(IPT_APPEND, chain_forward, "-j", chain_blocked);

	/* treat as not authed by default */
	snprintf(buffer, sizeof(buffer), "0/%u", FIREWALL_AUTHMASK);
	IP6T_FILTER(IPT_APPEND, chain_forward, "-m", "mark", "!", "--mark", buffer, "-j", "ACCEPT");
	IP6T_FILTER(IPT_APPEND, chain_forward, "-j", "DROP");
#endif

	return 0;
}
#endif

MODULE_REGISTER(firewall, 220)
RESIDENT_REGISTER(firewall_resident, 120)

#ifdef HOTSPOTD_RPC
#include "ext_rpc/rpc.h"
#include "rpc/011-firewall.h"

// Blacklist a client
static int firewall_rpc_block(struct rpc_handle *hndl, struct frmsg *frr) {
	struct frattr *attrs[_FRA_FW_SIZE];
	frm_parse(frr, attrs, _FRA_FW_SIZE);

	if (fra_length(attrs[FRA_FW_HWADDR]) <= 6)
		return -EINVAL;

	const uint8_t *hwaddr = fra_data(attrs[FRA_FW_HWADDR]);
	char markbuf[32], macbuf[24];
#ifdef __SC_BUILD__
    int i = 0;
#endif

	snprintf(markbuf, sizeof(markbuf), "0x%x", FIREWALL_KILL);
	snprintf(macbuf, sizeof(macbuf), "%02x:%02x:%02x:%02x:%02x:%02x",
		hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);

	int action = (frm_type(frr) == FRT_FW_NEWBLOCK) ? IPT_APPEND : IPT_DELETE;
#ifdef __SC_BUILD__
    for(i = 0; i < WIFI_IF_NUM; i++)
    {
        if (fw_ipv4)
            IPT_MANGLE(action, chain_auth[i], "-m", "mac", "--mac-source", macbuf, "-j", "MARK", "--or-mark", markbuf);
        if (fw_ipv6)
            IP6T_MANGLE(action, chain_auth[i], "-m", "mac", "--mac-source", macbuf, "-j", "MARK", "--or-mark", markbuf);
    }
#else
	if (fw_ipv4)
		IPT_MANGLE(action, chain_auth, "-m", "mac", "--mac-source", macbuf, "-j", "MARK", "--or-mark", markbuf);
	if (fw_ipv6)
		IP6T_MANGLE(action, chain_auth, "-m", "mac", "--mac-source", macbuf, "-j", "MARK", "--or-mark", markbuf);
#endif

	return 0;
}

static struct rpc_handler hndl[] = {
		{FRT_FW_NEWBLOCK, firewall_rpc_block},
		{FRT_FW_DELBLOCK, firewall_rpc_block},
		{0}
};

RPC_REGISTER(hndl)
#endif
