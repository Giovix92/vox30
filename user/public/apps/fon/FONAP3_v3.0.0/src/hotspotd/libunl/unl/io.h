/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#ifndef UNL_IO_H_
#define UNL_IO_H_

#include <stdint.h>
#include <stddef.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "parse.h"

#define UNL_BUFSIZ 8192

struct unl;


/*
 * Open a netlink socket
 *
 *
 * proto:	netlink protocol ID (NETLINK_ROUTE, NETLINK_GENERIC, ...)
 * peer:	Address of the peer to communicate with (NULL = kernel)
 *
 */
struct unl* unl_open(int proto, const struct sockaddr_nl *peer);


/*
 * Its obvious isn't it?
 */
int unl_fd(struct unl *hndl);
void unl_close(struct unl *hndl);


/*
 * Flushes the input buffer of a handle discarding all pending netlink packages.
 */
void unl_buffer_flush(struct unl *hndl);


/*
 * Sets the size of the internal message buffer
 * This might fail if there is not enough memory or if the new buffer size
 * is smaller than the amount of data currently buffered.
 */
int unl_buffer_set(struct unl *hndl, size_t newsize);


/*
 * Subscribe / unsubscribe a multicast group
 * Returns 0 on success
 */
int unl_subscribe(struct unl *hndl, uint32_t group);
int unl_unsubscribe(struct unl *hndl, uint32_t group);

/*
 * Set the timeout of the socket in milliseconds
 * Return 0 on success
 */
int unl_timeout(struct unl *hndl, uint32_t msec);


/**
 * Send a netlink message and sets up the unl handle for receiving
 * answers for it.
 *
 * hndl:	An open unl handle
 *
 * nlm:		Your message
 * 			nlmsg_pid and nlmsg_seq will be set before sending
 * 			and the NLN_F_REQUEST flag will be set
 *
 * returns a sequence number > 0 on success (message send completely).
 * rerurns a negative value on error (check errno).
 */
int unl_request(struct unl *hndl, struct nlmsghdr *nlm);


/**
 * Receive the next pending netlink message.
 *
 * Returns a pointer to the netlink message on success, NULL otherwise.
 *
 * NOTE:
 * Memory is automatically managed. The pointer remains valid until one of
 * the unl_receive_* or unl_buffer_* functions is called on the same handle.
 */
struct nlmsghdr* unl_receive(struct unl *hndl);


/**
 * Returns the next pending netlink message related to the last request.
 *
 * NOTE: Unrelated messages are skipped.
 *
 * NOTE:
 * Memory is automatically managed. The pointer remains valid until one of
 * the unl_receive_* or unl_buffer_* functions is called on the same handle.
 */
struct nlmsghdr* unl_receive_response(struct unl *hndl);


/**
 * Iterates over all responses related to the last request
 *
 * Stops when all parts of the message have been received or when an error
 * has occured. Test for !nlmsg_success(nh) to check for errors afterwards.
 *
 *
 * nh: struct nlmsghdr* to the current message
 *
 */
#define unl_foreach_response(nh, hndl) \
	nh = unl_receive_response(hndl); for (int _stop = 0; \
	!nlmsg_errno(nh) && nh->nlmsg_type != NLMSG_DONE && !_stop; \
	_stop = nlmsg_complete(nh), nh = (_stop) ? nh : unl_receive_response(hndl))



/**
 * Convenience function for netlink messages returning only a status code.
 * This is basically unl_request and unl_response combined
 * after setting the NLM_F_ACK flag.
 *
 * Returns 0 on success, a negative error code otherwise and sets errno.
 */
int unl_call(struct unl *hndl, struct nlmsghdr *nlm);


/**
 * Flush all receive buffers (both buffer and kernel)
 *
 * NOTE: This includes unl_buffer_flush(hndl)
 */
void unl_flush(struct unl *hndl);


#endif /* UNL_IO_H_ */
