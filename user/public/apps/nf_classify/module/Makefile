#
#module makefile for 2.6 kernel
#

PWD  := $(shell pwd)
KVER := 2.6


obj-m += nf_classify.o 
EXTRA_CFLAGS = -DCONFIG_SCM_SUPPORT
all:
	$(MAKE)   M=$(PWD) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers
install:
	$(STRIP) --strip-unneeded nf_classify.ko
	install -D nf_classify.ko $(INSTALLDIR)/lib/modules/nf_classify.ko	
	
