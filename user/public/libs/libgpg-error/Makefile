SUBDIR=libgpg-error-1.32

all:
	cd $(SUBDIR); test -f Makefile || ./configure --host=arm-unknown-linux-gnueabi CC=$(CC) prefix=$(INSTALLDIR) --includedir=$(PUBLIC_APPSPATH)/libs/libgpg-error/$(SUBDIR)/src/
	cd $(SUBDIR); make
install:
	install $(SUBDIR)/src/.libs/libgpg-error.so.0.24.3 $(INSTALLDIR)/lib/
	$(STRIP) $(INSTALLDIR)/lib/libgpg-error.so.0.24.3
	ln -sf libgpg-error.so.0.24.3 $(INSTALLDIR)/lib/libgpg-error.so.0
	ln -sf libgpg-error.so.0 $(INSTALLDIR)/lib/libgpg-error.so
clean:
	cd $(SUBDIR); make clean
