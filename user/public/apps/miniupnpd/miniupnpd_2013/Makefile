# $Id: Makefile.linux,v 1.78 2013/05/03 09:30:33 nanard Exp $
# MiniUPnP project
# (c) 2006-2013 Thomas Bernard
# http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
# Author : Thomas Bernard
# for use with GNU Make
#
# options can be passed to genconfig.sh through CONFIG_OPTIONS :
# $ CONFIG_OPTIONS="--ipv6 --igd2" make -f Makefile.linux
#
# To install use :
# $ DESTDIR=/dummyinstalldir make -f Makefile.linux install
# or :
# $ INSTALLPREFIX=/usr/local make -f Makefile.linux install
# or :
# $ make -f Makefile.linux install
# (default INSTALLPREFIX is /usr)
#
# if your system hasn't iptables libiptc headers and binary correctly
# installed, you need to get iptables sources from http://netfilter.org/
# ./configure them and build them then miniupnpd will build using :
# $ IPTABLESPATH=/path/to/iptables-1.4.1 make -f Makefile.linux
#
#CFLAGS = -O -g -DDEBUG
#add by sercomm 2013-9
IPTABLESPATH = $(PUBLIC_APPSPATH)/apps/iptables/iptables-1.4.12
#end
ifeq ($(CONFIG_SUPPORT_BUILD_DEBUG), 1)
CFLAGS ?= -g
else
CFLAGS ?= -Os
endif
CFLAGS += -fno-strict-aliasing
CFLAGS += -fno-common
CFLAGS += -D_GNU_SOURCE
CFLAGS += -Wall
CFLAGS += -Wextra -Wstrict-prototypes -Wdeclaration-after-statement
#add by sercomm 2013-9
CFLAGS += -D_GNU_SOURCE -I$(PRIVATE_APPSPATH)/include/ -I$(PRIVATE_APPSPATH)/include/cli 
ifneq ($(BRCM_502), 1)
CFLAGS += -I$(KERNELDIR)/include
else
CFLAGS += -idirafter$(KERNELDIR)/include
endif
CFLAGS += -I$(PRIVATE_APPSPATH)/include/rcl -I$(PRIVATE_APPSPATH)/include/cal -I$(PRIVATE_APPSPATH)/include/sal/
CFLAGS += -I$(PUBLIC_APPSPATH)/apps/iptables/iptables-1.4.12/include/
CFLAGS += -I$(PUBLIC_APPSPATH)/include/
ifeq ($(CONFIG_SUPPORT_PCPD), 1)
ifeq ($(CONFIG_SUPPORT_UBUS),1)
CFLAGS += -I$(PUBLIC_APPSPATH)/libs/ubus/ubus/ubus-2015-07-07/
CFLAGS += -I$(PUBLIC_APPSPATH)/libs/ubus/libubox/libubox-2015-06-29/include
endif
endif

CFLAGS += -D__SC_BUILD__
LDFLAGS =
LDFLAGS += -L$(TARGETDIR)/lib -lslog $(X_PRIVATE_HSAL_LIBS) $(X_PRIVATE_HCAL_LIBS) $(X_PRIVATE_WLAN_LIBS)
ifeq ($(CONFIG_SUPPORT_PCPD), 1)
LDFLAGS += -L$(TARGETDIR)/lib -lubus -lblobmsg_json -ljson-c -lubox
endif
ifeq ($(CONFIG_SUPPORT_UBUS),1)
CFLAGS += -I$(PUBLIC_APPSPATH)/libs/ubus/ubus/ubus/
CFLAGS += -I$(PUBLIC_APPSPATH)/libs/ubus/libubox/libubox/include
LDFLAGS += -L$(TARGETDIR)/lib -lubus -lblobmsg_json -ljson-c -lubox
endif
ifeq ($(CONFIG_SUPPORT_CGN), 1)
LDFLAGS += -lcpm
endif
#end
#CFLAGS += -Wno-missing-field-initializers
#CFLAGS += -ansi	# iptables headers does use typeof which is a gcc extension
CC ?= gcc
RM = rm -f
INSTALL = install
STRIP ?= strip
CP = cp
#add by sercomm 2013-9
PREFIX = $(INSTALLDIR)
INSTALLPREFIX ?= $(PREFIX)/usr
SBININSTALLDIR = $(INSTALLPREFIX)/sbin
ETCINSTALLDIR = $(PREFIX)/etc/igd_upnpd
MANINSTALLDIR = $(INSTALLPREFIX)/share/man/man8
CONFIG_OPTIONS = "--strict" "--igd2"

