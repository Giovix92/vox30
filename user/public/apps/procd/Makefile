DIR=procd-2015-08-16
all:
	if [ ! -e $(DIR)/Makefile ]; then\
		cd $(DIR) ; cmake . -DC_COMPILER=$(CC) -DCMAKE_LINKER=$(LD) -DCMAKE_AR=$(AR) -DCMAKE_STRIP=$(STRIP) -DCMAKE_C_FLAGS="$(CFLAGS) -I$(PUBLIC_APPSPATH)/libs/ubus/libubox/libubox/include/ -I$(PUBLIC_APPSPATH)/libs/ubus/ubus/ubus -I$(PUBLIC_APPSPATH)/libs/ubus/json-c/json-c/" -DCMAKE_EXE_LINKER_FLAGS="-L$(INSTALLDIR)/lib/" -Dubox_include_dir=$(PUBLIC_APPSPATH)/libs/ubus/libubox/libubox/include;\
	fi 
	cd $(DIR); \
	make

install:
	$(STRIP) $(DIR)/procd
	$(STRIP) $(DIR)/udevtrigger
	$(STRIP) $(DIR)/askfirst
	mkdir -p $(BUILDPATH)/PACKAGE/sbin
	install -D $(DIR)/procd $(BUILDPATH)/PACKAGE/sbin/init
	install -D $(DIR)/udevtrigger $(BUILDPATH)/PACKAGE/sbin/
	install -D $(DIR)/askfirst $(BUILDPATH)/PACKAGE/sbin/
ifeq ($(CONFIG_SUPPORT_PACKAGE), 1)
	install -D $(DIR)/procd $(INSTALLDIR)/usr/LXC/SmartThings-EE/sbin/init
	install -D $(DIR)/procd $(INSTALLDIR)/usr/LXC/Security-EE/sbin/init
endif


clean:
	cd $(DIR) && make clean;

