#Telecom operator: DT_TEST, VFES VFIN, NEUTRAL, VFPT
ifndef SC_CUSTOMER
export SC_CUSTOMER = VFIT
CFLAGS += -D$(SC_CUSTOMER)
CFLAGS += -DSC_CUSTOMER=\"$(SC_CUSTOMER)\"
endif

#second customer define for DT_TEST
ifndef SC_CUSTOMER1
export SC_CUSTOMER1 =
endif
#
#PRODUCT_TYPE: AD1018 FD1018 VD1018 VOX25 ESSENTIAL
#
ifndef PRODUCT_TYPE
export PRODUCT_TYPE = VOX30
CFLAGS += -D$(PRODUCT_TYPE)
ifeq ($(PRODUCT_TYPE), ESSENTIAL)
ifeq ($(SC_CUSTOMER), LOWI)
CFLAGS += -DPRODUCT_TYPE=\"lowi-h500s\"
else
ifeq ($(SC_CUSTOMER1), LOWI)
CFLAGS += -DPRODUCT_TYPE=\"lowi-h500s\"
else
CFLAGS += -DPRODUCT_TYPE=\"Vodafone-H-500-s\"
endif
endif
else
CFLAGS += -DPRODUCT_TYPE=\"$(PRODUCT_TYPE)\"
endif
endif
ifeq ($(PRODUCT_TYPE), VOX30)
export FW_VERSION_PREFIX = XS
CFLAGS += -DFW_VERSION_PREFIX=\"XS\"
else
ifeq ($(PRODUCT_TYPE), VD725)
export FW_VERSION_PREFIX = VD725
CFLAGS += -DFW_VERSION_PREFIX=\"VD725\"
else
ifeq ($(PRODUCT_TYPE), VD300)
export FW_VERSION_PREFIX = VD300
CFLAGS += -DFW_VERSION_PREFIX=\"VD300\"
else
CFLAGS += -DFW_VERSION_PREFIX=\"\"
endif
endif
endif
# manufacturer: SERCOMM
ifndef SC_OEM
export SC_OEM = SERCOMM
CFLAGS += -D$(SC_OEM)
CFLAGS += -DSC_OEM=\"$(SC_OEM)\"
endif

export BRCM_416 = 0
export BRCM_502 = 1
#
# Include the specific configuration files from the config.boardtype file
# here.  This removes the need to set environmental variables through a
# script before building
#
include $(BUILDPATH)/product/$(PRODUCT_TYPE)/config.mk
include $(BUILDPATH)/product/$(PRODUCT_TYPE)/oem/$(SC_OEM)/config.mk

ifneq ($(SC_CUSTOMER1), )
include $(BUILDPATH)/product/$(PRODUCT_TYPE)/customer/$(SC_CUSTOMER)/config.mk.$(SC_CUSTOMER1)
else
include $(BUILDPATH)/product/$(PRODUCT_TYPE)/customer/$(SC_CUSTOMER)/config.mk
endif
include $(BUILDPATH)/product/$(PRODUCT_TYPE)/cflags.mk

#
# Put in safety checks here to ensure all required variables are defined in
# the configuration file
#

ifndef TOOLPREFIX
#error "Must specify TOOLPREFIX value"
endif

ifndef KERNELARCH
#error "Must specify KERNELARCH value"
endif

#
# Other environmental variables that are configured as per the configuration file
# specified above.  These contain all platform specific configuration items.
#
export INSTALL_ROOT=$(TOPDIR)/build/rootfs.build
export MODULEPATH=$(INSTALL_ROOT)/lib/modules
ifeq ($(BRCM_502), 1)
export BSP_DIR=$(TOPDIR)/drivers/DSL/bcm963xx_5.02L03
export KERNEL_DIR=$(BSP_DIR)/kernel/linux-4.1
else
ifeq ($(BRCM_416), 1)
export BSP_DIR=$(TOPDIR)/drivers/DSL/bcm963xx_4.16_L05
export KERNEL_DIR=$(BSP_DIR)/kernel/linux-3.4rt
else
export BSP_DIR=$(TOPDIR)/drivers/DSL/bcm963xx
export KERNEL_DIR=$(BSP_DIR)/kernel/linux
endif
endif
export KERNEL_INC=$(KERNEL_DIR)/include

