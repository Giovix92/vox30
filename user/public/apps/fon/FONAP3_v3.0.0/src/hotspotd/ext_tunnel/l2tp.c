/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <byteswap.h>
#include <netdb.h>
#include <unistd.h>
#include <syslog.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/types.h>
#include <linux/if_pppox.h>
#include <sys/types.h>

#include "core/hotspotd.h"
#include "lib/list.h"
#include "lib/event.h"
#include "lib/urandom.h"
#include "l2tp.h"

#ifdef __UCLIBC__
#include <features.h>
#if __UCLIBC_MAJOR__ == 0 && __UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ <= 29
#define res_init __res_init
extern int __res_init();
#endif
#endif

#ifndef PX_PROTO_OL2TP
#define PX_PROTO_OL2TP 1

// linux/if_pppo2ltp.h
struct pppol2tp_addr {
        __kernel_pid_t  pid;            /* pid that owns the fd.
                                         * 0 => current */
        int     fd;                     /* FD of UDP socket to use */

        struct sockaddr_in addr;        /* IP address and port to send to */

        __u16 s_tunnel, s_session;      /* For matching incoming packets */
        __u16 d_tunnel, d_session;      /* For sending outgoing packets */
};

// linux/if_pppox.h
struct sockaddr_pppol2tp {
        sa_family_t     sa_family;      /* address family, AF_PPPOX */
        unsigned int    sa_protocol;    /* protocol identifier */
        struct pppol2tp_addr pppol2tp;
} __attribute__((packed));

#endif

struct l2tp_hdr {
	uint16_t flags;
	uint16_t length;
	uint16_t t_id;
	uint16_t s_id;
	uint16_t ns;
	uint16_t nr;
	uint8_t payload[];
};

struct l2tp_avp_hdr {
	uint16_t flags;
	uint16_t vendor;
	uint16_t type;
};

struct l2tp_msg_avp {
	uint16_t flags;
	uint32_t type;
	uint8_t *payload;
	size_t len;
};

enum l2tp_msgtype {
	L2TP_SCCRQ = 1,
	L2TP_SCCRP = 2,
	L2TP_SCCCN = 3,
	L2TP_STOPCCN = 4,
	L2TP_HELLO = 6,
	L2TP_OCRQ = 7,
	L2TP_ICRQ = 10,
	L2TP_ICRP = 11,
	L2TP_ICCN = 12,
	L2TP_CDN = 14,
	L2TP_SLI = 16,
};

enum avp_types {
	AVP_MSG_TYPE = 0,
	AVP_RESULT = 1,
	AVP_VERSION = 2,
	AVP_FRAMING_CAPS = 3,
	AVP_BEARER_CAPS = 4,
	AVP_TIE_BREAKER = 5,
	AVP_FIRMWARE_REV = 6,
	AVP_VENDOR = 8,
	AVP_HOSTNAME = 7,
	AVP_TUNNEL_ID = 9,
	AVP_RWIN = 10,
	AVP_SESSION_ID = 14,
	AVP_CALL_SERIAL = 15,
	AVP_BEARER_TYPE = 18,
	AVP_FRAMING_TYPE = 19,
	AVP_CALLED_NUMBER = 21,
	AVP_CALLING_NUMBER = 22,
	AVP_TX_SPEED = 24,
};


#define L2TP_HEADER_MAGIC	0xc802
#define L2TP_HEADER_MASK	0xcb0f
#define L2TP_FLAG_MANDATORY	0x8000
#define L2TP_VERSION		0x0100
#define L2TP_FRAMING_SYNC	0x1
#define L2TP_RWIN_DEFAULT	4

struct l2tp_request {
	struct list_head head;
	int32_t timeout;
	unsigned try;
	uint8_t msg[];
};

struct l2tp_lac {
	struct event_epoll lns;
	struct event_timer timer;
	struct l2tp_cfg cfg;
	struct list_head queue;
	enum l2tp_state state;
	int fd_tunnel;
	uint16_t rwin;
	uint16_t queued;
	uint16_t nr;
	uint16_t ns;
	uint16_t tid_local;
	uint16_t tid_remote;
	uint32_t serial;
	uint8_t sessions[HOTSPOT_LIMIT_RES];
	int32_t lasthello;
	union {
		struct sockaddr saddr;
		struct sockaddr_in6 saddr_in6;
	} addr;
};

