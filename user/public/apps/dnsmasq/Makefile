export CFLAGS += -D__SC_BUILD__ -I$(PRIVATE_APPSPATH)/include -I$(PUBLIC_APPSPATH)/include/ -L$(INSTALLDIR)/lib -O2 -fomit-frame-pointer -pipe -fno-strict-aliasing 
export LDFLAGS += -lm -lslog

DNSMASQ_VERSION = 2.80

all:
	cd dnsmasq-$(DNSMASQ_VERSION) && make

install:
	$(STRIP) ./dnsmasq-$(DNSMASQ_VERSION)/src/dnsmasq
	install -m 755 ./dnsmasq-$(DNSMASQ_VERSION)/src/dnsmasq $(INSTALLDIR)/usr/sbin/
	cp -v ./dnsmasq-$(DNSMASQ_VERSION)/dnsmasq.conf.example $(INSTALLDIR)/etc/dnsmasq.conf
	ln -sf /var/dnsmasq.resolv.conf $(INSTALLDIR)/etc/dnsmasq.resolv.conf
	rm -rf $(INSTALLDIR)/tmp/dnsmasq.d
	mkdir $(INSTALLDIR)/tmp/dnsmasq.d

clean:
	cd ./dnsmasq-$(DNSMASQ_VERSION)&& make clean
