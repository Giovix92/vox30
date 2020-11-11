
CFLAGS += -D__SC_BUILD__ -I$(PRIVATE_APPSPATH)/include -L$(INSTALLDIR)/lib -O2 -fomit-frame-pointer -pipe -fno-strict-aliasing 
export CFLAGS += -I$(PUBLIC_APPSPATH)/include 
export LDFLAGS += -L$(INSTALLDIR)/lib -lslog $(X_PRIVATE_SAL_LIBS) $(X_PRIVATE_CAL_LIBS)

all:
	cd ./wide-dhcpv6-20080615 && test -f Makefile || ./configure --with-opt-refreshtime=32 CC=$(CC) --host=mips-target-linux-gnu --prefix=$(INSTALLDIR)  --enable-static=yes --enable-shared=no
	cd ./wide-dhcpv6-20080615 && make 
install:
	$(STRIP) ./wide-dhcpv6-20080615/dhcp6s
	install -m 755 ./wide-dhcpv6-20080615/dhcp6s $(INSTALLDIR)/usr/sbin/
	$(STRIP) ./wide-dhcpv6-20080615/dhcp6c
	install -m 755 ./wide-dhcpv6-20080615/dhcp6c $(INSTALLDIR)/usr/sbin/

clean:
	cd ./wide-dhcpv6-20080615 && test -f Makefile &&  make clean



