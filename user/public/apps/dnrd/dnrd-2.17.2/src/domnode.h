/*

    File: domnode.c
    
    Copyright (C) 2004 by Natanael Copa <n@tanael.org>

    This source is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2, or (at your option)
    any later version.

    This source is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#ifndef DOMNODE_H
#define DOMNODE_H

#include <sys/types.h>
#include "srvnode.h"

typedef struct _domnode {
  char            *domain;  /* the domain */
#ifdef SCM_BINDING
  uint32_t        domain_type;
#endif
  srvnode_t       *srvlist; /* linked list of servers */
  srvnode_t       *current;
#ifdef SCM_BINDING
  srvnode_t       *current_bk; /* Pointer to different interface node 
                                  which priortiy same with current */
#endif
  int             roundrobin; /* load balance the servers */
  int             retrydelay; /* delay before reactivating the servers */
  struct _domnode *next;    /* ptr to next server */
} domnode_t;


domnode_t *alloc_domnode(void);
domnode_t *ins_domnode (domnode_t *list, domnode_t *p);
domnode_t *del_domnode(domnode_t *list);
domnode_t *destroy_domnode(domnode_t *p);
domnode_t *empty_domlist(domnode_t *head);
domnode_t *destroy_domlist(domnode_t *head);
domnode_t *add_domain(domnode_t *list, const int load_balance, 
		      char *name, const int maxlen);
domnode_t *del_domain(domnode_t *head, char *cname, const int maxlen);
#ifdef __SC_BUILD__ 
srvnode_t *next_active_by_wan_id(domnode_t *d, int wan_id, int inf_pri, int ip_pri);
domnode_t *search_domnode_by_dnametype(domnode_t *head, const char *name, uint32_t dtype);
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
domnode_t *search_subdomnode(domnode_t *head, const char *name, 
			     const int maxlen, int dtype, struct sockaddr_in6 *fromaddrp);
#else
domnode_t *search_subdomnode(domnode_t *head, const char *name, 
			     const int maxlen, int dtype, struct sockaddr_in *fromaddrp);
#endif
#else
domnode_t *search_subdomnode(domnode_t *head, const char *name, 
			     const int maxlen);
#endif /* End Of __SC_BUILD__ */
domnode_t *search_domnode(domnode_t *head, const char *name);

srvnode_t *set_current(domnode_t *d, srvnode_t *s);

#ifdef __SC_BUILD__ 
srvnode_t *next_active(domnode_t *d, int isremote);
#else
srvnode_t *next_active(domnode_t *d);
#endif
srvnode_t *deactivate_current(domnode_t *d);

void reactivate_srvlist(domnode_t *d);
void retry_srvlist(domnode_t *d, const int delay);
#endif




