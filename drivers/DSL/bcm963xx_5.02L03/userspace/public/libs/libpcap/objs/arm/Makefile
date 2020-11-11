#  Copyright (c) 1993, 1994, 1995, 1996
# 	The Regents of the University of California.  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that: (1) source code distributions
#  retain the above copyright notice and this paragraph in its entirety, (2)
#  distributions including binary code include the above copyright notice and
#  this paragraph in its entirety in the documentation or other materials
#  provided with the distribution, and (3) all advertising materials mentioning
#  features or use of this software display the following acknowledgement:
#  ``This product includes software developed by the University of California,
#  Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
#  the University nor the names of its contributors may be used to endorse
#  or promote products derived from this software without specific prior
#  written permission.
#  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
#  WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#
# Various configurable paths (remember to edit Makefile.in, not Makefile)
#

# Top level hierarchy
prefix = /home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/targets/963138GWV_VOX_IPV6/fs.build/public
exec_prefix = ${prefix}
datarootdir = ${prefix}/share
# Pathname of directory to install the configure program
bindir = ${exec_prefix}/bin
# Pathname of directory to install the include files
includedir = ${prefix}/include
# Pathname of directory to install the library
libdir =  ${exec_prefix}/lib
# Pathname of directory to install the man pages
mandir = ${datarootdir}/man

# VPATH
srcdir = ../../libpcap
VPATH = ../../libpcap

#
# You shouldn't need to edit anything below.
#

