/* dnsmasq is Copyright (c) 2000-2018 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"
#include "debug.h"
#include "debug.h"
#ifdef __SC_BUILD__
#define LOCK_RETRY_COUNT    3
/* Define domain type value */
#define DOMAIN_TYPE_UNKNOWN         (0x0000)
#define DOMAIN_TYPE_DATA            (0x0001)
#define DOMAIN_TYPE_DATA_MAN        (0x0002)
#define DOMAIN_TYPE_TR069           (0x0003)
#define DOMAIN_TYPE_NTP             (0x0004)
#define DOMAIN_TYPE_VOIP            (0x0005)
#define DOMAIN_TYPE_IPTV            (0x0006)
#define DOMAIN_TYPE_CLI             (0x0007)
#define DOMAIN_TYPE_IPPHONE         (0x0008)

#define MAX_MSG_LEN                 (512)
#define MAX_DOMAIN_LEN              (200)

int util_lock_reg(int fd, short type, int retry_time)
{
	struct flock ret;
	ret.l_type = type;
	ret.l_start = 0;
	ret.l_whence = SEEK_SET;
	ret.l_len = 0;
	ret.l_pid = getpid();
	int times = 0;

again:
	times++;
	if (times <= LOCK_RETRY_COUNT) {
		if (fcntl(fd, F_SETLK, &ret) < 0) {
			usleep(retry_time);
			goto again;
		}
	}
	if (times == LOCK_RETRY_COUNT + 1)
		return -1;
	else
		return 0;
}

int util_lock(char *file, int retry_time)
{
	int times = 0;
	int fd = -1;
	struct flock flockptr = {.l_type = F_WRLCK,
		       .l_start = 0,
		        .l_whence = SEEK_SET,
		         .l_len = 0
	};

	if ((fd = open(file, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR)) < 0)
		return -1;
add_wlock:
	if (0 != retry_time && ++times > retry_time) {
		close(fd);
		return -1;
	}
	if (fcntl(fd, F_SETLKW, &flockptr) < 0) {
		if (errno == EINTR) {
			goto add_wlock;
		} else {
			close(fd);
			return -1;
		}
	}
	return fd;

}
void util_unlock(int fd)
{

	struct flock flockptr = {.l_type = F_UNLCK,
		       .l_start = 0,
		        .l_whence = SEEK_SET,
		         .l_len = 0
	};
	if (fd >= 0) {
		fcntl(fd, F_SETLK, &flockptr);
		close(fd);
	}
}
static int redirect_ipv4(struct all_addr **addrpp)
{
	/*
	   inet_aton("192.168.0.1",&serv_tmmp->addr.in.sin_addr);*/
	static struct in_addr my_addr;
	char *redirectip = NULL;
	if (!(redirectip = strstr(daemon->redirect_ip,"#"))) {
		return -1;
	}
	inet_aton(redirectip+2,&my_addr);
	*addrpp = (struct all_addr *)&my_addr;
	return 0;
}
#ifdef HAVE_IPV6
static int redirect_ipv6(struct all_addr **addrpp)
{
    static struct in6_addr my_addr;
    char *redirectip = NULL;
    if(!(redirectip = strstr(daemon->redirect_ipv6,"#"))) {
        return -1;
    }
    inet_pton(AF_INET6,redirectip+2,&my_addr);
    *addrpp = (struct all_addr*)&my_addr;
    return 0;
}
#endif
int sc_server_search(int type, char *domain, int dtype, int origi_pri);
int sc_dtype_parse(char *dns_pkt, int *len);
void sc_dns_answer_parse(int dtype, char *msg, int *len);
#endif

static struct frec *lookup_frec(unsigned short id, void *hash);
static struct frec *lookup_frec_by_sender(unsigned short id,
					  union mysockaddr *addr,
					  void *hash);
static unsigned short get_id(void);
static void free_frec(struct frec *f);

/* Send a UDP packet with its source address set as "source" 
   unless nowild is true, when we just send it with the kernel default */
int send_from(int fd, int nowild, char *packet, size_t len, 
	      union mysockaddr *to, struct all_addr *source,
	      unsigned int iface)
{
  struct msghdr msg;
  struct iovec iov[1]; 
  union {
    struct cmsghdr align; /* this ensures alignment */
#if defined(HAVE_LINUX_NETWORK)
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#elif defined(IP_SENDSRCADDR)
    char control[CMSG_SPACE(sizeof(struct in_addr))];
#endif
#ifdef HAVE_IPV6
    char control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#endif
  } control_u;
  
  iov[0].iov_base = packet;
  iov[0].iov_len = len;

  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  msg.msg_name = to;
  msg.msg_namelen = sa_len(to);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  
  if (!nowild)
    {
      struct cmsghdr *cmptr;
      msg.msg_control = &control_u;
      msg.msg_controllen = sizeof(control_u);
      cmptr = CMSG_FIRSTHDR(&msg);

      if (to->sa.sa_family == AF_INET)
	{
#if defined(HAVE_LINUX_NETWORK)
	  struct in_pktinfo p;
	  p.ipi_ifindex = 0;
	  p.ipi_spec_dst = source->addr.addr4;
	  memcpy(CMSG_DATA(cmptr), &p, sizeof(p));
	  msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
	  cmptr->cmsg_level = IPPROTO_IP;
	  cmptr->cmsg_type = IP_PKTINFO;
#elif defined(IP_SENDSRCADDR)
	  memcpy(CMSG_DATA(cmptr), &(source->addr.addr4), sizeof(source->addr.addr4));
	  msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_addr));
	  cmptr->cmsg_level = IPPROTO_IP;
	  cmptr->cmsg_type = IP_SENDSRCADDR;
#endif
	}
      else
#ifdef HAVE_IPV6
	{
	  struct in6_pktinfo p;
	  p.ipi6_ifindex = iface; /* Need iface for IPv6 to handle link-local addrs */
	  p.ipi6_addr = source->addr.addr6;
	  memcpy(CMSG_DATA(cmptr), &p, sizeof(p));
	  msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
	  cmptr->cmsg_type = daemon->v6pktinfo;
	  cmptr->cmsg_level = IPPROTO_IPV6;
	}
#else
      (void)iface; /* eliminate warning */
#endif
    }
  
  while (retry_send(sendmsg(fd, &msg, 0)));

  /* If interface is still in DAD, EINVAL results - ignore that. */
  if (errno != 0 && errno != EINVAL)
    {
      my_syslog(LOG_ERR, _("failed to send packet: %s"), strerror(errno));
      return 0;
    }
  
  return 1;
}
          
#ifdef __SC_BUILD__
static unsigned int search_servers(union mysockaddr *udpaddr, time_t now, struct all_addr **addrpp, unsigned int qtype,
				   char *qdomain, int *type, char **domain, int *norebind)
#else
static unsigned int search_servers(time_t now, struct all_addr **addrpp, unsigned int qtype,
				   char *qdomain, int *type, char **domain, int *norebind)
#endif
			      
{
  /* If the query ends in the domain in one of our servers, set
     domain to point to that name. We find the largest match to allow both
     domain.org and sub.domain.org to exist. */
  
  unsigned int namelen = strlen(qdomain);
  unsigned int matchlen = 0;
  struct server *serv;
  unsigned int flags = 0;
  static struct all_addr zero;
  
  for (serv = daemon->servers; serv; serv=serv->next)
    if (qtype == F_DNSSECOK && !(serv->flags & SERV_DO_DNSSEC))
      continue;
    /* domain matches take priority over NODOTS matches */
    else if ((serv->flags & SERV_FOR_NODOTS) && *type != SERV_HAS_DOMAIN && !strchr(qdomain, '.') && namelen != 0)
      {
	unsigned int sflag = serv->addr.sa.sa_family == AF_INET ? F_IPV4 : F_IPV6; 
	*type = SERV_FOR_NODOTS;
	if (serv->flags & SERV_NO_ADDR)
	  flags = F_NXDOMAIN;
	else if (serv->flags & SERV_LITERAL_ADDRESS)
	  { 
	    /* literal address = '#' -> return all-zero address for IPv4 and IPv6 */
	    if ((serv->flags & SERV_USE_RESOLV) && (qtype & (F_IPV6 | F_IPV4)))
	      {
		memset(&zero, 0, sizeof(zero));
		flags = qtype;
		*addrpp = &zero;
	      }
	    else if (sflag & qtype)
	      {
		flags = sflag;
		if (serv->addr.sa.sa_family == AF_INET) 
		  *addrpp = (struct all_addr *)&serv->addr.in.sin_addr;
#ifdef HAVE_IPV6
		else
		  *addrpp = (struct all_addr *)&serv->addr.in6.sin6_addr;
#endif 
	      }
	    else if (!flags || (flags & F_NXDOMAIN))
	      flags = F_NOERR;
	  } 
      }
    else if (serv->flags & SERV_HAS_DOMAIN)
      {
#ifdef __SC_BUILD__
            if(udpaddr)
            {
                if((0 != strcmp("127.0.0.1", inet_ntoa(udpaddr->in.sin_addr))) && (0 != strcmp("0.0.0.0", inet_ntoa(udpaddr->in.sin_addr))) && (serv->domain_type == 5))
                    continue;
            }
#endif
	unsigned int domainlen = strlen(serv->domain);
	char *matchstart = qdomain + namelen - domainlen;
	if (namelen >= domainlen &&
	    hostname_isequal(matchstart, serv->domain) &&
	    (domainlen == 0 || namelen == domainlen || *(matchstart-1) == '.' ))
	  {
	    if ((serv->flags & SERV_NO_REBIND) && norebind)	
	      *norebind = 1;
	    else
	      {
		unsigned int sflag = serv->addr.sa.sa_family == AF_INET ? F_IPV4 : F_IPV6;
		/* implement priority rules for --address and --server for same domain.
		   --address wins if the address is for the correct AF
		   --server wins otherwise. */
		if (domainlen != 0 && domainlen == matchlen)
		  {
		    if ((serv->flags & SERV_LITERAL_ADDRESS))
		      {
			if (!(sflag & qtype) && flags == 0)
			  continue;
		      }
		    else
		      {
			if (flags & (F_IPV4 | F_IPV6))
			  continue;
		      }
		  }
		
		if (domainlen >= matchlen)
		  {
		    *type = serv->flags & (SERV_HAS_DOMAIN | SERV_USE_RESOLV | SERV_NO_REBIND | SERV_DO_DNSSEC);
            *domain = serv->domain;
		    matchlen = domainlen;
		    if (serv->flags & SERV_NO_ADDR)
		      flags = F_NXDOMAIN;
		    else if (serv->flags & SERV_LITERAL_ADDRESS)
		      {
			 /* literal address = '#' -> return all-zero address for IPv4 and IPv6 */
			if ((serv->flags & SERV_USE_RESOLV) && (qtype & (F_IPV6 | F_IPV4)))
			  {			    
			    memset(&zero, 0, sizeof(zero));
			    flags = qtype;
			    *addrpp = &zero;
			  }
			else if (sflag & qtype)
			  {
			    flags = sflag;
			    if (serv->addr.sa.sa_family == AF_INET) 
			      *addrpp = (struct all_addr *)&serv->addr.in.sin_addr;
#ifdef HAVE_IPV6
			    else
			      *addrpp = (struct all_addr *)&serv->addr.in6.sin6_addr;
#endif
			  }
			else if (!flags || (flags & F_NXDOMAIN))
			  flags = F_NOERR;
		      }
		    else
		      flags = 0;
		  } 
	      }
	  }
      }
  
  if (flags == 0 && !(qtype & (F_QUERY | F_DNSSECOK)) && 
      option_bool(OPT_NODOTS_LOCAL) && !strchr(qdomain, '.') && namelen != 0)
    /* don't forward A or AAAA queries for simple names, except the empty name */
    flags = F_NOERR;
  
  if (flags == F_NXDOMAIN && check_for_local_domain(qdomain, now))
    flags = F_NOERR;

  if (flags)
    {
#ifdef __SC_BUILD__
		if (redirect == 1) {
			flags = F_IPV4;
			return flags;
		}
#endif
       if (flags == F_NXDOMAIN || flags == F_NOERR)
	 log_query(flags | qtype | F_NEG | F_CONFIG | F_FORWARD, qdomain, NULL, NULL);
       else
	 {
	   /* handle F_IPV4 and F_IPV6 set on ANY query to 0.0.0.0/:: domain. */
	   if (flags & F_IPV4)
	     log_query((flags | F_CONFIG | F_FORWARD) & ~F_IPV6, qdomain, *addrpp, NULL);
#ifdef HAVE_IPV6
	   if (flags & F_IPV6)
	     log_query((flags | F_CONFIG | F_FORWARD) & ~F_IPV4, qdomain, *addrpp, NULL);
#endif
	 }
    }
  else if ((*type) & SERV_USE_RESOLV)
    {
      *type = 0; /* use normal servers for this domain */
      *domain = NULL;
#ifdef __SC_BUILD__
      if (redirect == 1) {
          flags = F_IPV4;
          return flags;
      }
#endif
    }
  return  flags;
}

