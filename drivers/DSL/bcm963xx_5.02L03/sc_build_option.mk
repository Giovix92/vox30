# SeRcOmM
SC_BUILD = y
# Sercomm Product Name
ifeq ($(PRODUCT_TYPE), VD1018)
SC_PROJECT = VD1018
else
ifeq ($(PRODUCT_TYPE), FD1018)
SC_PROJECT = FD1018
else
SC_PROJECT = AD1018
endif
endif
ifeq ($(SC_CUSTOMER), NEUTRAL)
SC_NEUTRAL = y
endif


# Sercomm Upgrade Built-in
SC_UPGRADE = y
# Sercomm Upgrade with ECC check
SC_UPGRADE_ECC = n
# Sercomm CFE Command added
SC_CFE_CMD = y

# Sercomm Flash Map
SC_FL_MAP = y
# Sercomm LED 
SC_GPIO = y

# Sercomm ETH LED
SC_ETH_LED = n

# Sercomm DRAM Test Code
SC_RAM_TEST = n
# Sercomm Remove SLIC
SC_NO_SLIC = n
#sercomm log close
SC_SECURE_CLOSE = y

export		\
SC_BUILD	\
SC_NEUTRAL  \
SC_PROJECT 	\
SC_UPGRADE 	\
SC_UPGRADE_ECC  \
SC_CFE_CMD 	\
SC_FL_MAP 	\
SC_GPIO    	\
SC_RAM_TEST 	\
SC_ETH_LED   	\
SC_NO_SLIC  	\
SC_SECURE_CLOSE
# Broadcomm CFE Build Options
CFE_DIR=cfe/build/broadcom/bcm63xx_rom
ifeq ($(PRODUCT_TYPE), VD1018)
BRCM_CHIP=63268
else
ifeq ($(PRODUCT_TYPE), FD1018)
BRCM_CHIP=63268
else
BRCM_CHIP=6328
endif
endif
#BLD_NAND=1
export \
BRCM_CHIP \
BLD_NAND
######################################################################################################
CFE_DIR=cfe/build/broadcom/bcm63xx_rom
#STR_BOARDID="963168VX" //change board ID for CUT 2 which have INTERNEL PA
# Default NVRAM Settings Options
ifeq ($(PRODUCT_TYPE), VD1018)
CFE=cfe63268rom.bin
CHIP_TYPE=63268
else
CFE=cfe6328rom.bin
CHIP_TYPE=6328
endif
FLASH_TYPE=NAND128
ifeq ($(PRODUCT_TYPE), VD1018)
STR_BOARDID="963168MBV_30A"
STR_VOICEID="SI3217X"
NUM_MAC=12
else
STR_BOARDID="963281TAN"
STR_VOICEID="SI3217x_NOFXO"
NUM_MAC=11
endif
STR_MAC_ADDR="00:c0:02:12:35:88"
NUM_TP=0
NUM_PSISIZE=24
ifeq ($(PRODUCT_TYPE), VD1018)
CFE_NVRAM=cfe63268rom_nvram.bin
else
CFE_NVRAM=cfe6328rom_nvram.bin
endif