LD = /usr/bin/ld
CC = /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/bin/arm-linux-gcc
AR = /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/bin/arm-linux-ar
LN_S = ln -s
MKDEP = 
CCOPT = -I$(srcdir) -fpic -DVFIT -DSC_CUSTOMER=\"VFIT\" -DVOX30 -DPRODUCT_TYPE=\"VOX30\" -DFW_VERSION_PREFIX=\"XS\" -DSERCOMM -DSC_OEM=\"SERCOMM\" -DSC_OEM_OUI=\"BC15AC\" -DSC_SSID_PREFIX=\"Vodafone\" -DSPECIAL_ARPPING_URI_DEFINE -DSUPPORT_RESET_ALL_CONFIG -DLANGUAGE_SUPPORT=\"en\" -DSC_OPCO=\"it\" -DCONFIG_SUPPORT_USER_MANAGEMENT_BY_ENDUSER -DCONFIG_SUPPORT_WEEK_DATE -DSC_MANUFACTUREERURL=\"www.sercomm.com\" -DSC_MODELURL=\"www.vodafone.com\" -DCONFIG_SUPPORT_BLOCKING_ACCESS_TO_NEW_DEVICES -DCONFIG_SUPPORT_ARP_TABLE_FILTER -DCONFIG_SUPPORT_DYNAMIC_CLI_PROMOTE -DSC_HW_VERSION=\"v2\" -DSC_OEM_OUI_STRING=\"_VODAFONE_\" -DBRCM_502 -DCONFIG_SUPPORT_PASSWORD_PROTECTION -DMULTI_LEVEL_CONFIG -DCONFIG_SAFE_NETWORK -DCONFIG_SUPPORT_WEB_PRIVOXY -DCONFIG_SCM_SUPPORT -D__SC_BUILD__ -DVOX30_SFP -DVOX_LED_SPEC -DCONFIG_SUPPORT_NEW_LED_BEHAV -DSC_MIXVENDOR_SPT -DSC_VOIP_SUPPORT_PBX -DSC_VOIP_SUPPORT_SILENT_CALL -D_SC_SPI_FIREWALL_ -DSUPPORT_OMCI -DSUPPORT_WIFI -DCONFIG_SUPPORT_WIFI -DSUPPORT_MIMO -DSUPPORT_WSC -DWMF -DSUPPORT_WIFI_BRCM -DSUPPORT_WIFI_11N_CERTIFICATE -DSUPPORT_WIFI_WPS_CERTIFICATE -DSUPPORT_VOIP -DSUPPORT_ROUTER -DCONFIG_SUPPORT_CHRONY -DSUPPORT_MULTIWAN_DOMAIN -DSUPPORT_MULTIWAN_NTP_BINDIP -DSUPPORT_SNMP -DSUPPORT_TR069 -DSUPPORT_SSHD -DSUPPORT_DUALIMAGE -DSCM_PACKET_MARK -DSUPPORT_USB_STORAGE -DCONFIG_SUPPORT_SAMBA_VERSION_36 -DSUPPORT_USB_PRINTER -DCONFIG_SUPPORT_IPV6 -DCML_CONVERT_BOOL -DSUPPORT_STATISTICS_NOTIFICATION -DCONFIG_SUPPORT_MEDIA_SERVER -DCONFIG_SUPPORT_DSL -DCONFIG_SUPPORT_VDSL -DCONFIG_SUPPORT_QOS -DCONFIG_SUPPORT_AUTO_SENSE -DCONFIG_SUPPORT_JSON -DCONFIG_BRCM_SUPPORT -DCONFIG_BRCM_SUPPORT_VMUX -DCONFIG_SUPPORT_PRIVATE_HOSTNAME -DHTTP_LOGIN_COOKIE -DCONFIG_SUPPORT_REDIRECT -DCONFIG_SUPPORT_CWMP_RANDOM_PORT -DCONFIG_SUPPORT_WIZARD -DCONFIG_SUPPORT_3G -DCONFIG_SUPPORT_SMS -DCONFIG_SUPPORT_K4203 -DCONFIG_SUPPORT_K5150 -DCONFIG_SUPPORT_3G_BACKUP -DCONFIG_SUPPORT_FIRST_AUTO_DOD -DCONFIG_SUPPORT_VOCS -DSUPPORT_PHONE_PIN_WIZARD -DCONFIG_SUPPORT_WAN_BACKUP -DCONFIG_SUPPORT_ENCRYPTION_AES -DCONFIG_SUPPORT_IPTV -DCONFIG_SUPPORT_FON -DCONFIG_SUPPORT_RUNNER -DCONFIG_SUPPORT_L2TP -DCONFIG_SUPPORT_WIFI_MSSID -DCONFIG_USE_DNSMASQ_IPV6_ONLY -DCONFIG_SUPPORT_WIFI_5G -DCONFIG_SUPPORT_5G_BRCM -DCONFIG_SUPPORT_STATIC_NAT -DSUPPORT_TWO_FXS -DCONFIG_SUPPORT_WEBAPI -DCONFIG_SUPPORT_UBUS -DCONFIG_SUPPORT_PRPL_HL_API -DCONFIG_SUPPORT_IPERF -DCONFIG_SUPPORT_BANDSTEERING -DCONFIG_SUPPORT_WOLAN -DCONFIG_SUPPORT_ENERGY_SAVING -DCONFIG_SUPPORT_SJCL_ENCRYPT -DCONFIG_SUPPORT_CGN -DCONFIG_SUPPORT_SEC_BOOT -DCONFIG_SUPPORT_AUTO_DETECT -DCONFIG_SUPPORT_DSL_FTTH_SWITCH -DCONFIG_SUPPORT_L2TP_CLIENT -DCONFIG_SUPPORT_L2TP_SERVER -DCONFIG_SUPPORT_PPTP_CLIENT -DCONFIG_SUPPORT_PPTP_SERVER -DCONFIG_CRASH_LOG -DCONFIG_SUPPORT_IPSEC_CLIENT -DCONFIG_SUPPORT_NATLOOPBACK -DCONFIG_SUPPORT_PACKAGE -DCONFIG_SUPPORT_BOOSTER_MODE -DCONFIG_PPP_SUPPORT_PADM_PADN -DCONFIG_SUPPORT_TR157 -DCONFIG_SUPPORT_LCM -DCONFIG_SUPPORT_SUPERWIFI -fstack-protector -DCONFIG_SUPPORT_SMARTTHING -DCONFIG_SUPPORT_FWA -DCONFIG_SUPPORT_HA -DCONFIG_SUPPORT_MQTT -DFW_VERSION_PREFIX=\"XS\" -DSC_OEM_OUI=\"BC15AC\" -DSC_SSID_PREFIX=\"Vodafone\" -DSPECIAL_ARPPING_URI_DEFINE -DSUPPORT_RESET_ALL_CONFIG -DLANGUAGE_SUPPORT=\"en\" -DSC_OPCO=\"it\" -DCONFIG_SUPPORT_USER_MANAGEMENT_BY_ENDUSER -DCONFIG_SUPPORT_WEEK_DATE -DSC_MANUFACTUREERURL=\"www.sercomm.com\" -DSC_MODELURL=\"www.vodafone.com\" -DCONFIG_SUPPORT_BLOCKING_ACCESS_TO_NEW_DEVICES -DCONFIG_SUPPORT_ARP_TABLE_FILTER -DCONFIG_SUPPORT_DYNAMIC_CLI_PROMOTE -DSC_HW_VERSION=\"v2\" -DSC_OEM_OUI_STRING=\"_VODAFONE_\" -DBRCM_502 -DCONFIG_SUPPORT_PASSWORD_PROTECTION -DMULTI_LEVEL_CONFIG -DCONFIG_SAFE_NETWORK -DCONFIG_SUPPORT_WEB_PRIVOXY -DCONFIG_SCM_SUPPORT -D__SC_BUILD__ -DVOX30_SFP -DVOX_LED_SPEC -DCONFIG_SUPPORT_NEW_LED_BEHAV -DSC_MIXVENDOR_SPT -DSC_VOIP_SUPPORT_PBX -DSC_VOIP_SUPPORT_SILENT_CALL -D_SC_SPI_FIREWALL_ -DSUPPORT_OMCI -DSUPPORT_WIFI -DCONFIG_SUPPORT_WIFI -DSUPPORT_MIMO -DSUPPORT_WSC -DWMF -DSUPPORT_WIFI_BRCM -DSUPPORT_WIFI_11N_CERTIFICATE -DSUPPORT_WIFI_WPS_CERTIFICATE -DSUPPORT_VOIP -DSUPPORT_ROUTER -DCONFIG_SUPPORT_CHRONY -DSUPPORT_MULTIWAN_DOMAIN -DSUPPORT_MULTIWAN_NTP_BINDIP -DSUPPORT_SNMP -DSUPPORT_TR069 -DSUPPORT_SSHD -DSUPPORT_DUALIMAGE -DSCM_PACKET_MARK -DSUPPORT_USB_STORAGE -DCONFIG_SUPPORT_SAMBA_VERSION_36 -DSUPPORT_USB_PRINTER -DCONFIG_SUPPORT_IPV6 -DCML_CONVERT_BOOL -DSUPPORT_STATISTICS_NOTIFICATION -DCONFIG_SUPPORT_MEDIA_SERVER -DCONFIG_SUPPORT_DSL -DCONFIG_SUPPORT_VDSL -DCONFIG_SUPPORT_QOS -DCONFIG_SUPPORT_AUTO_SENSE -DCONFIG_SUPPORT_JSON -DCONFIG_BRCM_SUPPORT -DCONFIG_BRCM_SUPPORT_VMUX -DCONFIG_SUPPORT_PRIVATE_HOSTNAME -DHTTP_LOGIN_COOKIE -DCONFIG_SUPPORT_REDIRECT -DCONFIG_SUPPORT_CWMP_RANDOM_PORT -DCONFIG_SUPPORT_WIZARD -DCONFIG_SUPPORT_3G -DCONFIG_SUPPORT_SMS -DCONFIG_SUPPORT_K4203 -DCONFIG_SUPPORT_K5150 -DCONFIG_SUPPORT_3G_BACKUP -DCONFIG_SUPPORT_FIRST_AUTO_DOD -DCONFIG_SUPPORT_VOCS -DSUPPORT_PHONE_PIN_WIZARD -DCONFIG_SUPPORT_WAN_BACKUP -DCONFIG_SUPPORT_ENCRYPTION_AES -DCONFIG_SUPPORT_IPTV -DCONFIG_SUPPORT_FON -DCONFIG_SUPPORT_RUNNER -DCONFIG_SUPPORT_L2TP -DCONFIG_SUPPORT_WIFI_MSSID -DCONFIG_USE_DNSMASQ_IPV6_ONLY -DCONFIG_SUPPORT_WIFI_5G -DCONFIG_SUPPORT_5G_BRCM -DCONFIG_SUPPORT_STATIC_NAT -DSUPPORT_TWO_FXS -DCONFIG_SUPPORT_WEBAPI -DCONFIG_SUPPORT_UBUS -DCONFIG_SUPPORT_PRPL_HL_API -DCONFIG_SUPPORT_IPERF -DCONFIG_SUPPORT_BANDSTEERING -DCONFIG_SUPPORT_WOLAN -DCONFIG_SUPPORT_ENERGY_SAVING -DCONFIG_SUPPORT_SJCL_ENCRYPT -DCONFIG_SUPPORT_CGN -DCONFIG_SUPPORT_SEC_BOOT -DCONFIG_SUPPORT_AUTO_DETECT -DCONFIG_SUPPORT_DSL_FTTH_SWITCH -DCONFIG_SUPPORT_L2TP_CLIENT -DCONFIG_SUPPORT_L2TP_SERVER -DCONFIG_SUPPORT_PPTP_CLIENT -DCONFIG_SUPPORT_PPTP_SERVER -DCONFIG_CRASH_LOG -DCONFIG_SUPPORT_IPSEC_CLIENT -DCONFIG_SUPPORT_NATLOOPBACK -DCONFIG_SUPPORT_PACKAGE -DCONFIG_SUPPORT_BOOSTER_MODE -DCONFIG_PPP_SUPPORT_PADM_PADN -DCONFIG_SUPPORT_TR157 -DCONFIG_SUPPORT_LCM -DCONFIG_SUPPORT_SUPERWIFI -fstack-protector -DCONFIG_SUPPORT_SMARTTHING -DCONFIG_SUPPORT_FWA -DCONFIG_SUPPORT_HA -DCONFIG_SUPPORT_MQTT  -DNONE -DLINUX_FW_EXTRAVERSION=50203 -DNONE -DSUPPORT_RDPA -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE   -DLINUX -DCHIP_63138 -DCONFIG_BCM963138 -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC   -I/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/include -L/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/lib -Werror=return-type -Werror=uninitialized -Wno-date-time -DBCM_RDP -DCONFIG_BCM_MAX_GEM_PORTS=1  -DBRCM_VOICE_SUPPORT=1 -DEPTAPPLOAD=y -DVRG_COUNTRY_CFG_AUSTRALIA=1 -DVRG_COUNTRY_CFG_BELGIUM=1 -DVRG_COUNTRY_CFG_BRAZIL=1 -DVRG_COUNTRY_CFG_CHILE=1 -DVRG_COUNTRY_CFG_CHINA=1 -DVRG_COUNTRY_CFG_CYPRUS=1 -DVRG_COUNTRY_CFG_CZECH=1 -DVRG_COUNTRY_CFG_DENMARK=1 -DVRG_COUNTRY_CFG_ETSI=1 -DVRG_COUNTRY_CFG_FINLAND=1 -DVRG_COUNTRY_CFG_FRANCE=1 -DVRG_COUNTRY_CFG_GERMANY=1 -DVRG_COUNTRY_CFG_HUNGARY=1 -DVRG_COUNTRY_CFG_INDIA=1 -DVRG_COUNTRY_CFG_ITALY=1 -DVRG_COUNTRY_CFG_JAPAN=1 -DVRG_COUNTRY_CFG_NETHERLANDS=1 -DVRG_COUNTRY_CFG_NEW_ZEALAND=1 -DVRG_COUNTRY_CFG_NORTH_AMERICA=1 -DVRG_COUNTRY_CFG_NORWAY=1 -DVRG_COUNTRY_CFG_SPAIN=1 -DVRG_COUNTRY_CFG_SWEDEN=1 -DVRG_COUNTRY_CFG_SWITZERLAND=1 -DVRG_COUNTRY_CFG_TAIWAN=1 -DVRG_COUNTRY_CFG_UK=1 -DBRCM_VOICE_LOW_POWER_IDLE=1 -DVOICE_IPV6_SUPPORT=1 -DBRCM_VOICE_DSPHAL_CAPTURE=1 -Wtype-limits -I/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/userspace/private/apps/voice/endpt/ldx/ldx_apps/dsl/apps/hausware_libs_gateway_nodist_arm/lib/arm_glibc_risc.arm.armv7.conly -I/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/userspace/private/apps/voice/inc -DDMP_TAENDPOINT_1=1 -DDMP_ENDPOINT_1=1 -DDMP_X_BROADCOM_COM_PSTNENDPOINT_1=1 -DDMP_VOICE_SERVICE_1=1  -DNONE -DLINUX_FW_EXTRAVERSION=50203 -DNONE -DSUPPORT_RDPA -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE   -DLINUX -DCHIP_63138 -DCONFIG_BCM963138 -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC   -I/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/include -L/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/lib -Werror=return-type -Werror=uninitialized -Wno-date-time -DBCM_RDP -DCONFIG_BCM_MAX_GEM_PORTS=1 
INCLS = -I. 
DEFS = -DBUILDING_PCAP -DHAVE_CONFIG_H  -D_U_=""
ADDLOBJS = 
ADDLARCHIVEOBJS = 
LIBS = 
CROSSFLAGS=
CFLAGS = -DVFIT -DSC_CUSTOMER=\"VFIT\" -DVOX30 -DPRODUCT_TYPE=\"VOX30\" -DFW_VERSION_PREFIX=\"XS\" -DSERCOMM -DSC_OEM=\"SERCOMM\" -DSC_OEM_OUI=\"BC15AC\" -DSC_SSID_PREFIX=\"Vodafone\" -DSPECIAL_ARPPING_URI_DEFINE -DSUPPORT_RESET_ALL_CONFIG -DLANGUAGE_SUPPORT=\"en\" -DSC_OPCO=\"it\" -DCONFIG_SUPPORT_USER_MANAGEMENT_BY_ENDUSER -DCONFIG_SUPPORT_WEEK_DATE -DSC_MANUFACTUREERURL=\"www.sercomm.com\" -DSC_MODELURL=\"www.vodafone.com\" -DCONFIG_SUPPORT_BLOCKING_ACCESS_TO_NEW_DEVICES -DCONFIG_SUPPORT_ARP_TABLE_FILTER -DCONFIG_SUPPORT_DYNAMIC_CLI_PROMOTE -DSC_HW_VERSION=\"v2\" -DSC_OEM_OUI_STRING=\"_VODAFONE_\" -DBRCM_502 -DCONFIG_SUPPORT_PASSWORD_PROTECTION -DMULTI_LEVEL_CONFIG -DCONFIG_SAFE_NETWORK -DCONFIG_SUPPORT_WEB_PRIVOXY -DCONFIG_SCM_SUPPORT -D__SC_BUILD__ -DVOX30_SFP -DVOX_LED_SPEC -DCONFIG_SUPPORT_NEW_LED_BEHAV -DSC_MIXVENDOR_SPT -DSC_VOIP_SUPPORT_PBX -DSC_VOIP_SUPPORT_SILENT_CALL -D_SC_SPI_FIREWALL_ -DSUPPORT_OMCI -DSUPPORT_WIFI -DCONFIG_SUPPORT_WIFI -DSUPPORT_MIMO -DSUPPORT_WSC -DWMF -DSUPPORT_WIFI_BRCM -DSUPPORT_WIFI_11N_CERTIFICATE -DSUPPORT_WIFI_WPS_CERTIFICATE -DSUPPORT_VOIP -DSUPPORT_ROUTER -DCONFIG_SUPPORT_CHRONY -DSUPPORT_MULTIWAN_DOMAIN -DSUPPORT_MULTIWAN_NTP_BINDIP -DSUPPORT_SNMP -DSUPPORT_TR069 -DSUPPORT_SSHD -DSUPPORT_DUALIMAGE -DSCM_PACKET_MARK -DSUPPORT_USB_STORAGE -DCONFIG_SUPPORT_SAMBA_VERSION_36 -DSUPPORT_USB_PRINTER -DCONFIG_SUPPORT_IPV6 -DCML_CONVERT_BOOL -DSUPPORT_STATISTICS_NOTIFICATION -DCONFIG_SUPPORT_MEDIA_SERVER -DCONFIG_SUPPORT_DSL -DCONFIG_SUPPORT_VDSL -DCONFIG_SUPPORT_QOS -DCONFIG_SUPPORT_AUTO_SENSE -DCONFIG_SUPPORT_JSON -DCONFIG_BRCM_SUPPORT -DCONFIG_BRCM_SUPPORT_VMUX -DCONFIG_SUPPORT_PRIVATE_HOSTNAME -DHTTP_LOGIN_COOKIE -DCONFIG_SUPPORT_REDIRECT -DCONFIG_SUPPORT_CWMP_RANDOM_PORT -DCONFIG_SUPPORT_WIZARD -DCONFIG_SUPPORT_3G -DCONFIG_SUPPORT_SMS -DCONFIG_SUPPORT_K4203 -DCONFIG_SUPPORT_K5150 -DCONFIG_SUPPORT_3G_BACKUP -DCONFIG_SUPPORT_FIRST_AUTO_DOD -DCONFIG_SUPPORT_VOCS -DSUPPORT_PHONE_PIN_WIZARD -DCONFIG_SUPPORT_WAN_BACKUP -DCONFIG_SUPPORT_ENCRYPTION_AES -DCONFIG_SUPPORT_IPTV -DCONFIG_SUPPORT_FON -DCONFIG_SUPPORT_RUNNER -DCONFIG_SUPPORT_L2TP -DCONFIG_SUPPORT_WIFI_MSSID -DCONFIG_USE_DNSMASQ_IPV6_ONLY -DCONFIG_SUPPORT_WIFI_5G -DCONFIG_SUPPORT_5G_BRCM -DCONFIG_SUPPORT_STATIC_NAT -DSUPPORT_TWO_FXS -DCONFIG_SUPPORT_WEBAPI -DCONFIG_SUPPORT_UBUS -DCONFIG_SUPPORT_PRPL_HL_API -DCONFIG_SUPPORT_IPERF -DCONFIG_SUPPORT_BANDSTEERING -DCONFIG_SUPPORT_WOLAN -DCONFIG_SUPPORT_ENERGY_SAVING -DCONFIG_SUPPORT_SJCL_ENCRYPT -DCONFIG_SUPPORT_CGN -DCONFIG_SUPPORT_SEC_BOOT -DCONFIG_SUPPORT_AUTO_DETECT -DCONFIG_SUPPORT_DSL_FTTH_SWITCH -DCONFIG_SUPPORT_L2TP_CLIENT -DCONFIG_SUPPORT_L2TP_SERVER -DCONFIG_SUPPORT_PPTP_CLIENT -DCONFIG_SUPPORT_PPTP_SERVER -DCONFIG_CRASH_LOG -DCONFIG_SUPPORT_IPSEC_CLIENT -DCONFIG_SUPPORT_NATLOOPBACK -DCONFIG_SUPPORT_PACKAGE -DCONFIG_SUPPORT_BOOSTER_MODE -DCONFIG_PPP_SUPPORT_PADM_PADN -DCONFIG_SUPPORT_TR157 -DCONFIG_SUPPORT_LCM -DCONFIG_SUPPORT_SUPERWIFI -fstack-protector -DCONFIG_SUPPORT_SMARTTHING -DCONFIG_SUPPORT_FWA -DCONFIG_SUPPORT_HA -DCONFIG_SUPPORT_MQTT -DFW_VERSION_PREFIX=\"XS\" -DSC_OEM_OUI=\"BC15AC\" -DSC_SSID_PREFIX=\"Vodafone\" -DSPECIAL_ARPPING_URI_DEFINE -DSUPPORT_RESET_ALL_CONFIG -DLANGUAGE_SUPPORT=\"en\" -DSC_OPCO=\"it\" -DCONFIG_SUPPORT_USER_MANAGEMENT_BY_ENDUSER -DCONFIG_SUPPORT_WEEK_DATE -DSC_MANUFACTUREERURL=\"www.sercomm.com\" -DSC_MODELURL=\"www.vodafone.com\" -DCONFIG_SUPPORT_BLOCKING_ACCESS_TO_NEW_DEVICES -DCONFIG_SUPPORT_ARP_TABLE_FILTER -DCONFIG_SUPPORT_DYNAMIC_CLI_PROMOTE -DSC_HW_VERSION=\"v2\" -DSC_OEM_OUI_STRING=\"_VODAFONE_\" -DBRCM_502 -DCONFIG_SUPPORT_PASSWORD_PROTECTION -DMULTI_LEVEL_CONFIG -DCONFIG_SAFE_NETWORK -DCONFIG_SUPPORT_WEB_PRIVOXY -DCONFIG_SCM_SUPPORT -D__SC_BUILD__ -DVOX30_SFP -DVOX_LED_SPEC -DCONFIG_SUPPORT_NEW_LED_BEHAV -DSC_MIXVENDOR_SPT -DSC_VOIP_SUPPORT_PBX -DSC_VOIP_SUPPORT_SILENT_CALL -D_SC_SPI_FIREWALL_ -DSUPPORT_OMCI -DSUPPORT_WIFI -DCONFIG_SUPPORT_WIFI -DSUPPORT_MIMO -DSUPPORT_WSC -DWMF -DSUPPORT_WIFI_BRCM -DSUPPORT_WIFI_11N_CERTIFICATE -DSUPPORT_WIFI_WPS_CERTIFICATE -DSUPPORT_VOIP -DSUPPORT_ROUTER -DCONFIG_SUPPORT_CHRONY -DSUPPORT_MULTIWAN_DOMAIN -DSUPPORT_MULTIWAN_NTP_BINDIP -DSUPPORT_SNMP -DSUPPORT_TR069 -DSUPPORT_SSHD -DSUPPORT_DUALIMAGE -DSCM_PACKET_MARK -DSUPPORT_USB_STORAGE -DCONFIG_SUPPORT_SAMBA_VERSION_36 -DSUPPORT_USB_PRINTER -DCONFIG_SUPPORT_IPV6 -DCML_CONVERT_BOOL -DSUPPORT_STATISTICS_NOTIFICATION -DCONFIG_SUPPORT_MEDIA_SERVER -DCONFIG_SUPPORT_DSL -DCONFIG_SUPPORT_VDSL -DCONFIG_SUPPORT_QOS -DCONFIG_SUPPORT_AUTO_SENSE -DCONFIG_SUPPORT_JSON -DCONFIG_BRCM_SUPPORT -DCONFIG_BRCM_SUPPORT_VMUX -DCONFIG_SUPPORT_PRIVATE_HOSTNAME -DHTTP_LOGIN_COOKIE -DCONFIG_SUPPORT_REDIRECT -DCONFIG_SUPPORT_CWMP_RANDOM_PORT -DCONFIG_SUPPORT_WIZARD -DCONFIG_SUPPORT_3G -DCONFIG_SUPPORT_SMS -DCONFIG_SUPPORT_K4203 -DCONFIG_SUPPORT_K5150 -DCONFIG_SUPPORT_3G_BACKUP -DCONFIG_SUPPORT_FIRST_AUTO_DOD -DCONFIG_SUPPORT_VOCS -DSUPPORT_PHONE_PIN_WIZARD -DCONFIG_SUPPORT_WAN_BACKUP -DCONFIG_SUPPORT_ENCRYPTION_AES -DCONFIG_SUPPORT_IPTV -DCONFIG_SUPPORT_FON -DCONFIG_SUPPORT_RUNNER -DCONFIG_SUPPORT_L2TP -DCONFIG_SUPPORT_WIFI_MSSID -DCONFIG_USE_DNSMASQ_IPV6_ONLY -DCONFIG_SUPPORT_WIFI_5G -DCONFIG_SUPPORT_5G_BRCM -DCONFIG_SUPPORT_STATIC_NAT -DSUPPORT_TWO_FXS -DCONFIG_SUPPORT_WEBAPI -DCONFIG_SUPPORT_UBUS -DCONFIG_SUPPORT_PRPL_HL_API -DCONFIG_SUPPORT_IPERF -DCONFIG_SUPPORT_BANDSTEERING -DCONFIG_SUPPORT_WOLAN -DCONFIG_SUPPORT_ENERGY_SAVING -DCONFIG_SUPPORT_SJCL_ENCRYPT -DCONFIG_SUPPORT_CGN -DCONFIG_SUPPORT_SEC_BOOT -DCONFIG_SUPPORT_AUTO_DETECT -DCONFIG_SUPPORT_DSL_FTTH_SWITCH -DCONFIG_SUPPORT_L2TP_CLIENT -DCONFIG_SUPPORT_L2TP_SERVER -DCONFIG_SUPPORT_PPTP_CLIENT -DCONFIG_SUPPORT_PPTP_SERVER -DCONFIG_CRASH_LOG -DCONFIG_SUPPORT_IPSEC_CLIENT -DCONFIG_SUPPORT_NATLOOPBACK -DCONFIG_SUPPORT_PACKAGE -DCONFIG_SUPPORT_BOOSTER_MODE -DCONFIG_PPP_SUPPORT_PADM_PADN -DCONFIG_SUPPORT_TR157 -DCONFIG_SUPPORT_LCM -DCONFIG_SUPPORT_SUPERWIFI -fstack-protector -DCONFIG_SUPPORT_SMARTTHING -DCONFIG_SUPPORT_FWA -DCONFIG_SUPPORT_HA -DCONFIG_SUPPORT_MQTT  -DNONE -DLINUX_FW_EXTRAVERSION=50203 -DNONE -DSUPPORT_RDPA -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE   -DLINUX -DCHIP_63138 -DCONFIG_BCM963138 -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC   -I/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/include -L/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/lib -Werror=return-type -Werror=uninitialized -Wno-date-time -DBCM_RDP -DCONFIG_BCM_MAX_GEM_PORTS=1  -DBRCM_VOICE_SUPPORT=1 -DEPTAPPLOAD=y -DVRG_COUNTRY_CFG_AUSTRALIA=1 -DVRG_COUNTRY_CFG_BELGIUM=1 -DVRG_COUNTRY_CFG_BRAZIL=1 -DVRG_COUNTRY_CFG_CHILE=1 -DVRG_COUNTRY_CFG_CHINA=1 -DVRG_COUNTRY_CFG_CYPRUS=1 -DVRG_COUNTRY_CFG_CZECH=1 -DVRG_COUNTRY_CFG_DENMARK=1 -DVRG_COUNTRY_CFG_ETSI=1 -DVRG_COUNTRY_CFG_FINLAND=1 -DVRG_COUNTRY_CFG_FRANCE=1 -DVRG_COUNTRY_CFG_GERMANY=1 -DVRG_COUNTRY_CFG_HUNGARY=1 -DVRG_COUNTRY_CFG_INDIA=1 -DVRG_COUNTRY_CFG_ITALY=1 -DVRG_COUNTRY_CFG_JAPAN=1 -DVRG_COUNTRY_CFG_NETHERLANDS=1 -DVRG_COUNTRY_CFG_NEW_ZEALAND=1 -DVRG_COUNTRY_CFG_NORTH_AMERICA=1 -DVRG_COUNTRY_CFG_NORWAY=1 -DVRG_COUNTRY_CFG_SPAIN=1 -DVRG_COUNTRY_CFG_SWEDEN=1 -DVRG_COUNTRY_CFG_SWITZERLAND=1 -DVRG_COUNTRY_CFG_TAIWAN=1 -DVRG_COUNTRY_CFG_UK=1 -DBRCM_VOICE_LOW_POWER_IDLE=1 -DVOICE_IPV6_SUPPORT=1 -DBRCM_VOICE_DSPHAL_CAPTURE=1 -Wtype-limits -I/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/userspace/private/apps/voice/endpt/ldx/ldx_apps/dsl/apps/hausware_libs_gateway_nodist_arm/lib/arm_glibc_risc.arm.armv7.conly -I/home/edward/370306/VOX30_SERCOMM_VFIT_370306/drivers/DSL/bcm963xx_5.02L03/userspace/private/apps/voice/inc -DDMP_TAENDPOINT_1=1 -DDMP_ENDPOINT_1=1 -DDMP_X_BROADCOM_COM_PSTNENDPOINT_1=1 -DDMP_VOICE_SERVICE_1=1  -DNONE -DLINUX_FW_EXTRAVERSION=50203 -DNONE -DSUPPORT_RDPA -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE   -DLINUX -DCHIP_63138 -DCONFIG_BCM963138 -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Werror=return-type -Werror=uninitialized -Wno-date-time -Wall -Darm -g -fPIC   -I/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/include -L/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/lib -Werror=return-type -Werror=uninitialized -Wno-date-time -DBCM_RDP -DCONFIG_BCM_MAX_GEM_PORTS=1    ${CROSSFLAGS}
LDFLAGS =  ${CROSSFLAGS}
DYEXT = so
V_RPATH_OPT = -Wl,-rpath,
DEPENDENCY_CFLAG = 
PROG=libpcap

