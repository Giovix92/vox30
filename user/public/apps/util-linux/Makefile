
#export CFLAGS += -D__SC_BUILD__ -L$(INSTALLDIR)/lib -I$(PUBLIC_APPSPATH)/libs/zlib/zlib-1.2.3/
#export LDFLAGS += -L$(INSTALLDIR)/lib $(X_PRIVATE_LIBS) $(X_PUBLIC_LIBS)

TARGET_DIR = ./util-linux-2.32/

all:
	test -d $(TARGET_DIR) || tar zxf util-linux.tgz
	cd $(TARGET_DIR);test -f Makefile || (./configure CC=$(CC) --host=arm-unknown-linux-gnu --prefix=$(INSTALLDIR) --disable-agetty)
	cd $(TARGET_DIR) && make
install:
	install -m 755 $(TARGET_DIR)/.libs/lsblk $(INSTALLDIR)/usr/sbin/
	install -m 755 $(TARGET_DIR)/.libs/libblkid.so.1.1.0 $(INSTALLDIR)/lib/
	ln -sf libblkid.so.1.1.0 $(INSTALLDIR)/lib/libblkid.so.1
	ln -sf libblkid.so.1.1.0 $(INSTALLDIR)/lib/libblkid.so
	install -m 755 $(TARGET_DIR)/.libs/libmount.so.1.1.0 $(INSTALLDIR)/lib/
	ln -sf libmount.so.1.1.0 $(INSTALLDIR)/lib/libmount.so.1
	ln -sf libmount.so.1.1.0 $(INSTALLDIR)/lib/libmount.so
	install -m 755 $(TARGET_DIR)/.libs/libsmartcols.so.1.1.0 $(INSTALLDIR)/lib/
	ln -sf libsmartcols.so.1.1.0 $(INSTALLDIR)/lib/libsmartcols.so.1
	ln -sf libsmartcols.so.1.1.0 $(INSTALLDIR)/lib/libsmartcols.so
	install -m 755 $(TARGET_DIR)/.libs/libuuid.so.1.3.0 $(INSTALLDIR)/lib/
	ln -sf libuuid.so.1.3.0 $(INSTALLDIR)/lib/libuuid.so.1
	ln -sf libuuid.so.1.3.0 $(INSTALLDIR)/lib/libuuid.so
clean:
	cd $(TARGET_DIR) && test -f Makefile && make clean


