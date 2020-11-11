SUBDIR=gnutls-3.6.2

all:
	cd $(SUBDIR); test -f Makefile || ./configure --exec-prefix=$(INSTALLDIR) --with-p11-kit=no --enable-static=no --host=arm-linux --with-nettle-mini=yes --with-included-libtasn1 --with-libunistring-prefix --with-included-unistring --enable-local-libopts --enable-tests=no --disable-doc --enable-manpages=no NETTLE_CFLAGS="-I$(PUBLIC_APPSPATH)/libs/libnettle" NETTLE_LIBS="-L$(INSTALLDIR)/lib -lhogweed -lnettle" HOGWEED_CFLAGS="-I$(PUBLIC_APPSPATH)/libs/libnettle" HOGWEED_LIBS="-L$(INSTALLDIR)/lib -lhogweed" 
	cd $(SUBDIR); make
	ln -sf includes $(SUBDIR)/lib/include
	ln -sf .libs $(SUBDIR)/lib/lib

install:
	install $(SUBDIR)/lib/.libs/libgnutls.so.30.20.2 $(INSTALLDIR)/lib/
	ln -s libgnutls.so.30.20.2 $(INSTALLDIR)/lib/libgnutls.so.30
	ln -s libgnutls.so.30 $(INSTALLDIR)/lib/libgnutls.so
clean:
	cd $(SUBDIR); make clean
