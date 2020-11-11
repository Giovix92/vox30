/*SH0
*******************************************************************************
**                                                                           **
**         Copyright (c) 2016 Quantenna Communications, Inc.                 **
**         All rights reserved.                                              **
**                                                                           **
*******************************************************************************
EH0*/

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <net/sock.h>
#include <net/netlink.h>
#include "qwerpe.h"

#define QWERPE_PROC_NAME "qwerpe"

#define EXT_CAP_BSS_TRANS_BIT		19
#define HT_CAP_40M_CH_BIT		1
#define HT_CAP_20M_SHORT_GI     5
#define HT_CAP_40M_SHORT_GI     6
#define VHT_CAP_160M_CH_BIT		2
#define VHT_CAP_160M_80P80_CH_BIT	3
#define VHT_CAP_MU_BEAMFORMER_BIT	19
#define VHT_CAP_MU_BEAMFORMEE_BIT	20

#define IS_SET_BIT(u8_array, i)                  ((u8_array)[(i) / 8] & (1 << ((i) % 8)))

/* RPE msg may be forward to NPU via raw socket, the lenth should not be too long */
#define MAX_RPEMSG_LEN		1200
#define RPEMSG_LEN(len)		(sizeof(struct rpemsg) + (len))

/* ieee80211_wlan_xxx functions are defined in ieee80211_input.c (wlan.ko) */
static uint32_t ieee80211_wlan_ht_rx_maxrate(void *ht_cap, uint32_t *rx_ss);
//extern uint32_t ieee80211_wlan_vht_rxstreams(void *vht_cap);
//extern uint32_t ieee80211_wlan_vht_rx_maxrate(void *vht_cap);
void* (*qwerpe_register_bss_hook)(uint8_t *bssid, char *ifname, void *drv_ctx, struct qwerpe_drv_ops *drv_ops) = NULL;
void (*qwerpe_unregister_bss_hook)(uint8_t *bssid) = NULL;
EXPORT_SYMBOL(qwerpe_register_bss_hook);
EXPORT_SYMBOL(qwerpe_unregister_bss_hook);

struct mac_node {
	struct list_head lh;
	uint8_t mac[ETH_ALEN];
};

struct mac_list {
	struct list_head head;
	spinlock_t lock;
	void (*destroy)(const void *ctx);
};
#define LIST_LOCK(list) spin_lock(&(list)->lock)
#define LIST_UNLOCK(list) spin_unlock(&(list)->lock);

struct qwerpe_acl_ctx {
	struct mac_node node;
};

struct qwerpe_bss_ctx {
	struct mac_node node;
	char ifname[IFNAMSIZ];
	void *drv_ctx;
	struct qwerpe_drv_ops *drv_ops;
	struct mac_list acl_list;
};

struct qwerpe_ctx {
	qwerpe_state_t state;
	struct mac_list bss_list;
	struct sk_buff_head nlmsg_queue;
	struct work_struct nlmsg_send_wq;
};

struct rpecmd {
	const char *name;
	int payload_len;
	int (*handle)(struct rpemsg *msg);
#define UNFIXED_PAYLOAD_LEN -1
};

struct qwerpe_nlmsg {
	struct sk_buff *skb;
	struct rpemsg *rpemsg;
};

static struct qwerpe_ctx *qwerpe_ctx = NULL;
int qwerpe_log_level = QWERPE_LOG_ERR;
EXPORT_SYMBOL(qwerpe_log_level);
const uint32_t MCS_DATA_RATEStr[2][2][24] =
{
	{{6, 13, 19, 26, 39, 52, 58, 65, 
	  13, 26, 39 ,52, 78, 104, 117, 130, 
	  19, 39, 58, 78 ,117, 156, 175, 195},		// Long GI, 20MHz
	  
	 {7, 14, 21, 28, 43, 57, 65, 72, 
	  14, 28, 43, 57, 86, 115, 130, 144, 
	  21, 43, 65, 86, 130, 173, 195, 216}	},	// Short GI, 20MHz
	  
	{{13, 27, 40, 54, 81, 108, 121, 135, 
	  27, 54, 81, 108, 162, 216, 243, 270, 
	  40, 81, 121, 162, 243, 324, 364, 405},		// Long GI, 40MHz
	  
	 {15, 30, 45, 60, 90, 120, 135, 150, 
	  30, 60, 90, 120, 180, 240, 270, 300, 
	  45, 90, 135, 180, 270, 360, 405, 450}	}			// Short GI, 40MHz
};

static uint32_t ieee80211_wlan_ht_rx_maxrate(void *ht_cap_ie, uint32_t *rx_ss)
{
	struct ht_cap_ie *ht_cap = (struct ht_cap_ie *)ht_cap_ie;
    int is_40M = 0;
    int mcs = 0;
    int shortgi = 0;
    int index;

	if (IS_SET_BIT(ht_cap->cap_info, HT_CAP_40M_CH_BIT))
    {
        is_40M = 1;
	    if (IS_SET_BIT(ht_cap->cap_info, HT_CAP_40M_SHORT_GI))
            shortgi = 1;
    }
    else
    {
	    if (IS_SET_BIT(ht_cap->cap_info, HT_CAP_20M_SHORT_GI))
            shortgi = 1;
    }
    for (index=0; index<24; index++)
    {
        if (IS_SET_BIT(ht_cap->mcs_set, index))
            mcs = index;
    }
    if (mcs & 0xf0)
        *rx_ss = 3;
    else if (mcs & 0x8)
        *rx_ss = 2;
    else
        *rx_ss = 1;
    return MCS_DATA_RATEStr[is_40M][shortgi][mcs];
}


static inline uint16_t tlv_vlen(uint16_t type)
{
	switch (type) {
	case TLVTYPE_IFNAME:
	case TLVTYPE_SSID:
		return 0;	/* variable */
		break;
	case TLVTYPE_BSSID_MDID:
	case TLVTYPE_STA_MAC:
	case TLVTYPE_TS_LAST_RX:
	case TLVTYPE_TS_LAST_TX:
	case TLVTYPE_VHT_OPERATION:
		return 8;
		break;
	case TLVTYPE_CHANNEL_BAND:
	case TLVTYPE_RX_PHYRATE:
	case TLVTYPE_TX_PHYRATE:
	case TLVTYPE_AVERAGE_FAT:
	case TLVTYPE_INTERFACE_CAPABILITY:
	case TLVTYPE_RSSI:
	case TLVTYPE_BEACON_INTERVAL:
		return 4;
		break;
	case TLVTYPE_HT_CAPABILITY:
		return 28;
		break;
	case TLVTYPE_HT_OPERATION:
		return 24;
		break;
	case TLVTYPE_VHT_CAPABILITY:
		return 16;
		break;
	default:
		panic("rpe: Cannot calculate the length for type %d\n", type);
		break;
	}
	return -1;
}
#define tlv_tlen(type) (tlv_vlen(type) + sizeof(struct rpe_tlv))

