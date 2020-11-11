#***********************************************************************
#
#  Copyright (c) 2012  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/
OPENSSL_LDFLAGS := -L$(BUILDPATH)/lib/lib -L$(INSTALLDIR)/lib
OPENSSL_LDFLAGS += -Wl,-rpath-link=$(BUILDPATH)/lib/lib:$(INSTALLDIR)/lib
export OPENSSL_LDFLAGS

all dynamic install: conditional_build

# Use host openssl for properly configuring openvswitch
PATH := $(PLUMEWIFI_HOST_DIR)/openssl/install/bin:$(PATH)
export LD_LIBRARY_PATH := $(PLUMEWIFI_HOST_DIR)/openssl/install/lib:$(LD_LIBRARY_PATH)

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /user, /user,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
#$(info TOOLCHAIN_PREFIX=$(TOOLCHAIN_PREFIX))
ifeq ($(TOOLCHAIN_PREFIX),)
$(error unknown TOOLCHAIN_PREFIX)
endif

UNTAR_COMPLETE_FILE = .untar_complete
CONFIG_COMPLETE_FILE = config_complete

APP = openvswitch-2.8.5

clean:
	rm -rf $(INSTALLDIR)/etc/openvswitch
	rm -rf $(INSTALLDIR)/opt/modules/openvswitch
	@if [ -e $(APP)/Makefile ]; then \
	cd $(APP); $(MAKE) clean ;$(MAKE) uninstall; \
	rm -f Makefile; \
	cd datapath/linux; $(MAKE) clean;  \
	fi

# The next line is a hint to our release scripts
# GLOBAL_RELEASE_SCRIPT_CALL_DISTCLEAN

$(APP)/$(UNTAR_COMPLETE_FILE): $(APP).tar.gz Makefile
	rm -rf $(APP)
	tar xf $(APP).tar.gz
	(cd $(APP); for PATCH in ../*.patch; do if [ -e "$$PATCH" ]; then patch -p1 < $$PATCH; fi; done)
	@echo "$(APP) is untarred"
	touch $(APP)/$(UNTAR_COMPLETE_FILE)

$(APP)/$(CONFIG_COMPLETE_FILE): $(APP)/$(UNTAR_COMPLETE_FILE) 
	@env
	(cd $(APP); \
		./configure --host=$(TOOLCHAIN_PREFIX) \
		--target=$(TOOLCHAIN_PREFIX) \
		--prefix=/usr \
		--localstatedir=/var \
		--with-logdir=/var/log \
		--with-rundir=/var/run \
		--with-dbdir=/tmp/etc/openvswitch \
		--with-openssl=$(OPENSSL_DIR) \
		--with-linux=$(KERNEL_DIR) \
		--enable-shared \
		--enable-ndebug \
		--disable-libcapng \
		--without-debug \
		LDFLAGS="$(OPENSSL_LDFLAGS) -Wl,-rpath-link,lib/.libs" \
		CFLAGS="-I$(OPENSSL_DIR)/include" \
		KARCH=$(ARCH) \
		);
	@echo "$(APP) is configured"
	touch $(APP)/$(CONFIG_COMPLETE_FILE)

# options that enable kernel module build:
#		--with-linux=$(KERNEL_DIR) \

conditional_build: $(APP)/$(CONFIG_COMPLETE_FILE)
	echo "Building $(APP)"
	$(MAKE) -C $(APP) EXTRA_CFLAGS="-Iarch/arm/mach-bcm963xx/include -Iarch/arm/plat-bcm63xx/include"
	echo "Done building $(APP)"

ROOTFS_TMP="$(CURR_DIR)/$(APP)/rootfs.tmp"

install:
	echo "Installing $(APP)"
	# install to temporary rootfs.tmp
	rm -rf $(ROOTFS_TMP)
	mkdir -p $(ROOTFS_TMP)
	$(MAKE) -C $(APP) install-strip DESTDIR=$(ROOTFS_TMP)
	# cleanup not-needed files
	rm -rf $(ROOTFS_TMP)/usr/bin/*docker*
	rm -rf $(ROOTFS_TMP)/usr/lib/*.a
	rm -rf $(ROOTFS_TMP)/usr/lib/*.la
	rm -rf $(ROOTFS_TMP)/usr/lib/pkgconfig
	rm -rf $(ROOTFS_TMP)/usr/etc
	rm -rf $(ROOTFS_TMP)/usr/include
	rm -rf $(ROOTFS_TMP)/usr/share
	rm -rf $(ROOTFS_TMP)/var
	rm -rf $(ROOTFS_TMP)/tmp
	# install kernel module
	#$(MAKE) -C $(APP) modules_install DESTDIR=$(ROOTFS_TMP)
	mkdir -p $(ROOTFS_TMP)/lib/modules
	install -m 644 ./$(APP)/datapath/linux/openvswitch.ko $(ROOTFS_TMP)/lib/modules/$(APP).ko
	$(STRIP) -g $(ROOTFS_TMP)/lib/modules/$(APP).ko
	# copy rootfs.tmp to actual rootfs
	rm -f $(INSTALLDIR)/usr/sbin/ovsdb-client
	cp -a $(ROOTFS_TMP)/. $(INSTALLDIR)
ifneq ($(CONFIG_SUPPORT_PLUME), 1)
	mkdir -p $(INSTALLDIR)/usr/plume/etc/
	install -m 644 conf.db.bck $(INSTALLDIR)/usr/plume/etc/
endif
	echo "Done installing $(APP)"

install_ovsdb_only:
	${STRIP} ./$(APP)/ovsdb/ovsdb-server
	${STRIP} ./$(APP)/ovsdb/ovsdb-client
	cp -P ./$(APP)/ovsdb/ovsdb-server ${INSTALLDIR}/usr/sbin
	cp -P ./$(APP)/ovsdb/ovsdb-client ${INSTALLDIR}/usr/bin
	echo "Done installing $(APP)"

distclean: clean
	rm -rf $(APP)