# Standard CFLAGS
FULL_CFLAGS = $(CCOPT) $(INCLS) $(DEFS) $(CFLAGS)

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
RANLIB = /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.24-binutils-2.25//usr/bin/arm-linux-ranlib

LEX = flex
YACC = bison -y

# Explicitly define compilation rule since SunOS 4's make doesn't like gcc.
# Also, gcc does not remove the .o before forking 'as', which can be a
# problem if you don't own the file but can write to the directory.
.c.o:
	@rm -f $@
	$(CC) $(FULL_CFLAGS) -c $(srcdir)/$*.c

PSRC =	pcap-linux.c pcap-usb-linux.c   pcap-netfilter-linux.c 
FSRC =  fad-getad.c
SSRC =  
CSRC =	pcap.c inet.c fad-helpers.c gencode.c optimize.c nametoaddr.c \
	etherent.c savefile.c sf-pcap.c sf-pcap-ng.c pcap-common.c \
	bpf_image.c bpf_dump.c
GENSRC = scanner.c grammar.c bpf_filter.c version.c
LIBOBJS = 

SRC =	$(PSRC) $(FSRC) $(CSRC) $(SSRC) $(GENSRC)

# We would like to say "OBJ = $(SRC:.c=.o)" but Ultrix's make cannot
# hack the extra indirection
OBJ =	$(PSRC:.c=.o) $(FSRC:.c=.o) $(CSRC:.c=.o) $(SSRC:.c=.o) $(GENSRC:.c=.o) $(LIBOBJS)
PUBHDR = \
	pcap.h \
	pcap-bpf.h \
	pcap-namedb.h \
	pcap/bpf.h \
	pcap/bluetooth.h \
	pcap/can_socketcan.h \
	pcap/dlt.h \
	pcap/export-defs.h \
	pcap/ipnet.h \
	pcap/namedb.h \
	pcap/nflog.h \
	pcap/pcap.h \
	pcap/sll.h \
	pcap/vlan.h \
	pcap/usb.h

