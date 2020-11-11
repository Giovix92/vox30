#ifndef GRE_H_
#define GRE_H_

#include <stdint.h>

struct net_device_info {
	unsigned long int rx_packets;	/* total packets received       */
	unsigned long int tx_packets;	/* total packets transmitted    */
	unsigned long int rx_bytes;	/* total bytes received         */
	unsigned long int tx_bytes;	/* total bytes transmitted      */
	unsigned long int rx_errors;	/* bad packets received         */
	unsigned long int tx_errors;	/* packet transmit problems     */
	unsigned long int rx_dropped;	/* no space in linux buffers    */
	unsigned long int tx_dropped;	/* no space available in linux  */
	unsigned long int rx_multicast;	/* multicast packets received   */
	unsigned long int rx_compressed;
	unsigned long int tx_compressed;
	unsigned long int collisions;
	
    unsigned long int rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long int tx_fifo_errors;
    unsigned long int rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long int tx_carrier_errors;
    unsigned long int tx_multicast_packets;
    unsigned long int rx_multicast_packets;
    unsigned long int tx_multicast_bytes;
    unsigned long int rx_multicast_bytes;
    unsigned long int tx_broadcast_packets;
    unsigned long int rx_broadcast_packets;
    unsigned long int tx_unicast_packets;
    unsigned long int rx_unicast_packets;
    unsigned long int rx_unknownproto_packets;

};
enum gre_state {
    GRE_NONE = 0,
	GRE_UNREACHABLE,
	GRE_CONNECTING,
	GRE_ESTABLISHED,
	GRE_SHUTDOWN,
};
struct gre_lac {
	struct event_timer timer;
	struct gre_cfg* cfg;
	enum gre_state state;
	int tunnel_id;
	int is_backup;
    int is_up;
    int  wan_id;
    unsigned long last_keepalive;
    unsigned long last_detectalive;
    unsigned long last_rx;
    unsigned long rx_packets;
    unsigned int  lost_pings;
    char cmd[32];
    char server_ip[16];
    char wan_if[16];
    char gre_if[16];
    union {
        struct sockaddr saddr;
        struct sockaddr_in6 saddr_in6;
    } addr;

};


struct SSID
{
    struct list_head _head;
    char* interface;
    char* name;
    char bridge[16];
    int is_eap;
    int vlan;
    int block_boardcast;
    int is_up;
#ifdef CONFIG_SUPPORT_WIFI_5G
    int is_5g;
#endif
};
struct VLAN
{
    char vlan_buf;
    int bridge_id;
};

typedef void (gre_cb)(enum gre_state state, struct gre_lac* lac);

struct gre_cfg {
	uint16_t enable;
	uint16_t type;
	uint16_t keepalive_interval;
	uint16_t timeout_num;
	uint16_t timeout_interval;
	uint16_t timeout;
	uint16_t ip_fragment;
    gre_cb *cb;
    struct list_head ssid;
};

struct gre_lac* gre_create(int tunnel_id, const char *host, const struct gre_cfg *cfg, int is_backup);
enum gre_state gre_state(struct gre_lac *lac);
void gre_destroy(struct gre_lac *lac);
char* get_setup_script(int wan_id, int tunnel_id, int is_backup);
int gre_cleanup(struct gre_lac *lac);
int tunnel_gre_get_status(void);
int SYSTEM(const char *format, ...);

#endif /* GRE_H_ */
