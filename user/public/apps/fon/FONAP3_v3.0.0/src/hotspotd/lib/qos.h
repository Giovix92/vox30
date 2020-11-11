#ifndef QOS_H_
#define QOS_H_

#include <linux/pkt_sched.h>

struct qos_hfsc {
	struct tc_service_curve realt;
	struct tc_service_curve share;
	struct tc_service_curve limit;
};

int qos_ingress(struct unl *unl, int srcif, int dstif, uint32_t ehandle);
int qos_stat(struct unl *unl, int iface, uint32_t handle, uint64_t *bytes, uint32_t *packets);
int qos_qdisc(struct unl *unl, int iface, uint32_t parent, uint32_t handle, uint16_t defclass);
int qos_class(struct unl *unl, int iface, uint32_t parent, uint32_t handle, struct qos_hfsc *curves);
int qos_filter_mark(struct unl *unl, int iface, uint16_t prio, uint32_t parent, uint32_t mark, uint32_t mask, uint32_t flow);
int qos_filter_basic(struct unl *unl, int iface, uint16_t prio, uint32_t parent, uint32_t flow);

#endif /* QOS_H_ */
