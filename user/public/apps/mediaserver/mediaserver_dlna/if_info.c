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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* strcmp and friends */
#include <ctype.h>		/* isdigit and friends */
#include <stddef.h>		/* offsetof */
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_ether.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "if_info.h"


int _get_sockfd()
{
	int sockfd = -1;
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("user: socket creating failed");
		return (-1);
	}
	return sockfd;
}



/*TODO: maybe we can get ip address directly from nvram */
int getIFInfo(if_info_t *if_info)
{
	unsigned char *pt;
	struct ifreq ifr;
	struct sockaddr_in *saddr;
	int fd;
	int ret=0;
	if ((fd=_get_sockfd())>=0)
	{
		strcpy(ifr.ifr_name, if_info->ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		/* get ip address */
		if (ioctl(fd, SIOCGIFADDR, &ifr)==0){
			saddr = (struct sockaddr_in *)&ifr.ifr_addr;
			strcpy(if_info->ipaddr,(char *)inet_ntoa(saddr->sin_addr));
			/* for hide on demand ip */
			if(strcmp(if_info->ipaddr,"10.64.64.64")==0)
				ret=-2;			
		}else
			ret=-1;
		/* get mac address */
		if (ioctl(fd, SIOCGIFHWADDR, &ifr)==0){
			pt=ifr.ifr_hwaddr.sa_data;
			sprintf(if_info->mac,"%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx"
					,*pt,*(pt+1),*(pt+2),*(pt+3),*(pt+4),*(pt+5));
			if_info->mac[17]='\0';
		}else 
			ret=-1;
		/* get netmask */
		if (ioctl(fd,SIOCGIFNETMASK , &ifr)==0){
			saddr = (struct sockaddr_in *)&ifr.ifr_addr;
			strcpy(if_info->mask,(char *)inet_ntoa(saddr->sin_addr));
		}else
			ret=-1;
		
		/* get mtu */
		if (ioctl(fd,SIOCGIFMTU, &ifr)==0){
			if_info->mtu=ifr.ifr_mtu;	
		}else
			ret=-1;	
		close(fd);
		return ret;

	}
	return -1;
}



