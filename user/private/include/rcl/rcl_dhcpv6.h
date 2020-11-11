
#ifndef _RCL_DHCPV6_H_
#define _RCL_DHCPV6_H_

#define DHCP_LEASE_TIME_IN_UNCONFIGRED_MODE 60   //seconds
#define DHCPV6_NAME "dhcp6s"
#define DHCP6S_CONF "/var/dhcpd/dhcp6s.conf"
#define DHCP6S_DIR  "/var/dhcpd/"
#define DHCP6S_DB   "/var/db/"

int rcl_lan_dhcp6s_cfg_create(void);
int rcl_lan_dhcp6s_cfg_reload(void);
int rcl_lan_dhcp6s_start(void);
int rcl_lan_dhcp6s_stop(void);

#endif /* _RCL_DHCPV6_H_ */
