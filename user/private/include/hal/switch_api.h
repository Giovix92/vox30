
#ifndef SWITCH_API_H

#define FAILED -1
#define MAX_VIDS 10

#define UNTAGGED_VID   4096
#define UNCONFIG_VID    -1

#define UPSTREAM_MASK          0x01
#define DOWNSTREAM_MASK     0x02

#define TAG_MODE     0
#define UNTAG_MODE 1

typedef struct ether_stat_port_cntr_s
{
    unsigned long long ifInOctets;
    unsigned int dot3StatsFCSErrors;
    unsigned int dot3StatsSymbolErrors;
    unsigned int dot3InPauseFrames;
    unsigned int dot3ControlInUnknownOpcodes;
    unsigned int etherStatsFragments;
    unsigned int etherStatsJabbers;
    unsigned int ifInUcastPkts;
    unsigned int etherStatsDropEvents;
    unsigned long long etherStatsOctets;
    unsigned int etherStatsUndersizePkts;
    unsigned int etherStatsOversizePkts;
    unsigned int etherStatsPkts64Octets;
    unsigned int etherStatsPkts65to127Octets;
    unsigned int etherStatsPkts128to255Octets;
    unsigned int etherStatsPkts256to511Octets;
    unsigned int etherStatsPkts512to1023Octets;
    unsigned int etherStatsPkts1024toMaxOctets;
    unsigned int etherStatsMcastPkts;
    unsigned int etherStatsBcastPkts;	
    unsigned long long ifOutOctets;
    unsigned int dot3StatsSingleCollisionFrames;
    unsigned int dot3StatsMultipleCollisionFrames;
    unsigned int dot3StatsDeferredTransmissions;
    unsigned int dot3StatsLateCollisions;
    unsigned int etherStatsCollisions;
    unsigned int dot3StatsExcessiveCollisions;
    unsigned int dot3OutPauseFrames;
    unsigned int dot1dBasePortDelayExceededDiscards;
    unsigned int dot1dTpPortInDiscards;
    unsigned int ifOutUcastPkts;
    unsigned int ifOutMulticastPkts;
    unsigned int ifOutBrocastPkts;
    unsigned int outOampduPkts;
    unsigned int inOampduPkts;
    unsigned int pktgenPkts;
}ether_stat_port_cntr_t;

struct port_counter
{
    int portid;
    ether_stat_port_cntr_t counter;
};

int hal_switch_get_ethernet_port_counter(int port_id, struct port_counter  *ptr);
/** 
 * Reset the switch, all phy link will drop.
 * 
 * @param none 
 * 
 * @return 
 */
extern void hal_switch_reset(void);

/** 
 * 1. Init the switch function and enable the VLAN function.
 * 2. Reset the switch global config.
 *
 * @param none 
 * 
 * @return 
 */
extern int hal_switch_init(void);

/** 
 * Set port vlan id and priority. If receive the upstream untagged packet
 * The switch will add the vid and pri.
 *
 * @portid Switch port index 
 * @vid     VLAN ID 
 * @pri      VLAN Priority 
 * 
 * @return 
 */
extern int hal_switch_set_port_vid(int portid, int vid, int pri);

/** 
  * Get the port vlan id and priority by port index.
 *
 * @portid Switch port index 
 * @vid     return VLAN ID 
 * @pri      return VLAN Priority 
 * 
 * @return the vlan id and priority
 */
extern int hal_switch_get_port_vid(int portid, int *vid, int *pri);

/** 
 * Let port join the vlan group.
 * Port can receive the upstream/downstream whose vlan id is equal to @group_vid
 *
 * @portid Switch port index 
 * @group_vid     group vlan 
 * @untag_mode  whether remove the tag for downstream 
 * 
 * @return the vlan id and priority
 */
extern int hal_switch_join_group(int portid, int group_vid, int untag_mode);

/** 
 * Let port leave the vlan group.
 * Port can not receive the upstream/downstream whose vlan id is equal to @group_vid.
 *
 * @portid Switch port index 
 * @group_vid     group vlan 
 * 
 * @return the vlan id and priority
 */
