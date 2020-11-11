#ifndef L2TP_H_
#define L2TP_H_

#include <stdint.h>

struct l2tp_lac;

enum l2tp_state {
	L2TP_NONE,
	L2TP_HANDSHAKE,
	L2TP_ESTABLISHED,
	L2TP_RTX,
	L2TP_SHUTDOWN,
};

typedef void (l2tp_cb)(int handle, enum l2tp_state state, uint32_t code,
															void *ctx);

struct l2tp_cfg {
	uint16_t start_max_retry;
	uint16_t start_rtx_delay;
	uint16_t max_retry;
	uint16_t rtx_delay;
	uint16_t hello_delay;
	l2tp_cb *cb;
	void *cb_ctx;
};

struct l2tp_lac* l2tp_create(const char *host, const struct l2tp_cfg *cfg);
enum l2tp_state l2tp_state(struct l2tp_lac *lac);
void l2tp_destroy(struct l2tp_lac *lac);
int l2tp_shutdown(struct l2tp_lac *lac, int handle, uint32_t code);
int l2tp_session(struct l2tp_lac *lac);

#endif /* L2TP_H_ */