BASEOBJS = miniupnpd.o upnphttp.o upnpdescgen.o upnpsoap.o \
           upnpreplyparse.o minixml.o \
           upnpredirect.o getifaddr.o daemonize.o upnpglobalvars.o \
           options.o upnppermissions.o minissdp.o natpmp.o \
           upnpevents.o upnputils.o getconnstatus.o \
           upnppinhole.o

LNXOBJS = linux/getifstats.o linux/ifacewatcher.o linux/getroute.o
NETFILTEROBJS = netfilter/iptcrdr.o netfilter/iptpinhole.o
TESTGETROUTEOBJS = testgetroute.o upnputils.o linux/getroute.o
MINIUPNPDCTLOBJS = miniupnpdctl.o
TESTGETIFADDROBJS = testgetifaddr.o getifaddr.o
TESTUPNPPERMISSIONSOBJS = testupnppermissions.o upnppermissions.o
TESTGETIFSTATSOBJS = testgetifstats.o linux/getifstats.o

ALLOBJS = $(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)

PCFILE_FOUND := $(shell pkg-config --exists libiptc; echo $$?)

ifeq (${PCFILE_FOUND},0)

IPTABLESVERSION := $(shell pkg-config --modversion libiptc)
IPTABLESVERSION1 := $(shell echo $(IPTABLESVERSION) | cut -d. -f1 )
IPTABLESVERSION2 := $(shell echo $(IPTABLESVERSION) | cut -d. -f2 )
IPTABLESVERSION3 := $(shell echo $(IPTABLESVERSION) | cut -d. -f3 )
# test if iptables version >= 1.4.3
TEST := $(shell [ \( \( $(IPTABLESVERSION1) -ge 1 \) -a \( $(IPTABLESVERSION2) -ge 4 \) \) -a \( $(IPTABLESVERSION3) -ge 3 \) ] && echo 1 )
ifeq ($(TEST), 1)
CFLAGS += -DIPTABLES_143
endif

CFLAGS  += $(shell pkg-config --cflags libiptc)
LIBS    += $(shell pkg-config --libs-only-l libiptc)
LDFLAGS += $(shell pkg-config --libs-only-L libiptc)
LDFLAGS += $(shell pkg-config --libs-only-other libiptc)

else

ifeq "$(wildcard /etc/gentoo-release )" ""
LIBS ?= -liptc
else # gentoo
# the following is better, at least on gentoo with iptables 1.4.6
# see http://miniupnp.tuxfamily.org/forum/viewtopic.php?p=1618
# and http://miniupnp.tuxfamily.org/forum/viewtopic.php?p=2183
LIBS ?= -lip4tc
CFLAGS := -DIPTABLES_143 $(CFLAGS)
endif

