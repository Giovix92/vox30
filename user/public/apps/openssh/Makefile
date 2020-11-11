SUBDIR = openssh-8.0p1

export CFLAGS += -L$(INSTALLDIR)/lib -O2 -fomit-frame-pointer -pipe -fno-strict-aliasing -D__SC_BUILD__
CFLAGS += -D_SC_SUPPORT_RSYNC_
CFLAGS += -I. -I$(OPENSSL_DIR)/include -I$(PRIVATE_APPSPATH)/include -I$(srcdir)  $(PATHS) -DHAVE_CONFIG_H 
CFLAGS += -I$(PUBLIC_APPSPATH)/include 
CFLAGS += -I$(PUBLIC_APPSPATH)/libs/zlib/zlib-1.2.11
export LDFLAGS = -L$(INSTALLDIR)/lib -L$(OPENSSL_DIR)
LDFLAGS += -L$(INSTALLDIR)/lib $(X_PRIVATE_CAL_LIBS) -lsex_crypt -lcrypto -lssl -lslog -lmd5 -lsalx -lnv
all:
	cd $(SUBDIR) && test -f Makefile || ./configure CC=$(CC) --host=mips-target-linux-gnu --prefix=/usr --sysconfdir=/etc/ssh --enable-static=yes --enable-shared=no
	cd $(SUBDIR) && make

install:
	$(STRIP) $(SUBDIR)/sshd
	cd $(SUBDIR) && make install


clean:
	cd $(SUBDIR) && make clean



