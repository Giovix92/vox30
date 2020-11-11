#ifndef _RCL_DHCP6C_H
#define _RCL_DHCP6C_H

#define DHCP6C_NAME "dhcp6c"
#define DHCP6C_CONF "/var/dhcpd/dhcp6c.conf"
#define DHCPV6_DB "/var/db/"
#define DHCPV6_CONF "/var/dhcpd/"

int rcl_wan_dhcp6c_cfg_create(int wan);
int rcl_wan_dhcp6c_cfg_reload(int wan);
int rcl_wan_dhcp6c_start(int wan);
int rcl_wan_dhcp6c_stop(int wan);
int rcl_wan_dhcp6c_renew(int wan);

#endif
