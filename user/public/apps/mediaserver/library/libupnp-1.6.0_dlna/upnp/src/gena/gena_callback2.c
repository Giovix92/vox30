///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000-2003 Intel Corporation 
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

#include "config.h"
#if EXCLUDE_GENA == 0
#include "gena.h"
#include "upnpapi.h"
#include "gena_device.h"
#include "gena_ctrlpt.h"

#include "httpparser.h"
#include "httpreadwrite.h"
#include "statcodes.h"
#include "unixutil.h"

// For Windows Media Connect handler 
#define WMC_EVENTURL "/MediaReceiverRegistrar/Event"
#define WMC_CONTROLURL "/MediaReceiverRegistrar/Control"
#define WMC_WMP11 "UPnP/1.0; Windows" // WinMediaConnect flag for Vista
#define WMC_URL "mr_reg.xml"
#define WMC_SERVICETYPE "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1"
#define WMC_SERVICEID "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar"

/************************************************************************
* Function : error_respond									
*																	
* Parameters:														
*	IN SOCKINFO *info: Structure containing information about the socket
*	IN int error_code: error code that will be in the GENA response
*	IN http_message_t* hmsg: GENA request Packet 
*
* Description:														
*	This function send an error message to the control point in the case
*	incorrect GENA requests.
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
void
error_respond( IN SOCKINFO * info,
               IN int error_code,
               IN http_message_t * hmsg )
{
    int major,
      minor;

    // retrieve the minor and major version from the GENA request
    http_CalcResponseVersion( hmsg->major_version,
                              hmsg->minor_version, &major, &minor );

    http_SendStatusResponse( info, error_code, major, minor );
}

/************************************************************************
* Function : genaCallback									
*																	
* Parameters:														
*	IN http_parser_t *parser: represents the parse state of the request
*	IN http_message_t* request: HTTP message containing GENA request
*	INOUT SOCKINFO *info: Structure containing information about the socket
*
* Description:														
*	This is the callback function called by the miniserver to handle 
*	incoming GENA requests. 
*
* Returns: int
*	UPNP_E_SUCCESS if successful else appropriate error
***************************************************************************/
void
genaCallback( IN http_parser_t * parser,
              IN http_message_t * request,
              INOUT SOCKINFO * info )
{
    xboolean found_function = FALSE;
    xboolean is_wmc = FALSE;
    struct Handle_Info *handle_info;
    UpnpDevice_Handle device_handle;
    service_info *service;
    service_info *wmc;
    service_table *in;
    http_header_t *hdr = NULL;
    memptr user_agent;

#if 1
    // For Windows Media Connect for WMP11 on Vista. Added by Jick on 2007/12/03

    if( GetDeviceHandleInfo(&device_handle, &handle_info) != HND_DEVICE) {
		goto normal;
    }
    service = FindServiceEventURLPath( &handle_info->ServiceTable,
	    WMC_EVENTURL);

    hdr = httpmsg_find_hdr( request, HDR_USER_AGENT, &user_agent);
    if( (hdr != NULL)
	    && (strstr(user_agent.buf, WMC_WMP11) != NULL))
		is_wmc = TRUE;
    else
		is_wmc = FALSE;


    in = &(handle_info->ServiceTable);

    if ( is_wmc && service == NULL) {
		service_info *head = NULL;
		char wmc_url[255] = { 0 };
		char control_url[255] = { 0 };
		char event_url[255] = { 0 };
	
		sprintf(wmc_url, "http://%s:%d/%s",
			UpnpGetServerIpAddress(), UpnpGetServerPort(), WMC_URL);
		sprintf(control_url, "http://%s:%d%s",
			UpnpGetServerIpAddress(), UpnpGetServerPort(), WMC_CONTROLURL);
		sprintf(event_url, "http://%s:%d%s",
			UpnpGetServerIpAddress(), UpnpGetServerPort(), WMC_EVENTURL);
	
    HandleLock();
		head = ( service_info *)malloc(sizeof(service_info));
		wmc = head;
	
		if(!wmc) {
		    freeServiceList(head);
		    goto normal;
		}
	
		wmc->next = NULL;
		wmc->controlURL = strdup(control_url);
		wmc->eventURL = strdup(event_url);
		wmc->serviceType = strdup(WMC_SERVICETYPE);
		wmc->serviceId = strdup(WMC_SERVICEID);
		wmc->SCPDURL = strdup(wmc_url);
		wmc->UDN = strdup(in->endServiceList->UDN);;
		wmc->active = 1;
		wmc->subscriptionList = NULL;
		wmc->TotalSubscriptions = 0;
	
		in->endServiceList->next = wmc;
		in->endServiceList = wmc;
    HandleUnlock();
    } else if ( !is_wmc && service != NULL) {
		service_info *current_service = NULL;
		service_info *start_search = NULL;
		service_info *prev_service = NULL;
	
		current_service = in->serviceList;
		start_search = in->serviceList;
	
    HandleLock();
		while (current_service) {
		    if(strcmp(current_service->serviceId, WMC_SERVICEID) == 0) {
				if(prev_service) {
				    prev_service->next = current_service->next;
				} else {
				    in->serviceList = current_service->next;
				}
				if (current_service == in->endServiceList)
				    in->endServiceList = prev_service;
				break;
		    } else {
				prev_service = current_service;
				start_search = current_service->next;
				current_service = start_search;
		    }
		}
    HandleUnlock();
    }
    // End of WMC
#endif

normal:
    if( request->method == HTTPMETHOD_SUBSCRIBE ) {
#ifdef INCLUDE_DEVICE_APIS
        found_function = TRUE;
        if( httpmsg_find_hdr( request, HDR_NT, NULL ) == NULL ) {
            // renew subscription
            gena_process_subscription_renewal_request
            ( info, request );
	} else {
            // subscribe
            gena_process_subscription_request( info, request );
	}
        UpnpPrintf( UPNP_ALL, GENA, __FILE__, __LINE__,
            "got subscription request\n" );
    } else if( request->method == HTTPMETHOD_UNSUBSCRIBE ) {
        found_function = TRUE;
        // unsubscribe
        gena_process_unsubscribe_request( info, request );
#endif
    } else if( request->method == HTTPMETHOD_NOTIFY ) {
#ifdef INCLUDE_CLIENT_APIS
        found_function = TRUE;
        // notify
        gena_process_notification_event( info, request );
#endif
    }

    if( !found_function ) {
            // handle missing functions of device or ctrl pt
            error_respond( info, HTTP_NOT_IMPLEMENTED, request );
    }
}
#endif // EXCLUDE_GENA

