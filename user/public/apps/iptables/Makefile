export CFLAGS += -L$(INSTALLDIR)/lib -O2 -fomit-frame-pointer -pipe -fno-strict-aliasing 
ifneq ($(BRCM_502), 1)
CFLAGS += -I$(KERNELDIR)/include
else
CFLAGS += -idirafter$(KERNELDIR)/include
endif
all:
	touch iptables-1.4.12/Makefile.in
	touch iptables-1.4.12/libipq/Makefile.in
	touch iptables-1.4.12/include/Makefile.in
	touch iptables-1.4.12/libiptc/Makefile.in
	touch iptables-1.4.12/utils/Makefile.in
	touch iptables-1.4.12/iptables/Makefile.in
	touch iptables-1.4.12/aclocal.m4
ifeq ($(CONFIG_SUPPORT_FON), 1)
	cd ./iptables-1.4.12 && test -f Makefile || ./configure CC=$(CC) --host=mips-target-linux-gnu --prefix=$(INSTALLDIR)  --enable-static=no --enable-shared=yes --with-xtlibdir=/lib/xtables
else
	cd ./iptables-1.4.12 && test -f Makefile || ./configure CC=$(CC) --host=mips-target-linux-gnu --prefix=$(INSTALLDIR)  --enable-static=yes --enable-shared=no
endif
ifeq ($(CONFIG_SUPPORT_CNAPT), 1)
	ln -sfv $(shell ls $(PRIVATE_APPSPATH)/cnapt/user/iptables/*) iptables-1.4.12/extensions/ ;
	rm -f iptables-1.4.12/extensions/libipt_NATMGR.c;
endif
	cd ./iptables-1.4.12 && make 
ifeq ($(CONFIG_SUPPORT_FON), 1)
	install -m 644 ./iptables-1.4.12/libiptc/.libs/libip4tc.so.0.0.0 $(INSTALLDIR)/lib/
	install -m 644 ./iptables-1.4.12/libiptc/.libs/libip6tc.so.0.0.0 $(INSTALLDIR)/lib/
	install -m 644 ./iptables-1.4.12/libiptc/.libs/libiptc.so.0.0.0 $(INSTALLDIR)/lib/
	ln -sf libiptc.so.0.0.0 $(INSTALLDIR)/lib/libiptc.so
	ln -sf libiptc.so.0.0.0 $(INSTALLDIR)/lib/libiptc.so.0
	ln -sf libip4tc.so.0.0.0 $(INSTALLDIR)/lib/libip4tc.so
	ln -sf libip4tc.so.0.0.0 $(INSTALLDIR)/lib/libip4tc.so.0
ifeq ($(CONFIG_SUPPORT_IPV6), 1)
	ln -sf libip6tc.so.0.0.0 $(INSTALLDIR)/lib/libip6tc.so
	ln -sf libip6tc.so.0.0.0 $(INSTALLDIR)/lib/libip6tc.so.0
endif
endif

install:
ifeq ($(CONFIG_SUPPORT_FON), 1)
	$(STRIP) ./iptables-1.4.12/iptables/.libs/xtables-multi
	install -m 755 ./iptables-1.4.12/iptables/.libs/xtables-multi $(INSTALLDIR)/usr/sbin/
	install -m 644 ./iptables-1.4.12/iptables/.libs/libxtables.so.7.0.0 $(INSTALLDIR)/lib/
	ln -sf /lib/libxtables.so.7.0.0 $(INSTALLDIR)/lib/libxtables.so
	ln -sf /lib/libxtables.so.7.0.0 $(INSTALLDIR)/lib/libxtables.so.7
	test -d $(INSTALLDIR)/lib/xtables || mkdir $(INSTALLDIR)/lib/xtables
	install -m 644 ./iptables-1.4.12/extensions/*.so $(INSTALLDIR)/lib/xtables/
else
	$(STRIP) ./iptables-1.4.12/iptables/xtables-multi
	install -m 755 ./iptables-1.4.12/iptables/xtables-multi $(INSTALLDIR)/usr/sbin/
endif
	ln -sf /usr/sbin/xtables-multi $(INSTALLDIR)/usr/sbin/iptables
	ln -sf /usr/sbin/xtables-multi $(INSTALLDIR)/usr/sbin/iptables-save
	ln -sf /usr/sbin/xtables-multi $(INSTALLDIR)/usr/sbin/iptables-restore
ifeq ($(CONFIG_SUPPORT_IPV6), 1)
	ln -sf /usr/sbin/xtables-multi $(INSTALLDIR)/usr/sbin/ip6tables
	ln -sf /usr/sbin/xtables-multi $(INSTALLDIR)/usr/sbin/ip6tables-save
	ln -sf /usr/sbin/xtables-multi $(INSTALLDIR)/usr/sbin/ip6tables-restore
endif


clean:
	cd ./iptables-1.4.12 && test -f Makefile &&  make clean


