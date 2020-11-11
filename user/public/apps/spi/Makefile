ifndef PATCHLEVEL
#include ../../Rules.mak
endif 

#CROSS_COMPILE=$(CROSS)

#KERNELDIR=$(KERNEL_DIR)
SUBDIR = $(shell pwd)
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-bcm963xx/include/


OBJS = common.ko firewall_block.ko fake_source_dos.ko \
		psd_and_special_udp_dos.ko tcp_syn_dos.ko clamp_total_session_for_one_src.ko others_dos.ko \
		xt_ct_dir.ko icmp_flood_dos.ko brdcst_src.ko tcp_fin_flood_dos.ko

obj-m:= common.o firewall_block.o fake_source_dos.o \
		psd_and_special_udp_dos.o tcp_syn_dos.o clamp_total_session_for_one_src.o others_dos.o \
		xt_ct_dir.o icmp_flood_dos.o brdcst_src.o tcp_fin_flood_dos.o 

EXTRA_CFLAGS += -Werror

all:modules

modules:
	$(MAKE)   M=$(SUBDIR) -C $(KERNELDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
	#$(MAKE) -C $(KERNELDIR) SUBDIRS=$(SUBDIR) modules V=1 CROSS_COMPILE=$(CROSS_COMPILE)

clean:
	rm -rf $(OBJS) *.o *.ko *.cmd .*.cmd *.mod.c
	rm -rf .tmp_versions Module.symvers

install:
	$(STRIP) --strip-unneeded $(OBJS)
	install -D $(OBJS) $(INSTALLDIR)/lib/modules
	

