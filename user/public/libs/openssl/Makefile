SUBDIR = openssl-1.0.2q
ifeq ($(CONFIG_SUPPORT_OPENVPN_CLIENT), 1)
SUBDIRS = openssl-1.1.0g
endif
all:
	touch $(SUBDIR)/Makefile
	cd $(SUBDIR) && make
ifeq ($(CONFIG_SUPPORT_OPENVPN_CLIENT), 1)
	cd $(SUBDIRS) && make
endif

install:
	cd $(SUBDIR) && make install 
	-rm $(INSTALLDIR)/usr/local
	mkdir -p $(INSTALLDIR)/usr/local/ssl
	cp $(SUBDIR)/apps/openssl $(INSTALLDIR)/bin
	cp $(SUBDIR)/apps/openssl.cnf $(INSTALLDIR)/usr/local/ssl
ifeq ($(CONFIG_SUPPORT_OPENVPN_CLIENT), 1)
	cd $(SUBDIRS) && make install
endif

build_gpl:
	cd $(SUBDIR) && make build_gpl
	

clean:
	cd $(SUBDIR) && make clean
ifeq ($(CONFIG_SUPPORT_OPENVPN_CLIENT), 1)
	cd $(SUBDIRS) && make clean
endif