static int forward_query(int udpfd, union mysockaddr *udpaddr,
			 struct all_addr *dst_addr, unsigned int dst_iface,
			 struct dns_header *header, size_t plen, time_t now, 
			 struct frec *forward, int ad_reqd, int do_bit
#ifdef __SC_BUILD__
             ,int domain_type, unsigned char *client_mac
#endif
             )
{
  char *domain = NULL;
  int type = SERV_DO_DNSSEC, norebind = 0;
  struct all_addr *addrp = NULL;
  unsigned int flags = 0;
  struct server *start = NULL;
#ifdef __SC_BUILD__
#ifdef HAVE_IPV6
    char buf[64] = "";
#endif
    int priority;
#endif
#ifdef HAVE_DNSSEC
  void *hash = hash_questions(header, plen, daemon->namebuff);
  int do_dnssec = 0;
#else
  unsigned int crc = questions_crc(header, plen, daemon->namebuff);
  void *hash = &crc;
#endif
  unsigned int gotname = extract_request(header, plen, daemon->namebuff, NULL);
  unsigned char *oph = find_pseudoheader(header, plen, NULL, NULL, NULL, NULL);
  (void)do_bit;

  /* may be no servers available. */
  if (forward || (hash && (forward = lookup_frec_by_sender(ntohs(header->id), udpaddr, hash))))
    {
      /* If we didn't get an answer advertising a maximal packet in EDNS,
	 fall back to 1280, which should work everywhere on IPv6.
	 If that generates an answer, it will become the new default
	 for this server */
      forward->flags |= FREC_TEST_PKTSZ;
      
#ifdef HAVE_DNSSEC
      /* If we've already got an answer to this query, but we're awaiting keys for validation,
	 there's no point retrying the query, retry the key query instead...... */
      if (forward->blocking_query)
	{
	  int fd, is_sign;
	  unsigned char *pheader;
	  
	  forward->flags &= ~FREC_TEST_PKTSZ;
	  
	  while (forward->blocking_query)
	    forward = forward->blocking_query;
	   
	  blockdata_retrieve(forward->stash, forward->stash_len, (void *)header);
	  plen = forward->stash_len;
	  
	  forward->flags |= FREC_TEST_PKTSZ;
	  if (find_pseudoheader(header, plen, NULL, &pheader, &is_sign, NULL) && !is_sign)
	    PUTSHORT(SAFE_PKTSZ, pheader);
	  
	  if (forward->sentto->addr.sa.sa_family == AF_INET) 
	    log_query(F_NOEXTRA | F_DNSSEC | F_IPV4, "retry", (struct all_addr *)&forward->sentto->addr.in.sin_addr, "dnssec");
#ifdef HAVE_IPV6
	  else
	    log_query(F_NOEXTRA | F_DNSSEC | F_IPV6, "retry", (struct all_addr *)&forward->sentto->addr.in6.sin6_addr, "dnssec");
#endif
  
	  if (forward->sentto->sfd)
	    fd = forward->sentto->sfd->fd;
	  else
	    {
#ifdef HAVE_IPV6
	      if (forward->sentto->addr.sa.sa_family == AF_INET6)
		fd = forward->rfd6->fd;
	      else
#endif
		fd = forward->rfd4->fd;
	    }
	  
	  while (retry_send(sendto(fd, (char *)header, plen, 0,
				   &forward->sentto->addr.sa,
				   sa_len(&forward->sentto->addr))));
	  
	  return 1;
	}
#endif

      /* retry on existing query, send to all available servers  */
      domain = forward->sentto->domain;
      forward->sentto->failed_queries++;
      if (!option_bool(OPT_ORDER))
	{
	  forward->forwardall = 1;
	  daemon->last_server = NULL;
	}
      type = forward->sentto->flags & SERV_TYPE;
#ifdef HAVE_DNSSEC
      do_dnssec = forward->sentto->flags & SERV_DO_DNSSEC;
#endif

      if (!(start = forward->sentto->next))
	start = daemon->servers; /* at end of list, recycle */
      header->id = htons(forward->new_id);
#ifdef __SC_BUILD__
      resume_domain(header, plen, forward->new_name);
#endif

    }
  else 
    {
      if (gotname)
#ifdef __SC_BUILD__
	flags = search_servers(udpaddr,now, &addrp, gotname, daemon->namebuff, &type, &domain, &norebind);
#else
	flags = search_servers(now, &addrp, gotname, daemon->namebuff, &type, &domain, &norebind);
#endif
      
#ifdef HAVE_DNSSEC
      do_dnssec = type & SERV_DO_DNSSEC;
#endif
      type &= ~SERV_DO_DNSSEC;      

      if (daemon->servers && !flags)
	forward = get_new_frec(now, NULL, 0);
      /* table full - flags == 0, return REFUSED */
      
      if (forward)
	{
	  forward->source = *udpaddr;
	  forward->dest = *dst_addr;
	  forward->iface = dst_iface;
	  forward->orig_id = ntohs(header->id);
	  forward->new_id = get_id();
	  forward->fd = udpfd;
#ifdef __SC_BUILD__
            forward->dtype = domain_type;
            forward->pri = 0;
            forward->retry = 0;
#endif
	  memcpy(forward->hash, hash, HASH_SIZE);
	  forward->forwardall = 0;
	  forward->flags = 0;
	  if (norebind)
	    forward->flags |= FREC_NOREBIND;
	  if (header->hb4 & HB4_CD)
	    forward->flags |= FREC_CHECKING_DISABLED;
	  if (ad_reqd)
	    forward->flags |= FREC_AD_QUESTION;
#ifdef HAVE_DNSSEC
	  forward->work_counter = DNSSEC_WORK;
	  if (do_bit)
	    forward->flags |= FREC_DO_QUESTION;
#endif
#ifdef __SC_BUILD__
      daemon->id = forward->new_id;
      sprintf(forward->orig_name, "%s", daemon->namebuff);
      sprintf(forward->new_name, "%s", daemon->namebuff);
      create_newname_follow_bit20(forward->new_name);
      resume_domain(header, plen, forward->new_name);
      my_syslog(LOG_INFO, "%s#%d,receive %s,change = %s", __func__, __LINE__, forward->orig_name, forward->new_name);
#endif
	  
	  header->id = htons(forward->new_id);
	  
	  /* In strict_order mode, always try servers in the order 
	     specified in resolv.conf, if a domain is given 
	     always try all the available servers,
	     otherwise, use the one last known to work. */
	  
	  if (type == 0)
	    {
	      if (option_bool(OPT_ORDER))
		start = daemon->servers;
	      else if (!(start = daemon->last_server) ||
		       daemon->forwardcount++ > FORWARD_TEST ||
		       difftime(now, daemon->forwardtime) > FORWARD_TIME)
		{
		  start = daemon->servers;
		  forward->forwardall = 1;
		  daemon->forwardcount = 0;
		  daemon->forwardtime = now;
		}
	    }
	  else
	    {
	      start = daemon->servers;
	      if (!option_bool(OPT_ORDER))
		forward->forwardall = 1;
	    }
	}
    }

  /* check for send errors here (no route to host) 
     if we fail to send to all nameservers, send back an error
     packet straight away (helps modem users when offline)  */
  
  if (!flags && forward)
    {
      struct server *firstsentto = start;
      int subnet, forwarded = 0;
      size_t edns0_len;
      unsigned char *pheader;
      
      /* If a query is retried, use the log_id for the retry when logging the answer. */
      forward->log_id = daemon->log_id;
      
      plen = add_edns0_config(header, plen, ((unsigned char *)header) + PACKETSZ, &forward->source, now, &subnet);
      
      if (subnet)
	forward->flags |= FREC_HAS_SUBNET;
      
#ifdef HAVE_DNSSEC
      if (option_bool(OPT_DNSSEC_VALID) && do_dnssec)
	{
	  plen = add_do_bit(header, plen, ((unsigned char *) header) + PACKETSZ);
	 	      
	  /* For debugging, set Checking Disabled, otherwise, have the upstream check too,
	     this allows it to select auth servers when one is returning bad data. */
	  if (option_bool(OPT_DNSSEC_DEBUG))
	    header->hb4 |= HB4_CD;

	}
#endif

      if (find_pseudoheader(header, plen, &edns0_len, &pheader, NULL, NULL))
	{
	  /* If there wasn't a PH before, and there is now, we added it. */
	  if (!oph)
	    forward->flags |= FREC_ADDED_PHEADER;

	  /* If we're sending an EDNS0 with any options, we can't recreate the query from a reply. */
	  if (edns0_len > 11)
	    forward->flags |= FREC_HAS_EXTRADATA;

	  /* Reduce udp size on retransmits. */
	  if (forward->flags & FREC_TEST_PKTSZ)
	    PUTSHORT(SAFE_PKTSZ, pheader);
	}
#ifdef __SC_BUILD__
        priority = sc_server_search(type, domain, forward->dtype, -1);
#endif

      while (1)
	{ 
	  /* only send to servers dealing with our domain.
	     domain may be NULL, in which case server->domain 
	     must be NULL also. */
	  
	  if (type == (start->flags & SERV_TYPE) &&
	      (type != SERV_HAS_DOMAIN || hostname_isequal(domain, start->domain)) &&
	      !(start->flags & (SERV_LITERAL_ADDRESS | SERV_LOOP)))
	    {
	      int fd;

	      /* find server socket to use, may need to get random one. */
	      if (start->sfd)
		fd = start->sfd->fd;
	      else 
		{
#ifdef HAVE_IPV6
		  if (start->addr.sa.sa_family == AF_INET6)
		    {
		      if (!forward->rfd6 &&
			  !(forward->rfd6 = allocate_rfd(AF_INET6)))
			break;
		      daemon->rfd_save = forward->rfd6;
		      fd = forward->rfd6->fd;
		    }
		  else
#endif
		    {
		      if (!forward->rfd4 &&
			  !(forward->rfd4 = allocate_rfd(AF_INET)))
			break;
		      daemon->rfd_save = forward->rfd4;
		      fd = forward->rfd4->fd;
		    }

#ifdef HAVE_CONNTRACK
		  /* Copy connection mark of incoming query to outgoing connection. */
		  if (option_bool(OPT_CONNTRACK))
		    {
		      unsigned int mark;
		      if (get_incoming_mark(&forward->source, &forward->dest, 0, &mark))
			setsockopt(fd, SOL_SOCKET, SO_MARK, &mark, sizeof(unsigned int));
		    }
#endif
		}
	      
#ifdef HAVE_DNSSEC
	      if (option_bool(OPT_DNSSEC_VALID) && (forward->flags & FREC_ADDED_PHEADER))
		{
		  /* Difficult one here. If our client didn't send EDNS0, we will have set the UDP
		     packet size to 512. But that won't provide space for the RRSIGS in many cases.
		     The RRSIGS will be stripped out before the answer goes back, so the packet should
		     shrink again. So, if we added a do-bit, bump the udp packet size to the value
		     known to be OK for this server. We check returned size after stripping and set
		     the truncated bit if it's still too big. */		  
		  unsigned char *pheader;
		  int is_sign;
		  if (find_pseudoheader(header, plen, NULL, &pheader, &is_sign, NULL) && !is_sign)
		    PUTSHORT(start->edns_pktsz, pheader);
		}
#endif
#ifdef __SC_BUILD__
        if(udpaddr)
        {
            if(forward->pri != start->srv_pri)
            {
                if (!(start = start->next))
                    start = daemon->servers;
                if (start == firstsentto)
                    break;
                continue;
            }
            if((0 == strcmp("127.0.0.1", inet_ntoa(udpaddr->in.sin_addr)) || 0 == strcmp("0.0.0.0", inet_ntoa(udpaddr->in.sin_addr))) && (start->domain_type == 2) && (strcmp(inet_ntoa(start->addr.in.sin_addr), "0.0.0.0") != 0))
            {
                if (!(start = start->next))
                    start = daemon->servers;
                if (start == firstsentto)
                    break;
                continue;
            }
            else if(0 != strcmp("127.0.0.1", inet_ntoa(udpaddr->in.sin_addr)) && (0 != strcmp("0.0.0.0", inet_ntoa(udpaddr->in.sin_addr)))  && (start->domain_type == 0) && (strcmp(inet_ntoa(start->addr.in.sin_addr), "0.0.0.0") != 0))
            {
                if (!(start = start->next))
                    start = daemon->servers;
                if (start == firstsentto)
                    break;
                continue;
            }
#ifdef HAVE_IPV6
            else if(0 == strcmp("::1", inet_ntop(AF_INET6,&udpaddr->in6.sin6_addr,buf,sizeof(buf))) && (start->domain_type == 2) && (strcmp(inet_ntoa(start->addr.in.sin_addr), "0.0.0.0") == 0))
            {
                if (!(start = start->next))
                    start = daemon->servers;
                if (start == firstsentto)
                    break;
                continue;
            }
            else if(0 != strcmp("::1", inet_ntop(AF_INET6,&udpaddr->in6.sin6_addr,buf,sizeof(buf))) && (start->domain_type == 0) && (strcmp(inet_ntoa(start->addr.in.sin_addr), "0.0.0.0") == 0))
            {
                if (!(start = start->next))
                    start = daemon->servers;
                if (start == firstsentto)
                    break;
                continue;
            }
#endif
        }
#endif

#ifdef __SC_BUILD__
                if (sendto(fd, (char *)header, plen, 0,
                            &start->addr.sa,
                            sa_len(&start->addr)) != -1)
#else
	      if (retry_send(sendto(fd, (char *)header, plen, 0,
				    &start->addr.sa,
				    sa_len(&start->addr))))
		continue;
	    
	      if (errno == 0)
#endif
		{
#ifdef HAVE_DUMPFILE
		  dump_packet(DUMP_UP_QUERY, (void *)header, plen, NULL, &start->addr);
#endif
		  
		  /* Keep info in case we want to re-send this packet */
		  daemon->srv_save = start;
		  daemon->packet_len = plen;
		  
		  if (!gotname)
		    strcpy(daemon->namebuff, "query");
		  if (start->addr.sa.sa_family == AF_INET)
		    log_query(F_SERVER | F_IPV4 | F_FORWARD, daemon->namebuff, 
			      (struct all_addr *)&start->addr.in.sin_addr, NULL); 
#ifdef HAVE_IPV6
		  else
		    log_query(F_SERVER | F_IPV6 | F_FORWARD, daemon->namebuff, 
			      (struct all_addr *)&start->addr.in6.sin6_addr, NULL);
#endif 
		  start->queries++;
		  forwarded = 1;
		  forward->sentto = start;
		  if (!forward->forwardall) 
		    break;
		  forward->forwardall++;
#ifdef __SC_BUILD__
          forward->plen = plen;
          memcpy(forward->header, header, plen);
          gettimeofday(&forward->tv,NULL);

          forward->try_count++;
          if (start->addr.sa.sa_family == AF_INET) {
              forward->v4counter++;
          }
          else {
              forward->v6counter++;
          }
          forward->curr_srv_pri = priority;
#endif
		}
	    } 
	  
	  if (!(start = start->next))
 	    start = daemon->servers;
	  
	  if (start == firstsentto)
	    break;
	}
      
      if (forwarded)
      {
#ifdef __SC_BUILD__
          if(forward->pri == 0)
              forward->pri = 1;
          else
              forward->pri = 0;
#endif
          return 1;
      }
      
      /* could not send on, prepare to return */ 
      header->id = htons(forward->orig_id);
      free_frec(forward); /* cancel */
    }	  
  
  /* could not send on, return empty answer or address if known for whole domain */
  if (udpfd != -1)
    {
#ifdef __SC_BUILD__
		if (redirect == 1 && (0 != strcmp("127.0.0.1", inet_ntoa(udpaddr->in.sin_addr))) && (0 != strcmp("0.0.0.0", inet_ntoa(udpaddr->in.sin_addr))))
        {
            if(gotname == F_IPV4)
            {
                redirect_ipv4(&addrp);
                flags = F_IPV4;
            }
#ifdef HAVE_IPV6
            else if(gotname == F_IPV6)
            {
                redirect_ipv6(&addrp);
                flags = F_IPV6;
            }
#endif
			plen = setup_reply(header, plen, addrp, flags, daemon->local_ttl);
		} else
			plen = setup_reply(header, plen, addrp, flags, daemon->local_ttl);
#else
      plen = setup_reply(header, plen, addrp, flags, daemon->local_ttl);
#endif
      if (oph)
	plen = add_pseudoheader(header, plen, ((unsigned char *) header) + PACKETSZ, daemon->edns_pktsz, 0, NULL, 0, do_bit, 0);
      send_from(udpfd, option_bool(OPT_NOWILD) || option_bool(OPT_CLEVERBIND), (char *)header, plen, udpaddr, dst_addr, dst_iface);
    }

  return 0;
}

