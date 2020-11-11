/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <unl.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#endif

#include "lib/insmod.h"
#include "lib/config.h"
#include "lib/qos.h"

#include "hotspotd.h"
#include "routing.h"
#include "firewall.h"
#include "traffic.h"

static struct unl *unl = NULL;
static const uint32_t cl_major = 1 << 16;
static int overall_in, overall_out;
static uint32_t ratio_inter, ratio_small, ratio_bulk;
static int cfg_shaping = 0;

static int traffic_apply() {
	ratio_inter = config_get_int("traffic", "ratio_inter", 60);
	ratio_small = config_get_int("traffic", "ratio_small", 30);
	ratio_bulk = config_get_int("traffic", "ratio_bulk", 10);

	overall_in = config_get_int("traffic", "overall_kbit_in", 1000000) * 125;
	overall_out = config_get_int("traffic", "overall_kbit_out", 1000000) * 125;

	if (overall_in == 0)
		overall_in = 1000000 * 125;

	if (overall_out == 0)
		overall_out = 1000000 * 125;

	cfg_shaping = !strcasecmp(config_get_string("traffic", "shaping",
								"no"), "yes");

	// Default classes
	struct qos_hfsc crv = {.share = {.m2 = 0}};

	// root class 1:1 for overall traffic limits
	crv.share.m2 = crv.limit.m2 = overall_in;
	qos_class(unl, routing_cfg.ifb_index,
			cl_major, cl_major | 1, &crv);

	crv.share.m2 = crv.limit.m2 = overall_out;
	qos_class(unl, routing_cfg.ifb2_index,
			cl_major, cl_major | 1, &crv);

	// default class 1:2
	if (qos_class(unl, routing_cfg.ifb_index, cl_major | 1, cl_major | 2, &crv)
	|| qos_class(unl, routing_cfg.ifb2_index, cl_major | 1, cl_major | 2, &crv)) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to setup hfsc base classes: %s",
				strerror(errno));
#else
		syslog(LOG_ERR, "Failed to setup hfsc base classes: %s",
				strerror(errno));
#endif
		return -1;
	}

	return 0;
}

static int traffic_init() {
	if (!(unl = unl_open(NETLINK_ROUTE, NULL)))
		return -1;

	unl_timeout(unl, 1000);

#ifndef __SC_BUILD__
	insmod("sch_hfsc", "");
	insmod("cls_basic", "");
	insmod("cls_fw", "");
#endif

	// Drop qdiscs
	qos_qdisc(unl, routing_cfg.ifb2_index, 0, 0, 0);
	qos_qdisc(unl, routing_cfg.ifb_index, 0, 0, 0);

	// Disable ingress redirect
	if (strncmp(routing_cfg.ifb_name, "imq", 3)) {
#ifndef __SC_BUILD__
		insmod("sch_ingress", "");
		insmod("act_mirred", "");
#endif
		insmod("act_connmark", "");
#ifdef __SC_BUILD__
		qos_ingress(unl, routing_cfg.wifi_iface_index, 0, 0);
		qos_ingress(unl, routing_cfg.wifi_iface_index, 0, cl_major);
		if (routing_cfg.wifi_iface_index != routing_cfg.wifi_eap_iface_index) {
			qos_ingress(unl, routing_cfg.wifi_eap_iface_index,
					0, 0);
			qos_ingress(unl, routing_cfg.wifi_eap_iface_index,
					0, cl_major);
		}
#else
		qos_ingress(unl, routing_cfg.iface_index, 0, 0);
		qos_ingress(unl, routing_cfg.iface_index, 0, cl_major);
		if (routing_cfg.iface_index != routing_cfg.iface_eap_index) {
			qos_ingress(unl, routing_cfg.iface_eap_index,
					0, 0);
			qos_ingress(unl, routing_cfg.iface_eap_index,
					0, cl_major);
		}
#endif
	}

	// Create root qdisc 1: with default class 1:2
	if (qos_qdisc(unl, routing_cfg.ifb2_index, 0, cl_major, 2)
	|| (qos_qdisc(unl, routing_cfg.ifb_index, 0, cl_major, 2))) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to setup hfsc qdiscs: %s",
							strerror(errno));
#else
		syslog(LOG_ERR, "Failed to setup hfsc qdiscs: %s",
							strerror(errno));
#endif
		return -1;
	}

	if (!strncmp(routing_cfg.ifb_name, "imq", 3))
		goto out;

	// Redirect ingress traffic to ifb
#ifdef __SC_BUILD__
	if (qos_ingress(unl, routing_cfg.wifi_iface_index, routing_cfg.ifb_index, 0)
	|| qos_ingress(unl, routing_cfg.wifi_iface_index, routing_cfg.ifb2_index, cl_major)) {
#else
	if (qos_ingress(unl, routing_cfg.iface_index, routing_cfg.ifb_index, 0)
	|| qos_ingress(unl, routing_cfg.iface_index, routing_cfg.ifb2_index, cl_major)) {
#endif
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "traffic: ingress redirect setup failed: %s. "
		"Is the act_connmark kernel module present?", strerror(errno));
#else

		syslog(LOG_ERR, "traffic: ingress redirect setup failed: %s. "
		"Is the act_connmark kernel module present?", strerror(errno));
#endif
		return -1;
	}

#ifdef __SC_BUILD__
	if (!routing_cfg.wifi_eap_iface_index ||
			routing_cfg.wifi_eap_iface_index == routing_cfg.wifi_iface_index)
		goto out;

	if (qos_ingress(unl, routing_cfg.wifi_eap_iface_index, routing_cfg.ifb_index, 0)
	|| qos_ingress(unl, routing_cfg.wifi_eap_iface_index, routing_cfg.ifb2_index, cl_major)) {
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "traffic: ingress redirect setup failed: %s. "
		"Is the act_connmark kernel module present?", strerror(errno));
		return -1;
	}
#else
	if (!routing_cfg.iface_eap_index ||
			routing_cfg.iface_eap_index == routing_cfg.iface_index)
		goto out;

	if (qos_ingress(unl, routing_cfg.iface_eap_index, routing_cfg.ifb_index, 0)
	|| qos_ingress(unl, routing_cfg.iface_eap_index, routing_cfg.ifb2_index, cl_major)) {
		syslog(LOG_ERR, "traffic: ingress redirect setup failed: %s. "
		"Is the act_connmark kernel module present?", strerror(errno));
		return -1;
	}
#endif

out:
	return traffic_apply();
}

static void traffic_deinit() {
	if (unl) {
		// Disable ingress redirect
		if (strncmp(routing_cfg.ifb_name, "imq", 3)) {
#ifdef __SC_BUILD__
			qos_ingress(unl, routing_cfg.wifi_iface_index, 0, 0);
			qos_ingress(unl, routing_cfg.wifi_iface_index, 0, cl_major);
			qos_ingress(unl, routing_cfg.wifi_eap_iface_index, 0, 0);
			qos_ingress(unl, routing_cfg.wifi_eap_iface_index, 0, cl_major);
#else
			qos_ingress(unl, routing_cfg.iface_index, 0, 0);
			qos_ingress(unl, routing_cfg.iface_index, 0, cl_major);
			qos_ingress(unl, routing_cfg.iface_eap_index, 0, 0);
			qos_ingress(unl, routing_cfg.iface_eap_index, 0, cl_major);
#endif
		}

		// Drop qdiscs
		qos_qdisc(unl, routing_cfg.ifb2_index, 0, 0, 0);
		qos_qdisc(unl, routing_cfg.ifb_index, 0, 0, 0);

		unl_close(unl);
		unl = NULL;
	}
}

int traffic_create(uint16_t classid) {
	uint32_t cl = cl_major | (classid << 4);
	uint32_t mark = firewall_id_to_authmark(classid);
	struct qos_hfsc crv = {.share = {.m2 = 0}};

	// Create user main class
	crv.share.m2 = overall_out;
	int s1 = qos_class(unl, routing_cfg.ifb2_index, cl_major | 1, cl, &crv);

	crv.share.m2 = overall_in;
	int s2 = qos_class(unl, routing_cfg.ifb_index, cl_major | 1, cl, &crv);

	if (s1 || s2) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to setup hfsc user classes: %s",
							strerror(errno));
#else
		syslog(LOG_ERR, "Failed to setup hfsc user classes: %s",
							strerror(errno));
#endif
		traffic_destroy(classid);
		return -1;
	}

	// Create user filter rule
	s1 = qos_filter_mark(unl, routing_cfg.ifb2_index, classid, cl_major,
						mark, FIREWALL_AUTHMASK, cl);
	s2 = qos_filter_mark(unl, routing_cfg.ifb_index, classid, cl_major,
						mark, FIREWALL_AUTHMASK, cl);

	if (s1 || s2) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Failed to setup tc fw filters: %s",
							strerror(errno));
#else
		syslog(LOG_ERR, "Failed to setup tc fw filters: %s",
							strerror(errno));
#endif
		traffic_destroy(classid);
		return -1;
	}

	if (!cfg_shaping)
		return 0;

	// Create per user classes
	crv.share.m2 = ratio_inter * overall_out / 100;
	crv.limit.m2 = overall_out;
	qos_class(unl, routing_cfg.ifb2_index, cl, cl + 1, &crv);
	crv.share.m2 = ratio_inter * overall_in / 100;
	crv.limit.m2 = overall_in;
	qos_class(unl, routing_cfg.ifb_index, cl, cl + 1, &crv);

	crv.share.m2 = ratio_small * overall_out / 100;
	crv.limit.m2 = overall_out;
	qos_class(unl, routing_cfg.ifb2_index, cl, cl + 2, &crv);
	crv.share.m2 = ratio_small * overall_in / 100;
	crv.limit.m2 = overall_in;
	qos_class(unl, routing_cfg.ifb_index, cl, cl + 2, &crv);

	crv.share.m2 = ratio_bulk * overall_out / 100;
	crv.limit.m2 = overall_out;
	qos_class(unl, routing_cfg.ifb2_index, cl, cl + 3, &crv);
	crv.share.m2 = ratio_bulk * overall_in / 100;
	crv.limit.m2 = overall_in;
	qos_class(unl, routing_cfg.ifb_index, cl, cl + 3, &crv);

	// Create per-user traffic shaping filters
	qos_filter_mark(unl, routing_cfg.ifb2_index, 1, cl,
			FIREWALL_CLS_INTER, FIREWALL_CLASSMASK, cl + 1);
	qos_filter_mark(unl, routing_cfg.ifb_index, 1, cl,
			FIREWALL_CLS_INTER, FIREWALL_CLASSMASK, cl + 1);
	qos_filter_mark(unl, routing_cfg.ifb2_index, 2, cl,
			FIREWALL_CLS_SMALL, FIREWALL_CLASSMASK, cl + 2);
	qos_filter_mark(unl, routing_cfg.ifb_index, 2, cl,
			FIREWALL_CLS_SMALL, FIREWALL_CLASSMASK, cl + 2);
	qos_filter_basic(unl, routing_cfg.ifb2_index, 3, cl, cl + 3);
	qos_filter_basic(unl, routing_cfg.ifb_index, 3, cl, cl + 3);

	return 0;
}

int traffic_stats(uint16_t classid,
uint64_t *byte_in, uint64_t *byte_out, uint32_t *pkt_in, uint32_t *pkt_out) {
	if (!unl)
		return -1;

	uint32_t cl = (classid) ? cl_major | (classid << 4) : cl_major | 1;

	return qos_stat(unl, routing_cfg.ifb_index, cl, byte_out, pkt_out) |
	qos_stat(unl, routing_cfg.ifb2_index, cl, byte_in, pkt_in);
}

void traffic_destroy(uint16_t classid) {
	// Delete classes
	uint32_t cl = cl_major | (classid << 4);

	// Delete per-user traffic shaping filters
	qos_filter_mark(unl, routing_cfg.ifb2_index, 1, cl, 
		FIREWALL_CLS_INTER, FIREWALL_CLASSMASK, 0);
	qos_filter_mark(unl, routing_cfg.ifb_index, 1, cl, 
		FIREWALL_CLS_INTER, FIREWALL_CLASSMASK, 0);
	qos_filter_mark(unl, routing_cfg.ifb2_index, 2, cl, 
		FIREWALL_CLS_SMALL, FIREWALL_CLASSMASK, 0);
	qos_filter_mark(unl, routing_cfg.ifb_index, 2, cl, 
		FIREWALL_CLS_SMALL, FIREWALL_CLASSMASK, 0);
	qos_filter_basic(unl, routing_cfg.ifb2_index, 3, cl, 0);
	qos_filter_basic(unl, routing_cfg.ifb_index, 3, cl, 0);


	// Delete filters
	qos_filter_mark(unl, routing_cfg.ifb2_index, classid, cl_major, 0, 0, 0);
	qos_filter_mark(unl, routing_cfg.ifb_index, classid, cl_major, 0, 0, 0);

	// Delete classes
	for (int i = 3; i >= 1; --i) {
		qos_class(unl, routing_cfg.ifb2_index, cl, cl + i, NULL);
		qos_class(unl, routing_cfg.ifb_index, cl, cl + i, NULL);
	}

	qos_class(unl, routing_cfg.ifb2_index, cl_major | 1, cl, NULL);
	qos_class(unl, routing_cfg.ifb_index, cl_major | 1, cl, NULL);
}

MODULE_REGISTER(traffic, 230)

