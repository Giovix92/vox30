#ifndef _GPON_API_H_
#define _GPON_API_H_

#include <netinet/in.h>
#include <sal/sal_wan.h>
#define UNI_NUM         4
#define IPHOST_UNI_NUM  2
#define MAX_RULES       5
#define HOST_PORT_ID    UNI_NUM

#define MAX_WAN_DEV_NUM 4
#define MAX_LAN_DEV_NUM 4
#define MAX_WAN_TRAP_FILTER_NUM 2
#define MAX_LAN_TRAP_FILTER_NUM 5

#define GPON_DEBUG_MODE_OMCI            ((int) 0x01)
#define GPON_DEBUG_MODE_LOG             ((int) 0x02)
#define GPON_DEBUG_MODE_FILE            ((int) 0x10)
#define GPON_DEBUG_MODE_CONSOLE         ((int) 0x20)

#define GPON_TRAFFIC_MGMT_PRIORITY_QUEUE                       (0)
/* Rate Control */
#define GPON_TRAFFIC_MGMT_RATE_CONTROL                         (1)
/* Weighted Round Robin */
#define GPON_TRAFFIC_MGMT_WRR                                  (2)
/* Hybrid Priority Queues & WRR */
#define GPON_TRAFFIC_MGMT_HYBRID_PRIORITY_AND_WRR              (3)
/* Hybrid Priority Queues & Rate control */
#define GPON_TRAFFIC_MGMT_HYBRID_PRIORITY_AND_RATE_CONTROLLER  (4)
/* Priority Queues & Rate control */
#define GPON_TRAFFIC_MGMT_PRIORITY_AND_RATE_CONTROLLER         (5)
/* Traffic Management Don't care */
#define GPON_TRAFFIC_MGMT_DONT_CARE                            (0xFFFF)

/* CPU RX queue 0 */
#define GPON_FLOW_DESTINATION_CPU_RX_QUEUE_0 ( 0 )
/* CPU RX queue 1 */
#define GPON_FLOW_DESTINATION_CPU_RX_QUEUE_1 ( 1 )
/* CPU RX queue 2 */
#define GPON_FLOW_DESTINATION_CPU_RX_QUEUE_2 ( 2 )
/* CPU RX queue 3 */
#define GPON_FLOW_DESTINATION_CPU_RX_QUEUE_3 ( 3 )
/* CPU RX queue 4 */
#define GPON_FLOW_DESTINATION_CPU_RX_QUEUE_4 ( 4 )
/* CPU RX queue 5 */
#define GPON_FLOW_DESTINATION_CPU_RX_QUEUE_5 ( 5 )
/* CPU RX queue 6 */
#define GPON_FLOW_DESTINATION_CPU_RX_QUEUE_6 ( 6 )
/* CPU RX queue 7 */
#define GPON_FLOW_DESTINATION_CPU_RX_QUEUE_7 ( 7 )
/* IPTV */
#define GPON_FLOW_DESTINATION_IPTV           ( 8 )
/* Bridge */
#define GPON_FLOW_DESTINATION_ETH            ( 9 )
/* None */
#define GPON_FLOW_DESTINATION_NONE           ( 0xFF )

#define MAX_TCONT_NUM 32

typedef enum
{
    CS_NOT_CONFIGURED = 0,
    CS_CONFIGURED = 1
}BL_CONFIGURATION_STATE_DTS;

typedef enum {
    GPON_RET_MIN,
    GPON_RET_SUCCESS = GPON_RET_MIN,
    GPON_RET_FAILURE,
    GPON_RET_INVALID_ARG,
} Gpon_Ret;

/* 
 * The macros should corresponding to the ones which defined in
 * "drivers/GPON_OMCI/PONWiz-4.8/ocssrc/uspace/user/boardcustom.h" 
 */

typedef enum 
{
    GAPI_PEER_OLT_AUTO  = 0,
    GAPI_PEER_OLT_HUAWEI   = 1,
    GAPI_PEER_OLT_ERICSSON = 2,
    GAPI_PEER_OLT_ZTE = 3,
    GAPI_PEER_OLT_MAX       
} eGapiOltType;

