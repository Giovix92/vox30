#ifndef NEIGH_H_
#define NEIGH_H_

#include "lib/list.h"

enum neigh_event {
	NEIGH_ADD = 0,
	NEIGH_DEL = 1,
};


typedef void (neigh_handle_cb)(uint8_t mac[6], char ip[INET6_ADDRSTRLEN], enum neigh_event nevent); 

struct neigh_handler {
	struct list_head _head;
	neigh_handle_cb *cb;
	int ifindex;
};

int neigh_ip2mac(int ifindex, uint8_t hwaddr[6], int af, const void *addr);
ssize_t neigh_mac2ip(int ifindex, int af, void *addr, size_t len, const uint8_t hwaddr[6]);
void neigh_handler(struct neigh_handler *neh);
void neigh_deregister_handler(struct neigh_handler *neh);

#endif /* NEIGH_H_ */