static size_t process_reply(struct dns_header *header, time_t now, struct server *server, size_t n, int check_rebind, 
			    int no_cache, int cache_secure, int bogusanswer, int ad_reqd, int do_bit, int added_pheader, 
			    int check_subnet, union mysockaddr *query_source
#ifdef __SC_BUILD__
                            , int dtype, unsigned char *client_mac
#endif
                           )
{
  unsigned char *pheader, *sizep;
  char **sets = 0;
  int munged = 0, is_sign;
  unsigned int rcode = RCODE(header);
  size_t plen; 
  
  (void)ad_reqd;
  (void)do_bit;
  (void)bogusanswer;

#ifdef HAVE_IPSET
  if (daemon->ipsets && extract_request(header, n, daemon->namebuff, NULL))
    {
      /* Similar algorithm to search_servers. */
      struct ipsets *ipset_pos;
      unsigned int namelen = strlen(daemon->namebuff);
      unsigned int matchlen = 0;
      for (ipset_pos = daemon->ipsets; ipset_pos; ipset_pos = ipset_pos->next) 
	{
	  unsigned int domainlen = strlen(ipset_pos->domain);
	  char *matchstart = daemon->namebuff + namelen - domainlen;
	  if (namelen >= domainlen && hostname_isequal(matchstart, ipset_pos->domain) &&
	      (domainlen == 0 || namelen == domainlen || *(matchstart - 1) == '.' ) &&
	      domainlen >= matchlen) 
	    {
	      matchlen = domainlen;
	      sets = ipset_pos->sets;
	    }
	}
    }
#endif
  
  if ((pheader = find_pseudoheader(header, n, &plen, &sizep, &is_sign, NULL)))
    {
      /* Get extended RCODE. */
      rcode |= sizep[2] << 4;

      if (check_subnet && !check_source(header, plen, pheader, query_source))
	{
	  my_syslog(LOG_WARNING, _("discarding DNS reply: subnet option mismatch"));
	  return 0;
	}
      
      if (!is_sign)
	{
	  if (added_pheader)
	    {
	      /* client didn't send EDNS0, we added one, strip it off before returning answer. */
	      n = rrfilter(header, n, 0);
	      pheader = NULL;
	    }
	  else
	    {
	      unsigned short udpsz;

	      /* If upstream is advertising a larger UDP packet size
		 than we allow, trim it so that we don't get overlarge
		 requests for the client. We can't do this for signed packets. */
	      GETSHORT(udpsz, sizep);
	      if (udpsz > daemon->edns_pktsz)
		{
		  sizep -= 2;
		  PUTSHORT(daemon->edns_pktsz, sizep);
		}

#ifdef HAVE_DNSSEC
	      /* If the client didn't set the do bit, but we did, reset it. */
	      if (option_bool(OPT_DNSSEC_VALID) && !do_bit)
		{
		  unsigned short flags;
		  sizep += 2; /* skip RCODE */
		  GETSHORT(flags, sizep);
		  flags &= ~0x8000;
		  sizep -= 2;
		  PUTSHORT(flags, sizep);
		}
#endif
	    }
	}
    }
  
  /* RFC 4035 sect 4.6 para 3 */
  if (!is_sign && !option_bool(OPT_DNSSEC_PROXY))
     header->hb4 &= ~HB4_AD;
  
  if (OPCODE(header) != QUERY)
    return resize_packet(header, n, pheader, plen);

  if (rcode != NOERROR && rcode != NXDOMAIN)
    {
      struct all_addr a;
      a.addr.rcode.rcode = rcode;
      log_query(F_UPSTREAM | F_RCODE, "error", &a, NULL);
      
      return resize_packet(header, n, pheader, plen);
    }
  
  /* Complain loudly if the upstream server is non-recursive. */
  if (!(header->hb4 & HB4_RA) && rcode == NOERROR &&
      server && !(server->flags & SERV_WARNED_RECURSIVE))
    {
      prettyprint_addr(&server->addr, daemon->namebuff);
      my_syslog(LOG_WARNING, _("nameserver %s refused to do a recursive query"), daemon->namebuff);
      if (!option_bool(OPT_LOG))
	server->flags |= SERV_WARNED_RECURSIVE;
    }  

  if (daemon->bogus_addr && rcode != NXDOMAIN &&
      check_for_bogus_wildcard(header, n, daemon->namebuff, daemon->bogus_addr, now
#ifdef __SC_BUILD__
                , dtype, client_mac
#endif
                )) 

    {
      munged = 1;
      SET_RCODE(header, NXDOMAIN);
      header->hb3 &= ~HB3_AA;
      cache_secure = 0;
    }
  else 
    {
      int doctored = 0;
      
      if (rcode == NXDOMAIN && 
	  extract_request(header, n, daemon->namebuff, NULL) &&
	  check_for_local_domain(daemon->namebuff, now))
	{
	  /* if we forwarded a query for a locally known name (because it was for 
	     an unknown type) and the answer is NXDOMAIN, convert that to NODATA,
	     since we know that the domain exists, even if upstream doesn't */
	  munged = 1;
	  header->hb3 |= HB3_AA;
	  SET_RCODE(header, NOERROR);
	  cache_secure = 0;
	}
      
      if (extract_addresses(header, n, daemon->namebuff, now, sets, is_sign, check_rebind, no_cache, cache_secure, &doctored
#ifdef __SC_BUILD__
                            , dtype, client_mac
#endif
                           ))
	{
	  my_syslog(LOG_WARNING, _("possible DNS-rebind attack detected: %s"), daemon->namebuff);
	  munged = 1;
	  cache_secure = 0;
	}

      if (doctored)
	cache_secure = 0;
    }
  
#ifdef HAVE_DNSSEC
  if (bogusanswer && !(header->hb4 & HB4_CD) && !option_bool(OPT_DNSSEC_DEBUG))
    {
      /* Bogus reply, turn into SERVFAIL */
      SET_RCODE(header, SERVFAIL);
      munged = 1;
    }

  if (option_bool(OPT_DNSSEC_VALID))
    {
      header->hb4 &= ~HB4_AD;
      
      if (!(header->hb4 & HB4_CD) && ad_reqd && cache_secure)
	header->hb4 |= HB4_AD;
      
      /* If the requestor didn't set the DO bit, don't return DNSSEC info. */
      if (!do_bit)
	n = rrfilter(header, n, 1);
    }
#endif

  /* do this after extract_addresses. Ensure NODATA reply and remove
     nameserver info. */
  
  if (munged)
    {
      header->ancount = htons(0);
      header->nscount = htons(0);
      header->arcount = htons(0);
      header->hb3 &= ~HB3_TC;
    }
  
  /* the bogus-nxdomain stuff, doctor and NXDOMAIN->NODATA munging can all elide
     sections of the packet. Find the new length here and put back pseudoheader
     if it was removed. */
  return resize_packet(header, n, pheader, plen);
}

/* sets new last_server */
void reply_query(int fd, int family, time_t now)
{
  /* packet from peer server, extract data for cache, and send to
     original requester */
  struct dns_header *header;
  union mysockaddr serveraddr;
  struct frec *forward;
  socklen_t addrlen = sizeof(serveraddr);
  ssize_t n = recvfrom(fd, daemon->packet, daemon->packet_buff_sz, 0, &serveraddr.sa, &addrlen);
  size_t nn;
  struct server *server;
  void *hash;
#ifndef HAVE_DNSSEC
  unsigned int crc;
#endif
#ifdef __SC_BUILD__
  unsigned char client_mac[DHCP_CHADDR_MAX] = "";
#endif

  /* packet buffer overwritten */
  daemon->srv_save = NULL;
  
  /* Determine the address of the server replying  so that we can mark that as good */
  serveraddr.sa.sa_family = family;
#ifdef HAVE_IPV6
  if (serveraddr.sa.sa_family == AF_INET6)
    serveraddr.in6.sin6_flowinfo = 0;
#endif
  
  header = (struct dns_header *)daemon->packet;

  if (n < (int)sizeof(struct dns_header) || !(header->hb3 & HB3_QR))
    return;
  
  /* spoof check: answer must come from known server, */
  for (server = daemon->servers; server; server = server->next)
    if (!(server->flags & (SERV_LITERAL_ADDRESS | SERV_NO_ADDR)) &&
	sockaddr_isequal(&server->addr, &serveraddr))
      break;
  
  if (!server)
    return;

  /* If sufficient time has elapsed, try and expand UDP buffer size again. */
  if (difftime(now, server->pktsz_reduced) > UDP_TEST_TIME)
    server->edns_pktsz = daemon->edns_pktsz;

#ifdef HAVE_DNSSEC
  hash = hash_questions(header, n, daemon->namebuff);
#else
  hash = &crc;
  crc = questions_crc(header, n, daemon->namebuff);
#endif
  
  if (!(forward = lookup_frec(ntohs(header->id), hash)))
    return;
  
#ifdef HAVE_DUMPFILE
  dump_packet((forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY)) ? DUMP_SEC_REPLY : DUMP_UP_REPLY,
	      (void *)header, n, &serveraddr, NULL);
#endif
#ifndef HAVE_DNSSEC
#ifdef __SC_BUILD__
    my_syslog(LOG_DEBUG, "namebuff [%s] new_name [%s]\n", daemon->namebuff, forward->new_name);
    if (strcmp(daemon->namebuff, forward->new_name)) {
        return;
    }
#endif
#endif

  /* log_query gets called indirectly all over the place, so 
     pass these in global variables - sorry. */
  daemon->log_display_id = forward->log_id;
  daemon->log_source_addr = &forward->source;
  
  if (daemon->ignore_addr && RCODE(header) == NOERROR &&
      check_for_ignored_address(header, n, daemon->ignore_addr))
    return;

#ifdef __SC_BUILD__
    resume_domain(header, n, forward->orig_name);
    sc_dns_answer_parse(forward->dtype, (char *)header, &n);
    find_mac(&forward->source, client_mac, 1, now);
#endif
  /* Note: if we send extra options in the EDNS0 header, we can't recreate
     the query from the reply. */
  if (
#ifdef __SC_BUILD__
      (RCODE(header) != NOERROR || RCODE(header) == SERVFAIL) &&
#else
      (RCODE(header) == REFUSED || RCODE(header) == SERVFAIL) &&
#endif
      forward->forwardall == 0 &&
      !(forward->flags & FREC_HAS_EXTRADATA))
    /* for broken servers, attempt to send to another one. */
    {
      unsigned char *pheader;
      size_t plen;
      int is_sign;

#ifdef HAVE_DNSSEC
      /* For DNSSEC originated queries, just retry the query to the same server. */
      if (forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY))
	{
	  struct server *start;
	  
	  blockdata_retrieve(forward->stash, forward->stash_len, (void *)header);
	  plen = forward->stash_len;

	  forward->forwardall = 2; /* only retry once */
	  start = forward->sentto;

	  /* for non-domain specific servers, see if we can find another to try. */
	  if ((forward->sentto->flags & SERV_TYPE) == 0)
	    while (1)
	      {
		if (!(start = start->next))
		  start = daemon->servers;
		if (start == forward->sentto)
		  break;
		
		if ((start->flags & SERV_TYPE) == 0 &&
		    (start->flags & SERV_DO_DNSSEC))
		  break;
	      }
	    
	  
	  if (start->sfd)
	    fd = start->sfd->fd;
	  else
	    {
#ifdef HAVE_IPV6
	      if (start->addr.sa.sa_family == AF_INET6)
		{
		  /* may have changed family */
		  if (!forward->rfd6)
		    forward->rfd6 = allocate_rfd(AF_INET6);
		  fd = forward->rfd6->fd;
		}
	      else
#endif
		{
		  /* may have changed family */
		  if (!forward->rfd4)
		    forward->rfd4 = allocate_rfd(AF_INET);
		  fd = forward->rfd4->fd;
		}
	    }
	
	  while (retry_send(sendto(fd, (char *)header, plen, 0,
				   &start->addr.sa,
				   sa_len(&start->addr))));
	  
	  if (start->addr.sa.sa_family == AF_INET) 
	    log_query(F_NOEXTRA | F_DNSSEC | F_IPV4, "retry", (struct all_addr *)&start->addr.in.sin_addr, "dnssec");
#ifdef HAVE_IPV6
	  else
	    log_query(F_NOEXTRA | F_DNSSEC | F_IPV6, "retry", (struct all_addr *)&start->addr.in6.sin6_addr, "dnssec");
#endif
	  
	  return;
	}
#endif
      
      /* In strict order mode, there must be a server later in the chain
	 left to send to, otherwise without the forwardall mechanism,
	 code further on will cycle around the list forwever if they
	 all return REFUSED. Note that server is always non-NULL before 
	 this executes. */
      if (option_bool(OPT_ORDER))
	for (server = forward->sentto->next; server; server = server->next)
	  if (!(server->flags & (SERV_LITERAL_ADDRESS | SERV_HAS_DOMAIN | SERV_FOR_NODOTS | SERV_NO_ADDR | SERV_LOOP)))
	    break;

      /* recreate query from reply */
      pheader = find_pseudoheader(header, (size_t)n, &plen, NULL, &is_sign, NULL);
      if (!is_sign && server)
	{
	  header->ancount = htons(0);
	  header->nscount = htons(0);
	  header->arcount = htons(0);
	  if ((nn = resize_packet(header, (size_t)n, pheader, plen)))
	    {
	      header->hb3 &= ~(HB3_QR | HB3_AA | HB3_TC);
	      header->hb4 &= ~(HB4_RA | HB4_RCODE | HB4_CD | HB4_AD);
	      if (forward->flags & FREC_CHECKING_DISABLED)
		header->hb4 |= HB4_CD;
	      if (forward->flags & FREC_AD_QUESTION)
		header->hb4 |= HB4_AD;
	      if (forward->flags & FREC_DO_QUESTION)
		add_do_bit(header, nn,  (unsigned char *)pheader + plen);
	      forward_query(-1, NULL, NULL, 0, header, nn, now, forward, forward->flags & FREC_AD_QUESTION, forward->flags & FREC_DO_QUESTION
#ifdef __SC_BUILD__
                        ,forward->dtype, client_mac
#endif
                        );
	      return;
	    }
	}
    }   
   
  server = forward->sentto;
  if ((forward->sentto->flags & SERV_TYPE) == 0)
    {
#ifdef __SC_BUILD__
        if (RCODE(header) != NOERROR && RCODE(header) != NXDOMAIN)
#else
      if (RCODE(header) == REFUSED)
#endif
	server = NULL;
      else
	{
	  struct server *last_server;
	  
	  /* find good server by address if possible, otherwise assume the last one we sent to */ 
	  for (last_server = daemon->servers; last_server; last_server = last_server->next)
	    if (!(last_server->flags & (SERV_LITERAL_ADDRESS | SERV_HAS_DOMAIN | SERV_FOR_NODOTS | SERV_NO_ADDR)) &&
		sockaddr_isequal(&last_server->addr, &serveraddr))
	      {
		server = last_server;
		break;
	      }
	} 
      if (!option_bool(OPT_ALL_SERVERS))
	daemon->last_server = server;
    }
 
  /* We tried resending to this server with a smaller maximum size and got an answer.
     Make that permanent. To avoid reduxing the packet size for a single dropped packet,
     only do this when we get a truncated answer, or one larger than the safe size. */
  if (server && server->edns_pktsz > SAFE_PKTSZ && (forward->flags & FREC_TEST_PKTSZ) && 
      ((header->hb3 & HB3_TC) || n >= SAFE_PKTSZ))
    {
      server->edns_pktsz = SAFE_PKTSZ;
      server->pktsz_reduced = now;
      prettyprint_addr(&server->addr, daemon->addrbuff);
      my_syslog(LOG_WARNING, _("reducing DNS packet size for nameserver %s to %d"), daemon->addrbuff, SAFE_PKTSZ);
    }

    
  /* If the answer is an error, keep the forward record in place in case
     we get a good reply from another server. Kill it when we've
     had replies from all to avoid filling the forwarding table when
     everything is broken */
