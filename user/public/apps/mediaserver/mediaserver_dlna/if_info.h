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

#ifndef _IF_INFO_H_
#define _IF_INFO_H_

typedef struct if_info_s{
	char ifname[16];
	char ipaddr[16];
	char mac[18];
	char mask[16];
	char gateway[16];
	int  mtu;
	
}if_info_t;

/* Obtain a ip address string from ifname , maybe we can get it from nvram 
   without using open socket method 
 */
int getIFInfo(if_info_t *if_info);


#endif
