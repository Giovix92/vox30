/***********************************************************************
*
* discovery.c
*
* Perform PPPoE discovery
*
* Copyright (C) 1999 by Roaring Penguin Software Inc.
*
***********************************************************************/

static char const RCSID[] =
"$Id: discovery.c,v 1.6 2008/06/15 04:35:50 paulus Exp $";

#define _GNU_SOURCE 1
#include "pppoe.h"
#include "pppd/pppd.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef USE_LINUX_PACKET
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#include <signal.h>
#ifdef __SC_BUILD__
#include <syslog.h>
#include "log/slog.h"
#include <sal/sal_wan.h>
#include "utility.h"

static int ac_name_flag=0;
#endif
//

/**********************************************************************
*%FUNCTION: parseForHostUniq
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data.
* extra -- user-supplied pointer.  This is assumed to be a pointer to int.
*%RETURNS:
* Nothing
*%DESCRIPTION:
* If a HostUnique tag is found which matches our PID, sets *extra to 1.
***********************************************************************/
static void
parseForHostUniq(UINT16_t type, UINT16_t len, unsigned char *data,
		 void *extra)
{
    int *val = (int *) extra;
#ifdef __SC_BUILD__
    int hu_len;
    if(host_uniq && strlen(host_uniq))
    {
        hu_len = strlen(host_uniq);
        if (type == TAG_HOST_UNIQ && len == hu_len)
        {
            if(strncmp(host_uniq, data, len) == 0)
                *val = 1;
        }
    } else {
#endif
    if (type == TAG_HOST_UNIQ && len == sizeof(pid_t)) {
	pid_t tmp;
	memcpy(&tmp, data, len);
	if (tmp == getpid()) {
	    *val = 1;
	}
    }
#ifdef __SC_BUILD__
    }
#endif
}

/**********************************************************************
*%FUNCTION: packetIsForMe
*%ARGUMENTS:
* conn -- PPPoE connection info
* packet -- a received PPPoE packet
*%RETURNS:
* 1 if packet is for this PPPoE daemon; 0 otherwise.
*%DESCRIPTION:
* If we are using the Host-Unique tag, verifies that packet contains
* our unique identifier.
***********************************************************************/
static int
packetIsForMe(PPPoEConnection *conn, PPPoEPacket *packet)
{
    int forMe = 0;

    if (memcmp(packet->ethHdr.h_dest, conn->myEth, ETH_ALEN)) return 0;
    /* If we're not using the Host-Unique tag, then accept the packet */
    if (!conn->useHostUniq) return 1;

    parsePacket(packet, parseForHostUniq, &forMe);
    return forMe;
}