#define RPEMSG_ALIGNTO   4
#define RPEMSG_ALIGN(len) (((len) + RPEMSG_ALIGNTO - 1) & ~(RPEMSG_ALIGNTO - 1))

static inline int encap_tlv(uint8_t * start, uint16_t type, void *value, uint16_t value_len)
{
	struct rpe_tlv *tlv = (struct rpe_tlv *)start;
	uint16_t len = tlv_vlen(type);

	tlv->type = cpu_to_le16(type);
	tlv->len = cpu_to_le16(len);
	memcpy(tlv->value, value, value_len);

	return len + sizeof(struct rpe_tlv);
}

static inline int encap_var_tlv(uint8_t * start, uint16_t type, void *value, uint16_t value_len)
{
	struct rpe_tlv *tlv = (struct rpe_tlv *)start;
	uint16_t len = RPEMSG_ALIGN(value_len);

	tlv->type = cpu_to_le16(type);
	tlv->len = cpu_to_le16(len);
	memcpy(tlv->value, value, value_len);

	return len + sizeof(struct rpe_tlv);
}

static inline void qwerpe_init_msg_hdr(struct rpemsg *msg, uint16_t id, uint8_t version,
					uint8_t *bssid, uint16_t payload_len)
{
	msg->id = cpu_to_le16(id);
	msg->version = version;
	memcpy(msg->bssid, bssid, ETH_ALEN);
	msg->payload_len = cpu_to_le16(payload_len);
	/* msg->coding = LITTLE_ENDIAN */
}

static inline int qwerpe_get_state(void)
{
	return qwerpe_ctx->state;
}

static const char *log_level_to_str(int log_level)
{
	static const char *names[] = {
		"error",
		"warn",
		"notice",
		"info",
		"debug",
	};
	int max = sizeof(names) / sizeof(names[0]);

	if (log_level >= max || log_level < 0) {
		return "unknown";
	}

	return names[log_level];
}

static const char *evt_to_str(uint16_t id)
{
	static const char *names[] = {
		"EVT_UNKNOWN",
		"EVT_INTF_STATUS",
		"EVT_INTF_INFO",
		"EVT_PROBE_REQ",
		"EVT_CONNECT_COMPLETE",
		"EVT_DEAUTH",
		"EVT_DISASSOC",
		"EVT_STA_PHY_STATS",
		"EVT_BSS_TRANS_STATUS",
		"EVT_NONASSOC_STA_PHY_STATS",
	};

	if (id >= RPE_EVT_MAX)
		id = 0;

	return names[id];
}

static void mac_list_clear(struct mac_list *list)
{
	struct mac_node *node, *node_tmp;

	LIST_LOCK(list);
	list_for_each_entry_safe(node, node_tmp, &list->head, lh) {
		list_del(&node->lh);
		list->destroy(node);
	}
	LIST_UNLOCK(list);
}

static void *mac_list_find(struct mac_list *list, uint8_t *mac)
{
	struct mac_node *node;

	LIST_LOCK(list);
	list_for_each_entry(node, &list->head, lh) {
		if (memcmp(mac, node->mac, ETH_ALEN) == 0) {
			LIST_UNLOCK(list);
			return node;
		}
	}
	LIST_UNLOCK(list);

	return NULL;
}

static int mac_list_del(struct mac_list *list, uint8_t *mac)
{
	struct mac_node *node;

	LIST_LOCK(list);
	list_for_each_entry(node, &list->head, lh) {
		if (memcmp(mac, node->mac, ETH_ALEN) == 0) {
			list_del(&node->lh);
			list->destroy(node);
			LIST_UNLOCK(list);
			return 1;
		}
	}
	LIST_UNLOCK(list);

	return 0;
}

static void mac_list_add(struct mac_list *list, struct mac_node *node)
{
	LIST_LOCK(list);
	list_add(&node->lh, &list->head);
	LIST_UNLOCK(list);
}

static struct qwerpe_nlmsg *qwerpe_alloc_nlmsg(struct qwerpe_nlmsg *nlmsg, int len)
{
	struct sk_buff *skb;

	skb = nlmsg_new(len, GFP_KERNEL);
	if (!skb)
		return NULL;

	nlmsg->skb = skb;
	nlmsg->rpemsg = nlmsg_data((struct nlmsghdr *)skb_tail_pointer(skb));
	memset(nlmsg->rpemsg, 0, len);

	return nlmsg;
}

static void qwerpe_send_nlmsg(struct qwerpe_nlmsg *nlmsg)
{
	int rpemsg_len = RPEMSG_LEN(le16_to_cpu(nlmsg->rpemsg->payload_len));

	if (!nlmsg_put(nlmsg->skb, 0, 0, RTM_RPEEVENT, rpemsg_len, 0)) {
		QWERPE_ERR("rpemsg over size\n");
		nlmsg_free(nlmsg->skb);
	}

	QWERPE_DEBUG("qwerpe enqueue event %s\n", evt_to_str(le16_to_cpu(nlmsg->rpemsg->id)));

	skb_queue_tail(&qwerpe_ctx->nlmsg_queue, nlmsg->skb);
	schedule_work(&qwerpe_ctx->nlmsg_send_wq);
}

/* The nlmsg should not be freed if it is sent by qwerpe_send_nlmsg */
static void qwerpe_free_nlmsg(struct qwerpe_nlmsg *nlmsg)
{
	nlmsg_free(nlmsg->skb);
}

static void evt_send_work(struct work_struct *work)
{
	struct sk_buff *skb;

	while (1) {
		skb = skb_dequeue(&qwerpe_ctx->nlmsg_queue);
		if(!skb)
			break;
		rtnl_notify(skb, &init_net, 0, RTNLGRP_NOTIFY, NULL, GFP_KERNEL);
	}}

