DIR=opkg-0.1.8
all:
	if [ ! -e $(DIR)/Makefile ]; then\
		cd $(DIR) && ./autogen.sh --prefix=$(INSTALLDIR) --host="arm-linux" --enable-openssl=yes --enable-gpg=0 CC=${CC} CFLAGS="$(CFLAGS) -I$(OPENSSL_DIR)/include/" CURL_LIBS="-L$(INSTALLDIR)/lib/ -lcurl" OPENSSL_LIBS="-L$(INSTALLDIR)/lib/ -lcrypto -lssl " CURL_CFLAGS="-I$(PRIVATE_APPSPATH)/libs/libcurl/curl-7.47.0/include/";\
	fi
	cd $(DIR); \
	make

install:
	$(STRIP) $(DIR)/src/.libs/opkg-cl 
	install $(DIR)/src/.libs/opkg-cl $(INSTALLDIR)/bin/opkg
	#$(STRIP) $(DIR)/ubus/.libs/opkg-ubus
	#install $(DIR)/ubus/.libs/opkg-ubus $(INSTALLDIR)/bin/opkg-ubus
	install $(DIR)/libopkg/.libs/libopkg.so.0 $(INSTALLDIR)/lib/libopkg.so.0
	ln -sf libopkg.so.0 $(INSTALLDIR)/lib/libopkg.so
	$(STRIP) $(INSTALLDIR)/lib/libopkg.so
	#install -m 744 $(DIR)/ubus/opkg-ubus.script $(BUILDPATH)/PACKAGE/etc/init.d/opkg-ubus
	#ln -s ../init.d/opkg-ubus $(BUILDPATH)/PACKAGE/etc/rc.d/S1opkg-ubus


clean:
	cd $(DIR) && make clean;
	#	cd $(DIR) ; ./autogen.sh --host="mips-linux" CC=${CC} CFLAGS="$(CFLAGS) -I$(OPENSSL_DIR)/include/" ;

