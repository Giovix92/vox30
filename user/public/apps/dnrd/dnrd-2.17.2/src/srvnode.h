/*

    File: srvnode.h
    
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



#ifndef SRVNODE_H
#define SRVNODE_H

#include <netinet/in.h>

#ifdef SCM_BINDING
#include <net/if.h>
#include <fcntl.h>
#define QUERY_INVALID_TIMEOUT (-1)
#endif

typedef struct _srvnode {
  /*  int                 sock;*/ /* the communication socket */
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  struct sockaddr_in6  addr;      /* IP address of server */
#else
  struct sockaddr_in  addr;      /* IP address of server */
#endif
#ifdef SCM_BINDING
  int                 srv_pri;  /* Priority of server */
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  struct sockaddr_in6  inf_addr;      /* IP address of server */
#else
  struct sockaddr_in  inf_addr;      /* IP address of server */
#endif
  char 				  inf[IFNAMSIZ];
  int				  inf_pri;   /* Priority of interface */
  int                 wan_id;
  int				  time_out;
#endif
  time_t              inactive; /* is this server active? */
  unsigned int        send_count;
  int                 send_time;
  int                 tcp;
#ifdef __SC_BUILD__
  int                 local;
#endif
  struct _query   *newquery; /* new opened socket, prepared for a new query */
  struct _srvnode     *next; /* ptr to next server */
} srvnode_t;


srvnode_t *alloc_srvnode(void);
srvnode_t *init_srvlist(void);
srvnode_t *ins_srvnode (srvnode_t *list, srvnode_t *p);
srvnode_t *del_srvnode_after(srvnode_t *list);
srvnode_t *destroy_srvnode(srvnode_t *p);
srvnode_t *clear_srvlist(srvnode_t *head);
srvnode_t *destroy_srvlist(srvnode_t *head);
srvnode_t *add_srv(srvnode_t *head, const char *ipaddr);
srvnode_t *last_srvnode(srvnode_t *head);
int no_srvlist(srvnode_t *head);

#ifdef SCM_BINDING
srvnode_t *add_srv_by_pri(srvnode_t *head, const char *ipaddr);
#endif
#endif