HDR = $(PUBHDR) \
	arcnet.h \
	atmuni31.h \
	ethertype.h \
	extract.h \
	gencode.h \
	ieee80211.h \
	llc.h \
	nametoaddr.h \
	nlpid.h \
	pcap-common.h \
	pcap-int.h \
	pcap-stdinc.h \
	portability.h \
	ppp.h \
	sf-pcap.h \
	sf-pcap-ng.h \
	sunatmpos.h

TESTS = \
	 \
	capturetest \
	can_set_rfmon_test \
	filtertest \
	findalldevstest \
	opentest \
	reactivatetest \
	selpolltest

TESTS_SRC = \
	tests/valgrindtest.c \
	tests/capturetest.c \
	tests/can_set_rfmon_test.c \
	tests/filtertest.c \
	tests/findalldevstest.c \
	tests/opentest.c \
	tests/reactivatetest.c \
	tests/selpolltest.c

GENHDR = \
	scanner.h grammar.h pcap_version.h

TAGFILES = \
	$(SRC) $(HDR)

CLEANFILES = $(OBJ) libpcap.* $(TESTS) \
	$(PROG)-`cat $(srcdir)/VERSION`.tar.gz $(GENSRC) $(GENHDR) \
	lex.yy.c pcap-config

MAN1 = pcap-config.1

