#ifndef __CML_CORE_H_
#define __CML_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "cml_type.h"
#include <sys/types.h>
#include <signal.h>
#include "cml_api.h"
#include "common/list.h"    

#define CM_RPC_GROUPLEN 15
#define CM_RPC_INTSIZE 8
#define CM_RPC_MAXCONN 16
#define CM_RPC_MAXBUFFER CM_API_MAX_SOCKETBUF
#define CM_MAX_URI_BUFFER 1024
#define CM_RPC_SOCKETNAME "/tmp/Sercomm_cm_socket"
#define CM_RPC_DUMPNAME "/tmp/Sercomm_cm_dump"

#define CM_RPC_ENDSTR "Sercomm_RPC_End"
/* pid-group queue struct*/
#ifdef MULTI_LEVEL_CONFIG
struct pid_groupList{
    int pid;
    char group[CM_RPC_GROUPLEN];
    time_t expiry_time;
    int SAAlert;
    struct list_head list;
};
#endif
/*
#define CM_RPC_CMD_SetParameterValues "SetParameterValues"
#define CM_RPC_CMD_GetParameterValues "GetParameterValues"
#define CM_RPC_CMD_GetParameterNames "GetParameterNames"
#define CM_RPC_CMD_SetParameterAttributes "SetParameterAttributes"
#define CM_RPC_CMD_GetParameterAttributes "GetParameterAttributes"
#define CM_RPC_CMD_AddObject "AddObject"
#define CM_RPC_CMD_DeleteObject "DeleteObject"
#define CM_RPC_CMD_Reboot "Reboot"
#define CM_RPC_CMD_Download "Download"
#define CM_RPC_CMD_Event "Event"
#define CM_RPC_CMD_Quit "Quit"
*/
enum
{
	CM_RPC_CMD_Invalid,
	CM_RPC_CMD_SetParameterValues,
	CM_RPC_CMD_SetParameterAttributes,
	CM_RPC_CMD_GetParameterValues,
	CM_RPC_CMD_GetFactoryParameterValues,
	CM_RPC_CMD_SetFactoryParameterValues,
	CM_RPC_CMD_GetParameterNames,
	CM_RPC_CMD_GetParameterAttributes,
	CM_RPC_CMD_GetParameterDataType,
	CM_RPC_CMD_GetParameterMaxLen,
	CM_RPC_CMD_GetParameterMinLen,
	CM_RPC_CMD_GetParameterURI,
	CM_RPC_CMD_GetParameterEnumeration,
#ifdef CONFIG_SUPPORT_WIZARD
	CM_RPC_CMD_GetParameterUserSetGroup,
#endif
	CM_RPC_CMD_AddInstance,
	CM_RPC_CMD_AddVirtualInstance,
	CM_RPC_CMD_DelInstance,
    CM_RPC_CMD_DelInstanceSort,
	CM_RPC_CMD_DelNode,
	CM_RPC_CMD_InsertInstance,
	CM_RPC_CMD_InsertVirtualInstance,
	CM_RPC_CMD_VirtualInstanceCheck,
	CM_RPC_CMD_GetInstanceList,
	CM_RPC_CMD_ReArrange,
	CM_RPC_CMD_Commit,
//	CM_RPC_CMD_CommitDPF,
	CM_RPC_CMD_Reset2Default,
	CM_RPC_CMD_SoftReset,
	CM_RPC_CMD_PartialReset2Default,
    CM_RPC_CMD_GetParameterValues_x,
	CM_RPC_CMD_GetParameterValues_secure,
	CM_RPC_CMD_NodeMove,
	CM_RPC_CMD_SNameDump,
	CM_RPC_CMD_SetParameterValues_ShortName,
	CM_RPC_CMD_SetParameterAttributes_ShortName,
	CM_RPC_CMD_GetParameterValues_ShortName,
	CM_RPC_CMD_GetParameterNames_ShortName,
	CM_RPC_CMD_GetParameterAttributes_ShortName,
	CM_RPC_CMD_GetParameterDataType_ShortName,
	CM_RPC_CMD_SaveCfg,
	CM_RPC_CMD_Restore,
#ifdef MULTI_LEVEL_CONFIG
    CM_RPC_CMD_Register,
    CM_RPC_CMD_Unregister,
    CM_RPC_CMD_GetState,
    CM_RPC_CMD_ClearState,
#endif
#ifdef SUPPORT_TR069_CWMP
	CM_RPC_CMD_TR069_GetNodeXML,
	CM_RPC_CMD_TR069_GetParameterXML,
	CM_RPC_CMD_TR069_PID_Registration,
    CM_RPC_CMD_TR069_GetNodeName,
    CM_RPC_CMD_TR069_GetParameterName,
    CM_RPC_CMD_TR069_GetNodeValue,
    CM_RPC_CMD_TR069_GetParameterValue,
#endif
#ifdef CML_DEBUG
	CM_RPC_CMD_UnCompressFile,
#endif
	CM_RPC_CMD_End, // used to define the number of function pointer only now
	CM_RPC_CMD_AddObject,
	CM_RPC_CMD_DeleteObject,
	CM_RPC_CMD_Reboot,
	CM_RPC_CMD_Download,
	CM_RPC_CMD_Event,
	CM_RPC_CMD_Quit,
}CM_RPC_CMD;

//CM_Ret CM_RPC_RPOCESS_SetParameterValues(int client_fd, char* buffer);

//int (*p[4]) (int x, int y);


#ifdef CONFIG_SUPPORT_UBUS
CM_Ret CM_SendChangeEvent(IN char* uri, IN char* value);
#endif


int CM_RPC_SocketRead(int socket, char* buffer);
void CM_RPC_SocketWrite (int socket_fd, const char* text);
CM_Ret CM_RPC_ServerStart(void);
CM_Ret CM_RPC_RPOCESS_GetParameter(int client_fd, char* buffer, CM_ParaObj_t** obj);
CM_Ret CM_RPC_RPOCESS_GetObjNode(int client_fd, char* buffer, CM_NodeObj_t** obj);
CM_Ret CM_RPC_TR069_Notification_Check(CM_ParaObj_t* para_node, char* buffer, int buffer_len, int pid);
#ifdef SUPPORT_TR069_CWMP
typedef struct TR069_Callback_t
{
	pid_t pid;
	int signum;
	char* filename;
}TR069_Callback_t;
#endif

#ifdef __cplusplus
}
#endif

#endif

