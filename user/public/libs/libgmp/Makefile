VERSION_DIR=gmp-6.1.2

GMP_CONF_OPT := prefix=$(INSTALLDIR)/ CC=$(CC) AR=$(AR) AS=$(AS) LD=$(LD) NM=$(NM) \
                CXX=$(CXX) OBJCOPY=$(OBJCOPY) OBJDUMP=$(OBJDUMP) STRIP=$(STRIP) \
                RANLIB=$(RANLIB) READELF=$(CROSS_COMPILE)readelf CPP=$(CROSS_COMPILE)cpp \
                 --host=arm-linux --enable-static 
#CFLAGS="-fPIC -fno-stack-protector"

all:
	cd ./$(VERSION_DIR) && test -f Makefile || ./configure $(GMP_CONF_OPT)
	make -C $(VERSION_DIR)

install:
	@echo install libgmp
	$(STRIP) $(VERSION_DIR)/.libs/libgmp.so.10.3.2
	install -m 755 $(VERSION_DIR)/.libs/libgmp.so $(INSTALLDIR)/lib/
	install -m 755 $(VERSION_DIR)/.libs/libgmp.so.10 $(INSTALLDIR)/lib/
	install -m 755 $(VERSION_DIR)/.libs/libgmp.so.10.3.2 $(INSTALLDIR)/lib/

	ln -sf libgmp.so.10.3.2 $(INSTALLDIR)/lib/libgmp.so
	ln -sf libgmp.so.10.3.2 $(INSTALLDIR)/lib/libgmp.so.10

clean:
	-cd ./$(VERSION_DIR) && test -f Makefile &&  make distclean
