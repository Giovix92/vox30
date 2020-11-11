#ifndef TRAFFIC_H_
#define TRAFFIC_H_

#include <stdint.h>

/**
 * Create a new traffic class for a given user id.
 * Create a new traffic for user with ID classid (max. 12-bits) and setup
 * filters to match its traffic.
 *
 * returns status
 */
int traffic_create(uint16_t classid);

/**
 * Returns the current traffic stats for a user.
 * Returns the traffic stats for the given user with ID classid and stores
 * the values in the given parameters.
 *
 * returns status
 */
int traffic_stats(uint16_t classid,
uint64_t *byte_in, uint64_t *byte_out, uint32_t *pkt_in, uint32_t *pkt_out);

/**
 * Remove traffic classes and filters for a given classid.
 */
void traffic_destroy(uint16_t classid);

#endif /* TRAFFIC_H_ */
