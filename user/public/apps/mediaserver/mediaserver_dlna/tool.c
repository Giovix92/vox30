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
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
/* #include <linux/in.h> */
/* #include <linux/mii.h> */
#include <linux/sockios.h>
#include "tool.h"
#include "upnpd.h"

//static int get_ethernet_link_status(char *phy_id);
#define SIOCGETCPHYRD (SIOCDEVPRIVATE+9)
extern struct MediaEnv media;

#define IX_ETH_MII_SR_LINK_STATUS   0x0004 /* link Status -- 1 = link */
#define IX_ETH_MII_SR_AUTO_SEL      0x0008 /* auto speed select capable */
#define IX_ETH_MII_SR_REMOTE_FAULT  0x0010 /* Remote fault detect */
#define IX_ETH_MII_SR_AUTO_NEG      0x0020 /* auto negotiation complete */
#define IX_ETH_MII_SR_10T_HALF_DPX  0x0800 /* 10BaseT HD capable */
#define IX_ETH_MII_SR_10T_FULL_DPX  0x1000 /* 10BaseT FD capable */
#define IX_ETH_MII_SR_TX_HALF_DPX   0x2000 /* TX HD capable */
#define IX_ETH_MII_SR_TX_FULL_DPX   0x4000 /* TX FD capable */

int get_sockfd()
{

	 int sockfd = -1;

		//if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			perror("user: socket creating failed");
			return (-1);
		}
	return sockfd;
}