ARCH ?= $(shell uname -m | grep -q "x86_64" && echo 64)
ifdef IPTABLESPATH
CFLAGS := $(CFLAGS) -I$(IPTABLESPATH)/include/
#change LDFLAFGS to LDFLAGS,2013-9
LDFLAGS := $(LDFLAGS) -L$(IPTABLESPATH)/libiptc/
# get iptables version and set IPTABLES_143 macro if needed
ifeq ($(TARGET_OPENWRT),)
IPTABLESVERSION := $(shell grep "\#define VERSION" $(IPTABLESPATH)/config.h | tr -d \" |cut -d" " -f3 )
IPTABLESVERSION1 := $(shell echo $(IPTABLESVERSION) | cut -d. -f1 )
IPTABLESVERSION2 := $(shell echo $(IPTABLESVERSION) | cut -d. -f2 )
IPTABLESVERSION3 := $(shell echo $(IPTABLESVERSION) | cut -d. -f3 )
# test if iptables version >= 1.4.3
TEST := $(shell [ \( \( $(IPTABLESVERSION1) -ge 1 \) -a \( $(IPTABLESVERSION2) -ge 4 \) \) -a \( $(IPTABLESVERSION3) -ge 3 \) ] && echo 1 )
ifeq ($(TEST), 1)
CFLAGS := $(CFLAGS) -DIPTABLES_143
# the following sucks, but works
# modify by sercomm 2013-9
ifeq ($(CONFIG_SUPPORT_FON), 1)
LIBS = $(IPTABLESPATH)/libiptc/.libs/libip4tc.so
else
#LIBS = $(IPTABLESPATH)/libiptc/.libs/libip4tc.o
LIBS = $(IPTABLESPATH)/libiptc/.libs/libip4tc.a
endif
else # ifeq ($(TEST), 1)
ifeq ($(CONFIG_SUPPORT_FON), 1)
LIBS = $(IPTABLESPATH)/libiptc/libiptc.so
else
LIBS = $(IPTABLESPATH)/libiptc/libiptc.a
endif
endif # ifeq ($(TEST), 1)
else # ($(TARGET_OPENWRT),)
# openWRT :
# check for system-wide iptables files. Test if iptables version >= 1.4.3
# the following test has to be verified :
TEST := $(shell test -f /usr/include/iptables/internal.h && grep -q "\#define IPTABLES_VERSION" /usr/include/iptables/internal.h && echo 1)
ifeq ($(TEST), 1)
CFLAGS := $(CFLAGS) -DIPTABLES_143
LIBS = -liptc
endif  # ($(TEST), 1)
TEST_LIB := $(shell test -f /usr/lib$(ARCH)/libiptc.a && echo 1)
ifeq ($(TEST_LIB), 1)
LIBS = -liptc /usr/lib$(ARCH)/libiptc.a
endif # ($(TEST_LIB), 1)
endif # ($(TARGET_OPENWRT),)
else # ifdef IPTABLESPATH
# IPTABLESPATH not defined
# the following test has to be verified :
TEST := $(shell test -f /usr/include/xtables.h && grep -q "XTABLES_VERSION_CODE" /usr/include/xtables.h && echo 1)
ifeq ($(TEST), 1)
CFLAGS := $(CFLAGS) -DIPTABLES_143
LIBS = -liptc
TESTIP4TC := $(shell test -f /lib/libip4tc.so && echo 1)
ifeq ($(TESTIP4TC), 1)
LIBS := $(LIBS) -lip4tc
endif # ($(TESTIP4TC), 1)
TESTIP6TC := $(shell test -f /lib/libip6tc.so && echo 1)
ifeq ($(TESTIP6TC), 1)
LIBS := $(LIBS) -lip6tc
endif # ($(TESTIP6TC), 1)
endif # ($(TEST), 1)
endif # ifdef IPTABLESPATH
endif # ifdef PCFILE_FOUND
# modify by sercomm 2013-9
#LIBS += -lnfnetlink
LIBS += $(PUBLIC_APPSPATH)/libs/libnfnetlink/libnfnetlink.a 

ifeq ($(CONFIG_SUPPORT_UBUS),1)
#LIBS += -lubus -lblobmsg_json -ljson-c -lubox
endif
ifeq ($(CONFIG_SUPPORT_PCPD),1)
LIBS += -lpcpc
endif

TESTUPNPDESCGENOBJS = testupnpdescgen.o upnpdescgen.o

EXECUTABLES = igd_upnpd \
              testupnppermissions miniupnpdctl testgetifaddr \
              testgetroute

.PHONY:	all clean install depend genuuid

all:	clean_config $(EXECUTABLES) 

clean_config:
	$(RM) config.h	

clean:
	$(RM) $(ALLOBJS)
	$(RM) $(EXECUTABLES)
	$(RM) testupnpdescgen.o testgetifstats.o
	$(RM) testupnppermissions.o testgetifaddr.o
	$(RM) testgetroute.o
	$(RM) miniupnpdctl.o

install:	igd_upnpd miniupnpd.8 miniupnpd.conf genuuid \
	netfilter/iptables_init.sh netfilter/iptables_removeall.sh \
	netfilter/ip6tables_init.sh netfilter/ip6tables_removeall.sh \
	linux/miniupnpd.init.d.script
	$(STRIP) igd_upnpd
	$(INSTALL) igd_upnpd $(INSTALLDIR)/usr/sbin 

# genuuid is using the uuidgen CLI tool which is part of libuuid
# from the e2fsprogs
# 'cat /proc/sys/kernel/random/uuid' could be also used
genuuid:
ifeq ($(TARGET_OPENWRT),)
	sed -i -e "s/^uuid=[-0-9a-f]*/uuid=`(genuuid||uuidgen||uuid) 2>/dev/null`/" miniupnpd.conf
else
	sed -i -e "s/^uuid=[-0-9a-f]*/uuid=`($(STAGING_DIR_HOST)/bin/genuuid||$(STAGING_DIR_HOST)/bin/uuidgen||$(STAGING_DIR_HOST)/bin/uuid) 2>/dev/null`/" miniupnpd.conf
endif

igd_upnpd: config.h $(ALLOBJS)
	echo $(CFLAGS) $(LDFLAGS) $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(ALLOBJS) $(LIBS)

testupnpdescgen:	$(TESTUPNPDESCGENOBJS)
	$(CC) $(CFLAGS) -o $@ $(TESTUPNPDESCGENOBJS)

testgetifstats:    $(TESTGETIFSTATSOBJS) 	
	$(CC) $(CFLAGS) -o $@ $(TESTGETIFSTATSOBJS) $(LDFLAGS) $(LIBS)

testupnppermissions:	$(TESTUPNPPERMISSIONSOBJS)
	$(CC) $(CFLAGS) -o $@ $(TESTUPNPPERMISSIONSOBJS)

testgetifaddr:	$(TESTGETIFADDROBJS)
	$(CC) $(CFLAGS) -o $@ $(TESTGETIFADDROBJS)

testgetroute:	$(TESTGETROUTEOBJS)
	$(CC) $(CFLAGS) -o $@ $(TESTGETROUTEOBJS) $(LDFLAGS) $(LIBS)

miniupnpdctl:	$(MINIUPNPDCTLOBJS)
	$(CC) $(CFLAGS) -o $@ $(MINIUPNPDCTLOBJS)

config.h:	genconfig.sh VERSION
	./genconfig.sh $(CONFIG_OPTIONS)

depend:	config.h
	makedepend -f$(MAKEFILE_LIST) -Y \
	$(ALLOBJS:.o=.c) $(TESTUPNPDESCGENOBJS:.o=.c) \
	testgetifstats.c testupnppermissions.c testgetifaddr.c \
	testgetroute.c miniupnpdctl.c 2>/dev/null

# DO NOT DELETE

miniupnpd.o: config.h macros.h upnpglobalvars.h upnppermissions.h
miniupnpd.o: miniupnpdtypes.h upnphttp.h upnpdescgen.h miniupnpdpath.h
miniupnpd.o: getifaddr.h upnpsoap.h options.h minissdp.h upnpredirect.h
miniupnpd.o: upnppinhole.h daemonize.h upnpevents.h natpmp.h commonrdr.h
miniupnpd.o: upnputils.h ifacewatcher.h
upnphttp.o: config.h upnphttp.h upnpdescgen.h miniupnpdpath.h upnpsoap.h
upnphttp.o: upnpevents.h upnputils.h
upnpdescgen.o: config.h getifaddr.h upnpredirect.h upnpdescgen.h
upnpdescgen.o: miniupnpdpath.h upnpglobalvars.h upnppermissions.h
upnpdescgen.o: miniupnpdtypes.h upnpdescstrings.h upnpurns.h getconnstatus.h
upnpsoap.o: macros.h config.h upnpglobalvars.h upnppermissions.h
upnpsoap.o: miniupnpdtypes.h upnphttp.h upnpsoap.h upnpreplyparse.h
upnpsoap.o: upnpredirect.h upnppinhole.h getifaddr.h getifstats.h
upnpsoap.o: getconnstatus.h upnpurns.h
upnpreplyparse.o: upnpreplyparse.h minixml.h
minixml.o: minixml.h
upnpredirect.o: macros.h config.h upnpredirect.h upnpglobalvars.h
upnpredirect.o: upnppermissions.h miniupnpdtypes.h upnpevents.h
upnpredirect.o: netfilter/iptcrdr.h commonrdr.h
getifaddr.o: config.h getifaddr.h
daemonize.o: daemonize.h config.h
upnpglobalvars.o: config.h upnpglobalvars.h upnppermissions.h
upnpglobalvars.o: miniupnpdtypes.h
options.o: config.h options.h upnppermissions.h upnpglobalvars.h
options.o: miniupnpdtypes.h
upnppermissions.o: config.h upnppermissions.h
minissdp.o: config.h upnpdescstrings.h miniupnpdpath.h upnphttp.h
minissdp.o: upnpglobalvars.h upnppermissions.h miniupnpdtypes.h minissdp.h
minissdp.o: upnputils.h getroute.h codelength.h
natpmp.o: macros.h config.h natpmp.h upnpglobalvars.h upnppermissions.h
natpmp.o: miniupnpdtypes.h getifaddr.h upnpredirect.h commonrdr.h upnputils.h
upnpevents.o: config.h upnpevents.h miniupnpdpath.h upnpglobalvars.h
upnpevents.o: upnppermissions.h miniupnpdtypes.h upnpdescgen.h upnputils.h
upnputils.o: config.h upnputils.h upnpglobalvars.h upnppermissions.h
upnputils.o: miniupnpdtypes.h getroute.h
getconnstatus.o: getconnstatus.h getifaddr.h
upnppinhole.o: macros.h config.h upnpredirect.h upnpglobalvars.h
upnppinhole.o: upnppermissions.h miniupnpdtypes.h upnpevents.h
upnppinhole.o: netfilter/iptpinhole.h
linux/getifstats.o: config.h getifstats.h 
linux/ifacewatcher.o: config.h ifacewatcher.h config.h minissdp.h
linux/ifacewatcher.o: miniupnpdtypes.h getifaddr.h upnpglobalvars.h
linux/ifacewatcher.o: upnppermissions.h natpmp.h
linux/getroute.o: getroute.h upnputils.h
netfilter/iptcrdr.o: macros.h config.h netfilter/iptcrdr.h commonrdr.h
netfilter/iptcrdr.o: config.h upnpglobalvars.h upnppermissions.h
netfilter/iptcrdr.o: miniupnpdtypes.h
netfilter/iptpinhole.o: config.h netfilter/iptpinhole.h upnpglobalvars.h
netfilter/iptpinhole.o: upnppermissions.h config.h miniupnpdtypes.h
testupnpdescgen.o: macros.h config.h upnpdescgen.h
upnpdescgen.o: config.h getifaddr.h upnpredirect.h upnpdescgen.h
upnpdescgen.o: miniupnpdpath.h upnpglobalvars.h upnppermissions.h
upnpdescgen.o: miniupnpdtypes.h upnpdescstrings.h upnpurns.h getconnstatus.h
testgetifstats.o: getifstats.h
testupnppermissions.o: upnppermissions.h config.h
testgetifaddr.o: getifaddr.h
testgetroute.o: getroute.h upnputils.h upnpglobalvars.h upnppermissions.h
testgetroute.o: config.h miniupnpdtypes.h
miniupnpdctl.o: macros.h
