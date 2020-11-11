/*

    File: srvnode.c
    
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



#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

#include "srvnode.h"
#include "lib.h"
#include "common.h"

srvnode_t *alloc_srvnode(void) {
  srvnode_t *p = allocate(sizeof(srvnode_t));
#ifdef __SC_BUILD__
  memset(p, 0, sizeof(srvnode_t));
#endif
  p->inactive = -1;
  p->send_time = 0;
  p->send_count = 0;
#ifdef SCM_BINDING
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  // Have do the  memset to zero.
#else
  p->inf_addr.sin_addr.s_addr = 0;
#endif
  p->inf[0] = '\0';
  p->time_out = QUERY_INVALID_TIMEOUT;
#endif
  /* actually we return a new emty list... */
  return p->next=p;
}


/* init the linked server list
 * returns ptr to the head/tail dummy node in an empty list
 */

srvnode_t *init_srvlist(void) {
  srvnode_t *p = alloc_srvnode();
  /*  p->sock=0; */
  p->next = p;
  return p;
}

/* insert srvnode in the list 
 * returns the new node
 */
srvnode_t *ins_srvnode (srvnode_t *list, srvnode_t *p) {
  assert(list!=NULL);
  p->next = list->next;
  list->next = p;
  return p;
} 

/* removes a node from the list.
 * returns the deleted node 
 */
srvnode_t *del_srvnode_after(srvnode_t *list) {
  srvnode_t *p = list->next;
  assert(list!=NULL);
  list->next = p->next;
  return p;
}

/* closes the server socket and frees the mem */
srvnode_t *destroy_srvnode(srvnode_t *p) {
  /* close socket */
  assert(p!=NULL);
  /*  if (p->sock) close(p->sock); */
  free(p);
  return NULL;
}

/* emties a linked server list. returns the head */
srvnode_t *clear_srvlist(srvnode_t *head) {
  srvnode_t *p=head;
  assert(head != NULL);
  while (p->next != head) {
    destroy_srvnode(del_srvnode_after(p));
  }
  return (head);
}

/* destroys the server list, including the head */
srvnode_t *destroy_srvlist(srvnode_t *head) {
  assert(head != NULL);
  clear_srvlist(head);
  free(head);
  return NULL;
}

