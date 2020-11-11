
# -----------------------------------------------------------------|
# sercomm add
# -----------------------------------------------------------------|

# Setup sc build options.
ifeq ($(SC_BUILD),1)
export BUILD_WLCTL_SHLIB=1
export SC_BUILD=1
export WLAN_StdSrcDirsA=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/shared/
export WLAN_ComponentSrcDirs=$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/shared/bcmwifi/src/
export WLAN_StdIncPathA=-I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/include
export WLAN_ComponentIncPath=-I$(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/shared/bcmwifi/include
endif

help:
	@echo "make help      --> show usage, sercomm modified"
	@echo "make sc_kernel --> build vmlinux.lz"
	@echo "make clean     --> clean kernel"
	@echo "make sc_driver --> install drivers"      
	

sc_kernel:  vmlinux.lz_only  
	@echo -e "\\e[36m -- make kernel done --\e[0m"	
	@echo

build_gpl:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/nas build_gpl
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/acsd build_gpl
ifeq ($(BRCM_DRIVER_DHD), m)
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/wl/exe build_gpl
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dhd/exe build_gpl
	install -d $(GPL_INSTALLDIR)/etc/wlan/dhd
	install -d $(GPL_INSTALLDIR)/etc/wlan/dhd/43602a1
	install -d $(GPL_INSTALLDIR)/etc/wlan/dhd/43602a3 
	install -d $(GPL_INSTALLDIR)/etc/wlan/dhd/mfg/43602a1
	install -d $(GPL_INSTALLDIR)/etc/wlan/dhd/mfg/43602a3 
	install -d $(GPL_INSTALLDIR)/etc/wlan/dhd/4366c0
	install -d $(GPL_INSTALLDIR)/etc/wlan/dhd/mfg/4366c0
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/43602a1/rtecdc.bin $(GPL_INSTALLDIR)/etc/wlan/dhd/43602a1
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/43602a3/rtecdc.bin $(GPL_INSTALLDIR)/etc/wlan/dhd/43602a3
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/mfg/43602a1/rtecdc.bin $(GPL_INSTALLDIR)/etc/wlan/dhd/mfg/43602a1/
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/mfg/43602a3/rtecdc.bin $(GPL_INSTALLDIR)/etc/wlan/dhd/mfg/43602a3/
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/4366c0/rtecdc.bin $(GPL_INSTALLDIR)/etc/wlan/dhd/4366c0/
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/mfg/4366c0/rtecdc.bin $(GPL_INSTALLDIR)/etc/wlan/dhd/mfg/4366c0/
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/4366c0/rtecdc.bin $(GPL_INSTALLDIR)/etc/wlan/dhd/4366c0/
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/mfg/4366c0/rtecdc.bin $(GPL_INSTALLDIR)/etc/wlan/dhd/mfg/4366c0/
	install -m 755 $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dongle/nvram.txt $(GPL_INSTALLDIR)/etc/wlan/dhd/nvram.txt
else
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/wl/exe build_gpl
endif
#	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/wl/exe build_gpl
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/wps build_gpl
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/libupnp build_gpl
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/eapd/linux build_gpl -f Makefile.dslcpe
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/bsd build_gpl
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/toad build_gpl
#	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/router/igd build_gpl
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/lltd build_gpl
#	sudo rm -rf $(INSTALLDIR)/etc/wlan/bcm43602_nvramvars.bin
#	sudo rm -rf $(INSTALLDIR)/etc/wlan/bcm43664_nvramvars.bin
#	sudo mv $(INSTALLDIR)/etc/wlan/bcm43602_nvramvars.bak.bin $(INSTALLDIR)/etc/wlan/bcm43602_nvramvars.bin
#	sudo mv $(INSTALLDIR)/etc/wlan/bcm43664_nvramvars.bak.bin $(INSTALLDIR)/etc/wlan/bcm43664_nvramvars.bin
	cp -f $(INSTALLDIR)/etc/wlan/*.bin $(GPL_INSTALLDIR)/etc/wlan


ifeq ($(CONFIG_SUPPORT_DSL),1)
	cp  $(BRCMDRIVERS_DIR)/broadcom/char/adsl/bcm9$(BRCM_CHIP)/adsl_phy.bin $(GPL_INSTALLDIR)/etc/adsl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/atmctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/xdslctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/dsldiagd build_gpl
endif
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/vlanctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/vlanctl build_gpl
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/spuctl build_gpl
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/spuctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/ethctl build_gpl
ifneq ($(strip $(BUILD_SWMDK)),)
	$(MAKE) -j 1 -C $(BUILD_DIR)/userspace/private/libs/mdk build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/swmdk build_gpl
else
	@echo "Skipping switch mdk "
endif

ifeq ($(PRODUCT_TYPE), VOX25)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/fapctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/bpmctl build_gpl
endif
ifeq ($(PRODUCT_TYPE), ESSENTIAL)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/fapctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/bpmctl build_gpl
endif

	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/iqctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/iqctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/fcctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/fcctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/pwrctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/pwrctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/ethctl build_gpl
ifeq ($(SC_VOIP), 1)
	$(MAKE) -C $(BUILD_DIR)/../../VoIP/vgw build_gpl
endif
ifeq ($(CONFIG_SUPPORT_IPTV), 1)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/mcpd build_gpl
endif

ifeq ($(CONFIG_SUPPORT_DSL),1)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/xdslctl build_gpl
endif
ifeq ($(BUILD_RDPA), y)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/rdpactl build_gpl
endif
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/atmctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/ethswctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/ethswctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/tmctl build_gpl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/tmctl build_gpl
#	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/wl/exe build_gpl
ifeq ($(BRCM_DRIVER_DHD), m)
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/wl/exe build_gpl
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dhd/exe build_gpl
else
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/wl/exe build_gpl
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/pktflow/bcm9$(BRCM_CHIP)/pktflow.ko $(GPL_INSTALLDIR)/lib/modules

ifeq ($(BRCM_DRIVER_PKTRUNNER), m)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/pktrunner/bcm9$(BRCM_CHIP)/pktrunner.ko $(GPL_INSTALLDIR)/lib/modules
endif
ifeq ($(CONFIG_SUPPORT_DSL),1)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/xtmcfg/bcm9$(BRCM_CHIP)/bcmxtmcfg.ko $(GPL_INSTALLDIR)/lib/modules
endif
ifeq ($(strip $(BRCM_DRIVER_FAP)),m)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/fap/bcm9$(BRCM_CHIP)/bcmfap.ko $(GPL_INSTALLDIR)/lib/modules
endif
ifeq ($(CONFIG_SUPPORT_DSL),1)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/adsl/bcm9$(BRCM_CHIP)/adsldd.ko $(GPL_INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/vlan/bcm9$(BRCM_CHIP)/bcmvlan.ko $(GPL_INSTALLDIR)/lib/modules
ifeq ($(BUILD_RDPA), y)
	cp -af  $(BRCMDRIVERS_DIR)/../rdp/projects/DSL_$(BRCM_CHIP)/target/rdpa/rdpa.ko $(GPL_INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/chipinfo/bcm9$(BRCM_CHIP)/chipinfo.ko $(GPL_INSTALLDIR)/lib/modules
endif
ifeq ($(BRCM_DRIVER_DHD), m)

else
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/build/wlobj-wlconfig_lx_wl_dslcpe_pci_ap_2nv-kdb/wl.ko $(GPL_INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/emf/wlemf.ko $(GPL_INSTALLDIR)/lib/modules/
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/wlcsm_ext/bcm9$(BRCM_CHIP)/wlcsm.ko $(GPL_INSTALLDIR)/lib/modules/
#	cp -af $(BRCMDRIVERS_DIR)/opensource/net/enet/bcm9$(BRCM_CHIP)/bcm_enet.ko $(GPL_INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/otp/bcm9$(BRCM_CHIP)/otp.ko $(GPL_INSTALLDIR)/lib/modules
ifeq ($(BRCM_DRIVER_DHD), m)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/build/dhdobj-dhdconfig_lx_dhd_dslcpe_pci_ap_2nv-kdb/dhd.ko $(GPL_INSTALLDIR)/lib/modules
ifeq ($(VOX30_SFP), 1)

else
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/build/wlobj-wlconfig_lx_wl_dslcpe_pci_ap_2nv-kdb/wl.ko $(GPL_INSTALLDIR)/lib/modules
endif
else
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/build/wlobj-wlconfig_lx_wl_dslcpe_pci_ap_2nv-kdb/wl.ko $(GPL_INSTALLDIR)/lib/modules
endif
ifeq ($(BRCM_DRIVER_DHD), m)

else
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/build/wlobj-wlconfig_lx_wl_dslcpe_pci_ap_2nv-kdb/wl.ko $(GPL_INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/ingqos/bcm9$(BRCM_CHIP)/bcm_ingqos.ko $(GPL_INSTALLDIR)/lib/modules
	cp -af $(BUILD_DIR)/userspace/private/libs/wlcsm/libwlcsm.so $(GPL_INSTALLDIR)/lib
ifeq ($(CONFIG_BCM_ARL), m)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/arl/bcm9$(BRCM_CHIP)/bcmarl.ko $(GPL_INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/pwrmngt/bcm9$(BRCM_CHIP)/pwrmngtd.ko $(GPL_INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/tms/bcm9$(BRCM_CHIP)/nciTMSkmod.ko $(GPL_INSTALLDIR)/lib/modules
ifeq ($(SC_VOIP), 1)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/slicslac/bcm9$(BRCM_CHIP)/slicslac.ko $(GPL_INSTALLDIR)/lib/modules
endif
#	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/secosdrv/bcm9$(BRCM_CHIP)/secosdrv.ko $(GPL_INSTALLDIR)/lib/modules
# netfilter
#	cp -af $(KERNEL_DIR)/net/netfilter/*.ko $(GPL_INSTALLDIR)/lib/modules
#	cp -af $(KERNEL_DIR)/net/ipv4/netfilter/*.ko $(GPL_INSTALLDIR)/lib/modules
#	cp -af $(KERNEL_DIR)/net/ipv6/netfilter/*.ko $(GPL_INSTALLDIR)/lib/modules
#ifeq ($(CONFIG_SUPPORT_3G),1)
#	cp -af $(KERNEL_DIR)/drivers/usb/serial/*.ko $(GPL_INSTALLDIR)/lib/modules
#endif

ifneq ($(strip $(BRCM_VODSL_DECT)),)
#	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/dect/bcm9$(BRCM_CHIP)/dect.ko $(GPL_INSTALLDIR)/lib/modules
endif

sc_libs: data_model cms_sc wlcsm_lib wlctl_sc
ifeq ($(BUILD_RDPA), y)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/rdpactl sc
endif
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/ethswctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/ethctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/tmctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/xdslctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/atmctl sc

ifeq ($(CONFIG_SUPPORT_IPTV), 1)
#	$(MAKE) -C $(BUILD_DIR)/userspace/public/libs/cms_util sc 
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/snoopctl sc 
endif
data_model:
	$(MAKE) -C data-model
#-------------------------------------------
ifeq ($(BRCM_CHIP), 63268)
sc_driver:  wl_sc adsl_sc vlan_sc ethctl_sc mdk_sc  spu_sc fc_sc pwrctl_sc btrmcfg_sc bpmctl iqctl fapctl_sc sc_tmsctl snoopctl_sc iptv_sc sc_wlan dtbs dsp_ept
else
sc_driver:  cms_sc sc_wlan wl_sc adsl_sc vlan_sc sc_tmsctl ethctl_sc mdk_sc  spu_sc fc_sc pwrctl_sc btrmcfg_sc bpmctl iqctl snoopctl_sc iptv_sc dtbs dsp_ept sc_ubi sc_openvswitch
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/pktflow/bcm9$(BRCM_CHIP)/pktflow.ko $(INSTALLDIR)/lib/modules
ifeq ($(BRCM_DRIVER_BDMF), m)
	cp -af rdp/projects/DSL_63138/target/bdmf/bdmf.ko $(INSTALLDIR)/lib/modules
endif
ifeq ($(BUILD_RDPA), y)
	cp -af rdp/projects/DSL_63138/target/rdpa//rdpa.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/opensource/char/rdpa_drv/bcm9$(BRCM_CHIP)/rdpa_cmd.ko $(INSTALLDIR)/lib/modules
	cp -af rdp/projects/DSL_63138/target/rdpa_gpl/rdpa_gpl.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/opensource/char/rdpa_gpl_ext/bcm9$(BRCM_CHIP)/rdpa_gpl_ext.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/opensource/char/rdpa_mw/bcm9$(BRCM_CHIP)/rdpa_mw.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/chipinfo/bcm9$(BRCM_CHIP)/chipinfo.ko $(INSTALLDIR)/lib/modules
endif

ifeq ($(BRCM_DRIVER_PKTRUNNER), m)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/pktrunner/bcm9$(BRCM_CHIP)/pktrunner.ko $(INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/tms/bcm9$(BRCM_CHIP)/nciTMSkmod.ko $(INSTALLDIR)/lib/modules
	cp -af kernel/dts/9$(BRCM_CHIP).dtb $(INSTALLDIR)/
ifeq ($(CONFIG_SUPPORT_DSL),1)
	cp -af $(BRCMDRIVERS_DIR)/opensource/net/xtmrt/bcm9$(BRCM_CHIP)/bcmxtmrtdrv.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/xtmcfg/bcm9$(BRCM_CHIP)/bcmxtmcfg.ko $(INSTALLDIR)/lib/modules
endif
ifeq ($(strip $(BRCM_DRIVER_FAP)),m)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/fap/bcm9$(BRCM_CHIP)/bcmfap.ko $(INSTALLDIR)/lib/modules
endif
ifeq ($(CONFIG_SUPPORT_DSL),1)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/adsl/bcm9$(BRCM_CHIP)/adsldd.ko $(INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/vlan/bcm9$(BRCM_CHIP)/bcmvlan.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/opensource/net/enet/bcm9$(BRCM_CHIP)/bcm_enet.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/otp/bcm9$(BRCM_CHIP)/otp.ko $(INSTALLDIR)/lib/modules
ifeq ($(BRCM_DRIVER_DHD), m)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/build/dhdobj-dhdconfig_lx_dhd_dslcpe_pci_ap_2nv-kdb/dhd.ko $(INSTALLDIR)/lib/modules
ifeq ($(VOX30_SFP), 1)

else
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/build/wlobj-wlconfig_lx_wl_dslcpe_pci_ap_2nv-kdb/wl.ko $(INSTALLDIR)/lib/modules
endif
else
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/build/wlobj-wlconfig_lx_wl_dslcpe_pci_ap_2nv-kdb/wl.ko $(INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/emf/wlemf.ko $(INSTALLDIR)/lib/modules/
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/wlcsm_ext/bcm9$(BRCM_CHIP)/wlcsm.ko $(INSTALLDIR)/lib/modules/
	cp -af $(BRCMDRIVERS_DIR)/opensource/net/wfd/bcm9$(BRCM_CHIP)/wfd.ko $(INSTALLDIR)/lib/modules/
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/ingqos/bcm9$(BRCM_CHIP)/bcm_ingqos.ko $(INSTALLDIR)/lib/modules
#	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/bpm/bcm9$(BRCM_CHIP)/bcm_bpm.ko $(INSTALLDIR)/lib/modules

ifeq ($(CONFIG_SUPPORT_BANDSTEERING),1)
ifeq ($(CONFIG_SUPPORT_5G_QD),1)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/qwerpe/bcm9$(BRCM_CHIP)/qwerpe.ko $(INSTALLDIR)/lib/modules
endif
endif
ifeq ($(CONFIG_BCM_ARL), m)
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/arl/bcm9$(BRCM_CHIP)/bcmarl.ko $(INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/pwrmngt/bcm9$(BRCM_CHIP)/pwrmngtd.ko $(INSTALLDIR)/lib/modules
#	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/secosdrv/bcm9$(BRCM_CHIP)/secosdrv.ko $(INSTALLDIR)/lib/modules
# netfilter
	cp -af $(KERNEL_DIR)/net/netfilter/*.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/drivers/net/ppp/*.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/net/ipv4/netfilter/*.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/net/ipv4/*.ko $(INSTALLDIR)/lib/modules
ifeq ($(CONFIG_SUPPORT_IPV6),1)
	cp -af $(KERNEL_DIR)/net/ipv6/netfilter/*.ko $(INSTALLDIR)/lib/modules
endif
ifeq ($(CONFIG_SUPPORT_3G),1)
	cp -af $(KERNEL_DIR)/drivers/usb/serial/*.ko $(INSTALLDIR)/lib/modules
endif
ifeq ($(BRCM_CHIP), 63138)
	cp -af $(KERNEL_DIR)/drivers/usb/storage/*.ko $(INSTALLDIR)/lib/modules
	#cp -af $(KERNEL_DIR)/arch/arm/plat-bcm63xx/bcm63xx_usb.ko $(INSTALLDIR)/lib/modules
	#cp -af $(KERNEL_DIR)/drivers/usb/class/usblp.ko $(INSTALLDIR)/lib/modules
endif
ifneq ($(strip $(BRCM_VODSL_DECT)),)
#	cp -af $(BRCMDRIVERS_DIR)/broadcom/char/dect/bcm9$(BRCM_CHIP)/dect.ko $(INSTALLDIR)/lib/modules
endif

	echo "Stripping kernel modules..."
# Modules that need parameters cannot be stripped
	eval "find $(INSTALLDIR)/lib/modules -name '*.ko' ! -name 'ip*.ko' |xargs $(STRIP) --strip-unneeded"

	
vmlinux.lz_only : prebuild_checks profile_saved_check sanity_check rdp_link astra_build \
     create_install kernelbuild modbuild hosttools  vmlinux.lz_gen

vmlinux.lz_gen:
	@rm -f ./vmlinux
	@rm -f ./vmlinux.bin
	@rm -f ./vmlinux.lz
	cp $(KERNEL_DIR)/vmlinux . ; \
	$(KSTRIP) --remove-section=.note --remove-section=.comment vmlinux; \
	$(KOBJCOPY) -O binary vmlinux vmlinux.bin; \
	$(HOSTTOOLS_DIR)/cmplzma -k -2 -lzma vmlinux vmlinux.bin vmlinux.lz
west_debug:
	$(KSTRIP) --remove-section=.note --remove-section=.comment vmlinux; \
	$(KOBJCOPY) -O binary vmlinux vmlinux.bin; \
	$(HOSTTOOLS_DIR)/cmplzma -k -2 -lzma vmlinux vmlinux.bin vmlinux.lz

wlcsm_lib:
ifneq ($(GPL),1)
	$(MAKE) -C userspace/private/libs/wlcsm 
endif
ifeq ($(BRCM_USER_SSP), y)
#	$(MAKE) -C userspace/public/libs/brcmssp_util 
#	cp -af $(BUILD_DIR)/userspace/public/libs/brcmssp_util/libssp.so $(INSTALLDIR)/lib
endif
ifneq ($(GPL),1)
	cp -af $(BUILD_DIR)/userspace/private/libs/wlcsm/libwlcsm.so $(INSTALLDIR)/lib
endif


cms_sc: 
#	$(MAKE) -C $(PRIVATE_APPSPATH)/vendor/brcm/sc_cms clean
#	$(MAKE) -C $(PRIVATE_APPSPATH)/vendor/brcm/sc_cms
	$(MAKE) -C userspace/public/include/
ifneq ($(GPL),1)
#	$(MAKE) -C userspace/private/libs/cms_core
#	$(MAKE) -C userspace/private/libs/cms_cli
#	$(MAKE) -C userspace/private/libs/cms_dal
#	$(MAKE) -C userspace/private/libs/cms_qdm
endif
#	$(MAKE) -C userspace/public/libs/cms_msg
#	$(MAKE) -C userspace/public/libs/cms_util
#	$(MAKE) -C userspace/public/libs/cms_boardctl
#	$(MAKE) -C userspace/public/libs/bcm_flashutil/ 
#	$(MAKE) -C userspace/public/libs/bcm_util/ 
ifneq ($(GPL),1)
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/nanoxml
#	cp -af $(BUILD_DIR)/userspace/private/libs/nanoxml/libnanoxml.so $(INSTALLDIR)/lib
endif
#	cp userspace/public/libs/bcm_util/libbcm_crc.so $(INSTALLDIR)/lib
#	cp userspace/public/libs/bcm_flashutil/libbcm_flashutil.so $(INSTALLDIR)/lib
ifneq ($(GPL),1)
#	cp userspace/private/libs/cms_core/libcms_core.so $(INSTALLDIR)/lib
#	cp userspace/private/libs/cms_cli/libcms_cli.so $(INSTALLDIR)/lib
#	cp userspace/private/libs/cms_dal/libcms_dal.so $(INSTALLDIR)/lib
#	cp userspace/private/libs/cms_qdm/libcms_qdm.so $(INSTALLDIR)/lib
endif
#	cp userspace/public/libs/cms_msg/libcms_msg.so $(INSTALLDIR)/lib
#	cp userspace/public/libs/cms_util/libcms_util.so $(INSTALLDIR)/lib
#	cp userspace/public/libs/cms_boardctl/libcms_boardctl.so $(INSTALLDIR)/lib

nas_sc:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/ nas_sc -f Makefile.dslcpe
        
acsd_sc:
#	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/acsd dynamic
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/acsd sc -f Makefile.dslcpe
        
wlctl_sc:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/bcmcrypto dynamic
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/shared/ all -f Makefile.dslcpe
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/libupnp/ sc -f Makefile.dslcpe
ifeq ($(BRCM_DRIVER_DHD), m)
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/wl/exe sc
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/dhd/src/dhd/exe sc
else
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/wl/exe sc
endif

wps_sc:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/wps sc
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/ eapd_sc -f Makefile.dslcpe
#	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/router/igd sc
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/lltd sc
nvram_lib:
#	$(MAKE) -C userspace/private/apps/wlan/nvram libnvram
#	$(MAKE) -C userspace/private/apps/wlan/nvram nvram
#	cp userspace/private/apps/wlan/nvram/libnvram.so $(INSTALLDIR)/lib
#	cp userspace/private/apps/wlan/nvram/nvram $(INSTALLDIR)/user/sbin

bsd_sc:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/bsd sc

toad_sc:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/toad sc

dhd_monitor_sc:
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/dhd_monitor all
	$(MAKE) -C $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src/router/dhd_monitor sc
wl_sc: wlcsm_lib nvram_lib  wlctl_sc nas_sc wps_sc acsd_sc bsd_sc toad_sc dhd_monitor_sc

adsl_sc:
ifeq ($(CONFIG_SUPPORT_DSL),1)
	cp  $(BRCMDRIVERS_DIR)/broadcom/char/adsl/bcm9$(BRCM_CHIP)/adsl_phy.bin $(INSTALLDIR)/etc/adsl
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/atmctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/atmctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/xdslctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/dsldiagd sc
endif
	$(MAKE) -C $(BUILD_DIR)/userspace/public/libs/libpcap clean
	$(MAKE) -C $(BUILD_DIR)/userspace/public/libs/libpcap all
	$(MAKE) -C $(BUILD_DIR)/userspace/public/apps/tcpdump clean
	$(MAKE) -C $(BUILD_DIR)/userspace/public/apps/tcpdump all
vlan_sc:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/vlanctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/vlanctl sc
spu_sc:
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/spuctl sc
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/spuctl sc
	
ethctl_sc:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/ethswctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/ethctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/gpl/apps/ethtool sc
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/gmacctl/ sc
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/gmacctl/ sc

mdk_sc:
ifneq ($(strip $(BUILD_SWMDK)),)
	if [ ! -e $(BUILD_DIR)/userspace/private/libs/mdk/RELEASE ]; then \
         echo untaring original mdk; \
	  (tar xkfj $(BUILD_DIR)/userspace/private/libs/mdk/$(ORIGINAL_MDK) 2> /dev/null || true); \
	fi
	$(MAKE) -j 1 -C $(BUILD_DIR)/userspace/private/libs/mdk sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/swmdk sc
ifeq ($(SC_CUSTOMER), DT_TEST)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/mdkshell
	cp $(BUILD_DIR)/userspace/private/apps/mdkshell/mdkshell $(INSTALLDIR)/usr/sbin
endif
else
	@echo "Skipping switch mdk "
endif

ifneq ($(strip $(BUILD_VODSL)),)
vodsl_sc:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/vodsl sc
voctl_sc:
	$(MAKE) -C $(SC_ROOT)/apps/voctl clean
	$(MAKE) -C $(SC_ROOT)/apps/voctl
else
vodsl_sc voctl_sc:
	@echo "voice not enabled, skip building $@"
endif
bb : iptv_sc

iptv_sc:
ifeq ($(CONFIG_SUPPORT_IPTV), 1)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/bcmmcast/ sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/bridgeutil/ sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/mcpd sc
endif

fapctl_sc:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/fapctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/fapctl sc

snoopctl_sc:
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/snoopctl
#	cp -af $(BUILD_DIR)/userspace/private/libs/snoopctl/libsnoopctl.so $(INSTALLDIR)/lib
sc_tmsctl:
ifeq ($(BRCM_DRIVER_BDMF), m)
	$(MAKE) -C $(BUILD_DIR)/userspace/public/apps/bdmf_shell sc
endif
ifneq ($(GPL),1)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/tmctl sc
endif

bpmctl:
ifneq ($(PRODUCT_TYPE), AD1018)
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/bpmctl
endif

iqctl:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/iqctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/iqctl sc

fc_sc:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/fcctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/fcctl sc

pwrctl_sc:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/libs/pwrctl sc
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/pwrctl sc

btrmcfg_sc:
	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/btrmcfg sc

sc_vgw:
	$(MAKE) -C $(BUILD_DIR)/../../VoIP/vgw scm_vgw_clean
	$(MAKE) -C $(BUILD_DIR)/../../VoIP/vgw scm_vgw

KERNEL_VERSION=4.1.38+
BSP_KO_PATH=$(PROFILE_DIR)/modules/lib/modules/
dsp_ept:
	$(MAKE) voice_full SC_BUILD=1
	cp -af $(BSP_KO_PATH)/$(KERNEL_VERSION)/extra/dsphal.ko $(INSTALLDIR)/lib/modules/
	cp -af $(BSP_KO_PATH)/$(KERNEL_VERSION)/extra/slicslac.ko $(INSTALLDIR)/lib/modules/

sc_wlan:
	cp $(PROFILE_DIR)/fs.install/etc/wlan/* -r $(INSTALLDIR)/etc/wlan/
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/wlan clean
#	$(MAKE) -C $(BUILD_DIR)/userspace/private/apps/wlan all
sc_gpl_driver: sc_tmsctl wlcsm_lib dtbs sc_ubi
	$(MAKE) -C $(BUILD_DIR)/userspace/public/libs/libpcap clean
	$(MAKE) -C $(BUILD_DIR)/userspace/public/libs/libpcap all
	$(MAKE) -C $(BUILD_DIR)/userspace/public/apps/tcpdump clean
	$(MAKE) -C $(BUILD_DIR)/userspace/public/apps/tcpdump all
ifeq ($(BUILD_RDPA), y)
	cp -af $(BRCMDRIVERS_DIR)/opensource/char/rdpa_drv/bcm9$(BRCM_CHIP)/rdpa_cmd.ko $(INSTALLDIR)/lib/modules
	cp -af rdp/projects/DSL_63138/target/rdpa_gpl/rdpa_gpl.ko $(INSTALLDIR)/lib/modules
	cp -af rdp/projects/DSL_63138/target/rdpa/rdpa.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/opensource/char/rdpa_gpl_ext/bcm9$(BRCM_CHIP)/rdpa_gpl_ext.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/opensource/char/rdpa_mw/bcm9$(BRCM_CHIP)/rdpa_mw.ko $(INSTALLDIR)/lib/modules
endif
	cp -af $(BRCMDRIVERS_DIR)/opensource/net/enet/bcm9$(BRCM_CHIP)/bcm_enet.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/opensource/net/xtmrt/bcm9$(BRCM_CHIP)/bcmxtmrtdrv.ko $(INSTALLDIR)/lib/modules
	cp -af $(BRCMDRIVERS_DIR)/opensource/net/wfd/bcm9$(BRCM_CHIP)/wfd.ko $(INSTALLDIR)/lib/modules/
ifeq ($(SC_VOIP), 1)
	cp -af $(BRCMDRIVERS_DIR)/opensource/char/dsphal/bcm9$(BRCM_CHIP)/dsphal.ko $(INSTALLDIR)/lib/modules/
endif
# netfilter
	cp -af $(KERNEL_DIR)/net/netfilter/*.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/net/ipv4/netfilter/*.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/net/ipv4/*.ko $(INSTALLDIR)/lib/modules
ifeq ($(CONFIG_SUPPORT_IPV6),1)
	cp -af $(KERNEL_DIR)/net/ipv6/netfilter/*.ko $(INSTALLDIR)/lib/modules
endif
ifeq ($(CONFIG_SUPPORT_3G),1)
	cp -af $(KERNEL_DIR)/drivers/usb/serial/*.ko $(INSTALLDIR)/lib/modules
endif
	cp -af $(KERNEL_DIR)/drivers/net/ppp/pptp.ko $(INSTALLDIR)/lib/modules
	cp -af kernel/dts/9$(BRCM_CHIP).dtb $(INSTALLDIR)/
	cp -af $(KERNEL_DIR)/crypto/*.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/drivers/mtd/ubi/*.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/drivers/usb/storage/*.ko $(INSTALLDIR)/lib/modules
ifeq ($(BRCM_DRIVER_BDMF), m)
	cp -af rdp/projects/DSL_63138/target/bdmf/bdmf.ko $(INSTALLDIR)/lib/modules
endif

show:
	@echo Profile=$(PROFILE)  
clean_bcm_links:
	@echo -e "\\e[36m -- clean_bcm_links --\e[0m"	
	rm -f bcmdrivers/opensource/char/serial/bcm9$(BRCM_CHIP)
	rm -f bcmdrivers/opensource/char/board/bcm963xx/bcm9$(BRCM_CHIP)
	rm -f bcmdrivers/broadcom/atm/bcm9$(BRCM_CHIP)
	rm -f bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)
	rm -f bcmdrivers/broadcom/net/enet/bcm9$(BRCM_CHIP)
	rm -f bcmdrivers/broadcom/char/adsl/bcm9$(BRCM_CHIP)
	rm -f bcmdrivers/broadcom/char/atmapi/bcm9$(BRCM_CHIP)
	rm -f bcmdrivers/broadcom/char/bcmprocfs/bcm9$(BRCM_CHIP)
dhd_build:
	cd $(BRCMDRIVERS_DIR)/broadcom/net/wl/bcm9$(BRCM_CHIP)/firmware && ./build.perl
	
sc_ubi:
	cp -af $(KERNEL_DIR)/drivers/mtd/ubi/ubi.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/fs/ubifs/ubifs.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/crypto/lzo.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/crypto/zlib.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/crypto/deflate.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/lib/lzo/lzo_compress.ko $(INSTALLDIR)/lib/modules
	cp -af $(KERNEL_DIR)/lib/lzo/lzo_decompress.ko $(INSTALLDIR)/lib/modules

sc_openvswitch:
ifeq ($(CONFIG_SUPPORT_OPENVSWITCH),1)
#	cp -af $(KERNEL_DIR)/net/openvswitch/openvswitch.ko $(INSTALLDIR)/lib/modules
#	cp -af $(KERNEL_DIR)/net/openvswitch/vport-gre.ko $(INSTALLDIR)/lib/modules
endif
