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
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#ifdef __UCLIBC__
#include <features.h>
#if __UCLIBC_MAJOR__ == 0 && __UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ <= 29
#define res_init __res_init
extern int __res_init();
#endif
#endif

#include "usock.h"

#ifndef IP_TRANSPARENT
#define IP_TRANSPARENT 19
#endif

#ifndef IPV6_TRANSPARENT
#define IPV6_TRANSPARENT 75
#endif


#define USOCK_FAMILY(x) ((x) & 0xf0)
#define USOCK_SOCKTYPE(x) ((x) & 0xf)
#define USOCK_PROTOCOL(x) (((x) >> 8) & 0xff)


static void usock_set_flags(int sock, unsigned int type)
{
	if (!(type & USOCK_NOCLOEXEC))
		fcntl(sock, F_SETFD, fcntl(sock, F_GETFD) | FD_CLOEXEC);

	if (type & USOCK_NONBLOCK)
		fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
}

static int usock_connect(const char *host, struct sockaddr *sa, int sa_len, int family, int type)
{
	int sock = socket(family, USOCK_SOCKTYPE(type), USOCK_PROTOCOL(type));
	if (sock < 0)
		return -1;

	usock_set_flags(sock, type);

	if ((type & USOCK_BINDTODEV) &&
		setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, host, strlen(host) + 1)) {
		// FAIL through
	} else if (type & USOCK_SERVER) {
		const int one = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

		if (type & USOCK_TRANSPARENT) {
			setsockopt(sock, SOL_IP, IP_TRANSPARENT, &one, sizeof(one));
			setsockopt(sock, SOL_IPV6, IPV6_TRANSPARENT, &one, sizeof(one));
		}

		if (!bind(sock, sa, sa_len) &&
		    (USOCK_SOCKTYPE(type) == SOCK_DGRAM || !listen(sock, SOMAXCONN)))
			return sock;
	} else {
		if (!connect(sock, sa, sa_len) || errno == EINPROGRESS)
			return sock;
	}

	close(sock);
	return -1;
}

static int usock_unix(const char *host, int type)
{
	struct sockaddr_un sun = {.sun_family = AF_UNIX};

	if (strlen(host) >= sizeof(sun.sun_path)) {
		errno = EINVAL;
		return -1;
	}
	strcpy(sun.sun_path, host);

	return usock_connect(NULL, (struct sockaddr*)&sun, sizeof(sun), AF_UNIX, type);
}

static int usock_inet(int type, const char *host, const char *service)
{
	struct addrinfo *result, *rp;
	struct addrinfo hints = {
		.ai_family = (USOCK_FAMILY(type) == USOCK_IPV6ONLY) ? AF_INET6 :
			(USOCK_FAMILY(type) == USOCK_IPV4ONLY) ? AF_INET : AF_UNSPEC,
		.ai_socktype = USOCK_SOCKTYPE(type),
		.ai_flags = AI_ADDRCONFIG
			| ((type & USOCK_SERVER) ? AI_PASSIVE : 0)
#ifndef __UCLIBC__
			| ((type & USOCK_LITERALPORT) ? 0 : AI_NUMERICSERV)
#endif
			| ((type & USOCK_NUMERICHOST) ? AI_NUMERICHOST : 0),
	};
	int sock = -1;

	// (older) uclibc versions do not handle changes in resolv.conf correctly
#ifdef __UCLIBC__
#include <features.h>
#if __UCLIBC_MAJOR__ == 0 && __UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ <= 29
	res_init();
#endif
#endif

	errno = ENOENT;
	if (getaddrinfo((type & USOCK_BINDTODEV) ? NULL : host, service, &hints, &result))
		return -1;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sock = usock_connect(host, rp->ai_addr, rp->ai_addrlen, rp->ai_family, type);
		if (sock >= 0)
			break;
	}

	freeaddrinfo(result);
	return sock;
}

int usock(int type, const char *host, const char *service) {
	int sock;

	if (USOCK_FAMILY(type) == USOCK_UNIX)
		sock = usock_unix(host, type);
	else
		sock = usock_inet(type, host, service);

	if (sock < 0)
		return -1;

	return sock;
}
