/*SH0
*******************************************************************************
**                                                                           **
**         Copyright (c) 2016 Quantenna Communications, Inc.                 **
**         All rights reserved.                                              **
**                                                                           **
*******************************************************************************
EH0*/

#ifndef _QWERPE_H_
#define _QWERPE_H_

#define RPE_IFNAMSIZ	19
#define MDID_LEN	2

#define RPE_API_V1	1
#define RPE_API_V2	2
#define RPE_API_V3	3

enum rpe_cmd_id {
	CMD_INIT = 1,
	CMD_DEINIT,
	CMD_GET_INTF_STATUS,
	CMD_GET_INTF_INFO,
	CMD_DEAUTH,
	CMD_STA_MAC_FILTER,
	CMD_GET_STA_STATS,
	CMD_BSS_TRANS_REQ,
	CMD_START_FAT_MONITORING,
	CMD_MONITOR_START,
	CMD_MONITOR_STOP,
	CMD_GET_NONASSOC_STATS,
	RPE_CMD_MAX,
};

typedef enum rpe_evt_id {
	EVT_INTF_STATUS = 1,
	EVT_INTF_INFO,
	EVT_PROBE_REQ,
	EVT_CONNECT_COMPLETE,
	EVT_DEAUTH,
	EVT_DISASSOC,
	EVT_STA_PHY_STATS,
	EVT_BSS_TRANS_STATUS,
	EVT_NONASSOC_STA_PHY_STATS,
	RPE_EVT_MAX,
} rpe_evt_id_t;

enum rpe_tlv_type {
	TLVTYPE_IFNAME = 500,
	TLVTYPE_BSSID_MDID = 501,
	TLVTYPE_CHANNEL_BAND = 502,
	TLVTYPE_STA_MAC = 503,
	TLVTYPE_RX_PHYRATE = 504,
	TLVTYPE_TX_PHYRATE = 505,
	TLVTYPE_TS_LAST_RX = 506,
	TLVTYPE_TS_LAST_TX = 507,
	TLVTYPE_AVERAGE_FAT = 508,
	TLVTYPE_INTERFACE_CAPABILITY = 509,
	TLVTYPE_RSSI = 510,
	TLVTYPE_SSID = 511,
	TLVTYPE_BEACON_INTERVAL = 518,
	TLVTYPE_HT_CAPABILITY = 45,
	TLVTYPE_HT_OPERATION = 61,
	TLVTYPE_VHT_CAPABILITY = 191,
	TLVTYPE_VHT_OPERATION = 192,
};

enum rpe_deauth_code {
	DEAUTH_CODE_INVALID = 0,
	DEAUTH_CODE_BSS_TRANS = 12,
	DEAUTH_CODE_LACK_BANDWIDTH = 33,
};

enum rpe_sta_cap {
	RPE_STA_CAP_40M = BIT(0),
	RPE_STA_CAP_160M = BIT(1),
	RPE_STA_CAP_160M_80P80 = BIT(2),
	RPE_STA_CAP_BSS_TRANS_SUPPORT = BIT(3),
	RPE_STA_CAP_VHT_SUPPORT = BIT(4),
	RPE_STA_CAP_MU_BEAMFORMER = BIT(5),
	RPE_STA_CAP_MU_BEAMFORMEE = BIT(6),
	RPE_STA_CAP_HT_SUPPORT = BIT(7),
};

enum rpe_drv_cap {
	RPE_DRV_CAP_BSS_TRANS = BIT(0),
	RPE_DRV_CAP_HT = BIT(1),
	RPE_DRV_CAP_VHT = BIT(2),
	RPE_DRV_CAP_SUPPORTS_MONITOR = BIT(3),
};

enum rpe_phy_mode {
	RPE_PHY_CAPAB_HT = BIT(0),
	RPE_PHY_CAPAB_VHT = BIT(1),
};

enum rpe_band {
	RPE_BAND_2G = 0,
	RPE_BAND_5G = 1,
	RPE_BAND_MAX = 2,
};

enum rpe_direction {
	RPE_DIR_SELF_GENERATED = 0,
	RPE_DIR_PEER_GENERATED = 1,
};

enum ACCESS_MODE {
	DENY_ACCESS = 1,
	ALLOW_ACCESS = 2,
};

