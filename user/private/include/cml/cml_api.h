#ifndef __CML_API_H_
#define __CML_API_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "cml_type.h"
#define CM_API_MAX_SOCKETBUF (32769)  //32K + 1 max string length reached 32K according to TR-098 Amendment 2.
#define CM_API_MAX_URI 256
#define CM_API_MAX_INTSTR 11 //4294967295
#ifndef MAX_VALUE_LEN 
#define MAX_VALUE_LEN 32769
#endif

/****************************************************************************
Function name : CM_GetValue
Description:
	Get the value of target URI node
Input :
	uri => location of target URI node
	value => a allocated character buffer used to get result
	length => the length of output buffer "value"
Example:
	
Return:
	Status code define in cml_type.h
*****************************************************************************/
CM_Ret CM_GetValue(IN char* uri, OUT char* value, IN int length);
CM_Ret CM_GetCAContentByInfo(char *rootUrl, char *contentUrl, char *nameUrl, char *info, char *content);
#ifdef CONFIG_SUPPORT_WIZARD
CM_Ret CM_GetUserSetGroup(IN char* uri, OUT char* value, IN int length);
#endif
/****************************************************************************
Function name : CM_GetValue_safe, CM_GetValue_safe_n_ext
Description:
	Based on CM_GetValue, it will return an allocated buffer directly(no need given buffer).
	Remember, you have to free the returned buffer by yourself
Input :
	uri => location of target URI node
Example:
	
Return:
	If the target URI is not exist, return an NULL pointer. Otherwise, return an allocated buffer of target URI.
*****************************************************************************/
char* CM_GetValue_safe(IN char* uri);
char* CM_GetValue_safe_n_ext(const char *format, ...);
int CM_GetValue_int_n_ext(const char *format, ...);

/****************************************************************************
Function name : CM_GetAttribute / CM_SetAttribute
Description:
	Get/Set attribute of target URI node
Input :
	uri => URI of target node
	ptype => Refer to CM_AttrType in cml_type.h.
	value => an integert contianer to get boolean value
Example:
	
Return:
	Status code define in cml_type.h
*****************************************************************************/
CM_Ret CM_GetAttribute(IN char* uri, int ptype, OUT int* value);
CM_Ret CM_SetAttribute(IN char* uri, IN int ptype, IN int value);

/****************************************************************************
Function name : CM_Commit
Description:
	Save configurations into mtd block 3.
	Currently, this function will also created an physical configuration file at /tmp/backup.xml
Input :
	None
Example:
	
Return:
	Status code define in cml_type.h
*****************************************************************************/
CM_Ret CM_Commit(void);
CM_Ret CM_CommitDPF(char* filename);
#ifdef CML_DEBUG
CM_Ret CM_UnCompressFile(char* filename);
#endif

/****************************************************************************
Function name : CM_AddInstance
Input : 
 	URI => the uri of instance you want to create without instance number. For exampe, if you wanna to look up "LANDevice", the uri is "InternetGatwayDevice.LANDevice."
	index => the instance number to be created
	For example, if you wanna to add "InternetGatewayDevice.LANDevice.1.Hosts.Host.3.".
	The parameters will be 
		URI = InternetGatewayDevice.LANDevice.1.Hosts.Host.
		index = 3
		
This function is based on TR069 specification. Applications can add an object node at an instansible node.

Definitions:

1. The first node is always there. 
If AP would like to remove the first node, this lib wouldn't acturally remove it. (Set disabled flag)
It will set NumberOfEntries to 0, remove the value of instance 1, and then, keep the structure

So, when we try to add instance and fiind the NumberOfEntries is 0, we should set up the first instance, and update NumberOfEntries to 1.

2. The all instances must be connected together.
The structure of linked list should be look like this 

"other data" -> "instance.1." -> "intstance.2." -> ..... -> "instance N" -> " other data"


3. The order of those instance may not be sequentail, but must be ascending sort. For example,

"instance.1." -> "intstance.2." -> "intstance.7." -> "intstance.8." -> "intstance.10." (OK)
"instance.1." -> "intstance.2." -> "intstance.8." -> "intstance.7." -> "intstance.10." (Wrong)

*****************************************************************************/
CM_Ret CM_AddInstance(IN char* uri, IN int index);
CM_Ret CM_AddVirtualInstance(IN char* uri, IN int index);
CM_Ret CM_AddInstance_ext(IN int data, const char *format, ...);

/****************************************************************************
Function name : CM_DelInstance
Description:
	Opposite to function  "CM_AddInstance", if will delete an instance node
Input :
	uri => location of target instance without instance number. For exampe, if you wanna to look up "LANDevice", the uri is "InternetGatwayDevice.LANDevice."
	index => target instance number you want to delete
	
Result in buffer:
	Intance list in ascending sort is formated by comma-seperated.
*****************************************************************************/
CM_Ret CM_DelInstance(IN char* uri, IN int index);
CM_Ret CM_DelAllInstance(IN char* uri);
CM_Ret CM_DelInstanceSort(IN char* uri, IN int index);
CM_Ret CM_DelNode(IN char* uri);
CM_Ret CM_DelVirtualInstance(IN char* uri, IN int index);
CM_Ret CM_DelInstance_ext(IN int data, const char *format, ...);

/****************************************************************************
Function name : CM_GetInstanceList
Description:
	Get a comma-seperated string of existing instances.
Input :
	uri => location of target instance without instance number. For exampe, if you wanna to look up "LANDevice", the uri is "InternetGatwayDevice.LANDevice."
	buffer => an string array to get data
	buffer_len => the length of buffer
	For example, if you wanna to the instance list of "InternetGatewayDevice.LANDevice.1.Hosts.Host.{i}.".
	The inputted uri should be "InternetGatewayDevice.LANDevice.1.Hosts.Host.".
	
Result in buffer:
	Intance list in ascending sort is formated by comma-seperated.
*****************************************************************************/
CM_Ret CM_GetInstanceList(IN char* uri, OUT char* buffer, IN int buffer_len);
char* CM_GetInstanceList_safe(IN char* uri);
char* CM_GetInstanceList_safe_n_ext(const char *format, ...);

/****************************************************************************
Function name : CM_MinInsInstance
Description:
	Get the smallest instance index which able to be added.
Input :
	uri => location of target instance without instance number. For exampe, if you wanna to look up "LANDevice", the uri is "InternetGatwayDevice.LANDevice."
	min_index => obtain the smallest index
	For example, if the number of existing instances are 1, 2, 4, and 5, the min_index will be 3.
	
Return:

*****************************************************************************/
CM_Ret CM_MinInsInstance(IN char* uri, OUT int* min_index);

/****************************************************************************
Function name : CM_InstanceReArrange_Internal
Description:
	Resort instance list
Input :
	uri => location of target instance without instance number. For exampe, if you wanna to look up "LANDevice", the uri is "InternetGatwayDevice.LANDevice."
	start_index => the first index number you want to re-sort from.
	len => the total number you want to re-sort. If this value is zero, this function will try to re-sort to the end of target instance list.
	For example, if the number of existing instances are 1, 2, 4, and 5. After running this function to re-sort all instances, the listof targe instances will be 1,2,3,4.
	
Return:

*****************************************************************************/
// internal use only, not open
//static CM_Ret CM_InstanceReArrange_Internal(IN char* uri, IN int start_index, IN int len);
CM_Ret CM_InstanceReArrange(IN char* uri);
CM_Ret CM_InstanceReArrange_ext(const char *format, ...);


/****************************************************************************
Function name : CM_InsertInstance
Description:
	Insert a new instance node 
Input :
	uri => The location you want to insert ( Without instance number )
	index => Instance number
Example:
	There are some instance at "AAA.BBB.CCC.{i}.", and the instance list is "1,2,3,4,5,"

	If we wanna to insert a new instance at index 3, please use the following command
		CM_InsertInstance("AAA.BBB.CCC.", 3);

	After the command finish, the instance list will be "1,2,3(inserted),4,5,6,"
	
Return:
	Status code defined in cml_type.h
*****************************************************************************/
CM_Ret CM_InsertInstance(IN char* uri, IN int index);
CM_Ret CM_InsertVirtualInstance(IN char* uri, IN int index);
CM_Ret CM_InsertInstance_ext(IN int index, const char *format, ...);

/****************************************************************************
Function name : CM_NodeMove
Description:
	Move whole sub-tree to another node.
	
	This is a special API for Sercomm design. 
	Sometime, we need to whole settings from EthA to EthB. So that we use this API to implement.
Input :
	new_parent_URI => The new parent you want to move to.
	src_node_URI => The node you want to move
Example:
	If we wanna to move SSID1 from group1 to group8, we need to change the location of node "WLANConfiguration.1.".
	The full URI is changed form
		InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.
	to
		InternetGatewayDevice.LANDevice.8.WLANConfiguration.1.
		
	So, what we need to do is use the API as following format
		CM_NodeMove("InternetGatewayDevice.LANDevice.8.", "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.");
Return:
	Status code defined in cml_type.h
*****************************************************************************/
CM_Ret CM_NodeMove(char* new_parent_URI, char* src_node_URI);

/****************************************************************************
Function name : CM_NodeMove
Description:
	Based on CM_NodeMove, this is another API implemented for short-name usage.
Input :
	new_parent_URI => The new parent you want to move to.
	src_node_sname => Based on this value, its parent will be moved to new position.
Example:
	Refer to CM_NodeMove
	
	If we wanna to move SSID1 from group1 to group8, we need to change the location of node "WLANConfiguration.1.".
	The full URI is changed form
		InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.
	to
		InternetGatewayDevice.LANDevice.8.WLANConfiguration.1.

	In this case, we can choose any children of "WLANConfiguration.1." as a base for node moving.
	So, what we need to do is use the API as following format
		CM_NodeMove("InternetGatewayDevice.LANDevice.8.", "WC1_SSID");
Return:
	Status code define in cml_type.h
*****************************************************************************/
CM_Ret CM_GetName(IN char* uri, OUT char* value, IN int length);

CM_Ret CM_SetValue(IN char* uri, OUT char* value);
CM_Ret CM_SetVirtualValue(IN char* uri, IN char* value);
CM_Ret CM_GetMethod(IN char* uri, IN int rpc, OUT char* value, IN int length);



CM_Ret CM_SetValue_safe_n_ext(IN char* data, const char *format, ...);
CM_Ret CM_SetValue_int_n_ext(IN UINT32 data, const char *format, ...);

CM_Ret CM_VirtualInstanceCheck(IN char* uri, OUT int* type);

CM_Ret CM_SetValue_n(IN char* format, IN int index, IN char* value);
CM_Ret CM_Reset2Default(void);
CM_Ret CM_SoftReset(void);
    

#ifdef MULTI_LEVEL_CONFIG    
CM_Ret CM_Register(char *group,int expireTime, int SAAlert);
CM_Ret CM_Unregister();
/****************************************************************************
Function name : CM_Register and CM_Unregister
Description:
    be used to implement Multi-level config
Input : group is [admin,user,support],expireTime is seconds,can not more than four-digit
        And SAAlert is for indicate whether all changes made by this app needs to alert to user.
*****************************************************************************/
CM_Ret CM_get_state(char* value);
CM_Ret CM_clear_state();
#endif
#ifdef SUPPORT_TR069_CWMP
/****************************************************************************
Function name : CM_TR069_GetNodeValue
Description:
	Requested by TR-069, you may this API to get the sub-tree below the target node.
Input :
	uri => The top node you want to read.
	xml_text => A charater pointer to get text data formated in XML
Example:

Return:
	Status code define in cml_type.h
*****************************************************************************/
CM_Ret CM_TR069_GetNodeValue(IN char* uri, OUT char** xml_text);
CM_Ret CM_TR069_GetNodeStructName(IN char* uri, IN int nextLevel, OUT char** xml_text, OUT int* list_number);
CM_Ret CM_TR069_GetNodeStructValue(IN char* uri, OUT char** xml_text, OUT int* list_number);
/****************************************************************************
Function name : CM_TR069_Notification_Registration
Description:
	For TR-069, regist a signal to cmld to get notification

	Based on TR-069 SPEC, this will make your process regist to attribute CM_AttrType_Notification_Passive and CM_AttrType_Notification_Active automatically.
Input :
	pid => Indicate the process waited for notification signal
	signum => siganl number defined in kernel
	filename => used to create a shared file between server and client
Example:

Return:
	Status code define in cml_type.h
*****************************************************************************/
CM_Ret CM_TR069_Notification_Registration(IN int signum, IN char* filename);
/****************************************************************************
Function name : CM_TR069_Notification_GetValue
Description:
	Note : you have to set CM_TR069_Notification_Registration before use this function

	To get the modified list in cmld.

	After reading the list, the file will be set empty.
Input :
	data => character buffer to recive file data
	filename => indicate a shared file between server and client ( must the same with the file for CM_TR069_Notification_Registration)
Example:
	The result in each line is a modifition log, and the format is [URI]/[Notify type]

	It looks like:
	InternetGatewayDevice.LANDevice.1.WLANConfiguration.1.SSID,Active\n
	InternetGatewayDevice.X_SC_DeviceFeature.X_SC_NumOfPort,Passive

Return:
	Status code define in cml_type.h
*****************************************************************************/
CM_Ret CM_TR069_Notification_GetValue(IN char* filename, OUT char** data);
CM_Ret CM_Restore(IN char* filename);


#endif

#define CM_TRUE(v) (v && ((*v == '1') || (*v == 'T') || (*v == 't')))
#define CM_FALSE(v) (v && ((*v == '0') || (*v == 'F') || (*v == 'f')))

#define for_each_instance(instance, p_token, out)                           \
	for(p_token=strtok_r(instance, ",", &out);p_token != NULL; p_token=strtok_r(NULL, ",", &out))


#ifdef __cplusplus
}
#endif

#endif

