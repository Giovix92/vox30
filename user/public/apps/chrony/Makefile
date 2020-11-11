
SYSCONFDIR=/etc
BINDIR=/usr/bin
SBINDIR=/usr/sbin
LOCALSTATEDIR=/var
CHRONYVARDIR=/var/lib/chrony
DESTDIR=$(INSTALLDIR)
CHRONYDIR=./chrony-3.1

all: 
	cd $(CHRONYDIR) && test -f Makefile ||  ./configure CC=$(CC) --prefix=$(INSTALLDIR)
	cd $(CHRONYDIR) && make
install:
	test -d $(DESTDIR)$(SYSCONFDIR)  || mkdir -p $(DESTDIR)$(SYSCONFDIR)
	test -d $(DESTDIR)$(SBINDIR)  || mkdir -p $(DESTDIR)$(SBINDIR)
	test -d $(DESTDIR)$(BINDIR)  || mkdir -p $(DESTDIR)$(BINDIR)
	test -d $(DESTDIR)$(CHRONYVARDIR)  || mkdir -p $(DESTDIR)$(CHRONYVARDIR)
	if [test -f $(DESTDIR)$(SBINDIR)/chronyd ]; then rm -f $(DESTDIR)$(SBINDIR)/chronyd ; fi
	if [test -f $(DESTDIR)$(BINDIR)/chronyc ]; then rm -f $(DESTDIR)$(BINDIR)/chronyc ; fi
	cp $(CHRONYDIR)/chronyd $(DESTDIR)$(SBINDIR)/chronyd
	chmod 755 $(DESTDIR)$(SBINDIR)/chronyd
	cp $(CHRONYDIR)/chronyc $(DESTDIR)$(BINDIR)/chronyc
	chmod 755 $(DESTDIR)$(BINDIR)/chronyc
	ln -f -s  chronyd  $(DESTDIR)$(SBINDIR)/chronydc

clean:
	cd ./chrony-3.1 && make clean
