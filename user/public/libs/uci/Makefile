DIR=uci-01012018
all:
	if [ ! -e $(DIR)/Makefile ]; then\
		cd $(DIR) ; cmake . -DCMAKE_LIBRARY_PATH=$(INSTALLDIR)/lib/ -Dubox_include_dir=$(PUBLIC_APPSPATH)/libs/ubus  -DCMAKE_C_FLAGS="$(CFLAGS) -I$(PUBLIC_APPSPATH)/libs/ubus/libubox/libubox/include/ -I$(PUBLIC_APPSPATH)/libs/ubus/ubus/ubus -I$(PUBLIC_APPSPATH)/libs/ubus/json-c/json-c/ -L$(INSTALLDIR)/lib/" -DBUILD_LUA=off; \
	fi 
	cd $(DIR); \
	make

install:
	$(STRIP) $(DIR)/libuci.so
	install -m 755 $(DIR)/libuci.so $(INSTALLDIR)/lib/libuci.so
	$(STRIP) $(DIR)/uci
	install -m 755 $(DIR)/uci $(INSTALLDIR)/bin/uci

clean:
	cd $(DIR) && make clean;

