#module makefile for 2.6 kernel
#

PWD  := $(shell pwd)
KVER := 2.6
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-bcm963xx/include/

obj-m = event_forwarder.o 

all: event_forwarder

event_forwarder:
	$(MAKE)   M=$(PWD) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

install:all
	install -D event_forwarder.ko  $(INSTALLDIR)/lib/modules/event_forwarder.ko

clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.symvers *.order