/* Bridge interface bit mask define */
/* Bridge port EMAC 0 */
#define  GPON_BRIDGE_MASK_LAN0          (1<<0)
/* Bridge port EMAC 1 */
#define  GPON_BRIDGE_MASK_LAN1          (1<<1)
/* Bridge port EMAC 2 */
#define  GPON_BRIDGE_MASK_LAN2          (1<<2)
/* Bridge port EMAC 3 */
#define  GPON_BRIDGE_MASK_LAN3          (1<<3)
/* Bridge port EMAC 4 */
#define  GPON_BRIDGE_MASK_LAN4          (1<<4)
/* Bridge port WAN */
#define  GPON_BRIDGE_MASK_WAN           (1<<5)
/* Bridge port WAN IPTV */
#define  GPON_BRIDGE_MASK_IPTV          (1<<6)
/* Bridge port WAN QUASI */
#define  GPON_BRIDGE_MASK_QUASI         (1<<7)
/* Bridge port WAN ROUTED */
#define  GPON_BRIDGE_MASK_ROUTED        (1<<8)
/* Bridge port CPU */
#define  GPON_BRIDGE_MASK_CPU           (1<<9)
/* Bridge port WIFI_0 */
#define  GPON_BRIDGE_MASK_WIFI_0        (1<<10)
/* Bridge port WIFI_1 */
#define  GPON_BRIDGE_MASK_WIFI_1        (1<<11)
/* Bridge port WIFI_2 */
#define  GPON_BRIDGE_MASK_WIFI_2        (1<<12)
/* Bridge port WIFI_3 */
#define  GPON_BRIDGE_MASK_WIFI_3        (1<<13)
/* Bridge port WIFI_4 */
#define  GPON_BRIDGE_MASK_WIFI_4        (1<<14)
/* Bridge port WIFI_5 */
#define  GPON_BRIDGE_MASK_WIFI_5        (1<<15)
/* Bridge port WIFI_6 */
#define  GPON_BRIDGE_MASK_WIFI_6        (1<<16)
/* Bridge port WIFI_7 */
#define  GPON_BRIDGE_MASK_WIFI_7        (1<<17)
/* Bridge Port ALL */
#define  GPON_BRIDGE_MASK_ALL             0xFFFFFFFF

#define GAPI_IN_FILTER_PBIT_VALID_MIN       0   /* filter on this valid pbit */
#define GAPI_IN_FILTER_PBIT_VALID_MAX       7   /* filter on this valid pbit */
#define GAPI_IN_FILTER_PBIT_NO_PBIT_FILTER  8   /* not filter on this pbit */
#define GAPI_IN_FILTER_PBIT_DEF_FILTER      14  /* this is the default filter when no other one-tag rule applies - 984.4 */
#define GAPI_IN_FILTER_PBIT_NO_TAG_FILTER   15  /* this entry is a no-tag rule, ignore all other filter fields. - 984.4 */
#define GAPI_IN_FILTER_VID_VALID_MIN        0   /* filter on this vid */
#define GAPI_IN_FILTER_VID_VALID_MAX        4094/* filter on this vid */
#define GAPI_IN_FILTER_VID_NA               4096/* not filter on this tag */

#define GAPI_OUT_FILTER_PBIT_VALID_MIN      0                                /* remark tag with this valid pbit */
#define GAPI_OUT_FILTER_PBIT_VALID_MAX      7                                /* remark tag with this valid pbit */
#define GAPI_OUT_FILTER_PBIT_NA             15                               /* pbit is not avaliable */
#define GAPI_OUT_FILTER_PBIT_NEED_CHECK     8                                /* tag pbit need check treatment */
#define GAPI_OUT_FILTER_VID_VALID_MIN       0                                /* remark tag with this valid VID */
#define GAPI_OUT_FILTER_VID_VALID_MAX       4094                             /* remark tag with this valid VID */
#define GAPI_OUT_FILTER_VID_NEED_CHECK      4096                             /* tag vid need check treatment */
#define GAPI_OUT_FILTER_PBIT_BASE           256
#define GAPI_OUT_FILTER_PBIT_UNCHANGE       (GAPI_OUT_FILTER_PBIT_BASE + 16) /* pbit keep unchanged */

#define GAPI_UNCONFIG_VID (-1) 
#define GAPI_UNTAGGED_VID GAPI_OUT_FILTER_VID_NEED_CHECK /* 4096 */ 
#define GAPI_DEFAULT_VID  GAPI_OUT_FILTER_VID_VALID_MAX  /* 4094 */
#define GAPI_PA_DONT_ADD  GAPI_OUT_FILTER_PBIT_NA        /* 15 */
#define GAPI_PA_DEFAULT   GAPI_OUT_FILTER_PBIT_VALID_MIN /* 0 */

struct iphost_data
{
    int ip_option;
    unsigned char mac[6];
    unsigned long int ip_addr;
    unsigned long int ip_mask;
    unsigned long int ip_gateway;
    unsigned long int pri_dns;
    unsigned long int sec_dns;
    char host_name[25];
    char domain_name[25];
};