#ifdef SCM_BINDING
srvnode_t *add_srv_by_pri(srvnode_t *head, const char *ipaddr) {
    srvnode_t *p;
    srvnode_t *psrv = head;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    char buf[64];
    struct sockaddr_in6 addr;
#else
    struct sockaddr_in addr;
#endif
    char *srv_pri = NULL;
    char *inf_ip = NULL, *inf = NULL, *inf_pri = NULL;
    char *wan_id = NULL;
    char *time_out = NULL;
    int cur_inf_pri = -1;
    int cur_srv_pri = -1;

    if((inf_ip = strchr(ipaddr, (int)'#')))
    {
	*inf_ip = '\0';
	inf_ip++;
	inf = strchr(inf_ip, (int)'#');
    }
    if(inf)
    {
        *inf = '\0';
        inf++;
        wan_id = strchr(inf, (int)'#');
    }
    if(wan_id)
    {
        *wan_id = '\0';
        wan_id++;
	time_out = strchr(inf, (int)'#');
    }
    if(time_out)
    {
	*time_out = '\0';
	time_out++;
    }
	/* head should never be NULL. a new list is allocated with newdomnode */
    assert(head != NULL);
    if((srv_pri = strchr(ipaddr, (int)'-')) != NULL) {
        *srv_pri = '\0';
        srv_pri++;
    }
	
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    memset(&addr.sin6_addr,0,sizeof(addr.sin6_addr));
    if (!inet_pton(AF_INET,ipaddr, &addr.sin6_addr)) {
	    if (!inet_pton(AF_INET6,ipaddr, &addr.sin6_addr)) {
            return NULL;
     }
     } else {
        SC_IPV4_ADDR_CONVERT(&addr.sin6_addr);
    }
    if(IN6_IS_ADDR_V4COMPAT(&addr.sin6_addr)){
        SC_IPV4_ADDR_TO_IPV6(&addr.sin6_addr);  // e.g. covert 192.168.0.145 to ::ffff:192.168.0.145
    }
    p = alloc_srvnode();
    memcpy(&p->addr.sin6_addr, &addr.sin6_addr, sizeof(p->addr.sin6_addr));
    p->inactive = 0;
    if(srv_pri) {
        p->srv_pri = atoi(srv_pri);
        if (p->srv_pri & 0x10)
            p->local = 0;
        else
            p->local = 1;
        p->srv_pri = p->srv_pri & 0x0f;
    }
    else
        p->local = 1;
    if(inf_ip)
    {
        if (inet_pton(AF_INET,inf_ip, &addr.sin6_addr)) {
            SC_IPV4_ADDR_CONVERT(&addr.sin6_addr);
            if(IN6_IS_ADDR_V4COMPAT(&addr.sin6_addr))
                SC_IPV4_ADDR_TO_IPV6(&addr.sin6_addr);  // e.g. covert 192.168.0.145 to ::ffff:192.168.0.145
	    memcpy(&p->inf_addr.sin6_addr, &addr.sin6_addr, sizeof(p->addr.sin6_addr));
	} 
        else 
        {
	    if(inet_pton(AF_INET6,inf_ip, &addr.sin6_addr)) {
	        memcpy(&p->inf_addr.sin6_addr, &addr.sin6_addr, sizeof(p->addr.sin6_addr));
            }
        }
    } 
#else
	p = alloc_srvnode();
        if(!p)
            return NULL;
        if (inet_aton(ipaddr, &addr.sin_addr)) {
	    memcpy(&p->addr.sin_addr, &addr.sin_addr, sizeof(p->addr.sin_addr));
	}
        else
        {
            free(p);
            return NULL;
        }
	p->inactive = 0;
	if(srv_pri) {
		p->srv_pri = atoi(srv_pri);
        if (p->srv_pri & 0x10)
            p->local = 0;
        else
            p->local = 1;
        p->srv_pri = p->srv_pri & 0x0f;
	}
        else
            p->local = 1;
	if(inf_ip)
	{
	    if (inet_aton(inf_ip, &addr.sin_addr)) {
		memcpy(&p->inf_addr.sin_addr, &addr.sin_addr, sizeof(p->addr.sin_addr));
	    }
	}
#endif
	if(inf && strlen(inf))
	{
		if((inf_pri = strchr(inf, (int)'-')) != NULL) {
			*inf_pri = '\0';
			inf_pri++;
			p->inf_pri = atoi(inf_pri);
		}
		strncpy((char *)&p->inf, inf, (sizeof(p->inf) - 1));
	}
	if(wan_id) {
		p->wan_id = atoi(wan_id);
	}
	if(time_out) {
		p->time_out = atoi(time_out);
	}
    
    cur_inf_pri = p->inf_pri;
    cur_srv_pri = p->srv_pri;
    
    while(psrv->next != head)
    {
        if(psrv->next->inf_pri == cur_inf_pri)
        {
            if(psrv->next->srv_pri >= cur_srv_pri) {
                ins_srvnode(psrv, p);
                break;
            }
        }
        if(psrv->next->inf_pri > cur_inf_pri)
        {
            ins_srvnode(psrv, p);
            break;
        }
        psrv = psrv->next;
    }
    if(psrv->next == head)
        ins_srvnode(psrv, p);
    
    if(srv_pri) {
		srv_pri--;
		*srv_pri = '-';
	}
	if(inf_ip) {
		inf_ip--;
		*inf_ip = '#';
	}
	if(inf) {
		if(inf_pri) {
			inf_pri--;
			*inf_pri = '-';
		}
		inf--;
		*inf = '#';
	}
	if(wan_id) {
		wan_id--;
		*wan_id = '#';
	}
	if(time_out) {
		time_out--;
		*time_out = '#';
	}
    return p;
}
#endif
/* add a server.*/
srvnode_t *add_srv(srvnode_t *head, const char *ipaddr) {
	srvnode_t *p;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
	struct sockaddr_in6 addr;
#else
	struct sockaddr_in addr;
#endif
#ifdef SCM_BINDING
	char *srv_pri = NULL;
	char *inf_ip = NULL, *inf = NULL, *inf_pri = NULL;
	char *wan_id = NULL;
	char *time_out = NULL;
	
	if((inf_ip = strchr(ipaddr, (int)'#')))
	{
		*inf_ip = '\0';
		inf_ip++;
		inf = strchr(inf_ip, (int)'#');
	}
	if(inf)
	{
		*inf = '\0';
		inf++;
		wan_id = strchr(inf, (int)'#');
	}
	if(wan_id)
	{
		*wan_id = '\0';
		wan_id++;
		time_out = strchr(inf, (int)'#');
	}
	if(time_out)
	{
		*time_out = '\0';
		time_out++;
	}
#endif
	/* head should never be NULL. a new list is allocated with newdomnode */
	assert(head != NULL);
#ifdef SCM_BINDING
	if((srv_pri = strchr(ipaddr, (int)'-')) != NULL) {
		*srv_pri = '\0';
		srv_pri++;
	}
	
#endif
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
	if (!inet_pton(AF_INET6,ipaddr, &addr.sin6_addr)) {
		return NULL;
	}
	p = alloc_srvnode();
	memcpy(&p->addr.sin6_addr, &addr.sin6_addr, sizeof(p->addr.sin6_addr));
#else
	if (!inet_aton(ipaddr, &addr.sin_addr)) {
		return NULL;
	}
	p = alloc_srvnode();
	memcpy(&p->addr.sin_addr, &addr.sin_addr, sizeof(p->addr.sin_addr));
#endif
	p->inactive = 0;
#ifdef SCM_BINDING
	if(srv_pri) {
		p->srv_pri = atoi(srv_pri);
	}
	if(inf_ip)
	{
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
		if (inet_pton(AF_INET6,inf_ip, &addr.sin6_addr)) {
				memcpy(&p->inf_addr.sin6_addr, &addr.sin6_addr, sizeof(p->addr.sin6_addr));
		}
#else
		if (inet_aton(inf_ip, &addr.sin_addr)) {
				memcpy(&p->inf_addr.sin_addr, &addr.sin_addr, sizeof(p->addr.sin_addr));
		}
#endif
	}
	if(inf && strlen(inf))
	{
		if((inf_pri = strchr(ipaddr, (int)'-')) != NULL) {
			*inf_pri = '\0';
			inf_pri++;
			p->inf_pri = atoi(inf_pri);
		}
		strncpy((char *)&p->inf, inf, (sizeof(p->inf) - 1));
	}
	if(wan_id) {
		p->wan_id = atoi(wan_id);
	}
	if(time_out) {
		p->time_out = atoi(time_out);
	}
#endif
	ins_srvnode(head, p);
#ifdef SCM_BINDING
	if(srv_pri) {
		srv_pri--;
		*srv_pri = '-';
	}
	if(inf_ip) {
		inf_ip--;
		*inf_ip = '#';
	}
	if(inf) {
		if(inf_pri) {
			inf_pri--;
			*inf_pri = '-';
		}
		inf--;
		*inf = '#';
	}
	if(wan_id) {
		wan_id--;
		*wan_id = '#';
	}
	if(time_out) {
		time_out--;
		*time_out = '#';
	}
#endif
	return p;
}

/* returns the last srvnode in the list */
srvnode_t *last_srvnode(srvnode_t *head) {
  srvnode_t *p = head;
  /* head should always be != NULL */
  assert(p != NULL);
  while (p->next != head) p = p->next;
  return p;
}

/* check if there are a list or not 
   retruns 1 if its empty or NULL
   returns 0 if there are servers in the list
*/
int no_srvlist(srvnode_t *head) {
  if (!head) return 1;
  return (head->next == head);
}

