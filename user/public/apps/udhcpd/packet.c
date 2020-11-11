#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <features.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif
#include <errno.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"


void init_header(struct dhcpMessage *packet, char type)
{
	memset(packet, 0, sizeof(struct dhcpMessage));
	switch (type) {
	case DHCPDISCOVER:
	case DHCPREQUEST:
	case DHCPRELEASE:
	case DHCPINFORM:
		packet->op = BOOTREQUEST;
		break;
	case DHCPOFFER:
	case DHCPACK:
	case DHCPNAK:
		packet->op = BOOTREPLY;
	}
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
}


/* read a packet from socket fd, return -1 on read error, -2 on packet error */
#if defined(__SC_BUILD__)
int get_packet(struct dhcpMessage *packet, int fd, struct in_pktinfo *pkt_info)
#else
int get_packet(struct dhcpMessage *packet, int fd)
#endif
{
	int bytes;
	int i;
	const char broken_vendors[][8] = {
		"MSFT 98",
		""
	};
	char unsigned *vendor;
#if defined(__SC_BUILD__)
    struct in_pktinfo*  ptr_in_pktinfo = NULL;
    struct msghdr       msg;
    struct cmsghdr*     cmsg;
    struct iovec        iov;
    char                ctrl[sizeof(struct cmsghdr) + sizeof(struct in_pktinfo)];
#endif

    memset(packet, 0, sizeof(struct dhcpMessage));
#if defined(__SC_BUILD__)
    if (pkt_info)
    {
        memset ((void *)&msg, 0, sizeof (msg));
        iov.iov_base        = (char*)packet;
        iov.iov_len         = sizeof(struct dhcpMessage);
        msg.msg_iov         = &iov;
        msg.msg_iovlen      = 1;
        msg.msg_control     = ctrl;
        msg.msg_controllen  = sizeof(ctrl);
        bytes = recvmsg (fd, &msg, 0);
    }
    else
#endif
	bytes = read(fd, packet, sizeof(struct dhcpMessage));
	if (bytes < 0) {
		DEBUG(LOG_INFO, "couldn't read on listening socket, ignoring");
		return -1;
	}

	if (ntohl(packet->cookie) != DHCP_MAGIC) {
		LOG(LOG_ERR, "received bogus message, ignoring");
		return -2;
	}
	DEBUG(LOG_INFO, "Received a packet");

#if defined(__SC_BUILD__)
    if (pkt_info)
    {
        /* Cycle through the data and get packet information. */
        for(cmsg=CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg,cmsg))
        {
            /* Is this the packet information ? */
            if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO)
                ptr_in_pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);
        }

        if (ptr_in_pktinfo)
        {
            memcpy(pkt_info, ptr_in_pktinfo, sizeof(struct in_pktinfo));
        }
        else
        {
            DEBUG(LOG_WARNING, "Not find the in_pktinfo in the CMSG_DATA.");
        }
    }
#endif

	if (packet->op == BOOTREQUEST && (vendor = get_option(packet, DHCP_VENDOR))) {
		for (i = 0; broken_vendors[i][0]; i++) {
			if (vendor[OPT_LEN - 2] == (unsigned char) strlen(broken_vendors[i]) &&
			    !strncmp(vendor, broken_vendors[i], vendor[OPT_LEN - 2])) {
			    	DEBUG(LOG_INFO, "broken client (%s), forcing broadcast",
			    		broken_vendors[i]);
			    	packet->flags |= htons(BROADCAST_FLAG);
			}
		}
	}

	return bytes;
}

