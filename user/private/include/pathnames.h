#ifndef __PATHNAMES_H__
#define __PATHNAMES_H_

#define CMD_IFCFG       "/sbin/ifconfig"
#define CMD_TC          "/usr/sbin/tc"
#define CMD_RC          "/usr/sbin/rc"
#define CMD_IP6TABLES   "/usr/sbin/ip6tables"
#define CMD_IPTABLES    "/usr/sbin/iptables"
#define CMD_EBTABLES    "/usr/sbin/ebtables"
#define CMD_IP          "/usr/sbin/ip"
#ifdef CONFIG_SUPPORT_OPENVSWITCH
#define CMD_OVSCTL      "/usr/bin/ovs-vsctl"
#define CMD_OFCTL       "/usr/bin/ovs-ofctl"
#else
#define CMD_BRCTL       "/usr/sbin/brctl"
#endif
#define CMD_ASENSE      "/usr/sbin/asense"
#define CMD_INSMOD      "/sbin/insmod"
#define CMD_KILLALL     "/usr/bin/killall"

#ifdef CONFIG_BRCM_SUPPORT
#define CMD_VLANCTL     "/bin/vlanctl"
#define CMD_XTMCTL     "/bin/xtmctl"
#endif
#endif
