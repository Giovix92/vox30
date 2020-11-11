SUBDIR=libmicrohttpd-0.9.59

all:
	cd $(SUBDIR); test -f Makefile || ./configure --host=arm-linux enable_https=yes enable_curl=yes with_gnutls=$(GNUTLS_DIR)/lib/ with_libgcrypt_prefix=$(INSTALLDIR)/ --enable-examples=no
	cd $(SUBDIR); make
install:
	$(STRIP) $(SUBDIR)/src/microhttpd/.libs/libmicrohttpd.so.12.46.0
	install $(SUBDIR)/src/microhttpd/.libs/libmicrohttpd.so.12.46.0 $(INSTALLDIR)/lib/
	ln -sf libmicrohttpd.so.12.46.0 $(INSTALLDIR)/lib/libmicrohttpd.so.12
	ln -sf libmicrohttpd.so.12 $(INSTALLDIR)/lib/libmicrohttpd.so
clean:
	cd $(SUBDIR); make clean
