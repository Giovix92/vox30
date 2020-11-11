#
#module makefile for 2.6 kernel
#

PWD  := $(shell pwd)
KVER := 4.16
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-bcm963xx/include/



obj-m        := act_connmark.o

all:
ifeq ($(BRCM_502), 1)
	cp act_connmark.c.502 act_connmark.c
endif
	$(MAKE)   M=$(PWD) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers

install:
#	$(STRIP) --strip-unneeded act_connmark.ko
	install -D act_connmark.ko $(INSTALLDIR)/lib/modules/act_connmark.ko	

