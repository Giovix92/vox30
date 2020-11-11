/**
 * hotspotd - A speedy hotspot solution
 * All rights reserved by Fon Wireless Ltd.
 *
 * Authors:
 * Steven Barth <steven.barth@fon.com>
 * John Crispin <john.crispin@fon.com>
 *
 */


#include "fonrpc.h"
#include <stdint.h>
#include <string.h>

struct frattr* fra_init(void *buffer, uint16_t type)
{
	struct frattr *fra = buffer;
	fra->fra_type = fswap16(type);
	fra->fra_len = fswap16(FRA_HDRLEN);
	return fra;
}

void* fra_claim(struct frattr *fra, uint16_t type, uint16_t len)
{
	uint8_t *buffer = ((uint8_t*)fra) + FRA_ALIGN(fswap16(fra->fra_len));
	fra->fra_len = fswap16(FRA_HDRLEN + len + (buffer - (uint8_t*)fra));
	fra = (struct frattr*)buffer;
	fra->fra_len = fswap16(FRA_HDRLEN + len);
	fra->fra_type = fswap16(type);
	return FRA_DATA(fra);
}

void fra_put(struct frattr *fra, uint16_t type, const void *data, uint16_t len)
{
	memcpy(fra_claim(fra, type, len), data, len);
}

void fra_parse(struct frattr *fra, struct frattr *fratbl[], size_t size)
{
	memset(fratbl, 0, size * sizeof(*fratbl));
	struct frattr *frac;
	fra_foreach_attr(frac, fra) {
		uint16_t type = FRA_TYPE(frac);
		if (type <= size)
			fratbl[type] = frac;
	}
}

const char *fra_to_string(struct frattr *fra)
{
	const char *fdata = (fra) ? FRA_DATA(fra) : NULL;
	size_t flen = (fdata) ? FRA_PAYLOAD(fra) : 0;
	return (flen > 0 && fdata[flen - 1] == 0) ? fdata : NULL;
}

uint32_t fra_to_u32(struct frattr *fra, uint32_t def)
{
	return (fra && FRA_PAYLOAD(fra) == sizeof(uint32_t))
			? fswap32(*((uint32_t*)FRA_DATA(fra))) : def;
}

uint64_t fra_to_u64(struct frattr *fra, uint64_t def)
{
	return (fra && FRA_PAYLOAD(fra) == sizeof(uint64_t))
			? fswap64(*((uint64_t*)FRA_DATA(fra))) : def;
}
