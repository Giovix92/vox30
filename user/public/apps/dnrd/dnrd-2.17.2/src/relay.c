/*
 * relay.c - the guts of the program.
 *
 * Copyright (C) 1998 Brad M. Garcia <garsh@home.com>
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
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "query.h"
#include "relay.h"
#include "cache.h"
#include "common.h"
#include "tcp.h"
#include "udp.h"
#include "dns.h"
#include "domnode.h"
#include "clsock.h"
#ifndef EXCLUDE_MASTER
#include "master.h"
#endif
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
extern void mbug(char *format, ...);
#define printf mbug


/* prepare the dns packet for a not found reply */
/* not used anymore
char *set_notfound(char *msg, const int len) {
  if (len < 4) return NULL;
  msg[2] |= 0x84;
  msg[3] = 0x83;
  return msg;
}
*/

/* prepare the dns packet for a Server Failure reply */
char *set_srvfail(char *msg, const int len) {
  if (len < 4) return NULL;
  /* FIXME: host to network should be called here */
  /* Set flags QR and AA */
  msg[2] |= 0x84;
  /* Set flags RA and RCODE=3 */
  msg[3] = 0x82;
  return msg;
}



/*
 * handle_query()
 *
 * In:      fromaddrp - address of the sender of the query.
 *
 * In/Out:  msg       - the query on input, the reply on output.
 *          len       - length of the query/reply
 *
 * Out:     dptr      - dptr->current contains the server to which to forward the query
 *
 * Returns:  -1 if the query is bogus
 *           1  if the query should be forwarded to the srvidx server
 *           0  if msg now contains the reply
 *
 * Takes a single DNS query and determines what to do with it.
 * This is common code used for both TCP and UDP.
 *
 * Assumptions: There is only one request per message.
 */
#ifdef __SC_BUILD__
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
int handle_query(const struct sockaddr_in6 *fromaddrp, char *msg, int *len,
		 domnode_t **dptr, int dtype)
#else
int handle_query(const struct sockaddr_in *fromaddrp, char *msg, int *len,
		 domnode_t **dptr, int dtype)
#endif
#else
int handle_query(const struct sockaddr_in *fromaddrp, char *msg, int *len,
		 domnode_t **dptr)
#endif

{
    int       replylen;
    domnode_t *d;
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    char buf[64];
#endif
#ifdef __SC_BUILD__
  //  char ip[64];
    int local = 0;
//#ifdef CONFIG_SUPPORT_IPV6
    //if (0 != strcmp("::1", inet_ntop(AF_INET6,&fromaddrp->sin6_addr, ip,sizeof(ip))))
//#else
    if (0 == strcmp("127.0.0.1", inet_ntoa(fromaddrp->sin_addr)))
//#endif
    {
        local = 1;
    }
#endif
    if (opt_debug) {
		char      cname_buf[256];
		sprintf_cname(&msg[12], *len-12, cname_buf, 256);
		log_debug(3, "Received DNS query for \"%s\"", cname_buf);
		if (dump_dnspacket("query", (unsigned char *)msg, *len) < 0)
			log_debug(3, "Format error");
    }
#ifdef __SC_BUILD__
    char cname_buf[256] = {0};
    char *p = NULL;
    char buf_tmp[512] = {0};
    char *q = buf_tmp;
    sprintf_cname(&msg[12], *len-12, cname_buf, 256);
    p = cname_buf;
    while(*p)
    {
        if(*p == '%')
        {
            *q++ = '%';
            *q++ = '%';
        }
        else
            *q++ = *p;
        p++;
    }
    log_dns(LOG_DEBUG, NORM_LOG,LOG_NONUSE_ID,LOG_NONUSE_BLOCK_TIME,"Received DNS query for \"%s\"\n", buf_tmp);
#endif
   
#ifndef EXCLUDE_MASTER
#if (defined(__SC_BUILD__) && ((defined(CONFIG_SUPPORT_WEB_PRIVOXY)) || (defined(CONFIG_SUPPORT_FON)) || (defined(CONFIG_SUPPORT_WEB_URL_FILTER))))
    if ((replylen = master_lookup((unsigned char *)msg, *len,fromaddrp)) != 0) {
        log_debug(2, "Replying to query as master");
        if(replylen > 0)
        {
            *len = replylen;
            return 0;
        }
        else
            return -1;
    }
#else
    /* First, check to see if we are master server */
    if ((replylen = master_lookup((unsigned char *)msg, *len)) > 0) {
	log_debug(2, "Replying to query as master");
	*len = replylen;
	return 0;
    }
#endif
#endif


    /* Next, see if we have the answer cached */
    if ((replylen = cache_lookup(msg, *len)) > 0) {
	log_debug(3, "Replying to query with cached answer.");
	*len = replylen;
	return 0;
    }
    
    /* get the server list for this domain */
#ifdef __SC_BUILD__
    d = search_subdomnode(domain_list, &msg[12], *len, dtype, fromaddrp);
#else
    d = search_subdomnode(domain_list, &msg[12], *len);
#endif
	if (no_srvlist(d->srvlist)) {
                /* there is no servers for this domain, reply with "Server failure" */
#ifdef __SC_BUILD__
        if(d == domain_list) // not match any domain
        {
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_REDIRECT))
        /*no match servers for this domain, check if need to send redirect*/
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
            if (((strcmp("::1", inet_ntop(AF_INET6,&fromaddrp->sin6_addr,buf,sizeof(buf)))) &&
                    (strcmp("::ffff:127.0.0.1", inet_ntop(AF_INET6,&fromaddrp->sin6_addr,buf,sizeof(buf))))) &&
                (replylen = send_redirect((unsigned char *)msg, *len)) > 0)
#else
            if ((strcmp("127.0.0.1", inet_ntoa(fromaddrp->sin_addr))) &&
                    (replylen = send_redirect((unsigned char *)msg, *len)) > 0)
#endif
            {
                *len = replylen;
                return 0;
            }
#endif
        }
        log_dns(LOG_ERR, NORM_LOG, LOG_DNS_QUERY_ID, 360, "DNS query failed, no active DNS server\n");
        return -1;