#ifdef __SC_BUILD__
    if (family == AF_INET) {
        forward->v4counter--;
    } else {
        forward->v6counter--;
    }
    if (forward->forwardall == 0 || --forward->forwardall == 1 ||
            (RCODE(header) == NOERROR || RCODE(header) == NXDOMAIN )
            || (forward->islast && forward->v4counter <= 0 && forward->v6counter <= 0))
#else
  if (forward->forwardall == 0 || --forward->forwardall == 1 ||
      (RCODE(header) != REFUSED && RCODE(header) != SERVFAIL))
#endif
    {
      int check_rebind = 0, no_cache_dnssec = 0, cache_secure = 0, bogusanswer = 0;
      
      if (option_bool(OPT_NO_REBIND))
	check_rebind = !(forward->flags & FREC_NOREBIND);
      
      /*   Don't cache replies where DNSSEC validation was turned off, either
	   the upstream server told us so, or the original query specified it.  */
      if ((header->hb4 & HB4_CD) || (forward->flags & FREC_CHECKING_DISABLED))
	no_cache_dnssec = 1;
      
#ifdef HAVE_DNSSEC
      if (server && (server->flags & SERV_DO_DNSSEC) && 
	  option_bool(OPT_DNSSEC_VALID) && !(forward->flags & FREC_CHECKING_DISABLED))
	{
	  int status = 0;

	  /* We've had a reply already, which we're validating. Ignore this duplicate */
	  if (forward->blocking_query)
	    return;
	  
	   /* Truncated answer can't be validated.
	      If this is an answer to a DNSSEC-generated query, we still
	      need to get the client to retry over TCP, so return
	      an answer with the TC bit set, even if the actual answer fits.
	   */
	  if (header->hb3 & HB3_TC)
	    status = STAT_TRUNCATED;
	  
	  while (1)
	    {
	      /* As soon as anything returns BOGUS, we stop and unwind, to do otherwise
		 would invite infinite loops, since the answers to DNSKEY and DS queries
		 will not be cached, so they'll be repeated. */
	      if (status != STAT_BOGUS && status != STAT_TRUNCATED && status != STAT_ABANDONED)
		{
		  if (forward->flags & FREC_DNSKEY_QUERY)
		    status = dnssec_validate_by_ds(now, header, n, daemon->namebuff, daemon->keyname, forward->class
#ifdef __SC_BUILD__
                                , forward->dtype, client_mac
#endif
                                );
		  else if (forward->flags & FREC_DS_QUERY)
		    status = dnssec_validate_ds(now, header, n, daemon->namebuff, daemon->keyname, forward->class
#ifdef __SC_BUILD__
                                , forward->dtype, client_mac
#endif
                                );
		  else
		    status = dnssec_validate_reply(now, header, n, daemon->namebuff, daemon->keyname, &forward->class, 
						   !option_bool(OPT_DNSSEC_IGN_NS) && (server->flags & SERV_DO_DNSSEC),
						   NULL, NULL
 #ifdef __SC_BUILD__
                                                       , forward->dtype, client_mac
#endif
                                                       );
#ifdef HAVE_DUMPFILE
		  if (status == STAT_BOGUS)
		    dump_packet((forward->flags & (FREC_DNSKEY_QUERY | FREC_DS_QUERY)) ? DUMP_SEC_BOGUS : DUMP_BOGUS,
				header, (size_t)n, &serveraddr, NULL);
#endif
		}
	      
	      /* Can't validate, as we're missing key data. Put this
		 answer aside, whilst we get that. */     
	      if (status == STAT_NEED_DS || status == STAT_NEED_KEY)
		{
		  struct frec *new, *orig;
		  
		  /* Free any saved query */
		  if (forward->stash)
		    blockdata_free(forward->stash);
		  
		  /* Now save reply pending receipt of key data */
		  if (!(forward->stash = blockdata_alloc((char *)header, n)))
		    return;
		  forward->stash_len = n;
		  
		  /* Find the original query that started it all.... */
		  for (orig = forward; orig->dependent; orig = orig->dependent);
		  
		  if (--orig->work_counter == 0 || !(new = get_new_frec(now, NULL, 1)))
		    status = STAT_ABANDONED;
		  else
		    {
		      int querytype, fd, type = SERV_DO_DNSSEC;
		      struct frec *next = new->next;
		      char *domain;
		      
		      *new = *forward; /* copy everything, then overwrite */
		      new->next = next;
		      new->blocking_query = NULL;

		      /* Find server to forward to. This will normally be the 
			 same as for the original query, but may be another if
			 servers for domains are involved. */		      
#ifdef __SC_BUILD__
		      if (search_servers(NULL, now, NULL, F_DNSSECOK, daemon->keyname, &type, &domain, NULL) == 0)
#else
		      if (search_servers(now, NULL, F_DNSSECOK, daemon->keyname, &type, &domain, NULL) == 0)
#endif
			{
			  struct server *start = server, *new_server = NULL;
			  
			  while (1)
			    {
			      if (type == (start->flags & (SERV_TYPE | SERV_DO_DNSSEC)) &&
				  ((type & SERV_TYPE) != SERV_HAS_DOMAIN || hostname_isequal(domain, start->domain)) &&
				  !(start->flags & (SERV_LITERAL_ADDRESS | SERV_LOOP)))
				{
				  new_server = start;
				  if (server == start)
				    {
				      new_server = NULL;
				      break;
				    }
				}
			      
			      if (!(start = start->next))
				start = daemon->servers;
			      if (start == server)
				break;
			    }
			  
			  if (new_server)
			    server = new_server;
			}
		      
		      new->sentto = server;
		      new->rfd4 = NULL;
#ifdef HAVE_IPV6
		      new->rfd6 = NULL;
#endif
		      new->flags &= ~(FREC_DNSKEY_QUERY | FREC_DS_QUERY | FREC_HAS_EXTRADATA);
		      new->forwardall = 0;
		      
		      new->dependent = forward; /* to find query awaiting new one. */
		      forward->blocking_query = new; /* for garbage cleaning */
		      /* validate routines leave name of required record in daemon->keyname */
		      if (status == STAT_NEED_KEY)
			{
			  new->flags |= FREC_DNSKEY_QUERY; 
			  querytype = T_DNSKEY;
			}
		      else 
			{
			  new->flags |= FREC_DS_QUERY;
			  querytype = T_DS;
			}

		      nn = dnssec_generate_query(header,((unsigned char *) header) + server->edns_pktsz,
						 daemon->keyname, forward->class, querytype, server->edns_pktsz);

		      if (server->addr.sa.sa_family == AF_INET) 
			log_query(F_NOEXTRA | F_DNSSEC | F_IPV4, daemon->keyname, (struct all_addr *)&(server->addr.in.sin_addr),
				  querystr("dnssec-query", querytype));
#ifdef HAVE_IPV6
		      else
			log_query(F_NOEXTRA | F_DNSSEC | F_IPV6, daemon->keyname, (struct all_addr *)&(server->addr.in6.sin6_addr),
				  querystr("dnssec-query", querytype));
#endif
  
		      if ((hash = hash_questions(header, nn, daemon->namebuff)))
			memcpy(new->hash, hash, HASH_SIZE);
		      new->new_id = get_id();
		      header->id = htons(new->new_id);
		      /* Save query for retransmission */
		      new->stash = blockdata_alloc((char *)header, nn);
		      new->stash_len = nn;
		      
		      /* Don't resend this. */
		      daemon->srv_save = NULL;
		      
		      if (server->sfd)
			fd = server->sfd->fd;
		      else
			{
			  fd = -1;
#ifdef HAVE_IPV6
			  if (server->addr.sa.sa_family == AF_INET6)
			    {
			      if (new->rfd6 || (new->rfd6 = allocate_rfd(AF_INET6)))
				fd = new->rfd6->fd;
			    }
			  else
#endif
			    {
			      if (new->rfd4 || (new->rfd4 = allocate_rfd(AF_INET)))
				fd = new->rfd4->fd;
			    }
			}
		      
		      if (fd != -1)
			{
#ifdef HAVE_CONNTRACK
			  /* Copy connection mark of incoming query to outgoing connection. */
			  if (option_bool(OPT_CONNTRACK))
			    {
			      unsigned int mark;
			      if (get_incoming_mark(&orig->source, &orig->dest, 0, &mark))
				setsockopt(fd, SOL_SOCKET, SO_MARK, &mark, sizeof(unsigned int));
			    }
#endif
			  
#ifdef HAVE_DUMPFILE
			  dump_packet(DUMP_SEC_QUERY, (void *)header, (size_t)nn, NULL, &server->addr);
#endif
			  
			  while (retry_send(sendto(fd, (char *)header, nn, 0, 
						   &server->addr.sa, 
						   sa_len(&server->addr)))); 
			  server->queries++;
			}
		    }		  
		  return;
		}
	  
	      /* Validated original answer, all done. */
	      if (!forward->dependent)
		break;
	      
	      /* validated subsidiary query, (and cached result)
		 pop that and return to the previous query we were working on. */
	      struct frec *prev = forward->dependent;
	      free_frec(forward);
	      forward = prev;
	      forward->blocking_query = NULL; /* already gone */
	      blockdata_retrieve(forward->stash, forward->stash_len, (void *)header);
	      n = forward->stash_len;
	    }
	
	  
	  no_cache_dnssec = 0;
	  
	  if (status == STAT_TRUNCATED)
	    header->hb3 |= HB3_TC;
	  else
	    {
	      char *result, *domain = "result";
	      
	      if (status == STAT_ABANDONED)
		{
		  result = "ABANDONED";
		  status = STAT_BOGUS;
		}
	      else
		result = (status == STAT_SECURE ? "SECURE" : (status == STAT_INSECURE ? "INSECURE" : "BOGUS"));
	      
	      if (status == STAT_BOGUS && extract_request(header, n, daemon->namebuff, NULL))
		domain = daemon->namebuff;
	      
	      log_query(F_SECSTAT, domain, NULL, result);
	    }
	  
	  if (status == STAT_SECURE)
	    cache_secure = 1;
	  else if (status == STAT_BOGUS)
	    {
	      no_cache_dnssec = 1;
	      bogusanswer = 1;
	    }
	}
#endif

      /* restore CD bit to the value in the query */
      if (forward->flags & FREC_CHECKING_DISABLED)
	header->hb4 |= HB4_CD;
      else
	header->hb4 &= ~HB4_CD;
      
      if ((nn = process_reply(header, now, forward->sentto, (size_t)n, check_rebind, no_cache_dnssec, cache_secure, bogusanswer, 
			      forward->flags & FREC_AD_QUESTION, forward->flags & FREC_DO_QUESTION, 
			      forward->flags & FREC_ADDED_PHEADER, forward->flags & FREC_HAS_SUBNET, &forward->source
#ifdef __SC_BUILD__
                                ,forward->dtype, client_mac
#endif
                             )))
	{
	  header->id = htons(forward->orig_id);
	  header->hb4 |= HB4_RA; /* recursion if available */
#ifdef HAVE_DNSSEC
	  /* We added an EDNSO header for the purpose of getting DNSSEC RRs, and set the value of the UDP payload size
	     greater than the no-EDNS0-implied 512 to have space for the RRSIGS. If, having stripped them and the EDNS0
             header, the answer is still bigger than 512, truncate it and mark it so. The client then retries with TCP. */
	  if (option_bool(OPT_DNSSEC_VALID) && (forward->flags & FREC_ADDED_PHEADER) && (nn > PACKETSZ))
	    {
	      header->ancount = htons(0);
	      header->nscount = htons(0);
	      header->arcount = htons(0);
	      header->hb3 |= HB3_TC;
	      nn = resize_packet(header, nn, NULL, 0);
	    }
#endif

#ifdef HAVE_DUMPFILE
	  dump_packet(DUMP_REPLY, daemon->packet, (size_t)nn, NULL, &forward->source);
#endif
	  
	  send_from(forward->fd, option_bool(OPT_NOWILD) || option_bool (OPT_CLEVERBIND), daemon->packet, nn, 
		    &forward->source, &forward->dest, forward->iface);
	}

#ifdef __SC_BUILD__
      if(RCODE(header) != NOERROR)
      {
        if(forward->retry == 1)
            free_frec(forward);
        else
            forward->resp_err = 1;
      }
      else
#endif
      free_frec(forward); /* cancel */
    }
#ifdef __SC_BUILD__
    else {
        //do server retry when receive error and all sever received reply or timeout
        if (forward->v4counter <= 0 && forward->v6counter <= 0)
            forward->resp_err = 1;
    }
#endif

}
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
static int filter_domain_lookup(unsigned char *mac,char *domain)
{
    struct filter_domain_list *tmp = NULL;
    struct filter_domain_list *domain_node = NULL;
    char mac_str[18] = {0};
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    list_for_each_entry_safe(domain_node, tmp,&(daemon->head_filterlist), list)
    {
         if(domain_node != NULL)
         {
            if(strncmp(domain_node->domain,domain,strlen(domain_node->domain)) == 0)
            {
                if(strncasecmp(domain_node->mac, mac_str,strlen(domain_node->mac)) == 0)
                    return 0;
            }
         }
    }
    return -1;
}
#endif


