#
#module makefile for 2.6 kernel
#

PWD  := $(shell pwd)
KVER := 2.6


obj-m        := act_connmark.o

all:
	$(MAKE)   M=$(PWD) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers

install:
	$(STRIP) --strip-unneeded act_connmark.ko
	install -D act_connmark.ko $(INSTALLDIR)/lib/modules/act_connmark.ko	