struct vlan_info {
    int ethertype;              /* 0 for all protocols */
    int tag_num;                /* 0 for untagged, 1 for single tagged, 2 for double tagged. */
    int outer_vid;              /* 4096 mean that the packet is not double tagged. */
    int outer_pbit;             
    int outer_tpid;
    int inner_vid;              /* 4096 mean that the packet is untagged packet. */
    int inner_pbit;             
    int inner_tpid;
};

struct vlan_treatment{
    int tag_rem; /* The tag number to be removed */
    int outer_vid_action; /* 4096 for copy vid from inner VID, 4097 for copy vid from outer VID  */
    int outer_pbit_action;/* 8 for copy pbit from inner pbit, 9 for copy pbit from outer pbit 15 for Do not add an outer Tag */
    int outer_tpid_action;
    int inner_vid_action; /* 4096 for copy vid from inner VID, 4097 for copy vid from outer VID */
    int inner_pbit_action; /* 8 for copy pbit from inner pbit, 9 for copy pbit from outer pbit 15 for Do not add an inner Tag */
    int inner_tpid_action;
};
#define VLAN_ACTION_RULE_EQ(x,y) \
	((x)->enable == (y)->enable && \
	 (x)->in_filter.ethertype == (y)->in_filter.ethertype && \
	 (x)->in_filter.tag_num == (y)->in_filter.tag_num && \
	 (x)->in_filter.outer_vid == (y)->in_filter.outer_vid && \
	 (x)->in_filter.outer_pbit == (y)->in_filter.outer_pbit && \
	 (x)->in_filter.outer_tpid == (y)->in_filter.outer_tpid && \
	 (x)->in_filter.inner_vid == (y)->in_filter.inner_vid && \
	 (x)->in_filter.inner_pbit == (y)->in_filter.inner_pbit && \
	 (x)->in_filter.inner_tpid == (y)->in_filter.inner_tpid && \
	 (x)->out_filter.ethertype == (y)->out_filter.ethertype && \
	 (x)->out_filter.tag_num == (y)->out_filter.tag_num && \
	 (x)->out_filter.outer_vid == (y)->out_filter.outer_vid && \
	 (x)->out_filter.outer_pbit == (y)->out_filter.outer_pbit && \
	 (x)->out_filter.outer_tpid == (y)->out_filter.outer_tpid && \
	 (x)->out_filter.inner_vid == (y)->out_filter.inner_vid && \
	 (x)->out_filter.inner_pbit == (y)->out_filter.inner_pbit && \
	 (x)->out_filter.inner_tpid == (y)->out_filter.inner_tpid && \
	 (x)->action.tag_rem == (y)->action.tag_rem && \
	 (x)->action.outer_vid_action == (y)->action.outer_vid_action && \
	 (x)->action.outer_pbit_action == (y)->action.outer_pbit_action && \
	 (x)->action.outer_tpid_action == (y)->action.outer_tpid_action && \
	 (x)->action.inner_vid_action == (y)->action.inner_vid_action && \
	 (x)->action.inner_pbit_action == (y)->action.inner_pbit_action && \
	 (x)->action.inner_tpid_action == (y)->action.inner_tpid_action \
	)
struct vlan_action {
    int enable;
    struct vlan_info in_filter;
    struct vlan_info out_filter;
    struct vlan_treatment action;
};

struct igmp_tag_ctrl
{
    unsigned char igmpTagCtrl;
    unsigned char userPri;
    unsigned char cfi;
    unsigned short vlanId;
    int flowid;
};

#define DSCP_TO_PBIT_MAX 64
struct uni_config {
    int omci_bridge_id;
    int emac_id;
    struct vlan_action rules[MAX_RULES];
    unsigned char dscp_to_pbit_tbl[DSCP_TO_PBIT_MAX];
};

struct packet_info {
    int is_cpu_mac;
    int packet_type;            /* unicast multicast  broadcast */
    int trap_reason;
    int source_bridge_port;
    int source_flow;
    unsigned int dst_port;
    struct vlan_info packet_vlan;
    int reserve;
};

struct l2_rules 
{
	int enable;
	int match_num; /* this rules's match num*/
	unsigned int bridge_port; /* physic port */
	unsigned int l2_flood_map; /* Bit-MAP for flood packet. */
	struct vlan_info vlan_filter; /* incoming packet feature */
};

struct packet_trap_filter
{
	int enable;
	int match_num; /* this rules's match num*/
	struct vlan_info vlan_filter; /* Incoming packet filter */
};

