
    
    
    
    
ifeq ($(CONFIG_SUPPORT_FON), 1)
ifeq ($(CONFIG_SUPPORT_DSL), 1)
ifeq ($(BRCM_416), 1)
ifeq ($(CONFIG_SUPPORT_5G_QD), 1)
#export X_PRIVATE_LIBS = -lutility -lmd5 -lsex_crypt -lcal -lhcal -lfwutil -lixml -lnv -lrcl -lsal -lfonrpc -lsecxal -lwlapi -lm -lcrypt -lcrypto -lcml_api -lethswctl -lcms_msg -lcms_util -lcms_boardctl -lxdslctl -latmctl -lwlctl -lbcm_crc -lbcm_flashutil -lwlbcmshared -lwlbcmcrypto -lqcsapi_client -lubus -lubox
else
#export X_PRIVATE_LIBS = -lutility -lmd5 -lsex_crypt -lcal -lhcal -lfwutil -lixml -lnv -lrcl -lsal -lfonrpc -lsecxal -lwlapi -lm -lcrypt -lcrypto -lcml_api -lethswctl -lcms_msg -lcms_util -lcms_boardctl -lxdslctl -latmctl -lwlctl -lbcm_crc -lbcm_flashutil -lwlbcmshared -lwlbcmcrypto
endif
else
#export X_PRIVATE_LIBS = -lutility -lmd5 -lsex_crypt -lcal -lhcal -lfwutil -lixml -lnv -lrcl -lsal -lfonrpc -lsecxal -lwlapi -lm -lcrypt -lcrypto -lcml_api -lethswctl -lcms_msg -lcms_util -lcms_boardctl -lxdslctl -latmctl -lwlctl
endif
else
#export X_PRIVATE_LIBS = -lutility -lmd5 -lsex_crypt -lcal -lhcal -lfwutil -lixml -lnv -lrcl -lsal -lfonrpc -lsecxal -lwlapi -lm -lcrypt -lcrypto -lcml_api -lethswctl -lcms_msg -lcms_util -lcms_boardctl -lwlctl
endif
else
ifeq ($(CONFIG_SUPPORT_DSL), 1)
#export X_PRIVATE_LIBS = -lutility -lmd5 -lsex_crypt -lcal -lhcal -lfwutil -lixml -lnv -lrcl -lsal -lsecxal -lwlapi -lm -lcrypt -lcrypto -lcml_api -lethswctl -lcms_msg -lcms_util -lcms_boardctl -lxdslctl -latmctl -lwlctl
else
#export X_PRIVATE_LIBS = -lutility -lmd5 -lsex_crypt -lcal -lhcal -lfwutil -lixml -lnv -lrcl -lsal -lsecxal -lwlapi -lm -lcrypt -lcrypto -lcml_api -lethswctl -lcms_msg -lcms_util -lcms_boardctl -lwlctl
endif
endif
#export X_PUBLIC_LIBS = -lz -lslog -lsalx
#export X_PRIVATE_LIBS += -lpthread 

ifeq ($(CONFIG_SUPPORT_UBUS), 1)
#export X_PUBLIC_LIBS += -lubus -lblobmsg_json -ljson-c -lubox
endif

ifeq ($(CONFIG_SUPPORT_DMALLOC), 1)
#export X_PRIVATE_LIBS += -ldmalloc -l dmallocth
endif
export X_PRIVATE_UTIL_LIBS = -lutility -lpthread
export X_PRIVATE_CAL_LIBS = -lcml_api -lslog -lcal -lservicectrl -lbud $(X_PRIVATE_UTIL_LIBS)
ifeq ($(CONFIG_SUPPORT_UBUS), 1)
export X_PRIVATE_CAL_LIBS += -lubus -lblobmsg_json -ljson-c -lubox
endif
export X_PRIVATE_HCAL_LIBS = $(X_PRIVATE_CAL_LIBS) -lhcal -lmd5 -lhmac -lsex_crypt -lcrypto -lslog_e -lsalx -lnv -lledctrl -lcrypt 
export X_PRIVATE_SAL_LIBS = -lsal -lsalx -lnv $(X_PRIVATE_UTIL_LIBS)
ifeq ($(CONFIG_SUPPORT_FON), 1)
export X_PRIVATE_WLAN_LIBS += -lwlapi -lfonrpc
else
export X_PRIVATE_WLAN_LIBS = -lwlapi
endif
export X_PRIVATE_BOARD_LIBS = -lz -lfwutil -lboard
ifeq ($(CONFIG_BRCM_SUPPORT), 1)
ifeq ($(BRCM_416), 1)
export X_PRIVATE_BRCM_WLAN_LIBS = -lwlbcmshared -lwlbcmcrypto 
export X_PRIVATE_BRCM_UTIL_LIBS = -lcrypt -lcms_boardctl -lcms_util -lcms_msg -lbcm_flashutil -lbcm_crc
else
export X_PRIVATE_BRCM_WLAN_LIBS = -lwlbcmshared -lwlbcmcrypto -lgcc_s
export X_PRIVATE_BRCM_UTIL_LIBS = -lcrypt #-lbcm_flashutil -lbcm_crc
endif
X_PRIVATE_WLAN_LIBS += -lwlctl $(X_PRIVATE_BRCM_WLAN_LIBS) 
endif
ifeq ($(CONFIG_SUPPORT_5G_QD), 1)
X_PRIVATE_WLAN_LIBS += -lqcsapi_client 
endif
ifeq ($(CONFIG_SUPPORT_DSL), 1)
export X_PRIVATE_DLS_LIBS = -ldsl -lxdslctl -latmctl $(X_PRIVATE_BRCM_UTIL_LIBS)
endif
ifeq ($(CONFIG_SUPPORT_3G), 1)
export X_PRIVATE_UMTS_LIBS = -lumts_util
endif
ifeq ($(CONFIG_SUPPORT_FON), 1)
export X_PRIVATE_FON_LIBS = -lfonrpc
endif
ifeq ($(SC_USB_STORAGE), 1)
export X_PRIVATE_STORAGE_LIBS = -lusb_mnt
endif
export X_PRIVATE_SWITCH_LIBS = -lswitch
ifeq ($(CONFIG_BRCM_SUPPORT), 1)
X_PRIVATE_SWITCH_LIBS += -lethswctl $(X_PRIVATE_BRCM_UTIL_LIBS)
endif
export X_PRIVATE_HSAL_LIBS = $(X_PRIVATE_CAL_LIBS) $(X_PRIVATE_SAL_LIBS) $(X_PRIVATE_DLS_LIBS) $(X_PRIVATE_SWITCH_LIBS) -lnmap_api -lhsal -lbandwidth

ifeq ($(CONFIG_SUPPORT_UBUS), 1)
X_PRIVATE_SAL_LIBS += -ljson-c -lubox -lubus
X_PRIVATE_HSAL_LIBS += -ljson-c -lubox -lubus
endif
export X_SC_VOIP_LIBS = -lumts_util -lnmap  -lvoip -lslog -lledctrl -lcron -lrt -lutility -lpthread -ldns $(X_PRIVATE_SAL_LIBS) $(X_PRIVATE_HCAL_LIBS) -L$(PRIVATE_APPSPATH)/libs/libmd5 -L$(PRIVATE_APPSPATH)/libs/libcurl/curl-7.47.0/lib/.libs -L$(OPENSSL_DIR) $(X_PRIVATE_WLAN_LIBS) $(X_PRIVATE_HSAL_LIBS) -lm 
