

SUBDIRS =
ifeq ($(CONFIG_SUPPORT_PACKAGE), 1)
SUBDIRS += libncurses
SUBDIRS += libreadline
SUBDIRS += liblua
endif
ifeq ($(CONFIG_SUPPORT_IPERF), 1)
SUBDIRS += libiperf
endif


ifneq ($(CONFIG_SUPPORT_UBUS)_$(CONFIG_SUPPORT_BANDSTEERING), 0_0)
SUBDIRS += ubus
endif

ifeq ($(CONFIG_SUPPORT_FON),1)
SUBDIRS += $(PUBLIC_APPSPATH)/apps/fon/FONAP3_v3.0.0/src/hotspotd_l2tp/libfonrpc
endif
ifeq ($(SC_ROUTER),1)
#    SUBDIRS += libupnp-1.2.1
endif
SUBDIRS += openssl
ifeq ($(CONFIG_SUPPORT_IPV6), 1)
SUBDIRS += libnetlink
endif
SUBDIRS += libnfnetlink
SUBDIRS += libslog
SUBDIRS += libsalx
SUBDIRS += libnv

SUBDIRS += zlib

ifeq ($(CONFIG_SUPPORT_OPENVPN), 1)
SUBDIRS += libpam
SUBDIRS += lzo
endif
ifeq ($(CONFIG_SUPPORT_NGINX), 1)
SUBDIRS += pcre
endif
ifeq ($(ROOTFSTYPE), ubi)
#SUBDIRS += lzo
SUBDIRS += lib-mtd-utils
endif
ifeq ($(SC_USB_STORAGE), 1)
     SUBDIRS += libiconv
     SUBDIRS += fuse
     SUBDIRS += libusb
endif
ifeq ($(CONFIG_SAFE_NETWORK), 1)
SUBDIRS += libnl
else
ifeq ($(SC_CUSTOMER), NEUTRAL)
SUBDIRS += libnl
endif
endif
ifeq ($(CONFIG_SUPPORT_L2TP_CLIENT), 1)
SUBDIRS += libflex
endif
ifeq ($(CONFIG_SUPPORT_IPSEC_CLIENT), 1)
SUBDIRS += libgmp
endif
ifeq ($(CONFIG_SUPPORT_PACKAGE), 1)
SUBDIRS += uci
#SUBDIRS += libgpgme

ifneq ($(CONFIG_SUPPORT_GNULIBS)_$(CONFIG_SUPPORT_PLUME)_$(CONFIG_SUPPORT_MQTT), 0_0_0)
SUBDIRS += libmosquitto
endif
ifeq ($(CONFIG_SUPPORT_GNULIBS), 1)
SUBDIRS += libapparmor
SUBDIRS += pcre
SUBDIRS += libselinux
SUBDIRS += libcap
SUBDIRS += libseccomp
SUBDIRS += libnettle
SUBDIRS += libgpg-error
SUBDIRS += libgcrypt
SUBDIRS += libgnutls
SUBDIRS += python 
SUBDIRS += libmicrohttpd
endif
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

