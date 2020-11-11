#ifndef _RCL_DNSV6_H
#define _RCL_DNSV6_H

#define DNS_BIN_FILE               "/usr/sbin/dnsmasq"
#define DNS_BIN_NAME               "dnsmasq"
#define DNS_PID_FILE               "/var/run/dnsmasq.pid"
#define DNS_RESOLVE_CONFIG         "/var/dnsmasq.resolv.conf"
#define DNS_RESOLVE_CONFIG_BAK     "/var/dnsmasq.resolv.conf.bak"
#define DNS_CFG_LOCK               "/var/lock/dns_cfg.lock"

int rcl_wan_dnsmasq_dump_cache();
int rcl_wan_dnsmasq_flush_cache();
int rcl_wan_dnsmasq_cfg_reload(int wan_id);
int rcl_wan_dnsmasq_start(void);
int rcl_wan_dnsmasq_stop(void);
int rcl_wan_dnsmasq_cfg_clean(void);
#ifdef CONFIG_SUPPORT_SECURE_NET
int update_edns0_interface_info(void);
#endif

#endif
