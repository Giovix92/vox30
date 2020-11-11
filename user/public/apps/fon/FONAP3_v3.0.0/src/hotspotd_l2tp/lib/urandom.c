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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "lib/event.h"
#include "urandom.h"

static int fd = -1;

int urandom_init() {
	return ((fd = event_cloexec(open("/dev/urandom", O_RDONLY))) < 0) ? -1 : 0;
}

void urandom_deinit() {
	close(fd);
}

int urandom(void *buffer, size_t len) {
	return read(fd, buffer, len);
}