MAN3PCAP_EXPAND = \
	pcap.3pcap.in \
	pcap_compile.3pcap.in \
	pcap_datalink.3pcap.in \
	pcap_dump_open.3pcap.in \
	pcap_get_tstamp_precision.3pcap.in \
	pcap_list_datalinks.3pcap.in \
	pcap_list_tstamp_types.3pcap.in \
	pcap_open_dead.3pcap.in \
	pcap_open_offline.3pcap.in \
	pcap_set_tstamp_precision.3pcap.in \
	pcap_set_tstamp_type.3pcap.in

MAN3PCAP_NOEXPAND = \
	pcap_activate.3pcap \
	pcap_breakloop.3pcap \
	pcap_can_set_rfmon.3pcap \
	pcap_close.3pcap \
	pcap_create.3pcap \
	pcap_datalink_name_to_val.3pcap \
	pcap_datalink_val_to_name.3pcap \
	pcap_dump.3pcap \
	pcap_dump_close.3pcap \
	pcap_dump_file.3pcap \
	pcap_dump_flush.3pcap \
	pcap_dump_ftell.3pcap \
	pcap_file.3pcap \
	pcap_fileno.3pcap \
	pcap_findalldevs.3pcap \
	pcap_freecode.3pcap \
	pcap_get_selectable_fd.3pcap \
	pcap_geterr.3pcap \
	pcap_inject.3pcap \
	pcap_is_swapped.3pcap \
	pcap_lib_version.3pcap \
	pcap_lookupdev.3pcap \
	pcap_lookupnet.3pcap \
	pcap_loop.3pcap \
	pcap_major_version.3pcap \
	pcap_next_ex.3pcap \
	pcap_offline_filter.3pcap \
	pcap_open_live.3pcap \
	pcap_set_buffer_size.3pcap \
	pcap_set_datalink.3pcap \
	pcap_set_immediate_mode.3pcap \
	pcap_set_promisc.3pcap \
	pcap_set_rfmon.3pcap \
	pcap_set_snaplen.3pcap \
	pcap_set_timeout.3pcap \
	pcap_setdirection.3pcap \
	pcap_setfilter.3pcap \
	pcap_setnonblock.3pcap \
	pcap_snapshot.3pcap \
	pcap_stats.3pcap \
	pcap_statustostr.3pcap \
	pcap_strerror.3pcap \
	pcap_tstamp_type_name_to_val.3pcap \
	pcap_tstamp_type_val_to_name.3pcap

MAN3PCAP = $(MAN3PCAP_NOEXPAND) $(MAN3PCAP_EXPAND:.in=)

MANFILE = \
	pcap-savefile.manfile.in

MANMISC = \
	pcap-filter.manmisc.in \
	pcap-linktype.manmisc.in \
	pcap-tstamp.manmisc.in

