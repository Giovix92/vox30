export TOOLPREFIX=mips-linux-
export KERNELARCH=mips
export ARCH=mips
#
# Cross infomation
#
CROSS_COMPILE  = $(TOOLPREFIX)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CROSS_COMPILE)gcc -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
RANLIB      = $(CROSS_COMPILE)ranlib
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
CXX		= $(CROSS_COMPILE)g++

export CROSS_COMPILE AS LD CC CPP AR NM RANLIB STRIP OBJCOPY OBJDUMP CXX

#
#Rootfs squash or jffs2, now jffs2 is not supported
#

export ROOTFSTYPE=jffs2
export SC_DUALIMAGE = 1
export SC_NAND_FLASH_SIZE = 128
#
#Voice Slic model name
#
export SC_VOICE_SLIC_MODEL = SI32177C


#
#SC Module Name and ODM Name
#


export SC_ANNEX ?= A

#
#FW versions
#
ifndef FW_EXTRA_VERSION
export FW_EXTRA_VERSION =
endif
ifeq ($(SC_CUSTOMER), VFIN)
export FW_BOOT_VERSION = 1.1.1.0
else
ifeq ($(BRCM_416), 1)
export FW_BOOT_VERSION = 1.0.3.0
else
export FW_BOOT_VERSION = 1.0.2.0
endif
endif
ifndef FW_VERSION
ifeq ($(ROOTFSTYPE), jffs2) 
ifneq ($(FW_EXTRA_VERSION), )
export FW_VERSION = $(shell cat ./product/VOX25/jffs2/.pid | cut -b 101-104)_$(FW_EXTRA_VERSION)
else
export FW_VERSION = $(shell cat ./product/VOX25/jffs2/.pid | cut -b 101-104)
endif
else
export FW_VERSION = $(FW_EXTRA_VERSION)
endif
endif
ifeq ($(SC_ANNEX), A)
export FW_DSL_VERSION = A2pv6F038j_rc6.d24h
else
export FW_DSL_VERSION = B2pvF038h3.d24h
endif

#HW version
#
export SC_HW_VERSION = v2
# Please enable this option if you want to build fw for BL demo board.
export SC_BL_DEMO_BOARD_SUPPORT = 0
export SC_ROOTFS_LIBS_VER = 1002
#
# Define the Features Here
#

export SC_SPI = 1
export SC_GPON = 0
export SC_GPON_DEBUG = 0
export SC_OMCI = 1
export SC_IPHOST = 1
export CONFIG_SCM_SUPPORT = 1
export SC_SUPPORT_ERICSSON = 1
export SC_MIXVENDOR_SPT = 1
export SC_DMM = 0
export SC_SUPPORT_CATV = 1

export SC_WIFI = 1
export SC_WSC = 0
export SC_WIFI_CALIBRATION = 0
#please do not enable VOIP &SLIC_TEST at the same time
export SC_VOIP = 1
export SC_SLIC_TEST = 0
export SC_VOIP_LINE_EXCHANGE = 1

export SC_SWITCHTYPE = 

export SC_ROUTER = 1
export SC_MULTIWAN = 1
export SC_QUICK_TABLE = 0
export SC_HW_NAT = 1
export SC_DATA_RATE = 0

export CONFIG_SUPPORT_ALLJOYN = 0
export CONFIG_SUPPORT_DSL = 0
export CONFIG_SUPPORT_VDSL = 0
export CONFIG_SUPPORT_QOS = 0

export SC_SNMP = 1
export SC_BFTPD = 0
export SC_TR069 = 1
export SC_SSHD = 1
export SC_DROPBEAR = 0
export SC_USB_STORAGE = 0
export CONFIG_SUPPORT_IPV6 = 0
export CONFIG_SUPPORT_IPV6_LOGO = 0

#wifi chip: realtek, atheros
export SC_WIFI_CHIP = brcm
export SC_WIFI_11N_CERTIFICATE = 1
export SC_WIFI_WPS_CERTIFICATE = 1
export SC_WIFI_WPS_V2 = 0
export CONFIG_SUPPORT_WIFI_CERTIFICATE = 0

