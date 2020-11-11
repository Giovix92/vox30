/*
 * udp.c - handle upd connections
 *
 * Copyright (C) 1999 Brad M. Garcia <garsh@home.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#define NDEBUG
#include <assert.h>

#include "common.h"
#include "relay.h"
#include "cache.h"
#include "query.h"
#include "domnode.h"
#include "check.h"
#include "dns.h"

#ifndef EXCLUDE_MASTER
#include "master.h"
#endif
#include "clsock.h"
//extern void mbug(char *format, ...);
//#define printf mbug

/*
 * dnssend()						22OCT99wzk
 *
 * Abstract: A small wrapper for send()/sendto().  If an error occurs a
 *           message is written to syslog.
 *
 * Returns:  The return code from sendto().
 */
static int udp_send(int sock, srvnode_t *srv, void *msg, int len)
{
    int	rc;
    time_t now = time(NULL);
#ifdef __SC_BUILD__
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    char buf[64]; //for ipv6
        rc = sendto(sock, msg, len,  MSG_DONTWAIT,
		(const struct sockaddr *) &srv->addr,
		sizeof(srv->addr));
#else
    rc = sendto(sock, msg, len,  MSG_DONTWAIT,
		(const struct sockaddr *) &srv->addr,
		sizeof(struct sockaddr_in));
#endif
#else
    rc = sendto(sock, msg, len, 0,
		(const struct sockaddr *) &srv->addr,
		sizeof(struct sockaddr_in));
#endif
    if (rc != len) {
#ifdef __SC_BUILD__
#if (defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "send to %s, error: %s\n", inet_ntop(AF_INET6,&srv->addr.sin6_addr,buf,sizeof(buf)), strerror(errno));
#else
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "send to %s, error: %s\n", inet_ntoa(srv->addr.sin_addr), strerror(errno));
#endif
#else
      log_msg(LOG_ERR, "sendto %s error: %s ",
		inet_ntoa(srv->addr.sin_addr), strerror(errno));
#endif
	    return (rc);
    }
    if ((srv->send_time == 0)) srv->send_time = now;
        srv->send_count++;
    return (rc);
}

#ifdef __SC_BUILD__
int send2current_bk(query_t *q, void *msg, const int len) {
    /* If we have domains associated with our servers, send it to the
       appropriate server as determined by srvr */
  domnode_t *d;
  assert(q != NULL);
  assert(q->domain != NULL);

  d = q->domain;

  if (d == NULL)
  {
  	return 0;
  }
  
  q->try_count++;
  if((d->current_bk == NULL) || (udp_send(q->sock, d->current_bk, msg, len) != len)) {
       return 0;
  }
  else 
  {
      return len;
  }

}
#endif

int send2current(query_t *q, void *msg, const int len) {
    /* If we have domains associated with our servers, send it to the
       appropriate server as determined by srvr */
  domnode_t *d;
  assert(q != NULL);
  assert(q->domain != NULL);

  d = q->domain;

  /*
  	patched by chenyl(2005/0202):

  	Dr. Edward found a DNRD's bug:

  		/bin/dnrd: udp.c: 75: send2current: Assertion `q->domain != ((void *)0)' failed.

  		DNRD crashed due to this bug.
  */
#ifdef __SC_BUILD__ 
    if (d == NULL || NULL == d->current)
    {
  	return 0;
    }

    srvnode_t       *current;
   /* q->client_time = time(NULL);
    if(q->srv->time_out != QUERY_INVALID_TIMEOUT)
        q->ttl = q->srv->time_out;
    else
        q->ttl = forward_timeout;
*/
#ifndef FAILOVER_ALG1
    current = next_active_by_wan_id(d, q->master->wan_id, q->master->inf_pri, q->master->srv_pri);
    if(current && (1 == (q->try_count % 2)))
    {
	q->srv = current;
    }
    else
    {
	q->srv = q->master;
    }
#else
    q->srv = q->master;
#endif
    q->try_count++;
    if(NULL == q->srv || (udp_send(q->sock, q->srv, msg, len) != len))
	return 0;
    else
	return len;

#else
  while ((d->current != NULL) && (udp_send(q->sock, d->current, msg, len) != len)) {
    if (reactivate_interval) deactivate_current(d);
  }
  if (d->current != NULL) {
    return len;
  } else return 0;
}
#endif
}