extern int hal_switch_leave_group(int portid, int group_vid);

/** 
 * Dump the switch configuration by vlan id.
 * This function will search the vlan group @group_vid, and record the port to port_map
 * and untag mode to untag_map.
 *
 * @group_vid     vlan group id 
 * @port_map     Bit-map for record the port. For example if (@port_map&(1<<@port_index)) != 0
 *                       means, @port_index belong to the vlan grooup @group_vid.
 * @untag_map   Bit-map for record the untag mode. For example if(@untag_map>>@port_index == 0x1)
 *                       means, @port_index is untag mode for @group_vid.
 * 
 * @return the vlan id and priority
 */
extern int hal_switch_get_group_by_vid(int group_vid, unsigned int *port_map, unsigned int *untag_map);

/** 
 * Dump the switch configuration by Port id. This function cost much CPU resource, just use for debug.
 *
 * @portid            Switch port index
 * @max_num      The max element eumber for @vid and @untag_mode
 * @output_num   Return the real vlan group number for @portid
 * @vid                Record the vlan group id
 * @untag_mode   Recoed the untag mode
 * 
 * @return the vlan id and priority
 */
extern int hal_switch_get_group_by_port(int portid, int max_num, int *output_num, int *vid, int *untag_mode);

enum eth_port_num
{
    ETH_PORT_1 = 0,
    ETH_PORT_2,
    ETH_PORT_3,
    ETH_PORT_4,
#if defined (FD1018) || defined (VOX25) || defined(VOX30)
    ETH_PORT_5,
#endif
    ETH_PORT_INVALID
};
#if defined (FD1018) || defined (VOX25) || defined(VOX30)
#define ETH_WAN_PORT  4
#else
#define ETH_WAN_PORT  3
#endif

enum eth_port_speed
{
    ETH_PORT_SPEED_ATUO = 0,
    ETH_PORT_SPEED_10M_FULL,
    ETH_PORT_SPEED_10M_HALF,
    ETH_PORT_SPEED_100M_FULL,
    ETH_PORT_SPEED_100M_HALF,
    ETH_PORT_SPEED_INVALID,
};

typedef enum
{
    ETH_PORT_LINK_DOWN = 0,
    ETH_PORT_LINK_UP,
    ETH_PORT_LINK_LAST
} SC_DRV_ETH_PORT_LINK_STATUS;

typedef struct eth_port_speed_cfg_s
{
    int speed[4];
}eth_port_speed_cfg_t;

typedef struct tag_eth_port_link_status
{
    unsigned long link;//mib ulIfLastChange actually;
    unsigned long speed;
    unsigned long duplex;
}eth_port_link_status;

typedef struct {
    unsigned long int tx_bytes;
    unsigned long int rx_bytes;
    unsigned long int tx_packets;
    unsigned long int rx_packets;
    unsigned long int tx_multicast_packets;
    unsigned long int rx_multicast_packets;
    unsigned long int tx_drops;
    unsigned long int rx_drops;
    unsigned long int tx_crc_errors;
    unsigned long int rx_crc_errors;
    unsigned long int tx_broadcast_packets;
    unsigned long int rx_broadcast_packets;
    unsigned long int tx_unicast_packets;
    unsigned long int rx_unicast_packets;
    unsigned long int rx_unknownproto_packets;
} ETH_PORT_COUNTERS;
#define ETH_PORT_SPEED_CFG_FILE     "/tmp/eth_port_speed_cfg"

int get_eth_port_speed(eth_port_speed_cfg_t *t);
void hal_phy_reset(void);
int set_eth_port_speed(int port, int speed);
void hal_phy_down(void);
void hal_phy_up(void);
int hal_eth_get_link_status(int port, eth_port_link_status *port_status);
int hal_eth_get_port_speed(int port);
int hal_eth_get_port_counter(int port, ETH_PORT_COUNTERS *counter);
int hal_eth_clear_port_counter(void);
int hal_eth_get_link_port(const unsigned char *mac);
int eth_port_id_mapping_to_mac_id(int port);
int hal_eth_enable_wire_speed(void);
#endif