static int qwerpe_send_sta_stats(void *bss_ctx, uint8_t *sta_mac)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;
	struct qwerpe_sta_stats *sta_list, *sta;
	struct qwerpe_nlmsg __nlmsg, *nlmsg;
	int i, sta_num = 1;
	uint8_t *pos;
	uint16_t payload_len;
	uint32_t le32;
	uint64_t le64;

	if (is_broadcast_ether_addr(sta_mac))
		sta_num = MAX_STA_NUM_PER_BSS;

	sta_list = kzalloc(sizeof(struct qwerpe_sta_stats) * sta_num, GFP_KERNEL);
	if (!sta_list) {
		QWERPE_WARN("Failed to alloc memory\n");
		return -1;
	}

	sta_num = bss->drv_ops->get_sta_stats(bss->drv_ctx, sta_mac, sta_list);
	if (sta_num == 0) {
		kfree(sta_list);
		return -1;
	}

	nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, MAX_RPEMSG_LEN);
	if (!nlmsg) {
		QWERPE_WARN("Failed to alloc nlmsg\n");
		kfree(sta_list);
		return -1;
	}
	pos = nlmsg->rpemsg->payload;

	for (i = 1, sta = sta_list; i <= sta_num; ++i, ++sta) {
		pos += encap_tlv(pos, TLVTYPE_STA_MAC, sta->sta_mac, ETH_ALEN);
		le32 = cpu_to_le32(sta->rx_phy_rate);
		pos += encap_tlv(pos, TLVTYPE_RX_PHYRATE, &le32, sizeof(le32));
		le32 = cpu_to_le32(sta->tx_phy_rate);
		pos += encap_tlv(pos, TLVTYPE_TX_PHYRATE, &le32, sizeof(le32));
		le64 = cpu_to_le64(sta->rx_tstamp);
		pos += encap_tlv(pos, TLVTYPE_TS_LAST_RX, &le64, sizeof(le64));
		le64 = cpu_to_le64(sta->tx_tstamp);
		pos += encap_tlv(pos, TLVTYPE_TS_LAST_TX, &le64, sizeof(le64));
		le32 = cpu_to_le32(sta->rssi);
		pos += encap_tlv(pos, TLVTYPE_RSSI, &le32, sizeof(le32));

		/* send 16 sta_stats per msg */
		if ((i & 0xf) == 0) {
			payload_len = pos - nlmsg->rpemsg->payload;
			qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_STA_PHY_STATS, RPE_API_V1, bss->node.mac, payload_len);
			qwerpe_send_nlmsg(nlmsg);

			if (i == sta_num) {
				kfree(sta_list);
				return 0;
			}

			nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, MAX_RPEMSG_LEN);
			if (!nlmsg) {
				QWERPE_WARN("Failed to alloc nlmsg\n");
				kfree(sta_list);
				return -1;
			}
			pos = nlmsg->rpemsg->payload;
		}
	}
	payload_len = pos - nlmsg->rpemsg->payload;
	qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_STA_PHY_STATS, RPE_API_V1, bss->node.mac, payload_len);
	qwerpe_send_nlmsg(nlmsg);
	kfree(sta_list);

	return 0;
}

static int qwerpe_send_nonassoc_sta_stats(void *bss_ctx, uint8_t *sta_mac)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;
	struct qwerpe_nonassoc_sta_stats *sta_list, *sta;
	struct qwerpe_nlmsg __nlmsg, *nlmsg;
	int i, sta_num = 1;
	uint8_t *pos;
	uint16_t payload_len;
	uint32_t le32;
	uint64_t le64;

	if (is_broadcast_ether_addr(sta_mac))
		sta_num = MAX_NONASSOC_STA_NUM_PER_BSS;

	sta_list = kzalloc(sizeof(struct qwerpe_nonassoc_sta_stats) * sta_num, GFP_KERNEL);
	if (!sta_list) {
		QWERPE_WARN("Failed to alloc memory\n");
		return -1;
	}

	sta_num = bss->drv_ops->get_nonassoc_sta_stats(bss->drv_ctx, sta_mac, sta_list);
	if (sta_num == 0) {
		kfree(sta_list);
		return -1;
	}

	nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, MAX_RPEMSG_LEN);
	if (!nlmsg) {
		QWERPE_WARN("Failed to alloc nlmsg\n");
		kfree(sta_list);
		return -1;
	}
	pos = nlmsg->rpemsg->payload;

	for (i = 1, sta = sta_list; i <= sta_num; ++i, ++sta) {
		pos += encap_tlv(pos, TLVTYPE_STA_MAC, sta->sta_mac, ETH_ALEN);
		le32 = cpu_to_le32(sta->rssi);
		pos += encap_tlv(pos, TLVTYPE_RSSI, &le32, sizeof(le32));
		le64 = cpu_to_le64(sta->rx_tstamp);
		pos += encap_tlv(pos, TLVTYPE_TS_LAST_RX, &le64, sizeof(le64));

		/* send 32 nonassoc_sta_stats per msg */
		if ((i & 0x1f) == 0) {
			payload_len = pos - nlmsg->rpemsg->payload;
			qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_NONASSOC_STA_PHY_STATS, RPE_API_V1, bss->node.mac, payload_len);
			qwerpe_send_nlmsg(nlmsg);

			if (i == sta_num) {
				kfree(sta_list);
				return 0;
			}

			nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, MAX_RPEMSG_LEN);
			if (!nlmsg) {
				QWERPE_WARN("Failed to alloc nlmsg\n");
				kfree(sta_list);
				return -1;
			}
			pos = nlmsg->rpemsg->payload;
		}
	}
	payload_len = pos - nlmsg->rpemsg->payload;
	qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_NONASSOC_STA_PHY_STATS, RPE_API_V1, bss->node.mac, payload_len);
	qwerpe_send_nlmsg(nlmsg);
	kfree(sta_list);

	return 0;
}

uint64_t qwerpe_get_timestamp(void)
{
	struct timespec ts;

	do_posix_clock_monotonic_gettime(&ts);

	return ts.tv_sec;
}
EXPORT_SYMBOL(qwerpe_get_timestamp);

void qwerpe_parse_cap_ie(uint8_t *frm, int len,
				uint8_t **ht_cap_ie, uint8_t **vht_cap_ie, uint8_t **ext_cap_ie)
{
	uint8_t *efrm = frm + len;

	*ht_cap_ie = NULL;
	*vht_cap_ie = NULL;
	*ext_cap_ie = NULL;

	while (frm < efrm) {
		switch(*frm) {
		case HT_CAP_IE_ID:
			*ht_cap_ie = frm;
			break;
		case VHT_CAP_IE_ID:
			*vht_cap_ie = frm;
			break;
		case EXT_CAP_IE_ID:
			*ext_cap_ie = frm;
			break;
		default:
			break;
		}

		frm += frm[1] + 2;
	}
}
EXPORT_SYMBOL(qwerpe_parse_cap_ie);

int qwerpe_send_intf_status(void *bss_ctx, int status)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;
	struct qwerpe_nlmsg __nlmsg, *nlmsg;
	struct evt_intf_status *intf;
    
    if (!bss) {
		QWERPE_WARN("bss_ctx is NULL");
		return -1;
	}
	nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, RPEMSG_LEN(sizeof(struct evt_intf_status)));
	if (!nlmsg) {

		QWERPE_WARN("Failed to alloc nlmsg\n");
		return -1;
	}
	intf = (struct evt_intf_status *)nlmsg->rpemsg->payload;
    
    intf->ifname_size = strlen(bss->ifname);
	strncpy(intf->ifname, bss->ifname, sizeof(intf->ifname));
	intf->status = cpu_to_le32(status);

	qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_INTF_STATUS, RPE_API_V3, bss->node.mac, sizeof(struct evt_intf_status));
	qwerpe_send_nlmsg(nlmsg);
	return 0;
}
EXPORT_SYMBOL(qwerpe_send_intf_status);

