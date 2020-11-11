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
#include <asm/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "core.h"


struct unl* unl_open(int proto, const struct sockaddr_nl *peer) {
	const struct sockaddr_nl sa_default = {.nl_family = AF_NETLINK};
	if (!peer) peer = &sa_default;

	struct unl *hndl = malloc(sizeof(struct unl));
	if (!hndl) return NULL;

	hndl->fd = -1;
	hndl->buffer = NULL;
	hndl->buffered = 0;
	hndl->seq = time(NULL);

	if (unl_buffer_set(hndl, UNL_BUFSIZ)) goto err;

	if ((hndl->fd = socket(AF_NETLINK, SOCK_RAW, proto)) < 0) goto err;
	fcntl(hndl->fd, F_SETFD, fcntl(hndl->fd, F_GETFD) | FD_CLOEXEC);

	if (connect(hndl->fd, (struct sockaddr*)peer, sizeof(*peer))) goto err;

	return hndl;

err:
	unl_close(hndl);
	return NULL;
}

void unl_flush(struct unl *hndl) {
	unl_buffer_flush(hndl);
	uint8_t buf[32];
	struct iovec iov = { .iov_base = buf, .iov_len = sizeof(buf) };
	struct msghdr msg = { .msg_iov = &iov, .msg_iovlen = 1 };
	while (recvmsg(hndl->fd, &msg, MSG_DONTWAIT) > 0);
}


void unl_buffer_flush(struct unl *hndl) {
	hndl->buffered = 0;
}

int unl_buffer_set(struct unl *hndl, size_t newsize) {
	if (hndl->buffered > newsize) return -1;
	void *newbuf = realloc(hndl->buffer, newsize);
	if (!newbuf) return -1;
	hndl->buffer = newbuf;
	hndl->bufsiz = newsize;
	return 0;
}

int unl_subscribe(struct unl *hndl, uint32_t group) {
	return setsockopt(hndl->fd, SOL_NETLINK,
		NETLINK_ADD_MEMBERSHIP, &group, sizeof(group));
}

int unl_unsubscribe(struct unl *hndl, uint32_t group) {
	return setsockopt(hndl->fd, SOL_NETLINK,
		NETLINK_DROP_MEMBERSHIP, &group, sizeof(group));
}

int unl_timeout(struct unl *hndl, uint32_t msec) {
	struct timeval tv = {
		.tv_sec = msec / 1000,
		.tv_usec = msec % 1000,
	};
	setsockopt(hndl->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	return setsockopt(hndl->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

int unl_fd(struct unl *hndl) {
	return hndl->fd;
}

void unl_close(struct unl *hndl) {
	close(hndl->fd);
	free(hndl->buffer);
	free(hndl);
}



/* IO functions */

// Sends a request (return a message sequence id > 0 on success, -1 on fail)
int unl_request(struct unl *hndl, struct nlmsghdr *nlm) {
	struct iovec iov = { .iov_base = nlm, .iov_len = nlm->nlmsg_len };
	struct msghdr msg = { .msg_iov = &iov, .msg_iovlen = 1 };
	nlm->nlmsg_flags |= NLM_F_REQUEST;
	nlm->nlmsg_pid = 0;
	nlm->nlmsg_seq = ++hndl->seq;
	ssize_t txed = sendmsg(hndl->fd, &msg, 0);
	if (txed == nlm->nlmsg_len) {
		return hndl->seq;
	} else if (txed > 0) {
		errno = EPIPE;
	}
	return -1;
}

// Simple receive
struct nlmsghdr* unl_receive(struct unl *hndl) {
	while (!NLMSG_OK(hndl->next, hndl->buffered)) {
		struct iovec iov = { .iov_base = hndl->buffer, .iov_len = hndl->bufsiz };
		struct msghdr msg = { .msg_iov = &iov, .msg_iovlen = 1 };
		ssize_t len = recvmsg(hndl->fd, &msg, 0);
		if (msg.msg_flags & MSG_TRUNC) {
			errno = ENOBUFS;
			len = -1;
		}

		if (len < 0) {
			if (errno == EINTR) continue;
			return NULL;
		}
		hndl->buffered = len;
		hndl->next = hndl->buffer;
	}
	struct nlmsghdr *nh = hndl->next;
	hndl->next = NLMSG_NEXT(hndl->next, hndl->buffered);
	return nh;
}


struct nlmsghdr* unl_receive_response(struct unl *hndl) {
	struct nlmsghdr *nh;
	do {
		nh = unl_receive(hndl);
	} while (nh && nh->nlmsg_seq != hndl->seq);
	return nh;
}


int unl_call(struct unl *hndl, struct nlmsghdr *nlm) {
	nlm->nlmsg_flags |= NLM_F_ACK; // We need some reply
	if (unl_request(hndl, nlm) < 0) return -1;
	return nlmsg_success(unl_receive_response(hndl));
}