int getIPAddress(char *ifname, char *pIPStr)
{
	struct ifreq ifr;
	struct sockaddr_in *saddr;
	int fd;

	fd = get_sockfd();
	if (fd >= 0 ) {
		memset(&ifr,0,sizeof(ifr));
		strcpy(ifr.ifr_name, ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
		{
			saddr = (struct sockaddr_in *)&ifr.ifr_addr;
			close(fd);
			strcpy(pIPStr,inet_ntoa(saddr->sin_addr));
			return 0;
		}
		else {
			close(fd);
			return -1;
		}
	}
	return -1;
}

int substr(char *infile, char *outfile, char *replace, char *to)
{
	#define TMP_FILE "/tmp/upnp_tmp"
	FILE *fpi, *ftmp;
        char inputfile[256], outputfile[256];
        char inputbuf[4096], outputbuf[4096];
	 char bufcmd[256];
        int buff_len, replace_len, to_len;
        int i, j;

        sprintf(inputfile, "%s", infile);
        if ((fpi = fopen(inputfile,"r")) == NULL)
        {
//                printf("input file can not open\n");
                return (-1);
        }

        sprintf(outputfile, "%s",TMP_FILE);
        if ((ftmp = fopen(outputfile,"w")) == NULL) {
//                printf("output file can not open\n");
                fclose(fpi);
                return (-1);
        }

        replace_len = strlen(replace);
        to_len   = strlen(to);

        while (fgets(inputbuf, 4096, fpi) != NULL)
        {
                buff_len = strlen(inputbuf);
                for (i=0, j=0; i <= buff_len-replace_len; i++, j++)
                {
                        if (strncmp(inputbuf+i, replace, replace_len)==0)
                        {
                                strcpy (outputbuf+j, to);
                                i += replace_len - 1;
                                j += to_len - 1;
                        } else
                        *(outputbuf + j) = *(inputbuf + i);
                }
                strcpy(outputbuf + j, inputbuf + i);
                fputs(outputbuf, ftmp);
        }

        fclose(ftmp);
        fclose(fpi);
	 sprintf(bufcmd,"cp -f %s %s",TMP_FILE,outfile);
	 system(bufcmd);
	 unlink(TMP_FILE);
        return (0);
}


#if 0

unsigned int getIPAddress2(char *ifname)
{
	struct ifreq ifr;
	struct sockaddr_in *saddr;
	int fd;

	fd = get_sockfd();
	if (fd >= 0 )
	{
		memset(&ifr,0,sizeof(ifr));
		strcpy(ifr.ifr_name, ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
		{
			saddr = (struct sockaddr_in *)&ifr.ifr_addr;
			close(fd);
			return saddr->sin_addr.s_addr;
		}
		else
		{
			//perror("get IP address");
			close(fd);
			return 0;
		}
	}
	return 0;
}

/** Typedefs. **/

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned long   UINT32;

int getUptime(void)
{
	struct sysinfo info;
	sysinfo(&info);
	return (info.uptime - media.startup_time);
}

int getLayer1MaxBitRate(void)
{
//if (getConnectionStatus()==1) {
	int wan_speed=get_ethernet_link_status("4");
	if(wan_speed==-1)
		wan_speed=0;
	wan_speed*=1000*1000;
	media.UpstreamSpeed=wan_speed;
	media.DownstreamSpeed=wan_speed;
#if 0
	media.DownstreamSpeed=1000*1000*1000;
	media.UpstreamSpeed=1000*1000*1000;
}
else {
	media.DownstreamSpeed=0;
	media.UpstreamSpeed=0;
}
#endif
return 1;
}

/* it seems that getTotalBytesSent(void) & getTotalBytesReceived(void)
   are not being used */

unsigned long getTotalBytesSent(void)
{
	char dev[15];
	FILE *stream;
        unsigned long bytes=0, total=0;

	/* Read sent from /proc */
	stream = fopen ( "/proc/net/dev", "r" );
	if ( stream != NULL )
	{
		while ( getc ( stream ) != '\n' );
		while ( getc ( stream ) != '\n' );

		while ( !feof( stream ) )
		{
			fscanf ( stream, "%[^:]:%*u %*u %*u %*u %*u %*u %*u %*u %lu %*u %*u %*u %*u %*u %*u %*u\n", dev, &bytes );
                        if ( strcmp ( dev, media.ifname)==0 )
                                total += bytes;
		}
		fclose ( stream );
	}
   return total;


}

unsigned long getTotalBytesReceived(void)
{
	char dev[15];
	FILE *stream;
        unsigned long bytes=0, total=0;

	/* Read sent from /proc */
	stream = fopen ( "/proc/net/dev", "r" );
	if ( stream != NULL )
	{
		while ( getc ( stream ) != '\n' );
		while ( getc ( stream ) != '\n' );

		while ( !feof( stream ) )
		{
                        fscanf ( stream, "%[^:]:%lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", dev, &bytes );
                        if ( strcmp ( dev, media.ifname)==0 )
                                total += bytes;
		}
		fclose ( stream );
	}
   return total;

}

int getNATEnabled(void)
{
#ifdef NVRAM
	char *fw_nat;

	fw_nat=nvram_safe_get("fw_nat");
	if(*fw_nat=='0')
	{
			free(fw_nat);
			return 0;
	}
	if(*fw_nat=='1')
	{
			free(fw_nat);
			return 1;
	}
	else
	{
			free(fw_nat);
			return -1;
	}
#else

	return 1;
#endif

}



unsigned long getTotalPacketsSent(void)
{

	char dev[15];
	FILE *stream;
        unsigned long pkt=0, total=0;

	/* Read sent from /proc */
	stream = fopen ( "/proc/net/dev", "r" );
	if ( stream != NULL )
	{
		while ( getc ( stream ) != '\n' );
		while ( getc ( stream ) != '\n' );

		while ( !feof( stream ) )
		{
                        fscanf ( stream, "%[^:]:%*u %*u %*u %*u %*u %*u %*u %*u %*u %lu %*u %*u %*u %*u %*u %*u\n", dev, &pkt );
                        if ( strcmp ( dev, media.ifname)==0 )
                                total += pkt;
		}
		fclose ( stream );
	}
   return total;
}

unsigned long getTotalPacketsReceived(void)
{

	char dev[15];
        FILE *stream;
        unsigned long pkt=0, total=0;

        /* Read sent from /proc */
        stream = fopen ( "/proc/net/dev", "r" );
        if ( stream != NULL )
        {
                while ( getc ( stream ) != '\n' );
                while ( getc ( stream ) != '\n' );

                while ( !feof( stream ) )
                {
                        fscanf ( stream, "%[^:]:%*u %lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", dev, &pkt );
                        if ( strcmp ( dev, media.ifname )==0 )
                                total += pkt;
                }
                fclose ( stream );
        }

	//printf(" received  %lu\n",total);
	return total;

}

int getEnabledForInternet(void)
{
      return 1;
}

int getDHCPServerConfigurable(void)
{
   return 1; // yes
}

int getDHCPRelay(void)
{
   return 1; // yes
}

char * getSubnetMask(void)
{
    char *netmask="255.255.255.0";
    return netmask;
}

char * getIPRouters(void)
{
   return NULL;
}

char * getDNSServers(void)
{
      return NULL;
}

char * getDomainName(void)
{
   return NULL;
}

char * getMinAddress(void)
{
   return NULL;
}

char * getMaxAddress(void)
{
   return NULL;
}

char * getReservedAddress(void)
{
   return NULL;
}

char * getHostName(void)
{
	char *host="UPNP_HOST";
	return  host;
}


static int get_ethernet_link(char *_phy_id)
{
	int s;
	struct ifreq ifr;
	unsigned short *data, statRegval, regval4, regval5, regval;
	unsigned phy_id;
	return 100; //will update this function after IOCTL is supported by KENEL.
	s = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ - 1);
	if (ioctl(s, SIOCGMIIPHY, &ifr) < 0) {
		perror("ioctl errorA");
		exit(1);
	}

	data	= (unsigned short *)&ifr.ifr_data;
	phy_id	= data[0];

	data[1]	= 1;

	if (ioctl(s, SIOCGMIIREG, &ifr) < 0) {
		perror("ioctl errorB");
		exit(1);
	}

	statRegval	= data[3];
//	printf("statRegval:\t %X\n", statRegval);
	
		data[1]	= 4;

	if (ioctl(s, SIOCGMIIREG, &ifr) < 0) {
		perror("ioctl errorB");
		exit(1);
	}

	regval4	= data[3];
//	printf("regval4:\t %X\n", regval4);
	
		data[1]	= 5;

	if (ioctl(s, SIOCGMIIREG, &ifr) < 0) {
		perror("ioctl errorB");
		exit(1);
	}

	regval5	= data[3];
//	printf("regval5:\t %X\n", regval5);

	close(s);
				
	regval = (statRegval & ((regval4 & regval5) << 6));

  /* initialise from status register values */
  if ((regval & IX_ETH_MII_SR_TX_FULL_DPX) != 0)
  {
      /* 100 Base X full dplx */
//      *speed100 = TRUE;
//      *fullDuplex = TRUE;
      return 100;
  }
  if ((regval & IX_ETH_MII_SR_TX_HALF_DPX) != 0)
  {
      /* 100 Base X half dplx */
//      *speed100 = TRUE;
      return 100;
  }
  if ((regval & IX_ETH_MII_SR_10T_FULL_DPX) != 0)
  {
     /* 10 mb full dplx */
//      *fullDuplex = TRUE;
      return 10;
  }
  if ((regval & IX_ETH_MII_SR_10T_HALF_DPX) != 0)
  {
      /* 10 mb half dplx */
      return 10;
  }		
}
static int get_ethernet_link_status(char *phy_id)
{
	get_ethernet_link(phy_id);
	return get_ethernet_link(phy_id);
}

#endif