/* Total Gem Counter */
typedef struct {
    unsigned long int port_id;
    unsigned long int tx_packets;
    unsigned long int rx_packets;
    unsigned long int rx_discard_packets;
    unsigned long long tx_bytes;
    unsigned long long rx_bytes;
    unsigned long int cpu_tx_packets;
    unsigned long int cpu_rx_packets;
    unsigned long long cpu_tx_bytes;
    unsigned long long cpu_rx_bytes;
    unsigned long int cpu_tx_failed_packets;
} PON_GEM_COUNTERS;

/* Total Tcont Counter */
typedef struct {
    unsigned int xi_tcont_id;
    unsigned long int transmitted_idle_counter;
    unsigned long int transmitted_gem_counter;
    unsigned long int transmitted_packet_counter;
    unsigned long int requested_dbr_counter;
    unsigned long int valid_access_counter;
} PON_TCONT_COUNTERS;

/* OMCI counter */
typedef struct {
    unsigned long int tx_packets;
    unsigned long int rx_packets;
    unsigned long long tx_blocks;
    unsigned long long rx_blocks;
} PON_OMCI_COUNTERS;

/* LAN counter */
typedef struct {
    unsigned int lan_port;
    unsigned long long tx_packets;
    unsigned long long rx_packets;
    unsigned long long tx_bytes;
    unsigned long long rx_bytes;
} LAN_COUNTERS;

typedef struct {
    unsigned long int tx_packets;
    unsigned long long tx_bytes;
    unsigned long int rx_packets;
    unsigned long long rx_bytes;
    unsigned long long tx_drops;
    unsigned long long rx_crc_errors;
    unsigned long long rx_drops;
} PON_COUNTERS;

typedef struct {
    unsigned int eth_port;
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
} ETH_COUNTERS;

/* ETH status */
typedef struct {
    unsigned int phy_port;
    unsigned int link_mode;
    unsigned int link_state;
} PON_ETH_STATUS;

typedef struct {
    int tcont_state;            /* Tcont configuration state */
    unsigned int  alloc_id;     /* Alloc ID */
    int traffic_mgmt;           /* Traffic Managment */
} PON_TCONT_ENTRY;

/* DS flow table Entry */
#define VAL_GEMPORT_DIRECTION_UP        0
#define VAL_GEMPORT_DIRECTION_DS        1
#define VAL_GEMPORT_DIRECTION_BOTH      2
typedef struct {
    int index;
    int state ;                 /* DS flow configuration state */
    int direction;              /*0:UP, 1:DOWN, 2:BOTH*/
    unsigned int port_id ;      /* GEM port ID */
    unsigned int type;          /* 4:OMCI, 8:IPTV, 9:ETHERNET */
    int tcont_id;
} PON_GEM_FLOW_TABLE_ENTRY;

#define TC_NAME_LENGTH    32
struct transceiver_info{
    char vendor[TC_NAME_LENGTH];
    int rf_support;
    int catv_state;
    int tx_power_state;
    int tx_sd;
};

typedef struct {
    int valid;
    int port_mask;
    unsigned char mac[6];
} IPTV_MAC_TABLE_ENTRY;

typedef struct {
#define US_TCONT_RC_NUM 8
#define US_TCONT_QUEUE_NUM 8
    int tcont_id;
    int gem_flow_id;
    int rate_ctl_id[US_TCONT_RC_NUM];
    int sir[US_TCONT_RC_NUM];
    int pir[US_TCONT_RC_NUM];
    int weight[US_TCONT_RC_NUM];
    int queue_id[US_TCONT_RC_NUM][US_TCONT_QUEUE_NUM];
} PON_US_TCONT_QOS_ENTRY;

/**
 * Get GPON API Version, format Major.Minor.GMP(Version).Extra
 *
 *
 * @return
 */
char *val_gpon_api_version (void);

/**
 * Trap router WAN pppoe ctrl and data packets to CPU
 *
 * @param wan_id lan/wan GMP defined subnet number
 * @param mac pppoe wan mac address
 *
 * @return
 */
int val_gpon_ppp_open (int wan_id, char *mac);

/**
 * Forward router WAN pppoe ctrl and data packets to Runner (no trap
 * to cpu)
 *
 * @param wan_id lan/wan GMP defined subnet number
 * @param mac pppoe wan mac address
 *
 * @return
 */
int val_gpon_ppp_close (int wan_id, char *mac);

/**
 * Trap bridge WAN and LAN packets to CPU, to prevent from bridge
 * pppoe connection working.
 *
 */
void val_gpon_lan_bridge_pppoe_enable (int lan_id);