/*
 * handle_udprequest()
 *
 * This function handles udp DNS requests by either replying to them (if we
 * know the correct reply via master, caching, etc.), or forwarding them to
 * an appropriate DNS server.
 */
query_t *udp_handle_request()
{
    unsigned           addr_len;
    int                len;
    const int          maxsize = UDP_MAXSIZE;
    static char        msg[UDP_MAXSIZE+4];
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    struct sockaddr_in6 from_addr;
    char buf[64];
#else
    struct sockaddr_in from_addr;
#endif
    int                fwd;
    domnode_t          *dptr;
    query_t *q, *prev;
#ifdef SCM_BINDING    
    int dtype;
    int ret1 = 0; 
    int ret2 = 0;
#endif

#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    /* Read in the message */
    addr_len = sizeof(struct sockaddr_in6);
#else
    addr_len = sizeof(struct sockaddr_in);
#endif
#ifdef __SC_BUILD__
    len = recvfrom(isock, msg, maxsize, MSG_DONTWAIT,
		   (struct sockaddr *)&from_addr, &addr_len);
#else
    len = recvfrom(isock, msg, maxsize, 0,
		   (struct sockaddr *)&from_addr, &addr_len);
#endif
    if (len < 0) {
		log_debug(1, "recvfrom error %s", strerror(errno));
		return NULL;
    }

    /* do some basic checking */
    if (check_query(msg, len) < 0) return NULL;
    
#ifdef SCM_BINDING
    /* check and change query packet, does this packet have special char 
     * Sercomm changed special domain name like below
     * www.sercomm.com.ww2w1
     * */
    dtype = sc_dtype_parse(msg, &len);
#endif

    /* Determine how query should be handled */
#ifdef __SC_BUILD__
    if ((fwd = handle_query(&from_addr, msg, &len, &dptr, dtype)) < 0)
#else
    if ((fwd = handle_query(&from_addr, msg, &len, &dptr)) < 0)
#endif
    {
      return NULL; /* if its bogus, just ignore it */
    }
    /* If we already know the answer, send it and we're done */
    if (fwd == 0) {
#ifdef SCM_BINDING
        sc_dns_answer_parse(dtype, msg, &len);
#endif
#ifdef __SC_BUILD__
        if (sendto(isock, msg, len, MSG_DONTWAIT, (const struct sockaddr *)&from_addr,
		           addr_len) != len) {
            log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "send to client failed, error: %s\n", strerror(errno));
        }
#else
        if (sendto(isock, msg, len, 0, (const struct sockaddr *)&from_addr,
		           addr_len) != len) {
	    log_debug(1, "sendto error %s", strerror(errno));
        }

#endif
	return NULL;
    }

    /* dptr->current should never be NULL it is checked in handle_query */

    /* rewrite msg, get id and add to list*/