enum rpe_phy_type {
	RPE_PHYTYPE_FHSS = 1,
	RPE_PHYTYPE_DSSS = 2,
	RPE_PHYTYPE_IRBASEBAND = 3,
	RPE_PHYTYPE_OFDM = 4,
	RPE_PHYTYPE_HRDSSS = 5,
	RPE_PHYTYPE_ERP = 6,
	RPE_PHYTYPE_HT = 7,
	RPE_PHYTYPE_DMG = 8,
	RPE_PHYTYPE_VHT = 9,
};

enum rpe_intf_status {
	RPE_INTF_STATUS_INVALID = 0,
	RPE_INTF_STATUS_DOWN = 1,
	RPE_INTF_STATUS_UP = 2,
	RPE_INTF_STATUS_DELETED = 3,
	RPE_INTF_STATUS_MAX = 4,
};

typedef enum qwerpe_state {
	QWERPE_STATE_OFF = 0,
	QWERPE_STATE_ON = 1,
} qwerpe_state_t;

struct rpemsg {
	uint16_t id;
	uint8_t coding;
	uint8_t version;
	uint8_t bssid[ETH_ALEN];
	uint16_t payload_len;
	uint8_t payload[0];
} __attribute__ ((packed));

/* start RPE Fixed format event message */
struct evt_intf_status {
	uint8_t ifname_size;
	char ifname[RPE_IFNAMSIZ];
	uint32_t status;
} __attribute__ ((packed));

struct evt_probe_req {
	uint8_t sta_mac[ETH_ALEN];
	uint16_t curr_band;
	int32_t rssi;
	uint16_t rx_ss;
	uint16_t max_phyrate;
	uint64_t tstamp;
	uint8_t channel;
	uint8_t capability;
	uint16_t cookie_len;
	uint8_t cookie[0];
} __attribute__ ((packed));

struct evt_connect_complete {
	uint8_t sta_mac[ETH_ALEN];
	uint16_t rx_ss;
	uint16_t max_phyrate;
	uint16_t curr_band;
	uint8_t channel;
	uint8_t capability;
	uint16_t cookie_len;
	uint8_t cookie[0];
} __attribute__ ((packed));

struct evt_deauth {
	uint8_t sta_mac[ETH_ALEN];
	uint16_t reason_code;
	uint8_t direction;
} __attribute__ ((packed));

struct evt_disassoc {
	uint8_t sta_mac[ETH_ALEN];
	uint16_t reason_code;
	uint8_t direction;
} __attribute__ ((packed));

struct evt_bss_trans_status {
	uint8_t sta_mac[ETH_ALEN];
	uint16_t status_code;
} __attribute__ ((packed));
/* start RPE Fixed format event message */

/* start RPE fixed format command message */
struct cmd_get_intf_status {
	uint8_t ifname_size;
	char ifname[RPE_IFNAMSIZ];
} __attribute__ ((packed));

struct cmd_get_intf_info {
	uint8_t ifname_size;
	char ifname[RPE_IFNAMSIZ];
	uint32_t specifier;
} __attribute__ ((packed));

struct cmd_deauth {
	uint8_t sta_mac[ETH_ALEN];
	uint16_t reasoncode;
} __attribute__ ((packed));

struct cmd_mac_filter {
	uint8_t sta_mac[ETH_ALEN];
	uint16_t permission;
} __attribute__ ((packed));

struct cmd_get_sta_stats {
	uint8_t sta_mac[ETH_ALEN];
} __attribute__ ((packed));

struct cmd_bss_trans_req {
	uint8_t sta_mac[ETH_ALEN];
	uint16_t timer;
	uint8_t mode;
	uint8_t validity;
	uint8_t bssid[ETH_ALEN];
	uint32_t bssid_info;
	uint8_t opclass;
	uint8_t channel;
	uint8_t phytype;
	uint8_t subel_len;
	uint8_t subels[0];
} __attribute__ ((packed));

struct cmd_start_fat_monitor {
	uint8_t ifname_size;
	char ifname[RPE_IFNAMSIZ];
	uint32_t fat_period;
} __attribute__ ((packed));