int qwerpe_send_intf_info(void *bss_ctx, uint32_t specifier)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;
	struct qwerpe_intf_info intf_info;
	struct qwerpe_nlmsg __nlmsg, *nlmsg;
	uint8_t *pos;
	uint16_t payload_len;
	uint8_t bssid_mdid[ETH_ALEN + MDID_LEN];
	uint16_t beacon_interval;
	struct ht_cap_ie ht_cap;
	struct ht_op_ie ht_op;
	struct vht_cap_ie vht_cap;
	struct vht_op_ie vht_op;
	struct {
		uint8_t channel;
		uint8_t band;
		uint8_t op_class;
	} __attribute__ ((packed)) ch_band_opclass;
	struct {
		uint8_t drv_cap;
		uint8_t phy_type;
		uint16_t cap_info;
	} __attribute__ ((packed)) intf_cap;
	struct {
		uint8_t channel;
		uint8_t band;
		uint16_t fat;
	} __attribute__ ((packed)) ch_band_fat;

	if (!bss) {
		QWERPE_WARN("bss_ctx is NULL");
		return -1;
	}

	memset(&intf_info, 0, sizeof(intf_info));
	if (bss->drv_ops->get_intf_info(bss->drv_ctx, &intf_info) != 0) {
		QWERPE_WARN("Failed to get info for %s\n", bss->ifname);
		return -1;
	}

	nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, MAX_RPEMSG_LEN);
	if (!nlmsg) {
		QWERPE_WARN("Failed to alloc nlmsg\n");
		return -1;
	}
	pos = nlmsg->rpemsg->payload;

	if (specifier == 1) {
		memcpy(bssid_mdid, bss->node.mac, ETH_ALEN);
		memcpy(bssid_mdid + ETH_ALEN, intf_info.mdid, MDID_LEN);
		ch_band_opclass.channel = intf_info.channel;
		ch_band_opclass.band = intf_info.band;
		ch_band_opclass.op_class = intf_info.op_class;
		intf_cap.drv_cap = intf_info.drv_cap;
		intf_cap.phy_type = intf_info.phy_type;
		intf_cap.cap_info = cpu_to_le16(intf_info.cap_info);
		beacon_interval = cpu_to_le16(intf_info.beacon_interval);

		/* TLVTYPE_BSSID_MDID should be the first TLV RPE_EVENT_INTF_INFO */
		pos += encap_tlv(pos, TLVTYPE_BSSID_MDID, bssid_mdid, sizeof(bssid_mdid));
		pos += encap_var_tlv(pos, TLVTYPE_IFNAME, bss->ifname, strlen(bss->ifname));
		pos += encap_tlv(pos, TLVTYPE_CHANNEL_BAND, &ch_band_opclass, sizeof(ch_band_opclass));
		pos += encap_tlv(pos, TLVTYPE_INTERFACE_CAPABILITY, &intf_cap, sizeof(intf_cap));
		pos += encap_tlv(pos, TLVTYPE_BEACON_INTERVAL, &beacon_interval, sizeof(beacon_interval));
		if (intf_info.drv_cap & RPE_DRV_CAP_HT) {
			if (bss->drv_ops->get_intf_ht_ie(bss->drv_ctx, &ht_cap, &ht_op) != 0) {
				QWERPE_WARN("Failed to get ht ie for %s\n", bss->ifname);
				qwerpe_free_nlmsg(nlmsg);
				return -1;
			}
			pos += encap_tlv(pos, TLVTYPE_HT_CAPABILITY, &ht_cap, sizeof(ht_cap));
			pos += encap_tlv(pos, TLVTYPE_HT_OPERATION, &ht_op, sizeof(ht_op));
		}
		if (intf_info.drv_cap & RPE_DRV_CAP_VHT) {
			if (bss->drv_ops->get_intf_vht_ie(bss->drv_ctx, &vht_cap, &vht_op) != 0) {
				QWERPE_WARN("Failed to get vht ie for %s\n", bss->ifname);
				qwerpe_free_nlmsg(nlmsg);
				return -1;
			}
			pos += encap_tlv(pos, TLVTYPE_VHT_CAPABILITY, &vht_cap, sizeof(vht_cap));
			pos += encap_tlv(pos, TLVTYPE_VHT_OPERATION, &vht_op, sizeof(vht_op));
		}
	} else if (specifier == 3) {
		ch_band_fat.channel = intf_info.channel;
		ch_band_fat.band = intf_info.band;
		ch_band_fat.fat = bss->drv_ops->get_intf_fat(bss->drv_ctx);
		ch_band_fat.fat = cpu_to_le16(ch_band_fat.fat);
		pos += encap_tlv(pos, TLVTYPE_AVERAGE_FAT, &ch_band_fat, sizeof(ch_band_fat));
	} else {
		QWERPE_WARN("qwerpe_send_intf_info don't support specifier %u\n", specifier);
		qwerpe_free_nlmsg(nlmsg);
		return -1;
	}

	payload_len = pos - nlmsg->rpemsg->payload;
	qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_INTF_INFO, RPE_API_V3, bss->node.mac, payload_len);
	qwerpe_send_nlmsg(nlmsg);

	return 0;
}
EXPORT_SYMBOL(qwerpe_send_intf_info);