/**********************************************************************
*%FUNCTION: parsePADOTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data.  Should point to a PacketCriteria structure
*          which gets filled in according to selected AC name and service
*          name.
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks interesting tags out of a PADO packet
***********************************************************************/
static void
parsePADOTags(UINT16_t type, UINT16_t len, unsigned char *data,
	      void *extra)
{
    struct PacketCriteria *pc = (struct PacketCriteria *) extra;
    PPPoEConnection *conn = pc->conn;
    int i;
#ifdef __SC_BUILD__
    char *tag_p=NULL;    
#endif
    switch(type) {
    case TAG_AC_NAME:
	pc->seenACName = 1;
#ifdef __SC_BUILD__
	if(len>0)
	{
    	tag_p=malloc(len+1);
    	if(tag_p)
    	{
    	    ac_name_flag=1;
    	    memset(tag_p, 0, len+1);
            strncpy(tag_p, data, len);
	        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADO has been received from WAN %s, BRAS name is %s.\n", pppd_get_wan_connection_name(wanid), tag_p);
            free(tag_p);
        }
    }    
#endif
	if (conn->printACNames) {
	    info("Access-Concentrator: %.*s", (int) len, data);
	}
#ifdef __SC_BUILD__
	int acNamePWildFlag = 0;
    	int acNameNWildFlag = 0; 	
	unsigned char nameBuf[512] = {'\0'};	
	if(conn->acName)
	{
		unsigned char *p = conn->acName;
	        unsigned char *pBuf = nameBuf;
		if(*p == '!')
		{
			acNameNWildFlag = 1;
		}
		while(*p != '\0')
		{
		    if(*p == '*')
		    {
			acNamePWildFlag = 1;
		    }
		    if((*p != '!') && (*p != '*'))
		    {		
		    	*pBuf = *p;
			pBuf++;
		    }		
		    p++; 			
		}	
	}
#endif
	if (conn->acName && len == strlen(conn->acName) &&
	    !strncmp((char *) data, conn->acName, len)) {
	    pc->acNameOK = 1;
	}
#ifdef __SC_BUILD__
	if(conn->acName)
	{
		if(acNameNWildFlag == 0 && acNamePWildFlag == 1)
		{
			if(NULL != strstr((char *) data, nameBuf))
			    pc->acNameOK = 1;		
		}
		else if(acNameNWildFlag == 1)
		{
			if(acNamePWildFlag == 1)
			{
				if(NULL == strstr((char *) data, nameBuf))
				    pc->acNameOK = 1;	
			}
			else
			{
				if(strncmp((char *) data, nameBuf, len - 1))
				    pc->acNameOK = 1;	
			}	
		}
	}
#endif
	break;
    case TAG_SERVICE_NAME:
	pc->seenServiceName = 1;
	if (conn->serviceName && len == strlen(conn->serviceName) &&
	    !strncmp((char *) data, conn->serviceName, len)) {
	    pc->serviceNameOK = 1;
	}
	break;
    case TAG_AC_COOKIE:
	conn->cookie.type = htons(type);
	conn->cookie.length = htons(len);
	memcpy(conn->cookie.payload, data, len);
	break;
    case TAG_RELAY_SESSION_ID:
	conn->relayId.type = htons(type);
	conn->relayId.length = htons(len);
	memcpy(conn->relayId.payload, data, len);
	break;
    case TAG_SERVICE_NAME_ERROR:
#ifdef __SC_BUILD__
        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADO: Service Name Error: %.*s from WAN %s\n", (int)len, data, pppd_get_wan_connection_name(wanid));
#endif
	error("PADO: Service-Name-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_AC_SYSTEM_ERROR:
#ifdef __SC_BUILD__
        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADO: System Error: %.*s from WAN %s\n", (int)len, data, pppd_get_wan_connection_name(wanid));
#endif
	error("PADO: System-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_GENERIC_ERROR:
#ifdef __SC_BUILD__
        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADO: Generic Error: %.*s from WAN %s\n", (int)len, data, pppd_get_wan_connection_name(wanid));
#endif
	error("PADO: Generic-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    }
}

/**********************************************************************
*%FUNCTION: parsePADSTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data (pointer to PPPoEConnection structure)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks interesting tags out of a PADS packet
***********************************************************************/
static void
parsePADSTags(UINT16_t type, UINT16_t len, unsigned char *data,
	      void *extra)
{
    PPPoEConnection *conn = (PPPoEConnection *) extra;
    switch(type) {
    case TAG_SERVICE_NAME:
	dbglog("PADS: Service-Name: '%.*s'", (int) len, data);
	break;
    case TAG_SERVICE_NAME_ERROR:
#ifdef __SC_BUILD__
        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADS: Service Name Error: %.*s from WAN %s\n", (int)len, data, pppd_get_wan_connection_name(wanid));
#endif
	error("PADS: Service-Name-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_AC_SYSTEM_ERROR:
#ifdef __SC_BUILD__
        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADS: Service Error: %.*s from WAN %s\n", (int)len, data, pppd_get_wan_connection_name(wanid));
#endif

	error("PADS: System-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_GENERIC_ERROR:
#ifdef __SC_BUILD__
        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADS: Generic Error: %.*s from WAN %s\n", (int)len, data, pppd_get_wan_connection_name(wanid));
#endif

	error("PADS: Generic-Error: %.*s", (int) len, data);
	conn->error = 1;
	break;
    case TAG_RELAY_SESSION_ID:
	conn->relayId.type = htons(type);
	conn->relayId.length = htons(len);
	memcpy(conn->relayId.payload, data, len);
	break;
    }
}

#if (defined(__SC_BUILD__) && defined(CONFIG_PPP_SUPPORT_PADM_PADN))
/**********************************************************************
*%FUNCTION: parsePADMTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data (pointer to PPPoEConnection structure)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks interesting tags out of a PADS packet
***********************************************************************/
static void
parsePADMTags(UINT16_t type, UINT16_t len, unsigned char *data,
	      void *extra)
{
    PPPoEConnection *conn = (PPPoEConnection *) extra;
    switch(type) {
    case TAG_HURL:
	conn->hurl.type = type;
	conn->hurl.length = len;
	memcpy(conn->hurl.payload, data, len);
	break;
    case TAG_MOTM:
	conn->motm.type = type;
	conn->motm.length = len;
	memcpy(conn->motm.payload, data, len);
	break;
    }
}
/**********************************************************************
*%FUNCTION: parsePADNTags
*%ARGUMENTS:
* type -- tag type
* len -- tag length
* data -- tag data
* extra -- extra user data (pointer to PPPoEConnection structure)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Picks interesting tags out of a PADS packet
***********************************************************************/
static void
parsePADNTags(UINT16_t type, UINT16_t len, unsigned char *data,
	      void *extra)
{
    char ip[16] = {0}, mask[16] = {0}, gw[16] = {0};
    struct in_addr s;
    PPPoEConnection *conn = (PPPoEConnection *) extra;
    switch(type) {
        case TAG_IP_ROUTE_ADD:
            if (len < 12)
                break;
            conn->ipRouteAdd.type = type;
            s.s_addr = htonl((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]); 
            snprintf(ip, sizeof(ip), "%s", inet_ntoa(s));
            s.s_addr = htonl((data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7]); 
            snprintf(mask, sizeof(mask), "%s", inet_ntoa(s));
            s.s_addr = htonl((data[8] << 24) | (data[9] << 16) | (data[10] << 8) | data[11]); 
            snprintf(gw, sizeof(gw), "%s", inet_ntoa(s));
            conn->ipRouteAdd.length += snprintf((char *)(&conn->ipRouteAdd.payload[conn->ipRouteAdd.length]), sizeof(conn->ipRouteAdd.payload)-conn->ipRouteAdd.length, "%s,%s,%s;", ip, mask, gw);
	        break;
    }
}
#endif
/***********************************************************************
*%FUNCTION: sendPADI
*%ARGUMENTS:
* conn -- PPPoEConnection structure
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADI packet
***********************************************************************/
static void
sendPADI(PPPoEConnection *conn)
{
    PPPoEPacket packet;
    unsigned char *cursor = packet.payload;
    PPPoETag *svc = (PPPoETag *) (&packet.payload);
    UINT16_t namelen = 0;
    UINT16_t plen;
    int omit_service_name = 0;

    if (conn->serviceName) {
	namelen = (UINT16_t) strlen(conn->serviceName);
	if (!strcmp(conn->serviceName, "NO-SERVICE-NAME-NON-RFC-COMPLIANT")) {
	    omit_service_name = 1;
	}
    }

    /* Set destination to Ethernet broadcast address */
    memset(packet.ethHdr.h_dest, 0xFF, ETH_ALEN);
    memcpy(packet.ethHdr.h_source, conn->myEth, ETH_ALEN);

    packet.ethHdr.h_proto = htons(Eth_PPPOE_Discovery);
    packet.vertype = PPPOE_VER_TYPE(1, 1);
    packet.code = CODE_PADI;
    packet.session = 0;

    if (!omit_service_name) {
	plen = TAG_HDR_SIZE + namelen;
	CHECK_ROOM(cursor, packet.payload, plen);

	svc->type = TAG_SERVICE_NAME;
	svc->length = htons(namelen);

	if (conn->serviceName) {
	    memcpy(svc->payload, conn->serviceName, strlen(conn->serviceName));
	}
	cursor += namelen + TAG_HDR_SIZE;
    } else {
	plen = 0;
    }

    /* If we're using Host-Uniq, copy it over */
    if (conn->useHostUniq) {
	PPPoETag hostUniq;
#ifdef __SC_BUILD__
    if(host_uniq && strlen(host_uniq))
    {
        int hu_len = strlen(host_uniq);
	    hostUniq.type = htons(TAG_HOST_UNIQ);
	    hostUniq.length = htons(hu_len);
	    memcpy(hostUniq.payload, host_uniq, hu_len);
	    CHECK_ROOM(cursor, packet.payload, hu_len + TAG_HDR_SIZE);
	    memcpy(cursor, &hostUniq, hu_len + TAG_HDR_SIZE);
	    cursor += hu_len + TAG_HDR_SIZE;
	    plen += hu_len + TAG_HDR_SIZE;
    } else {
#endif    
	pid_t pid = getpid();
	hostUniq.type = htons(TAG_HOST_UNIQ);
	hostUniq.length = htons(sizeof(pid));
	memcpy(hostUniq.payload, &pid, sizeof(pid));
	CHECK_ROOM(cursor, packet.payload, sizeof(pid) + TAG_HDR_SIZE);
	memcpy(cursor, &hostUniq, sizeof(pid) + TAG_HDR_SIZE);
	cursor += sizeof(pid) + TAG_HDR_SIZE;
	plen += sizeof(pid) + TAG_HDR_SIZE;
#ifdef __SC_BUILD__
    }
#endif    
    }

    packet.length = htons(plen);

    sendPacket(conn, conn->discoverySocket, &packet, (int) (plen + HDR_SIZE));
#ifdef __SC_BUILD__
    closelog();
    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADI has been sent from WAN %s\n", pppd_get_wan_connection_name(wanid));
#endif
}

/**********************************************************************
*%FUNCTION: waitForPADO
*%ARGUMENTS:
* conn -- PPPoEConnection structure
* timeout -- how long to wait (in seconds)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Waits for a PADO packet and copies useful information
***********************************************************************/
void
waitForPADO(PPPoEConnection *conn, int timeout)
{
    fd_set readable;
    int r;
    struct timeval tv;
    PPPoEPacket packet;
    int len;

    struct PacketCriteria pc;
    pc.conn          = conn;
    pc.acNameOK      = (conn->acName)      ? 0 : 1;
    pc.serviceNameOK = (conn->serviceName) ? 0 : 1;
    pc.seenACName    = 0;
    pc.seenServiceName = 0;
    conn->error = 0;
#ifdef __SC_BUILD__
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
#endif
    do {
	if (BPF_BUFFER_IS_EMPTY) {
#ifndef __SC_BUILD__
	    tv.tv_sec = timeout;
	    tv.tv_usec = 0;
#endif
	    FD_ZERO(&readable);
	    FD_SET(conn->discoverySocket, &readable);

	    while(1) {
		r = select(conn->discoverySocket+1, &readable, NULL, NULL, &tv);
		if (r >= 0 || errno != EINTR) break;
	    }
	    if (r < 0) {
		error("select (waitForPADO): %m");
		return;
	    }
	    if (r == 0) return;        /* Timed out */
	}

	/* Get the packet */
	receivePacket(conn->discoverySocket, &packet, &len);

	/* Check length */
	if (ntohs(packet.length) + HDR_SIZE > len) {
	    error("Bogus PPPoE length field (%u)",
		   (unsigned int) ntohs(packet.length));
	    continue;
	}

#ifdef USE_BPF
	/* If it's not a Discovery packet, loop again */
	if (etherType(&packet) != Eth_PPPOE_Discovery) continue;
#endif

	/* If it's not for us, loop again */
	if (!packetIsForMe(conn, &packet)) continue;

	if (packet.code == CODE_PADO) {
	    if (NOT_UNICAST(packet.ethHdr.h_source)) {
		error("Ignoring PADO packet from non-unicast MAC address");
		continue;
	    }
	    if (conn->req_peer
		&& memcmp(packet.ethHdr.h_source, conn->req_peer_mac, ETH_ALEN) != 0) {
		warn("Ignoring PADO packet from wrong MAC address");
		continue;
	    }
	    if (parsePacket(&packet, parsePADOTags, &pc) < 0)
		return;
#ifdef __SC_BUILD__
	    if(ac_name_flag==0)
               log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADO has been received from WAN %s, BRAS name is null\n", pppd_get_wan_connection_name(wanid));
#endif
	    if (conn->error)
		return;
	    if (!pc.seenACName) {
		error("Ignoring PADO packet with no AC-Name tag");
		continue;
	    }
	    if (!pc.seenServiceName) {
		error("Ignoring PADO packet with no Service-Name tag");
		continue;
	    }
	    conn->numPADOs++;
	    if (pc.acNameOK && pc.serviceNameOK) {
#ifdef __SC_BUILD__
		static unsigned char buf[64];
		int len = sizeof(buf);
#endif
		memcpy(conn->peerEth, packet.ethHdr.h_source, ETH_ALEN);
#ifdef __SC_BUILD__
		if(0 == getTagValueFromPacket(&packet, TAG_SERVICE_NAME, buf, &len))
		{
			/*Martin Huang:Trick service name*/
			if(NULL == conn->serviceName)
				conn->serviceName = buf;			
		}
        sal_wan_set_con_alive_t(wanid, "1");
#endif
		conn->discoveryState = STATE_RECEIVED_PADO;
		break;
	    }
	}
    } while (conn->discoveryState != STATE_RECEIVED_PADO);
}

/***********************************************************************
*%FUNCTION: sendPADR
*%ARGUMENTS:
* conn -- PPPoE connection structur
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Sends a PADR packet
***********************************************************************/
static void
sendPADR(PPPoEConnection *conn)
{
    PPPoEPacket packet;
    PPPoETag *svc = (PPPoETag *) packet.payload;
    unsigned char *cursor = packet.payload;

    UINT16_t namelen = 0;
    UINT16_t plen;

    if (conn->serviceName) {
	namelen = (UINT16_t) strlen(conn->serviceName);
    }
    plen = TAG_HDR_SIZE + namelen;
    CHECK_ROOM(cursor, packet.payload, plen);

    memcpy(packet.ethHdr.h_dest, conn->peerEth, ETH_ALEN);
    memcpy(packet.ethHdr.h_source, conn->myEth, ETH_ALEN);

    packet.ethHdr.h_proto = htons(Eth_PPPOE_Discovery);
    packet.vertype = PPPOE_VER_TYPE(1, 1);
    packet.code = CODE_PADR;
    packet.session = 0;

    svc->type = TAG_SERVICE_NAME;
    svc->length = htons(namelen);
    if (conn->serviceName) {
	memcpy(svc->payload, conn->serviceName, namelen);
    }
    cursor += namelen + TAG_HDR_SIZE;

    /* If we're using Host-Uniq, copy it over */
    if (conn->useHostUniq) {
	PPPoETag hostUniq;
#ifdef __SC_BUILD__
    if(host_uniq && strlen(host_uniq))
    {
        int hu_len = strlen(host_uniq);
	    hostUniq.type = htons(TAG_HOST_UNIQ);
	    hostUniq.length = htons(hu_len);
	    memcpy(hostUniq.payload, host_uniq, hu_len);
	    CHECK_ROOM(cursor, packet.payload, hu_len + TAG_HDR_SIZE);
	    memcpy(cursor, &hostUniq, hu_len + TAG_HDR_SIZE);
	    cursor += hu_len + TAG_HDR_SIZE;
	    plen += hu_len + TAG_HDR_SIZE;
    } else {
#endif    
	pid_t pid = getpid();
	hostUniq.type = htons(TAG_HOST_UNIQ);
	hostUniq.length = htons(sizeof(pid));
	memcpy(hostUniq.payload, &pid, sizeof(pid));
	CHECK_ROOM(cursor, packet.payload, sizeof(pid)+TAG_HDR_SIZE);
	memcpy(cursor, &hostUniq, sizeof(pid) + TAG_HDR_SIZE);
	cursor += sizeof(pid) + TAG_HDR_SIZE;
	plen += sizeof(pid) + TAG_HDR_SIZE;
#ifdef __SC_BUILD__
    }
#endif    
    }

    /* Copy cookie and relay-ID if needed */
    if (conn->cookie.type) {
	CHECK_ROOM(cursor, packet.payload,
		   ntohs(conn->cookie.length) + TAG_HDR_SIZE);
	memcpy(cursor, &conn->cookie, ntohs(conn->cookie.length) + TAG_HDR_SIZE);
	cursor += ntohs(conn->cookie.length) + TAG_HDR_SIZE;
	plen += ntohs(conn->cookie.length) + TAG_HDR_SIZE;
    }

    if (conn->relayId.type) {
	CHECK_ROOM(cursor, packet.payload,
		   ntohs(conn->relayId.length) + TAG_HDR_SIZE);
	memcpy(cursor, &conn->relayId, ntohs(conn->relayId.length) + TAG_HDR_SIZE);
	cursor += ntohs(conn->relayId.length) + TAG_HDR_SIZE;
	plen += ntohs(conn->relayId.length) + TAG_HDR_SIZE;
    }

    packet.length = htons(plen);
    sendPacket(conn, conn->discoverySocket, &packet, (int) (plen + HDR_SIZE));

#ifdef __SC_BUILD__
    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADR has been sent from WAN %s\n", pppd_get_wan_connection_name(wanid));
#endif
}
#ifdef __SC_BUILD__
/**********************************************************************
*%FUNCTION: waitForPADT
*%ARGUMENTS:
* conn -- PPPoE connection info
* timeout -- how long to wait (in seconds)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Waits for a PADT packet and copies useful information
***********************************************************************/
void
waitForPADT(PPPoEConnection *conn, int timeout)
{
    fd_set readable;
    int r;
    struct timeval tv;
    PPPoEPacket packet;
    int len;

    conn->error = 0;
    do {
	if (BPF_BUFFER_IS_EMPTY) {
	    tv.tv_sec = timeout;
	    tv.tv_usec = 0;

	    FD_ZERO(&readable);
	    FD_SET(conn->discoverySocket, &readable);

	    while(1) {
		r = select(conn->discoverySocket+1, &readable, NULL, NULL, &tv);
		if (r >= 0 || errno != EINTR) break;
	    }
	    if (r < 0) {
		error("select (waitForPADS): %m");
		return;
	    }
	    if (r == 0) return;
	}

	/* Get the packet */
	receivePacket(conn->discoverySocket, &packet, &len);

	/* Check length */
	if (ntohs(packet.length) + HDR_SIZE > len) {
	    error("Bogus PPPoE length field (%u)",
		   (unsigned int) ntohs(packet.length));
	    continue;
	}

#ifdef USE_BPF
	/* If it's not a Discovery packet, loop again */
	if (etherType(&packet) != Eth_PPPOE_Discovery) continue;
#endif

	/* If it's not from the AC, it's not for me */
	if (memcmp(packet.ethHdr.h_source, conn->peerEth, ETH_ALEN)) continue;

	/* If it's not for us, loop again */
	if (!packetIsForMe(conn, &packet)) continue;

	/* Is it PADS?  */
	if (packet.code == CODE_PADT) {
	  if(packet.session == conn->session)
		return;
	    if (conn->error)
		return;
	    conn->discoveryState = STATE_TERMINATED;
            log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADT has been received from WAN %s\n", pppd_get_wan_connection_name(wanid));
	    break;
	}
    } while (conn->discoveryState != STATE_TERMINATED);
}
#endif

/**********************************************************************
*%FUNCTION: waitForPADS
*%ARGUMENTS:
* conn -- PPPoE connection info
* timeout -- how long to wait (in seconds)
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Waits for a PADS packet and copies useful information
***********************************************************************/
static void
waitForPADS(PPPoEConnection *conn, int timeout)
{
    fd_set readable;
    int r;
    struct timeval tv;
    PPPoEPacket packet;
    int len;

    conn->error = 0;
#ifdef __SC_BUILD__
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
#endif
    do {
	if (BPF_BUFFER_IS_EMPTY) {
#ifndef __SC_BUILD__
	    tv.tv_sec = timeout;
	    tv.tv_usec = 0;
#endif
	    FD_ZERO(&readable);
	    FD_SET(conn->discoverySocket, &readable);

	    while(1) {
		r = select(conn->discoverySocket+1, &readable, NULL, NULL, &tv);
		if (r >= 0 || errno != EINTR) break;
	    }
	    if (r < 0) {
		error("select (waitForPADS): %m");
		return;
	    }
	    if (r == 0) return;
	}

	/* Get the packet */
	receivePacket(conn->discoverySocket, &packet, &len);

	/* Check length */
	if (ntohs(packet.length) + HDR_SIZE > len) {
	    error("Bogus PPPoE length field (%u)",
		   (unsigned int) ntohs(packet.length));
	    continue;
	}

#ifdef USE_BPF
	/* If it's not a Discovery packet, loop again */
	if (etherType(&packet) != Eth_PPPOE_Discovery) continue;
#endif

	/* If it's not from the AC, it's not for me */
	
	if (memcmp(packet.ethHdr.h_source, conn->peerEth, ETH_ALEN)) 
#ifdef __SC_BUILD__
	{
	    if (packetIsForMe(conn, &packet) && packet.code == CODE_PADO) {
			/*Martin Huang:Send PADT for non-first PADO*/
			sendPADTFromClientForMultiBras(conn, packet.ethHdr.h_source);
		}
	//	
            continue;
	}
#else
		continue;
#endif
	/* If it's not for us, loop again */
	if (!packetIsForMe(conn, &packet)) continue;

	/* Is it PADS?  */
	if (packet.code == CODE_PADS) {
	    /* Parse for goodies */
	    if (parsePacket(&packet, parsePADSTags, conn) < 0)
		return;
	    if (conn->error)
		return;
	    conn->discoveryState = STATE_SESSION;
#ifdef __SC_BUILD__
            log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADS has been received from WAN %s\n", pppd_get_wan_connection_name(wanid));
#endif
	    break;
	}
    } while (conn->discoveryState != STATE_SESSION);

    /* Don't bother with ntohs; we'll just end up converting it back... */
    conn->session = packet.session;

    info("PPP session is %d", (int) ntohs(conn->session));

    /* RFC 2516 says session id MUST NOT be zero or 0xFFFF */
    if (ntohs(conn->session) == 0 || ntohs(conn->session) == 0xFFFF) {
	error("Access concentrator used a session value of %x -- the AC is violating RFC 2516", (unsigned int) ntohs(conn->session));
    }
}
#if (defined(__SC_BUILD__) && defined(CONFIG_PPP_SUPPORT_PADM_PADN))
/**********************************************************************
*%FUNCTION: waitForPADMandPADN
*%ARGUMENTS:
* conn -- PPPoE connection info
* timeout -- how long to wait (in seconds)
*%RETURNS:
* Nothing
l%DESCRIPTION:
* Waits for PADM and PADN packets and copies useful information
***********************************************************************/
void
waitForPADMandPADN(PPPoEConnection *conn, int timeout)
{
    fd_set readable;
    int r;
    struct timeval tv;
    PPPoEPacket packet;
    int len;
    int rece_times = 3;
    WAN_PPP_TAG_t tags = {0};
    char hurl_val[1024] = {0};
    char ntp_val[5][64] = {0};
    char provcode_val[64] = {0};
    char *p = NULL;
    char *p1 = NULL;
    int i;
    if(!conn)
        return;
	if(conn->discoveryState != STATE_SESSION)
        return;
    if(conn->discoverySocket  <= 0 )
        return;
    conn->error = 0;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
	if (BPF_BUFFER_IS_EMPTY) {
	    tv.tv_sec = timeout; //if timeout==0, non-block
	    tv.tv_usec = 0;
	    FD_ZERO(&readable);
	    FD_SET(conn->discoverySocket, &readable);

        do
        {
		    r = select(conn->discoverySocket+1, &readable, NULL, NULL, &tv);
		    if (r >= 0 || errno != EINTR)
                break;
        }while(timeout != 0);
        if (r <= 0)
            return;
	} else {
        return;
    }
    
	/* Get the packet */
	receivePacket(conn->discoverySocket, &packet, &len);
    
    /* Check length */
	if (ntohs(packet.length) + HDR_SIZE > len) {
	    error("Bogus PPPoE length field (%u)",
		   (unsigned int) ntohs(packet.length));
	    return;
	}

	if (packet.code == CODE_PADM) {
	    /* Parse for goodies */
	    if (parsePacket(&packet, parsePADMTags, conn) < 0)
		    return;
	    if (conn->error)
		    return;
        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADM has been received from WAN %s\n", pppd_get_wan_connection_name(wanid));
        if(conn->hurl.length)
        {
            if(conn->hurl.length > sizeof(hurl_val))
            {
                return;
            }
            strncpy(hurl_val, conn->hurl.payload, conn->hurl.length);
            tags.hurl = hurl_val;
        }
        if(conn->motm.length)
        {
            p = strstr(conn->motm.payload, "ntp");
            while(p)
            {
                p1 = strchr(p, ',');
                if(p1)
                    len = p1-p;
                else
                    len = strlen(p);
                if(*(p+3) == '=')
                {
                    strncpy(ntp_val[0], (p+4), (len-4));
                    tags.ntp[0] = ntp_val[0];
                }
                else
                {
                    i = *(p+3) - '1';
                    if(i < 0 && i > 4)
                        return;
                    strncpy(ntp_val[i], (p+5), (len-5));
                    tags.ntp[i] = ntp_val[i];
                }
                if (p1)
                    p = strstr(p1, "ntp");
                else 
                    break;
            }
            p = strstr(conn->motm.payload, "provcode");
            if(p)
            {
                p1 = strchr(p, ',');
                if(p1)
                    len = p1-p-9;
                else
                    len = strlen(p) - 9;
                strncpy(provcode_val, (p+9), len);
                tags.provcode = provcode_val;
            }
        }
        sal_wan_save_ppp_tags(wanid, &tags);
	} else if (packet.code == CODE_PADN){
	    /* Parse for goodies */
        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "PPPOE PADN has been received from WAN %s\n", pppd_get_wan_connection_name(wanid));
        conn->ipRouteAdd.length = 0;
        if (parsePacket(&packet, parsePADNTags, conn) < 0)
            return;
        if (conn->ipRouteAdd.length > 0)
        {
            sal_wan_set_con_ppp_ip_route_add_t(wanid, conn->ipRouteAdd.payload);
        }
    }
}
#endif
/**********************************************************************
*%FUNCTION: discover0y
*%ARGUMENTS:
* conn -- PPPoE connection info structure
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Performs the PPPoE discovery phase
***********************************************************************/
#ifdef __SC_BUILD__
static int first_send_padi = 1;
#define MAX_BACKOFF_INTERVAL 128
#endif
void
discovery(PPPoEConnection *conn)
{
#ifdef __SC_BUILD__
#else
    int padiAttempts = 0;
#endif
    int padrAttempts = 0;
#ifdef __SC_BUILD__
    int timeout = 0;
#if 0
    if(first_send_padi)
    {
        timeout = conn->discoveryTimeout;
        first_send_padi = 0;
    }
    else
    {
         timeout =  MAX_BACKOFF_INTERVAL;
    }
#endif
#else
    int timeout = conn->discoveryTimeout;
#endif
    conn->discoverySocket =
	openInterface(conn->ifName, Eth_PPPOE_Discovery, conn->myEth);

    do {
#ifdef __SC_BUILD__
#else
	padiAttempts++;
	if (padiAttempts > MAX_PADI_ATTEMPTS) {
#ifdef __SC_BUILD__
        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Timeout waiting for PADO packets for WAN %s\n", pppd_get_wan_connection_name(wanid));
#else
	    warn("Timeout waiting for PADO packets");
#endif
	    close(conn->discoverySocket);
	    conn->discoverySocket = -1;
	    return;
	}
#endif
	sendPADI(conn);
	conn->discoveryState = STATE_SENT_PADI;
#ifdef __SC_BUILD__
	ac_name_flag=0;
#endif
#ifdef __SC_BUILD__
    if (0 < timeout && timeout <= 20)
        timeout = rand()%90+40;
    else
        timeout = rand()%10+10;
#if 0
    if(padiAttempts == MAX_PADI_ATTEMPTS)
    {
        timeout = MAX_BACKOFF_INTERVAL - holdoff;
    }
#endif
#endif
	waitForPADO(conn, timeout);

#ifdef __SC_BUILD__
#if 0
	timeout *= 2;
    if(timeout > MAX_BACKOFF_INTERVAL)
        timeout = MAX_BACKOFF_INTERVAL;
#endif
#endif
    } while (conn->discoveryState == STATE_SENT_PADI);

    timeout = conn->discoveryTimeout;
#ifdef __SC_BUILD__
    first_send_padi = 1;
#endif
    do {
	padrAttempts++;
#ifdef __SC_BUILD__
	if (padrAttempts > MAX_PADR_ATTEMPTS) {
#else
	if (padrAttempts > MAX_PADI_ATTEMPTS) {
#endif
	    warn("Timeout waiting for PADS packets");
	    close(conn->discoverySocket);
	    conn->discoverySocket = -1;
	    return;
	}
	sendPADR(conn);
	conn->discoveryState = STATE_SENT_PADR;
	waitForPADS(conn, timeout);

#ifdef __SC_BUILD__
#if 0
	if(padrAttempts == MAX_PADR_ATTEMPTS)
	{
		/*Martin Huang:Give PPPoe server the last chance:maybe we could be lucky*/
		sendPADR(conn);
		conn->discoveryState = STATE_SENT_PADR;
		waitForPADS(conn, conn->discoveryTimeout);
	}
#endif
	timeout += conn->discoveryTimeout;
#else
	timeout *= 2;
#endif

    } while (conn->discoveryState == STATE_SENT_PADR);

    /* We're done. */
    conn->discoveryState = STATE_SESSION;
    return;
}