#else
		log_debug(2, "Replying to query with \"Server failure\"");
		if (!set_srvfail(msg, *len)) return -1;
		    return 0;
#endif
    }
    if(d == domain_list) // not match any domain
    {
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_REDIRECT))
        /*no match servers for this domain, check if need to send redirect*/
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
        if (((strcmp("::1", inet_ntop(AF_INET6,&fromaddrp->sin6_addr,buf,sizeof(buf)))) &&
                    (strcmp("::ffff:127.0.0.1", inet_ntop(AF_INET6,&fromaddrp->sin6_addr,buf,sizeof(buf))))) &&
                (replylen = send_redirect((unsigned char *)msg, *len)) > 0)
#else
            if ((strcmp("127.0.0.1", inet_ntoa(fromaddrp->sin_addr))) &&
                    (replylen = send_redirect((unsigned char *)msg, *len)) > 0)
#endif
            {
                *len = replylen;
                return 0;
            }
#endif
    }
#ifdef __SC_BUILD__
    if(dtype != -1)
    {
        if(d->domain == NULL)
        {
            log_dns(LOG_ERR, NORM_LOG, LOG_DNS_QUERY_ID, 360, "DNS query failed, no active DNS server for the the specific domain\n");
            return -1;
        }
    }
     /* find the first active server */
#ifdef FAILOVER_ALG1
start:
#endif
    d->current=NULL;
    d->current_bk=NULL;
    d->current = next_active(d, local);
#else 
    if (d->roundrobin) set_current(d, next_active(d));
    /* Send to a server until it "times out". */
    if (d->current) {
      time_t now = time(NULL);
      if ((d->current->send_time != 0) 
	  && (forward_timeout != 0)
	  && (reactivate_interval != 0)
	  && (now - d->current->send_time > forward_timeout)) {
	deactivate_current(d);
      }
    }
#endif

    if (d->current) {
#ifdef __SC_BUILD__       
        /* Find does this srvlist exist backup srvnode which 
         * interface priortiy same with current srvnode */
        
        srvnode_t *p = d->srvlist->next;
        while(p != d->srvlist)
        {
            if(p == d->current)
            {
                p = p->next;
                continue;
            }
#ifdef FAILOVER_ALG1
            if((p->inf_pri == d->current->inf_pri) && ((DOMAIN_TYPE_UNKNOWN != d->domain_type) || (local == p->local)))
#else
            if((p->inactive == 0) && (p->inf_pri == d->current->inf_pri) && 
               (p->srv_pri == d->current->srv_pri)
               )
#endif
            {
                d->current_bk = p;
                break;
            }
            p = p->next;
        }
#else
	log_debug(3, "Forwarding the query to DNS server %s",
		          inet_ntoa(d->current->addr.sin_addr));
#endif 
    } else {
#ifdef __SC_BUILD__
#ifdef FAILOVER_ALG1
       d->current = d->srvlist->next;
       if(d->srvlist->next->next != d->srvlist)
       {
           d->current_bk = d->srvlist->next->next;
       }
       goto end;
#endif
       return -1;
#else
	log_debug(3, "All servers deactivated. Replying with \"Server failure\"");
	if (!set_srvfail(msg, *len)) return -1;
	    return 0;
#endif

    }