/**
 * Do not trap the bridge WAN and LAN pppoe packets to CPU, let the bridge
 * pppoe connection working.
 *
 */
void val_gpon_lan_bridge_pppoe_disable (int lan_id);


/**
 * Change the GMP router interface attrubuite so that it can handle
 * pppoe sessions.
 *
 * 1. attach the session id to subnet wan
 * 2. bind the pppoe server ip and mac address
 * 3. add ip address to H/W NAT filter table
 *
 * This routine is called after pppoe get ip address, mostly from
 * pppoe ipup script.
 *
 * @param wan_id lan/wan GMP defined subnet number
 * @param session pppoe session id
 * @param mac pppoe wan mac
 * @param ip pppoe wan ip
 * @param gw_mac pppoe wan gateway mac
 * @param gw_ip pppoe wan gateway ip
 *
 * @return
 */
int val_gpon_pppoe_up (int wan_id, int session,
                       char *mac, char *ip,
                       char *gw_mac, char *gw_ip);


/**
 * Change the GMP router interface to IPoE.
 *
 * 1. delete session id
 * 2. remove pppoe server ip and mac from arp table
 *
 * @param wan_id lan/wan GMP defined subnet number
 * @param session pppoe session id
 * @param mac pppoe wan mac
 * @param ip pppoe wan ip
 * @param gw_ip pppoe wan gateway ip
 *
 * @return
 */
int val_gpon_pppoe_down (int wan_id, int session,
                         char *mac, char *ip,
                         char *gw_ip);

/**
 * Prepare GMP router interface for uplayer IPoE WAN.
 *
 * 1. add H/W NAT support for the IPoE interface.
 *
 * This routine is called after ipoe get ip address, mostly from dhcp
 * ipup script.
 *
 * @param wan_id lan/wan GMP defined subnet number
 * @param mac IPoE wan mac
 * @param ip IPoE wan ip address
 *
 * @return
 */
int val_gpon_ipoe_up (int wan_id, char *mac, char *ip);


/**
 *
 *
 * @param wan_id  lan/wan GMP defined subnet number
 * @param mac IPoE wan mac
 * @param ip IPoE wan ip address
 *
 * @return
 */
int val_gpon_ipoe_down (int wan_id, char *mac, char *ip);


/**
 * Clean linux kernel contrack sessions based on the parameter so that
 * H/W NAT can work properly.
 *
 * This routine should be called when a interface ip address changes.
 *
 *
 * @param ip the interface ip address
 *
 * @return
 */
int val_gpon_cleanup_ct(char *ip);


/**
 * Configure MAC Address let Runner trap upstream packet to CPU
 *
 * This routine should be called after lan assigned with a ip address.
 *
 * @param lan_id lan/wan GMP defined subnet number
 * @param mac LAN mac
 *
 * @return
 */
int val_gpon_lan_ip_up(int lan_id, char *mac, char *ip);


/**
 * NULL
 *
 * @param lan_id lan/wan GMP defined subnet number
 * @param mac LAN mac
 *
 * @return
 */
int val_gpon_lan_ip_down(int lan_id, char *mac);


/**
 * Init the GMP driver/stack and OMCI application.
 *
 * @param sn GPON serial number
 * @param passwd GPON password
 *
 * @return
 */
int val_gpon_init(char *sn, char *passwd,  int max_wan_num, int (*get_wan_mac_callback)(int wan, char *mac), int default_state, char *lan_if, int ipv6_mode, int gpon_mode);

/**
 * GMP/OMCI can't do deinit by now.
 *
 * @return
 */
int val_gpon_deinit(void);

/**
 * Configure packet filter.
 *
 * @param type The type of the trap
 * @param enable Enable/Disable the trap
 *
 * @return
 */
int val_gpon_config_packet_trap(int type, int enable);

/**
 * Dump all filter configuration.
 *
 *
 * @return
 */
int val_gpon_trap_filter_dump(void *data);


/**
 * Initialize the gpon debug abilities.
 *
 * @param mode The bit-map for debug mode
 *
 * @return
 */
int val_gpon_set_debug_mode(int mode);

/**
 * Get the debug mode
 *
 *
 * @return The bit-map for debug mode
 */
int val_gpon_get_debug_mode(void);


/**
 * Get the MAX tcont number
 *
 *
 * @return
 */
int val_gpon_get_max_tcont_number(void);

/**
 * Get the MAX of gem port number
 *
 *
 * @return
 */
int val_gpon_get_max_gem_port_number(void);

/**
 * Get the counter for gem port
 *
 * @param gem_port Gem port number
 * @param gem_counters Gem port tx/rx counter
 *
 * @return
 */