int qwerpe_send_probe_req(void *bss_ctx, uint8_t *sta_mac, int32_t rssi,
				uint8_t *ht_cap_ie, uint8_t *vht_cap_ie, uint8_t *ext_cap_ie,
				uint8_t *cookie, uint16_t cookie_len)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;
	struct qwerpe_intf_info intf_info;
	struct qwerpe_nlmsg __nlmsg, *nlmsg;
	struct evt_probe_req *sta;
	uint16_t payload_len;
	uint32_t ht_ss = 1, vht_ss = 0, max_ss;
	uint32_t ht_phy_rate = 54, vht_phy_rate = 0, max_phy_rate;
	struct ht_cap_ie *ht_cap = (struct ht_cap_ie *)ht_cap_ie;
	struct vht_cap_ie *vht_cap = (struct vht_cap_ie *)vht_cap_ie;
	struct ext_cap_ie *ext_cap = (struct ext_cap_ie *)ext_cap_ie;

	if (!bss) {
		QWERPE_WARN("bss_ctx is NULL");
		return -1;
	}

	if (qwerpe_get_state() != QWERPE_STATE_ON)
		return 0;

	memset(&intf_info, 0, sizeof(intf_info));
	if (bss->drv_ops->get_intf_info(bss->drv_ctx, &intf_info) != 0) {
		QWERPE_WARN("Failed to get info for %s\n", bss->ifname);
		return -1;
	}

	nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, MAX_RPEMSG_LEN);
	if (!nlmsg) {
		QWERPE_WARN("Failed to alloc nlmsg\n");
		return -1;
	}
	sta = (struct evt_probe_req *)nlmsg->rpemsg->payload;

	if (ht_cap) {
		sta->capability |= RPE_STA_CAP_HT_SUPPORT;
		if (IS_SET_BIT(ht_cap->cap_info, HT_CAP_40M_CH_BIT))
			sta->capability |= RPE_STA_CAP_40M;
		ht_phy_rate = ieee80211_wlan_ht_rx_maxrate(ht_cap, &ht_ss);
	}
	if (vht_cap) {
		sta->capability |= RPE_STA_CAP_VHT_SUPPORT;
		if (IS_SET_BIT(vht_cap->cap_info, VHT_CAP_160M_CH_BIT))
			sta->capability |= RPE_STA_CAP_160M;
		else if (IS_SET_BIT(vht_cap->cap_info, VHT_CAP_160M_80P80_CH_BIT))
			sta->capability |= RPE_STA_CAP_160M_80P80;

		if (IS_SET_BIT(vht_cap->cap_info, VHT_CAP_MU_BEAMFORMER_BIT))
			sta->capability |= RPE_STA_CAP_MU_BEAMFORMER;
		if (IS_SET_BIT(vht_cap->cap_info, VHT_CAP_MU_BEAMFORMEE_BIT))
			sta->capability |= RPE_STA_CAP_MU_BEAMFORMEE;

		//vht_ss = ieee80211_wlan_vht_rxstreams(vht_cap);
		//vht_phy_rate = ieee80211_wlan_vht_rx_maxrate(vht_cap);
	}
	if (ext_cap && ext_cap->len >= 3) {
		if (IS_SET_BIT(ext_cap->cap, EXT_CAP_BSS_TRANS_BIT))
			sta->capability |= RPE_STA_CAP_BSS_TRANS_SUPPORT;
	}

	max_ss = vht_ss > ht_ss ? vht_ss : ht_ss;
	max_phy_rate = vht_phy_rate > ht_phy_rate ? vht_phy_rate : ht_phy_rate;
	memcpy(sta->sta_mac, sta_mac, ETH_ALEN);
	sta->curr_band = cpu_to_le16((uint16_t)intf_info.band);
	sta->rssi = cpu_to_le32(rssi);
	sta->rx_ss = cpu_to_le16((uint16_t)max_ss);
	sta->max_phyrate = cpu_to_le16((uint16_t)max_phy_rate);
	sta->tstamp = cpu_to_le64(qwerpe_get_timestamp());
	sta->channel = intf_info.channel;
	if (cookie != NULL && cookie_len != 0) {
		if (RPEMSG_LEN(sizeof(struct evt_probe_req) + cookie_len) <= MAX_RPEMSG_LEN) {
			sta->cookie_len = cpu_to_le16(cookie_len);
			memcpy(sta->cookie, cookie, cookie_len);
		} else {
			QWERPE_WARN("cookie in probe req is too long(%u) to carry\n", cookie_len);
			cookie_len = 0;
		}
	} else
		cookie_len = 0;

	payload_len = sizeof(struct evt_probe_req) + cookie_len;
	qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_PROBE_REQ, RPE_API_V3, bss->node.mac, payload_len);
	qwerpe_send_nlmsg(nlmsg);

	return 0;

}
EXPORT_SYMBOL(qwerpe_send_probe_req);

int qwerpe_send_conn_complete(void *bss_ctx, uint8_t *sta_mac,
				uint8_t *ht_cap_ie, uint8_t *vht_cap_ie, uint8_t *ext_cap_ie,
				uint8_t *cookie, uint16_t cookie_len)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;
	struct qwerpe_intf_info intf_info;
	struct qwerpe_nlmsg __nlmsg, *nlmsg;
	struct evt_connect_complete *sta;
	uint16_t payload_len;
	uint32_t ht_ss = 1, vht_ss = 0, max_ss;
	uint32_t ht_phy_rate = 54, vht_phy_rate = 0, max_phy_rate;
	struct ht_cap_ie *ht_cap = (struct ht_cap_ie *)ht_cap_ie;
	struct vht_cap_ie *vht_cap = (struct vht_cap_ie *)vht_cap_ie;
	struct ext_cap_ie *ext_cap = (struct ext_cap_ie *)ext_cap_ie;

	if (!bss) {
		QWERPE_WARN("bss_ctx is NULL");
		return -1;
	}

	if (qwerpe_get_state() != QWERPE_STATE_ON)
		return 0;

	memset(&intf_info, 0, sizeof(intf_info));
	if (bss->drv_ops->get_intf_info(bss->drv_ctx, &intf_info) != 0) {
		QWERPE_WARN("Failed to get info for %s\n", bss->ifname);
		return -1;
	}

	nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, MAX_RPEMSG_LEN);
	if (!nlmsg) {
		QWERPE_WARN("Failed to alloc nlmsg\n");
		return -1;
	}
	sta = (struct evt_connect_complete *)nlmsg->rpemsg->payload;

	if (ht_cap) {
		sta->capability |= RPE_STA_CAP_HT_SUPPORT;
		if (IS_SET_BIT(ht_cap->cap_info, HT_CAP_40M_CH_BIT))
			sta->capability |= RPE_STA_CAP_40M;
		ht_phy_rate = ieee80211_wlan_ht_rx_maxrate(ht_cap, &ht_ss);
	}
	if (vht_cap) {
		sta->capability |= RPE_STA_CAP_VHT_SUPPORT;
		if (IS_SET_BIT(vht_cap->cap_info, VHT_CAP_160M_CH_BIT))
			sta->capability |= RPE_STA_CAP_160M;
		else if (IS_SET_BIT(vht_cap->cap_info, VHT_CAP_160M_80P80_CH_BIT))
			sta->capability |= RPE_STA_CAP_160M_80P80;

		if (IS_SET_BIT(vht_cap->cap_info, VHT_CAP_MU_BEAMFORMER_BIT))
			sta->capability |= RPE_STA_CAP_MU_BEAMFORMER;
		if (IS_SET_BIT(vht_cap->cap_info, VHT_CAP_MU_BEAMFORMEE_BIT))
			sta->capability |= RPE_STA_CAP_MU_BEAMFORMEE;

		//vht_ss = ieee80211_wlan_vht_rxstreams(vht_cap);
		//vht_phy_rate = ieee80211_wlan_vht_rx_maxrate(vht_cap);
	}
	if (ext_cap && ext_cap->len >= 3) {
		if (IS_SET_BIT(ext_cap->cap, EXT_CAP_BSS_TRANS_BIT))
			sta->capability |= RPE_STA_CAP_BSS_TRANS_SUPPORT;
	}

	max_ss = vht_ss > ht_ss ? vht_ss : ht_ss;
	max_phy_rate = vht_phy_rate > ht_phy_rate ? vht_phy_rate : ht_phy_rate;
	memcpy(sta->sta_mac, sta_mac, ETH_ALEN);
	sta->curr_band = cpu_to_le16((uint16_t)intf_info.band);
	sta->rx_ss = cpu_to_le16((uint16_t)max_ss);
	sta->max_phyrate = cpu_to_le16((uint16_t)max_phy_rate);
	sta->channel = intf_info.channel;
	if (cookie != NULL && cookie_len != 0) {
		if (RPEMSG_LEN(sizeof(struct evt_probe_req) + cookie_len) <= MAX_RPEMSG_LEN) {
			sta->cookie_len = cpu_to_le16(cookie_len);
			memcpy(sta->cookie, cookie, cookie_len);
		} else {
			QWERPE_WARN("cookie in probe req is too long(%u) to carry\n", cookie_len);
			cookie_len = 0;
		}
	} else
		cookie_len = 0;

	payload_len = sizeof(struct evt_connect_complete) + cookie_len;
	qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_CONNECT_COMPLETE, RPE_API_V3, bss->node.mac, payload_len);
	qwerpe_send_nlmsg(nlmsg);

	return 0;
}
EXPORT_SYMBOL(qwerpe_send_conn_complete);