EXTRA_DIST = \
	$(TESTS_SRC) \
	CHANGES \
	ChmodBPF/ChmodBPF \
	ChmodBPF/StartupParameters.plist \
	CREDITS \
	CMakeLists.txt \
	GenVersion.bat \
	INSTALL.txt \
	LICENSE \
	Makefile.in \
	Makefile-devel-adds \
	README \
	README.aix \
	README.dag \
	README.hpux \
	README.linux \
	README.macosx \
	README.septel \
	README.sita \
	README.tru64 \
	README.Win32 \
	SUNOS4/nit_if.o.sparc \
	SUNOS4/nit_if.o.sun3 \
	SUNOS4/nit_if.o.sun4c.4.0.3c \
	TODO \
	VERSION \
	aclocal.m4 \
	bpf/net/bpf_filter.c \
	chmod_bpf \
	cmakeconfig.h.in \
	cmake/preconfigure.cmake \
	config/have_siocglifconf.c \
	config.guess \
	config.h.in \
	config.sub \
	configure \
	configure.ac \
	dlpisubs.c \
	dlpisubs.h \
	fad-getad.c \
	fad-gifc.c \
	fad-glifc.c \
	fad-helpers.c \
	gen_version_c.sh \
	gen_version_header.sh \
	grammar.y \
	install-sh \
	lbl/os-aix4.h \
	lbl/os-aix7.h \
	lbl/os-hpux11.h \
	lbl/os-osf4.h \
	lbl/os-osf5.h \
	lbl/os-solaris2.h \
	lbl/os-sunos4.h \
	lbl/os-ultrix4.h \
	missing/getopt.c \
	missing/getopt.h \
	missing/snprintf.c \
	missing/strtok_r.c \
	missing/win_snprintf.c \
	mkdep \
	msdos/bin2c.c \
	msdos/common.dj \
	msdos/makefile \
	msdos/makefile.dj \
	msdos/makefile.wc \
	msdos/ndis2.c \
	msdos/ndis2.h \
	msdos/ndis_0.asm \
	msdos/pkt_rx0.asm \
	msdos/pkt_rx1.s \
	msdos/pktdrvr.c \
	msdos/pktdrvr.h \
	msdos/readme.dos \
	org.tcpdump.chmod_bpf.plist \
	pcap-bpf.c \
	pcap-bt-linux.c \
	pcap-bt-linux.h \
	pcap-bt-monitor-linux.c \
	pcap-bt-monitor-linux.h \
	pcap-config.in \
	pcap-dag.c \
	pcap-dag.h \
	pcap-dbus.c \
	pcap-dbus.h \
	pcap-dlpi.c \
	pcap-dos.c \
	pcap-dos.h \
	pcap-enet.c \
	pcap-int.h \
	pcap-libdlpi.c \
	pcap-linux.c \
	pcap-namedb.h \
	pcap-new.c \
	pcap-netfilter-linux.c \
	pcap-netfilter-linux.h \
	pcap-nit.c \
	pcap-null.c \
	pcap-pf.c \
	pcap-rpcap.c \
	pcap-rpcap.h \
	pcap-septel.c \
	pcap-septel.h \
	pcap-sita.h \
	pcap-sita.c \
	pcap-sita.html \
	pcap-snf.c \
	pcap-snf.h \
	pcap-snit.c \
	pcap-snoop.c \
	pcap-tc.c \
	pcap-tc.h \
	pcap-usb-linux.c \
	pcap-usb-linux.h \
	pcap-win32.c \
	remote-ext.h \
	sockutils.c \
	sockutils.h \
	scanner.l \
	tests/CMakeLists.txt \
	pcap_version.h.in \
	Win32/Include/Gnuc.h \
	Win32/Include/net/if.h \
	Win32/Prj/wpcap.sln \
	Win32/Prj/wpcap.vcxproj \
	Win32/Prj/wpcap.vcxproj.filters

all: libpcap.a shared pcap-config

libpcap.a: $(OBJ)
	@rm -f $@
	$(AR) rc $@ $(OBJ) $(ADDLARCHIVEOBJS)
	$(RANLIB) $@

shared: libpcap.$(DYEXT)

libpcap.so: $(OBJ)
	@rm -f $@
	VER=`cat $(srcdir)/VERSION`; \
	MAJOR_VER=`sed 's/\([0-9][0-9]*\)\..*/\1/' $(srcdir)/VERSION`; \
	$(CC) -shared -Wl,-soname,$@.$$MAJOR_VER $(LDFLAGS) \
	    -o $@.$$VER $(OBJ) $(ADDLOBJS) $(LIBS)

#
# The following rule succeeds, but the result is untested.
#
# In Mac OS X, the libpcap dylib has the name "libpcap.A.dylib", with
# its full path as the install_name, and with the compatibility and
# current version both set to 1.  The compatibility version is set to
# 1 so that programs built with a newer version of the library will run
# against older versions; multi-platform software probably will fail if
# it uses APIs added in the newer version, but Mac OS X-specific software
# will use weak linking and check at run time whether those APIs are
# available.
#
# We also use "A" as the major version, and 1 as the compatibility version,
# but set the current version to the value in VERSION, with any non-numeric
# stuff stripped off (the compatibility and current version must be of the
# form X[.Y[.Z]], with Y and Z possibly absent, and with all components
# numeric).
#
libpcap.dylib: $(OBJ)
	rm -f libpcap*.dylib
	VER=`cat $(srcdir)/VERSION`; \
	MAJOR_VER=A; \
	COMPAT_VER=1; \
	CURRENT_VER=`sed 's/[^0-9.].*$$//' $(srcdir)/VERSION`; \
	$(CC) -dynamiclib -undefined error $(LDFLAGS) \
	    -o libpcap.$$VER.dylib $(OBJ) $(ADDLOBJS) $(LIBS) \
	    -install_name $(libdir)/libpcap.$$MAJOR_VER.dylib \
	    -compatibility_version $$COMPAT_VER \
	    -current_version $$CURRENT_VER

#
# The HP-UX linker manual says that the convention for a versioned library
# is libXXX.{number}, not libXXX.sl.{number}.  That appears to be the case
# on at least one HP-UX 11.00 system; libXXX.sl is a symlink to
# libXXX.{number}.
#
# The manual also says "library-level versioning" (think "sonames") was
# added in HP-UX 10.0.
#
# XXX - this assumes we're using the HP linker, rather than the GNU
# linker, even with GCC.
#
libpcap.sl: $(OBJ)
	@MAJOR_VER=`sed 's/\([0-9][0-9]*\)\..*/\1/' $(srcdir)/VERSION`; \
	rm -f libpcap.$$MAJOR_VER
	MAJOR_VER=`sed 's/\([0-9][0-9]*\)\..*/\1/' $(srcdir)/VERSION`; \
	ld -b $(LDFLAGS) -o libpcap.$$MAJOR_VER +h libpcap.$$MAJOR_VER \
	    $(OBJ) $(ADDLOBJS) $(LIBS)

#
# AIX is different from everybody else.  A shared library is an archive
# library with one or more shared-object components.  We still build a
# normal static archive library on AIX, for the benefit of the traditional
# scheme of building libpcap and tcpdump in subdirectories of the
# same directory, with tcpdump statically linked with the libpcap
# in question, but we also build a shared library as "libpcap.shareda"
# and install *it*, rather than the static library, as "libpcap.a".
#
libpcap.shareda: $(OBJ)
	@rm -f $@ shr.o
	$(CC) -shared -o shr.o $(OBJ) $(ADDLOBJS) $(LDFLAGS) $(LIBS)
	$(AR) rc $@ shr.o

#
# For platforms that don't support shared libraries (or on which we
# don't support shared libraries).
#
libpcap.none:

scanner.c: $(srcdir)/scanner.l
	$(LEX) -P pcap_ --header-file=scanner.h --nounput -o scanner.c $<
scanner.h: scanner.c
## Recover from the removal of $@
	@if test -f $@; then :; else \
		rm -f scanner.c; \
		$(MAKE) $(MAKEFLAGS) scanner.c; \
	fi

scanner.o: scanner.c grammar.h
	$(CC) $(FULL_CFLAGS) -c scanner.c

pcap.o: pcap_version.h

grammar.c: $(srcdir)/grammar.y
	$(YACC) -p pcap_ -o grammar.c -d $<
grammar.h: grammar.c
## Recover from the removal of $@
	@if test -f $@; then :; else \
		rm -f grammar.c; \
		$(MAKE) $(MAKEFLAGS) grammar.c; \
	fi

grammar.o: grammar.c
	$(CC) $(FULL_CFLAGS) -c grammar.c

gencode.o: $(srcdir)/gencode.c grammar.h scanner.h
	$(CC) $(FULL_CFLAGS) -c $(srcdir)/gencode.c

version.o: version.c
	$(CC) $(FULL_CFLAGS) -c version.c

snprintf.o: $(srcdir)/missing/snprintf.c
	$(CC) $(FULL_CFLAGS) -o $@ -c $(srcdir)/missing/snprintf.c

