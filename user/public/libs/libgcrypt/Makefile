SUBDIR=libgcrypt-1.8.3

all:
	cd $(SUBDIR); test -f Makefile || ./configure --host=arm-linux --disable-doc GPG_ERROR_LIBS=$(INSTALLDIR)/lib/ GPG_ERROR_CONFIG="$(PUBLIC_APPSPATH)/libs/libgpg-error/libgpg-error-1.32/src/gpg-error-config"
	cd $(SUBDIR); make
install:
	$(STRIP) $(SUBDIR)/src/.libs/libgcrypt.so.20.2.3
	install $(SUBDIR)/src/.libs/libgcrypt.so.20.2.3 $(INSTALLDIR)/lib/
	ln -sf libgcrypt.so.20.2.3 $(INSTALLDIR)/lib/libgcrypt.so.20
	ln -sf libgcrypt.so.20.2.3 $(INSTALLDIR)/lib/libgcrypt.so
	install $(SUBDIR)/src/libgcrypt-config $(INSTALLDIR)/bin/
clean:
	cd $(SUBDIR); make clean
