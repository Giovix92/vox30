/*
 * socket.c -- DHCP server client/server socket creation
 *
 * Moreton Bay DHCP Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <features.h>
#include <linux/filter.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif

#include "debug.h"
#include "utility.h"

int read_interface(char *interface, int *ifindex, u_int32_t *addr, unsigned char *arp)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *sin;

	memset(&ifr, 0, sizeof(struct ifreq));
	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, interface);

		if (addr) {
			if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
				sin = (struct sockaddr_in *) &ifr.ifr_addr;
				*addr = sin->sin_addr.s_addr;
				DEBUG(LOG_INFO, "%s (our ip) = %s", ifr.ifr_name, inet_ntoa(sin->sin_addr));
			} else {
				LOG(LOG_ERR, "SIOCGIFADDR failed!: %s", strerror(errno));
				return -1;
			}
		}

		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			DEBUG(LOG_INFO, "adapter index %d", ifr.ifr_ifindex);
			*ifindex = ifr.ifr_ifindex;
		} else {
			LOG(LOG_ERR, "SIOCGIFINDEX failed!: %s", strerror(errno));
			return -1;
		}
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
			DEBUG(LOG_INFO, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x",
				arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
		} else {
			LOG(LOG_ERR, "SIOCGIFHWADDR failed!: %s", strerror(errno));
			return -1;
		}
	} else {
		LOG(LOG_ERR, "socket failed!: %s", strerror(errno));
		return -1;
	}
	close(fd);
	return 0;
}

#if defined(__SC_BUILD__)
int listen_socket(unsigned int ip, int port, char *inf, int ip_pktinfo_need)
#else
int listen_socket(unsigned int ip, int port, char *inf)
#endif
{
	struct ifreq interface;
	int fd;
	struct sockaddr_in addr;
	int n = 1;

	DEBUG(LOG_INFO, "Opening listen socket on 0x%08x:%d %s\n", ip, port, inf);
	
	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		DEBUG(LOG_ERR, "socket call failed: %s", strerror(errno));
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ip;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1) {
#ifdef __SC_BUILD__
        DEBUG(LOG_ERR, "setsockopt SO_REUSEADDR failed: %s\n", strerror(errno));
#endif
		close(fd);
		return -1;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n)) == -1) {
#ifdef __SC_BUILD__
        DEBUG(LOG_ERR, "setsockopt SO_BROADCAST failed: %s", strerror(errno));
#endif
		close(fd);
		return -1;
	}
#if defined(__SC_BUILD__)
    if (ip_pktinfo_need)
    {
        if (setsockopt(fd, IPPROTO_IP, IP_PKTINFO, (char *) &n, sizeof(n)) == -1 ) {
#ifdef __SC_BUILD__
            DEBUG(LOG_ERR, "setsockopt IP_PKTINFO failed: %s", strerror(errno));
#endif
            close(fd);
            return -1;
        }
    }
#endif

    if(inf && strlen(inf))
    {
        strncpy(interface.ifr_ifrn.ifrn_name, inf, IFNAMSIZ);
        if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&interface, sizeof(interface)) < 0) {
#ifdef __SC_BUILD__
            DEBUG(LOG_ERR, "setsockopt SO_BINDTODEVICE failed: %s", strerror(errno));
#endif
            close(fd);
            return -1;
        }
    }
	if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
#ifdef __SC_BUILD__
        DEBUG(LOG_ERR, "bind failed: %s", strerror(errno));
#endif
		close(fd);
		return -1;
	}
	
#ifdef __SC_BUILD__
    DEBUG(LOG_DEBUG, "Open listen socket successfully.");
#endif

	return fd;
}
#ifdef __SC_BUILD__
int raw_socket(int ifindex, unsigned int mark, unsigned int sc_mark)
#else
int raw_socket(int ifindex)
#endif
{
	int fd;
	struct sockaddr_ll sock;
#ifdef __SC_BUILD__
    struct sock_fprog filter;
    struct sock_filter bpf_code[] =
    {
        { 0x28, 0, 0, 0x00000016 }, //0
        { 0x15, 0, 1, 0x00000044 },//1
        { 0x6, 0, 0, 0x0000ffff },//2
        { 0x6, 0, 0, 0x00000000 },//3
    };
    filter.len = sizeof(bpf_code)/sizeof(bpf_code[0]);
    filter.filter = bpf_code;
#endif
	DEBUG(LOG_INFO, "Opening raw socket on ifindex %d\n", ifindex);
	if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		DEBUG(LOG_ERR, "socket call failed: %s", strerror(errno));
		return -1;
	}
#ifdef __SC_BUILD__
    setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter));
    setsockopt(fd, SOL_SOCKET, SO_SC_MARK, &sc_mark, sizeof(sc_mark));
    setsockopt(fd, SOL_SOCKET, SO_MARK, &mark, sizeof(mark));
#endif

	sock.sll_family = AF_PACKET;
	sock.sll_protocol = htons(ETH_P_IP);
	sock.sll_ifindex = ifindex;
	if (bind(fd, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
		DEBUG(LOG_ERR, "bind call failed: %s", strerror(errno));
		close(fd);
		return -1;
	}
	
#ifdef __SC_BUILD__
    DEBUG(LOG_DEBUG, "bind call failed: %s", strerror(errno));
#endif

	return fd;

}