int qwerpe_send_deauth(void *bss_ctx, uint8_t *sta_mac, uint16_t reason_code, uint8_t direction)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;
	struct qwerpe_nlmsg __nlmsg, *nlmsg;
	struct evt_deauth *evt;

	if (!bss) {
		QWERPE_WARN("bss_ctx is NULL");
		return -1;
	}

	nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, RPEMSG_LEN(sizeof(struct evt_deauth)));
	if (!nlmsg) {
		QWERPE_WARN("Failed to alloc nlmsg\n");
		return -1;
	}
	evt = (struct evt_deauth *)nlmsg->rpemsg->payload;

	memcpy(evt->sta_mac, sta_mac, ETH_ALEN);
	evt->reason_code = cpu_to_le16(reason_code);
	evt->direction = direction;

	qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_DEAUTH, RPE_API_V3, bss->node.mac, sizeof(struct evt_deauth));
	qwerpe_send_nlmsg(nlmsg);

	return 0;
}
EXPORT_SYMBOL(qwerpe_send_deauth);

int qwerpe_send_disassoc(void *bss_ctx, uint8_t *sta_mac, uint16_t reason_code, uint8_t direction)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;
	struct qwerpe_nlmsg __nlmsg, *nlmsg;
	struct evt_disassoc *evt;

	if (!bss) {
		QWERPE_WARN("bss_ctx is NULL");
		return -1;
	}

	nlmsg = qwerpe_alloc_nlmsg(&__nlmsg, RPEMSG_LEN(sizeof(struct evt_disassoc)));
	if (!nlmsg) {
		QWERPE_WARN("Failed to alloc nlmsg\n");
		return -1;
	}
	evt = (struct evt_disassoc *)nlmsg->rpemsg->payload;

	memcpy(evt->sta_mac, sta_mac, ETH_ALEN);
	evt->reason_code = cpu_to_le16(reason_code);
	evt->direction = direction;

	qwerpe_init_msg_hdr(nlmsg->rpemsg, EVT_DISASSOC, RPE_API_V3, bss->node.mac, sizeof(struct evt_disassoc));
	qwerpe_send_nlmsg(nlmsg);

	return 0;
}
EXPORT_SYMBOL(qwerpe_send_disassoc);

static int qwerpe_handle_init(struct rpemsg *msg)
{
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;

	qwerpe_ctx->state = QWERPE_STATE_ON;
	QWERPE_NOTICE("qwerpe is on\n");

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		mac_list_clear(&bss->acl_list);
		qwerpe_send_intf_info(bss, 1);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;
}

static int qwerpe_handle_deinit(struct rpemsg *msg)
{
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		mac_list_clear(&bss->acl_list);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	qwerpe_ctx->state = QWERPE_STATE_OFF;
	QWERPE_NOTICE("qwerpe is off\n");

	return 0;
}

static int qwerpe_handle_get_intf_status(struct rpemsg *msg)
{
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;
	int status;

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	if (likely(!is_broadcast_ether_addr(msg->bssid))) {
		bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
		if (!bss)
			return 0;
		status = bss->drv_ops->get_intf_status(bss->drv_ctx);
		qwerpe_send_intf_status(bss, status);
		return 0;
	}

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		status = bss->drv_ops->get_intf_status(bss->drv_ctx);
		qwerpe_send_intf_status(bss, status);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;
}
static int __test(void)
{
    struct mac_node *node;
	struct qwerpe_bss_ctx *bss;
	int status;
	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		status = bss->drv_ops->get_intf_status(bss->drv_ctx);
		qwerpe_send_intf_status(bss, status);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;


}

static int qwerpe_handle_get_intf_info(struct rpemsg *msg)
{
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;
	struct cmd_get_intf_info *cmd = (struct cmd_get_intf_info *)msg->payload;
	uint32_t specifier = le32_to_cpu(cmd->specifier);

	if (specifier != 1 && specifier != 3) {
		QWERPE_WARN("CMD_GET_INTF_INFO with invalid specifier %u\n", specifier);
		return 0;
	}

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	if (likely(!is_broadcast_ether_addr(msg->bssid))) {
		bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
		if (!bss)
			return 0;
		qwerpe_send_intf_info(bss, specifier);
		return 0;
	}

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		qwerpe_send_intf_info(bss, specifier);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;
}

static int qwerpe_handle_deauth(struct rpemsg *msg)
{
	struct cmd_deauth *cmd = (struct cmd_deauth *)msg->payload;
	struct qwerpe_bss_ctx *bss;

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
	if (!bss)
		return 0;

	return bss->drv_ops->deauth_sta(bss->drv_ctx, cmd->sta_mac, le16_to_cpu(cmd->reasoncode));
}

static int qwerpe_handle_sta_mac_filter(struct rpemsg *msg)
{
	struct qwerpe_bss_ctx *bss;
	struct qwerpe_acl_ctx *acl;
	struct cmd_mac_filter *cmd= (struct cmd_mac_filter *)msg->payload;
	uint16_t permission;

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
	if (!bss)
		return 0;

	permission = le16_to_cpu(cmd->permission);
	if (permission == ALLOW_ACCESS) {
		mac_list_del(&bss->acl_list, cmd->sta_mac);
		return 0;
	} else if (permission == DENY_ACCESS) {
		acl = mac_list_find(&bss->acl_list, cmd->sta_mac);
		if (acl)
			return 0;
		acl = kzalloc(sizeof(struct qwerpe_acl_ctx), GFP_KERNEL);
		if (!acl) {
			QWERPE_WARN("Fail to add [%pM] to qwerpe acl: no memory\n", cmd->sta_mac);
			return -1;
		}

		memcpy(acl->node.mac, cmd->sta_mac, ETH_ALEN);
		mac_list_add(&bss->acl_list, &acl->node);
	} else {
		QWERPE_WARN("Fail to add [%pM] to qwerpe acl: error permission %d\n", cmd->sta_mac, permission);
		return -1;
	}

	return 0;
}

static int qwerpe_handle_get_sta_stats(struct rpemsg *msg)
{
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;
	struct cmd_get_sta_stats *cmd = (struct cmd_get_sta_stats *)msg->payload;

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	if (likely(!is_broadcast_ether_addr(msg->bssid))) {
		bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
		if (!bss)
			return 0;
		qwerpe_send_sta_stats(bss, cmd->sta_mac);
		return 0;
	}

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		qwerpe_send_sta_stats(bss, cmd->sta_mac);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;
}

