/*
 * query.c
 *
 * This is a complete rewrite of Brad garcias query.c
 *
 * This file contains the data definitions, function definitions, and
 * variables used to implement our DNS query list.
 *
 * Assumptions: No multithreading.
 *
 * Copyright (C) Natanael Copa <ncopa@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <syslog.h>
#define NDEBUG
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#include "domnode.h"
#endif
#include "lib.h"
#include "common.h"
#include "query.h"
#include "qid.h"
extern void mbug(char *format, ...);
#define printf mbug

/*Not defined, for later use*/
#ifdef SCM_TOS_CONFIG
#define ATATC_TOSVAL (0x10)
#endif

query_t qlist; /* the active query list */
static query_t *qlist_tail;
static unsigned long total_queries=0;
static unsigned long total_timeouts=0;

int upstream_sockets = 0; /* number of upstream sockets */

static int dropping = 0; /* dropping new packets */

/* init the query list */
void query_init() {
  qlist_tail = (qlist.next = &qlist);
}


/* create a new query, and open a socket to the server */
query_t *query_create(domnode_t *d, srvnode_t *s) {
  query_t *q;
#ifdef RANDOM_SRC
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  struct sockaddr_in6 my_addr;
#else
  struct sockaddr_in my_addr;
#endif
#endif
#ifdef SCM_BINDING
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  struct sockaddr_in6 in_addr;
#else
  struct sockaddr_in in_addr;
#endif
  struct ifreq interface;
#endif

  /* should never be called with no server */
  assert(s != NULL);

  /* check if we have reached maximum of sockets */
  if (upstream_sockets >= max_sockets) {
    if (!dropping)
#ifdef __SC_BUILD__
      log_dns(LOG_WARNING, NORM_LOG, LOG_DNS_QUERY_ID, 360, "DNS query failed, exceed the max socket number\n");
#else
      log_msg(LOG_WARNING, "Socket limit reached. Dropping new queries");
#endif
    return NULL;
  }

  dropping=0;
  /* allocate */
  if ((q=(query_t *) allocate(sizeof(query_t))) == NULL)
  {
#ifdef __SC_BUILD__
      log_dns(LOG_WARNING, NORM_LOG, LOG_DNS_QUERY_ID, 360, "DNS query failed, resouce is limited\n");
#endif

      return NULL;
  }
  /* return an emtpy circular list */
  q->next = (struct _query *)q;

  /* we specify both domain and server */
  /* the dummy queries will use server but no domain is attatched */
  q->domain = d;
  q->srv = s;

  /* set the default time to live value */
#ifdef SCM_BINDING
  if(q->srv->time_out != QUERY_INVALID_TIMEOUT)
    q->ttl = q->srv->time_out;
  else
#endif
    q->ttl = forward_timeout;
  
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  /* open a new socket */
  if ((q->sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
#else
  if ((q->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
#endif
#ifdef __SC_BUILD__
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_QUERY_ID, 360, "DNS query failed, internal error\n");
#else
      log_msg(LOG_ERR, "query_create: Couldn't open socket");
#endif
      free(q);
      return NULL;
  } else upstream_sockets++;

#ifdef SCM_TOS_CONFIG
  {          
      int optval = ATATC_TOSVAL;

      if(setsockopt(q->sock, IPPROTO_IP, IP_TOS, &optval, sizeof(optval)) < 0)
      {
          perror("setsockopt:");
      }
  }
#endif  

#ifdef SCM_BINDING
#if defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY))
  if(!q->srv->inf_addr.sin6_addr.s6_addr)
      q->srv->inf_addr.sin6_addr.s6_addr = in6addr_any;
#else
  if(!q->srv->inf_addr.sin_addr.s_addr)
     q->srv->inf_addr.sin_addr.s_addr = INADDR_ANY;
#endif
  
    {
	  int i;

	  if(strlen(q->srv->inf))
	  {
		  strncpy(interface.ifr_ifrn.ifrn_name, q->srv->inf, IFNAMSIZ);
#ifdef CONFIG_SUPPORT_HA
                  if(strcmp((char *)&interface, "dummy0") != 0)
#endif
		  if (setsockopt(q->sock, SOL_SOCKET, SO_BINDTODEVICE,(char *)&interface, sizeof(interface)) < 0) {
			  close(q->sock);
			  free(q);
			  upstream_sockets--;
			  return NULL;
		  }
	  }

	  memset(&in_addr, 0, sizeof(in_addr));
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
	  in_addr.sin6_family = AF_INET6;
          memcpy(&in_addr.sin6_addr,&q->srv->inf_addr.sin6_addr, sizeof(in_addr.sin6_addr));
	  for(i = 0; i < 5; i++)
	  {
		  in_addr.sin6_port = htons( myrand(65536-1026)+1025 );
		  if (bind(q->sock, (struct sockaddr *)&in_addr,
			   sizeof(in_addr)) != -1) {
			   break;
		  }
      }
#else
	  in_addr.sin_family = AF_INET;
	  in_addr.sin_addr.s_addr = q->srv->inf_addr.sin_addr.s_addr;
	  for(i = 0; i < 5; i++)
	  {
		  in_addr.sin_port = htons( myrand(65536-1026)+1025 );
		  if (bind(q->sock, (struct sockaddr *)&in_addr,
			   sizeof(struct sockaddr)) != -1) {
			   break;
		  }
      }
#endif
	  if(i >= 5)
	  {
		  close(q->sock);
		  free(q);
		  upstream_sockets--;
#ifdef __SC_BUILD__
         log_dns(LOG_ERR, NORM_LOG, LOG_DNS_QUERY_ID, 360, "DNS query failed, internal error\n");
#endif

		  return NULL;
	  }
  }
#else
  /* bind to random source port */
#ifdef RANDOM_SRC
  memset(&my_addr, 0, sizeof(my_addr));
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  my_addr.sin6_family = AF_INET6;
  my_addr.sin6_addr = in6addr_any;
  my_addr.sin6_port = htons( myrand(65536-1026)+1025 );
#else
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY;
  my_addr.sin_port = htons( myrand(65536-1026)+1025 );
#endif
  if (bind(q->sock, (struct sockaddr *)&my_addr, 
	   sizeof(struct sockaddr)) == -1) {
    log_msg(LOG_WARNING, "bind: %s", strerror(errno));
  }
#endif
#endif
  /* add the socket to the master FD set */
  FD_SET(q->sock, &fdmaster);
  if (q->sock > maxsock) maxsock = q->sock;

  /* get an unused QID */
  q->my_qid = qid_get();
  return q;
}

query_t *query_destroy(query_t *q) {
  /* close the socket and return mem */
  qid_return(q->my_qid);

  /* unset the socket */
  FD_CLR(q->sock, &fdmaster);
  close(q->sock);

  upstream_sockets--;
  total_queries++;
  free(q);
  return NULL;
}

/* Get a new query */
query_t *query_get_new(domnode_t *dom, srvnode_t *srv) {
  query_t *q;
  assert(srv != NULL);
  /* if there are no prepared queries waiting for us, lets create one */
  if ((q=srv->newquery) == NULL) {
    if ((q=query_create(dom, srv)) == NULL) return NULL;
  }
  srv->newquery = NULL;
  q->domain=dom;
  q->srv = srv;
#ifdef __SC_BUILD__
  q->done = 0;
#endif

  return q;
}


/* get qid, rewrite and add to list. Retruns the query before the added  */
#ifdef __SC_BUILD__
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
query_t *query_add(domnode_t *dom, srvnode_t *srv, 
		   const struct sockaddr_in6* client, char* msg, 
		   unsigned len, int dtype) {
#else
query_t *query_add(domnode_t *dom, srvnode_t *srv, 
		   const struct sockaddr_in* client, char* msg, 
		   unsigned len, int dtype) {
#endif
#else
query_t *query_add(domnode_t *dom, srvnode_t *srv, 
		   const struct sockaddr_in* client, char* msg, 
		   unsigned len) {
#endif

  query_t *q, *p, *oldtail;
  unsigned short client_qid = *((unsigned short *)msg);
  time_t now = time(NULL);

  /* 
     look if the query are in the list 
     if it is, don't add it again. 
  */
  for (p=&qlist; p->next != &qlist; p = p->next) {
    if ((p->next->client_qid == client_qid) 
#ifdef __SC_BUILD__
	  && (p->next->srv == srv)
      && (p->next->domain == dom)
#endif
      ) {
      /* we found the qid in the list */
      *((unsigned short *)msg) = htons(p->next->my_qid);
      p->next->client_time = now;
#ifdef __SC_BUILD__
      p->next->client_count++;
      p->next->master = srv;
#endif
      return p;
    }
  }
  
  if ((q=query_get_new(dom, srv))==NULL) {
    /* if we could not allocate any new query, return with NULL */
    return NULL;
  }

  q->client_qid = client_qid;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  memcpy(&(q->client), client, sizeof(struct sockaddr_in6));
#else
  memcpy(&(q->client), client, sizeof(struct sockaddr_in));
#endif
  q->client_time = now;
  q->client_count = 1;
#ifdef __SC_BUILD__
  q->try_count = 0;
  q->master = srv;
#endif
#ifdef SCM_BINDING
  q->dtype = dtype;
#endif
  /* set new qid from random generator */
  *((unsigned short *)msg) = htons(q->my_qid);

  /* add the query to the list */
  q->next = qlist_tail->next;
  qlist_tail->next = q;

  /* new query is new tail */
  oldtail = qlist_tail;
  qlist_tail = q;
  return oldtail;

}

/* remove query after */
query_t *query_delete_next(query_t *q) {
  query_t *tmp = q->next;
  /* unlink tmp */
  q->next = q->next->next;
 
  /* if this was the last query in the list, we need to update the tail */
  if (qlist_tail == tmp) {
    qlist_tail = q;
  }

  /* destroy query */
  query_destroy(tmp);
  return q;
}

#ifdef __SC_BUILD__
void query_delete_by_domain(domnode_t *domain) {
  query_t *q;
  q=&qlist;
  while( q->next != &qlist) {
    if (q->next->domain == domain) 
    {
        query_delete_next(q);
        continue;
    } 
    q = q->next;
  }
}
void query_delete_by_wan_id(int wan_id) {
  query_t *q;
  
  q=&qlist;
  while(q->next != &qlist) {
    if (q->next->master &&  (q->next->master->wan_id == wan_id))
    {
        query_delete_next(q);
        continue;
    }
    q = q->next;
  }
}
void query_delete_by_domain_wan_id(domnode_t *domain, int wan_id) {
  query_t *q;
  
  q=&qlist;
  while (q->next != &qlist) {
    if (q->next->master && (q->next->master->wan_id == wan_id) && (q->next->domain == domain) )
    {
        query_delete_next(q);
        continue;
    }
    q = q->next;
  }
}
#endif
/* remove old unanswered queries */
void query_timeout(time_t age) {
  int count=0;
  time_t now = time(NULL);
  query_t *q;
  
  for (q=&qlist; q->next != &qlist; q = q->next) {
#ifdef __SC_BUILD__
    if(q->next->done != 1)
    {
	if (q->next->client_time < (now - q->next->ttl / 2)) {
            if(q->next->try_count >= ALL_SERVER_FAILED)
	    {
                if (q->next->client_time < (now - q->next->ttl)) {
		    count++;
#ifdef FAILOVER_ALG1
            if(q->next->srv && q->next->srv->srv_pri == 0)
            {
                srvnode_t *s = NULL;
                for (s=q->next->domain->srvlist->next; s != q->next->domain->srvlist; s = s->next)
                {
                    if((s != q->next->srv) && (s->srv_pri == 1))
                    {
                        s->inactive = 0;
                    }
                }
            }
#endif
		    query_delete_next(q);
		    //   log_msg(LOG_ERR, "query timeout, del, client_time = %d, ttl = %d, now = %d\n", q->next->client_time, q->next->ttl, now);
		}
	    }
	    else
	    {
#ifdef FAILOVER_ALG1
            query_t *p;
            if(q->next->srv && q->next->srv->srv_pri == 0)
            {
                for(p = &qlist; p->next != &qlist; p = p->next)
                {
                    if(p->next->client_qid == q->next->client_qid)
                    {
                        send2current(p->next, p->next->msg, p->next->msg_len);
                    }
                }
            }
#else
		   //log_msg(LOG_ERR, "query timeout, send to another, client_time = %d, ttl = %d, now = %d\n", q->next->client_time, q->next->ttl, now);
		send2current(q->next, q->next->msg, q->next->msg_len);
#endif
	    }
	}
    else
    {
    }
    }
    else
    {
        count++;
#ifdef FAILOVER_ALG1
        if(q->next->srv  && q->next->srv->srv_pri == 0)
        {
            srvnode_t *s = NULL;
            for (s=q->next->domain->srvlist->next; s != q->next->domain->srvlist; s = s->next)
            {
                if((s != q->next->srv) && (s->srv_pri == 1))
                {
                    s->inactive = 0;
                }
            }
        }
#endif
        query_delete_next(q);
    }
#else
    if (q->next->client_time < (now - q->next->ttl)) {
      count++;
      query_delete_next(q);
    }
#endif
  }
  if (count) log_debug(1, "query_timeout: removed %d entries", count);
  total_timeouts += count;
}

int query_count(void) {
  int count=0;
  query_t *q;
  
  for (q=&qlist; q->next != &qlist; q = q->next) {
    count++;
  }
  return count;
}



void query_dump_list(void) {
  query_t *p;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  char buf[64];
    for (p=&qlist; p->next != &qlist; p=p->next) {
      log_debug(2, "srv=%s, myqid=%i, client_qid=%i",
	      inet_ntop(AF_INET6,&p->next->srv->addr.sin6_addr,buf,sizeof(buf)), p->next->my_qid, 
	      p->next->client_qid);
  }
#else
  for (p=&qlist; p->next != &qlist; p=p->next) {
      log_debug(2, "srv=%s, myqid=%i, client_qid=%i",
	      inet_ntoa(p->next->srv->addr.sin_addr), p->next->my_qid, 
	      p->next->client_qid);
  }
#endif
}

/* print statics about the query list and open sockets */
void query_stats(time_t interval) {
  time_t now = time(NULL);
  int count = 0;
  static time_t last=0;
  if (last + interval < now) {
    last = now;
    log_debug(1, "Open sockets: %i, active: %i, count: %i, timeouts: %i", 
	      upstream_sockets, count=query_count(), total_queries, 
	      total_timeouts);
    if (count) query_dump_list();
  }
  
}
