# Makefile for systems with GNU tools
#CC 	=	$(TOOLPREFIX)gcc
INSTALL	=	install
IFLAGS  = -idirafter dummyinc
CHECK_IPV6_ADDRESS = 1

#CFLAGS = -g
CFLAGS	+= -O2 -Wall -W -Wshadow  -I$(OPENSSL_DIR)/include -I$(PUBLIC_APPSPATH)/include#-pedantic -Werror -Wconversion
LIBS	=  -L$(INSTALLDIR)/lib -L$(OPENSSL_DIR) `./vsf_findlibs.sh`  
LDFLAGS += -lcrypt
ifneq ($(BRCM_502), 1)
CFLAGS += -I$(KERNELDIR)/include
else
CFLAGS += -idirafter$(KERNELDIR)/include
endif
ifeq ($(CHECK_IPV6_ADDRESS),1)
CFLAGS  += -D__CHECK_IPV6_ADDRESS__
LDFLAGS += -L$(INSTALLDIR)/lib -lslog
endif

#LIBS	=	`./vsf_findlibs.sh`
LINK	=	-Wl,-s
#LDFLAGS	=	-fPIE -pie -Wl,-z,relro -Wl,-z,now

OBJS	=	main.o utility.o prelogin.o ftpcmdio.o postlogin.o privsock.o \
		tunables.o ftpdataio.o secbuf.o ls.o \
		postprivparent.o logging.o str.o netstr.o sysstr.o strlist.o \
    banner.o filestr.o parseconf.o secutil.o \
    ascii.o oneprocess.o twoprocess.o privops.o standalone.o hash.o \
    tcpwrap.o ipaddrparse.o access.o features.o readwrite.o opts.o \
    ssl.o sslslave.o ptracesandbox.o ftppolicy.o sysutil.o sysdeputil.o \
    seccompsandbox.o

ifeq ($(CONFIG_SUPPORT_PACKAGE), 1)
PKG_NAME:=vsftpd
PKG_RELEASE:=1
VERSION:=3.0.2
SECTION:=utils
CATEGORY:=Application
TITLE:=File transfer tool
define Package/$(PKG_NAME)/install
$(INSTALL_DIR) $(1)/usr/sbin;\
$(INSTALL_BIN) vsftpd $(1)/usr/sbin/;\
ln -s vsftpd $(1)/usr/sbin/ftps;\
$(INSTALL_DIR) $(1)/etc;\
$(INSTALL_BIN) vsftpd.pem $(1)/etc/;\
$(INSTALL_DIR) $(1)/etc/init.d;\
$(INSTALL_BIN) VSFTPD $(1)/etc/init.d/ftp;
endef
include $(BUILDPATH)/include/buildpackage.mk
endif
.c.o:
	$(CC) -c $*.c $(CFLAGS) $(IFLAGS)

vsftpd: $(OBJS) 
	chmod 755 ./vsf_findlibs.sh
	$(CC) -o vsftpd $(OBJS) $(LINK) $(LDFLAGS) $(LIBS)

install:
ifeq ($(CONFIG_SUPPORT_PACKAGE), 1)
	$(STRIP) vsftpd
	$(BuildTargetipkg)
	$(BUILDPATH)/util/bin/opkg-cl -f $(INSTALLDIR)/etc/opkg/arch.conf -o $(INSTALLDIR)/ install $(BUILDPATH)/ipk/ipk/$(PKG_NAME)_$(VERSION)_$(PKGARCH).ipk
	ln -sf ../init.d/ftp $(INSTALLDIR)/etc/rc.d/S6ftp
	ln -sf ../init.d/ftp $(INSTALLDIR)/etc/rc.d/K6ftp
	install pkg/vsftpd $(INSTALLDIR)/usr/local/package/pkg/
	install eu/3 $(INSTALLDIR)/usr/local/package/eu/
	install eu/3 $(BUILDPATH)/PACKAGE/defaultsoftwaremodules/eu/
	install du/4 $(INSTALLDIR)/usr/local/package/du/
	install du/4 $(BUILDPATH)/PACKAGE/defaultsoftwaremodules/du/
else
	if [ -x $(INSTALLDIR)/usr/local/sbin ]; then \
		$(INSTALL) -m 755 vsftpd $(INSTALLDIR)/usr/local/sbin/vsftpd; \
	else \
		$(INSTALL) -m 755 vsftpd $(INSTALLDIR)/usr/sbin/vsftpd; fi
	$(INSTALL) -m 755 vsftpd.pem $(INSTALLDIR)/etc/vsftpd.pem
	ln -sf /usr/sbin/vsftpd $(INSTALLDIR)/usr/sbin/ftps 
endif
clean:
	rm -f *.o *.swp vsftpd