static int qwerpe_handle_bss_trans_req(struct rpemsg *msg)
{
	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	return 0;
}

static int qwerpe_handle_start_fat_monitoring(struct rpemsg *msg)
{
	struct cmd_start_fat_monitor *cmd = (struct cmd_start_fat_monitor *)msg->payload;
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;
	uint32_t period = cpu_to_le32(cmd->fat_period);

	if (period <= 0) {
		QWERPE_WARN("Invalid period: %u, it should greater than 0\n", period);
		return -1;
	}

	if (likely(!is_broadcast_ether_addr(msg->bssid))) {
		bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
		if (!bss)
			return 0;
		bss->drv_ops->start_fat_monitor(bss->drv_ctx, period);
		return 0;
	}

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		bss->drv_ops->start_fat_monitor(bss->drv_ctx, period);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;
}

static int qwerpe_handle_monitor_start(struct rpemsg *msg)
{
	struct cmd_monitor_start *cmd = (struct cmd_monitor_start *)msg->payload;
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;
	uint16_t period = cpu_to_le16(cmd->period);
	uint16_t duty_cycle = cpu_to_le16(cmd->duty_cycle);

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	if (duty_cycle >= 100) {
		QWERPE_WARN("Invalid duty_cycle: %u, it should less than 100\n", duty_cycle);
		return -1;
	}

	if ((period * duty_cycle) / 100 <= 0) {
		QWERPE_WARN("Invalid period: %u and duty_cycle: %u, (period * duty_cycle) / 100 should greater than 0\n", period, duty_cycle);
		return -1;
	}

	if (likely(!is_broadcast_ether_addr(msg->bssid))) {
		bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
		if (!bss)
			return 0;
		bss->drv_ops->start_nonassoc_sta_monitor(bss->drv_ctx, period, duty_cycle);
		return 0;
	}

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		bss->drv_ops->start_nonassoc_sta_monitor(bss->drv_ctx, period, duty_cycle);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;
}

static int qwerpe_handle_monitor_stop(struct rpemsg *msg)
{
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	if (likely(!is_broadcast_ether_addr(msg->bssid))) {
		bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
		if (!bss)
			return 0;
		bss->drv_ops->stop_nonassoc_sta_monitor(bss->drv_ctx);
		return 0;
	}

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		bss->drv_ops->stop_nonassoc_sta_monitor(bss->drv_ctx);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;
}

static int qwerpe_handle_get_nonassoc_stats(struct rpemsg *msg)
{
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;
	struct cmd_get_nonassoc_sta_stats *cmd = (struct cmd_get_nonassoc_sta_stats *)msg->payload;

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;

	if (likely(!is_broadcast_ether_addr(msg->bssid))) {
		bss = mac_list_find(&qwerpe_ctx->bss_list, msg->bssid);
		if (!bss)
			return 0;
		qwerpe_send_nonassoc_sta_stats(bss, cmd->sta_mac);
		return 0;
	}

	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		qwerpe_send_nonassoc_sta_stats(bss, cmd->sta_mac);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);

	return 0;
}

static struct rpecmd rpecmd_list[] = {
	{"Unknow", 0, NULL},
	{"CMD_INIT", 0, qwerpe_handle_init},
	{"CMD_DEINIT", 0, qwerpe_handle_deinit},
	{"CMD_GET_INTF_STATUS", sizeof(struct cmd_get_intf_info), qwerpe_handle_get_intf_status},
	{"CMD_GET_INTF_INFO", sizeof(struct cmd_get_intf_info), qwerpe_handle_get_intf_info},
	{"CMD_DEAUTH", sizeof(struct cmd_deauth), qwerpe_handle_deauth},
	{"CMD_STA_MAC_FILTER", sizeof(struct cmd_mac_filter), qwerpe_handle_sta_mac_filter},
	{"CMD_GET_STA_STATS", sizeof(struct cmd_get_sta_stats), qwerpe_handle_get_sta_stats},
	{"CMD_BSS_TRANS_REQ", UNFIXED_PAYLOAD_LEN, qwerpe_handle_bss_trans_req},
	{"CMD_START_FAT_MONITORING", sizeof(struct cmd_start_fat_monitor), qwerpe_handle_start_fat_monitoring},
	{"CMD_MONITOR_START", sizeof(struct cmd_monitor_start), qwerpe_handle_monitor_start},
	{"CMD_MONITOR_STOP", sizeof(struct cmd_monitor_stop), qwerpe_handle_monitor_stop},
	{"CMD_GET_NONASSOC_STATS", sizeof(struct cmd_get_nonassoc_sta_stats), qwerpe_handle_get_nonassoc_stats},
};

static int qwerpe_recv_cmd(struct sk_buff *skb, struct nlmsghdr *nlh, void *arg)
{
	struct rpemsg *msg;
	uint16_t id;
	uint16_t payload_len;
	struct rpecmd *cmd;

	if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(struct rpemsg))) {
		QWERPE_DEBUG("Ignore too short msg\n");
		return 0;
	}

	msg = (struct rpemsg *)NLMSG_DATA(nlh);
	id = le16_to_cpu(msg->id);
	payload_len = le16_to_cpu(msg->payload_len);

	if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(struct rpemsg)) + payload_len) {
		QWERPE_DEBUG("Ignore truncated msg\n");
		return 0;
	}

	if (id >= RPE_CMD_MAX || id <= 0) {
		QWERPE_DEBUG("Ignore msg with id %d\n", id);
		return 0;
	}

	cmd = &rpecmd_list[id];
	if (unlikely(cmd->payload_len != UNFIXED_PAYLOAD_LEN && cmd->payload_len != payload_len)) {
		QWERPE_DEBUG("Ignore CMD %s to BSSID %pM: payload length not match\n", cmd->name, msg->bssid);
		return 0;
	}

	QWERPE_DEBUG("Recv CMD %s to BSSID %pM\n", cmd->name, msg->bssid);
	cmd->handle(msg);

	return 0;
}

static void destroy_qwerpe_bss_ctx(const void *ctx)
{
	mac_list_clear(&((struct qwerpe_bss_ctx *)ctx)->acl_list);
	kfree(ctx);
}

static void *qwerpe_register_bss(uint8_t *bssid, char *ifname, void *drv_ctx, struct qwerpe_drv_ops *drv_ops)
{
	struct qwerpe_bss_ctx *bss;
	bss = mac_list_find(&qwerpe_ctx->bss_list, bssid);
	if (unlikely(bss)) {
		strncpy(bss->ifname, ifname, IFNAMSIZ);
		bss->drv_ctx = drv_ctx;
		bss->drv_ops = drv_ops;
		QWERPE_INFO("qwerpe bss for %s [%pM] is updated\n", ifname, bssid);
		return bss;
	}

	bss = kzalloc(sizeof(struct qwerpe_bss_ctx), GFP_KERNEL);
	if (!bss) {
		QWERPE_WARN("Fail to register qwerpe bss for %s [%pM]: no memory\n", ifname, bssid);
		return NULL;
	}

	memcpy(bss->node.mac, bssid, ETH_ALEN);
	strncpy(bss->ifname, ifname, IFNAMSIZ);
	bss->drv_ctx = drv_ctx;
	bss->drv_ops = drv_ops;
	INIT_LIST_HEAD(&bss->acl_list.head);
	spin_lock_init(&bss->acl_list.lock);
	bss->acl_list.destroy = kfree;

	mac_list_add(&qwerpe_ctx->bss_list, &bss->node);

	QWERPE_INFO("qwerpe bss [%pM] for %s is created\n", bssid, ifname);
	return bss;
}