#ifdef FAILOVER_ALG1
end:
#endif
    *dptr = d;
    return 1;
}

/* Check if any deactivated server are back online again */

static void reactivate_servers(int interval) {
  time_t now=time(NULL);
  static int last_try = 0;
  domnode_t *d = domain_list;
  /*  srvnode_t *s;*/

  if (!last_try) last_try = now;
  /* check for reactivate servers */
  if ( (now - last_try < interval) || no_srvlist(d->srvlist)  ) 
    return;
 
  last_try = now;
  do {
    retry_srvlist(d, interval );
    if (!d->roundrobin) {
      /* find the first active server in serverlist */
      d->current=NULL;
#ifndef __SC_BUILD__       
      d->current=next_active(d);
#endif
    }
  } while ((d = d->next) != domain_list);  
}

void srv_stats(time_t interval) {
  srvnode_t *s;
  domnode_t *d=domain_list;
  time_t now = time(NULL);
  static time_t last=0;
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  char buf[64];
#endif  
  if (last + interval > now) {
    last = now;
    do {
      if ((s=d->srvlist)) 
	while ((s=s->next) != d->srvlist)
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
	  log_debug(1, "stats for %s: send count=%i",
		    inet_ntop(AF_INET6,&s->addr.sin6_addr,buf,sizeof(buf)), s->send_count);
#else
	  log_debug(1, "stats for %s: send count=%i",
		    inet_ntoa(s->addr.sin_addr), s->send_count);
#endif
    } while ((d=d->next) != domain_list);
  }
}


/*
 * run()
 *
 * Abstract: This function runs continuously, waiting for packets to arrive
 *           and processing them accordingly.
 */
void run()
{
  struct timeval     tout;
  fd_set             fdread;
  int                retn;
  /*
  domnode_t          *d = domain_list;
  srvnode_t          *s;
  */
    /*    int                i, j;*/

  FD_ZERO(&fdmaster);
  FD_SET(isock,   &fdmaster);
#ifdef ENABLE_TCP
  FD_SET(tcpsock, &fdmaster);
  maxsock = (tcpsock > isock) ? tcpsock : isock;
#else
  maxsock = isock;
#endif

#ifdef SCM_BINDING
  FD_SET(csock, &fdmaster);
  maxsock = (csock > maxsock) ? csock : maxsock;
#endif


  while(1) {
      query_t *q;
      tout.tv_sec  = select_timeout;
      tout.tv_usec = 0;
      fdread = fdmaster;

      /* Wait for input or timeout */
      retn = select(maxsock+1, &fdread, 0, 0, &tout);

      /* reactivate servers */
#ifndef __SC_BUILD__
      if (reactivate_interval != 0) 
          reactivate_servers(reactivate_interval);
#endif
      /* Handle errors */
      if (retn < 0) {
          log_msg(LOG_ERR, "select returned %s", strerror(errno));
          continue;
      }
     else if (retn != 0) {
          for (q = &qlist; q->next != &qlist; q = q->next) {
              if (FD_ISSET(q->next->sock, &fdread)) {
                  udp_handle_reply(q);
              }
#ifdef __SC_BUILD__
              else
              {
                  if(q->next->done == 1)
		  {
                      query_delete_next(q);
                  }
              }
#endif
          }
#ifdef SCM_BINDING          
          if (FD_ISSET(csock, &fdread)) {
            cls_handle(); 
          }
#endif
          
#ifdef ENABLE_TCP
          /* Check for incoming TCP requests */
          if (FD_ISSET(tcpsock, &fdread)) tcp_handle_request();
#endif
          /* Check for new DNS queries */
          if (FD_ISSET(isock, &fdread)) {

              q = udp_handle_request();
              if (q != NULL) {
              }
          }
      } else {
          /* idle */
      }

      /* ok, we are done with replies and queries, lets do some
         maintenance work */

      /* Expire lookups from the cache */
      cache_expire();
#ifndef EXCLUDE_MASTER
      /* Reload the master database if neccessary */
      master_reinit();
#endif
      /* Remove old unanswered queries */
      query_timeout(20);

      /* create new query/socket for next incomming request */
      /* this did not make the program run any faster
         d=domain_list;
         do {
         if ((s=d->srvlist)) 
         while ((s=s->next) != d->srvlist)
         if (s->newquery == NULL) 
         s->newquery = query_get_new(d, s);
         } while ((d=d->next) != domain_list);
         */
#ifndef SCM_BINDING
      /* print som query statestics */
      query_stats(10);
      srv_stats(10);
#endif
  }
}
