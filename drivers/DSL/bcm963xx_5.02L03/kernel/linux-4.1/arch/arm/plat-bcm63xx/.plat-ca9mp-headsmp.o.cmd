cmd_arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.o := /opt/toolchains//crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/bin/arm-linux-gcc -Wp,-MD,arch/arm/plat-bcm63xx/.plat-ca9mp-headsmp.o.d  -nostdinc -isystem /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/lib/gcc/arm-buildroot-linux-gnueabi/5.3.0/include -I./arch/arm/include -Iarch/arm/include/generated/uapi -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-bcm963xx/include -Iarch/arm/plat-bcm63xx/include   -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float -fno-PIE -DCC_HAVE_ASM_GOTO            -c -o arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.o arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.S

source_arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.o := arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.S

deps_arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.o := \
    $(wildcard include/config/bcm/kf/arm/bcm963xx.h) \
  arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/cpu/v7m.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  arch/arm/include/asm/linkage.h \
  include/linux/init.h \
    $(wildcard include/config/broken/rodata.h) \
    $(wildcard include/config/lto.h) \
  include/linux/types.h \
    $(wildcard include/config/have/uid16.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
    $(wildcard include/config/bcm/kf/unaligned/exception.h) \
    $(wildcard include/config/mips/bcm963xx.h) \
  include/uapi/linux/types.h \
  arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/arm/include/generated/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/uapi/asm-generic/bitsperlong.h \

arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.o: $(deps_arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.o)

$(deps_arch/arm/plat-bcm63xx/plat-ca9mp-headsmp.o):