export CONFIG_SUPPORT_MULTI_LEVEL_CONFIG = 0

# OLT send reboot command to ONU via OMCI, The ONU do reboot until 120 seconds later.
# Change only wait 5 seconds before the reboot.
export SCM_OMCI_REBOOT = 1
# Modify the queue size to 128 for NAT performatnce
export SCM_QUEUE_SIZE = 1
export SCM_IGMP_SNOOP = 1
export SCM_PACKET_MARK = 1
export SCM_GPON_US_QOS = 0
export CONFIG_SUPPORT_CORE_DUMP = 0
export CONFIG_SUPPORT_L2BRIDGING = 0

#
#BL PROJECT_NAME: GateMakerPro or PONMakerPro
#

export PROJECT_ROOT=$(TOPDIR)/drivers/GPON/GateMakerPro_v10.0/GMP

#
#OMCI version: ONT-R4.0.0.2, ONT-R4.0.1.3, ONT-R4.2.0.2, ONT-R4.2.1.1 PONWiz-4.8
#

export OMIC_NAME=PONWiz_10.0

export PONWIZ_PATH=$(TOPDIR)/drivers/GPON_OMCI/$(OMIC_NAME)

export SUPPORT_PERLINE_PROVISION=0

export CML_CONVERT_BOOL = 1
export TR069_EVENTCODE_BOOTSTRAP_EXCLUDE_BOOT = 1
export MULTIPLE_LANGUAGE_FOR_MULTIPLE_USER = 1

export SUPPORT_STATISTICS_NOTIFICATION = 1
export CONFIG_SUPPORT_CNAPT = 0
export CONFIG_SUPPORT_BBU = 0
export CONFIG_SUPPORT_AUTO_SENSE = 0

export CONFIG_BRCM_SUPPORT = 1
export CONFIG_BRCM_SUPPORT_VMUX = 0

export CONFIG_SUPPORT_REDIRECT = 1
export CONFIG_SUPPORT_DMALLOC = 0
export CONFIG_SUPPORT_VOIP_DMALLOC = 0
export CONFIG_SUPPORT_PRIVATE_HOSTNAME = 1
export CONFIG_SUPPORT_VOICE_WATCHDOG = 1
export CONFIG_SUPPORT_VOICE_PBX = 0
# 3G related features, default do not enable it
export CONFIG_SUPPORT_3G = 0
export DETECT_SIM_INFO_V1 = 0
export FAILOVER = 0
#support slic interrupt mode
export SC_SUPPORT_SLIC_INTERRUPT = 0
export CONFIG_SUPPORT_VOIP_ON_LAN = 0
export CONFIG_SUPPORT_FAP = 1
export CONFIG_SUPPORT_ATTACK_ALERT = 1
export CONFIG_SUPPORT_ENCRYPTION_AES = 1
export CONFIG_SUPPORT_DEBUG = 0
export CONFIG_SUPPORT_FON = 1
export CONFIG_SUPPORT_WIFI_MSSID = 1
export CONFIG_SUPPORT_WIFI_WDS = 0
export CONFIG_USE_DNSMASQ = 1
#support web api
export CONFIG_SUPPORT_WEBAPI = 0
export CONFIG_SUPPORT_5G_BRCM = 1
export CONFIG_SUPPORT_5G_QD = 0
#support ubusd
export CONFIG_SUPPORT_UBUSD = 1
export CONFIG_SUPPORT_UBUS = 1
#support bandsteering
export CONFIG_SUPPORT_BANDSTEERING = 0
#support pcpd
export CONFIG_SUPPORT_PCPD = 0
#support wake on LAN
export CONFIG_SUPPORT_WOLAN = 1
#support energy saving
export CONFIG_SUPPORT_ENERGY_SAVING = 1
export CONFIG_SUPPORT_LIBCURL = 1