strtok_r.o: $(srcdir)/missing/strtok_r.c
	$(CC) $(FULL_CFLAGS) -o $@ -c $(srcdir)/missing/strtok_r.c

version.c: $(srcdir)/VERSION $(srcdir)/gen_version_c.sh
	#
	# Older programs import this if they want to show the
	# libpcap version number, rather than calling
	# pcap_lib_version(), so we need to export it.
	#
	@rm -f $@
	$(srcdir)/gen_version_c.sh $(srcdir)/VERSION $@

pcap_version.h: $(srcdir)/VERSION $(srcdir)/pcap_version.h.in $(srcdir)/gen_version_header.sh
	@rm -f $@
	$(srcdir)/gen_version_header.sh $(srcdir)/VERSION $(srcdir)/pcap_version.h.in $@

bpf_filter.c: $(srcdir)/bpf/net/bpf_filter.c
	rm -f bpf_filter.c
	ln -s $(srcdir)/bpf/net/bpf_filter.c bpf_filter.c

bpf_filter.o: bpf_filter.c
	$(CC) $(FULL_CFLAGS) -c bpf_filter.c

#
# Generate the pcap-config script.
#
# Some Makes, e.g. AIX Make and Solaris Make, can't handle "--file=$@.tmp:$<";
# for example, the Solaris 9 make man page says
#
#	Because make assigns $< and $* as it would for implicit rules
#	(according to the suffixes list and the directory contents),
#	they may be unreliable when used within explicit target entries.
#
# and this is an explicit target entry.
#
# Therefore, instead of using $<, we explicitly put in $(srcdir)/pcap-config.in.
#
pcap-config: $(srcdir)/pcap-config.in ./config.status
	@rm -f $@ $@.tmp
	./config.status --file=$@.tmp:$(srcdir)/pcap-config.in
	mv $@.tmp $@
	chmod a+x $@

#
# Test programs - not built by default, and not installed.
#
tests: $(TESTS)

capturetest: tests/capturetest.c libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o capturetest $(srcdir)/tests/capturetest.c libpcap.a $(LIBS)

can_set_rfmon_test: tests/can_set_rfmon_test.c libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o can_set_rfmon_test $(srcdir)/tests/can_set_rfmon_test.c libpcap.a $(LIBS)

filtertest: tests/filtertest.c libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o filtertest $(srcdir)/tests/filtertest.c libpcap.a $(LIBS)

findalldevstest: tests/findalldevstest.c libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o findalldevstest $(srcdir)/tests/findalldevstest.c libpcap.a $(LIBS)

opentest: tests/opentest.c libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o opentest $(srcdir)/tests/opentest.c libpcap.a $(LIBS)

reactivatetest: tests/reactivatetest.c libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o reactivatetest $(srcdir)/tests/reactivatetest.c libpcap.a $(LIBS)

selpolltest: tests/selpolltest.c libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o selpolltest $(srcdir)/tests/selpolltest.c libpcap.a $(LIBS)

valgrindtest: tests/valgrindtest.c libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o valgrindtest $(srcdir)/tests/valgrindtest.c libpcap.a $(LIBS)

install: install-shared install-archive pcap-config
	[ -d $(DESTDIR)$(libdir) ] || \
	    (mkdir -p $(DESTDIR)$(libdir); chmod 755 $(DESTDIR)$(libdir))
	[ -d $(DESTDIR)$(includedir) ] || \
	    (mkdir -p $(DESTDIR)$(includedir); chmod 755 $(DESTDIR)$(includedir))
	[ -d $(DESTDIR)$(includedir)/pcap ] || \
	    (mkdir -p $(DESTDIR)$(includedir)/pcap; chmod 755 $(DESTDIR)$(includedir)/pcap)
	[ -d $(DESTDIR)$(mandir)/man1 ] || \
	    (mkdir -p $(DESTDIR)$(mandir)/man1; chmod 755 $(DESTDIR)$(mandir)/man1)
	[ -d $(DESTDIR)$(mandir)/man3 ] || \
	    (mkdir -p $(DESTDIR)$(mandir)/man3; chmod 755 $(DESTDIR)$(mandir)/man3)
	[ -d $(DESTDIR)$(mandir)/man5 ] || \
	    (mkdir -p $(DESTDIR)$(mandir)/man5; chmod 755 $(DESTDIR)$(mandir)/man5)
	[ -d $(DESTDIR)$(mandir)/man7 ] || \
	    (mkdir -p $(DESTDIR)$(mandir)/man7; chmod 755 $(DESTDIR)$(mandir)/man7)
	for i in $(PUBHDR); do \
		$(INSTALL_DATA) $(srcdir)/$$i \
		    $(DESTDIR)$(includedir)/$$i; done
	[ -d $(DESTDIR)$(bindir) ] || \
	    (mkdir -p $(DESTDIR)$(bindir); chmod 755 $(DESTDIR)$(bindir))
	$(INSTALL_PROGRAM) pcap-config $(DESTDIR)$(bindir)/pcap-config
	for i in $(MAN1); do \
		$(INSTALL_DATA) $(srcdir)/$$i \
		    $(DESTDIR)$(mandir)/man1/$$i; done
	for i in $(MAN3PCAP_NOEXPAND); do \
		$(INSTALL_DATA) $(srcdir)/$$i \
		    $(DESTDIR)$(mandir)/man3/$$i; done
	for i in $(MAN3PCAP_EXPAND:.in=); do \
		$(INSTALL_DATA) $$i \
		    $(DESTDIR)$(mandir)/man3/$$i; done
	(cd $(DESTDIR)$(mandir)/man3 && \
	rm -f pcap_datalink_val_to_description.3pcap && \
	$(LN_S) pcap_datalink_val_to_name.3pcap \
		 pcap_datalink_val_to_description.3pcap && \
	rm -f pcap_dump_fopen.3pcap && \
	$(LN_S) pcap_dump_open.3pcap pcap_dump_fopen.3pcap && \
	rm -f pcap_freealldevs.3pcap && \
	$(LN_S) pcap_findalldevs.3pcap pcap_freealldevs.3pcap && \
	rm -f pcap_perror.3pcap && \
	$(LN_S) pcap_geterr.3pcap pcap_perror.3pcap && \
	rm -f pcap_sendpacket.3pcap && \
	$(LN_S) pcap_inject.3pcap pcap_sendpacket.3pcap && \
	rm -f pcap_free_datalinks.3pcap && \
	$(LN_S) pcap_list_datalinks.3pcap pcap_free_datalinks.3pcap && \
	rm -f pcap_free_tstamp_types.3pcap && \
	$(LN_S) pcap_list_tstamp_types.3pcap pcap_free_tstamp_types.3pcap && \
	rm -f pcap_dispatch.3pcap && \
	$(LN_S) pcap_loop.3pcap pcap_dispatch.3pcap && \
	rm -f pcap_minor_version.3pcap && \
	$(LN_S) pcap_major_version.3pcap pcap_minor_version.3pcap && \
	rm -f pcap_next.3pcap && \
	$(LN_S) pcap_next_ex.3pcap pcap_next.3pcap && \
	rm -f pcap_open_dead_with_tstamp_precision.3pcap && \
	$(LN_S) pcap_open_dead.3pcap \
		 pcap_open_dead_with_tstamp_precision.3pcap && \
	rm -f pcap_open_offline_with_tstamp_precision.3pcap && \
	$(LN_S) pcap_open_offline.3pcap pcap_open_offline_with_tstamp_precision.3pcap && \
	rm -f pcap_fopen_offline.3pcap && \
	$(LN_S) pcap_open_offline.3pcap pcap_fopen_offline.3pcap && \
	rm -f pcap_fopen_offline_with_tstamp_precision.3pcap && \
	$(LN_S) pcap_open_offline.3pcap pcap_fopen_offline_with_tstamp_precision.3pcap && \
	rm -f pcap_tstamp_type_val_to_description.3pcap && \
	$(LN_S) pcap_tstamp_type_val_to_name.3pcap pcap_tstamp_type_val_to_description.3pcap && \
	rm -f pcap_getnonblock.3pcap && \
	$(LN_S) pcap_setnonblock.3pcap pcap_getnonblock.3pcap)
	for i in $(MANFILE); do \
		$(INSTALL_DATA) `echo $$i | sed 's/.manfile.in/.manfile/'` \
		    $(DESTDIR)$(mandir)/man5/`echo $$i | sed 's/.manfile.in/.5/'`; done
	for i in $(MANMISC); do \
		$(INSTALL_DATA) `echo $$i | sed 's/.manmisc.in/.manmisc/'` \
		    $(DESTDIR)$(mandir)/man7/`echo $$i | sed 's/.manmisc.in/.7/'`; done

