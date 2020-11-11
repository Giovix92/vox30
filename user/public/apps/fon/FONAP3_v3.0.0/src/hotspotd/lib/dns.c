/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include <string.h>
#include "dns.h"

// Extract a domain name from a DNS packet
int dns_getname(uint8_t *data, size_t *offp, size_t size, char buffer[255]) {
	size_t nameleft = 255, offset = *offp;
	for (;;) {
		if (size <= offset)
			return -1;

		uint16_t rlen = data[offset];
		if (rlen == 0) { /* End delimiter */
			if (nameleft != 255) {
				nameleft++;
				buffer--;
			}
			buffer[0] = 0; /* Replace last . with 0 */
			*offp = offset + 1;
			return 255 - nameleft;
		} else if (((rlen >> 6) == 3) && (size > offset + 1)) {	/* Link */
			rlen = ((rlen & 63) << 8) + data[offset + 1];
			if (rlen >= offset) /* Prevent link loops and other weird stuff */
				return -1;

			*offp = offset + 2;
			offp = &offset; /* Do not change the external offset any more */
			offset = rlen;
		} else if ((rlen >> 6) == 0) { /* Label (just copy it, if it is sane) */
			rlen++;
			if (nameleft < rlen || (offset + rlen) > size)
				return -1;

			memcpy(buffer, &data[offset + 1], rlen - 1);
			buffer += rlen;
			buffer[-1] = '.';

			offset += rlen;
			nameleft -= rlen;
		} else {
			return -1;
		}
	}
}

// Convert a textual representation of a domain into binary format
int dns_putname(uint8_t *data, size_t len, const char *domain) {
	uint8_t *d = data;
	if (strlen(domain) >= 255)
		return -1;

	while (*domain) {
		const char *c = strchr(domain, '.');
		size_t slen = (c) ? c - domain : strlen(domain);
		if (slen > 63 || slen + 2 > len)
			return -1;

		*d++ = slen;
		memcpy(d, domain, slen);
		len -= slen + 1;
		d += slen;

		if (!c)
			break;
		domain = c + 1;
	}
	*d++ = 0;
	return d - data;
}
