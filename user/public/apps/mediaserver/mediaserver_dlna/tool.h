/******************************************************************************
*  Copyright (c) 2002 Linux UPnP Internet MediaServer Device Project              *    
*  All rights reserved.                                                       *
*                                                                             *   
*  This file is part of The Linux UPnP Internet MediaServer Device (IGD).         *
*                                                                             *
*  The Linux UPnP IGD is free software; you can redistribute it and/or modify *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *    
*  The Linux UPnP IGD is distributed in the hope that it will be useful,      *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *   
*  You should have received a copy of the GNU General Public License          * 
*  along with Foobar; if not, write to the Free Software                      *
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  *
*                                                                             *  
*                                                                             *  
******************************************************************************/

#ifndef _TOOL_H_
#define _TOOL_H_


/* Obtain a ip address string from ifname , maybe we can get it from nvram 
   without using open socket method 
 */
int getIPAddress(char *ifname, char *pIPStr);

#if 0
/* Check the interface is up or down */
int getConnectionStatus(void);

int getUptime(void);

char * getDefaultConnectionService(void);
char * getConnectionType(void);
char * getPossibleConnectionTypes(void);

char * getWANAccessType(void);
int getLayer1MaxBitRate(void);
char * getPhysicalLinkStatus(void);
unsigned long  getTotalPacketsSent(void);
unsigned long  getTotalPacketsReceived(void);
unsigned long getTotalBytesSent(void);
unsigned long getTotalBytesReceived(void);
int    getNATEnabled(void);

/*
char * getWANAccessProvider(void);
char * getMaximumActiveConnections(void);
char * getNumberOfActiveConnections(void);
char * getActiveConnectionDeviceContainer(void);
char * getActiveConnectionServiceID(void);

int    getEnabledForInternet(void);
int    getDHCPServerConfigurable(void);
int    getDHCPRelay(void);
char * getSubnetMask(void);
char * getIPRouters(void);
char * getDNSServers(void);
char * getDomainName(void);
char * getMinAddress(void);
char * getMaxAddress(void);
char * getReservedAddress(void);
char * getHostName(void);
*/
#endif
int substr(char *infile, char *outfile, char *str_from, char *str_to);
#endif