int val_gpon_get_gem_counters(int gem_port, PON_GEM_COUNTERS *gem_counters);


/**
 * Get the upstream gem port table
 *
 * @param buflen the length of the input buffer 
 * @param gem_port_entry
 *
 * @return
 */
int val_gpon_dump_up_gem_port_tables(int buflen, PON_GEM_FLOW_TABLE_ENTRY * gem_port_entry);

/**
 * Get the downstream gem port table
 *
 * @param buflen the length of the input buffer 
 * @param gem_port_entry
 *
 * @return
 */
int val_gpon_dump_ds_gem_port_tables(int buflen, PON_GEM_FLOW_TABLE_ENTRY * gem_port_entry);

/**
 * Get the gem port table
 *
 * @param buflen the length of the input buffer 
 * @param gem_port_entry
 *
 * @return
 */
int val_gpon_dump_gem_port_tables(int buflen, PON_GEM_FLOW_TABLE_ENTRY * gem_port_entry);

/**
 * Get the ethernet port status
 *
 * @param phy_port
 * @param eth_status
 *
 * @return
 */
int val_gpon_dump_eth_port_status(int phy_port, PON_ETH_STATUS * eth_status);

/**
 * Get OMCI rx/tx packet counter
 *
 * @param omci_counter
 *
 * @return
 */
int val_gpon_get_omci_counters(PON_OMCI_COUNTERS *omci_counter);

/**
 * Get Lan rx/tx packet counter
 *
 * @param lan_port
 * @param lan_counter
 *
 * @return
 */
int val_gpon_get_lan_counters(int lan_port, LAN_COUNTERS *lan_counter);

/**
 * Get Ethernet rx/tx packet counter
 *
 * @param eth_port
 * @param eth_counter
 *
 * @return
 */
int val_gpon_get_eth_counters(int eth_port, ETH_COUNTERS *eth_counter);

/**
 * Clear Ethernet rx/tx packet counter
 *
 *
 * @return
 */
int val_gpon_clear_eth_counters(void);

/**
 * Pon total tx/rx packet counter
 *
 * @param pon_counter
 *
 * @return
 */
int val_gpon_get_total_counters(PON_COUNTERS *pon_counter);

/**
 * Clear pon total tx/rx packet counter
 *
 *
 * @return
 */
int val_gpon_clear_pon_counters(void);

/**
 * Dump tcont table
 *
 * @param buflen the length of the input buffer
 * @param tcont_entry
 *
 * @return
 */
int val_gpon_dump_tcont_tables(int buflen, PON_TCONT_ENTRY *tcont_entry);
int val_gpon_register_ok(void);

/**
 * Dump tcont counter
 *
 * @param tcont_id
 * @param tcont_counter
 *
 * @return
 */
#ifdef GPON_PASSWORD_EXTRA_CHECK
int val_gpon_connected(void);
#endif
int val_gpon_get_tcont_counters(int tcont_id, PON_TCONT_COUNTERS *tcont_counter);

/**
 * Get current GPON link status (O0 ~ O7)
 * O0 for no fiber
 *
 * @return
 */
int val_gpon_get_link_status(void);

/**
 * Check whether there is traffic(tx and rx) on PON interface
 *
 *
 * @return
 */
int val_gpon_has_traffic(void);

/**
 * Get pon tx power
 *
 *
 * @return
 */
int val_gpon_get_tx_power(double *tx_power);

/**
 * Get pon rx power
 *
 *
 * @return
 */
int val_gpon_get_rx_power(double *rx_power);
/**
 * Get catv tx power
 *
 *
 * @return
 */
int val_gpon_get_catv_tx_power(double *tx_power);

/**
 * Get pon rx power
 *
 *
 * @return
 */
int val_gpon_get_catv_rx_power(double *rx_power);
int val_gpon_set_catv_enable(int enable);

void val_gpon_set_catv_filter(char *value);

/**
 * Check whether there is traffic in H/W NAT.
 *
 * @param wan_id lan/wan GMP defined subnet number
 *
 * @return the HW NAT tx counter
 */
int val_gpon_get_hw_acc_tx_counter(int wan_id, int vid, int pri);


/**
 * Add rx filter for WAN interface.
 *
 * @param wan_id lan/wan GMP defined subnet number
 * @param filter Wan rx filter
 *
 * @return
 */
int val_gpon_add_wan_rx_filter(int wan_id, struct packet_trap_filter *filter);

