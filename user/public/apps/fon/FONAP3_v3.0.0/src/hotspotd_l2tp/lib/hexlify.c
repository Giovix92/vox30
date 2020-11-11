/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */

#include "hexlify.h"

static const char hexdigits[] = "0123456789abcdef";
static const char heXdigits[] = "0123456789ABCDEF";
static const char hexvalues[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -2, -1, -1, -2, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

void hexlify(char *dst, const uint8_t *src, size_t len, char sep) {
	for (size_t i = 0; i < len; ++i) {
		*dst++ = hexdigits[src[i] >> 4];
		*dst++ = hexdigits[src[i] & 0x0f];
		if (sep)
			*dst++ = sep;
	}
	if (sep)
		dst--;
	*dst = 0;
}

void heXlify(char *dst, const uint8_t *src, size_t len, char sep) {
	for (size_t i = 0; i < len; ++i) {
		*dst++ = heXdigits[src[i] >> 4];
		*dst++ = heXdigits[src[i] & 0x0f];
		if (sep)
			*dst++ = sep;
	}
	if (sep)
		dst--;
	*dst = 0;
}

int unhexlify(uint8_t *dst, const char *src, size_t len) {
	if (len % 2)
		return -1;

	for (size_t i = 0; i < len / 2; ++i) {
		int8_t x = src[2 * i], y = src[2 * i + 1];
		if (x < 0 || (x = hexvalues[x]) < 0 ||
				y < 0 || (y = hexvalues[y]) < 0)
			return -1;
		dst[i] = x << 4 | y;
	}

	return 0;
}
