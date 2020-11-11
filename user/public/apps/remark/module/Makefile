#module makefile for 2.6 kernel
#

PWD  := $(shell pwd)
KVER := 2.6
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-bcm963xx/include/

obj-m = qos_remark.o

all: qos_remark 

qos_remark:
	$(MAKE)   M=$(PWD) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

install:all
	install -D qos_remark.ko  $(INSTALLDIR)/lib/modules/qos_remark.ko

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers
