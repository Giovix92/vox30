VERSION_DIR=cups-1.7.2

all:
	cd ./$(VERSION_DIR) && test -f Makedefs || ./configure CC=$(CC) CXX=$(CXX) --host=mips-linux-uclibc \
		--sysconfdir=/var --disable-dbus --disable-gnutls --disable-gssapi --disable-ssl --disable-avahi --with-printcap=/tmp/printcap \
		--disable-shared 
	cd ./$(VERSION_DIR) && make 

install:
	install -m 755 ./$(VERSION_DIR)/scheduler/cupsd $(INSTALLDIR)/usr/sbin/ 
	install -m 755 ./$(VERSION_DIR)/systemv/lp $(INSTALLDIR)/usr/sbin
	
	-test -d $(INSTALLDIR)/usr/lib/cups || mkdir -m 755 $(INSTALLDIR)/usr/lib/cups
	-test -d $(INSTALLDIR)/usr/lib/cups/backend || mkdir -m 755 $(INSTALLDIR)/usr/lib/cups/backend
	-test -d $(INSTALLDIR)/usr/lib/cups/daemon || mkdir -m 755 $(INSTALLDIR)/usr/lib/cups/daemon
	-test -d $(INSTALLDIR)/usr/lib/cups/filter || mkdir -m 755 $(INSTALLDIR)/usr/lib/cups/filter
	-test -d $(INSTALLDIR)/etc/cups || mkdir -m 755 $(INSTALLDIR)/etc/cups
	-test -d $(INSTALLDIR)/etc/cups/ppd || mkdir -m 755 $(INSTALLDIR)/etc/cups/ppd
	
	#cp -rf ./$(VERSION_DIR)/cups/libcups.so $(INSTALLDIR)/lib/ 
	#install -m 755 ./$(VERSION_DIR)/cups/libcups.so.2 $(INSTALLDIR)/lib/ 
	#cp -rf ./$(VERSION_DIR)/scheduler/libcupsmime.so $(INSTALLDIR)/lib/ 
	#install -m 755 ./$(VERSION_DIR)/scheduler/libcupsmime.so.1 $(INSTALLDIR)/lib/ 
	
	install -m 755 ./$(VERSION_DIR)/scheduler/cups-lpd $(INSTALLDIR)/usr/lib/cups/daemon/
	$(STRIP) $(INSTALLDIR)/usr/lib/cups/daemon/cups-lpd
	install -m 755 ./$(VERSION_DIR)/backend/usb $(INSTALLDIR)/usr/lib/cups/backend/
	$(STRIP) $(INSTALLDIR)/usr/lib/cups/backend/usb
	install -m 755 ./$(VERSION_DIR)/filter/pstops $(INSTALLDIR)/usr/lib/cups/filter/
	$(STRIP) $(INSTALLDIR)/usr/lib/cups/filter/pstops
	install -m 755 ./$(VERSION_DIR)/filter/rastertopwg $(INSTALLDIR)/usr/lib/cups/filter/
	$(STRIP) $(INSTALLDIR)/usr/lib/cups/filter/rastertopwg
	install -m 755 ./$(VERSION_DIR)/conf/mime.convs $(INSTALLDIR)/etc/cups/mime.convs
	install -m 755 ./$(VERSION_DIR)/conf/mime.types $(INSTALLDIR)/etc/cups/mime.types
	install -m 755 ./$(VERSION_DIR)/berkeley/lpr $(INSTALLDIR)/usr/sbin/
	install -m 755 ./$(VERSION_DIR)/berkeley/lpc $(INSTALLDIR)/usr/sbin/
	install -m 755 ./$(VERSION_DIR)/berkeley/lpq $(INSTALLDIR)/usr/sbin/
	install -m 755 ./$(VERSION_DIR)/berkeley/lprm $(INSTALLDIR)/usr/sbin/

clean:
	-cd ./$(VERSION_DIR) && test -f Makedefs &&  make clean && rm -rf Makedefs 