void receive_query(struct listener *listen, time_t now)
{
  struct dns_header *header = (struct dns_header *)daemon->packet;
  union mysockaddr source_addr;
  unsigned char *pheader;
  unsigned short type, udp_size = PACKETSZ; /* default if no EDNS0 */
  struct all_addr dst_addr;
  struct in_addr netmask, dst_addr_4;
  size_t m;
  ssize_t n;
#ifdef __SC_BUILD__
    int domain_type;
    unsigned char client_mac[DHCP_CHADDR_MAX] = "";
#endif
  int if_index = 0, auth_dns = 0, do_bit = 0, have_pseudoheader = 0;
#ifdef HAVE_AUTH
  int local_auth = 0;
#endif
  struct iovec iov[1];
  struct msghdr msg;
  struct cmsghdr *cmptr;
  union {
    struct cmsghdr align; /* this ensures alignment */
#ifdef HAVE_IPV6
    char control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#endif
#if defined(HAVE_LINUX_NETWORK)
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#elif defined(IP_RECVDSTADDR) && defined(HAVE_SOLARIS_NETWORK)
    char control[CMSG_SPACE(sizeof(struct in_addr)) +
		 CMSG_SPACE(sizeof(unsigned int))];
#elif defined(IP_RECVDSTADDR)
    char control[CMSG_SPACE(sizeof(struct in_addr)) +
		 CMSG_SPACE(sizeof(struct sockaddr_dl))];
#endif
  } control_u;
#ifdef HAVE_IPV6
   /* Can always get recvd interface for IPv6 */
  int check_dst = !option_bool(OPT_NOWILD) || listen->family == AF_INET6;
#else
  int check_dst = !option_bool(OPT_NOWILD);
#endif

  /* packet buffer overwritten */
  daemon->srv_save = NULL;
  
  dst_addr_4.s_addr = dst_addr.addr.addr4.s_addr = 0;
  netmask.s_addr = 0;
  
  if (option_bool(OPT_NOWILD) && listen->iface)
    {
      auth_dns = listen->iface->dns_auth;
     
      if (listen->family == AF_INET)
	{
	  dst_addr_4 = dst_addr.addr.addr4 = listen->iface->addr.in.sin_addr;
	  netmask = listen->iface->netmask;
	}
    }
  
  iov[0].iov_base = daemon->packet;
  iov[0].iov_len = daemon->edns_pktsz;
    
  msg.msg_control = control_u.control;
  msg.msg_controllen = sizeof(control_u);
  msg.msg_flags = 0;
  msg.msg_name = &source_addr;
  msg.msg_namelen = sizeof(source_addr);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
#ifdef __SC_BUILD__ 
  if ((n = recvmsg(listen->fd, &msg, MSG_DONTWAIT)) == -1)
#else
  if ((n = recvmsg(listen->fd, &msg, 0)) == -1)
#endif
    return;
  
  if (n < (int)sizeof(struct dns_header) || 
      (msg.msg_flags & MSG_TRUNC) ||
      (header->hb3 & HB3_QR))
    return;

  /* Clear buffer beyond request to avoid risk of
     information disclosure. */
  memset(daemon->packet + n, 0, daemon->edns_pktsz - n);
  
  source_addr.sa.sa_family = listen->family;
  
  if (listen->family == AF_INET)
    {
       /* Source-port == 0 is an error, we can't send back to that. 
	  http://www.ietf.org/mail-archive/web/dnsop/current/msg11441.html */
      if (source_addr.in.sin_port == 0)
	return;
    }
#ifdef HAVE_IPV6
  else
    {
      /* Source-port == 0 is an error, we can't send back to that. */
      if (source_addr.in6.sin6_port == 0)
	return;
      source_addr.in6.sin6_flowinfo = 0;
    }
#endif
  
  /* We can be configured to only accept queries from at-most-one-hop-away addresses. */
  if (option_bool(OPT_LOCAL_SERVICE))
    {
      struct addrlist *addr;
#ifdef HAVE_IPV6
      if (listen->family == AF_INET6) 
	{
	  for (addr = daemon->interface_addrs; addr; addr = addr->next)
	    if ((addr->flags & ADDRLIST_IPV6) &&
		is_same_net6(&addr->addr.addr.addr6, &source_addr.in6.sin6_addr, addr->prefixlen))
	      break;
	}
      else
#endif
	{
	  struct in_addr netmask;
	  for (addr = daemon->interface_addrs; addr; addr = addr->next)
	    {
	      netmask.s_addr = htonl(~(in_addr_t)0 << (32 - addr->prefixlen));
	      if (!(addr->flags & ADDRLIST_IPV6) &&
		  is_same_net(addr->addr.addr.addr4, source_addr.in.sin_addr, netmask))
		break;
	    }
	}
      if (!addr)
	{
	  static int warned = 0;
	  if (!warned)
	    {
	      my_syslog(LOG_WARNING, _("Ignoring query from non-local network"));
	      warned = 1;
	    }
	  return;
	}
    }
		
  if (check_dst)
    {
      struct ifreq ifr;

      if (msg.msg_controllen < sizeof(struct cmsghdr))
	return;

#if defined(HAVE_LINUX_NETWORK)
      if (listen->family == AF_INET)
	for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	  if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_PKTINFO)
	    {
	      union {
		unsigned char *c;
		struct in_pktinfo *p;
	      } p;
	      p.c = CMSG_DATA(cmptr);
	      dst_addr_4 = dst_addr.addr.addr4 = p.p->ipi_spec_dst;
	      if_index = p.p->ipi_ifindex;
	    }
#elif defined(IP_RECVDSTADDR) && defined(IP_RECVIF)
      if (listen->family == AF_INET)
	{
	  for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	    {
	      union {
		unsigned char *c;
		unsigned int *i;
		struct in_addr *a;
#ifndef HAVE_SOLARIS_NETWORK
		struct sockaddr_dl *s;
#endif
	      } p;
	       p.c = CMSG_DATA(cmptr);
	       if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVDSTADDR)
		 dst_addr_4 = dst_addr.addr.addr4 = *(p.a);
	       else if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVIF)
#ifdef HAVE_SOLARIS_NETWORK
		 if_index = *(p.i);
#else
  	         if_index = p.s->sdl_index;
#endif
	    }
	}
#endif
      
#ifdef HAVE_IPV6
      if (listen->family == AF_INET6)
	{
	  for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
	    if (cmptr->cmsg_level == IPPROTO_IPV6 && cmptr->cmsg_type == daemon->v6pktinfo)
	      {
		union {
		  unsigned char *c;
		  struct in6_pktinfo *p;
		} p;
		p.c = CMSG_DATA(cmptr);
		  
		dst_addr.addr.addr6 = p.p->ipi6_addr;
		if_index = p.p->ipi6_ifindex;
	      }
	}
#endif
      
      /* enforce available interface configuration */
      
      if (!indextoname(listen->fd, if_index, ifr.ifr_name))
	return;
      
      if (!iface_check(listen->family, &dst_addr, ifr.ifr_name, &auth_dns))
	{
	   if (!option_bool(OPT_CLEVERBIND))
	     enumerate_interfaces(0); 
	   if (!loopback_exception(listen->fd, listen->family, &dst_addr, ifr.ifr_name) &&
	       !label_exception(if_index, listen->family, &dst_addr))
	     return;
	}

      if (listen->family == AF_INET && option_bool(OPT_LOCALISE))
	{
	  struct irec *iface;
	  
	  /* get the netmask of the interface which has the address we were sent to.
	     This is no necessarily the interface we arrived on. */
	  
	  for (iface = daemon->interfaces; iface; iface = iface->next)
	    if (iface->addr.sa.sa_family == AF_INET &&
		iface->addr.in.sin_addr.s_addr == dst_addr_4.s_addr)
	      break;
	  
	  /* interface may be new */
	  if (!iface && !option_bool(OPT_CLEVERBIND))
	    enumerate_interfaces(0); 
	  
	  for (iface = daemon->interfaces; iface; iface = iface->next)
	    if (iface->addr.sa.sa_family == AF_INET &&
		iface->addr.in.sin_addr.s_addr == dst_addr_4.s_addr)
	      break;
	  
	  /* If we failed, abandon localisation */
	  if (iface)
	    netmask = iface->netmask;
	  else
	    dst_addr_4.s_addr = 0;
	}
    }
   
  /* log_query gets called indirectly all over the place, so 
     pass these in global variables - sorry. */
  daemon->log_display_id = ++daemon->log_id;
  daemon->log_source_addr = &source_addr;

#ifdef HAVE_DUMPFILE
  dump_packet(DUMP_QUERY, daemon->packet, (size_t)n, &source_addr, NULL);
#endif
	  
  if (extract_request(header, (size_t)n, daemon->namebuff, &type))
    {
#ifdef HAVE_AUTH
      struct auth_zone *zone;
#endif
      char *types = querystr(auth_dns ? "auth" : "query", type);
      
      if (listen->family == AF_INET) 
	log_query(F_QUERY | F_IPV4 | F_FORWARD, daemon->namebuff, 
		  (struct all_addr *)&source_addr.in.sin_addr, types);
#ifdef HAVE_IPV6
      else
	log_query(F_QUERY | F_IPV6 | F_FORWARD, daemon->namebuff, 
		  (struct all_addr *)&source_addr.in6.sin6_addr, types);
#endif

#ifdef HAVE_AUTH
      /* find queries for zones we're authoritative for, and answer them directly */
      if (!auth_dns && !option_bool(OPT_LOCALISE))
	for (zone = daemon->auth_zones; zone; zone = zone->next)
	  if (in_zone(zone, daemon->namebuff, NULL))
	    {
	      auth_dns = 1;
	      local_auth = 1;
	      break;
	    }
#endif
      
#ifdef HAVE_LOOP
      /* Check for forwarding loop */
      if (detect_loop(daemon->namebuff, type))
	return;
#endif
    }
  
  if (find_pseudoheader(header, (size_t)n, NULL, &pheader, NULL, NULL))
    { 
      unsigned short flags;
      
      have_pseudoheader = 1;
      GETSHORT(udp_size, pheader);
      pheader += 2; /* ext_rcode */
      GETSHORT(flags, pheader);
      
      if (flags & 0x8000)
	do_bit = 1;/* do bit */ 
	
      /* If the client provides an EDNS0 UDP size, use that to limit our reply.
	 (bounded by the maximum configured). If no EDNS0, then it
	 defaults to 512 */
      if (udp_size > daemon->edns_pktsz)
	udp_size = daemon->edns_pktsz;
      else if (udp_size < PACKETSZ)
	udp_size = PACKETSZ; /* Sanity check - can't reduce below default. RFC 6891 6.2.3 */
    }

#ifdef __SC_BUILD__
        domain_type = sc_dtype_parse((char *)header, &n);
        my_syslog(LOG_DEBUG, "domain_type is [%d]\n", domain_type);
        find_mac(&source_addr, client_mac, 1, now);
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
        if(filter_domain_lookup(client_mac, daemon->namebuff) == 0)
        {
            return 0;
        }
#endif
#endif

#ifdef HAVE_AUTH
  if (auth_dns)
    {
      m = answer_auth(header, ((char *) header) + udp_size, (size_t)n, now, &source_addr, 
		      local_auth, do_bit, have_pseudoheader
#ifdef __SC_BUILD__
                        ,domain_type, client_mac
#endif
                     ); 
      if (m >= 1)
	{
#ifdef __SC_BUILD__
            sc_dns_answer_parse(domain_type, (char *)header, &m);
#endif

	  send_from(listen->fd, option_bool(OPT_NOWILD) || option_bool(OPT_CLEVERBIND),
		    (char *)header, m, &source_addr, &dst_addr, if_index);
	  daemon->metrics[METRIC_DNS_AUTH_ANSWERED]++;
	}
    }
  else
#endif
    {
      int ad_reqd = do_bit;
       /* RFC 6840 5.7 */
      if (header->hb4 & HB4_AD)
	ad_reqd = 1;

      m = answer_request(header, ((char *) header) + udp_size, (size_t)n, 
			 dst_addr_4, netmask, now, ad_reqd, do_bit, have_pseudoheader
#ifdef __SC_BUILD__
                            ,domain_type, client_mac
#endif
                        ); 
      
      if (m >= 1)
	{
#ifdef __SC_BUILD__
            sc_dns_answer_parse(domain_type, (char *)header, &m);
#endif

	  send_from(listen->fd, option_bool(OPT_NOWILD) || option_bool(OPT_CLEVERBIND),
		    (char *)header, m, &source_addr, &dst_addr, if_index);
	  daemon->metrics[METRIC_DNS_LOCAL_ANSWERED]++;
	}
      else if (forward_query(listen->fd, &source_addr, &dst_addr, if_index,
			     header, (size_t)n, now, NULL, ad_reqd, do_bit
#ifdef __SC_BUILD__
                        ,domain_type, client_mac
#endif
                        ))
	daemon->metrics[METRIC_DNS_QUERIES_FORWARDED]++;
      else
	daemon->metrics[METRIC_DNS_LOCAL_ANSWERED]++;
    }
}

