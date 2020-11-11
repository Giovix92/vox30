SUBDIR=nettle

all:
	cd $(SUBDIR); test -f Makefile || ./configure --host=arm-linux --enable-mini-gmp
	cd $(SUBDIR); make
install:
	install $(SUBDIR)/libnettle.so $(INSTALLDIR)/lib/libnettle.so.6
	ln -s libnettle.so.6 $(INSTALLDIR)/lib/libnettle.so
	install $(SUBDIR)/libhogweed.so $(INSTALLDIR)/lib/libhogweed.so.4
	ln -s libhogweed.so.4 $(INSTALLDIR)/lib/libhogweed.so

clean:
	cd $(SUBDIR); make clean
