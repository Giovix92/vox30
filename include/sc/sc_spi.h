#ifndef __SC_SPI__H_
#define __SC_SPI__H_

#include <linux/types.h>
#include <linux/param.h>
#include <linux/skbuff.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_core.h>

//#define CONFIG_SPI_FIRWALL
#define TCP_SYN_RCV_TIMEOUT  (5*HZ)

#define SEQ_OPERATION

#define MAX_CONNTRACK_COUNT	4096

extern int g_spi_enable;
extern int g_spi_br_index;

extern int nf_ct_tcp_syn_flood_enable;
extern int nf_ct_tcp_syn_flood_speed;
extern int nf_ct_tcp_fin_flood_enable;
extern int nf_ct_tcp_fin_flood_speed;
extern int nf_ct_tcp_port_scan_enable;
extern unsigned int tcp_timeouts[];

extern unsigned int nf_ct_icmp_timeout;
extern int nf_ct_icmp_flood_enable;
extern int nf_ct_icmp_flood_speed;
extern int nf_ct_udp_flood_enable;
extern int nf_ct_udp_flood_speed;
extern int nf_ct_udp_port_scan_enable;
extern int nf_ct_udp_bomb_enable;
extern unsigned int nf_ct_udp_timeout;
extern int nf_conntrack_ip_land;
extern int nf_conntrack_brdcst_src;
extern int nf_ct_tcp_syn_with_data_enable;
extern int nf_conntrack_fw_block_enable;
extern int nf_conntrack_dmz_enable;
extern unsigned int nf_conntrack_block_time;
extern int nf_conntrack_port_scan_max;
#define BLACK_ENTRY 		0
#define WHITE_ENTRY 		1
#define LOCAL_SERVICE_ENTRY 2

#define DETECT_NOT_MATCH		0
#define DETECT_BLOCK			1
#define DETECT_PASS				2
#define DETECT_LOCAL_SERVICE	4

#define IPPROTO_BOTH 255

#define BLOCK_INFINITE_TIME	0xFFFFFFFF

#define RESET_TCP_SYN_BYSRCIP	0x01
#define RESET_CLAMP_BYREMOTEIP	0x02

/* block function */
extern int (*sc_check_and_block_hook)(struct sk_buff *skb, const struct nf_conntrack_tuple *tuple);
extern int (*sc_add_block_pattern_hook)(struct nf_conntrack_tuple *tuple,
								struct nf_conntrack_tuple *mask,
								int dport_min,
								int dport_max,
								int type,
								int	timeout);
extern int (*sc_delete_block_pattern_hook)(struct nf_conntrack_tuple *tuple,
									int dport_min,
									int dport_max,
									int type);
/* special block function for port trigger */
extern int (*sc_add_special_block_pattern_hook)(int dport_min, int dport_max, 
										int type, u_int8_t protonum);
extern int (*sc_delete_special_block_pattern_hook)(int dport_min, int dport_max,
										int type, u_int8_t protonum);

extern int (*sc_check_block_src_protonum)(const struct sk_buff *skb);

/* Smurf Attack, Fraggle Attack, ICMP Flood/ICMP Storm Attack, 
	Ping Flood, UDP Flood/UDP Storm Attack function */
extern unsigned int (*sc_fake_source_detect_hook)(struct sk_buff *skb, struct nf_conn *ct, u_int8_t protonum);

/* psd and TCP/UDP Echo/Chargen Attack  detect */
extern unsigned int (*sc_psd_and_special_udp_detect_hook)(struct sk_buff *skb, struct nf_conn *ct, u_int8_t protonum, unsigned int dataoff);

/* tcp syn flood detect */
extern unsigned int (*sc_tcp_check_hook)(struct sk_buff *skb, struct nf_conn *ct, u_int8_t pf, unsigned int dataoff);
extern void (*sc_tcp_destroy_hook)(struct nf_conn *ct);
extern void (*sc_tcp_deal_establish_hook)(struct nf_conn *ct);

extern unsigned int (*sc_tcp_fin_check_hook)(struct sk_buff *skb, u_int8_t pf, u_int8_t protonum);
/* icmp flood detect */
extern unsigned int (*sc_icmp_check_hook)(struct sk_buff *skb, u_int8_t pf, struct nf_conn *ct);

/* broadcast as source ip detect */
extern unsigned int (*sc_brdcst_src_check_hook)(struct sk_buff *skb);
/*For a host on the Internet (i.e. a remote IP address), it can generate at most 100 inbound NAT sessions
 in total at a time to all Port Forwarding services and to DMZ.*/
extern unsigned int (*sc_detect_total_session_for_one_host_hook)(struct sk_buff *skb, struct nf_conn *ct);
extern void (*sc_remote_destroy_hook)(struct nf_conn *ct);
extern unsigned int (*sc_clamp_refresh_hook)(struct nf_conn *ct);
extern unsigned int (*sc_clamp_is_clamped_session_hook)(struct nf_conn *ct);


/* ip check */
extern unsigned int (*sc_packet_ip_check_hook)(struct sk_buff *skb, u_int8_t pf);
extern unsigned int (*sc_udp_check_hook)(struct sk_buff *skb, struct nf_conn *ct);
extern void (*sc_udp_destroy_hook)(struct nf_conn *ct);


#endif
