#module makefile for 2.6 kernel
#

PWD  := $(shell pwd)
KVER := 2.6
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-bcm963xx/include/

obj-m = ebt_sc_mark.o 

all: ebt_sc_mark

ebt_sc_mark:
	$(MAKE)   M=$(PWD) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

install:all
	install -D ebt_sc_mark.ko  $(INSTALLDIR)/lib/modules/ebt_sc_mark.ko

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers
