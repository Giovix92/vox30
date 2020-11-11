//
// Copyright (c) 2000 Intel Corporation 
// All rights reserved. 
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met: 
//
// * Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer. 
// * Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation 
// and/or other materials provided with the distribution. 
// * Neither name of Intel Corporation nor the names of its contributors 
// may be used to endorse or promote products derived from this software 
// without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////
//
// $Revision: 1.1.1.1 $
// $Date: 2009-05-05 05:06:56 $
//


#include "sample_util.h"
#include "mediaserver.h"
#include "upnp/upnptools.h"

#if 0
/********************************************************************************
 * SampleUtil_GetElementValue
 *
 * Description: 
 *       Given a DOM node such as <Channel>11</Channel>, this routine
 *       extracts the value (e.g., 11) from the node and returns it as 
 *       a string.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *
 ********************************************************************************/
char* SampleUtil_GetElementValue(IXML_Element * node)
{
  IXML_Node *child = ixmlNode_getFirstChild((IXML_Node *) node);
  char * temp =NULL;

 
  if ( (child!=0) && (ixmlNode_getNodeType(child)==eTEXT_NODE))
    {
      temp= strdup( ixmlNode_getNodeValue(child));
    }
      return temp;

}
#endif
/********************************************************************************
 * SampleUtil_GetServiceList
 *
 * Description: 
 *       Given a DOM node representing a UPnP Device Description Document,
 *       this routine parses the document and finds the i-th service list
 *       The service list is returned as a DOM node list.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the service list
 *   i    -- The index number of service to get
 *
 ********************************************************************************/
IXML_NodeList *SampleUtil_GetServiceList(IXML_Document *node, int i) 
{
    IXML_NodeList *ServiceList=NULL;
    IXML_NodeList *servlistnodelist=NULL;
    IXML_Node *servlistnode=NULL;
	
    servlistnodelist = ixmlDocument_getElementsByTagName(node, "serviceList");
    if(servlistnodelist && ixmlNodeList_length(servlistnodelist)) {
		/* we get the i-th service list */
		servlistnode = ixmlNodeList_item(servlistnodelist, i);
		/* create as list of DOM nodes */
		ServiceList = ixmlElement_getElementsByTagName((IXML_Element *)servlistnode, "service");
    }

    if (servlistnodelist) ixmlNodeList_free(servlistnodelist);

    return ServiceList;
}

/********************************************************************************
 * SampleUtil_GetDeviceList
 *
 * Description: 
 *       Given a DOM node representing a UPnP Device Description Document,
 *       this routine parses the document and finds the i-th service list
 *       The device list is returned as a DOM node list.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the device list
 *
 ********************************************************************************/
IXML_NodeList *SampleUtil_GetDeviceList(IXML_Document *node, int i) 
{
    IXML_NodeList *DeviceList=NULL;
    IXML_NodeList *devlistnodelist=NULL;
    IXML_Node *devlistnode=NULL;
	
    devlistnodelist = ixmlDocument_getElementsByTagName(node, "deviceList");
    if(devlistnodelist && ixmlNodeList_length(devlistnodelist)) {
	
	        
		/* we get the i-th device list */
		devlistnode = ixmlNodeList_item(devlistnodelist, i);
	
		/* create as list of DOM nodes */
		DeviceList = ixmlElement_getElementsByTagName((IXML_Element *)devlistnode, "device");
    }

    if (devlistnodelist) ixmlNodeList_free(devlistnodelist);

    return DeviceList;
}


/********************************************************************************
 * SampleUtil_GetDocumentItem
 *
 * Description: 
 *       Given a DOM node, this routine searches for the i-th element
 *       named by the input string item, and returns its value as a string.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *   item -- The item to search for
 *   i    -- The index of element to use
 *
 ********************************************************************************/

char* SampleUtil_GetDocumentItem(IXML_Document *node, char *item, int i) 
{
    IXML_NodeList *NodeList=NULL;
    IXML_Node *textNode=NULL;
    IXML_Node *tmpNode=NULL;
    char *ret=NULL;

    NodeList = ixmlDocument_getElementsByTagName(node, item);

    if (NodeList == NULL) {
		printf("Error finding %s in XML Node\n", item);
    } else {
		if ((tmpNode = ixmlNodeList_item(NodeList, i)) == NULL) {
		    //printf("Error finding %s value in XML Node\n", item);
		} else {
		    textNode = ixmlNode_getFirstChild(tmpNode);
		    
		    if (textNode==NULL) {   // for example <xxx />
		   // printf("find %s  textNode ==  NULL\n",item);
			ret=strdup("");
		    }
		    else {
		    	if (ixmlNode_getNodeValue(textNode)==NULL) {
				//printf("find %s getNodeValue return NULL \n",item);
				ret=strdup("");
			}
	   	    else
	   	    		ret = strdup(ixmlNode_getNodeValue(textNode));
		    }
		}
    }
    if (NodeList)
    	ixmlNodeList_free(NodeList);
    return ret;
}

/********************************************************************************
 * SampleUtil_GetFirstElementItem
 *
 * Description: 
 *       Given a DOM element, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char* SampleUtil_GetFirstElementItem(IXML_Element *node, char *item) 
{
    IXML_NodeList *NodeList=NULL;
    IXML_Node *textNode=NULL;
    IXML_Node *tmpNode=NULL;

    char *ret=NULL;
    int len;
	
    NodeList = ixmlElement_getElementsByTagName(node, item);
    if (NodeList == NULL) {
		//	printf("Error finding %s in XML Node\n", item);
    }
    else {
		if ((tmpNode = ixmlNodeList_item(NodeList, 0)) == NULL) {
			//	    printf("Error finding %s value in XML Node\n", item);
		}
		else {
		    textNode = ixmlNode_getFirstChild(tmpNode);
		    len = strlen(ixmlNode_getNodeValue(textNode));
	//	    if (err != NO_ERR) {
	//		printf("Error getting node value for %s in XML Node\n", item);
	//		if (NodeList) ixmlNodeList_free(NodeList);
	//		return ret;
	//	    }
		    ret = (char *) malloc(len+1);
		    if (!ret) {
	//			printf("Error allocating memory for %s in XML Node\n", item);
		    } else {
				strcpy(ret, ixmlNode_getNodeValue(textNode));
		    }
		}
    }
    if (NodeList) ixmlNodeList_free(NodeList);
    return ret;
}

#if 1
/********************************************************************************
 * SampleUtil_PrintEventType
 *
 * Description: 
 *       Prints a callback event type as a string.
 *
 * Parameters:
 *   S -- The callback event
 *
 ********************************************************************************/
void SampleUtil_PrintEventType(Upnp_EventType S)
{
    switch(S) {

    case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
	printf("UPNP_DISCOVERY_ADVERTISEMENT_ALIVE\n");
	break;
    case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
	printf("UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE\n");
	break;
    case UPNP_DISCOVERY_SEARCH_RESULT:
	printf("UPNP_DISCOVERY_SEARCH_RESULT\n");
	break;
    case UPNP_DISCOVERY_SEARCH_TIMEOUT:
	printf("UPNP_DISCOVERY_SEARCH_TIMEOUT\n");
	break;


	/* SOAP Stuff */
    case UPNP_CONTROL_ACTION_REQUEST:
	printf("UPNP_CONTROL_ACTION_REQUEST\n");
	break;
    case UPNP_CONTROL_ACTION_COMPLETE:
	printf("UPNP_CONTROL_ACTION_COMPLETE\n");
	break;
    case UPNP_CONTROL_GET_VAR_REQUEST:
	printf("UPNP_CONTROL_GET_VAR_REQUEST\n");
	break;
    case UPNP_CONTROL_GET_VAR_COMPLETE:
	printf("UPNP_CONTROL_GET_VAR_COMPLETE\n");
	break;

	/* GENA Stuff */
    case UPNP_EVENT_SUBSCRIPTION_REQUEST:
	printf("UPNP_EVENT_SUBSCRIPTION_REQUEST\n");
	break;
    case UPNP_EVENT_RECEIVED:
	printf("UPNP_EVENT_RECEIVED\n");
	break;
    case UPNP_EVENT_RENEWAL_COMPLETE:
	printf("UPNP_EVENT_RENEWAL_COMPLETE\n");
	break;
    case UPNP_EVENT_SUBSCRIBE_COMPLETE:
	printf("UPNP_EVENT_SUBSCRIBE_COMPLETE\n");
	break;
    case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
	printf("UPNP_EVENT_UNSUBSCRIBE_COMPLETE\n");
	break;

    case UPNP_EVENT_AUTORENEWAL_FAILED:
	printf("UPNP_EVENT_AUTORENEWAL_FAILED\n");
	break;
    case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
	printf("UPNP_EVENT_SUBSCRIPTION_EXPIRED\n");
	break;

    }
}
#endif
/********************************************************************************
 * SampleUtil_PrintEvent
 *
 * Description: 
 *       Prints callback event structure details.
 *
 * Parameters:
 *   EventType -- The type of callback event
 *   Event -- The callback event structure
 *
 ********************************************************************************/
int SampleUtil_PrintEvent(Upnp_EventType EventType, 
	       void *Event)
{
    printf("\n");
    SampleUtil_PrintEventType(EventType);
  
    switch ( EventType) {
      
	/* SSDP Stuff */
    case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
    case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
    case UPNP_DISCOVERY_SEARCH_RESULT:
#if 1    
    {
	struct Upnp_Discovery *d_event = (struct Upnp_Discovery * ) Event;
        
	printf("ErrCode     =  %d\n",d_event->ErrCode);
	printf("Expires     =  %d\n",d_event->Expires);
	printf("DeviceId    =  %s\n",d_event->DeviceId); 
	printf("DeviceType  =  %s\n",d_event->DeviceType);
	printf("ServiceType =  %s\n",d_event->ServiceType);
	printf("ServiceVer  =  %s\n",d_event->ServiceVer);
	printf("Location    =  %s\n",d_event->Location);
	printf("OS          =  %s\n",d_event->Os);
	printf("Ext         =  %s\n",d_event->Ext);
	
    }
#endif    
    break;
      
    case UPNP_DISCOVERY_SEARCH_TIMEOUT:
	// Nothing to print out here
	break;

    /* SOAP Stuff */
    case UPNP_CONTROL_ACTION_REQUEST:
    {
	//struct Upnp_Action_Request *a_event = (struct Upnp_Action_Request * ) Event;
	//char *xmlbuff=NULL;
        
#if 0
	printf("ErrCode     =  %d\n",a_event->ErrCode);
	printf("ErrStr      =  %s\n",a_event->ErrStr); 
	printf("ActionName  =  %s\n",a_event->ActionName); 
	printf("UDN         =  %s\n",a_event->DevUDN);
	printf("ServiceID   =  %s\n",a_event->ServiceID);
	if (a_event->ActionRequest) {
	    xmlbuff = UpnpNewPrintDocument(a_event->ActionRequest);
	    if (xmlbuff) printf("ActRequest  =  %s\n",xmlbuff);
	    if (xmlbuff) free(xmlbuff);
	    xmlbuff=NULL;
	} else {
	    printf("ActRequest  =  (null)\n");
	}
	if (a_event->ActionResult) {
	    xmlbuff = UpnpNewPrintDocument(a_event->ActionResult);
	    if (xmlbuff) printf("ActResult   =  %s\n",xmlbuff);
	    if (xmlbuff) free(xmlbuff);
	    xmlbuff=NULL;
	} else {
	    printf("ActResult   =  (null)\n");
	}
#endif
    }
    break;
      
    case UPNP_CONTROL_ACTION_COMPLETE:
    {
#if 0    
	struct Upnp_Action_Complete *a_event = (struct Upnp_Action_Complete * ) Event;
	char *xmlbuff=NULL;
        
	printf("ErrCode     =  %d\n",a_event->ErrCode);
	printf("CtrlUrl     =  %s\n",a_event->CtrlUrl);
	if (a_event->ActionRequest) {
	    xmlbuff = UpnpNewPrintDocument(a_event->ActionRequest);
	    if (xmlbuff) printf("ActRequest  =  %s\n",xmlbuff);
	    if (xmlbuff) free(xmlbuff);
	    xmlbuff=NULL;
	} else {
	    printf("ActRequest  =  (null)\n");
	}
	if (a_event->ActionResult) {
	    xmlbuff = UpnpNewPrintDocument(a_event->ActionResult);
	    if (xmlbuff) printf("ActResult   =  %s\n",xmlbuff);
	    if (xmlbuff) free(xmlbuff);
	    xmlbuff=NULL;
	} else {
	    printf("ActResult   =  (null)\n");
	}
#endif
    }
    break;
      
    case UPNP_CONTROL_GET_VAR_REQUEST:
    {
#if 1
	struct Upnp_State_Var_Request *sv_event = (struct Upnp_State_Var_Request * ) Event;
        
	printf("ErrCode     =  %d\n",sv_event->ErrCode);
	printf("ErrStr      =  %s\n",sv_event->ErrStr); 
	printf("UDN         =  %s\n",sv_event->DevUDN); 
	printf("ServiceID   =  %s\n",sv_event->ServiceID); 
	printf("StateVarName=  %s\n",sv_event->StateVarName); 
	printf("CurrentVal  =  %s\n",sv_event->CurrentVal);
#endif
    }
    break;
      
    case UPNP_CONTROL_GET_VAR_COMPLETE:
    {
#if 1    
	struct Upnp_State_Var_Complete *sv_event = (struct Upnp_State_Var_Complete * ) Event;
        
//	printf("ErrCode     =  %d\n",sv_event->ErrCode);
//	printf("CtrlUrl     =  %s\n",sv_event->CtrlUrl); 
	printf("StateVarName=  %s\n",sv_event->StateVarName); 
	printf("CurrentVal  =  %s\n",sv_event->CurrentVal);
#endif
    }
    break;
      
    /* GENA Stuff */
    case UPNP_EVENT_SUBSCRIPTION_REQUEST:
    {
#if 1
	struct Upnp_Subscription_Request *sr_event = (struct Upnp_Subscription_Request * ) Event;
        
	printf("ServiceID   =  %s\n",sr_event->ServiceId);
	printf("UDN         =  %s\n",sr_event->UDN); 
	printf("SID         =  %s\n",sr_event->Sid);
#endif
    }
    break;
      
    case UPNP_EVENT_RECEIVED:
    {
#if 0    
	struct Upnp_Event *e_event = (struct Upnp_Event * ) Event;
	char *xmlbuff=NULL;
        
	printf("SID         =  %s\n",e_event->Sid);
	printf("EventKey    =  %d\n",e_event->EventKey);
	xmlbuff = UpnpNewPrintDocument(e_event->ChangedVariables);
	printf("ChangedVars =  %s\n",xmlbuff);
	free(xmlbuff);
	xmlbuff=NULL;
#endif
    }
    break;

    case UPNP_EVENT_RENEWAL_COMPLETE:
    {
#if 1
	struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe * ) Event;
        
	printf("SID         =  %s\n",es_event->Sid);
	printf("ErrCode     =  %d\n",es_event->ErrCode);
	printf("TimeOut     =  %d\n",es_event->TimeOut);
#endif
    }
    break;

    case UPNP_EVENT_SUBSCRIBE_COMPLETE:
    case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
    {
#if 1
	struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe * ) Event;
        
	printf("SID         =  %s\n",es_event->Sid);
	printf("ErrCode     =  %d\n",es_event->ErrCode);
	printf("PublisherURL=  %s\n",es_event->PublisherUrl);
	printf("TimeOut     =  %d\n",es_event->TimeOut);
#endif
    }
    break;

    case UPNP_EVENT_AUTORENEWAL_FAILED:
    case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
    {
#if 1
	struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe * ) Event;

	printf("SID         =  %s\n",es_event->Sid);
	printf("ErrCode     =  %d\n",es_event->ErrCode);
	printf("PublisherURL=  %s\n",es_event->PublisherUrl);
	printf("TimeOut     =  %d\n",es_event->TimeOut);
#endif
    }
    break;



    }

    return(0);
}

/********************************************************************************
 * SampleUtil_FindAndParseService
 *
 * Description: 
 *       This routine finds the first occurance of a service in a DOM representation
 *       of a description document and parses it.  Note that this function currently
 *       assumes that the eventURL and controlURL values in the service definitions
 *       are full URLs.  Relative URLs are not handled here.
 *
 * Parameters:
 *   DescDoc -- The DOM description document
 *   location -- The location of the description document
 *   serviceSearchType -- The type of service to search for
 *   serviceId -- OUT -- The service ID
 *   eventURL -- OUT -- The event URL for the service
 *   controlURL -- OUT -- The control URL for the service
 *
 ********************************************************************************/
int SampleUtil_FindAndParseService (IXML_Document *DescDoc, char* location, char *serviceSearchType, char **serviceId, char **eventURL, char **controlURL) 
{
    int i, length, found=0;
    int index=3;
    int ret;
    char *serviceType=NULL;
    char *baseURL=NULL;
    char *base=NULL;
    char *relcontrolURL=NULL, *releventURL=NULL;
	
    IXML_NodeList *serviceList=NULL;
    IXML_Element *service=NULL;
//    baseURL = SampleUtil_GetDocumentItem(DescDoc, "URLBase", 0);
//    if (baseURL) 
//	base = baseURL;
 //   else
	base = location;

	for(index=0;index<DS_SERVICE_SERVCOUNT;index++) {
	    serviceList = SampleUtil_GetServiceList(DescDoc, index);
	    length = ixmlNodeList_length(serviceList);
	    for (i=0;i<length;i++) { 
			service = (IXML_Element *) ixmlNodeList_item(serviceList, i);
			serviceType = SampleUtil_GetFirstElementItem((IXML_Element *)service, "serviceType");
			if (strcmp(serviceType, serviceSearchType) == 0) {
		//	    printf("Found service: %s\n", serviceType);
			    *serviceId = SampleUtil_GetFirstElementItem(service, "serviceId");
			    relcontrolURL = SampleUtil_GetFirstElementItem(service, "controlURL");
			    releventURL = SampleUtil_GetFirstElementItem(service, "eventSubURL");
			    *controlURL = malloc(strlen(base) + strlen(relcontrolURL) + 2);
			    if (*controlURL) {
					ret = UpnpResolveURL(base, relcontrolURL, *controlURL);
			//		if (ret!=UPNP_E_SUCCESS)
			//		    printf("Error generating controlURL from %s + %s\n", base, relcontrolURL);
			    }
			    *eventURL = malloc(strlen(base) + strlen(releventURL) + 2);
			    if (*eventURL) {
					ret = UpnpResolveURL(base, releventURL, *eventURL);
			//		if (ret!=UPNP_E_SUCCESS)
			//		    printf("Error generating eventURL from %s + %s\n", base, releventURL);
			    }
			    if (relcontrolURL)
			    	free(relcontrolURL);
			    if (releventURL)
			    	free(releventURL);
			    relcontrolURL = releventURL = NULL;
			    found=1;
			    break;
			}
		
			if (serviceType) free(serviceType);
		 		serviceType=NULL;
	    }
	    if(found)
	        break;
	}

    if (serviceType)
    	free(serviceType);
    if (serviceList)
    	ixmlNodeList_free(serviceList);
    if (baseURL)
    	free(baseURL);
    return(found);
}