struct cmd_monitor_start {
	uint8_t ifname_size;
	char ifname[RPE_IFNAMSIZ];
	uint16_t period;
	uint16_t duty_cycle;
} __attribute__ ((packed));

struct cmd_monitor_stop {
	uint8_t ifname_size;
	char ifname[RPE_IFNAMSIZ];
} __attribute__ ((packed));

struct cmd_get_nonassoc_sta_stats {
	uint8_t sta_mac[ETH_ALEN];
} __attribute__ ((packed));
/* end RPE fixed format command message */

struct rpe_tlv {
	uint16_t type;
	uint16_t len;
	uint8_t value[0];
} __attribute__ ((packed));

#define QWERPE_LOG_ERR		0
#define QWERPE_LOG_WARN		1
#define QWERPE_LOG_NOTICE	2
#define QWERPE_LOG_INFO		3
#define QWERPE_LOG_DEBUG	4

extern int qwerpe_log_level;
#define QWRRPE_PREFIX "[QWERPE] "
#define QWERPE_DEBUG(fmt, args...) \
	do { if (qwerpe_log_level >= QWERPE_LOG_DEBUG)	printk(QWRRPE_PREFIX"[Debug] (%-16.16s:%04u)" fmt, __func__, __LINE__, ##args); } while (0)
#define QWERPE_INFO(fmt, args...) \
	do { if (qwerpe_log_level >= QWERPE_LOG_INFO)	printk(QWRRPE_PREFIX"[Info ] (%-16.16s:%04u)" fmt, __func__, __LINE__, ##args); } while (0)
#define QWERPE_NOTICE(fmt, args...) \
	do { if (qwerpe_log_level >= QWERPE_LOG_NOTICE)	printk(QWRRPE_PREFIX"[Notic] (%-16.16s:%04u)" fmt, __func__, __LINE__, ##args); } while (0)
#define QWERPE_WARN(fmt, args...) \
	do { if (qwerpe_log_level >= QWERPE_LOG_WARN)	printk(QWRRPE_PREFIX"[Warn ] (%-16.16s:%04u)" fmt, __func__, __LINE__, ##args); } while (0)
#define QWERPE_ERR(fmt, args...) \
	do { if (qwerpe_log_level >= QWERPE_LOG_ERR)	printk(QWRRPE_PREFIX"[Error] (%-16.16s:%04u)" fmt, __func__, __LINE__, ##args); } while (0)

struct ht_cap_ie {
	uint8_t	id;			/* Element ID */
	uint8_t	len;			/* Length */
	uint8_t	cap_info[2];		/* HT capabilities Info */
	uint8_t	ampdu_param;		/* A-MPDU Parameters */
	uint8_t	mcs_set[16];		/* Supported MCS Set */
	uint8_t	ext_cap[2];		/* HT Extended Capabilities */
	uint8_t	tx_bf_cap[4];		/* Transmit Beamforming Capabilities */
	uint8_t	asel_cap;		/* ASEL Capabilities */
} __attribute__ ((packed));

struct ht_op_ie {
	uint8_t	id;			/* Element ID */
	uint8_t	len;			/* Length*/
	uint8_t	pri_channel;		/* Primary Channel */
	uint8_t	op_info[5];		/* HT Operation Information */
	uint8_t	basic_mcs_set[16];	/* Basic MCS Set */
} __attribute__ ((packed));

struct vht_cap_ie {
	uint8_t	id;			/* Element ID */
	uint8_t	len;			/* Length */
	uint8_t	cap_info[4];		/* VHT capabilities info */
	uint8_t	mcs_nss_set[8];		/* supported MSC and NSS set */
} __attribute__ ((packed));

struct vht_op_ie {
	uint8_t	id;			/* element ID */
	uint8_t	len;			/* length in bytes */
	uint8_t	op_info[3];		/* VHT Operation Information */
	uint8_t	basic_mcs_nss_set[2];	/* Basic VHT MSC and NSS Set */
} __attribute__ ((packed));

struct ext_cap_ie {
	uint8_t	id;			/* Element ID */
	uint8_t	len;			/* Length*/
	uint8_t	cap[0];			/* Capabilities */
} __attribute__ ((packed));

#define HT_CAP_IE_ID	45
#define HT_OP_IE_ID	61
#define VHT_CAP_IE_ID	191
#define VHT_OP_IE_ID	192
#define EXT_CAP_IE_ID	127

#define HT_CAP_IE_LEN	26
#define HT_OP_IE_LEN	22
#define VHT_CAP_IE_LEN	12
#define VHT_OP_IE_LEN	5
#define EXT_CAP_IE_LEN	6 /* the actual len of Extended Capabilities may not be 6 */

struct qwerpe_intf_info {
	uint8_t		mdid[MDID_LEN];
	uint8_t		channel;
	uint8_t		band;
	uint8_t		op_class;
	uint8_t		phy_type;
	uint8_t		drv_cap;
	uint16_t	cap_info;
	uint16_t	beacon_interval;
};

struct qwerpe_sta_stats {
	uint8_t		sta_mac[ETH_ALEN];
	uint32_t	rx_phy_rate;
	uint32_t	tx_phy_rate;
	uint64_t	rx_tstamp;
	uint64_t	tx_tstamp;
	int32_t		rssi;
};

struct qwerpe_nonassoc_sta_stats {
	uint8_t		sta_mac[ETH_ALEN];
	uint64_t	rx_tstamp;
	int32_t		rssi;
};

#define MAX_STA_NUM_PER_BSS		128
#define MAX_NONASSOC_STA_NUM_PER_BSS	256

struct qwerpe_drv_ops {
	int (*get_intf_status)(void *drv_ctx);
	int (*get_intf_info)(void *drv_ctx, struct qwerpe_intf_info *info);
	int (*get_intf_ht_ie)(void *drv_ctx, struct ht_cap_ie *ht_cap, struct ht_op_ie *ht_op);
	int (*get_intf_vht_ie)(void *drv_ctx, struct vht_cap_ie *vht_cap, struct vht_op_ie *vht_op);
	int (*get_intf_fat)(void *drv_ctx);
	int (*deauth_sta)(void *drv_ctx, uint8_t *sta_mac, uint16_t reason_code);
	int (*get_sta_stats)(void *drv_ctx, uint8_t *sta_intst, struct qwerpe_sta_stats *sta_stats_list);
	int (*get_nonassoc_sta_stats)(void *drv_ctx, uint8_t *sta_intst, struct qwerpe_nonassoc_sta_stats *sta_stats_list);
	int (*start_fat_monitor)(void *drv_ctx, uint32_t period);
	int (*start_nonassoc_sta_monitor)(void *drv_ctx, uint16_t period, uint16_t duty_cycle);
	int (*stop_nonassoc_sta_monitor)(void *drv_ctx);
};

/* Functions provide to qwe adapter driver */
//void *qwerpe_register_bss(uint8_t *bssid, char *ifname, void *drv_ctx, struct qwerpe_drv_ops *drv_ops);
//void qwerpe_unregister_bss(uint8_t *bssid);
uint64_t qwerpe_get_timestamp(void);
int qwerpe_is_sta_denied(void *bss_ctx, uint8_t *mac);
int qwerpe_send_intf_status(void *bss_ctx, int status);
int qwerpe_send_intf_info(void *bss_ctx, uint32_t specifier);
int qwerpe_send_probe_req(void *bss_ctx, uint8_t *sta_mac, int32_t rssi,
				uint8_t *ht_cap_ie, uint8_t *vht_cap_ie, uint8_t *ext_cap_ie,
				uint8_t *cookie, uint16_t cookie_len);
int qwerpe_send_conn_complete(void *bss_ctx, uint8_t *sta_mac,
				uint8_t *ht_cap_ie, uint8_t *vht_cap_ie, uint8_t *ext_cap_ie,
				uint8_t *cookie, uint16_t cookie_len);
int qwerpe_send_deauth(void *bss_ctx, uint8_t *sta_mac, uint16_t reason_code, uint8_t direction);
int qwerpe_send_disassoc(void *bss_ctx, uint8_t *sta_mac, uint16_t reason_code, uint8_t direction);
void qwerpe_parse_cap_ie(uint8_t *frm, int len,
				uint8_t **ht_cap_ie, uint8_t **vht_cap_ie, uint8_t **ext_cap_ie);

#endif
