/*
 * common.h
 *
 * This file contains definitions useful in all sorts of places.
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

#ifndef _DNRD_COMMON_H_
#define _DNRD_COMMON_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <stdarg.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <semaphore.h>
#include "utility.h"
#include "domnode.h"

/* default chroot path. this might be redefined in compile time */ 
#ifndef DNRD_ROOT
#define DNRD_ROOT "/usr/local/etc/dnrd"
#endif 

#ifndef CONFIG_FILE
#define CONFIG_FILE "dnrd.conf"
#endif

/* Set the default timeout value for select in seconds */
#ifndef SELECT_TIMEOUT
#define SELECT_TIMEOUT 1
#endif

/* Set the default timeout value for forward DNS. If we get no
 * response from a DNS server within forward_timeout, deactivate the
 * server.  note that if select_timeout is greater than this, the
 * forward timeout *might* increase to select_timeout. This value
 * should be >= SELECT_TIMEOUT
 */
/* 12 seems to be a good value under heavy load... */
#ifndef FORWARD_TIMEOUT
#define FORWARD_TIMEOUT 12
#endif

/*
	current dnrd version
*/
#define PACKAGE_VERSION	"2.17.2"


/* not used yet */
#ifndef FORWARD_RETRIES
#define FORWARD_RETRIES 5
#endif

/* only check if any server are to be reactivated every
 * REACTIVATE_INTERVAL seconds
 */
#define REACTIVATE_INTERVAL  10

#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
#define SC_IPV4_ADDR_CONVERT(a)               \
    do{                                     \
    uint32_t tmp;                           \
    tmp = ((uint32_t *)(a))[0];               \
    ((uint32_t *)(a))[0] = ((uint32_t *)(a))[3];\
    ((uint32_t *)(a))[3] = tmp;               \
    tmp = ((uint32_t *)(a))[1];               \
    ((uint32_t *)(a))[1] = ((uint32_t *)(a))[2];\
    ((uint32_t *)(a))[2] = tmp;               \
    }while(0)

#define SC_IPV4_ADDR_TO_IPV6(a)      \
    ((uint32_t *) (a))[2] = htonl(0xffff)/* you should make suer the a is writeable */
#endif

struct dnssrv_t {
  int                    sock;      /* for communication with server */
  struct sockaddr_in     addr;      /* IP address of server */
  char*                  domain;    /* optional domain to match.  Set to
					 zero for a default server */
  
};


extern const char*         version;   /* the version number for this program */
extern const char*         progname;  /* the name of this program */
extern int                 opt_debug; /* debugging option */
extern const char*         pid_file; /* File containing current daemon's PID */

extern int                 isock;     /* for communication with clients */
extern int                 tcpsock;   /* same as isock, but for tcp requests */
extern int                 select_timeout; /* select timeout in seconds */
extern int                 forward_timeout; /* timeout for forward DNS */
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_IPV6) && (!defined(CONFIG_USE_DNSMASQ_IPV6_ONLY)))
extern struct sockaddr_in6  recv_addr; /* address on which we receive queries */
#else
extern struct sockaddr_in  recv_addr; /* address on which we receive queries */
#endif
extern uid_t               daemonuid; /* to switch to once daemonised */
extern gid_t               daemongid; /* to switch to once daemonised */
extern int                 gotterminal;
extern char		   master_param[200];
extern sem_t               dnrd_sem;  /* Used for all thread synchronization */

extern char                dnrd_root[512];
extern char                config_file[];
extern domnode_t           *domain_list;

extern int                 reactivate_interval;
extern int                 ignore_inactive_cache_hits; 
extern int			load_balance;

extern int max_sockets;
extern int maxsock;
extern fd_set fdmaster;

#if (defined(__SC_BUILD__) && ((defined(CONFIG_SUPPORT_WEB_PRIVOXY)) || (defined(CONFIG_SUPPORT_FON))))
extern char provisioncode[128];
extern char mainsubnet[32];
extern char mainmask[32];
#ifdef CONFIG_SUPPORT_FON
extern char fonsubnet[32];
extern char fonmask[32];
extern char ofonsubnet[32];
extern char ofonmask[32];
#endif
#endif


#ifdef SCM_BINDING
extern int csock;
extern struct sockaddr_un dnrd_local;
extern const char *dnrd_ls_path;
#endif

/* kill any currently running copies of dnrd */
int kill_current();

/* print messages to stderr or syslog */
void log_msg(int type, const char *fmt, ...);
/* same, but only if debugging is turned on */
void log_debug(int level, const char *fmt, ...);

#ifdef USERAPP_NOMMU
/* cleanup eveything */
void cleanall();

/* remove pid/servlist file */
void rmFile();
#endif

/* cleanup everything and exit */
void cleanexit(int status);

/* Reads in the domain name as a string, allocates space for the CNAME
   version of it */
char* make_cname(const char *text, const int maxlen);
void sprintf_cname(const char *cname, int namesize, char *buf, int bufsize);

char *cname2asc(const char *cname);


/* Dumping DNS packets */
int dump_dnspacket(char *type, unsigned char *packet, int len);

#endif  /* _DNRD_COMMON_H_ */
