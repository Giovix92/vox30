#ifndef _PACKET_H
#define _PACKET_H

#include <netinet/udp.h>
#include <netinet/ip.h>
#ifdef __SC_BUILD__
#define MAX_DHCP_OPTIONS_LEN 1232
#endif
struct dhcpMessage {
	u_int8_t op;        // message type
	u_int8_t htype;     // hardware type
	u_int8_t hlen;      // hardware address length
	u_int8_t hops;      // hops
	u_int32_t xid;      // transaction id
	u_int16_t secs;     // seconds elapsed
	u_int16_t flags;    // flags
	u_int32_t ciaddr;   // client ip address
	u_int32_t yiaddr;   // your ip address
	u_int32_t siaddr;   // next server ip address
	u_int32_t giaddr;   // relay agent ip address
	u_int8_t chaddr[16];// client mac address
	u_int8_t sname[64]; // server host name
	u_int8_t file[128]; // boot file name
	/* A DHCP client must be prepared to receive DHCP messages with an 'options' field of
       at least length 312 octets. This requirement implies that a DHCP client must be
       prepared to receive a message of up to 576 octets, the minimum IP datagram size an
       IP host must be prepared to accept. DHCP clients may negotiate the use of larger
       DHCP messages through the "Maximum DHCP message size" option (57). (RFC 2131) */
	u_int32_t cookie;   // magic cookie
#ifdef __SC_BUILD__
    u_int8_t options[MAX_DHCP_OPTIONS_LEN] ;
#else
	u_int8_t options[308]; /* 312 - sizeof(cookie) */
#endif	
};

struct udp_dhcp_packet {
	struct iphdr ip;
	struct udphdr udp;
	struct dhcpMessage data;
};

void init_header(struct dhcpMessage *packet, char type);
#if defined(__SC_BUILD__)
int get_packet(struct dhcpMessage *packet, int fd, struct in_pktinfo *pkt_info);
#else
int get_packet(struct dhcpMessage *packet, int fd);
#endif
u_int16_t checksum(void *addr, int count);
#if defined(__SC_BUILD__)
int raw_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port, unsigned char *dest_arp, int ifindex, unsigned int mark, unsigned int sc_mark);
#else
int raw_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port, unsigned char *dest_arp, int ifindex);
#endif
int kernel_packet(struct dhcpMessage *payload, u_int32_t source_ip, int source_port,
		   u_int32_t dest_ip, int dest_port);


#endif
