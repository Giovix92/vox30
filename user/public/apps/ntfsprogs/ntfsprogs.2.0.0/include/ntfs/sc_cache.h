#ifndef _SC_CACHE_H_
#define _SC_CACHE_H_

#include "types.h"
#include "list.h"

//#define CACHE_DBG 1

struct sc_cache_s {
	struct list_head list;        /* cached data list. */
	u8 *data;                     /* data buffer. */
	s64 size;                     /* data size. */
#ifdef  CACHE_DBG
	int  id;                      /* just what it says. */
#endif
};


void sc_cache_free(struct list_head *to_free);
int sc_cache_create(struct list_head *free_list, int cache_nr, size_t cachesize);
struct sc_cache_s* sc_cache_get(struct list_head *free_list);
void sc_cache_put(struct list_head *free_list, struct sc_cache_s *cachep);
void sc_cache_put_list(struct list_head *free_list, struct list_head *to_free);

#endif
