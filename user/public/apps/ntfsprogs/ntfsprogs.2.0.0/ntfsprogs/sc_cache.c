
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sc_cache.h"
#include "logging.h"

int sc_cache_create(struct list_head *free_list, int cache_nr, size_t cachesize)
{
	struct sc_cache_s *cachep;
	u8 *data;
	int i;

	for ( i = 0; i < cache_nr; i++ ) {
		cachep = malloc(sizeof(struct sc_cache_s));
		if ( !cachep ) {
			goto _err;
		}
		data = malloc(cachesize);
		if ( !data ) {
			free(cachep);
			goto _err;
		}
		cachep->data = data;
		list_add(&cachep->list, free_list);
#ifdef  CACHE_DBG
		cachep->id = i;
		ntfs_log_trace("init cache <%d> \n", i);
#endif
	}
	return 0;
_err:
	sc_cache_free(free_list);
	return -1;
}

void sc_cache_free(struct list_head *to_free)
{
	struct list_head *item, *n;
	struct sc_cache_s *cachep;
	
	list_for_each_safe(item, n, to_free) {
		cachep = list_entry(item, struct sc_cache_s, list);
		list_del(&cachep->list);
		free(cachep->data);
		free(cachep);
	}
}

struct sc_cache_s* sc_cache_get(struct list_head *free_list)
{
	struct sc_cache_s *cachep;

	if ( list_empty(free_list) ) {
		return NULL;
	}
	cachep = list_entry(free_list->next, struct sc_cache_s, list);
	list_del_init(&cachep->list); /* init it so we can put it safely. */
#ifdef  CACHE_DBG
	ntfs_log_trace("get cache <%d> \n", cachep->id);
#endif
	return cachep;
}

void sc_cache_put(struct list_head *free_list, struct sc_cache_s *cachep)
{
	list_del(&cachep->list);
	list_add(&cachep->list, free_list);
#ifdef  CACHE_DBG
	ntfs_log_trace("put cache <%d> \n", cachep->id);
#endif
}

void sc_cache_put_list(struct list_head *free_list, struct list_head *to_free)
{
#ifdef CACHE_DBG
	struct list_head *item;
	struct sc_cache_s *cachep;
	list_for_each(item, to_free) {
		cachep = list_entry(item, struct sc_cache_s, list);
		ntfs_log_trace("put cache <%d> \n", cachep->id);
	}
#endif
	list_splice(to_free, free_list);
}
