#ifndef __CML_TYPE_H_
#define __CML_TYPE_H_

#define XML_REDUCED_MODE 1

/* Broadcom types. */
#ifndef UINT8
typedef unsigned char UINT8;
#endif
#ifndef UINT16
typedef unsigned short UINT16;
#endif
#ifndef UINT32
#ifndef UINT32_DEFINED
typedef unsigned long UINT32;
#define UINT32_DEFINED
#endif
#endif

#ifndef BOOL
#define BOOL int;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/*
#define CM_DEBUG
#ifdef CM_DEBUG
#define print_console_cmld(fmt, args...) fprintf(stderr,"\nAt File %s, Function %s, Line %d\n", __FILE__, __FUNCTION__, __LINE__);fprintf(stderr,fmt, ##args)
#else
#define print_console_cmld(fmt, args...)
#endif
*/

#define CM_IPADDR_MAX_LEN 	15
#define CM_NETMASK_MAX_LEN 	CM_IPADDR_MAX_LEN
#define CM_MACADDR_MAX_LEN	17

#define CM_CHECK_NULLPTR(con) if((con) == NULL) {return CM_Ret_NullPtr;}
#define CM_CHECK_FREEPTR(con) if((con) != NULL) {CM_UTL_mfree((void*)con);con=NULL;}
#define CM_CHECK_OK(ret) if((ret) != CM_Ret_OK) {return ret;}

#define INSTANCE_NUMBER_OF_ENTRIES  "NumberOfEntries"

#define SUPPORT_TR069_CWMP

typedef enum
{
	CM_EventType_ReBoot=0,
	CM_EventType_NotificationChange,
	CM_EventType_Notification1,
	CM_EventType_Notification2,
}CM_EventType;
#define CM_GetEventValue(event)  (1 << (event))

// max number of this entry is 32
typedef enum
{
	CM_AttrType_Writable,
	CM_AttrType_Disabled,
	//CM_AttrType_Readable,
	CM_AttrType_Removable,
	
	//works on XML_REDUCED_MODE, if anyone set, this prameter/node will save to flash
	
    CM_AttrType_UserSet,
	CM_AttrType_UserSetAuto,
#ifdef MULTI_LEVEL_CONFIG    
	CM_AttrType_AdminSetAuto,
	CM_AttrType_SupSetAuto,
#endif	
	//CM_AttrType_Instansible,
	// TR-069 Start //////////////////
	CM_AttrType_Notification_Off,
	CM_AttrType_Notification_Passive,
	CM_AttrType_Notification_Active,
	CM_AttrType_NOT_ActiveNotification,
	CM_AttrType_AccessList,
	// TR-069 End ///////////////////
#ifdef DEVICE_MODEL_SUPPORT
    CM_AttrType_DeviceModel,
#endif
	CM_AttrType_Enumeration,
	CM_AttrType_Encryption,
	CM_AttrType_Password,
}CM_AttrType;
#define CM_GetAttrValue(attr) 1<<attr

// refer to TR098
typedef enum
{
	CM_ParaType_Start,
	// insert your new type in this section //
	CM_ParaType_Object,
	CM_ParaType_String,
	CM_ParaType_Int,
	CM_ParaType_UnsignedInt,
	CM_ParaType_Boolean,
	CM_ParaType_DateTime,
	CM_ParaType_Base64,
	CM_ParaType_Enumerate,
	CM_ParaType_IpAddr,
	CM_ParaType_NetMask,
	CM_ParaType_MacAddr,
	CM_ParaType_Any,
	///////////////////////////////////
	CM_ParaType_End,
}CM_ParaType;


typedef enum
{
	CM_Ret_FAIL=-100,
	CM_Ret_No=CM_Ret_FAIL,
	CM_Ret_OK=0,
	CM_Ret_Yes=CM_Ret_OK,
	CM_Ret_SameValueSet,
	CM_Ret_NotFound,
	CM_Ret_NullPtr,
	CM_Ret_AllocFail,
	CM_Ret_Exist,
	CM_Ret_BufferTooSmall,
	CM_Ret_InvalidOperation,
	/* For local socket start*/
	CM_Ret_Socket_Finish, // for socket now
	CM_Ret_Socket_Fail,
	CM_Ret_Socket_Closed,
	CM_Ret_Socket_CreateFail,
	CM_Ret_Socket_Timeout,
	/* For local socket end */
//#ifdef PARAM_SYNTAX_SUPPORT    
    CM_Ret_CwmpError_9000 = 9000, /* Method not supported */
    CM_Ret_CwmpError_9001 = 9001, /* Request denied */
    CM_Ret_CwmpError_9002 = 9002, /* Internal error */
    CM_Ret_CwmpError_9003 = 9003, /* Invalid arguments */
    CM_Ret_CwmpError_9004 = 9004, /* Resources exceeded */
    CM_Ret_CwmpError_9005 = 9005, /* Invalid parameter name */
    CM_Ret_CwmpError_9006 = 9006, /* Invalid parameter type */
    CM_Ret_CwmpError_9007 = 9007, /* Invalid parameter value */
    CM_Ret_CwmpError_9008 = 9008, /* Attempt to set a non-writable parameter */
    CM_Ret_CwmpError_9009 = 9009, /* Notification request rejected */
    CM_Ret_CwmpError_9010 = 9010, /* Download failure */
    CM_Ret_CwmpError_9011 = 9011, /* Upload failure */
    CM_Ret_CwmpError_9012 = 9012, /* File transfer server authentication failure */
    CM_Ret_CwmpError_9013 = 9013, /* Unsupported protocol for file transfer */
    CM_Ret_CwmpError_9014 = 9014, /* MaxEnvelopes exceeded */
//#endif    
}CM_Ret;


typedef struct
{
	char* name;
	char* value; // always save value as string, applications should convert this by themself
	char* factory_value; // save factory value as string
	UINT8 ptype; // CM_ParaType : String, int, unsigned int, ........etc.
	UINT8 event; // CM_EventType
    UINT16 attribute; // CM_AttrType : There is only 'W' in TR098
	//char* version;
	int min;
	int max;
	char* enumeration; //for special string type
}CM_TR98;

typedef struct CM_NodeObj_t
{
	char* name;
	char* sname;
	UINT16 attribute;
#ifdef MULTI_LEVEL_CONFIG    
	char *userSetGroup;
#endif
    //char* version;
	struct CM_NodeObj_t* parent;
	struct CM_NodeObj_t* NextSibling;
	struct CM_NodeObj_t* child;
	struct CM_ParaObj_t* ParaList;
}CM_NodeObj_t;

typedef CM_Ret (*pfn_platform)(IN char *uri, OUT char* buffer, IN int buf_len);

typedef void (*pfn_recall)(void);

typedef struct CM_PlatformFn_t {
	const char *name;
	pfn_platform pfn;
	int  reduce;
} CM_PlatformFn_T;

typedef struct CM_ParaObj_t
{
	CM_TR98 parameter;
	char* sname;
#ifdef MULTI_LEVEL_CONFIG    
	char *userSetGroup;
#endif
    CM_PlatformFn_T* function;
    CM_PlatformFn_T* setfunction;
	struct CM_NodeObj_t* parent;
	struct CM_ParaObj_t* NextSibling;
}CM_ParaObj_t;

typedef struct CM_FuncRecall_T{
	pfn_recall pfn;
	int interval; //seconds
	int left_sec; //seconds
}CM_FuncRecall_T;

void scan_recall_func_table(void);

#include <stdarg.h>

#endif