#ifdef HAVE_DNSSEC
/* Recurse up the key hierarchy */
static int tcp_key_recurse(time_t now, int status, struct dns_header *header, size_t n, 
			   int class, char *name, char *keyname, struct server *server, 
			   int have_mark, unsigned int mark, int *keycount
#ifdef __SC_BUILD__
                            ,int dtype, unsigned char *client_mac
#endif
                          )
{
  int new_status;
  unsigned char *packet = NULL;
  unsigned char *payload = NULL;
  struct dns_header *new_header = NULL;
  u16 *length = NULL;
 
  while (1)
    {
      int type = SERV_DO_DNSSEC;
      char *domain;
      size_t m; 
      unsigned char c1, c2;
      struct server *firstsendto = NULL;
      
      /* limit the amount of work we do, to avoid cycling forever on loops in the DNS */
      if (--(*keycount) == 0)
	new_status = STAT_ABANDONED;
      else if (status == STAT_NEED_KEY)
	new_status = dnssec_validate_by_ds(now, header, n, name, keyname, class
#ifdef __SC_BUILD__
                                            , dtype, client_mac
#endif
                                      );
      else if (status == STAT_NEED_DS)
	new_status = dnssec_validate_ds(now, header, n, name, keyname, class
#ifdef __SC_BUILD__
                                            , dtype, client_mac
#endif
                                   );
      else 
	new_status = dnssec_validate_reply(now, header, n, name, keyname, &class,
					   !option_bool(OPT_DNSSEC_IGN_NS) && (server->flags & SERV_DO_DNSSEC),
					   NULL, NULL
#ifdef __SC_BUILD__
                                               , dtype, client_mac
#endif
                                      );
      
      if (new_status != STAT_NEED_DS && new_status != STAT_NEED_KEY)
	break;

      /* Can't validate because we need a key/DS whose name now in keyname.
	 Make query for same, and recurse to validate */
      if (!packet)
	{
	  packet = whine_malloc(65536 + MAXDNAME + RRFIXEDSZ + sizeof(u16));
	  payload = &packet[2];
	  new_header = (struct dns_header *)payload;
	  length = (u16 *)packet;
	}
      
      if (!packet)
	{
	  new_status = STAT_ABANDONED;
	  break;
	}

      m = dnssec_generate_query(new_header, ((unsigned char *) new_header) + 65536, keyname, class, 
				new_status == STAT_NEED_KEY ? T_DNSKEY : T_DS, server->edns_pktsz);
      
      *length = htons(m);

      /* Find server to forward to. This will normally be the 
	 same as for the original query, but may be another if
	 servers for domains are involved. */		      
#ifdef __SC_BUILD__
      if (search_servers(now, NULL, F_DNSSECOK, keyname, &type, &domain, NULL) != 0)
#else
      if (search_servers(now, NULL, F_DNSSECOK, keyname, &type, &domain, NULL) != 0)
#endif
	{
	  new_status = STAT_ABANDONED;
	  break;
	}
	
      while (1)
	{
	  if (!firstsendto)
	    firstsendto = server;
	  else
	    {
	      if (!(server = server->next))
		server = daemon->servers;
	      if (server == firstsendto)
		{
		  /* can't find server to accept our query. */
		  new_status = STAT_ABANDONED;
		  break;
		}
	    }
	  
	  if (type != (server->flags & (SERV_TYPE | SERV_DO_DNSSEC)) ||
	      (type == SERV_HAS_DOMAIN && !hostname_isequal(domain, server->domain)) ||
	      (server->flags & (SERV_LITERAL_ADDRESS | SERV_LOOP)))
	    continue;

	retry:
	  /* may need to make new connection. */
	  if (server->tcpfd == -1)
	    {
	      if ((server->tcpfd = socket(server->addr.sa.sa_family, SOCK_STREAM, 0)) == -1)
		continue; /* No good, next server */
	      
#ifdef HAVE_CONNTRACK
	      /* Copy connection mark of incoming query to outgoing connection. */
	      if (have_mark)
		setsockopt(server->tcpfd, SOL_SOCKET, SO_MARK, &mark, sizeof(unsigned int));
#endif	
	      
	      if (!local_bind(server->tcpfd,  &server->source_addr, server->interface, 0, 1) ||
		  connect(server->tcpfd, &server->addr.sa, sa_len(&server->addr)) == -1)
		{
		  close(server->tcpfd);
		  server->tcpfd = -1;
		  continue; /* No good, next server */
		}
	      
	      server->flags &= ~SERV_GOT_TCP;
	    }
#ifdef __SC_BUILD__
	  if (!read_write(server->tcpfd, packet, m + sizeof(u16), 0, 0) ||
	      !read_write(server->tcpfd, &c1, 1, 1, 0) ||
	      !read_write(server->tcpfd, &c2, 1, 1, 0) ||
	      !read_write(server->tcpfd, payload, (c1 << 8) | c2, 1, 0))

#else      
	  if (!read_write(server->tcpfd, packet, m + sizeof(u16), 0) ||
	      !read_write(server->tcpfd, &c1, 1, 1) ||
	      !read_write(server->tcpfd, &c2, 1, 1) ||
	      !read_write(server->tcpfd, payload, (c1 << 8) | c2, 1))
#endif	 
      {
	      close(server->tcpfd);
	      server->tcpfd = -1;
	      /* We get data then EOF, reopen connection to same server,
		 else try next. This avoids DoS from a server which accepts
		 connections and then closes them. */
	      if (server->flags & SERV_GOT_TCP)
		goto retry;
	      else
		continue;
	    }


	  if (server->addr.sa.sa_family == AF_INET) 
	    log_query(F_NOEXTRA | F_DNSSEC | F_IPV4, keyname, (struct all_addr *)&(server->addr.in.sin_addr),
		      querystr("dnssec-query", new_status == STAT_NEED_KEY ? T_DNSKEY : T_DS));
#ifdef HAVE_IPV6
	  else
	    log_query(F_NOEXTRA | F_DNSSEC | F_IPV6, keyname, (struct all_addr *)&(server->addr.in6.sin6_addr),
		      querystr("dnssec-query", new_status == STAT_NEED_KEY ? T_DNSKEY : T_DS));
#endif
	  
	  server->flags |= SERV_GOT_TCP;
	  
	  m = (c1 << 8) | c2;
	  new_status = tcp_key_recurse(now, new_status, new_header, m, class, name, keyname, server, have_mark, mark, keycount
#ifdef __SC_BUILD__
                    ,dtype, client_mac
#endif
                    );
	  break;
	}
      
      if (new_status != STAT_OK)
	break;
    }
    
  if (packet)
    free(packet);
    
  return new_status;
}
#endif


/* The daemon forks before calling this: it should deal with one connection,
   blocking as necessary, and then return. Note, need to be a bit careful
   about resources for debug mode, when the fork is suppressed: that's
   done by the caller. */
unsigned char *tcp_request(int confd, time_t now,
			   union mysockaddr *local_addr, struct in_addr netmask, int auth_dns)
{
  size_t size = 0;
#ifdef __SC_BUILD__
    int domain_type;
    int priority;
    unsigned char client_mac[DHCP_CHADDR_MAX] = "";
#endif
  int norebind = 0;
#ifdef HAVE_AUTH
  int local_auth = 0;
#endif
  int checking_disabled, do_bit, added_pheader = 0, have_pseudoheader = 0;
  int check_subnet, no_cache_dnssec = 0, cache_secure = 0, bogusanswer = 0;
  size_t m;
  unsigned short qtype;
  unsigned int gotname;
  unsigned char c1, c2;
  /* Max TCP packet + slop + size */
  unsigned char *packet = whine_malloc(65536 + MAXDNAME + RRFIXEDSZ + sizeof(u16));
  unsigned char *payload = &packet[2];
  /* largest field in header is 16-bits, so this is still sufficiently aligned */
  struct dns_header *header = (struct dns_header *)payload;
  u16 *length = (u16 *)packet;
  struct server *last_server;
  struct in_addr dst_addr_4;
  union mysockaddr peer_addr;
  socklen_t peer_len = sizeof(union mysockaddr);
  int query_count = 0;
  unsigned char *pheader;
  unsigned int mark = 0;
  int have_mark = 0;

  (void)mark;
  (void)have_mark;

  if (getpeername(confd, (struct sockaddr *)&peer_addr, &peer_len) == -1)
    return packet;

#ifdef HAVE_CONNTRACK
  /* Get connection mark of incoming query to set on outgoing connections. */
  if (option_bool(OPT_CONNTRACK))
    {
      struct all_addr local;
#ifdef HAVE_IPV6		      
      if (local_addr->sa.sa_family == AF_INET6)
	local.addr.addr6 = local_addr->in6.sin6_addr;
      else
#endif
	local.addr.addr4 = local_addr->in.sin_addr;
      
      have_mark = get_incoming_mark(&peer_addr, &local, 1, &mark);
    }
#endif	

  /* We can be configured to only accept queries from at-most-one-hop-away addresses. */
  if (option_bool(OPT_LOCAL_SERVICE))
    {
      struct addrlist *addr;
#ifdef HAVE_IPV6
      if (peer_addr.sa.sa_family == AF_INET6) 
	{
	  for (addr = daemon->interface_addrs; addr; addr = addr->next)
	    if ((addr->flags & ADDRLIST_IPV6) &&
		is_same_net6(&addr->addr.addr.addr6, &peer_addr.in6.sin6_addr, addr->prefixlen))
	      break;
	}
      else
#endif
	{
	  struct in_addr netmask;
	  for (addr = daemon->interface_addrs; addr; addr = addr->next)
	    {
	      netmask.s_addr = htonl(~(in_addr_t)0 << (32 - addr->prefixlen));
	      if (!(addr->flags & ADDRLIST_IPV6) && 
		  is_same_net(addr->addr.addr.addr4, peer_addr.in.sin_addr, netmask))
		break;
	    }
	}
      if (!addr)
	{
	  my_syslog(LOG_WARNING, _("Ignoring query from non-local network"));
	  return packet;
	}
    }
#ifdef __SC_BUILD__
  find_mac(&peer_addr, client_mac, 1, now);
#endif

