#MODULES_BUILTIN=tunnel  radius  fonctl fonapi eapradserv localusers rpc ubus private service
MODULES_BUILTIN=tunnel  radius  fonctl fonapi eapradserv rpc 
MODULES:=$(MODULES_BUILTIN)
#IPV6?=1
#SC_DEBUG=1


# End of configuration
CFLAGS?=-Os -g $(EXPORT_RPC)
SFLAGS:=-std=gnu99 -D_GNU_SOURCE
#WFLAGS:=-Wall -Werror -pedantic
WFLAGS:=-Wall -Werror 
LDFLAGS?=
LDFLAGS+=
DESTDIR?=
DYNCFLAGS:=-I.
DYNLDFLAGS:=
BINARY:=hotspotd
CUR_PATH = $(shell pwd)


ifneq (,$(findstring rpc,$(MODULES)))
  SFLAGS+=-DHOTSPOTD_RPC
endif

ifneq (,$(findstring ubus,$(MODULES)))
  SFLAGS+=-DHOTSPOTD_UBUS -DHOTSPOTD_OPENSSL
endif

#ifneq (0,$(IPV6))
ifeq (1,$(CONFIG_SUPPORT_IPV6))
  SFLAGS+=-DHOTSPOTD_IPV6
endif

ifneq (,$(findstring private,$(MODULES)))
ifdef IPTABLES_PRE_1_4_3
  CFLAGS+=-DIPTABLES_PRE_1_4_3
  WFLAGS=-Wall
  LDFLAGS+=-L$(INSTALLDIR)/lib -liptc
else
  CFLAGS+=-I$(PUBLIC_APPSPATH)/apps/iptables/iptables-1.4.12/include
  DYNLDFLAGS += -L$(INSTALLDIR)/lib -lip4tc
endif
endif

CFLAGS+=-I$(PRIVATE_APPSPATH)/include
CFLAGS+=-I$(PUBLIC_APPSPATH)/include
DYNLDFLAGS+=-lslog
ifeq (1,$(SC_DEBUG))
CFLAGS +=  -DSC_DEBUG
endif

export CFLAGS
export WFLAGS
export SFLAGS

MODULES_OBJECTS=$(addprefix ext_,$(addsuffix /*.o,$(MODULES)))
binary: modules_builtin $(BINARY)

ifeq (shared,$(LIBUNL))
  DYNLDFLAGS += -lunl
else
  DYNCFLAGS += -Ilibunl
  DYNSOURCE += libunl/*.c
endif

ifeq (shared,$(LIBUCI))
  DYNLDFLAGS += -luci
else
  DYNCFLAGS += -Ilibuci
  DYNSOURCE += libuci/libuci.c libuci/file.c libuci/util.c libuci/delta.c
endif

ifeq (shared,$(LIBUBUS))
  DYNLDFLAGS += -lubus -lubox -lssl -lcrypto
else
#  CFLAGS += -I$(STAGING_DIR)/usr/include/cyassl
#  DYNLDFLAGS += -lcyassl
   CFLAGS += -I$(OPENSSL_DIR)/include
   SFLAGS+=-DHOTSPOTD_OPENSSL
ifeq (1,$(BRCM_416))
   DYNLDFLAGS+= -L$(INSTALLDIR)/lib -lcrypto -lsalx -lnv -lwlctl -lwlbcmshared -lwlbcmcrypto
else
   DYNLDFLAGS+= -L$(INSTALLDIR)/lib -lcrypto -lsalx -lnv -lwlctl 
endif
endif
LDFLAGS+=-L$(CUR_PATH)/libfonrpc -lfonrpc 
CFLAGS+=-I$(CUR_PATH)/libfonrpc


#$(BINARY): core/*.c lib/*.c libfonrpc/*.c $(sort $(DYNSOURCE))
$(BINARY): core/hotspotd.c core/client.c core/neigh.c core/loader.c core/trigger.c lib/*.c $(sort $(DYNSOURCE))
	$(CC) $(CFLAGS) $(SFLAGS) $(WFLAGS) $(FEATUREFLAGS) $(DYNCFLAGS) -o $@ $+ $(MODULES_OBJECTS) $(LDFLAGS) $(DYNLDFLAGS)

clean:
	for i in $(MODULES); do make -C ext_$$i clean; done
	rm -rf $(BINARY) *.so *.deb *.loop *.vdi *.ova
#	cd kmod-act_connmark; make clean

modules_builtin:
	rm -f $(BINARY)
	for i in $(MODULES); do make -C ext_$$i static || exit 1; done
#	cd kmod-act_connmark; make 

deb:
	fakeroot dpkg-buildpackage -b -us -uc
	rm ../hotspotd*.changes
	mv ../hotspotd*.deb ../act-connmark*.deb ./

install:
	install -m 755 hotspotd $(INSTALLDIR)/usr/sbin/
	ln -sf hotspotd $(INSTALLDIR)/usr/sbin/fonctl
#	cd kmod-act_connmark; make install

localsetup: deb
	sudo ./localsetup

debimage: $(BINARY).loop
debvm: $(BINARY).vdi $(BINARY).ova
cddebclean:
	debian/rules clean

help:
	@echo hotspotd build system
	@echo Usage: make [target] [options]
	@echo
	@echo Targets:
	@echo '(default)	build hotspotd executable'
	@echo 'deb		also build debian packages'
	@echo 'localsetup	prepare and guide through a test setup'
	@echo 'debimage	build a debian loop diskimage (for qemu)'
	@echo 'debvm		build a debian OVA virtual appliance (for VirtualBox, ...)'
	@echo
	@echo Options:
	@echo 'MODULES="MODULE1 MODULE2 ..."	modules to be built'
	@echo
	@echo 'Advanced Options (set only if necessary):'
	@echo 'LIBUCI=[builtin|shared]		link or bundle (default) libuci?'
	@echo 'LIBUNL=[builtin|shared]		link or bundle (default) libunl?'
	@echo 'LIBRT=[shared|builtin]		link against librt? (try builtin if shared fails)'
	@echo

$(BINARY).ova: $(BINARY).vdi
	NAME=$(BINARY) MEMORY=$(MEMORY) image/mkvbox-pc $+

$(BINARY).vdi: $(BINARY).loop
	vboxmanage convertfromraw $+ $@

$(BINARY).loop: deb
	touch $@
	sudo IMAGENAME=$(BINARY) IMAGESIZE=$(IMAGESIZE) INSTALL=*.deb image/mkimage-pc