#ifdef __SC_BUILD__
    if ((prev=query_add(dptr, dptr->current, &from_addr, msg, len, dtype)) == NULL){
#else
    if ((prev=query_add(dptr, dptr->current, &from_addr, msg, len)) == NULL){
#endif
      /* of some reason we could not get any new queries. we have to
	 drop this packet */
      return NULL;
    }
    q = prev->next;
#ifdef __SC_BUILD__
    memcpy(q->msg, msg, sizeof(msg));
    q->msg_len = len;
    ret1 = send2current(q, msg, len); 
    /* Check if have current_bk and send to current_bk at same time */
    if(dptr->current_bk)
    {
            /* recover to original qurery qid */
            *((unsigned short *)msg) = q->client_qid;

            if((prev = query_add(dptr, dptr->current_bk, &from_addr, msg, len, dtype)) != NULL)
            {
	           memcpy(prev->next->msg, msg, sizeof(msg));
    		   prev->next->msg_len = len;
#ifdef FAILOVER_ALG1
               if(dptr->current_bk->inactive == 0)
               {
                   ret2 = send2current_bk(prev->next, msg, len);

               }
#else
     
               ret2 = send2current_bk(prev->next, msg, len);
#endif
            }
    }
    if (ret1 > 0 || ret2 > 0) {
#else
    if (send2current(q, msg, len) > 0) {
#endif
      /* add to query list etc etc */
      return q;
    } else {
    

      /* we couldn't send the query */
#ifndef EXCLUDE_MASTER
      int	packetlen;
      char	packet[maxsize+4];

      /*
       * If we couldn't send the packet to our DNS servers,
       * perhaps the `network is unreachable', we tell the
       * client that we are unable to process his request
       * now.  This will show a `No address (etc.) records
       * available for host' in nslookup.  With this the
       * client won't wait hang around till he gets his
       * timeout.
       * For this feature dnrd has to run on the gateway
       * machine.
       */
      
      if ((packetlen = master_dontknow((unsigned char *)msg, len, (unsigned char *)packet)) > 0) {
          if(prev)
              query_delete_next(prev);
          return NULL;
#ifdef __SC_BUILD__
          if (sendto(isock, msg, len,  MSG_DONTWAIT, (const struct sockaddr *)&from_addr,
                     addr_len) != len) {
              log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "send to client failed, error: %s\n", strerror(errno));
              return NULL;
          }
#else
          if (sendto(isock, msg, len, 0, (const struct sockaddr *)&from_addr,
                     addr_len) != len) {
              log_debug(1, "sendto error %s", strerror(errno));
              return NULL;
          }
#endif
      }
#endif
    }

    return q;
}

/*
 * dnsrecv()							22OCT99wzk
 *
 * Abstract: A small wrapper for recv()/recvfrom() with output of an
 *           error message if needed.
 *
 * Returns:  A positove number indicating of the bytes received, -1 on a
 *           recvfrom error and 0 if the received message is too large.
 */
static int reply_recv(query_t *q, void *msg, int len)
{
    int	rc, fromlen;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    char buf[64];//for ipv6
    struct sockaddr_in6 from;
    fromlen = sizeof(struct sockaddr_in6);
#else
    struct sockaddr_in from;
    fromlen = sizeof(struct sockaddr_in);
#endif
#ifdef __SC_BUILD__
    rc = recvfrom(q->sock, msg, len, MSG_DONTWAIT,
		  (struct sockaddr *) &from, (socklen_t *)&fromlen);
#else
    rc = recvfrom(q->sock, msg, len, 0,
		  (struct sockaddr *) &from, (socklen_t *)&fromlen);
#endif
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    if (rc == -1) {
		log_msg(LOG_ERR, "recvfrom error: %s",
			inet_ntop(AF_INET6,&q->srv->addr.sin6_addr,buf,sizeof(buf)));
		return (-1);
    }
    else if (rc > len) {
		log_msg(LOG_NOTICE, "packet too large: %s",
			inet_ntop(AF_INET6,&q->srv->addr.sin6_addr,buf,sizeof(buf)));
		return (0);
    }
    else if (memcmp(&from.sin6_addr, &q->srv->addr.sin6_addr,
		    sizeof(from.sin6_addr)) != 0) {
		log_msg(LOG_WARNING, "unexpected server: %s",
			inet_ntop(AF_INET6,&q->srv->addr.sin6_addr,buf,sizeof(buf)));
		return (0);
    }
#else
    if (rc == -1) {
		log_msg(LOG_ERR, "recvfrom error: %s",
			inet_ntoa(q->srv->addr.sin_addr));
		return (-1);
    }
    else if (rc > len) {
		log_msg(LOG_NOTICE, "packet too large: %s",
			inet_ntoa(q->srv->addr.sin_addr));
		return (0);
    }
    else if (memcmp(&from.sin_addr, &q->srv->addr.sin_addr,
		    sizeof(from.sin_addr)) != 0) {
		log_msg(LOG_WARNING, "unexpected server: %s",
			inet_ntoa(from.sin_addr));
		return (0);
    }
#endif
    return (rc);
}

