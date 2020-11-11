SUBDIR=mosquitto-master

unexport CROSS_COMPILE
CFLAGS += -I$(OPENSSL_DIR)/include
export CFLAGS
LDFLAGS += -L$(INSTALLDIR)/lib -lssl -lcrypto
export LDFLAGS
all:
	cd $(SUBDIR); make

install:
	install $(SUBDIR)/lib/libmosquitto.so.1 $(INSTALLDIR)/lib/
	ln -sf libmosquitto.so.1 $(INSTALLDIR)/lib/libmosquitto.so
	install $(SUBDIR)/src/mosquitto $(INSTALLDIR)/usr/sbin
clean:
	cd $(SUBDIR); make clean
