cmd_fs/ntfs/sysctl.o := /opt/toolchains//crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/bin/arm-linux-gcc -Wp,-MD,fs/ntfs/.sysctl.o.d  -nostdinc -isystem /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/lib/gcc/arm-buildroot-linux-gnueabi/5.3.0/include -I./arch/arm/include -Iarch/arm/include/generated/uapi -Iarch/arm/include/generated  -Iinclude -I./arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Iarch/arm/mach-bcm963xx/include -Iarch/arm/plat-bcm63xx/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-dwarf2-cfi-asm -fno-ipa-sra -mabi=aapcs-linux -mno-thumb-interwork -mfpu=vfp -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -fno-delete-null-pointer-checks -fno-PIE -O2 --param=allow-store-data-races=0 -Wframe-larger-than=2048 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -DCC_HAVE_ASM_GOTO -D__SC_BUILD__ -DVOX30 -DVFIT -DSC_NAND_FLASH_SIZE=256 -DCONFIG_SUPPORT_SPI_FIREWALL -DCONFIG_SUPPORT_UBUS -DCONFIG_SUPPORT_IPERF -DCONFIG_BRCM_SUPPORT_VMUX -DCONFIG_SUPPORT_FON -DVOX30_SFP -DVOX_LED_SPEC -DCONFIG_CRASH_LOG -g -Werror -Wfatal-errors -Wno-date-time -Wno-declaration-after-statement -Wno-switch-bool -DNTFS_VERSION=\"2.1.32\" -DNTFS_RW    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(sysctl)"  -D"KBUILD_MODNAME=KBUILD_STR(ntfs)" -c -o fs/ntfs/sysctl.o fs/ntfs/sysctl.c

source_fs/ntfs/sysctl.o := fs/ntfs/sysctl.c

deps_fs/ntfs/sysctl.o := \
    $(wildcard include/config/sysctl.h) \

fs/ntfs/sysctl.o: $(deps_fs/ntfs/sysctl.o)

$(deps_fs/ntfs/sysctl.o):