/*
 * handle_udpreply()
 *
 * This function handles udp DNS requests by either replying to them (if we
 * know the correct reply via master, caching, etc.), or forwarding them to
 * an appropriate DNS server.
 *
 * Note that the mached query is prev->next and not prev.
 */
void udp_handle_reply(query_t *prev)
{
	//    const int          maxsize = 512; /* According to RFC 1035 */
	static char        msg[UDP_MAXSIZE+4];
	int                len;
	unsigned           addr_len;
	query_t *q = prev->next;
    int ret;
    int finish = 0;
#ifdef FAILOVER_ALG1
	query_t *p;
#endif
#ifdef SCM_BINDING
    unsigned short client_qid = 0;
#endif

	log_debug(3, "handling socket %i", q->sock);
	if ((len = reply_recv(q, msg, UDP_MAXSIZE)) < 0)
	{
		log_debug(1, "dnsrecv failed: %i", len);
		query_delete_next(prev);
		return; /* recv error */
	}

	/* do basic checking */
    ret = check_reply(q->srv, msg, len);
	if (ret < 0) {
		
#ifdef __SC_BUILD__
      if((q->try_count >= ALL_SERVER_FAILED) || (q->done == 1))
      {
#ifdef FAILOVER_ALG1
          if(ret == -1)
          {
              if(q->srv && q->srv->srv_pri == 0)
              {
                  srvnode_t *s = NULL;
                  for (s=q->domain->srvlist->next; s != q->domain->srvlist; s = s->next)
                  {
                      if((s != q->srv))
                      {
                          if(s->srv_pri == 1)
                          {
                              s->inactive = 0;
                          }
                      }
                  }
              }
          }
#endif
          if(ret == -1)
          {
              query_delete_next(prev);
              return;
          }
      }
      else
      {
#ifdef FAILOVER_ALG1
          if(q->srv && q->srv->srv_pri == 0)
          {
              for(p = &qlist; p->next != &qlist; p = p->next)
              {
                  if(p->next->client_qid == q->client_qid)
                  {
                      send2current(p->next, p->next->msg, p->next->msg_len);
                  }
              }
          }
#else
          send2current(q, q->msg, q->msg_len);
#endif
          return;
      }	
#else
        log_debug(1, "check_reply failed");
		query_delete_next(prev);
		return;
#endif
	}

	if (opt_debug) {
		char buf[256];
		sprintf_cname(&msg[12], len-12, buf, 256);
		log_debug(3, "Received DNS reply for \"%s\"", buf);
	}
	dump_dnspacket("reply", (unsigned char *)msg, len);
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
	addr_len = sizeof(struct sockaddr_in6);
#else
	addr_len = sizeof(struct sockaddr_in);
#endif
#ifdef __SC_BUILD__
	if ((q->domain != NULL) && (q->done != 1)) {
#else
	/* was this a dummy reactivate query? */
	if (q->domain != NULL) {
#endif
		/* no, lets cache the reply and send to client */
		cache_dnspacket(msg, len, q->srv);

		/* set the client qid */
		*((unsigned short *)msg) = q->client_qid;
#ifdef SCM_BINDING        
        sc_dns_answer_parse(q->dtype, msg, &len);
        client_qid = q->client_qid;
#endif
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
        {
        char buf[64]; //for ipv6
		log_debug(3, "Forwarding the reply to the host %s",
						inet_ntop(AF_INET6,&q->client.sin6_addr,buf,sizeof(buf)));
        }
#else
		log_debug(3, "Forwarding the reply to the host %s",
						inet_ntoa(q->client.sin_addr));
#endif
#ifdef __SC_BUILD__
	    if (sendto(isock, msg, len, MSG_DONTWAIT, 
		           (const struct sockaddr *)&q->client, addr_len) != len) {
            log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "send to client failed, error: %s\n", strerror(errno));
		}
#else
		if (sendto(isock, msg, len, 0, 
		           (const struct sockaddr *)&q->client, addr_len) != len) {
			log_debug(1, "sendto error %s", strerror(errno));
		}
#endif
	} else {
		log_debug(2, "We got a reactivation dummy reply. Cool!");
	}

	/* this server is obviously alive, we reset the counters */
	q->srv->send_time = 0;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
	{
    char buf[64];
    if (q->srv->inactive) log_debug(1, "Reactivating server %s",
					inet_ntop(AF_INET6,&q->srv->addr.sin6_addr,buf,sizeof(buf)));
    }
#else
	if (q->srv->inactive) log_debug(1, "Reactivating server %s",
					inet_ntoa(q->srv->addr.sin_addr));
#endif
	q->srv->inactive = 0;
#ifdef FAILOVER_ALG1
    if(q->domain)
    {
        if(q->srv && q->srv->srv_pri == 0)
        {
            srvnode_t *s = NULL;
            for (s=q->domain->srvlist->next; s != q->domain->srvlist; s = s->next)
            {
                if((s != q->srv) && (s->inactive == 0))
                {
                    if(s->srv_pri == 1)
                    {
                        s->inactive = time(NULL);
                    }
                }
            }
        }
    }
#endif
	/* remove query from list and destroy it */
	query_delete_next(prev);
#if SCM_BINDING
    if(client_qid)
    {
	    /* Need check whether have same client_qid and delete it */
	    for(q = &qlist; q->next != &qlist; q = q->next)
	    {
            if(q->next->client_qid == client_qid)
            {
                q->next->done = 1;
                break;
            }
	    }
    }
#endif 
}