  while (1)
    {
#ifdef __SC_BUILD__
        priority = -1;
#endif

      if (query_count == TCP_MAX_QUERIES ||
	  !packet ||
#ifdef __SC_BUILD__
	  !read_write(confd, &c1, 1, 1, 0) || !read_write(confd, &c2, 1, 1, 0) ||
#else
	  !read_write(confd, &c1, 1, 1) || !read_write(confd, &c2, 1, 1) ||
#endif
      !(size = c1 << 8 | c2) ||
#ifdef __SC_BUILD__
	  !read_write(confd, payload, size, 1, 0)
#else
	  !read_write(confd, payload, size, 1)
#endif
      )
       	return packet; 
  
      if (size < (int)sizeof(struct dns_header))
	continue;

      /* Clear buffer beyond request to avoid risk of
	 information disclosure. */
      memset(payload + size, 0, 65536 - size);
      
      query_count++;

      /* log_query gets called indirectly all over the place, so 
	 pass these in global variables - sorry. */
      daemon->log_display_id = ++daemon->log_id;
      daemon->log_source_addr = &peer_addr;
      
      /* save state of "cd" flag in query */
      if ((checking_disabled = header->hb4 & HB4_CD))
	no_cache_dnssec = 1;
       
      if ((gotname = extract_request(header, (unsigned int)size, daemon->namebuff, &qtype)))
	{
#ifdef HAVE_AUTH
	  struct auth_zone *zone;
#endif
	  char *types = querystr(auth_dns ? "auth" : "query", qtype);
	  
	  if (peer_addr.sa.sa_family == AF_INET) 
	    log_query(F_QUERY | F_IPV4 | F_FORWARD, daemon->namebuff, 
		      (struct all_addr *)&peer_addr.in.sin_addr, types);
#ifdef HAVE_IPV6
	  else
	    log_query(F_QUERY | F_IPV6 | F_FORWARD, daemon->namebuff, 
		      (struct all_addr *)&peer_addr.in6.sin6_addr, types);
#endif
	  
#ifdef HAVE_AUTH
	  /* find queries for zones we're authoritative for, and answer them directly */
	  if (!auth_dns && !option_bool(OPT_LOCALISE))
	    for (zone = daemon->auth_zones; zone; zone = zone->next)
	      if (in_zone(zone, daemon->namebuff, NULL))
		{
		  auth_dns = 1;
		  local_auth = 1;
		  break;
		}
#endif
	}
      
      if (local_addr->sa.sa_family == AF_INET)
	dst_addr_4 = local_addr->in.sin_addr;
      else
	dst_addr_4.s_addr = 0;
      
      do_bit = 0;

      if (find_pseudoheader(header, (size_t)size, NULL, &pheader, NULL, NULL))
	{ 
	  unsigned short flags;
	  
	  have_pseudoheader = 1;
	  pheader += 4; /* udp_size, ext_rcode */
	  GETSHORT(flags, pheader);
      
	  if (flags & 0x8000)
	    do_bit = 1; /* do bit */ 
	}

#ifdef __SC_BUILD__
            domain_type = sc_dtype_parse((char *)header, &size);
            my_syslog(LOG_DEBUG, "tcp domain_type [%d]\n", domain_type);
#endif
#ifdef HAVE_AUTH
      if (auth_dns)
	m = answer_auth(header, ((char *) header) + 65536, (size_t)size, now, &peer_addr, 
			local_auth, do_bit, have_pseudoheader
#ifdef __SC_BUILD__
                            , domain_type, client_mac
#endif
                            ); 
      else
#endif
	{
	   int ad_reqd = do_bit;
	   /* RFC 6840 5.7 */
	   if (header->hb4 & HB4_AD)
	     ad_reqd = 1;
	   
	   /* m > 0 if answered from cache */
	   m = answer_request(header, ((char *) header) + 65536, (size_t)size, 
			      dst_addr_4, netmask, now, ad_reqd, do_bit, have_pseudoheader
#ifdef __SC_BUILD__
                               , domain_type, client_mac
#endif
                               );
	  
	  /* Do this by steam now we're not in the select() loop */
	  check_log_writer(1); 
	  
	  if (m == 0)
	    {
	      unsigned int flags = 0;
	      struct all_addr *addrp = NULL;
	      int type = SERV_DO_DNSSEC;
	      char *domain = NULL;
	      unsigned char *oph = find_pseudoheader(header, size, NULL, NULL, NULL, NULL);

	      size = add_edns0_config(header, size, ((unsigned char *) header) + 65536, &peer_addr, now, &check_subnet);

	      if (gotname)
#ifdef __SC_BUILD__
		flags = search_servers(NULL, now, &addrp, gotname, daemon->namebuff, &type, &domain, &norebind);
#else
		flags = search_servers(now, &addrp, gotname, daemon->namebuff, &type, &domain, &norebind);
#endif

#ifdef HAVE_DNSSEC
	      if (option_bool(OPT_DNSSEC_VALID) && (type & SERV_DO_DNSSEC))
		{
		  size = add_do_bit(header, size, ((unsigned char *) header) + 65536);
		  
		  /* For debugging, set Checking Disabled, otherwise, have the upstream check too,
		     this allows it to select auth servers when one is returning bad data. */
		  if (option_bool(OPT_DNSSEC_DEBUG))
		    header->hb4 |= HB4_CD;
		}
#endif

	      /* Check if we added a pheader on forwarding - may need to
		 strip it from the reply. */
	      if (!oph && find_pseudoheader(header, size, NULL, NULL, NULL, NULL))
		added_pheader = 1;

	      type &= ~SERV_DO_DNSSEC;
	      
	      if (type != 0  || option_bool(OPT_ORDER) || !daemon->last_server)
		last_server = daemon->servers;
	      else
		last_server = daemon->last_server;
	      
	      if (!flags && last_server)
		{
		  struct server *firstsendto = NULL;
#ifdef HAVE_DNSSEC
		  unsigned char *newhash, hash[HASH_SIZE];
		  if ((newhash = hash_questions(header, (unsigned int)size, daemon->namebuff)))
		    memcpy(hash, newhash, HASH_SIZE);
		  else
		    memset(hash, 0, HASH_SIZE);
#else
		  unsigned int crc = questions_crc(header, (unsigned int)size, daemon->namebuff);
#endif		 
#ifdef __SC_BUILD__
          do {
              priority = sc_server_search(type, domain, domain_type, priority);
#endif

		  /* Loop round available servers until we succeed in connecting to one.
		     Note that this code subtly ensures that consecutive queries on this connection
		     which can go to the same server, do so. */
		  while (1) 
		    {
		      if (!firstsendto)
			firstsendto = last_server;
		      else
			{
			  if (!(last_server = last_server->next))
			    last_server = daemon->servers;
			  
			  if (last_server == firstsendto)
			    break;
			}
		      
		      /* server for wrong domain */
		      if (type != (last_server->flags & SERV_TYPE) ||
			  (type == SERV_HAS_DOMAIN && !hostname_isequal(domain, last_server->domain)) ||
			  (last_server->flags & (SERV_LITERAL_ADDRESS | SERV_LOOP)))
			continue;

		    retry:
		      if (last_server->tcpfd == -1)
			{
			  if ((last_server->tcpfd = socket(last_server->addr.sa.sa_family, SOCK_STREAM, 0)) == -1)
			    continue;
			  
#ifdef HAVE_CONNTRACK
			  /* Copy connection mark of incoming query to outgoing connection. */
			  if (have_mark)
			    setsockopt(last_server->tcpfd, SOL_SOCKET, SO_MARK, &mark, sizeof(unsigned int));
#endif	
		      
			  if ((!local_bind(last_server->tcpfd,  &last_server->source_addr, last_server->interface, 0, 1) ||
#ifdef __SC_BUILD__
                                       
                   connect_timeout(last_server->tcpfd, &last_server->addr.sa, sa_len(&last_server->addr), daemon->query_tcp_connect_timeout) == -1
#else

			       connect(last_server->tcpfd, &last_server->addr.sa, sa_len(&last_server->addr)) == -1
#endif
                   ))
			    {
			      close(last_server->tcpfd);
			      last_server->tcpfd = -1;
			      continue;
			    }
			  
			  last_server->flags &= ~SERV_GOT_TCP;
			}
		      
		      *length = htons(size);

		      /* get query name again for logging - may have been overwritten */
		      if (!(gotname = extract_request(header, (unsigned int)size, daemon->namebuff, &qtype)))
			strcpy(daemon->namebuff, "query");
		      
#ifdef __SC_BUILD__
              if (!read_write(last_server->tcpfd, packet, size + sizeof(u16), 0, 0) ||
			  !read_write(last_server->tcpfd, &c1, 1, 1, daemon->query_tcp_timeout) ||
			  !read_write(last_server->tcpfd, &c2, 1, 1, daemon->query_tcp_timeout) ||
			  !read_write(last_server->tcpfd, payload, (c1 << 8) | c2, 1, daemon->query_tcp_timeout))

#else
		      if (!read_write(last_server->tcpfd, packet, size + sizeof(u16), 0) ||
			  !read_write(last_server->tcpfd, &c1, 1, 1) ||
			  !read_write(last_server->tcpfd, &c2, 1, 1) ||
			  !read_write(last_server->tcpfd, payload, (c1 << 8) | c2, 1))
#endif
			{
			  close(last_server->tcpfd);
			  last_server->tcpfd = -1;
			  /* We get data then EOF, reopen connection to same server,
			     else try next. This avoids DoS from a server which accepts
			     connections and then closes them. */
			  if (last_server->flags & SERV_GOT_TCP)
			    goto retry;
			  else
			    continue;
			}
		      
		      last_server->flags |= SERV_GOT_TCP;

		      m = (c1 << 8) | c2;
		      
		      if (last_server->addr.sa.sa_family == AF_INET)
			log_query(F_SERVER | F_IPV4 | F_FORWARD, daemon->namebuff, 
				  (struct all_addr *)&last_server->addr.in.sin_addr, NULL); 
#ifdef HAVE_IPV6
		      else
			log_query(F_SERVER | F_IPV6 | F_FORWARD, daemon->namebuff, 
				  (struct all_addr *)&last_server->addr.in6.sin6_addr, NULL);
#endif 

#ifdef HAVE_DNSSEC
		      if (option_bool(OPT_DNSSEC_VALID) && !checking_disabled && (last_server->flags & SERV_DO_DNSSEC))
			{
			  int keycount = DNSSEC_WORK; /* Limit to number of DNSSEC questions, to catch loops and avoid filling cache. */
			  int status = tcp_key_recurse(now, STAT_OK, header, m, 0, daemon->namebuff, daemon->keyname, 
						       last_server, have_mark, mark, &keycount
#ifdef __SC_BUILD__
                                                         , dtype, client_mac
#endif
                                          ); 
			  char *result, *domain = "result";
			  
			  if (status == STAT_ABANDONED)
			    {
			      result = "ABANDONED";
			      status = STAT_BOGUS;
			    }
			  else
			    result = (status == STAT_SECURE ? "SECURE" : (status == STAT_INSECURE ? "INSECURE" : "BOGUS"));
			  
			  if (status == STAT_BOGUS && extract_request(header, m, daemon->namebuff, NULL))
			    domain = daemon->namebuff;

			  log_query(F_SECSTAT, domain, NULL, result);
			  
			  if (status == STAT_BOGUS)
			    {
			      no_cache_dnssec = 1;
			      bogusanswer = 1;
			    }

			  if (status == STAT_SECURE)
			    cache_secure = 1;
			}
#endif

		      /* restore CD bit to the value in the query */
		      if (checking_disabled)
			header->hb4 |= HB4_CD;
		      else
			header->hb4 &= ~HB4_CD;
		      
		      /* There's no point in updating the cache, since this process will exit and
			 lose the information after a few queries. We make this call for the alias and 
			 bogus-nxdomain side-effects. */
		      /* If the crc of the question section doesn't match the crc we sent, then
			 someone might be attempting to insert bogus values into the cache by 
			 sending replies containing questions and bogus answers. */
#ifdef HAVE_DNSSEC
		      newhash = hash_questions(header, (unsigned int)m, daemon->namebuff);
		      if (!newhash || memcmp(hash, newhash, HASH_SIZE) != 0)
			{ 
			  m = 0;
#ifdef __SC_BUILD__
                            priority = -1;
#endif
			  break;
			}
#else			  
		      if (crc != questions_crc(header, (unsigned int)m, daemon->namebuff))
			{
			  m = 0;
#ifdef __SC_BUILD__
              priority = -1;
#endif

			  break;
			}
#endif

		      m = process_reply(header, now, last_server, (unsigned int)m, 
					option_bool(OPT_NO_REBIND) && !norebind, no_cache_dnssec, cache_secure, bogusanswer,
					ad_reqd, do_bit, added_pheader, check_subnet, &peer_addr
#ifdef __SC_BUILD__
                    , domain_type, client_mac
#endif
                               );

#ifdef __SC_BUILD__
              priority = -1;
#endif
		      break;
		    }
#ifdef __SC_BUILD__
          } while (priority != -1);
#endif
		}
	
	      /* In case of local answer or no connections made. */
	      if (m == 0)
		{
		  m = setup_reply(header, (unsigned int)size, addrp, flags, daemon->local_ttl);
		  if (have_pseudoheader)
		    m = add_pseudoheader(header, m, ((unsigned char *) header) + 65536, daemon->edns_pktsz, 0, NULL, 0, do_bit, 0);
		}
	    }
	}
	  
      check_log_writer(1);
#ifdef __SC_BUILD__
        sc_dns_answer_parse(domain_type, (char *)header, &m);
#endif
   
      *length = htons(m);

#ifdef __SC_BUILD__
		if (m == 0 || !read_write(confd, packet, m + sizeof(u16), 0, 0))
#else
      if (m == 0 || !read_write(confd, packet, m + sizeof(u16), 0))
#endif
	return packet;
    }
}

static struct frec *allocate_frec(time_t now)
{
  struct frec *f;
  
  if ((f = (struct frec *)whine_malloc(sizeof(struct frec))))
    {
      f->next = daemon->frec_list;
      f->time = now;
      f->sentto = NULL;
      f->rfd4 = NULL;
      f->flags = 0;
#ifdef __SC_BUILD__
      f->plen = 0;
      f->try_count = 0;
#endif
#ifdef HAVE_IPV6
      f->rfd6 = NULL;
#endif
#ifdef __SC_BUILD__
      gettimeofday(&f->tv,NULL);
      memset(f->header, 0, sizeof(*f->header));
#endif
#ifdef HAVE_DNSSEC
      f->dependent = NULL;
      f->blocking_query = NULL;
      f->stash = NULL;
#endif
      daemon->frec_list = f;
    }

  return f;
}

struct randfd *allocate_rfd(int family)
{
  static int finger = 0;
  int i;

  /* limit the number of sockets we have open to avoid starvation of 
     (eg) TFTP. Once we have a reasonable number, randomness should be OK */

  for (i = 0; i < RANDOM_SOCKS; i++)
    if (daemon->randomsocks[i].refcount == 0)
      {
	if ((daemon->randomsocks[i].fd = random_sock(family)) == -1)
	  break;
      
	daemon->randomsocks[i].refcount = 1;
	daemon->randomsocks[i].family = family;
	return &daemon->randomsocks[i];
      }

  /* No free ones or cannot get new socket, grab an existing one */
  for (i = 0; i < RANDOM_SOCKS; i++)
    {
      int j = (i+finger) % RANDOM_SOCKS;
      if (daemon->randomsocks[j].refcount != 0 &&
	  daemon->randomsocks[j].family == family && 
	  daemon->randomsocks[j].refcount != 0xffff)
	{
	  finger = j;
	  daemon->randomsocks[j].refcount++;
	  return &daemon->randomsocks[j];
	}
    }

  return NULL; /* doom */
}

void free_rfd(struct randfd *rfd)
{
  if (rfd && --(rfd->refcount) == 0)
    close(rfd->fd);
}

static void free_frec(struct frec *f)
{
  free_rfd(f->rfd4);
  f->rfd4 = NULL;
  f->sentto = NULL;
  f->flags = 0;
#ifdef __SC_BUILD__
    f->plen = 0;
    f->try_count = 0;
    f->islast = 0;
    f->resp_err = 0;
    f->v4counter = 0;
    f->v6counter = 0;
    memset(f->header, 0, sizeof(*f->header));
#endif
 
#ifdef HAVE_IPV6
  free_rfd(f->rfd6);
  f->rfd6 = NULL;
#endif

#ifdef HAVE_DNSSEC
  if (f->stash)
    {
      blockdata_free(f->stash);
      f->stash = NULL;
    }

  /* Anything we're waiting on is pointless now, too */
  if (f->blocking_query)
    free_frec(f->blocking_query);
  f->blocking_query = NULL;
  f->dependent = NULL;
#endif
}



/* if wait==NULL return a free or older than TIMEOUT record.
   else return *wait zero if one available, or *wait is delay to
   when the oldest in-use record will expire. Impose an absolute
   limit of 4*TIMEOUT before we wipe things (for random sockets).
   If force is set, always return a result, even if we have
   to allocate above the limit. */
struct frec *get_new_frec(time_t now, int *wait, int force)
{
  struct frec *f, *oldest, *target;
  int count;
  
  if (wait)
    *wait = 0;

  for (f = daemon->frec_list, oldest = NULL, target =  NULL, count = 0; f; f = f->next, count++)
    if (!f->sentto)
      target = f;
    else 
      {
#ifdef HAVE_DNSSEC
	    /* Don't free DNSSEC sub-queries here, as we may end up with
	       dangling references to them. They'll go when their "real" query 
	       is freed. */
	    if (!f->dependent)
#endif
	      {
		if (difftime(now, f->time) >= 4*TIMEOUT)
		  {
		    free_frec(f);
		    target = f;
		  }
	     
	    
		if (!oldest || difftime(f->time, oldest->time) <= 0)
		  oldest = f;
	      }
      }

  if (target)
    {
      target->time = now;
      return target;
    }
  
  /* can't find empty one, use oldest if there is one
     and it's older than timeout */
  if (!force && oldest && ((int)difftime(now, oldest->time)) >= TIMEOUT)
    { 
      /* keep stuff for twice timeout if we can by allocating a new
	 record instead */
      if (difftime(now, oldest->time) < 2*TIMEOUT && 
	  count <= daemon->ftabsize &&
	  (f = allocate_frec(now)))
	return f;

      if (!wait)
	{
	  free_frec(oldest);
	  oldest->time = now;
	}
      return oldest;
    }
  
  /* none available, calculate time 'till oldest record expires */
  if (!force && count > daemon->ftabsize)
    {
      static time_t last_log = 0;
      
      if (oldest && wait)
	*wait = oldest->time + (time_t)TIMEOUT - now;
      
      if ((int)difftime(now, last_log) > 5)
	{
	  last_log = now;
	  my_syslog(LOG_WARNING, _("Maximum number of concurrent DNS queries reached (max: %d)"), daemon->ftabsize);
	}

      return NULL;
    }
  
  if (!(f = allocate_frec(now)) && wait)
    /* wait one second on malloc failure */
    *wait = 1;

  return f; /* OK if malloc fails and this is NULL */
}

/* crc is all-ones if not known. */
static struct frec *lookup_frec(unsigned short id, void *hash)
{
  struct frec *f;

  for(f = daemon->frec_list; f; f = f->next)
    if (f->sentto && f->new_id == id && 
	(!hash || memcmp(hash, f->hash, HASH_SIZE) == 0))
      return f;
      
  return NULL;
}

static struct frec *lookup_frec_by_sender(unsigned short id,
					  union mysockaddr *addr,
					  void *hash)
{
  struct frec *f;
  
  for(f = daemon->frec_list; f; f = f->next)
    if (f->sentto &&
	f->orig_id == id && 
	memcmp(hash, f->hash, HASH_SIZE) == 0 &&
	sockaddr_isequal(&f->source, addr))
      return f;
   
  return NULL;
}
 
/* Send query packet again, if we can. */
void resend_query()
{
  if (daemon->srv_save)
    {
      int fd;
      
      if (daemon->srv_save->sfd)
	fd = daemon->srv_save->sfd->fd;
      else if (daemon->rfd_save && daemon->rfd_save->refcount != 0)
	fd = daemon->rfd_save->fd;
      else
	return;
      
      while(retry_send(sendto(fd, daemon->packet, daemon->packet_len, 0,
			      &daemon->srv_save->addr.sa, 
			      sa_len(&daemon->srv_save->addr)))); 
    }
}

/* A server record is going away, remove references to it */
void server_gone(struct server *server)
{
  struct frec *f;
  
  for (f = daemon->frec_list; f; f = f->next)
    if (f->sentto && f->sentto == server)
      free_frec(f);
  
  if (daemon->last_server == server)
    daemon->last_server = NULL;

  if (daemon->srv_save == server)
    daemon->srv_save = NULL;
}

/* return unique random ids. */
static unsigned short get_id(void)
{
  unsigned short ret = 0;
  
  do
#ifdef __SC_BUILD__
    {
      unsigned char ran[2] = {0};
      get_random_bytes(ran,sizeof(ran));
      ret = (ran[0] << 8) + ran[1];
    }
    while (lookup_frec(ret, NULL) || check_standard_deviation(ret,daemon->id));
#else

    ret = rand16();
  while (lookup_frec(ret, NULL));
#endif 
  return ret;
}

#ifdef __SC_BUILD__
#define DTYPE_SEP_STR       ("ww2w")
#define DNS_QUERY_OFFSET    (12)

