#ifndef RPC_H_
#define RPC_H_

#include "libfonrpc/fonrpc-local.h"

/**
 * RPC handler callback prototype.
 * A handler is called when an RPC message with the given type arrives.
 * The handler will be given the request req and a handle for sending replies.
 *
 * The handler should return the following values:
 * 0	to send a success reply
 * < 0	to send an error reply where the negative error code should be
 * 	a UNIX error code like errno
 * 1	to tell the backend to don't send a status reply. The handler is
 * 	expected to use rpc_send and send a reply by itself before returning 1.
 */
struct rpc_handle;
typedef int (rpc_handler_cb)(struct rpc_handle *hndl, struct frmsg *req);

struct rpc_handler {
	uint16_t type;			// RPC message type
	rpc_handler_cb *handler;	// RPC message callback
	struct list_head _head;		// (internal)
};

/**
 * Automatically register a set of handler functions for RPC requests.
 */
#define RPC_REGISTER(rpcs) \
static void __attribute__((constructor)) _rpcinit() { rpc_register(rpcs); }

/*
 * Register a set of handler functions for RPC-messages of a given type.
 * (internally used by RPC_REGISTER macro)
 */
void rpc_register(struct rpc_handler *hndl);

/**
 * Sends a reply for a given RPC request.
 * Sends a reply for a given RPC request denoted by the handle hndl.
 */
void rpc_send(struct rpc_handle *hndl, void *buffer, size_t len);

#endif /* RPC_H_ */
