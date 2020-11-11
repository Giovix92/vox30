SUBDIR = iperf-3.5
export CFLAGS += -D__SC_BUILD__ -I$(PUBLIC_APPSPATH)/include -L$(INSTALLDIR)/lib -O2 -fomit-frame-pointer -pipe -fno-strict-aliasing
export LD_LIBRARY_PATH += $(INSTALLDIR)/usr/libs/libiperf.so.0
export LDFLAGS += -lm -lsalx -lnv -liperf_drv
export LDFLAGS += -L$(INSTALLDIR)/lib -L$(OPENSSL_DIR) 
LDFLAGS += -L$(INSTALLDIR)/lib  -lcrypto -lssl
all:
	cd ./$(SUBDIR) && test -f Makefile || (./configure CC=$(CC) --host=arm-linux --prefix=/ --with-openssl=$(OPENSSL_DIR))
	cd $(SUBDIR) && make
install:
	#$(STRIP) ./$(SUBDIR)/src/iperf3
	cd $(SUBDIR) && make install
clean:
	cd $(SUBDIR) && make clean
