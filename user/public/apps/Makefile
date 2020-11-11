SUBDIRS =
#SUBDIRS += bind
#SUBDIRS += quick_routing
SUBDIRS += chrony
ifeq ($(CONFIG_SUPPORT_IPERF), 1)
SUBDIRS += iperf
endif
ifeq ($(CONFIG_SUPPORT_OPENVPN_CLIENT), 1)
SUBDIRS += openvpn
endif
ifeq ($(CONFIG_SUPPORT_PACKAGE), 1)
SUBDIRS += lxc
SUBDIRS += procd
#SUBDIRS += rpcd
SUBDIRS += opkg
SUBDIRS += start-stop-daemon
endif

ifeq ($(CONFIG_SUPPORT_PPPOE_RELAY), 1)
SUBDIRS += rp-pppoe
endif
ifeq ($(SC_USB_STORAGE), 1)
	SUBDIRS += vsftpd
	SUBDIRS += usb-utility
	SUBDIRS += diskdev_cmds
	SUBDIRS += lsof
	SUBDIRS += mtool
	SUBDIRS += samba
	SUBDIRS += ntfs-3g
ifeq ($(CONFIG_SUPPORT_MEDIA_SERVER), 1)
	SUBDIRS += mediaserver
endif
endif
ifneq ($(CONFIG_SUPPORT_OPENVSWITCH), 1)
SUBDIRS += bridge-utils
else
SUBDIRS += openvswitch
endif
ifeq ($(CONFIG_SUPPORT_IPV6), 1)
SUBDIRS += ipv6wd
endif
SUBDIRS += udhcpd
SUBDIRS += busybox
SUBDIRS += cron

ifeq ($(CONFIG_SUPPORT_ALLJOYN), 1)
SUBDIRS += alljoyn
endif
ifeq ($(SC_SNMP),1)
	SUBDIRS += snmp
endif

ifeq ($(SC_BFTPD),1)
	SUBDIRS += bftpd
endif

ifeq ($(SC_SSHD),1)
	SUBDIRS += openssh
else
ifeq ($(SC_DROPBEAR),1)
	SUBDIRS += dropbear-0.51
endif
endif

SUBDIRS += ppp
SUBDIRS += iptables
ifeq ($(CONFIG_SUPPORT_IPV6), 1)
SUBDIRS += dhcpv6
SUBDIRS += radvd
ifeq ($(findstring $(CONFIG_USE_DNSMASQ)$(CONFIG_USE_DNSMASQ_IPV6_ONLY), 1),)
SUBDIRS += dnsmasq
endif
endif

ifeq ($(SC_ROUTER),1)
#	SUBDIRS += dhcp-forwarder-0.4
ifneq ($(CONFIG_USE_DNSMASQ), 1)
	SUBDIRS += dnrd
endif
	SUBDIRS += ez-ipupdate
#	SUBDIRS += alg
endif
	SUBDIRS += sc_br_nf
ifeq ($(CONFIG_SUPPORT_UBUS),1)
	SUBDIRS += event_forwarder
endif
	SUBDIRS += ebtables

ifeq ($(SC_MULTIWAN),1)
#	SUBDIRS += iproute-2.6.20-070313
endif

ifeq ($(SC_USB_PRINTER),1)
	SUBDIRS += cups
	SUBDIRS += inetd
endif

SUBDIRS += e2fsprogs
#SUBDIRS += mtd-utils
SUBDIRS += remark
SUBDIRS += nulldev 
#SUBDIRS += nf_classify
#SUBDIRS += hw_nat
SUBDIRS += spi
SUBDIRS += SpeedTestC-master
ifeq ($(SC_USB_STORAGE), 1)
SUBDIRS += util-linux
endif
SUBDIRS += miniupnpd
ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
SUBDIRS += netcat
endif
ifeq ($(CONFIG_SUPPORT_FON), 1)
SUBDIRS += fon
endif
ifeq ($(CONFIG_SUPPORT_WEB_PRIVOXY), 1)
SUBDIRS += privoxy
endif
ifeq ($(CONFIG_SUPPORT_L2TP_CLIENT), 1)
SUBDIRS += l2tp
endif
ifeq ($(CONFIG_SUPPORT_IPSEC_CLIENT), 1)
SUBDIRS += openswan
endif
ifeq ($(CONFIG_SUPPORT_L2TP_SERVER), 1)
SUBDIRS += xl2tp
endif
ifeq ($(CONFIG_SUPPORT_PPTP_CLIENT), 1)
SUBDIRS += pptp
endif
ifeq ($(CONFIG_SUPPORT_PPTP_SERVER), 1)
SUBDIRS += pptpd
endif
SUBDIRS += curl
ifeq ($(CONFIG_SUPPORT_HA), 1)
#SUBDIRS += ethtool
endif

all:
	make applications

applications::

	@for i in ${SUBDIRS} ;  do                                                    \
            echo "-----------------------------------------------------------------"; \
            echo -e "\033[;34m                         $$i                 \033[;0m"; \
            echo "-----------------------------------------------------------------"; \
            if test "$$i" = "tcp_wrappers_7.6"; then       \
                make -C $$i linux || exit 1;                \
            else                                            \
            make -C $$i || exit 1 ;                     \
            make -C $$i install || exit 1 ;                                      \
            fi;                                           \
	done

install:

clean:
	@for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean || exit 1; done