union l2tp_sockaddr {
	struct sockaddr_in ipv4;
	struct sockaddr_in6 ipv6;
};

static struct l2tp_hdr* l2tp_msg_init(void *buffer);
static void l2tp_msg_avp(struct l2tp_hdr *msg,
uint32_t type, const void *payload, size_t size);
static void l2tp_msg_avp_u16(struct l2tp_hdr *msg,
uint32_t type, uint16_t val);
static void l2tp_msg_avp_u32(struct l2tp_hdr *msg,
uint32_t type, uint32_t val);
static void l2tp_msg_avp_string(struct l2tp_hdr *msg,
uint32_t type, const char *val);
static int l2tp_msg_avp_next(struct l2tp_hdr *msg, struct l2tp_msg_avp *avp);
static int l2tp_queue(struct l2tp_lac *lac, struct l2tp_hdr *msg);
static void l2tp_receive(struct event_epoll *event, uint32_t revents);
static void l2tp_ack(struct l2tp_lac *lac);
static uint16_t l2tp_get_u16(uint8_t *val);
static void l2tp_handle(struct l2tp_lac *lac, struct l2tp_hdr *msg,
						union l2tp_sockaddr *addr);
static int l2tp_icrq(struct l2tp_lac *lac, uint16_t sid);
static void l2tp_teardown(struct l2tp_lac *lac, uint32_t code);
static void l2tp_dequeue(struct l2tp_lac *lac, uint16_t lastid);
static void l2tp_timer(struct event_timer *timer, int64_t now);
static int l2tp_transmit(struct l2tp_lac *lac);

// Notify the external control about status changes
static inline void l2tp_notify(struct l2tp_lac *lac, int handle,
					enum l2tp_state msg, uint32_t code) {
	lac->cfg.cb(handle, msg, code, lac->cfg.cb_ctx);
}


// Create a new LAC and initiate a connection
struct l2tp_lac* l2tp_create(const char *host, const struct l2tp_cfg *cfg) {
	struct l2tp_lac *lac = calloc(1, sizeof(*lac));
	if (!lac)
		return NULL;

	memcpy(&lac->cfg, cfg, sizeof(lac->cfg));

	urandom(&lac->tid_local, sizeof(lac->tid_local));
	urandom(&lac->serial, sizeof(lac->serial));
	memset(lac->sessions, 0, sizeof(lac->sessions));
	INIT_LIST_HEAD(&lac->queue);
	lac->rwin = L2TP_RWIN_DEFAULT;

	char hostname[HOST_NAME_MAX] = ".";
	gethostname(hostname, sizeof(hostname));

	// Construct the start control connection request
	uint8_t buffer[384];
	struct l2tp_hdr *msg = l2tp_msg_init(buffer);
	l2tp_msg_avp_u16(msg, AVP_MSG_TYPE, L2TP_SCCRQ);
	l2tp_msg_avp_u16(msg, AVP_VERSION, L2TP_VERSION);
	l2tp_msg_avp_string(msg, AVP_HOSTNAME, hostname);
	l2tp_msg_avp_u32(msg, AVP_FRAMING_CAPS, L2TP_FRAMING_SYNC);
	l2tp_msg_avp_u16(msg, AVP_TUNNEL_ID, lac->tid_local);

	// DNS lookup
	struct addrinfo *result, *rp;
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_DGRAM,
		.ai_flags = AI_ADDRCONFIG
	};

	// (older) uclibc versions do not handle changes in resolv.conf correctly
#ifdef __UCLIBC__
#if __UCLIBC_MAJOR__ == 0 && __UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ <= 29
	res_init();
#endif
#endif

	int st = getaddrinfo(host, "1701", &hints, &result);
	if (st) {
#ifdef __SC_BUILD__
       log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "getaddrinfo() failed: %s\n",
						gai_strerror(st));
#else

		syslog(LOG_WARNING, "getaddrinfo() failed: %s",
						gai_strerror(st));
#endif
		goto err;
	}

	if ((lac->fd_tunnel = socket(AF_PPPOX, SOCK_DGRAM, PX_PROTO_OL2TP)) < 0) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unable to open PPPoX socket, try modprobe l2tp_ppp / pppol2tp?");
