VERSION_DIR=openswan-2.6.41

export LD_LIBRARY_PATH += $(PUBLIC_APPSPATH)/libs/libgmp/gmp-6.1.2/.libs/
export USERCOMPILE  += -I$(PUBLIC_APPSPATH)/libs/libgmp/gmp-6.1.2/ -fPIC

export $(CXX)
export $(CC)
export $(LD)
export $(RANLIB)
export $(NM)
export $(AR)
export $(AS)
export $(STRIP)
export DESTDIR=$(PUBLIC_APPSPATH)/apps/openswan/$(VERSION_DIR)/tmp-install

all:
	@if test -f ./$(VERSION_DIR)/Makefile.inc ; \
	then \
		echo 'Chnage the INC_USRLOCAL=/usr/local to INC_USRLOCAL=/usr' ; \
		sed -i 's/INC_USRLOCAL=\/usr\/local/INC_USRLOCAL=\/usr/g' ./$(VERSION_DIR)/Makefile.inc ; \
	fi
	make -C $(VERSION_DIR) programs

install:
#	@echo $(PUBLIC_APPSPATH)/apps/openswan/$(VERSION_DIR)/tmp-install
	@if test -d $(PUBLIC_APPSPATH)/apps/openswan/$(VERSION_DIR)/tmp-install/ ; \
	then \
		echo 'rm -Rf tmp-install' ; \
		rm -Rf $(PUBLIC_APPSPATH)/apps/openswan/$(VERSION_DIR)/tmp-install/ ; \
	fi
	make -C $(VERSION_DIR) install
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/addconn
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/eroute
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/ikeping
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/klipsdebug
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/pf_key
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/pluto
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/ranbits
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/rsasigkey
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/showhostkey
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/spi
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/spigrp
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/tncfg
	$(STRIP) $(DESTDIR)/usr/libexec/ipsec/whack

	cp -a $(DESTDIR)/usr/sbin/ipsec    $(INSTALLDIR)/usr/sbin/
	cp -Rf $(DESTDIR)/usr/libexec      $(INSTALLDIR)/usr/
	cp -Rf $(DESTDIR)/usr/lib/*        $(INSTALLDIR)/usr/lib/
	ln -sf /var/ipsec.secrets          $(INSTALLDIR)/etc/ipsec.secrets
	ln -sf /var/ipsec.conf          $(INSTALLDIR)/etc/ipsec.conf

clean:
	make -C $(VERSION_DIR) distclean 
