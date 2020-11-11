SUB =
SUB += source3
#include ../../../Rules.mak
TARDIR = ../../../fs1
ifeq ($(CONFIG_SUPPORT_PACKAGE), 1)
PKG_NAME:=samba
PKG_RELEASE:=1
VERSION:=3.6.22
SECTION:=utils
CATEGORY:=Application
TITLE:=File transfer tool
define Package/$(PKG_NAME)/install
$(INSTALL_DIR) $(1)/etc; \
$(INSTALL_DIR) $(1)/etc/samba.conf;\
$(INSTALL_BIN) etc/samba.conf/smb.conf $(1)/etc/samba.conf/;\
$(INSTALL_DIR) $(1)/usr;\
$(INSTALL_DIR) $(1)/usr/sbin/;\
$(INSTALL_BIN) source3/bin/samba_multicall $(1)/usr/sbin/;\
ln -s samba_multicall $(1)/usr/sbin/smbd;\
ln -s samba_multicall $(1)/usr/sbin/nmbd;\
ln -s samba_multicall $(1)/usr/sbin/smbpasswd;\
$(INSTALL_DIR) $(1)/etc/;\
$(INSTALL_DIR) $(1)/etc/init.d;\
$(INSTALL_BIN) SAMBA $(1)/etc/init.d/samba;
endef
include $(BUILDPATH)/include/buildpackage.mk
endif
#STRIP=$(CROSS)strip

.EXPORT_ALL_VARIABLES:

all:
	cd ./source3 && echo samba_cv_CC_NEGATIVE_ENUM_VALUES=yes > mips-linux.cache && \
		echo replacelib_cv_HAVE_GETADDRINFO=no >> mips-linux.cache \
		&& echo ac_cv_file__proc_sys_kernel_core_pattern=false >> mips-linux.cache \
		&& echo libreplace_cv_HAVE_IFACE_IFCONF=yes >> mips-linux.cache
	cd ./source3 && test -f Makefile || ./configure CC=$(CC) --build=i686-linux --host=mips-target-linux-gnu --target=mips-target-linux-gnu --prefix=$(INSTALLDIR) \
         --enable-swat=no \
         --enable-cups=no \
         --enable-iprint=no \
         --enable-pie=no \
         --enable-fam=no \
         --enable-static=yes \
         --enable-shared-libs=no \
		 --enable-dnssd=no \
         --with-utmp=no \
		 --with-ads=no \
         --with-ldap=no \
		 --with-libtalloc=no \
         --with-libtdb=no \
         --with-libnetapi=no \
         --with-libaddns=no \
		 --with-libsmbclient=no \
         --with-libsmbsharemodes=no \
         --with-acl-support=no \
		 --with-winbind=no \
		 --with-dnsupdate=no \
         --cache-file=mips-linux.cache 
	     cd ./source3 && make

install:
ifeq ($(CONFIG_SUPPORT_PACKAGE), 1)
	$(STRIP) source3/bin/samba_multicall
	$(BuildTargetipkg)
	$(BUILDPATH)/util/bin/opkg-cl -f $(INSTALLDIR)/etc/opkg/arch.conf -o $(INSTALLDIR)/ install $(BUILDPATH)/ipk/ipk/$(PKG_NAME)_$(VERSION)_$(PKGARCH).ipk
	ln -sf ../init.d/samba $(INSTALLDIR)/etc/rc.d/S8samba
	ln -sf ../init.d/samba $(INSTALLDIR)/etc/rc.d/K8samba
	install pkg/samba $(INSTALLDIR)/usr/local/package/pkg/
	install eu/2 $(INSTALLDIR)/usr/local/package/eu/
	install eu/2 $(BUILDPATH)/PACKAGE/defaultsoftwaremodules/eu/
	install du/3 $(INSTALLDIR)/usr/local/package/du/
	install du/3 $(BUILDPATH)/PACKAGE/defaultsoftwaremodules/du/
else
	mkdir -p $(TARDIR)
	mkdir -p $(TARDIR)/lib
	mkdir -p $(TARDIR)/usr/sbin
	cp -rf etc/samba.conf $(INSTALLDIR)/etc/
	cp codepages/lowcase.dat codepages/upcase.dat $(INSTALLDIR)/etc/samba.conf
	cp source3/bin/samba_multicall $(TARDIR)/usr/sbin/
	$(STRIP) $(TARDIR)/usr/sbin/samba_multicall
	cp -f $(TARDIR)/usr/sbin/samba_multicall $(INSTALLDIR)/usr/sbin/
	ln -sf /usr/sbin/samba_multicall $(INSTALLDIR)/usr/sbin/smbd
	ln -sf /usr/sbin/samba_multicall $(INSTALLDIR)/usr/sbin/nmbd
	ln -sf /usr/sbin/samba_multicall $(INSTALLDIR)/usr/sbin/smbpasswd
	rm -rf  $(TARDIR)
endif
	
clean:
	rm -f *.[oa] *~ core
	for i in $(SUB) ; do ${MAKE} -C $$i clean || exit 1; done
	rm $(SUB)/Makefile 

	