#else
        syslog(LOG_WARNING, "Unable to open PPPoX socket, "
					"try modprobe l2tp_ppp / pppol2tp?");
#endif
		goto err;
	}

	// Setup retransmit timer
	lac->timer.interval = (lac->cfg.hello_delay / 2) * 1000;
	if (lac->timer.interval < 1000)
		lac->timer.interval = 1000;
	lac->timer.value = 0;
	lac->timer.context = lac;
	lac->timer.handler = l2tp_timer;
	event_ctl(EVENT_TIMER_ADD, &lac->timer);

	// Try all resolved IPs for one we have a route for
	for (lac->lns.fd = -1, rp = result; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_addrlen > sizeof(lac->addr))
			continue;

		lac->lns.fd = socket(rp->ai_family, rp->ai_socktype,
							rp->ai_protocol);
		memcpy(&lac->addr.saddr, rp->ai_addr, rp->ai_addrlen);
		lac->state = L2TP_HANDSHAKE;
		if (!l2tp_queue(lac, msg))
			break;

		lac->state = L2TP_NONE;
		close(lac->lns.fd);
		lac->lns.fd = -1;
		lac->ns = 0;
		lac->addr.saddr.sa_family = AF_UNSPEC;
	}

	freeaddrinfo(result);

	if (lac->lns.fd < 0) {
		event_ctl(EVENT_TIMER_DEL, &lac->timer);
		goto err;
	}

	event_cloexec(lac->lns.fd);
	lac->lns.events = EPOLLET | EPOLLIN;
	lac->lns.context = lac;
	lac->lns.handler = l2tp_receive;
	event_ctl(EVENT_EPOLL_ADD, &lac->lns);

	return lac;

err:
	free(lac);
	return NULL;
}

enum l2tp_state l2tp_state(struct l2tp_lac *lac) {
	return (lac) ? lac->state : L2TP_NONE;
}


// Destroy the LAC and free all resources
void l2tp_destroy(struct l2tp_lac *lac) {
	if (lac) {
		l2tp_shutdown(lac, 0, 6);
		close(lac->lns.fd);
		close(lac->fd_tunnel);
		event_ctl(EVENT_TIMER_DEL, &lac->timer);
		l2tp_dequeue(lac, lac->ns);
		free(lac);
	}
}

// Shutdown a specific session or all sessions and the control (handle 0)
int l2tp_shutdown(struct l2tp_lac *lac, int handle, uint32_t code) {
	if (lac->state == L2TP_SHUTDOWN) {
		errno = ENOTCONN;
		return -1;
	}

	if (handle && (handle >= HOTSPOT_LIMIT_RES || !lac->sessions[handle])) {
		errno = ENOENT;
		return -1;
	}

	uint8_t buffer[64];
	struct l2tp_hdr *msg = l2tp_msg_init(buffer);
	l2tp_msg_avp_u16(msg, AVP_MSG_TYPE, (handle) ? L2TP_CDN : L2TP_STOPCCN);
	l2tp_msg_avp_u32(msg, AVP_RESULT, bswap_32(code));

	if (handle == 0) {
		l2tp_msg_avp_u16(msg, AVP_TUNNEL_ID, lac->tid_local);
		l2tp_queue(lac, msg);
		l2tp_teardown(lac, code);
	} else {
		// If we are not yet established we haven't requested the session
		if (lac->state == L2TP_ESTABLISHED) {
			struct sockaddr_pppol2tp sa = {0};
			socklen_t salen = sizeof(sa);
			getpeername(handle, (void*)&sa, &salen);

			msg->s_id = htons(sa.pppol2tp.d_session);
			l2tp_msg_avp_u16(msg, AVP_SESSION_ID, handle);
			l2tp_queue(lac, msg);
		}

		lac->sessions[handle] = 0;
		l2tp_notify(lac, handle, L2TP_SHUTDOWN, code);
		close(handle);
	}

	return 0;
}

// Open a new L2TP session and return the PPPoL2TP fd
int l2tp_session(struct l2tp_lac *lac) {
	int fd = -1;
	if (lac->state == L2TP_SHUTDOWN) {
		errno = ENOTCONN;
		goto err;
	}

	if ((fd = socket(AF_PPPOX, SOCK_DGRAM, PX_PROTO_OL2TP)) <= 0)
		goto err;

	event_cloexec(fd);

	if (lac->state == L2TP_ESTABLISHED && l2tp_icrq(lac, fd))
		goto err;

	lac->sessions[fd] = 1;
	return fd;

err:
	if (fd >= 0)
		close(fd);
	return -1;
}

// Send new call request
static int l2tp_icrq(struct l2tp_lac *lac, uint16_t sid) {
	uint8_t buffer[64];
	struct l2tp_hdr *msg = l2tp_msg_init(buffer);
	l2tp_msg_avp_u16(msg, AVP_MSG_TYPE, L2TP_ICRQ);
	l2tp_msg_avp_u16(msg, AVP_SESSION_ID, sid);
	l2tp_msg_avp_u32(msg, AVP_CALL_SERIAL, lac->serial++);
	return l2tp_queue(lac, msg);
}

// Timer (schedule hellos and retransmit messages)
static void l2tp_timer(struct event_timer *timer, int64_t now) {
	struct l2tp_lac *lac = timer->context;
	int32_t nowsecs = now / 1000;

	if (lac->state == L2TP_ESTABLISHED && lac->cfg.hello_delay
	&& lac->lasthello + lac->cfg.hello_delay <= nowsecs) {
		uint8_t buffer[32];
		struct l2tp_hdr *msg = l2tp_msg_init(buffer);
		l2tp_msg_avp_u16(msg, AVP_MSG_TYPE, L2TP_HELLO);
		lac->lasthello = nowsecs;
		l2tp_queue(lac, msg); // implicit l2tp_transmit
	} else {
		l2tp_transmit(lac);
	}
}

// Handle transmit queue
static int l2tp_transmit(struct l2tp_lac *lac) {
	int32_t now = event_time() / 1000;
	int32_t next = (lac->lasthello + lac->cfg.hello_delay > now)
			? lac->lasthello + lac->cfg.hello_delay : INT32_MAX;

	int txed = 0;
	struct l2tp_request *req;

	list_for_each_entry(req, &lac->queue, head) {
		// Don't have more messages pending than peer can queue
		if (++txed > lac->rwin)
			break;

		if (req->timeout > now) {
			if (req->timeout < next)
				next = req->timeout;
			continue;
		}

		int max_retry = (lac->state == L2TP_HANDSHAKE)
			? lac->cfg.start_max_retry : lac->cfg.max_retry;
		int rtx_delay = (lac->state == L2TP_HANDSHAKE)
			? lac->cfg.start_rtx_delay : lac->cfg.rtx_delay;

		if (req->try > max_retry) {
			errno = ETIMEDOUT;
			goto err;
		}
		
		// Old implementation: exponential growth of delay
		//req->timeout = now + (rtx_delay << req->try++);
		// New specs: fixed delay value
		req->try++;
		req->timeout = now + rtx_delay;

		struct l2tp_hdr *msg = (void*)req->msg;
		if (req->try == 1) { // Update the ns only at FIRST try
			msg->ns = htons(lac->ns++);
			if (lac->state != L2TP_HANDSHAKE) {
				lac->state = L2TP_RTX;
			}
		}

		msg->nr = htons(lac->nr);

		if (sendto(lac->lns.fd, msg, ntohs(msg->length),
		MSG_DONTWAIT, &lac->addr.saddr, sizeof(lac->addr)) < 0)
			goto err;

		if (req->timeout < next)
			next = req->timeout;
	}

	lac->timer.value = (next - now) * 1000;
	event_ctl(EVENT_TIMER_MOD, &lac->timer);
	return 0;

err:
	if (lac->state != L2TP_NONE)
		l2tp_teardown(lac, ETIMEDOUT << 16);
	return -1;
}

// Queue a new message
static int l2tp_queue(struct l2tp_lac *lac, struct l2tp_hdr *msg) {
	struct l2tp_request *req =
		malloc(sizeof(struct l2tp_request) + msg->length);
	if (!req)
		return -1;

	req->timeout = 0;
	req->try = 0;
	msg = memcpy(req->msg, msg, msg->length);

	msg->t_id = htons(lac->tid_remote);
	msg->length = htons(msg->length);

	// Enqueue
	list_add_tail(&req->head, &lac->queue);
	lac->queued++;

	return l2tp_transmit(lac);
}

// Remove requests from retransmit queue that have been acked by the LNS
static void l2tp_dequeue(struct l2tp_lac *lac, uint16_t lastid) {
	for (int stop = 0; !stop && !list_empty(&lac->queue);) {
		struct l2tp_request *req;
		req = list_first_entry(&lac->queue, struct l2tp_request, head);
		struct l2tp_hdr *msg = (void*)req->msg;
		stop = (ntohs(msg->ns) == lastid);
		list_del(&req->head);
		free(req);
		lac->queued--;
	}
}

// Receive control messages
static void l2tp_receive(struct event_epoll *event, uint32_t revents) {
	union l2tp_sockaddr addr;
	uint8_t buffer[1500];
	struct l2tp_lac *lac = event->context;
	struct l2tp_hdr *msg = (void*)buffer;

	for (;;) {
		socklen_t addrlen = sizeof(addr);
		ssize_t len = recvfrom(event->fd, buffer, sizeof(buffer),
			MSG_DONTWAIT, (void*)&addr, &addrlen);

		if (len < 0)
			break;

		if (len < sizeof(struct l2tp_hdr)
		|| len < ntohs(msg->length)
		|| ntohs(msg->t_id) != lac->tid_local
		|| !(msg->flags & htons(0x8000))) // is not a control message
			continue;

		int ns = ntohs(msg->ns);
		// We normalize ns to 32768, to correctly classify messages
		int normalized_ns = ((ns + (32768 - (int)lac->nr)) & 0xffff);
		if (normalized_ns > 32768) {
			// Message id too high, wait for retransmission
			continue;
		} else if (normalized_ns < 32768) {
			// Message already handled, send ACK
			l2tp_ack(lac);
			continue;
		}

		// Remove ACKed messages from retransmit queue
		uint16_t nr = ntohs(msg->nr);
		l2tp_dequeue(lac, nr - 1);

		if(lac->state == L2TP_RTX) {
			lac->state = L2TP_ESTABLISHED;
		}

		// Already shutdown or ZLB
		if (lac->state == L2TP_SHUTDOWN
		|| len == sizeof(struct l2tp_hdr))
			continue;

		l2tp_handle(lac, msg, &addr);
	}

	// Update hello timer as we know the peer is alive
	lac->lasthello = event_time() / 1000;
}

// Handle incoming control messages
static void l2tp_handle
(struct l2tp_lac *lac, struct l2tp_hdr *msg, union l2tp_sockaddr *addr) {
	lac->nr++; // Increase expected message number

	struct l2tp_msg_avp avp = {.payload = NULL};
	// Read message type AVP
	if (l2tp_msg_avp_next(msg, &avp) || avp.type != AVP_MSG_TYPE ||
								avp.len != 2)
		goto err;

	uint16_t type = l2tp_get_u16(avp.payload);
	if (type == L2TP_STOPCCN) { // We can get this message in every state
		uint32_t code = 1;
		while (!l2tp_msg_avp_next(msg, &avp)) {
			if (avp.type != AVP_RESULT)
				continue;

			if (avp.len >= 2)
				code = l2tp_get_u16(avp.payload);

			if (avp.len >= 4)
				code |= l2tp_get_u16(&avp.payload[2]) << 16;

			break;
		}

		// First reply might come from different address
		if (lac->state == L2TP_HANDSHAKE)
			memcpy(&lac->addr, addr, sizeof(lac->addr));

		l2tp_ack(lac);

		if ((code & 0xffff) == 0) // Protect reserved 0 error namespace
			code = 1;

		l2tp_teardown(lac, code);
	} else if (type == L2TP_SLI || type == L2TP_HELLO) {
		// We only support synchronous connections as we send over UDP
		// so we ignore SLI (PPP escaping options)
		// also if its HELLO simply ACK it
		l2tp_ack(lac);
	} else if (type == L2TP_SCCRP && lac->state == L2TP_HANDSHAKE) {
		while (!l2tp_msg_avp_next(msg, &avp)) {
			switch (avp.type) {
			case AVP_VERSION:
			case AVP_VENDOR:
			case AVP_TIE_BREAKER:
			case AVP_BEARER_CAPS:
			case AVP_FIRMWARE_REV:
			case AVP_HOSTNAME:
			case AVP_FRAMING_CAPS:
				break;

			case AVP_TUNNEL_ID:
				if (avp.len != sizeof(uint16_t))
					goto errattr;
				lac->tid_remote = l2tp_get_u16(avp.payload);
				break;

			case AVP_RWIN:
				if (avp.len != sizeof(uint16_t) || (lac->rwin =
				l2tp_get_u16(avp.payload)) < L2TP_RWIN_DEFAULT)
					goto errattr;
				break;

			default:
				if (avp.flags & L2TP_FLAG_MANDATORY)
					goto errattr;
				break;
			}
		}

		// Finally connect() as peer address is now known
		connect(lac->lns.fd, (void*)addr,
				(addr->ipv4.sin_family == AF_INET)
				? sizeof(addr->ipv4) : sizeof(addr->ipv6));

		struct sockaddr_pppol2tp xaddr = {
			.sa_family = AF_PPPOX,
			.sa_protocol = PX_PROTO_OL2TP,
			.pppol2tp = {
				.fd = lac->lns.fd,
				.s_tunnel = lac->tid_local,
				.d_tunnel = lac->tid_remote,
			},
		};
		connect(lac->fd_tunnel, (void*)&xaddr, sizeof(xaddr));

		memcpy(&lac->addr, addr, sizeof(lac->addr));
		lac->state = L2TP_ESTABLISHED;

		uint8_t buffer[64];
		struct l2tp_hdr *rmsg = l2tp_msg_init(buffer);
		l2tp_msg_avp_u16(rmsg, AVP_MSG_TYPE, L2TP_SCCCN);
		l2tp_queue(lac, rmsg);

		// Check for pending tunnel requests and submit them
		for (int i = 1; i < HOTSPOT_LIMIT_RES; ++i)
			if (lac->sessions[i])
				l2tp_icrq(lac, i);

		l2tp_notify(lac, 0, L2TP_ESTABLISHED, 0);
	} else if (type == L2TP_ICRP && lac->state == L2TP_ESTABLISHED) {
		uint16_t sid = 0;
		while (!l2tp_msg_avp_next(msg, &avp)) {
			if (avp.type != AVP_SESSION_ID)
				continue;

			if (avp.len == sizeof(uint16_t))
				sid = l2tp_get_u16(avp.payload);

			break;
		}

		if (!sid) // No session ID
			goto err;

		int handle = ntohs(msg->s_id);
		if (handle >= HOTSPOT_LIMIT_RES || !lac->sessions[handle]) {
			l2tp_ack(lac); // Unknown session (might be a timing issue)
			return;
		}

		struct sockaddr_pppol2tp xaddr = {
			.sa_family = AF_PPPOX,
			.sa_protocol = PX_PROTO_OL2TP,
			.pppol2tp = {
				.fd = lac->lns.fd,
				.s_tunnel = lac->tid_local,
				.d_tunnel = lac->tid_remote,
				.s_session = handle,
				.d_session = sid,
			},
		};
		if (connect(handle, (void*)&xaddr, sizeof(xaddr))) {
			l2tp_shutdown(lac, handle, 1); // FAIL?!
			return;
		}

		uint8_t buffer[64];
		struct l2tp_hdr *rmsg = l2tp_msg_init(buffer);
		rmsg->s_id = htons(sid);
		l2tp_msg_avp_u16(rmsg, AVP_MSG_TYPE, L2TP_ICCN);
		l2tp_msg_avp_u32(rmsg, AVP_TX_SPEED, 10000000); // yeah, whatever...
		l2tp_msg_avp_u32(rmsg, AVP_FRAMING_TYPE, L2TP_FRAMING_SYNC);
		l2tp_queue(lac, rmsg);

		l2tp_notify(lac, handle, L2TP_ESTABLISHED, 0);
	} else if (type == L2TP_CDN && lac->state == L2TP_ESTABLISHED) {
		// Peer requested closing of a session
		uint32_t code = 3;
		while (!l2tp_msg_avp_next(msg, &avp)) {
			if (avp.type != AVP_RESULT)
				continue;

			if (avp.len >= 2)
				code = l2tp_get_u16(avp.payload);

			if (avp.len >= 4)
				code |= l2tp_get_u16(&avp.payload[2]) << 16;

			break;
		}

		if ((code & 0xffff) == 0) // Protect reserved 0 error namespace
			code = 3;

		l2tp_ack(lac);

		int handle = ntohs(msg->s_id);
		if (handle < HOTSPOT_LIMIT_RES && lac->sessions[handle]) {
			lac->sessions[handle] = 0;
			l2tp_notify(lac, handle, L2TP_SHUTDOWN, code);
			close(handle);
		}
	} else { // Unknown message
		goto err;
	}

	return;

err:
	l2tp_teardown(lac, 7); // Invalid state
	return;

errattr:
	l2tp_teardown(lac, 8 << 16 | 2); // Unknown mandatory attr
}

// Acknowledge packets received from the LNS
static void l2tp_ack(struct l2tp_lac *lac) {
	uint8_t buffer[16];
	struct l2tp_hdr *msg = l2tp_msg_init(buffer);
	msg->nr = htons(lac->nr);
	msg->ns = htons(lac->ns); // Don't increase ns for ZLB ACK
	msg->t_id = htons(lac->tid_remote);
	msg->length = htons(msg->length);

	// Send directly as we don't need to send them reliably
	sendto(lac->lns.fd, msg, sizeof(*msg), MSG_DONTWAIT,
		&lac->addr.saddr, sizeof(lac->addr));
}

// Close all session fds, set state to shutdown and notify controller
static void l2tp_teardown(struct l2tp_lac *lac, uint32_t code) {
	if (lac->state == L2TP_SHUTDOWN)
		return;

	for (int i = 1; i < HOTSPOT_LIMIT_RES; ++i)
		if (lac->sessions[i])
			close(i);

	memset(lac->sessions, 0, sizeof(lac->sessions));

	l2tp_dequeue(lac, lac->ns);

	lac->state = L2TP_SHUTDOWN;
	l2tp_notify(lac, 0, L2TP_SHUTDOWN, code);
}



/* Message parsing */

static int l2tp_msg_avp_next(struct l2tp_hdr *msg, struct l2tp_msg_avp *avp) {
	struct l2tp_avp_hdr hdr;
	size_t plen = ntohs(msg->length) - sizeof(*msg);
	uint8_t *next = avp->payload;

	if (!next) {
		next = msg->payload;
	} else {
		next += avp->len;
	}

	if (next + sizeof(hdr) - msg->payload > plen)
		return -1;

	memcpy(&hdr, next, sizeof(hdr));
	avp->flags = ntohs(hdr.flags) & 0xfc00;
	avp->type = (ntohs(hdr.vendor) << 16) | ntohs(hdr.type);
	avp->len = (ntohs(hdr.flags) & 0x3ff) - sizeof(struct l2tp_avp_hdr);
	avp->payload = next + sizeof(hdr);

	if (avp->payload + avp->len - msg->payload > plen)
		return -1;

	return 0;
}

static uint16_t l2tp_get_u16(uint8_t *val) {
	return val[0] << 8 | val[1];
}


/* Message construction */

static struct l2tp_hdr* l2tp_msg_init(void *buffer) {
	struct l2tp_hdr *msg = buffer;
	msg->flags = htons(L2TP_HEADER_MAGIC);
	msg->length = sizeof(*msg);
	msg->nr = msg->ns = msg->t_id = msg->s_id = 0;
	return msg;
}

static void l2tp_msg_avp
(struct l2tp_hdr *msg, uint32_t type, const void *payload, size_t size) {
	struct l2tp_avp_hdr hdr = {
		.flags = htons(L2TP_FLAG_MANDATORY | ((size + 6) & 0x3ff)),
		.vendor = htons(type >> 16),
		.type = htons(type & 0xffff),
	};
	memcpy(((uint8_t*)msg) + msg->length, &hdr, 6);
	memcpy(((uint8_t*)msg) + msg->length + 6, payload, size);
	msg->length += size + 6;
}

static void l2tp_msg_avp_u16
(struct l2tp_hdr *msg, uint32_t type, uint16_t val) {
	val = htons(val);
	l2tp_msg_avp(msg, type, &val, sizeof(val));
}

static void l2tp_msg_avp_u32
(struct l2tp_hdr *msg, uint32_t type, uint32_t val) {
	val = htonl(val);
	l2tp_msg_avp(msg, type, &val, sizeof(val));
}

static void l2tp_msg_avp_string
(struct l2tp_hdr *msg, uint32_t type, const char *val) {
	l2tp_msg_avp(msg, type, val, strlen(val));
}
