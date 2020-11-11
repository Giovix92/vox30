DIR=rpcd
all:
	if [ ! -e $(DIR)/Makefile ]; then\
		cd $(DIR) ; cmake . -DCMAKE_LIBRARY_PATH=$(INSTALLDIR)/lib/ -Dubus_include_dir=$(PUBLIC_APPSPATH)/libs/ubus  -DCMAKE_C_FLAGS="$(CFLAGS) -I$(PUBLIC_APPSPATH)/libs/ubus/libubox/libubox/include/ -I$(PUBLIC_APPSPATH)/libs/ubus/ubus/ubus -I$(PUBLIC_APPSPATH)/libs/ubus/json-c/json-c/ -I$(PUBLIC_APPSPATH)/libs/uci/uci/ -L$(INSTALLDIR)/lib/ -lcrypt " -DIWINFO_SUPPORT=off;\
	fi 
	cd $(DIR); \
	make

install:
	$(STRIP) $(DIR)/rpcd
	install -D $(DIR)/rpcd $(INSTALLDIR)/bin/
	mkdir -p $(BUILDPATH)/PACKAGE/usr
	mkdir -p $(BUILDPATH)/PACKAGE/usr/lib
	mkdir -p $(BUILDPATH)/PACKAGE/usr/lib/rpcd
	install -D $(DIR)/*.so $(BUILDPATH)/PACKAGE/usr/lib/rpcd/


clean:
	cd $(DIR) && make clean;