install-shared: install-shared-$(DYEXT)
install-shared-so: libpcap.so
	[ -d $(DESTDIR)$(libdir) ] || \
	    (mkdir -p $(DESTDIR)$(libdir); chmod 755 $(DESTDIR)$(libdir))
	VER=`cat $(srcdir)/VERSION`; \
	MAJOR_VER=`sed 's/\([0-9][0-9]*\)\..*/\1/' $(srcdir)/VERSION`; \
	$(INSTALL_PROGRAM) libpcap.so.$$VER $(DESTDIR)$(libdir)/libpcap.so.$$VER; \
	ln -sf libpcap.so.$$VER $(DESTDIR)$(libdir)/libpcap.so.$$MAJOR_VER; \
	ln -sf libpcap.so.$$MAJOR_VER $(DESTDIR)$(libdir)/libpcap.so
install-shared-dylib: libpcap.dylib
	[ -d $(DESTDIR)$(libdir) ] || \
	    (mkdir -p $(DESTDIR)$(libdir); chmod 755 $(DESTDIR)$(libdir))
	VER=`cat $(srcdir)/VERSION`; \
	MAJOR_VER=A; \
	$(INSTALL_PROGRAM) libpcap.$$VER.dylib $(DESTDIR)$(libdir)/libpcap.$$VER.dylib; \
	ln -sf libpcap.$$VER.dylib $(DESTDIR)$(libdir)/libpcap.$$MAJOR_VER.dylib; \
	ln -sf libpcap.$$MAJOR_VER.dylib $(DESTDIR)$(libdir)/libpcap.dylib
install-shared-sl: libpcap.sl
	[ -d $(DESTDIR)$(libdir) ] || \
	    (mkdir -p $(DESTDIR)$(libdir); chmod 755 $(DESTDIR)$(libdir))
	MAJOR_VER=`sed 's/\([0-9][0-9]*\)\..*/\1/' $(srcdir)/VERSION`; \
	$(INSTALL_PROGRAM) libpcap.$$MAJOR_VER $(DESTDIR)$(libdir)
	ln -sf libpcap.$$MAJOR_VER $(DESTDIR)$(libdir)/libpcap.sl
install-shared-shareda: libpcap.shareda
	#
	# AIX shared libraries are weird.  They're archive libraries
	# with one or more shared object components.
	#
	[ -d $(DESTDIR)$(libdir) ] || \
	    (mkdir -p $(DESTDIR)$(libdir); chmod 755 $(DESTDIR)$(libdir))
	$(INSTALL_PROGRAM) libpcap.shareda $(DESTDIR)$(libdir)/libpcap.a
install-shared-none:

install-archive: install-archive-$(DYEXT)
install-archive-so install-archive-dylib install-archive-sl install-archive-none: libpcap.a
	#
	# Most platforms have separate suffixes for shared and
	# archive libraries, so we install both.
	#
	[ -d $(DESTDIR)$(libdir) ] || \
	    (mkdir -p $(DESTDIR)$(libdir); chmod 755 $(DESTDIR)$(libdir))
	$(INSTALL_DATA) libpcap.a $(DESTDIR)$(libdir)/libpcap.a
	$(RANLIB) $(DESTDIR)$(libdir)/libpcap.a
install-archive-shareda:
	#
	# AIX, however, doesn't, so we don't install the archive
	# library on AIX.
	#

uninstall: uninstall-shared
	rm -f $(DESTDIR)$(libdir)/libpcap.a
	for i in $(PUBHDR); do \
		rm -f $(DESTDIR)$(includedir)/$$i; done
	-rmdir $(DESTDIR)$(includedir)/pcap
	rm -f $(DESTDIR)/$(bindir)/pcap-config
	for i in $(MAN1); do \
		rm -f $(DESTDIR)$(mandir)/man1/$$i; done
	for i in $(MAN3PCAP); do \
		rm -f $(DESTDIR)$(mandir)/man3/$$i; done
	rm -f $(DESTDIR)$(mandir)/man3/pcap_datalink_val_to_description.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_dump_fopen.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_freealldevs.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_perror.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_sendpacket.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_free_datalinks.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_free_tstamp_types.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_dispatch.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_minor_version.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_next.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_open_dead_with_tstamp_precision.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_open_offline_with_tstamp_precision.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_fopen_offline.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_fopen_offline_with_tstamp_precision.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_getnonblock.3pcap
	rm -f $(DESTDIR)$(mandir)/man3/pcap_tstamp_type_val_to_description.3pcap
	for i in $(MANFILE); do \
		rm -f $(DESTDIR)$(mandir)/man5/`echo $$i | sed 's/.manfile.in/.5/'`; done
	for i in $(MANMISC); do \
		rm -f $(DESTDIR)$(mandir)/man7/`echo $$i | sed 's/.manmisc.in/.7/'`; done

uninstall-shared: uninstall-shared-$(DYEXT)
uninstall-shared-so:
	VER=`cat $(srcdir)/VERSION`; \
	MAJOR_VER=`sed 's/\([0-9][0-9]*\)\..*/\1/' $(srcdir)/VERSION`; \
	rm -f $(DESTDIR)$(libdir)/libpcap.so.$$VER; \
	rm -f $(DESTDIR)$(libdir)/libpcap.so.$$MAJOR_VER; \
	rm -f $(DESTDIR)$(libdir)/libpcap.so
uninstall-shared-dylib:
	VER=`cat $(srcdir)/VERSION`; \
	MAJOR_VER=A; \
	rm -f $(DESTDIR)$(libdir)/libpcap.$$VER.dylib; \
	rm -f $(DESTDIR)$(libdir)/libpcap.$$MAJOR_VER.dylib; \
	rm -f $(DESTDIR)$(libdir)/libpcap.dylib
uninstall-shared-sl:
	MAJOR_VER=`sed 's/\([0-9][0-9]*\)\..*/\1/' $(srcdir)/VERSION`; \
	rm -f $(DESTDIR)$(libdir)/libpcap.$$MAJOR_VER; \
	rm -f $(DESTDIR)$(libdir)/libpcap.sl
uninstall-shared-shareda:
	rm -f $(DESTDIR)$(libdir)/libpcap.a
uninstall-shared-none:

clean:
	rm -f $(CLEANFILES)

distclean: clean
	rm -f Makefile config.cache config.log config.status \
	    config.h gnuc.h net os-proto.h bpf_filter.c pcap-config \
	    stamp-h stamp-h.in
	rm -f $(MAN3PCAP_EXPAND:.in=) $(MANFILE:.in=) $(MANMISC:.in=)
	rm -rf autom4te.cache

extags: $(TAGFILES)
	ctags $(TAGFILES)

tags: $(TAGFILES)
	ctags -wtd $(TAGFILES)

releasetar:
	@cwd=`pwd` ; dir=`basename $$cwd` ; name=$(PROG)-`cat VERSION` ; \
	   mkdir $$name; \
	   tar cf - $(CSRC) $(HDR) $(MAN1) $(MAN3PCAP_EXPAND) \
	      $(MAN3PCAP_NOEXPAND) $(MANFILE) $(MANMISC) $(EXTRA_DIST) | \
	      (cd $$name; tar xf -); \
	   tar -c -z -f $$name.tar.gz $$name; \
	   rm -rf $$name

depend:	$(GENSRC) $(GENHDR) bpf_filter.c
	$(MKDEP) -c $(CC) -m $(CFLAGS) $(DEPENDENCY_CFLAG) $(DEFS) $(INCLS) $(SRC)
