# ------------------------------------------------------------------------------------------
#                    Makefile for BroadLight GateMakerPro or PONMakerPro
#                                                      breeze zhang
#                                                       2010-1-11
# ------------------------------------------------------------------------------------------

export TOPDIR=$(shell pwd)/..
export BUILDPATH = $(TOPDIR)/build

include $(BUILDPATH)/rules.mk

#
# Common targts
#

HTML_CUSTOMER := $(addprefix html_, $(SC_CUSTOMER))
all:
	make toolchain_prep
	make rootfs_prep
	make pre_build
#	make uimage
	make host_build
	make libs_build
	make user_build
#	make drivers_build
#	make image_build

.PHONY:build_gpl
build_gpl:
	cd $(BSP_DIR); make build_gpl SC_BUILD=1
	cd $(TOPDIR)/user/private && $(MAKE) build_gpl 
	cd $(TOPDIR)/drivers && $(MAKE) build_gpl

toolchain_prep:
	sudo ln -sf $(PID) $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/.pid 
ifneq ($(GPL), 1)
ifeq ($(BRCM_502), 1)
	test -d /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/ || (test -f ../toolchain/brcm_toolchain_502/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25.Rel1.9.tar.bz2 || (mkdir -p ../toolchain && mkdir -p ../toolchain/brcm_toolchain_502 && svn co http://172.21.67.236/svn/VOX3.0/toolchain/brcm_toolchain_502/ ../toolchain/brcm_toolchain_502/))
	test -d /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/ || sudo tar -xvf ../toolchain/brcm_toolchain_502/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25.Rel1.9.tar.bz2 -C /
else
	test -d /opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL/ || (test -f ../toolchain/brcm_toolchain_416/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL.Rel1.2.tar.bz2 || (mkdir -p ../toolchain && mkdir -p ../toolchain/brcm_toolchain_416 && svn co http://172.21.67.236/svn/VOX3.0/toolchain/brcm_toolchain_416/ ../toolchain/brcm_toolchain_416/))
	test -d /opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL/ || sudo tar -xvf ../toolchain/brcm_toolchain_416/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL.Rel1.2.tar.bz2 -C /
endif
else
ifeq ($(BRCM_502), 1)
	cd ../toolchain/brcm_toolchain_502 && test -d src || tar -xvf crosstools-gcc-5.3-linux-4.1-uclibc-1.0.12-glibc-2.24-binutils-2.25-sources.tar.bz2
	test -d /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/ || (cd ../toolchain/brcm_toolchain_502 && test -d src &&  cd $(TOPDIR)/toolchain/brcm_toolchain_502 && sudo make ARCH=arm)
else
ifeq ($(BRCM_416), 1)
	cd ../toolchain/brcm_toolchain_416 && test -d src || tar -xvf crosstools-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-sources.tar.bz2
	cd ../toolchain/brcm_toolchain_416 && test -d src && cd $(TOPDIR)/toolchain/brcm_toolchain_416 && sudo make ARCH=arm
else
	cd ../ && test -f uclibc-crosstools-gcc_source-4.4.2-1.tar.bz2 || tar -xvf toolchain.tgz
	cd ../ && test -d buildroot-4.4.2-1 || tar -xvf uclibc-crosstools-gcc_source-4.4.2-1.tar.bz2
	cd ../ && test -d buildroot-4.4.2-1 && cd $(TOPDIR)/buildroot-4.4.2-1/ && sudo make
endif
endif
endif
#util build
host_build:
	@echo host utility build

ifneq ($(GPL), 1)
	cd ./util/src &&  $(MAKE)
	$(BUILDPATH)/util/bin/privilegeCvt $(PRIVATE_APPSPATH)/apps/www/PAGE_PROP_INFO.$(PRODUCT_TYPE).$(SC_CUSTOMER) $(PRIVATE_APPSPATH)/libs/libsal/auto_page_priv.c $(PRIVATE_APPSPATH)/apps/www/$(HTML_CUSTOMER)/js/menu_admin.js $(PRIVATE_APPSPATH)/apps/www/$(HTML_CUSTOMER)/js/menu_support.js $(PRIVATE_APPSPATH)/apps/www/$(HTML_CUSTOMER)/js/menu_user.js
	cd $(HOSTTOOLS_DIR) && $(MAKE)
endif

west_debug:
	cd $(BSP_DIR) && make west
host_clean:
	@echo host utility build clean
ifneq ($(GPL), 1)
	cd ./util/src &&  $(MAKE) clean
endif
# prepare the rootfs
rootfs_prep:
	@echo prepare the rootfs
	cd $(BUILDPATH)
	test -d $(INSTALL_ROOT) || mkdir $(INSTALL_ROOT)
	test -d lib || mkdir lib
	sudo rm -rf lib/* 
	test -d lib/lib || mkdir lib/lib
	test -d lib/bin || mkdir lib/bin
	test -d lib/usr || mkdir lib/usr

ifeq ($(PRODUCT_TYPE), VOX30)
	chmod 755 ../rootfs/VOX30/etc/tcpdump.sh
else
	chmod 755 ../rootfs/VD725/etc/tcpdump.sh
endif
ifeq ($(CONFIG_SUPPORT_5G_QD), 1)
	test -d lib/wlan || mkdir lib/wlan
endif
	test -d lib/usr/lib || mkdir lib/usr/lib
ifneq ($(GPL), 1)
	sudo rm -rf $(INSTALL_ROOT)/* && sudo rm -rf rootfs
else
	sudo rm -rf $(INSTALL_ROOT)/*
endif
ifneq ($(GPL), 1)
ifeq ($(PRODUCT_TYPE), VOX30)
ifeq ($(BRCM_502), 1)
	tar -zxf ../rootfs/common_502.arm.tgz
ifeq ($(CONFIG_SUPPORT_PLUME), 1)
	mkdir -p plume
	tar -zxf ../rootfs/plume.tgz -C plume
	cp -Rf plume/* $(INSTALL_ROOT) && sudo rm -rf plume
endif
else
	tar -zxf ../rootfs/common.arm.tgz
endif
else
ifeq ($(BRCM_416), 1)
ifeq ($(CONFIG_SUPPORT_PLUME), 1)
	tar -zxf ../rootfs/common_plume.tgz
else
	tar -zxf ../rootfs/common_416.tgz
endif
else
	tar -zxf ../rootfs/common.tgz
endif
endif
	cp -Rf rootfs/* $(INSTALL_ROOT) && sudo rm -rf rootfs
else
	tar -xvf ../rootfs/common_plume.tgz
	cp -Rf rootfs/* $(INSTALL_ROOT) #&& sudo rm -rf rootfs
ifeq ($(BRCM_416), 1)
	find $(TOPDIR)/toolchain/brcm_toolchain_416/src/buildroot-2011.11/output/toolchain/gcc-4.6.2-final/gcc/ -name *.so* ! -type d | xargs -i cp -Rf {} $(TOPDIR)/.rootfs/lib
	find $(TOPDIR)/toolchain/brcm_toolchain_416/src/buildroot-2011.11/output/toolchain/gcc-4.6.2-final/arm-unknown-linux-uclibcgnueabi/libgcc/ -name *.so* ! -type d | xargs -i cp -Rf {} $(TOPDIR)/.rootfs/lib
	find $(TOPDIR)/toolchain/brcm_toolchain_416/src/buildroot-2011.11/output/toolchain/gcc-4.6.2-final/arm-unknown-linux-uclibcgnueabi/libmudflap/.libs/ -name *.so* ! -type d | xargs -i cp -Rf {} $(TOPDIR)/.rootfs/lib
	find $(TOPDIR)/toolchain/brcm_toolchain_416/src/buildroot-2011.11/output/toolchain/gcc-4.6.2-final/arm-unknown-linux-uclibcgnueabi/libstdc++-v3/src/.libs/ -name *.so* ! -type d | xargs -i cp -Rf {} $(TOPDIR)/.rootfs/lib
	find $(TOPDIR)/toolchain/brcm_toolchain_416/src/buildroot-2011.11/output/toolchain/uClibc-0.9.32/lib/ -name *.so* ! -type d | xargs -i cp -Rf {} $(TOPDIR)/.rootfs/lib
else
	find /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/arm-buildroot-linux-gnueabi/sysroot/ -name *.so* ! -type d | xargs -i cp -Rf {} $(TOPDIR)/.rootfs/lib
	ln -sf libm.so.6 $(TOPDIR)/.rootfs/lib/libm.so
endif
endif
#	-cp -Rf lib_prep/* lib && sudo rm -rf lib_prep
	cp -Rf ../rootfs/$(PRODUCT_TYPE)/* $(INSTALL_ROOT)
	test -d $(INSTALL_ROOT)/usr/local || mkdir $(INSTALL_ROOT)/usr/local
	test -d $(INSTALL_ROOT)/etc/adsl || mkdir $(INSTALL_ROOT)/etc/adsl
	test -d $(INSTALL_ROOT)/etc/wlan || mkdir $(INSTALL_ROOT)/etc/wlan
ifeq ($(GPL), 1)
#	tar -xvf ../rootfs/common_502.arm.tgz
	cp -Rf rootfs/* $(INSTALL_ROOT) #&& sudo rm -rf rootfs
	cp -aRf $(TOPDIR)/.rootfs/* $(INSTALL_ROOT)/
#	cp -aRf $(TOPDIR)/.PACKAGE/* $(PACKAGE_ROOT)/
endif
ifeq ($(CONFIG_SUPPORT_5G_QD), 1)
ifeq ($(CONFIG_SUPPORT_DEBUG), 1)
	sudo rm -rf $(INSTALL_ROOT)/etc/wlan/topaz-linux.lzma.img
	sudo rm -rf $(INSTALL_ROOT)/etc/wlan/u-boot.bin
	sudo mv $(INSTALL_ROOT)/etc/wlan/topaz-linux.lzma_debug.img $(INSTALL_ROOT)/etc/wlan/topaz-linux.lzma.img
	sudo mv $(INSTALL_ROOT)/etc/wlan/u-boot_debug.bin $(INSTALL_ROOT)/etc/wlan/u-boot.bin
else
	sudo rm -rf $(INSTALL_ROOT)/etc/wlan/topaz-linux.lzma_debug.img
	sudo rm -rf $(INSTALL_ROOT)/etc/wlan/u-boot_debug.bin
endif
endif

	sudo ln -sf /var/passwd  $(INSTALL_ROOT)/etc/passwd
	sudo ln -sf /var/resolv.conf  $(INSTALL_ROOT)/etc/resolv.conf
	sudo ln -sf /var/bxxxd/ftpdpassword  $(INSTALL_ROOT)/etc/ftpdpassword
	sudo ln -sf /var/bxxxd/bftpd.conf  $(INSTALL_ROOT)/etc/bftpd.conf
	sudo ln -sf /tmp/gateways $(INSTALL_ROOT)/etc/gateways
	test -d $(INSTALL_ROOT)/lib/modules || mkdir $(INSTALL_ROOT)/lib/modules
ifeq ($(VOX30_SFP), 1)
ifeq ($(BRCM_502), 1)
	sudo cp $(INSTALL_ROOT)/etc/rcS.502 $(INSTALL_ROOT)/etc/rcS
else
	sudo cp $(INSTALL_ROOT)/etc/rcS.SFP $(INSTALL_ROOT)/etc/rcS
endif    
endif

pre_build:
ifneq ($(GPL),1)
	make update -C $(TOPDIR)/drivers/VoIP/
endif
ifneq ($(BRCM_502), 1)
	test -d $(BSP_DIR) ||  svn co http://172.21.67.236/svn/VOX3.0/branches/sc_bcm963xx_4.16L05 $(BSP_DIR)
endif

# kernel build
kernel_build:
	@echo Making Kernel Build
	cd $(TOPDIR); test -d linux || ln -sf $(KERNEL_DIR) $(TOPDIR)/linux
	@echo "PROFILE=963138GW_VOX_IPV6" > $(BSP_DIR)/PROFILE.make 
ifeq ($(VOX30_SFP), 1)
	@echo "PROFILE=963138GWV_VOX_IPV6" > $(BSP_DIR)/PROFILE.make 
endif
	@echo "IMP=14" >> $(BSP_DIR)/PROFILE.make 
ifeq ($(GPL),1)
	cd $(BSP_DIR)/hostTools/ && $(MAKE)
endif
#	@cd $(BSP_DIR); make sc_kernel;
	@echo Making Kernel Build Done

uimage:	kernel_build
	@echo making uImage
#	cd $(KERNEL_DIR); ./mkimg;
ifeq ($(ROOTFSTYPE), ubi)
	cd $(KERNEL_DIR); cp uImage $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi;
endif
	@echo making uImage Done

kernel_clean:
	@echo making kernel clean
	@cd $(BSP_DIR); make clean;
	cd $(KERNEL_DIR) &&  rm -f .config uImage vmlinux.bin.gz Module.symvers
	cd $(BUILDPATH) && rm -f uImage
	cd $(TOPDIR);rm -rf linux
ifeq ($(CONFIG_BRCM_SUPPORT), 1)
libs_build:
	@echo Makeing bcm libs
ifneq ($(GPL), 1)
	cd $(BSP_DIR); make sc_libs SC_BUILD=1
else
	cd $(BSP_DIR); make cms_sc SC_BUILD=1
endif
endif
user_build:
	@echo Makeing user
	cd $(TOPDIR)/user && $(MAKE) -s 

#Denny add only for quickly make cwmp
cwmp:
	@echo Makeing cwmp	
	$(MAKE) -s -C $(TOPDIR)/user/private/apps/mini_httpd-1.17beta1

user_clean:
	@echo Clean user
	make rootfs_clean
	cd $(TOPDIR)/user && $(MAKE) clean

private_clean:
	make rootfs_clean
	cd $(TOPDIR)/user/private/ && $(MAKE) clean

public_clean:
	make rootfs_clean
	cd $(TOPDIR)/user/public/ && $(MAKE) clean
#-------------------------------------------------------------
bgm:
	sudo rm -rf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/.pid 
	sudo ln -sf $(PID) $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/.pid 
	cd $(TOPDIR)/user/public/apps/privoxy && $(MAKE)
bgm_clean:
	cd $(TOPDIR)/user/private/apps/cml && $(MAKE) 
bb:
	 cd $(TOPDIR)/user/public/apps/ppp && $(MAKE) 
networkmap:
	 cd $(TOPDIR)/user/private/apps/networkmap_module && $(MAKE)
bb_clean:
	cd $(TOPDIR)/user/public/apps/miniupnpd && $(MAKE) 

strace:
	cd $(TOPDIR)/user/public/libs && $(MAKE)
rc_clean:
	cd $(TOPDIR)/user/private/apps/rc && $(MAKE) clean

www:
	cd $(TOPDIR)/user/public/libs/openssl && $(MAKE)
www_clean:
	cd $(TOPDIR)/user/private/apps/www && $(MAKE) clean

libx:
	cd $(TOPDIR)/user/private/libs/libutility && $(MAKE)
libx_clean:
	cd $(TOPDIR)/user/private/libs/libutility && $(MAKE) clean
#-----------------------------------------------------------------------
drivers_build:
	@echo Makeing drivers
ifneq ($(GPL),1)
	cd $(BSP_DIR); make sc_driver SC_BUILD=1
	cd $(TOPDIR)/drivers && $(MAKE)
else
	cd $(BSP_DIR); make sc_gpl_driver SC_BUILD=1
endif

drivers_clean:
	@echo Clean drivers
	make rootfs_clean
	cd $(TOPDIR)/drivers && $(MAKE) clean

voip_build:
	@echo Makeing voip
	cd $(TOPDIR)/drivers/VoIP && $(MAKE)


voip_clean:
	make rootfs_clean
	cd $(TOPDIR)/drivers/VoIP && $(MAKE) clean

rootfs_build:
	@echo make root file system

ifeq ($(CONFIG_SUPPORT_DEBUG), 1)
	@echo backup file system
	tar -zcf $(INSTALL_ROOT).tar.bz2 $(INSTALL_ROOT)/
endif
	-$(STRIP) -s $(INSTALL_ROOT)/usr/sbin/*
	-$(STRIP) -s $(INSTALL_ROOT)/lib/*
	-$(STRIP) -s $(INSTALL_ROOT)/bin/*
	cd $(BUILDPATH)
	chmod 755 do_link
	./do_link
	find rootfs.build/ -name .svn | xargs rm -rf
	find rootfs.build/ -name .cvsignore | xargs rm -rf
	find rootfs.build/ -name CVS | xargs rm -rf
	find rootfs.build/ -name Makefile | xargs rm -rf
	find rootfs.build/ -name _desktop.ini | xargs rm -rf
	find rootfs.build/ -name *.ko | xargs $(STRIP) -d
	echo "$(FW_BOOT_VERSION)" > rootfs.build/etc/boot_version
	echo "$(FW_EXTRA_VERSION) (build @ `export LC_TIME=C; date -u +'%F, %T'`)" > rootfs.build/etc/extra_version
	echo "$(SC_MODULE)" > rootfs.build/etc/hw_id
	echo "$(SC_MODULE_RF_TAG)" > rootfs.build/etc/hw_id_rf_tag
	echo "$(SC_MANUFACTOR)" > rootfs.build/etc/oem_id
	echo "${SC_ROOTFS_LIBS_VER}" > lib/lib_ver
	echo "${SC_PACKAGE_VER}" > PACKAGE/package_ver

	sudo mv $(INSTALL_ROOT)/etc/wlan/bcm43602_nvramvars.bin $(INSTALL_ROOT)/etc/wlan/bcm43602_nvramvars.bak.bin
	sudo ln -sf /tmp/bcm43602_nvramvars.bin $(INSTALL_ROOT)/etc/wlan/bcm43602_nvramvars.bin
	sudo mv $(INSTALL_ROOT)/etc/wlan/bcm43664_nvramvars.bin $(INSTALL_ROOT)/etc/wlan/bcm43664_nvramvars.bak.bin
	sudo ln -sf /tmp/bcm43664_nvramvars.bin $(INSTALL_ROOT)/etc/wlan/bcm43664_nvramvars.bin

ifeq ($(CUSTOMER_LOGO), $(wildcard $(CUSTOMER_LOGO)))
endif
	echo "$(SC_OEM).$(SC_CUSTOMER)" > rootfs.build/etc/build_tag
	sudo rm -rf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/.pid 
	sudo ln -sf $(PID) $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/.pid 
ifeq ($(SC_CUSTOMER), VFIN)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfinram.jffs2.img $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_vfin_rom.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_rom_128m.bin
else
#ifeq ($(SC_CUSTOMER), NEUTRAL)
#	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
#	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_vfes_rom.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_rom_128m.bin
#else
#ifeq ($(SC_CUSTOMER), VFPT)
#	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfinram.jffs2.img $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
#	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_vfin_rom.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_rom_128m.bin
#else

ifeq ($(BRCM_502), 1)
ifeq ($(PRODUCT_TYPE), VD300)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img_debug.256 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
else
ifeq ($(SC_CUSTOMER), VFIE)
ifeq ($(CONFIG_SUPPORT_DEBUG), 1)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfieram.jffs2.img_debug.502 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
else
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfieram.jffs2.img.502 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
endif
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_nvram_vfie.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_nvram.bin
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_sec_vfie.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_sec.bin
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img.mt.VFIT $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs.jffs2_416.mt.ie $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs.jffs2_416.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs_lib.jffs2_416.mt.ie $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs_lib.jffs2_416.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/kfs_info.mt.ie $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/kfs_info.mt.tmp
else
ifeq ($(CONFIG_SUPPORT_DEBUG), 1)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img_debug.502 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
else
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img.502 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
endif
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_nvram_vfit.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_nvram.bin
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_sec_vfit.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_sec.bin
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img.mt.VFIT $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs.jffs2_416.mt $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs.jffs2_416.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs_lib.jffs2_416.mt $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs_lib.jffs2_416.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/kfs_info.mt $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/kfs_info.mt.tmp
endif
endif
else
ifeq ($(BRCM_416), 1)
ifeq ($(SC_CUSTOMER), LOWI)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfelowiram.jffs2.img.416 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
else
ifeq ($(SC_CUSTOMER)_$(SC_CUSTOMER1), DT_TEST_LOWI)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfelowiram.jffs2.img.416 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
else
ifeq ($(VOX30_SFP), 1)
ifeq ($(SC_CUSTOMER), VFIE)
ifeq ($(CONFIG_SUPPORT_DEBUG), 1)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfieram.jffs2.img_debug.256 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
else
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfieram.jffs2.img.256 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
endif
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_nvram_vfie.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_nvram.bin
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_sec_vfie.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_sec.bin
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img.mt.VFIT $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs.jffs2_416.mt.ie $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs.jffs2_416.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs_lib.jffs2_416.mt.ie $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs_lib.jffs2_416.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/kfs_info.mt.ie $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/kfs_info.mt.tmp
else
ifeq ($(SC_NAND_FLASH_SIZE), 256)
ifeq ($(CONFIG_SUPPORT_DEBUG), 1)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img_debug.256 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
else
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img.256 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
endif
else
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img.416.SFP $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
endif
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_nvram_vfit.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_nvram.bin
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_sec_vfit.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe63138rom_sec.bin
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img.mt.VFIT $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs.jffs2_416.mt $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs.jffs2_416.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs_lib.jffs2_416.mt $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/rootfs_lib.jffs2_416.mt.tmp
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/kfs_info.mt $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/kfs_info.mt.tmp
endif
else
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img.416 $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
endif
endif
endif
else
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfevfesram.jffs2.img $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cferam.jffs2.img
endif
ifeq ($(SC_CUSTOMER), LOWI)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_lowi_rom.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_rom_128m.bin
else
ifeq ($(SC_CUSTOMER)_$(SC_CUSTOMER1), DT_TEST_LOWI)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_lowi_rom.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_rom_128m.bin
else
ifeq ($(VOX30_SFP), 1)
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_vfes_rom.bin.SFP $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_rom_128m.bin
else
	sudo ln -sf $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_vfes_rom.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/cfe_rom_128m.bin
endif
endif
endif
endif
#endif
#endif
endif
	cp $(PID) $(INSTALL_ROOT)/etc/pid
	cp -fv $(BSP_DIR)/vmlinux.lz $(INSTALL_ROOT)/
ifeq ($(ROOTFSTYPE), squash)
	./util/bin/sc_mksquashfs rootfs.build rootfs -be -root-owned -noappend -D product/$(PRODUCT_TYPE)/devTable
endif
ifeq ($(ROOTFSTYPE), jffs2)
ifeq ($(BRCM_502), 1)
	sudo $(BUILDPATH)/product/$(PRODUCT_TYPE)/makeDevs.502 $(KERNEL_DIR) $(INSTALLDIR)
else
	sudo $(BUILDPATH)/product/$(PRODUCT_TYPE)/makeDevs $(KERNEL_DIR) $(INSTALLDIR)
endif    
	echo -e "/vmlinux.lz" > $(INSTALL_ROOT)/nocomprlist
	echo -e "/kernel_cksum" >> $(INSTALL_ROOT)/nocomprlist
ifeq ($(BRCM_502), 1)
	echo -e "/963138.dtb" >> $(INSTALL_ROOT)/nocomprlist
endif
	echo -e "/cferam" > $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/nocomprlist
	echo -e "/cferam_bk" >> $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/nocomprlist
	sudo chown -R 0:0 $(INSTALL_ROOT)
ifeq ($(SC_NAND_FLASH_SIZE), $(findstring ${SC_NAND_FLASH_SIZE}, 128 256))
	-@echo -e '\E[32;40m'"Making jffs2 filesystem for ${SC_NAND_FLASH_SIZE}M flash" && tput sgr0
	cd $(BUILDPATH)/APP_DATA && find .svn|xargs rm -rf && cd -
	cd $(BUILDPATH)/EMPTY && find .svn|xargs rm -rf && cd -
ifeq ($(SC_NAND_FLASH_SIZE), 256)
	cd $(BUILDPATH)/PACKAGE && find .svn|xargs rm -rf && cd -
endif
	sudo $(HOSTTOOLS_DIR)/mkfs.jffs2 -l -p -n -e 131072 -r $(INSTALL_ROOT) -o $(BUILDPATH)/rootfs.jffs2 -N $(INSTALL_ROOT)/nocomprlist
	sudo $(HOSTTOOLS_DIR)/mkfs.jffs2 -l -p -n -e 131072 -r $(BUILDPATH)/lib -o $(BUILDPATH)/rootfs_lib.jffs2 -N $(INSTALL_ROOT)/nocomprlist
	sudo $(HOSTTOOLS_DIR)/mkfs.jffs2 -l -p -n -e 131072 -r $(BUILDPATH)/EMPTY -o $(BUILDPATH)/empty.jffs2 -N $(INSTALL_ROOT)/nocomprlist
	sudo $(HOSTTOOLS_DIR)/mkfs.jffs2 -l -p -n -e 131072 -r $(BUILDPATH)/APP_DATA -o $(BUILDPATH)/app_dat.jffs2 -N $(INSTALL_ROOT)/nocomprlist
ifeq ($(SC_NAND_FLASH_SIZE), 256)
ifeq ($(PRODUCT_TYPE), VOX30)
ifeq ($(shell cat ./product/VOX30/jffs2/.pid | cut -b 101-102), 35)
	sudo $(HOSTTOOLS_DIR)/mkfs.jffs2 -l -p -n -e 131072 -r $(BUILDPATH)/PACKAGE -o $(BUILDPATH)/package.jffs2 -N $(INSTALL_ROOT)/nocomprlist
else
	sudo /usr/sbin/mkfs.ubifs -m 2048 -e 126976 -c 640 -x "zlib" -r $(BUILDPATH)/PACKAGE $(BUILDPATH)/package.img -U -F
	sudo /usr/sbin/ubinize -o $(BUILDPATH)/package.jffs2 -m 2048 -p 128KiB -s 2048  $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/package.ini
endif
else
ifeq ($(PRODUCT_TYPE), VD725)
ifeq ($(shell cat ./product/VD725/jffs2/.pid | cut -b 101-102), 35)
	sudo $(HOSTTOOLS_DIR)/mkfs.jffs2 -l -p -n -e 131072 -r $(BUILDPATH)/PACKAGE -o $(BUILDPATH)/package.jffs2 -N $(INSTALL_ROOT)/nocomprlist
else
	sudo /usr/sbin/mkfs.ubifs -m 2048 -e 126976 -c 640 -x "zlib" -r $(BUILDPATH)/PACKAGE $(BUILDPATH)/package.img -U -F
	sudo /usr/sbin/ubinize -o $(BUILDPATH)/package.jffs2 -m 2048 -p 128KiB -s 2048  $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/package.ini
endif
else
	sudo /usr/sbin/mkfs.ubifs -m 2048 -e 126976 -c 640 -x "zlib" -r $(BUILDPATH)/PACKAGE $(BUILDPATH)/package.img -U -F
	sudo /usr/sbin/ubinize -o $(BUILDPATH)/package.jffs2 -m 2048 -p 128KiB -s 2048  $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/package.ini
endif
endif
#sudo $(HOSTTOOLS_DIR)/mkfs.jffs2 -l -p -n -e 131072 -r $(BUILDPATH)/PACKAGE -o $(BUILDPATH)/package.jffs2 -N $(INSTALL_ROOT)/nocomprlist
endif
endif
ifeq ($(CONFIG_SUPPORT_DT_TEST), 1)
	cp $(BUILDPATH)/rootfs.jffs2 $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/rootfs_fb.jffs2 -v
endif
	cp $(BUILDPATH)/rootfs.jffs2 $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/ -v
	cp $(BUILDPATH)/rootfs_lib.jffs2 $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/ -v
	cp $(BUILDPATH)/empty.jffs2 $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/xml_cfg.jffs2 -v
	cp $(BUILDPATH)/app_dat.jffs2 $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/app_dat.jffs2 -v
	cp $(BUILDPATH)/empty.jffs2 $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/scnvram.jffs2 -v
ifeq ($(SC_NAND_FLASH_SIZE), 256)
	cp $(BUILDPATH)/package.jffs2 $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/package.jffs2 -v
endif
ifeq ($(SC_CUSTOMER), LOWI)
	cp $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/rootfs.jffs2.mt.LOWI $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/rootfs.jffs2.mt -v
	cp $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/rootfs_lib.jffs2.mt.LOWI $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/rootfs_lib.jffs2.mt -v
	cp $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/bootflag.mt.LOWI $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/bootflag.mt -v
endif
	sudo rm -fv $(INSTALL_ROOT)/nocomprlist
	rm -fv $(BUILDPATH)/empty.jffs2
	rm -fv $(BUILDPATH)/rootfs.jffs2
	rm -fv $(BUILDPATH)/rootfs_lib.jffs2
	rm -fv $(BUILDPATH)/app_dat.jffs2
ifeq ($(SC_NAND_FLASH_SIZE), 256)
	rm -fv $(BUILDPATH)/package.jffs2
endif
endif

ifeq ($(ROOTFSTYPE), ubi)
ifeq ($(SC_NAND_FLASH_SIZE), 128)
	./util/bin/mtd-utils/i386/sbin/mkfs.ubifs -m 2048 -e 129024 -c 160 -x "zlib" -r $(INSTALL_ROOT)/ -o product/$(PRODUCT_TYPE)/ubi/rootfs.img -D product/$(PRODUCT_TYPE)/devTable -U
	./util/bin/mtd-utils/i386/sbin/mkfs.ubifs -m 2048 -e 129024 -c 160 -x "zlib" -r lib/ -o product/$(PRODUCT_TYPE)/ubi/lib.img -U
else
	./util/bin/mtd-utils/i386/sbin/mkfs.ubifs -m 2048 -e 129024 -c 160 -x "zlib" -r $(INSTALL_ROOT)/ -o product/$(PRODUCT_TYPE)/ubi/rootfs.img -D product/$(PRODUCT_TYPE)/devTable -U
endif
	-rm -rf product/$(PRODUCT_TYPE)/ubi/xml
	-rm -rf product/$(PRODUCT_TYPE)/ubi/xml.img
	-mkdir product/$(PRODUCT_TYPE)/ubi/xml
	./util/bin/mtd-utils/i386/sbin/mkfs.ubifs -m 2048 -e 129024 -c 80 -x "zlib" -r product/$(PRODUCT_TYPE)/ubi/xml -o product/$(PRODUCT_TYPE)/ubi/xml.img 
endif
rootfs_clean: image_clean
	@echo Clean rootfs
	cd $(BSP_DIR) && rm -f $(BSP_DIR)/vmlinux.lz
ifeq ($(ROOTFSTYPE), ubi)
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && rm -f *.ubi uImage rootfs.img xml.img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ && rm -f *.img *.bin
endif
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(ROOTFSTYPE)/ && rm -f cferam.jffs2.img cfe_rom_128m.bin
	sudo rm -rf $(INSTALL_ROOT)
	rm -rf PACKAGE/* ipk

image_build:rootfs_build
	@echo making image
ifneq ($(GPL), 1)
	$(BUILDPATH)/util/bin/SeDMConvert -i $(PRIVATE_APPSPATH)/apps/default/tmp_default.xml -o  $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(FW_VERSION_PREFIX)_$(FW_VERSION).xml -d $(SC_MODULE)$(SC_MODULE_RF_TAG) -p $(SC_MODULE)$(SC_MODULE_RF_TAG) -u $(SC_OEM_OUI) -m $(SC_MANUFACTOR)   
	rm $(PRIVATE_APPSPATH)/apps/default/tmp_default.xml
endif
ifeq ($(ROOTFSTYPE), jffs2)
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2 && ./bb_416 $(FW_VERSION_PREFIX)_$(FW_VERSION) $(SC_CUSTOMER)
ifeq ($(BURN), 1)
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2 && mv $(FW_VERSION_PREFIX)_$(FW_VERSION)_burn.bin $(BUILDPATH)/product/$(PRODUCT_TYPE)/
endif
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2 && mv $(FW_VERSION_PREFIX)_$(FW_VERSION)_all.img $(BUILDPATH)/product/$(PRODUCT_TYPE)/ 
endif

ifeq ($(ROOTFSTYPE), ubi)
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/mtd-utils/i386/sbin/ubinize -o uImage.ubi -m 2048 -p 128KiB -s 512 $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/linux.ini
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/mtd-utils/i386/sbin/ubinize -o xml.ubi -m 2048 -p 128KiB -s 512 $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/xml.ini
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/mtd-utils/i386/sbin/ubinize -o fs.ubi -m 2048 -p 128KiB -s 512  $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/rootfs.ini
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/mtd-utils/i386/sbin/ubinize -o fs_libs.ubi -m 2048 -p 128KiB -s 512  $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/lib.ini
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && cp -rf uboot.ubi.original uboot.ubi

ifeq ($(SC_NAND_FLASH_SIZE), 128)
ifeq ($(SC_BL_DEMO_BOARD_SUPPORT), 1)
	@echo "Build FW for BL demo board."
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/scBind -i $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/_128M_DemoBoard.par -o  $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION)_DemoBoard.bin
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg_DemoBoard -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION)_DemoBoard.img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg_128_libs -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_EXTRA_VERSION)_libs.img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg_128_full -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_EXTRA_VERSION)_full.img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg_128_all -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_EXTRA_VERSION)_all.img
else
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/scBind -i $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/_128M.par -o  $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION).bin
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg_128 -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION).img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg_128_libs -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION)_libs.img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg_128_full -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION)_full.img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg_128_all -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION)_all.img
endif
else
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/scBind -i $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/.par -o  $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION).bin
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && $(BUILDPATH)/util/bin/sc_mkimg -c $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/imgBindcfg -o $(BUILDPATH)/product/$(PRODUCT_TYPE)/$(SC_MODULE)_$(SC_OEM)_$(SC_CUSTOMER)_$(FW_VERSION).img
endif

endif

image_clean:
	@echo Clean image
ifeq ($(ROOTFSTYPE), jffs2)
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2 && rm -f *.jffs2 uImage rootfs.img xml.img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ && rm -f *.img *.bin *.xml *.jffs2
endif

ifeq ($(ROOTFSTYPE), ubi)
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi && rm -f *.ubi uImage rootfs.img xml.img
	cd $(BUILDPATH)/product/$(PRODUCT_TYPE)/ && rm -f *.img *.bin *.xml
endif
# do clean
clean:
	make rootfs_clean
	make user_clean
	make drivers_clean
	make image_clean
	make kernel_clean
	make host_clean
libfwutil:
	make -C $(TOPDIR)/user/private/libs/libfwutil
libfwutil_clean:
	make -C $(TOPDIR)/user/private/libs/libfwutil clean
# do cvs clean
cvsclean:
	find $(TOPDIR)/ -name CVS | xargs rm -rf
	find $(TOPDIR)/ -name .cvsignore | xargs rm -rf
	find $(TOPDIR)/ -name .svn | xargs rm -rf
dhd_build:
	test -d /projects/ || (test -f ../toolchain/brcm_toolchain_502/dhd_tools.tgz || (mkdir -p ../toolchain && mkdir -p ../toolchain/brcm_toolchain_502 && svn co http://172.21.67.236/svn/VOX3.0/toolchain/brcm_toolchain_502/ ../toolchain/brcm_toolchain_502/))
	test -d /projects/ || sudo tar -xvf ../toolchain/brcm_toolchain_502/dhd_tools.tgz -C /
	cd $(BSP_DIR); make dhd_build SC_BUILD=1

release:
	./do_release
SDK_VER=4.0.00.07
sdk:
	cd $(TOPDIR) && ./do_customer_release Sercomm_VOX3.0_SDK $(SDK_VER)
