#ifndef _SC_DNS_H_
#define _SC_DNS_H_

#include <stdio.h>
#include <stdint.h>

#ifdef CONFIG_USE_DNSMASQ
#define DNS_BIN_FILE                              "/usr/sbin/dnsmasq"
#define DNS_BIN_NAME                              "dnsmasq"
/*must be same with dnsmasq*/
#define DNSMASQ_CFG_DIR                           "/tmp/dnsmasq.d"
#define DNS_SERVERS_DIR                           "/tmp/dnsmasq_servers.d"
#define DNS_LOCAL_HOSTS_DIR                       "/tmp/dnsmasq_hosts.d"
#define DNS_PID_FILE                              "/var/run/dnsmasq.pid"
#define DNS_MORE_CFG                              "/etc/dnsmasq.more.conf"
#define DNS_CFG                                   "/etc/dnsmasq.conf"
#endif
/* Define the domain type */
#define DOMAIN_TYPE_UNKNOWN  (0x0000)
#define DOMAIN_TYPE_DATA     (0x0001)
#define DOMAIN_TYPE_TR069    (0x0002)
#define DOMAIN_TYPE_NTP      (0x0003)
#define DOMAIN_TYPE_VOIP     (0x0004)
#define DOMAIN_TYPE_IPTV     (0x0005)
#define DOMAIN_TYPE_CLI      (0x0006)
#define DOMAIN_TYPE_IPPHONE  (0x0007)
#ifdef CONFIG_USE_DNSMASQ
#define DOMAIN_TYPE_DATA_MAN (0x0008)
#define DOMAIN_TYPE_IPTV_STR  "iptv"
#endif
#define DOMAIN_TYPE_CLI_STR  "cli"
/*
 * Description:
 *  Add domain to DNRD domain list
 * Parameter:
 *  dytpe    - Domain type
 *  dns_ip   - DNS server IP[Format(192.168.0.1)]
 *  ip_pri   - Priority of DNS server IP[0(highest) ~ 4]
 *  inf_name - WAN interface name
 *  inf_pri  - Priority of WAN infterface[0(highest) ~ 8]
 *  domain   - Domain name (Can be NULL)
 *  time_out - Time out of this DNS server
 * */
#ifdef CONFIG_USE_DNSMASQ
int dns_add_domain_by_dns_ip(char *dns,char *src,char *ifName,char *domain, int dtype, int ip_priv, int inf_pri, int time_out);
#else
int dns_add_domain_by_dnsip(int dtype, char *dns_ip, int ip_priv, 
                            char *inf_name, int inf_pri, char * domain,
                            int time_out);

int dns_add_domain_by_dnsip_ex(int dtype, char *dns_ip, int ip_priv, 
                            int wan_id, char* inf_name, int inf_pri, char * domain,
                            int time_out);
#endif
/*
 * Description:
 *  Add domain to DNRD domain list
 * Parameter:
 *  dytpe    - Domain type
 *  wan_id   - WAN interface ID
 *  inf_pri  - Priority of WAN infterface of this wan ID [0(highest) ~ 8]
 *  domain   - Domain name (Can be NULL)
 *  time_out - Time out of this DNS server
 * */
#ifdef CONFIG_USE_DNSMASQ
int dns_add_domain_by_wan_id(int wan_id, char *domain, int dtype, int inf_pri, int time_out);
#else
int dns_add_domain_by_wanid(int dtype, int wan_id, int inf_pri, char * domain,
                            int time_out);
#endif


/*
 * Description:
 *  Delete domain from domain list by WAN ID
 * Parameter:
 *  wan_id   - WAN interface ID
 * */
#ifdef CONFIG_USE_DNSMASQ
int dns_del_domain_by_wan_id(int dtype,char *ifName);
int dns_del_domain_by_wan_id_ex(char *ifName);
#else
int dns_del_domain_by_wanid(int wan_id);
#endif

/*
 * Description:
 *  Delete domain from domain list by doman name
 * Parameter:
 *  dtype    - Domain type
 *  wan_id   - WAN interface ID
 *  domain   - Domain name
 * */
#ifdef CONFIG_USE_DNSMASQ
int dns_del_domain_by_domain(int dtype, char *domain, int wan_id);
#else
int dns_del_domain_by_domain(int dtype, int wan_id, char *domain);
#endif

/*
 * Description:
 *  Delete domain from domain list by doman type
 * Parameter:
 *  dtype    - Domain type
 * */
#ifdef CONFIG_USE_DNSMASQ
int dns_del_domain_by_domain_type(int dtype);
int dns_del_dnsv6_by_ifname(char *ifname);
#else
int dns_del_domain_by_dtype(int dtype);
#endif

/*
 * Description:
 *  Add domain type in the hostname
 * Parameter:
 *  name    - Domain name
 *  dtype   - Domain type
 * */
struct hostent *sc_gethostbyname(const char *name, int dtype);

int sc_gethostbyname_r(const char *name, int dtype, struct hostent *ret, 
                     char *buf, size_t buflen, struct hostent **result, 
                     int *h_errnop);
#ifndef CONFIG_USE_DNSMASQ
int dns_add_nameip(char * name,char * ip, int time_out);
int dns_del_nameip(void);
int dns_del_nameip_x(void);
int dns_del_namesrv(void);
int dns_add_namesrv(char * name,char * srv, int time_out);
#ifdef CONFIG_SUPPORT_WEB_PRIVOXY
int dns_add_provisioncode(char * provisioncode);
int dns_add_subnet(char * subnet);
#endif
#ifdef CONFIG_SUPPORT_FON
int dns_add_filter_ip(char * ip);
int dns_del_filter_ip(char * ip);
int dns_reset_filter_ip(void);
#endif
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
int dns_del_filter_domain(void);
int dns_add_filter_domain(char * domain,char* mac_addr, int time_out);
#endif
#ifdef CONFIG_SUPPORT_SECURE_NET
int dns_add_interface_info(char *if_info);
#endif
int dns_add_localhost_nameip(char * name,char* ip, int time_out);
int dns_add_nameip_x(char * name,char * ip, int time_out);
#ifdef CONFIG_SUPPORT_REDIRECT
int dns_enable_redirect(char * ip);
int dns_disable_redirect(void);
#endif
#ifdef CONFIG_SUPPORT_FON
int dns_add_whitelist(char * name);
int dns_del_whitelist(void);
int dns_enable_fon_redirect(char * ip);
#endif
#else
int dns_add_local_nameip(char *name, char *ip, int dtype);
int dns_del_local_nameip(int dtype);
#endif
int dns_set_dhcp_option15(char *opt_str);


#endif /*End Of _SC_DNS_H_*/
