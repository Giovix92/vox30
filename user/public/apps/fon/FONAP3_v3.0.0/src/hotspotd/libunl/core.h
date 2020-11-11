#ifndef CORE_H_
#define CORE_H_

#include "unl.h"

#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif

struct unl {
	int fd;
	int seq;

	/* Request state */
	struct nlmsghdr *next;
	void *buffer;
	size_t bufsiz;
	size_t buffered;
};

#endif /* CORE_H_ */