/** 
 * Delete the wan interface rx filter by rule index.
 *
 * @param wan_id lan/wan GMP defined subnet number
 * @param rule_index lan/wan rx fiter rule index
 * 
 * @return
 */
int val_gpon_del_wan_rx_filter_by_index(int wan_id, int rule_index);

/** 
 * Delete the wan interface rx filter.
 *
 * @param wan_id lan/wan GMP defined subnet number
 * @param filter lan/wan rx fiter
 * 
 * @return
 */
int val_gpon_del_wan_rx_filter(int wan_id, struct packet_trap_filter *filter);

/** 
 * Flush the wan interface rx filter.
 *
 * @param wan_id lan/wan GMP defined subnet number
 * 
 * @return
 */
int val_gpon_flush_wan_rx_filter(int wan_id);

/**************************************************************************
**  Function Name: val_gpon_get_wan_rx_filter
**  Purpose:
**    Get wan packet trap filter
**  Parameters:
**    wan_id - wan interface index
**    rule_index - wan rx trap filter rule index
**    filter - wan cpu packet trap filter
**  Return:
**  Notes:
**
**************************************************************************/
int val_gpon_get_wan_rx_filter(int wan_id, int rule_index,  struct packet_trap_filter *filter);

/**************************************************************************
**  Function Name: val_gpon_set_wan_rx_filter
**  Purpose:
**    Enable/disable wan packet trap filter
**  Parameters:
**    wan_id - wan interface index
**    rule_index - wan rx trap filter rule index
**    enable - Set wan rx trap filter enable/disable.
**  Return:
**  Notes:
**
**************************************************************************/
int val_gpon_set_wan_rx_filter(int wan_id, int rule_index, int enable);

/** 
 * Add the UNI port interface rx filter.
 *
 * @param lan_id The UNI port number
 * @param filter The UNI port rx filter
 * 
 * @return
 */
int val_gpon_add_lan_rx_filter(int lan_id, struct packet_trap_filter *filter);

/** 
 * Delete rx filter for UNI port by index.
 *
 * @param uni_port The UNI port number
 * @param rule_index The UNI port rx filter index
 * 
 * @return
 */
int val_gpon_del_lan_rx_filter_by_index(int lan_id, int rule_index);

/** 
 * Delete rx filter for UNI port.
 *
 * @param lan_id The UNI port number
 * @param filter The UNI port rx filter
 * 
 * @return
 */
int val_gpon_del_lan_rx_filter(int lan_id, struct packet_trap_filter *filter);

/** 
 * Flush rx filter for UNI port.
 *
 * @param lan_id The UNI port number
 * 
 * @return
 */
int val_gpon_flush_lan_rx_filter(int lan_id);

/**************************************************************************
**  Function Name: val_gpon_get_lan_rx_filter
**  Purpose:
**    Get lan interface rx cpu trap filter
**  Parameters:
**    lan_id - lan ether interface index
**    rule_index - lan rx trap filter rule index
**    filter - lan cpu packet trap filter
**  Return:
**  Notes:
**    
**************************************************************************/
int val_gpon_get_lan_rx_filter(int lan_id, int rule_index, struct packet_trap_filter *filter);

/**************************************************************************
**  Function Name: val_gpon_set_lan_rx_filter
**  Purpose:
**    Enable/disable lan packet trap filter
**  Parameters:
**    lan_id - lan ether interface index
**    rule_index - lan rx trap filter rule index
**    enable - Set lan rx trap filter enable/disable.
**  Return:
**  Notes:
**    
**************************************************************************/
int val_gpon_set_lan_rx_filter(int lan_id, int rule_index, int enable);

/** 
 * Add l2 rules for  WAN interface.
 *
 * @param rules Wan l2 rules
 * 
 * @return
 */
int val_gpon_add_wan_l2_rules(struct l2_rules  *rules);

/** 
 * Remove the wan interface l2 rules.
 *
 * @param rules wan l2 rules
 * 
 * @return
 */
int val_gpon_del_wan_l2_rules(struct l2_rules  *rules);

/** 
 * flush the wan interface l2 rules.
 *
 * @param
 * 
 * @return
 */
int val_gpon_flush_wan_l2_rules();

/** 
 * Add the UNI port l2 rules.
 *
 * @param uni_port The UNI port number
 * @param rules  lan l2 rules
 * 
 * @return The filter index
 */
int val_gpon_add_lan_l2_rules(int uni_port, struct l2_rules *rules);

/** 
 * Remove l2 rules for UNI port.
 *
 * @param uni_port The UNI port number
 * @param rules The UNI l2 rules
 * 
 * @return The filter index
 */
int val_gpon_del_lan_l2_rules(int uni_port, struct l2_rules *rules);

