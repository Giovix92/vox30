#module makefile for 2.6 kernel
#

PWD  := $(shell pwd)
KVER := 2.6
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-bcm963xx/include/

obj-m:= nulldev.o

all:modules

modules:
	$(MAKE)   M=$(PWD) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

install:all
	install -D nulldev.ko  $(INSTALLDIR)/lib/modules/nulldev.ko

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers
