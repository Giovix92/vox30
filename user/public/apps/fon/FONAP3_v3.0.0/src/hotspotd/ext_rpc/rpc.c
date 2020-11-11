/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>

#include "lib/config.h"
#include "lib/usock.h"
#include "lib/event.h"
#include "libfonrpc/fonrpc.h"
#include "rpc/010-core.h"
#include "ext_rpc/rpc.h"
#include "core/hotspotd.h"
#ifdef __SC_BUILD__
#include "ext_tunnel/gre.h"
#endif
struct rpc_handle {
	struct sockaddr_un sa;
	socklen_t sa_len;
};

static void rpc_handle(struct event_epoll *event, uint32_t revents);
static struct list_head handlers = LIST_HEAD_INIT(handlers);
static struct event_epoll event_rpc = {
	.events = EPOLLIN | EPOLLET,
	.handler = rpc_handle,
	.fd = -1,
};

void rpc_register(struct rpc_handler *hndl) {
	while (hndl->type)
		list_add(&((hndl++)->_head), &handlers);
}

void rpc_send(struct rpc_handle *hndl, void *buffer, size_t len) {
	sendto(event_rpc.fd, buffer, len, MSG_DONTWAIT,
			(void*)&hndl->sa, hndl->sa_len);
}


static int rpc_apply() {
	return 0;
}

static int rpc_init() {
	const char *rpc = config_get_string("main", "rpc",
				"/var/run/hotspotd.sock");
	if (rpc && rpc[0]) {
		unlink(rpc);
		event_rpc.fd = usock(USOCK_UNIX | SOCK_DGRAM | USOCK_SERVER,
								rpc, NULL);
		if (event_rpc.fd < 0)
			return -1;

		event_ctl(EVENT_EPOLL_ADD, &event_rpc);
	}

	return rpc_apply();
}

static void rpc_deinit() {
	if (event_rpc.fd >= 0) {
		close(event_rpc.fd);
		event_rpc.fd = -1;
	}
}

// Handle incoming RPC requests
static void rpc_handle(struct event_epoll *event, uint32_t revents) {
	for (;;) {
		uint8_t buf[1024];
		ssize_t res;
		struct rpc_handle hndl = {.sa_len = sizeof(hndl.sa)};
		struct frmsg *frm;

		if ((res = recvfrom(event->fd, buf, sizeof(buf),
			MSG_DONTWAIT, (void*)&hndl.sa, &hndl.sa_len)) < 0)
			break;

		if (!(frm = frm_load(buf, res)))
			continue;

		res = -ENOTSUP;
		uint16_t type = frm_type(frm);
		struct rpc_handler *rpc;
		list_for_each_entry(rpc, &handlers, _head) {
			if (rpc->type == type) {
				res = rpc->handler(&hndl, frm);
				break;
			}
		}

		if (res <= 0) {
			uint32_t seq = frm->frm_seq;
			frm = frm_init(buf, FRMSG_ERROR, 0);
			frm->frm_seq = seq;
			frm_put_u32(frm, FRE_CODE, -res);
			rpc_send(&hndl, frm, frm_length(frm));
		}
	}
}

RESIDENT_REGISTER(rpc, 150)



// RPC interface for system info
static int rpc_sys_info(struct rpc_handle *hndl, struct frmsg *frr) {
	struct mallinfo m = mallinfo();
	uint8_t buffer[64];
	struct frmsg *frm = frm_init(buffer, FRT_SYS_INFO, 0);
	frm_put_u32(frm, FRA_SYS_PID, getpid());
	frm_put_u32(frm, FRA_SYS_DYNMEMORY, m.uordblks);
	frm->frm_seq = frr->frm_seq;
	rpc_send(hndl, frm, frm_length(frm));
	return 1;
}
#ifdef __SC_BUILD__
static int rpc_sys(struct rpc_handle *hndl, struct frmsg *frr) 
{
    uint16_t type = frm_type(frr);
    DPRINTF("---------------%x\n",type);
    if(type == FRT_SYS_START)
    {
        event_stop(HOTSPOT_START);
    }
    else if (type == FRT_SYS_STOP)
    {
        event_stop(HOTSPOT_STOP);
    }
    else if (type == FRT_SYS_RELOAD)
    {
        event_stop(HOTSPOT_CONFIGRELOAD);
    }
    return 0;
}
static int rpc_sys_status(struct rpc_handle *hndl, struct frmsg *frr) {
	uint8_t buffer[64];
	struct frmsg *frm = frm_init(buffer, FRT_SYS_STATUS, 0);
    int l2tp_status = tunnel_get_status();
    if(server_status == HOTSPOT_LOWER_BW)
        frm_put_string(frm, FRA_SYS_STATUS, "Disable");
    else
    {
        if(gre_status == L2TP_ESTABLISHED || l2tp_status == L2TP_RTX)
            frm_put_string(frm, FRA_SYS_STATUS, "OK");
        else if(gre_status == L2TP_HANDSHAKE)
            frm_put_string(frm, FRA_SYS_STATUS, "error");
        else
            frm_put_string(frm, FRA_SYS_STATUS, "");
    }
	frm->frm_seq = frr->frm_seq;
	rpc_send(hndl, frm, frm_length(frm));
	return 1;
}
#endif
// RPC interface for configuration changes
static int rpc_cfg(struct rpc_handle *hndl, struct frmsg *frr) {
	uint16_t type = frm_type(frr);
	if (type == FRT_CFG_COMMIT) {
		return (!config_commit()) ? 0 : -errno;
	} else if (type == FRT_CFG_APPLY) {
		return (!hotspot_control(HOTSPOT_CTRL_APPLY)) ? 0 : -errno;
	} else if (type == FRT_CFG_RESTART) {
		raise(SIGHUP);
		return 0;
	}

	// FRT_CFG_SET

	struct frattr *attrs[_FRA_CFG_SIZE];
	frm_parse(frr, attrs, _FRA_CFG_SIZE);

	const char *section = fra_to_string(attrs[FRA_CFG_SECTION]);
	const char *option = fra_to_string(attrs[FRA_CFG_OPTION]);
	const char *value = fra_to_string(attrs[FRA_CFG_VALUE]);
    DPRINTF("--------------------%s %s %s\n",section,option,value);
	uint16_t flags = frm_flags(frr);

	if (!section || ((flags & FRM_F_APPEND) && (!option || !value)))
		return -EINVAL;

	if (flags & FRM_F_APPEND) {
		return (!config_add_string(section, option, value)) ? 0 : -errno;
	} else {
		return (!config_set_string(section, option, value)) ? 0 : -errno;
	}
}

static struct rpc_handler hndl[] = {
		{FRT_SYS_INFO, rpc_sys_info},
		{FRT_CFG_SET, rpc_cfg},
		{FRT_CFG_COMMIT, rpc_cfg},
		{FRT_CFG_APPLY, rpc_cfg},
#ifdef __SC_BUILD__
		{FRT_SYS_START, rpc_sys},
		{FRT_SYS_STOP, rpc_sys},
		{FRT_SYS_RELOAD, rpc_sys},
		{FRT_SYS_STATUS, rpc_sys_status},
#endif
		{0}
};

RPC_REGISTER(hndl)
