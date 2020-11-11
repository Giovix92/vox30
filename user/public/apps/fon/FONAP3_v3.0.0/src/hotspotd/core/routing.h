#ifndef ROUTING_H_
#define ROUTING_H_

#include <stdint.h>
#include <netinet/in.h>

extern struct routing_cfg {
	char iface_name[16];
	char ifb_name[16];
	char ifb2_name[16];
	char iface_eap_name[16];
	char iface_priv_name[16];
#ifdef __SC_BUILD__
	int wifi_eap_iface_index;
	int wifi_iface_index;
#endif
	int iface_index;
	int ifb_index;
	int ifb2_index;
	int iface_eap_index;
	int iface_priv_index;
	uint8_t iface_addr[8];
} routing_cfg;


int routing_policy_new(const char *iface, int af, uint32_t fwmark,
		uint32_t fwmask, uint32_t ifindex);
ssize_t routing_addresses(int iface, int af, void *addr, size_t len);

int routing_policy_del(int policy);
int routing_connkill(int af, const void *addr);

int routing_disconnect(int ifindex, const uint8_t hwaddr[6]);

#endif /* ROUTING_H_ */
