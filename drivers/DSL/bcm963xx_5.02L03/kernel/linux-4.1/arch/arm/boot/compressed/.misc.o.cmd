cmd_arch/arm/boot/compressed/misc.o := /opt/toolchains//crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/bin/arm-linux-gcc -Wp,-MD,arch/arm/boot/compressed/.misc.o.d  -nostdinc -isystem /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/lib/gcc/arm-buildroot-linux-gnueabi/5.3.0/include -I./arch/arm/include -Iarch/arm/include/generated/uapi -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-bcm963xx/include -Iarch/arm/plat-bcm63xx/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -fno-PIE -O2 --param=allow-store-data-races=0 -Wframe-larger-than=2048 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -DCC_HAVE_ASM_GOTO -D__SC_BUILD__ -DVOX30 -DVFIT -DSC_NAND_FLASH_SIZE=256 -DCONFIG_SUPPORT_SPI_FIREWALL -DCONFIG_SUPPORT_UBUS -DCONFIG_SUPPORT_IPERF -DCONFIG_BRCM_SUPPORT_VMUX -DCONFIG_SUPPORT_FON -DVOX30_SFP -DVOX_LED_SPEC -DCONFIG_CRASH_LOG -fpic -mno-single-pic-base -fno-builtin -Iarch/arm/boot/compressed -I/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/bcmdrivers/opensource/include/bcm963xx -I/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/shared/opensource/include/bcm963xx    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(misc)"  -D"KBUILD_MODNAME=KBUILD_STR(misc)" -c -o arch/arm/boot/compressed/misc.o arch/arm/boot/compressed/misc.c

source_arch/arm/boot/compressed/misc.o := arch/arm/boot/compressed/misc.c

deps_arch/arm/boot/compressed/misc.o := \
    $(wildcard include/config/uncompress/include.h) \
    $(wildcard include/config/debug/icedcc.h) \
    $(wildcard include/config/cpu/v6.h) \
    $(wildcard include/config/cpu/v6k.h) \
    $(wildcard include/config/cpu/v7.h) \
    $(wildcard include/config/cpu/xscale.h) \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
    $(wildcard include/config/gcov/kernel.h) \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  include/uapi/linux/types.h \
  arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
    $(wildcard include/config/64bit.h) \
  include/uapi/asm-generic/bitsperlong.h \
  include/uapi/linux/posix_types.h \
  include/linux/stddef.h \
  include/uapi/linux/stddef.h \
  arch/arm/include/uapi/asm/posix_types.h \
  include/uapi/asm-generic/posix_types.h \
  include/linux/types.h \
    $(wildcard include/config/have/uid16.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/bcm/kf/unaligned/exception.h) \
    $(wildcard include/config/mips/bcm963xx.h) \
  include/linux/linkage.h \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  arch/arm/include/asm/linkage.h \
  arch/arm/mach-bcm963xx/include/mach/uncompress.h \
    $(wildcard include/config/bcm/kf/arm/bcm963xx.h) \
    $(wildcard include/config/debug/uart/addr.h) \
  arch/arm/mach-bcm963xx/include/mach/hardware.h \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/plat/bcm63xx/acp.h) \
  arch/arm/include/generated/asm/sizes.h \
  include/asm-generic/sizes.h \
  include/linux/sizes.h \
  /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/bcmdrivers/opensource/include/bcm963xx/bcm_map_part.h \
    $(wildcard include/config/bcm963268.h) \
    $(wildcard include/config/bcm96838.h) \
    $(wildcard include/config/bcm963138.h) \
    $(wildcard include/config/bcm960333.h) \
    $(wildcard include/config/bcm963381.h) \
    $(wildcard include/config/bcm963148.h) \
    $(wildcard include/config/bcm96848.h) \
    $(wildcard include/config/bcm94908.h) \
    $(wildcard include/config/bcm96858.h) \
    $(wildcard include/config/bcm947189.h) \
    $(wildcard include/config/bcm96836.h) \
    $(wildcard include/config/bcm963158.h) \
  /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/shared/opensource/include/bcm963xx/63138_map_part.h \
    $(wildcard include/config/lock.h) \
    $(wildcard include/config/arm.h) \
    $(wildcard include/config/bcm/gmac.h) \
    $(wildcard include/config/5x3/crossbar/support.h) \
    $(wildcard include/config/4x2/crossbar/support.h) \
  /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/bcmdrivers/opensource/include/bcm963xx/bcmtypes.h \
  /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/shared/opensource/include/bcm963xx/bcm_io_map.h \

arch/arm/boot/compressed/misc.o: $(deps_arch/arm/boot/compressed/misc.o)

$(deps_arch/arm/boot/compressed/misc.o):
