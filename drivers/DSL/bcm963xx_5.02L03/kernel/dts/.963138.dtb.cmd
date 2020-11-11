cmd_/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dtb := mkdir -p /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/ ; /opt/toolchains//crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25/usr/bin/arm-linux-gcc -E -Wp,-MD,/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/.963138.dtb.d.pre.tmp -nostdinc -I./arch/arm/boot/dts -I./arch/arm/boot/dts/include -I./drivers/of/testcase-data -undef -D__DTS__ -x assembler-with-cpp -o /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/.963138.dtb.dts.tmp /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dts ; ./scripts/dtc/dtc -O dtb -o /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dtb -b 0 -i /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/  -d /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/.963138.dtb.d.dtc.tmp /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/.963138.dtb.dts.tmp ; cat /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/.963138.dtb.d.pre.tmp /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/.963138.dtb.d.dtc.tmp > /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/.963138.dtb.d

source_/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dtb := /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dts

deps_/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dtb := \
    $(wildcard include/config/bcm/astra.h) \
  /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/bcm_963xx_template.dtsi \
    $(wildcard include/config/bcm/adsl.h) \
    $(wildcard include/config/bcm/rdpa.h) \
    $(wildcard include/config/bcm/dhd/runner.h) \
  arch/arm/boot/dts/include/dt-bindings/soc/bcm963xx_dt_bindings.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \

/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dtb: $(deps_/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dtb)

$(deps_/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/kernel/dts/963138.dtb):