/* return -1 means no one matches */
int sc_server_search(int type, char *domain, int dtype, int origi_pri)
{
	struct server *serv;
    int priority = -1;
	for (serv = daemon->servers; serv; serv=serv->next) {
        if (type == (serv->flags & SERV_TYPE) &&
                (type != SERV_HAS_DOMAIN || hostname_isequal(domain, serv->domain)) &&
                !(serv->flags & (SERV_LITERAL_ADDRESS | SERV_LOOP)) &&
                (serv->srv_pri > origi_pri) && 
                (dtype == -1 || dtype == serv->domain_type)) {
            if ((priority == -1) || (serv->srv_pri < priority)) {
                priority = serv->srv_pri;
            }
        }
    }
    my_syslog(LOG_DEBUG, "not find type [%d], domain [%s], dtype [%d], origi_pri [%d], priority [%d]\n", type, domain, dtype, origi_pri, priority);
    return priority;
}

/*
 * Description:
 *      Find the special char '#' from domain name, and parse 
 *      string after '#' as domain type, and need delete '#xxxx' 
 *      from dns packet
 * Parameter:
 *      dns_pkt: dns packet received from socket
 *      len:     dns packet length
 * Retrun:
 *      domain type
 * */
int sc_dtype_parse(char *dns_pkt, int *len)
{
    char *p = NULL, *q = NULL, *r = NULL;
    int dtype = 0;
    int len_after_r = 0;
    
    if((p = strstr(&dns_pkt[DNS_QUERY_OFFSET], DTYPE_SEP_STR)) == NULL)
    {
        return -1; 
    }
    
    dtype = *(p + strlen(DTYPE_SEP_STR)) - '0';
    
    switch(dtype){
        case DOMAIN_TYPE_UNKNOWN:
        case DOMAIN_TYPE_DATA:
        case DOMAIN_TYPE_DATA_MAN:
        case DOMAIN_TYPE_TR069:
        case DOMAIN_TYPE_NTP:
        case DOMAIN_TYPE_IPTV:
        case DOMAIN_TYPE_VOIP:
        case DOMAIN_TYPE_CLI:
        case DOMAIN_TYPE_IPPHONE:
            break;
        default:
            my_syslog(LOG_DEBUG, "Dtype mis-matched.\n");
        return -1;
    }

    q = p - 1;
    r = p + strlen(p);
    len_after_r = *len - (r - dns_pkt);
    memcpy(q, r, len_after_r);
    *len = (q - dns_pkt) + len_after_r;
    
    return dtype;
}

void sc_dns_answer_parse(int dtype, char *msg, int *len)
{
    int i = 0;
    char buffer[MAX_MSG_LEN];
    int query_offset = DNS_QUERY_OFFSET;
    int answers_offset = 0;
    int query_name_end_offset = query_offset + strlen(&msg[query_offset]);
    int buff_len = *len - query_name_end_offset;
    unsigned short temp_len = query_name_end_offset;
    struct dns_header *dheader = (struct dns_header *)msg;
    int dns_ancount = ntohs(dheader->ancount);
    int dns_nscount = ntohs(dheader->nscount);
    int dns_arcount = ntohs(dheader->arcount);
    unsigned short data_len = 0;
    unsigned short name_offset = 0;
    int insert_len = strlen(DTYPE_SEP_STR) + 1;

    if(dtype == -1)
        return;
    
   if(temp_len > (query_offset + MAX_DOMAIN_LEN))
      return; 

    memcpy(buffer, msg + temp_len, buff_len);
    msg[temp_len++] = insert_len;
    memcpy(&msg[temp_len], DTYPE_SEP_STR, strlen(DTYPE_SEP_STR)); 
    temp_len += strlen(DTYPE_SEP_STR);
    msg[temp_len++] = dtype + 0x30;
    memcpy(&msg[temp_len], buffer, buff_len);
    insert_len++;
    *len += insert_len;

    answers_offset = temp_len + 5; //5 means \0 + TYPE(2)+ CLASS(2) 
    for(i = 0; i < dns_ancount; i++)
    {
        if((unsigned char)msg[answers_offset] == 0xC0)//Compressed
        {
            msg[answers_offset] &= 0x0F;
            name_offset = ntohs(*(unsigned short *)&msg[answers_offset]); 
            if(name_offset > temp_len)
            {
                name_offset += insert_len;
                name_offset = htons(name_offset);
                memcpy(&msg[answers_offset], &name_offset, 2);
            }
            msg[answers_offset] |= 0xC0;
            answers_offset += 2;

            answers_offset += 2; //Type
            answers_offset += 2; //CLASS
            answers_offset += 4; //TTL
            data_len = ntohs(*(unsigned short *)&msg[answers_offset]);
            answers_offset += 2; //Data Length
            answers_offset += data_len;
        }
    }


    for(i = 0; i < dns_nscount; i++)
    {
        if((unsigned char)msg[answers_offset] == 0xC0)//Compressed
        {
            msg[answers_offset] &= 0x0F;
            name_offset = ntohs(*(unsigned short *)&msg[answers_offset]); 
            if(name_offset > temp_len)
            {
                name_offset += insert_len;
                name_offset = htons(name_offset);
                memcpy(&msg[answers_offset], &name_offset, 2);
            }
            msg[answers_offset] |= 0xC0;
            answers_offset += 2;

            answers_offset += 2; //Type
            answers_offset += 2; //CLASS
            answers_offset += 4; //TTL
            data_len = ntohs(*(unsigned short *)&msg[answers_offset]);
            answers_offset += 2; //Data Length
            answers_offset += data_len;
        }
    }
    for(i = 0; i < dns_arcount; i++)
    {
        if((unsigned char)msg[answers_offset] == 0xC0)//Compressed
        {
            msg[answers_offset] &= 0x0F;
            name_offset = ntohs(*(unsigned short *)&msg[answers_offset]); 
            if(name_offset > temp_len)
            {
                name_offset += insert_len;
                name_offset = htons(name_offset);
                memcpy(&msg[answers_offset], &name_offset, 2);
            }
            msg[answers_offset] |= 0xC0;
            answers_offset += 2;

            answers_offset += 2; //Type
            answers_offset += 2; //CLASS
            answers_offset += 4; //TTL
            data_len = ntohs(*(unsigned short *)&msg[answers_offset]);
            answers_offset += 2; //Data Length
            answers_offset += data_len;
        }
    }
    return;
}

static double difftimeval(struct timeval now, struct timeval tv)
{
    double d;
    time_t s;
    suseconds_t u;

    s = now.tv_sec - tv.tv_sec;
    u = now.tv_usec - tv.tv_usec;

    if(u < 0)
        --s;
    
    d = s;
    d *= 1000000.0;
    d += u;

    return d;
}

/* A server record is going away, remove references to it */
void retry_dns_query(void)
{
    struct timeval now;
    struct frec *f;
    int priority;
    struct server *firstsentto;
    char buf[256] = "";

    gettimeofday(&now,NULL);

    for(f = daemon->frec_list; f; f = f->next)
    {
        if (f->try_count >= MAX_DNS_SERVER) {
            my_syslog(LOG_INFO, _("All dns server had try")); 
//            free_frec(f); /* cancel */

        }
        else if (difftimeval(now, f->tv) >= daemon->query_timeout || f->resp_err)
        {
            char *domain = NULL;
            int type = 0, plen = 0;
            unsigned int gotname = 0;
            struct server *start = NULL;
            union mysockaddr *udpaddr = NULL;
            struct all_addr *dst_addr = NULL;
            struct dns_header *header = (struct dns_header *)(f->header);
            int forwarded = 0;
            f->retry = 1;
  
            /* check for send errors here (no route to host) 
               if we fail to send to all nameservers, send back an error
               packet straight away (helps modem users when offline)  */
            if (f->v4counter > 0 && f->rfd4 ) {
                f->v4counter--;
            }
            if (f->v6counter > 0 && f->rfd6 ) {
                f->v6counter--;
            }
            if (f->sentto)
            {
                start = daemon->servers;
                firstsentto = start;
                type = f->sentto->flags & SERV_TYPE;
#if 0
                start = f->sentto;
                if( start->next && start->next != daemon->servers)
                    start = start->next;
                else
                {
                    my_syslog(LOG_INFO, _("free frec for no server ")); 
                    free_frec(f); /* cancel */
                    if(f->next == daemon->frec_list)
                        break;
                    else
                        continue;
                }
#endif
                domain = f->sentto->domain;
                udpaddr = &(f->source);
                dst_addr = &(f->dest);
                plen = f->plen;
                my_syslog(LOG_INFO, _("resend dns query when time out")); 

                gotname = extract_request(header, plen, daemon->namebuff, NULL);
                priority = sc_server_search(type, domain, f->dtype, f->curr_srv_pri);
                if (priority == -1) {
                    f->islast = 1;
                }
                while (1)
                {
                    /* only send to servers dealing with our domain.
                       domain may be NULL, in which case server->domain 
                       must be NULL also. */

                    if (type == (start->flags & SERV_TYPE) &&
                            (type != SERV_HAS_DOMAIN || hostname_isequal(domain, start->domain)) &&
                            !(start->flags & (SERV_LITERAL_ADDRESS | SERV_LOOP)) && (f->pri == start->srv_pri) && ((f->dtype == -1 || f->dtype == start->domain_type)))
                    {
                        int fd;

                        /* find server socket to use, may need to get random one. */
                        if (start->sfd)
                            fd = start->sfd->fd;
                        else
                        {
#ifdef HAVE_IPV6
                            if (start->addr.sa.sa_family == AF_INET6)
                            {
                                if (!f->rfd6 &&
                                        !(f->rfd6 = allocate_rfd(AF_INET6)))
                                    break;
                                daemon->rfd_save = f->rfd6;
                                fd = f->rfd6->fd;
                            }
                            else
#endif
                            {
                                if (!f->rfd4 &&
                                        !(f->rfd4 = allocate_rfd(AF_INET)))
                                    break;
                                daemon->rfd_save = f->rfd4;
                                fd = f->rfd4->fd;
                            }

#ifdef HAVE_CONNTRACK
                            /* Copy connection mark of incoming query to outgoing connection. */
                            if (option_bool(OPT_CONNTRACK))
                            {
                                unsigned int mark;
                                if (get_incoming_mark(udpaddr, dst_addr, 0, &mark))
                                    setsockopt(fd, SOL_SOCKET, SO_MARK, &mark, sizeof(unsigned int));
                            }
#endif
                        }
#ifdef HAVE_DNSSEC
                        if (option_bool(OPT_DNSSEC_VALID) && (forward->flags & FREC_ADDED_PHEADER))
                        {
                            /* Difficult one here. If our client didn't send EDNS0, we will have set the UDP
                             packet size to 512. But that won't provide space for the RRSIGS in many cases.
                             The RRSIGS will be stripped out before the answer goes back, so the packet should
                             shrink again. So, if we added a do-bit, bump the udp packet size to the value
                             known to be OK for this server. We check returned size after stripping and set
                             the truncated bit if it's still too big. */
                            unsigned char *pheader;
                            int is_sign;
                            if (find_pseudoheader(header, plen, NULL, &pheader, &is_sign, NULL) && !is_sign)
                                PUTSHORT(start->edns_pktsz, pheader);
                        }
#endif
                        if(udpaddr)
                        {
                            if((0 == strcmp("127.0.0.1", inet_ntoa(udpaddr->in.sin_addr)) || 0 == strcmp("0.0.0.0", inet_ntoa(udpaddr->in.sin_addr))) && (start->domain_type == 2) && (strcmp(inet_ntoa(start->addr.in.sin_addr), "0.0.0.0") != 0))
                            {
                                if (!(start = start->next))
                                    start = daemon->servers;
                                if (start == firstsentto)
                                    break;
                                continue;
                            }
                            else if(0 != strcmp("127.0.0.1", inet_ntoa(udpaddr->in.sin_addr)) && (0 != strcmp("0.0.0.0", inet_ntoa(udpaddr->in.sin_addr)))  && (start->domain_type == 0) && (strcmp(inet_ntoa(start->addr.in.sin_addr), "0.0.0.0") != 0))
                            {
                                if (!(start = start->next))
                                    start = daemon->servers;
                                if (start == firstsentto)
                                    break;
                                continue;
                            }
#ifdef HAVE_IPV6
                            else if(0 == strcmp("::1", inet_ntop(AF_INET6,&udpaddr->in6.sin6_addr,buf,sizeof(buf))) && (start->domain_type == 2) && (strcmp(inet_ntoa(start->addr.in.sin_addr), "0.0.0.0") == 0))
                            {
                                if (!(start = start->next))
                                    start = daemon->servers;
                                if (start == firstsentto)
                                    break;
                                continue;
                            }
                            else if(0 != strcmp("::1", inet_ntop(AF_INET6,&udpaddr->in6.sin6_addr,buf,sizeof(buf))) && (start->domain_type == 0) && (strcmp(inet_ntoa(start->addr.in.sin_addr), "0.0.0.0") == 0))
                            {
                                if (!(start = start->next))
                                    start = daemon->servers;
                                if (start == firstsentto)
                                    break;
                                continue;
                            }
#endif
                        }        

                        if (sendto(fd, (char *)(header), plen, 0,
                                    &start->addr.sa,
                                    sa_len(&start->addr)) != -1)
                        {
                            /* Keep info in case we want to re-send this packet */
                            daemon->srv_save = start;
                            daemon->packet_len = plen;

                            if (!gotname)
                                strcpy(daemon->namebuff, "query");
                            if (start->addr.sa.sa_family == AF_INET)
                                log_query(F_SERVER | F_IPV4 | F_FORWARD, daemon->namebuff, 
                                        (struct all_addr *)&start->addr.in.sin_addr, NULL); 
#ifdef HAVE_IPV6
                            else
                                log_query(F_SERVER | F_IPV6 | F_FORWARD, daemon->namebuff, 
                                        (struct all_addr *)&start->addr.in6.sin6_addr, NULL);
#endif 
                            start->queries++;
                            forwarded = 1;
                            f->sentto = start;
                            f->forwardall++;
                            gettimeofday(&f->tv,NULL);
                            f->try_count++;
                            f->curr_srv_pri = priority;
                            //break;
                            if (start->addr.sa.sa_family == AF_INET) {
                                f->v4counter++;
                            }
                            else {
                                f->v6counter++;
                            }
                        }
                        else
                            my_syslog(LOG_INFO, _("sent to fail")); 
                    }

                    if (!(start = start->next))
                        start = daemon->servers;

                    if (start == daemon->servers)
                        break;

                }
                if(f->resp_err == 1)
                    f->resp_err = 0;
            }
            if (!forwarded)
            {
                //my_syslog(LOG_INFO, _("free frec for forward fail")); 
                free_frec(f); /* cancel */
            }
            else
            {
                if(f->pri == 0)
                    f->pri = 1;
                else
                    f->pri = 0;
            }
        }
        if(f->next == daemon->frec_list)
            break;
    }
}

#endif