u_int16_t checksum(void *addr, int count)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	register int32_t sum = 0;
	u_int16_t *source = (u_int16_t *) addr;

	while( count > 1 )  {
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any*/

	if (count > 0) {
		/* Make sure that the left-over byte is added correctly both
		 * with little and big endian hosts */
		u_int16_t tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}

	/*  Fold 32-bit sum to 16 bits */
	while (sum>>16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}
#ifdef __SC_BUILD__
static int minimum_dhcp_len(struct dhcpMessage *pDhcp)
{
    unsigned char *options;
    unsigned char *tail;
    int totalLen;

    options = pDhcp->options;
    tail = options;
    while(tail!=NULL && *tail!=0xff )
    {
        tail += tail[OPT_LEN] + 2 ;
        if( (int)(tail - (unsigned char*)pDhcp) >= sizeof(struct dhcpMessage) )
        {
            return  sizeof(struct dhcpMessage);
        }
    }
    tail++;// should include the end code 0xff
    totalLen = (int)(tail - (unsigned char* )pDhcp);
    if( totalLen%2 )
    {
        totalLen++; // padding for udp checksum
    }
    return totalLen;
}
#endif
/* Constuct a ip/udp header for a packet, and specify the source and dest hardware address */
#ifdef __SC_BUILD__
int raw_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port, unsigned char *dest_arp, int ifindex, unsigned int mark, unsigned int sc_mark)
#else
int raw_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port, unsigned char *dest_arp, int ifindex)
#endif
{
	int fd;
	int result;
	struct sockaddr_ll dest;
	struct udp_dhcp_packet packet;
#ifdef __SC_BUILD__
    int dhcpMinimumLen;
#endif
	if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		DEBUG(LOG_ERR, "socket call failed: %s", strerror(errno));
		return -1;
	}
	
	memset(&dest, 0, sizeof(dest));
	memset(&packet, 0, sizeof(packet));
	
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_IP);
	dest.sll_ifindex = ifindex;
	dest.sll_halen = 6;
	memcpy(dest.sll_addr, dest_arp, 6);
	if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0) {
		DEBUG(LOG_ERR, "bind call failed: %s", strerror(errno));
		close(fd);
		return -1;
	}
#ifdef __SC_BUILD__
    memset((payload->chaddr)+MAC2_OFFSET, 0, MAC_LENGTH);
    //calc the minimum dhcp packet len
    dhcpMinimumLen = minimum_dhcp_len(payload);
    setsockopt(fd, SOL_SOCKET, SO_SC_MARK, &sc_mark, sizeof(sc_mark));
    setsockopt(fd, SOL_SOCKET, SO_MARK, &mark, sizeof(mark));
#endif
	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source_ip;
	packet.ip.daddr = dest_ip;
	packet.udp.source = htons(source_port);
	packet.udp.dest = htons(dest_port);
#ifdef __SC_BUILD__
    packet.udp.len = htons(sizeof(packet.udp) + dhcpMinimumLen);
#else
	packet.udp.len = htons(sizeof(packet.udp) + sizeof(struct dhcpMessage)); /* cheat on the psuedo-header */
#endif
	packet.ip.tot_len = packet.udp.len;
	memcpy(&(packet.data), payload, sizeof(struct dhcpMessage));
#ifdef __SC_BUILD__
	packet.udp.check = checksum(&packet,  sizeof(struct iphdr)+sizeof(struct udphdr)+dhcpMinimumLen);
	
	packet.ip.tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + dhcpMinimumLen); 
#else
	packet.udp.check = checksum(&packet, sizeof(struct udp_dhcp_packet));

	packet.ip.tot_len = htons(sizeof(struct udp_dhcp_packet));
#endif
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = checksum(&(packet.ip), sizeof(packet.ip));
#ifdef __SC_BUILD__
    result = sendto(fd, &packet,sizeof(struct iphdr) + sizeof(struct udphdr) + dhcpMinimumLen, 0, (struct sockaddr *) &dest, sizeof(dest));
#else
	result = sendto(fd, &packet, sizeof(struct udp_dhcp_packet), 0, (struct sockaddr *) &dest, sizeof(dest));
#endif
	if (result <= 0) {
		DEBUG(LOG_ERR, "write on socket failed: %s", strerror(errno));
	}
	close(fd);
	return result;
}


/* Let the kernel do all the work for packet generation */
int kernel_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port)
{
	int n = 1;
	int fd, result;
	struct sockaddr_in client;
#ifdef __SC_BUILD__
    int dhcpMinimumLen;
#endif

	if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		return -1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1)
		return -1;

	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(source_port);
	client.sin_addr.s_addr = source_ip;

	if (bind(fd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1)
		return -1;

	memset(&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(dest_port);
	client.sin_addr.s_addr = dest_ip;

#ifdef __SC_BUILD__
    dhcpMinimumLen = minimum_dhcp_len(payload);
#endif
	if (connect(fd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1)
		return -1;

#ifdef __SC_BUILD__
	result = write(fd, payload, dhcpMinimumLen);
#else
	result = write(fd, payload, sizeof(struct dhcpMessage));
#endif
	close(fd);
	return result;
}

