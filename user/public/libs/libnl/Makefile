CFLAGS =
SUBDIR = ./libnl-3.3.0

all:
	cd $(SUBDIR) && test -f Makefile || (./configure CC=$(CC) --host=arm-unknown-linux-gnu --prefix=$(INSTALLDIR) --sysconfdir=$(INSTALLDIR) --disable-static)
	cd $(SUBDIR) && test -f Makefile && make 
	install -m 755 $(SUBDIR)/lib/.libs/libnl-3.so.200.25.0 $(INSTALLDIR)/lib/
	install -m 755 $(SUBDIR)/lib/.libs/libnl-genl-3.so.200.25.0 $(INSTALLDIR)/lib/
	install -m 755 $(SUBDIR)/lib/.libs/libnl-idiag-3.so.200.25.0 $(INSTALLDIR)/lib/
	install -m 755 $(SUBDIR)/lib/.libs/libnl-nf-3.so.200.25.0 $(INSTALLDIR)/lib/
	install -m 755 $(SUBDIR)/lib/.libs/libnl-route-3.so.200.25.0 $(INSTALLDIR)/lib/
	install -m 755 $(SUBDIR)/lib/.libs/libnl-xfrm-3.so.200.25.0 $(INSTALLDIR)/lib/
	install -m 755 $(SUBDIR)/src/lib/.libs/libnl-cli-3.so.200.25.0 $(INSTALLDIR)/lib/
	
	ln -sf libnl-3.so.200.25.0 $(INSTALLDIR)/lib/libnl-3.so
	ln -sf libnl-3.so.200.25.0 $(INSTALLDIR)/lib/libnl-3.so.200
	ln -sf libnl-genl-3.so.200.25.0  $(INSTALLDIR)/lib/libnl-genl-3.so
	ln -sf libnl-genl-3.so.200.25.0  $(INSTALLDIR)/lib/libnl-genl-3.so.200
	ln -sf libnl-idiag-3.so.200.25.0 $(INSTALLDIR)/lib/libnl-idiag-3.so
	ln -sf libnl-nf-3.so.200.25.0 $(INSTALLDIR)/lib/libnl-nf-3.so
	ln -sf libnl-route-3.so.200.25.0 $(INSTALLDIR)/lib/libnl-route-3.so
	ln -sf libnl-xfrm-3.so.200.25.0 $(INSTALLDIR)/lib/libnl-xfrm-3.so
	ln -sf libnl-cli-3.so.200.25.0 $(INSTALLDIR)/lib/libnl-cli-3.so
install:

clean:
	cd $(SUBDIR) && test -f Makefile && $(MAKE) clean
