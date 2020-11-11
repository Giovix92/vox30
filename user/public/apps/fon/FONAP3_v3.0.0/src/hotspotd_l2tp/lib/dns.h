#ifndef DNS_H_
#define DNS_H_

#include <stdint.h>
#include <stdlib.h>

struct dnspkt {
	uint16_t header[6];
	uint8_t payload[];
};

enum dns_header {
	DNS_IDENT,
	DNS_FLAGS,
	DNS_QDCOUNT,
	DNS_ANCOUNT
};

enum dns_type {
	DNS_T_A = 1,
	DNS_T_TXT = 16,
	DNS_T_AAAA = 28,
};

enum dns_flag {
	DNS_F_RD = 0x0100,
};

enum dns_class {
	DNS_C_IN = 1,
};

/**
 * Decode a domain in DNS format into a string.
 *
 * data:	pointer to the beginning of a DNS request
 * offp:	pointer to the current data offset in the DNS request (updated)
 * size:	size of the DNS request
 * buffer:	buffer to store the domain string (min. 255 Bytes)
 *
 * returns the length of the string placed in buffer or -1 on error
 * the offset (*offp) is set to the next byte after the decoded domain
 */
int dns_getname(uint8_t *data, size_t *offp, size_t size, char buffer[255]);

/**
 * Encode a domain given as a string in DNS format
 *
 * data:	buffer to place the encoded domain
 * len:		length of the buffer
 * domain:	domain
 *
 * returns the amount of bytes placed in data or -1 on error
 */
int dns_putname(uint8_t *data, size_t len, const char *domain);

/**
 * Return a 16-bit integer in an alignment and endianes-safe way
 *
 * data:	pointer to a buffer
 * offp:	pointer to the current data offset in the buffer (updated)
 *
 * returns the integer in host byte-order
 */
static inline uint16_t dns_get16(uint8_t *data, size_t *offp) {
	*offp += 2;
	return data[(*offp)-2] << 8 | data[(*offp)-1];
}

#endif /* DNS_H_ */