static void qwerpe_unregister_bss(uint8_t *bssid)
{
	mac_list_del(&qwerpe_ctx->bss_list, bssid);
	QWERPE_INFO("qwerpe bss [%pM] is deleted\n", bssid);
}

int qwerpe_is_sta_denied(void *bss_ctx, uint8_t *mac)
{
	struct qwerpe_bss_ctx *bss = (struct qwerpe_bss_ctx *)bss_ctx;

	if (qwerpe_ctx->state != QWERPE_STATE_ON)
		return 0;
	if (mac_list_find(&bss->acl_list, mac))
		return 1;

	return 0;
}
EXPORT_SYMBOL(qwerpe_is_sta_denied);

static int init_qwerpe_ctx(void)
{
	qwerpe_ctx = kzalloc(sizeof(struct qwerpe_ctx), GFP_KERNEL);
	if (!qwerpe_ctx)
		return -ENOMEM;

	INIT_LIST_HEAD(&qwerpe_ctx->bss_list.head);
	spin_lock_init(&qwerpe_ctx->bss_list.lock);
	qwerpe_ctx->bss_list.destroy = destroy_qwerpe_bss_ctx;
	qwerpe_ctx->state = QWERPE_STATE_ON;
	skb_queue_head_init(&qwerpe_ctx->nlmsg_queue);
	INIT_WORK(&qwerpe_ctx->nlmsg_send_wq, evt_send_work);

	return 0;
}

static void destroy_qwerpe_ctx(void)
{
	if (!qwerpe_ctx)
		return;

	qwerpe_ctx->state = QWERPE_STATE_OFF;
	//mac_list_clear(&qwerpe_ctx->bss_list);
	flush_scheduled_work();
	skb_queue_purge(&qwerpe_ctx->nlmsg_queue);

	kfree(qwerpe_ctx);
	qwerpe_ctx = NULL;
}

static void qwerpe_show_acl(struct mac_list *list)
{
	struct mac_node *node;

	LIST_LOCK(list);
	list_for_each_entry(node, &list->head, lh) {
		printk("                              %pM\n", node->mac);
	}
	LIST_UNLOCK(list);
}

static void qwerpe_show_bss(void)
{
	struct mac_node *node;
	struct qwerpe_bss_ctx *bss;

	printk("MAC               Name        ACL\n");
	LIST_LOCK(&qwerpe_ctx->bss_list);
	list_for_each_entry(node, &qwerpe_ctx->bss_list.head, lh) {
		bss = (struct qwerpe_bss_ctx *)node;
		printk("%pM %s\n", node->mac, bss->ifname);
		qwerpe_show_acl(&bss->acl_list);
	}
	LIST_UNLOCK(&qwerpe_ctx->bss_list);
}

static void qwerpe_show_help(void)
{
	printk("\nCommands:\n");
	printk("help			:show usage\n");
	printk("show			:show bss and qwerpe status\n");
	printk("log_level <level>	:set log level\n");
}

static int qwerpe_write_proc(struct file *file, const char __user *buffer,
		unsigned long count, void *_unused)
{
#define QWERPE_MAX_ARG	16
	int argc;
	char *argv[QWERPE_MAX_ARG] = {NULL};
	char *buf, *pos;

	buf = kzalloc(count + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
	if (copy_from_user(buf, buffer, count))
		goto out;

	pos = buf;
	for (argc = 0; *pos && argc < QWERPE_MAX_ARG; ++argc) {
		argv[argc] = pos;
		while (*pos) {
			if (*pos == ' ' || *pos == '\n') {
				*pos++ = 0;
				break;
			}
			++pos;
		}
	}

	if (strcmp(argv[0], "show") == 0) {
		printk("state: %s\n", qwerpe_ctx->state == QWERPE_STATE_ON ? "on" : "off");
		printk("log_level: %s\n", log_level_to_str(qwerpe_log_level));
		qwerpe_show_bss();
	} else if (strcmp(argv[0], "log_level") == 0) {
		if (argc != 2) {
			printk("please specify the level you want to set\n");
			goto out;
		}
		if (strcmp(argv[1], "debug") == 0)
			qwerpe_log_level = QWERPE_LOG_DEBUG;
		else if (strcmp(argv[1], "info") == 0)
			qwerpe_log_level = QWERPE_LOG_INFO;
		else if (strcmp(argv[1], "notice") == 0)
			qwerpe_log_level = QWERPE_LOG_NOTICE;
		else if (strcmp(argv[1], "warn") == 0)
			qwerpe_log_level = QWERPE_LOG_WARN;
		else if (strcmp(argv[1], "error") == 0)
			qwerpe_log_level = QWERPE_LOG_ERR;
		else {
			printk("invalid level \"%s\", must be one of debug, info, notice, warn, error\n", argv[1]);
			goto out;
		}
	} else {
		qwerpe_show_help();
	}
out:
	kfree(buf);
	return count;
}

static int qwerpe_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *_unused)
{
    __test();
	return 0;
}

/*
 * Module glue.
 */
MODULE_AUTHOR("Quantenna, Jason.Wang");
MODULE_DESCRIPTION("Quantenna RPE for QWE radio");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

static int __init init_qwerpe(void)
{
	int ret;
	struct proc_dir_entry *entry;

	printk("Start RPE for QWE radio\n");
	rtnl_register(PF_UNSPEC, RTM_RPECMD, qwerpe_recv_cmd, NULL, NULL);
	ret = init_qwerpe_ctx();
	if (ret != 0)
		goto fail;
    qwerpe_register_bss_hook = qwerpe_register_bss;
    qwerpe_unregister_bss_hook = qwerpe_unregister_bss;

	entry = create_proc_entry(QWERPE_PROC_NAME, 0600, NULL);
	if (entry) {
		entry->write_proc = qwerpe_write_proc;
		entry->read_proc = qwerpe_read_proc;
	}

fail:
	return 0;

}
module_init(init_qwerpe);

static void __exit exit_qwerpe(void)
{
	printk("Stop RPE for QWE radio\n");
	rtnl_unregister(PF_UNSPEC, RTM_RPECMD);
	destroy_qwerpe_ctx();
	remove_proc_entry(QWERPE_PROC_NAME, NULL);
}
module_exit(exit_qwerpe);
