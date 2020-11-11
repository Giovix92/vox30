/*
 * check.c - validation of DNS packets
 *
 * Copyright (C) 2004 Natanael Copa <n@tanael.org>
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

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <syslog.h>
#include <sys/time.h>
#include <string.h>

#include "common.h"
#include "check.h"
#include "dns.h"
#include "srvnode.h"
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
/*
 * check_query()
 *
 * This functions does some test to verify that the message is sane
 * returns:
 *          -1 if the query is insane and should be ignored
 *          0 if the query is ok and should be processed
 *          1 if the query is not ok and a format error should be sent
 */

int check_query(void *msg, int len) {
  short int flags = ntohs(((short int *)msg)[1]); /* flags */

  /* first check the size */
  if (len <12) {
#ifdef __SC_BUILD__
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_QUERY_ID, 360, "DNS query failed, invalid DNS query length\n");
#endif

    log_debug(1, "Query packet is to small. Ignoring");
    return -1;
  }

  if (len > UDP_MAXSIZE) {
#ifdef __SC_BUILD__
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_QUERY_ID, 360, "DNS query failed, invalid DNS query length\n");
#endif

    log_debug(1, "Query packet is too big. Ignoring");
    return -1;
  }

  /* Flags check. If Z flag or QR is set, just ignore
   * the request. According to rfc1035 4.1.1 Z flag must be zero in
   * all queries and responses. 
   * BIND set the RCODE on recursive lookups so we dont check that.
   */

  /* check if Z is set. It should never been set */
  /*
  if (flags & MASK_Z) {
#ifdef __SC_BUILD__
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "DNS query failed, invalid DNS query format\n");
#endif

    log_debug(1, "Z was set. Ignoring query");
    return -1;
  }
*/
  /* Check if it is a query, if QR is set */
  if (flags & MASK_QR) {
#ifdef __SC_BUILD__
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "DNS query failed, invalid DNS query format\n");
#endif

    log_debug(1, "QR was set. Ignoring query");
    return -1;
  }
#ifdef __SC_BUILD__
  if (!(flags & MASK_RD)) {
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "DNS query failed, invalid DNS query format(None-Rec)\n");
      return -1;
  }
#endif
  /* ok this is valid */
  return 0;
}

/*
 * check_reply()
 *
 * This functions does some test to verify that the message is sane
 * returns:
 *          -1 if the reply is insane and should be ignored
 *          0 if the query is ok and should be processed
 */

int check_reply(srvnode_t *s, void *msg, int len) {
  short int flags = ntohs(((short int *)msg)[1]); /* flags */
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    char buf[64];
#endif
  /* bad server replies are pretty bad. Lets log them to syslog */
  if (len <12) {
#ifdef __SC_BUILD__
      log_dns(LOG_WARNING, NORM_LOG, LOG_DNS_RELAY_ID, 360, "DNS relay failed, invalid DNS reply length\n");
#endif

#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    log_msg(LOG_WARNING, "Reply packet was to small. Ignoring reply from %s",
	    inet_ntop(AF_INET6,&(s->addr.sin6_addr),buf,sizeof(buf)));
#else
    log_msg(LOG_WARNING, "Reply packet was to small. Ignoring reply from %s",
	    inet_ntoa(s->addr.sin_addr));
#endif
    return -1;
  }
  /* we have already checkd for oversized packets */
  /* check if Z is set. It should never been set */
  /*
  if (flags & MASK_Z) {
#ifdef __SC_BUILD__
      log_dns(LOG_WARNING, NORM_LOG, LOG_DNS_RELAY_ID, 360, "DNS relay failed, invalid DNS reply format\n");
#endif

#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    log_msg(LOG_WARNING, "Z was set. Ignoring reply from %s",
	    inet_ntop(AF_INET6,&(s->addr.sin6_addr),buf,sizeof(buf)));
#else
    log_msg(LOG_WARNING, "Z was set. Ignoring reply from %s",
		inet_ntoa(s->addr.sin_addr));
#endif
    return -1;
  }
*/
#ifdef __SC_BUILD__
  /* martin_huang@sdc.sercomm.com:We should do not relay this to LAN with reply error*/
  
#ifdef FAILOVER_ALG1
  if ((flags & MASK_NO_SUCH_NAME) == MASK_NO_SUCH_NAME) {
      return -2;
  }
#endif
  
  if (flags & MASK_RCODE) {
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "DNS relay failed, DNS reply error\n");
      return -1;
  }
#endif

  /* Check if it is a query, if QR is set */
  if (! (flags & MASK_QR)) {
#ifdef __SC_BUILD__
      log_dns(LOG_ERR, NORM_LOG, LOG_DNS_RELAY_ID, 360, "DNS relay failed, invalid DNS reply format\n");
#endif

#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) &&(!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
    log_msg(LOG_WARNING, "QR was not set. Ignoring reply from %s",
	    inet_ntop(AF_INET6,&(s->addr.sin6_addr),buf,sizeof(buf)));
#else
    log_msg(LOG_WARNING, "QR was not set. Ignoring reply from %s",
	    inet_ntoa(s->addr.sin_addr));
#endif
    return -1;
  }

  /* check for possible cache poisoning attempts etc here.... */

  /* ok this packet is ok */
  return 0;
}
