all:
ifeq ($(BRCM_502), 1)
	cd ./curl-7.47.0 && test -f Makefile || env CPPFLAGS="-I$(OPENSSL_DIR)/include" CFLAGS="-D__SC_BUILD__ -DBRCM_502" LDFLAGS="-L$(INSTALLDIR)/lib/" ./configure CC=$(CC) --host=mips-linux-gnu --prefix=$(INSTALLDIR) --with-ssl=$(INSTALLDIR)/lib --enable-static=yes --enable-shared=yes
else
	cd ./curl-7.47.0 && test -f Makefile || env CPPFLAGS="-I$(OPENSSL_DIR)/include" CFLAGS="-D__SC_BUILD__" LDFLAGS="-L$(INSTALLDIR)/lib/" ./configure CC=$(CC) --host=mips-linux-gnu --prefix=$(INSTALLDIR) --with-ssl=$(INSTALLDIR)/lib --enable-static=yes --enable-shared=yes
endif    
	cd ./curl-7.47.0/ && test -f Makefile && make 
#	mkdir $(INSTALLDIR)/lib/curl-7.47.0
	cp ./curl-7.47.0/lib/.libs/libcurl.so* $(INSTALLDIR)/lib/
	cd $(INSTALLDIR)/lib/ && ln -sf libcurl.so.4.4.0  libcurl.so.4
ifeq ($(CONFIG_SUPPORT_LIBCURL), 0)
	rm -f $(INSTALLDIR)/lib/libcurl.so
else
	cd $(INSTALLDIR)/lib/ && ln -sf libcurl.so.4  libcurl.so
endif

install:
	install -m 755 curl-7.47.0/src/.libs/curl $(INSTALLDIR)/usr/sbin/

build_gpl:
	cp ./curl-7.47.0/lib/.libs/libcurl.so* $(GPL_INSTALLDIR)/lib/
	cd $(GPL_INSTALLDIR)/lib/ && ln -sf libcurl.so.4.4.0  libcurl.so.4
ifeq ($(CONFIG_SUPPORT_LIBCURL), 0)
	rm -f $(GPL_INSTALLDIR)/lib/libcurl.so
else
	cd $(GPL_INSTALLDIR)/lib/ && ln -sf libcurl.so.4  libcurl.so
endif
	install -m 755 curl-7.47.0/src/.libs/curl $(GPL_INSTALLDIR)/usr/sbin/

clean:
#	cd ./curl-7.47.0 test -f Makefile  && make clean && rm -f Makefile
#	-rm -f libcurl.so*