/** 
 * Remove all l2 rules for UNI port.
 *
 * @param uni_port The UNI port number
 * 
 * @return The filter index
 */
int val_gpon_flush_lan_l2_rules(int uni_port);

/**
 * Callback function, when OLT config the UNI, omci will call
 * this function to transfer UNI config.
 *
 * @param config The UNI configuration
 *
 * @return
 */
typedef int (*UNI_UPDATE_HOOK)(struct uni_config * , struct uni_config *);
int val_gpon_update_uni_config(struct uni_config *config);
int val_gpon_get_uni_config(int portid, struct uni_config *config);
int val_gpon_dump_uni_config(void);
void val_gpon_set_uni_update_hook(UNI_UPDATE_HOOK hook);
/** 
 * get uni host port's config.
 *
 * @return uni_config pointer
 */
int val_gpon_get_host_port_uni_config(struct uni_config *config);
/** 
 * get uni host port's config.
 *
 * @return uni_config id
 */
int val_gpon_get_host_port_id(void);

int val_gpon_hw_nat_disable(int wan_id);
int val_gpon_hw_nat_enable(int wan_id, struct in_addr *ip);
 
int val_gpon_get_rf_state(int *state);
int val_gpon_get_transceiver_info(struct transceiver_info *info);

int val_gpon_set_rogueont(int state);
int val_gpon_set_rx_threshold(int threshold);

/* IPHOST API */
#define SC_OMCI_SOCK_PATH "/tmp/sc_omci_sock_path"
#define handle_error(msg) \
    do { close(fd); unlink(SC_OMCI_SOCK_PATH); perror(msg); return; } while (0)

/* RF signal temporary file */
#define RF_SIGNAL "/tmp/RF"

typedef enum {
    SC_MIDDLE_LAYER_CMD_MIN = 0,
    SC_IPHOST_GET_NUM,
    SC_IPHOST_GET_VLAN_INFO,
    SC_IPHOST_GET_IP_INFO,
    SC_IPHOST_SET_IP_INFO,
    SC_VOIP_SET_STATUS_INFO,
    SC_RF_SET_OPER_ENABLE,
    SC_RF_SET_OPER_DISABLE,
    SC_MIDDLE_LAYER_CMD_MAX
}SC_MIDDLE_LAYER_CMD;

/**
 * get iphost number from OMCI
 *
 */
int val_gpon_get_iphost_num(void);

/**
 * get iphost vlan config from OMCI
 *
 */
int val_gpon_get_iphost_vlan_config(struct uni_config *config);

/**
 * get iphost ip config from OMCI
 *
 */
int val_gpon_get_iphost_ip_config(struct iphost_data *data);

/**
 * set iphost ip config to OMCI
 *
 */
int val_gpon_set_iphost_ip_config(void);

/**
 * set iphost voip status to OMCI
 *
 */
int val_gpon_set_voip_status(void);

/**
 * set RF signal to OMCI
 *
 */
int val_gpon_set_rf_signal(int cmd);

typedef enum {
    LAN_MODE_UNCONFIGURED = 0,
    LAN_MODE_NAT,
    LAN_MODE_BRIDGE,
    LAN_MODE_MIX,
}SC_LAN_MODE;
int val_gpon_set_lan_mode(int lan_id, SC_LAN_MODE mode);
int val_gpon_update_ds_igmp_tag_ctrl(int port, struct igmp_tag_ctrl *ctrl);
int val_gpon_dump_ds_igmp_tag_ctrl(int port, struct igmp_tag_ctrl *ctrl);
/*
    bit31 ... bit3 bit2 bit1 bit0
              lan3 lan2 lan1 lan0
 */
int val_gpon_set_lan_group(int lan_port_map);
int val_gpon_get_wan_counters(int wan_id, int vid, int pri, WAN_STATISTICS_INFO *info);

int bl_gpon_get_rx_power(double *rx_power);
int bl_gpon_get_catv_tx_power(double *tx_power);

/**
 * Get IPTV MAC Table.
 *
 * @param 
 *     header: will malloc buffer and return
 *
 * @return
 *     -1: error
 *     0:  No entry
 *     >0: Entry count. After using, please remeber free the header.
 *
 */
int val_get_iptv_mac_table(IPTV_MAC_TABLE_ENTRY **header);

int val_set_us_tcont_qos_cfg_from_vlan(int vlan_id, PON_US_TCONT_QOS_ENTRY *ut);
int val_gpon_set_lan_vid(int lan, int vid, int pbit);
#endif /* _GPON_API_H_ */

