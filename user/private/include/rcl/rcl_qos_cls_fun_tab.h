/**
 * @file   
 * @author 
 * @date   2010-08-30
 * @brief  
 *
 * Copyright - 2009 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */
#ifndef _QOS_CLS_FUN_TAB_H__
#define _QOS_CLS_FUN_TAB_H__

#include "rcl_qos_cls_func.h"

#define CLSENABLE         "ClsEn"      // ClassificationEnable
#define CLSORDER          "ClsOd"      // ClassificationOrder
#define CLSSTYPE          "ClsType"     // ClassificationSources
#define CLSSAPP           "ClsApp"     // ClassificationSources
#define CLSSRC            "ClsSrc"     // ClassificationSources
#define CLSDESTIF         "ClsDestIf"    // ClassificationDestInterface
#define DSTIP             "DIP"        // DestIP
#define DSTIPMASK         "DMsk"       // DestMask
#define DSTIPMODE         "DIPEx"      // DestIPExclude
#define SRCIP             "SIP"        // SourceIP
#define SRCIPMASK         "SMsk"       // SourceMask
#define SRCIPMODE         "SIPEx"      // SourceIPExclude
#define PROTOCOL          "Proto"      // Protocol
#define PROTOMODE         "ProtoEx"    // ProtocolExclude
#define DPORTSTART        "DPort"      // DestPort
#define DPORTEND          "DportEnd"   // DestPortRangeMax
#define DPORTMODE         "DportEx"    // DestPortExclude
#define SPORTSTART        "SPort"      // SourcePort
#define SPORTEND          "SPortEnd"   // SourcePortRangeMax
#define SPORTMODE         "SPortEx"    // SourcePortExclude
#define TCPACKCTRL        "TCPACK"     // TCPACK
#define TCPACKCTRLMODE    "TCPACKEx"   // TCPACKExclude
#define DSCPVALUE         "DSCPCk"     // DSCPCheck
#define DSCPVALUEEND      "DSCPCkEnd"     // DSCPCheckRangeMax
#define DSCPMODE          "DSCPEx"     // DSCPExclude
#define DSCPREMARKTO      "DSCPMrk"    // DSCPMark, for remark
#define ETHERPRIO        "8021p"      // EthernetPriorityCheck
#define ETHERPRIOMODE         "8021pEx"    // EthernetPriorityExclude
#define ETHERPRIOREMARKTO     "8021pMrk"   // EthernetPriorityMark
#define QUEUEINDEX        "ClsQIdx"    // ClassQueue, queue index
#define SVDCLSIDMODE      "SVdClsIDEx" // SourceVendorClassIDExclude
#define SVDCLSID          "SVdClsID"   // SourceVendorClassID
#define DVDCLSIDMODE      "DVdClsIDEx" // DestVendorClassIDExclude
#define DVDCLSID          "DVdClsID"   // DestVendorClassID
#define SCLIENTID         "SClientID"  // SourceClientID
#define SUCLSID           "SUClsID"    // SourceUserClassID
#define VLANID            "VID"        // VLANID
#define IPLENGTHMIN       "IPLenMin"   // IPLengthMin
#define IPLENGTHMAX       "IPLenMax"   // IPLengthMax
#define TCPSYNCTRL        "TCPSYN"     // TCPSYN
#define TCPSYNCTRLMODE    "TCPSYNEx"   // TCPSYNExclude
#define TCPFINCTRL        "TCPFIN"     // TCPFIN
#define TCPFINCTRLMODE    "TCPFINEx"   // TCPFINExclude
#define TOSVALUE          "ToS"        // TosCheck
#define IPPRECEDENCEVALUE "IPPre"      // IPPrecedence
#define SRCMAC            "SMac"       // SourceMAC
#define SRCMACMASK        "SMacMask"   // SourceMACMask
#define SRCMACEXCLUDE     "SMacExclude"// SourceMACExclude
#define DESTMAC           "DMac"       // DestMAC
#define DESTMACMASK       "DMacMask"   // DestMACMask
#define DESTMACEXCLUDE    "DMacExclude"// DestMACExclude
#define ETHTYPE           "EType"      // EthernetType
#define CLSOWNER          "CLSOWNER"
#define CLSAPP            "CLSAPP"   //CLASSAPP
#define APPKEY            "APPKEY"   //APPKEY <-> app_id
#define APPENABLE         "APPENABLE" //APP ENABLE
#define APPURN            "APPURN"    //APP URN[256]
#define APPID           "APPID" //APP ID
#define APPINDEX           "APPINDEX" //APP PHY INDEX
#define APPETHMARK           "APPETHMARK" 
#define APPDSCPMARK           "APPDSCPMARK" 
#define APPQUEUE           "APPQUEUE" 
#define FLOWKEY           "FLOWKEY"   //FLOW TYPE
#define FLOWENABLE         "FLOWENABLE" //FLOW ENABLE
#define FLOWTYPE           "FLOWTYPE"   //FLOW TYPE
#define FLOWNUM           "FLOWNUM" 
#ifdef CONFIG_SUPPORT_QOS_DHCP_OPTION121
#define DESTOPTIONID       "DOptionID"   // DestOptionID
#endif


// for hidden policy
#define ALG       "ALG"   // ALG
#define INTERFACE "Iface" // Interface
// #define QUEUEINDEX "ClsQIdx" // already defined in normal classify fun

//---------------------------------------------------------------
// purpose: for dump data
// input: p - policy to get
// output: buf - buffer for the return data
//         *buf1 - if the return data > 128 bytes then GetFun
//                 will allocate a buffer by itself and put the
//                 buffer address in *buf1
// return: 0 - success
// Note: 1. Right now only select tag can use buf1
//       2. If GetFun return data in *buf1 then caller must free
//          the memory after using the data
//       3. If the error message is large then 128 bytes, you can
//          use gMsg instead of set msg and err_sec
//---------------------------------------------------------------
typedef int (*GetFun)(char *buf,qos_cls_policy_entry *p);

//-----------------------------------------------------------------
// input: value - value to save
//        p     - policy to set
//-----------------------------------------------------------------
typedef int (*SetFun)(char *value,qos_cls_policy_entry *p);
typedef char *(*CMGetFun)(int index);

enum
{
    RULE_PROCESS_NONE = 0,
    RULE_PROCESS_EBIPTABLE = 1,
    RULE_PROCESS_INGRESSMODULE = 1<<1,
    RULE_PROCESS_ANYONESUPPORT = 1<<2,
};

typedef struct qosfun_tab{
    char *key;
    int whoProcess;
    SetFun setfun;
    GetFun getfun;
    CMGetFun cm_get_fun;
}qosfun_tab;


extern qosfun_tab qos_fun_tab[]; 

typedef int (*Get)(char *buf,app_policy *p);

typedef int (*Set)(char *value,app_policy *p);
//typedef char *(*CMGetFun)(int index);

typedef struct qosapp_tab{
    char *key;
    int whoProcess;
    Set setfun;
    Get getfun;
    CMGetFun cm_get_fun;
}qosapp_tab;


extern qosapp_tab qos_app_tab[]; 


#endif