/* send a dummy packet to a deactivated server to check if its back*/
int udp_send_dummy(srvnode_t *s) {
  static unsigned char dnsbuf[] = {
  /* HEADER */
    /* will this work on a big endian system? */
    0x00, 0x00, /* ID */
    0x00, 0x00, /* QR|OC|AA|TC|RD -  RA|Z|RCODE  */
    0x00, 0x01, /* QDCOUNT */
    0x00, 0x00, /* ANCOUNT */
    0x00, 0x00, /* NSCOUNT */
    0x00, 0x00, /* ARCOUNT */
    
    /* QNAME */
    9, 'l','o','c','a','l','h','o','s','t',0,
    /* QTYPE */
    0x00,0x01,   /* A record */
    
    /* QCLASS */
    0x00,0x01   /* IN */
  };
  query_t *q;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
  char buf[64]; //for ipv6
  struct sockaddr_in6 srcaddr;
#else
  struct sockaddr_in srcaddr;
#endif
  /* should not happen */
  assert(s != NULL);

#ifdef __SC_BUILD__
  if ((q=query_add(NULL, s, &srcaddr, (char *)dnsbuf, sizeof(dnsbuf), -1)) != NULL) {
#else
  if ((q=query_add(NULL, s, &srcaddr, (char *)dnsbuf, sizeof(dnsbuf))) != NULL) {
#endif
    int rc;
    q = q->next; /* query add returned the query 1 before in list */
    /* don't let those queries live too long */
    q->ttl = reactivate_interval;
    memset(&srcaddr, 0, sizeof(srcaddr));
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    log_debug(2, "Sending dummy id=%i to %s", ((unsigned short *)dnsbuf)[0], 
	      inet_ntop(AF_INET6,&s->addr.sin6_addr,buf,sizeof(buf)));
#else
    log_debug(2, "Sending dummy id=%i to %s", ((unsigned short *)dnsbuf)[0], 
	      inet_ntoa(s->addr.sin_addr));
#endif
    /*  return dnssend(s, &dnsbuf, sizeof(dnsbuf)); */
    rc=udp_send(q->sock, s, dnsbuf, sizeof(dnsbuf));
    ((unsigned short *)dnsbuf)[0]++;
    return rc;
  }
  return -1;
}
