
/*

    File: master.h
    
    Copyright (C) 1999 by Wolfgang Zekoll  <wzk@quietsche-entchen.de>

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

#ifndef _DNRD_MASTER_H_
#define _DNRD_MASTER_H_
#ifdef __SC_BUILD__
/*
 * This is the primaty data structure for our little DNS.  It
 * contains an object name and a variable data type.  For usual
 * name - IP mappings the object name is the FQDN (hostname
 * with domain) and the data part contains the IP number in
 * string and binary format (both in `network order').
 */
typedef struct _string {
    unsigned int code;
    char    *string;
}string_t;
    
    
    
typedef struct _nameip {
    string_t    arpa;
    unsigned long ipnum;
} nameip_t;
                            

typedef struct _dnsrec {
    int     type;
    string_t    object;
    union {
    nameip_t    nameip;
    string_t    dns;
    } u;
    int pri;
} dnsrec_t;
#endif

/* Interface to our master DNS */
#if (defined(__SC_BUILD__) && (defined(CONFIG_SUPPORT_WEB_PRIVOXY)))
int master_lookup(unsigned char *msg, int len,const struct sockaddr_in *fromaddrp);
#else
int master_lookup(unsigned char *msg, int len);
#endif
int master_dontknow(unsigned char *msg, int len, unsigned char *answer);
int master_reinit(void);
int master_init(void);
#ifdef __SC_BUILD__
dnsrec_t * add_nameip(char *name, const int maxlen, char *ipnum);
int reset_nameip(void);

#ifdef CONFIG_SUPPORT_REDIRECT
int send_redirect(unsigned char *msg, int len);
int enable_redirect(char *ipnum);
int disable_redirect(void);
#endif/* CONFIG_SUPPORT_REDIRECT */

#endif/* __SC_BUILD__ */

#endif /* _DNRD_MASTER_H_ */

