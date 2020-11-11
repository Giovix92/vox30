#ifndef FIREWALL_H_
#define FIREWALL_H_

#include <stdbool.h>
#ifdef __SC_BUILD__
// replace skb mark[26:19] instead
#define SKBMARK_FON_S       19    
#define FIREWALL_KILL		(0xff << SKBMARK_FON_S)	/* reject this traffic */
#define FIREWALL_TPROXY		(0xfe << SKBMARK_FON_S)	/* route this traffic to local */
#define FIREWALL_WHITELIST	(0xfd << SKBMARK_FON_S)	/* the garden */
#define FIREWALL_AUTHMASK	(0xff << SKBMARK_FON_S)	/* user id */
#else
#define FIREWALL_KILL		0x0fff0000	/* reject this traffic */
#define FIREWALL_TPROXY		0x0ffe0000	/* route this traffic to local */
#define FIREWALL_WHITELIST	0x0ffd0000	/* the garden */
#define FIREWALL_AUTHMASK	0x0fff0000	/* user id */
#endif
#ifdef __SC_BUILD__
#define SKBMARK_FON_CLS_S       5    
// replace skb mark[8:5] instead
#define FIREWALL_CLASSMASK	(0x7 << SKBMARK_FON_CLS_S)	/* traffic class */

#define FIREWALL_CLS_INTER	(0x7 << SKBMARK_FON_CLS_S)	/* interactive traffic */
#define FIREWALL_CLS_SMALL	(0x3 << SKBMARK_FON_CLS_S)	/* small traffic */
#define FIREWALL_CLS_BULK	(0x1 << SKBMARK_FON_CLS_S)	/* bulk traffic */
#else
#define FIREWALL_CLASSMASK	0x70000000	/* traffic class */

#define FIREWALL_CLS_INTER	0x70000000	/* interactive traffic */
#define FIREWALL_CLS_SMALL	0x30000000	/* small traffic */
#define FIREWALL_CLS_BULK	0x10000000	/* bulk traffic */
#endif

#define FIREWALL_SERVICE_EXTERNAL 0x10000

/**
 * Control access to local services.
 * Create firewall rules to accept / reject access from hotspot clients
 * to a given port of a protocol.
 *
 * NOTE: service access rules are flushed when the config is reapplied!
 *
 * returns status
 */
int firewall_set_service(int proto, int port, bool accept);

/**
 * Control access to local service per mac.
 * Create firewall rules to accept / reject access from specific hotspot clients
 * to a given port of a protocol.
 *
 * returns status
 */
int firewall_set_client_service(const uint32_t id, int proto, 
			const char *port, bool accept);

/**
 * Control access of unauthorized users to external hosts.
 * Create a firewall rules to accept / reject traffic of unauthorized users
 * to a given network (denoted by address and prefix) for a given adress family
 *
 * returns status
 */
int firewall_set_whitelist(int af, const void *addr, uint8_t prefix, bool accept);

/**
 * Control authentication states for users.
 * Mark / stop marking all traffic of a user with hardware-address hwaddr
 * with the given firewall mark.
 *
 * returns status
 */
int firewall_set_auth(int ifindex, const uint8_t *hwaddr, uint32_t id, bool set);

/**
 * Add IP to the list of authroized IPs for a user
 */
int firewall_add_auth_ip(uint32_t id, int af, const void *addr);

/**
 * Flush all IP address for which a client is authorized
 */
void firewall_flush_auth_ips(uint32_t id);

/**
 * Control traffic interception.
 * Add / remove firewall rules to intercept traffic of a given adress family af
 * on a given port of a given protocol and redirect them to local port dport.
 *
 * returns status
 */
int firewall_set_redirect(int proto, int port, int dport, bool redirect);

/**
 * Control traffic logging.
 * Start / stop logging traffic of a given adress family af and source address
 * of a given port of protocol proto to the netfilter log group with ID group.
 */
int firewall_set_nfqueue_out(int proto, int port, uint32_t group, bool set,
		bool nfqueue);

/**
 * Control traffic logging.
 * Start / stop logging traffic of a given adress family af and source address
 * of a given port of protocol proto to the netfilter log group with ID group.
 */
int firewall_set_nfqueue_src(int family, const void *addr,
		int proto, int port, uint32_t grp, bool set, bool nfqueue);

/**
 * Control NAT masquerading.
 * Add / remove a NAT masquerading rule for traffic leaving a given interface.
 *
 * returns status
 */
int firewall_set_nat(const char *dev, bool set);

/**
 * Convert a given firewall mark to its corresponding user ID
 */
static inline uint32_t firewall_authmark_to_id(uint32_t mark) {
#ifdef __SC_BUILD__
	return (mark & FIREWALL_AUTHMASK) >> SKBMARK_FON_S;
#else
	return (mark & FIREWALL_AUTHMASK) >> 16;
#endif
}

/**
 * Convert a given user ID to its corresponding firewall mark
 */
static inline uint32_t firewall_id_to_authmark(uint32_t id) {
#ifdef __SC_BUILD__
	return (id << SKBMARK_FON_S) & FIREWALL_AUTHMASK;
#else
	return (id << 16) & FIREWALL_AUTHMASK;
#endif
}

#endif /* CAPTIVE_H_ */