export HOSTTOOLS_DIR = $(BSP_DIR)/hostTools
export KERNELDIR=$(KERNEL_DIR)

export MAKEARCH=$(MAKE)
export ARCH=$(KERNELARCH)
export CROSS_COMPILE=$(TOOLPREFIX)

export APPSPATH = $(TOPDIR)/user
export PRIVATE_APPSPATH = $(TOPDIR)/user/private
export PUBLIC_APPSPATH = $(TOPDIR)/user/public
export DMALLOC_DIR = $(TOPDIR)/user/debug/apps/dmalloc/dmalloc-5.5.2
export DEBUG_APPSPATH = $(TOPDIR)/user/debug
export OPENSSL_DIR = $(TOPDIR)/user/public/libs/openssl/openssl-1.0.2q
export USER_VENDOR_PATH = $(TOPDIR)/user/private/vendor
export TFTPPATH = $(BUILDPATH)
export INSTALLDIR = $(INSTALL_ROOT)
export TARGETDIR = $(INSTALL_ROOT)
export DESTDIR = $(INSTALL_ROOT)


ifeq ($(BRCM_502), 1)
export CROSS_DIR = /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/
export TOOLPATH = $(CROSS_DIR)/usr/bin
export TOOLCHAIN_TOP = $(CROSS_DIR)
export SYSROOTS = $(CROSS_DIR)/usr/bin
export TOOL_INC = $(CROSS_DIR)/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include
else
ifeq ($(BRCM_416), 1)
export CROSS_DIR = /opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL/
export TOOLPATH = /opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL/usr/bin
export TOOLCHAIN_TOP =/opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL/
export SYSROOTS = /opt/toolchains/crosstools-arm-gcc-4.6-linux-3.4-uclibc-0.9.32-binutils-2.21-NPTL/usr/bin
else
export CROSS_DIR = /opt/toolchains/uclibc-crosstools-gcc-4.4.2-1/
export TOOLPATH = /opt/toolchains/uclibc-crosstools-gcc-4.4.2-1/usr/bin
export TOOLCHAIN_TOP = /opt/toolchains/uclibc-crosstools-gcc-4.4.2-1
export SYSROOTS = /opt/toolchains/uclibc-crosstools-gcc-4.4.2-1/usr/bin
endif
endif
export PATH:=$(TOPDIR)/build/util:$(TOOLPATH):$(SYSROOTS):$(KERNEL_DIR):$(TOPDIR)/build:`pwd`:$(PATH)

ifeq ($(ROOTFSTYPE), jffs2)
ifneq ($(SC_CUSTOMER1), )
export PID = $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/.$(SC_OEM)_$(SC_CUSTOMER)_pid.$(SC_CUSTOMER1)
else
ifeq ($(VOX30_SFP), 1)
export PID = $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/.$(SC_OEM)_$(SC_CUSTOMER)_pid_sfp
else
export PID = $(BUILDPATH)/product/$(PRODUCT_TYPE)/jffs2/.$(SC_OEM)_$(SC_CUSTOMER)_pid
endif
endif
else
export PID = $(BUILDPATH)/product/$(PRODUCT_TYPE)/ubi/.$(SC_OEM)_$(SC_CUSTOMER)_pid
endif
ifeq ($(CONFIG_SUPPORT_FON), 1)
export BUILD_FON=y
endif
ifeq ($(CONFIG_SUPPORT_ALLJOYN), 1)
export BUILD_ALLJOYN=y
endif
export PACKAGE_ROOT=$(TOPDIR)/build/PACKAGE
export PACKAGE_MAIN=$(INSTALL_ROOT)/package
export GNUTLS_DIR=$(PUBLIC_APPSPATH)/libs/libgnutls/gnutls-3.6.2/
