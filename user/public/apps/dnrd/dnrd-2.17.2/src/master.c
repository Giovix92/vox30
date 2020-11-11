
/*

    File: master.c - Small master DNS for local hosts
    
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef EXCLUDE_MASTER

#include <stdlib.h>
#include <stdio.h>
#define _GNU_SOURCE
#include <string.h>
#include <ctype.h>

#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "dns.h"
#include "lib.h"
#ifdef __SC_BUILD__
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
#include "arp.h"
#include "common/list.h"
#endif
#ifdef CONFIG_SUPPORT_WEB_PRIVOXY 
#ifdef CONFIG_SUPPORT_FON
#include "common/list.h"
#endif
#endif
#endif

#ifndef EXCLUDE_MASTER
#include "master.h"
#define	MASTER_CONFIG		DNRD_ROOT "/master"
#endif

#define	PACKET_ASSEMBLYSIZE	600


#define	ARPADOMAIN		".in-addr.arpa"

#define	DNS_TYPE_A		1
#define	DNS_TYPE_AAAA	0x1c
#define	DNS_TYPE_SRV	0x21
#define	DNS_TYPE_NS		2
#define	DNS_TYPE_PTR		12
#define	DNS_TYPE_MX		15


#define	DEFAULT_TTL		(60 * 60)

#ifndef __SC_BUILD__
typedef struct _string {
    unsigned int code;
    char    *string;
}string_t;
     
     
     
typedef struct _nameip {
    string_t    arpa;
    unsigned long ipnum;
} nameip_t;

/*
 * This is the primaty data structure for our little DNS.  It
 * contains an object name and a variable data type.  For usual
 * name - IP mappings the object name is the FQDN (hostname
 * with domain) and the data part contains the IP number in
 * string and binary format (both in `network order').
 */
#endif

#define	DNS_NAMEIP		1
#define	DNS_DNS			2
#define	DNS_AUTHORITY	3
#define	DNS_SRV     	4
#define	DNS_NAMEIP_X    5
#define	DNS_NAMEIP_HOST    6

#ifdef __SC_BUILD__
#ifdef CONFIG_SUPPORT_FON
static LIST_HEAD(head_whitelist);
struct domain_list
{
    struct list_head list;
    char *domain;
};
#endif
#ifdef CONFIG_SUPPORT_WEB_PRIVOXY 
#ifdef CONFIG_SUPPORT_FON
static LIST_HEAD(head_list);
struct ip_list
{
    struct list_head list;
    char *ip;
};
#endif
#endif
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
static LIST_HEAD(head_filterlist);
struct filter_domain_list
{
    struct list_head list;
    char *domain;
    char *mac;
};
#endif
#endif

#ifndef __SC_BUILD__
typedef struct _dnsrec {
    int     type;
    string_t    object;
    union {
    nameip_t    nameip;
    string_t    dns;
    } u;
} dnsrec_t;
#endif
char master_param[200]		= "";

static int master_onoff		= 1;
static int master_initialised	= 0;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_REDIRECT))
#define REDIRECT_FAKE_NAME "www.redirect_fake_name.com"
static int master_redirect_onoff = 0;
#endif
#ifdef __SC_BUILD__
static char option15[256] = "";
#endif
static char config[200]		= MASTER_CONFIG;

static int auto_authority	= 1;
static int master_reload	= 0;

	/*
	 * The DNS database is stored in an array of dbmax length.  The
	 * elements 0..dbc are already in use.
	 */

static int dbmax =		0;
static int dbc =		0;
static dnsrec_t	**dbv =		NULL;
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_REDIRECT))
static dnsrec_t	*redirect_dbv = NULL;
#endif
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_FON))
static dnsrec_t	*fon_redirect_dbv = NULL;
#endif

	/*
	 * mkstring() simply fills a string_t structure.
	 */

static char *mkstring(string_t *string, const char *name, const int maxlen)
{
    string->string = (char *)strdup(name);
    strnlwr(string->string, maxlen);
    string->code   = get_stringcode(string->string);

    return (string->string);
}


/*
 * These are handler for the data part of the dnsrec_t
 * structure.  They return their DNS-type identifier on
 * success and 0 otherwise.
 */
static int create_nameip(nameip_t *nameip, const int maxnamelen, char *ip)
{
    struct in_addr ipnum;

    if (inet_aton(ip, &ipnum) == 0) {
	log_msg(LOG_NOTICE, "invalid ip number: %s", ip);
	return (0);
    }

    nameip->ipnum = ntohl(ipnum.s_addr);
    // Swap the bytes.  Don't assume 32-bit integers.
    ipnum.s_addr = ((nameip->ipnum & 0xff000000 >> 24) |
		    (nameip->ipnum & 0x00ff0000 >> 8) |
		    (nameip->ipnum & 0x0000ff00 << 8) |
		    (nameip->ipnum & 0x000000ff << 24));

    mkstring(&nameip->arpa, inet_ntoa(ipnum), maxnamelen);
    
    return (DNS_NAMEIP);
}


/*
 * Because the dnsrec_t type contains different pointers to
 * allocated memory we have to be careful when freeing it.
 */
static int free_dnsrec(dnsrec_t *rec)
{
    switch (rec->type) {
      case DNS_NAMEIP:
	  free(rec->u.nameip.arpa.string);
	  break;
      case DNS_NAMEIP_X:
	  free(rec->u.nameip.arpa.string);
	  break;

      case DNS_DNS:
	  free(rec->u.dns.string);
	  break;
#if __SC_BUILD__
      case DNS_SRV:
	  free(rec->u.dns.string);
	  break;
      case DNS_NAMEIP_HOST:
	  free(rec->u.nameip.arpa.string);
	  break;
#endif
      case DNS_AUTHORITY:	/* No allocated memory. */
	  break;
	    
      case 0: /* This is allowed. Assume that the data part is not used. */
	  break;

      default:
	  log_msg(LOG_ALERT, "unknown DNS record type: %d "
		 "-- expect memory leaks", rec->type);
	  break;
    }

    free(rec->object.string);
    free(rec);

    return (0);
}

static dnsrec_t *alloc_dnsrec(char *name, const int maxlen)
{
    dnsrec_t *rec;

    rec = allocate(sizeof(dnsrec_t));
    mkstring(&rec->object, name, maxlen);

    return (rec);
}

static int reset_master(void)
{
    int	i;

    log_msg(LOG_NOTICE, "resetting master DNS");

#ifndef EMBED
    sem_wait(&dnrd_sem);
#endif

    for (i = 0; i < dbc; i++) {
	free_dnsrec(dbv[i]);
    }

    if (dbv)
    {
	free(dbv);
	dbc = 0;
    }

    dbc = 0;    
    dbmax = 0;  

#ifndef EMBED
    sem_post(&dnrd_sem);
#endif

    return (0);
}


	/*
	 * Functions to manipulate the DNS database.
	 */

static dnsrec_t *add_record(dnsrec_t *rec)
{
    if (dbc >= dbmax) {
	dbv = reallocate(dbv, (dbmax += 10) * sizeof(dnsrec_t));
    }

    dbv[dbc++] = rec;
    return (rec);
}
#ifndef __SC_BUILD__
static dnsrec_t *add_nameip(char *name, const int maxlen, char *ipnum)
#else
dnsrec_t *add_nameip(char *name, const int maxlen, char *ipnum)
#endif
{
    dnsrec_t *rec;
    
    rec = alloc_dnsrec(name, maxlen);
    if ((rec->type = create_nameip(&rec->u.nameip, maxlen, ipnum)) != 0) {
	add_record(rec);
	return (rec);
    }

    free_dnsrec(rec);
    return (NULL);
}

dnsrec_t *add_nameip_x(char *name, const int maxlen, char *ipnum)
{
    dnsrec_t *rec;
    
    rec = alloc_dnsrec(name, maxlen);
    if ((rec->type = create_nameip(&rec->u.nameip, maxlen, ipnum)) != 0) {
        rec->type = DNS_NAMEIP_X;
        add_record(rec);
        return (rec);
    }

    free_dnsrec(rec);
    return (NULL);
}
static dnsrec_t *add_dns(char *domain, const int maxdomainlen, 
			 char *dns, const int maxdnslen)
{
    dnsrec_t *rec;
    
    rec = alloc_dnsrec(domain, maxdomainlen);
    mkstring(&rec->u.dns, dns, maxdnslen);
    rec->type = DNS_DNS;
    add_record(rec);

    return (rec);
}
#ifdef __SC_BUILD__
#ifdef CONFIG_SUPPORT_FON
static int white_list_lookup(char *domain)
{
    struct domain_list *tmp = NULL;
    struct domain_list *domain_node = NULL;
    char *p = NULL;

    list_for_each_entry_safe(domain_node, tmp,&head_whitelist, list)
    {
         if(domain_node != NULL)
         {
             p = strchr(domain_node->domain, '*');
             if(p)
             {
                 if(strstr(domain, p+1))
                     return 0;
             }
             else if(strstr(domain, domain_node->domain))
            {
                return 0;
            }
         }
    }
    return -1;
}
int add_white_list(char *domain)
{
    struct domain_list *domain_node = NULL;
    
    if(white_list_lookup(domain) == 0)
        return 0;
    domain_node =(struct domain_list *)malloc(sizeof(struct domain_list));
    if (NULL == domain_node)
    {
        return -1;
    }
    memset(domain_node,0,sizeof(struct domain_list));
    domain_node->domain = (char *)strdup(domain);
    list_add(&domain_node->list, &head_whitelist);

    return 0;
}
int reset_white_list(void)
{
    struct domain_list *tmp = NULL;
    struct domain_list *domain_node = NULL;

    list_for_each_entry_safe(domain_node, tmp,&head_whitelist, list)
    {
         if(domain_node != NULL)
         {
            list_del(&domain_node->list);
            free(domain_node->domain);
            domain_node->domain = NULL;
            free(domain_node);
            domain_node = NULL;
         }
    }
    return 0;
}
#endif
#ifdef CONFIG_SUPPORT_WEB_URL_FILTER
#define _PATH_PROCNET_ARP "/proc/net/arp"

static int sal_arp_get_arp_entry(sal_arp ** arp_info)
{
	FILE *fp = NULL;
	static sal_arp arp[SAL_ARP_MAX_ENTRY];	
	char line[512] = "";
	int tmp_num = 0;
	int cnt = 0;

	fp = fopen(_PATH_PROCNET_ARP, "r");
	if (!fp)
		return 0;
	
	if (fgets(line, sizeof(line), fp) == NULL)
		goto end;
	for (; fgets(line, sizeof(line), fp) && (cnt < SAL_ARP_MAX_ENTRY);)
	{
		tmp_num = sscanf(line, "%s 0x%x 0x%x %100s %100s %100s\n",
			arp[cnt].ip, &(arp[cnt].type),&(arp[cnt].flags),
			 arp[cnt].hwa, arp[cnt].mask, arp[cnt].dev);
		if (tmp_num < 4)
			goto end;
		cnt ++;	
	}
	
end:
	fclose(fp);
	*arp_info = arp;
	return cnt;		
}

static sal_arp arp_information[SAL_ARP_MAX_ENTRY];
static int filter_domain_lookup(char *ipaddr,char *domain)
{
    struct filter_domain_list *tmp = NULL;
    struct filter_domain_list *domain_node = NULL;
    char *ip = NULL;
    sal_arp *arp_info = NULL;
    int num = 0, i = 0, count = 0;

    num = sal_arp_get_arp_entry(&arp_info);
    for (i=0; i<num; i++)
    {
        memcpy(&(arp_information[count]), &(arp_info[i]), sizeof(sal_arp));
        count++;
    }

    list_for_each_entry_safe(domain_node, tmp,&head_filterlist, list)
    {
         if(domain_node != NULL)
         {
            if(strncmp(domain_node->domain,domain,strlen(domain_node->domain)) == 0)
            {
                for(i = 0; i < count; i++)
                {
                    if(strncasecmp(domain_node->mac,arp_information[i].hwa,strlen(domain_node->mac)) == 0)
                    {
                        if(strncmp(arp_information[i].ip,ipaddr,strlen(arp_information[i].ip)) == 0)
                        {
                            return 0;
                        }
                    }
                }
            }
         }
    }
    return -1;
}
int add_domain_mac_list(char *domain, char *mac)
{
    struct filter_domain_list *domain_node = NULL;
    
    domain_node =(struct filter_domain_list *)malloc(sizeof(struct filter_domain_list));
    if (NULL == domain_node)
    {
        return -1;
    }
    memset(domain_node,0,sizeof(struct filter_domain_list));
    domain_node->domain = (char *)strdup(domain);
    domain_node->mac = (char *)strdup(mac);
    list_add(&domain_node->list, &head_filterlist);

    return 0;
}

int del_domain_mac_list(void)
{
    struct filter_domain_list *tmp = NULL;
    struct filter_domain_list *domain_node = NULL;

    list_for_each_entry_safe(domain_node,tmp,&head_filterlist, list)
    {
         if(domain_node != NULL)
         {
             list_del(&domain_node->list);
             free(domain_node->domain);
             domain_node->domain = NULL;
             free(domain_node->mac);
             domain_node->mac = NULL;
             free(domain_node);
             domain_node  = NULL;
         }
    }
    return 0;
}
#endif
#ifdef CONFIG_SUPPORT_FON
int add_filter_ip(char *ipnum)
{
    struct ip_list *ip_node = NULL;
    
    del_filter_ip(ipnum);
    ip_node =(struct ip_list *)malloc(sizeof(struct ip_list));
    if (NULL == ip_node)
    {
        return -1;
    }
    memset(ip_node,0,sizeof(struct ip_list));
    ip_node->ip = (char *)strdup(ipnum);
    list_add(&ip_node->list, &head_list);

    return 0;
}

int del_filter_ip(char *ipnum)
{
    struct ip_list *tmp = NULL;
    struct ip_list *ip_node = NULL;

    list_for_each_entry_safe(ip_node,tmp,&head_list, list)
    {
         if(ip_node != NULL)
         {
            if(strncmp(ip_node->ip,ipnum,strlen(ip_node->ip)) == 0)
            {
                list_del(&ip_node->list);
                free(ip_node->ip);
                ip_node->ip = NULL;
                free(ip_node);
                ip_node     = NULL;
                break;
            }
         }
    }
    return 0;
}

int reset_filter_ip(void)
{
    struct ip_list *tmp = NULL;
    struct ip_list *ip_node = NULL;

    list_for_each_entry_safe(ip_node, tmp,&head_list, list)
    {
         if(ip_node != NULL)
         {
            list_del(&ip_node->list);
            free(ip_node->ip);
            ip_node->ip = NULL;
            free(ip_node);
            ip_node     = NULL;
         }
    }
    return 0;
}

static int filter_ip_lookup(char *ipaddr)
{
    struct ip_list *tmp = NULL;
    struct ip_list *ip_node = NULL;

    list_for_each_entry_safe(ip_node, tmp,&head_list, list)
    {
         if(ip_node != NULL)
         {
            if(strncmp(ip_node->ip,ipaddr,strlen(ip_node->ip)) == 0)
            {
                return 0;
            }
         }
    }
    return -1;
}
#endif

dnsrec_t *add_nameip_host(char *name, const int maxlen, char *ipnum)
{
    dnsrec_t *rec;
    int i;
    for (i = 0; i < dbc; i++) 
    {
        if(dbv[i]->type == DNS_NAMEIP_HOST)
        {
            if(strcasecmp(dbv[i]->object.string, name) == 0)
            {
                free_dnsrec(dbv[i]);
                if((dbc-1) > i)
                {
                    dbv[i] = dbv[dbc-1];
                    i --;
                }
                dbc --;
            }
        }
    }
    rec = alloc_dnsrec(name, maxlen);
    if ((rec->type = create_nameip(&rec->u.nameip, maxlen, ipnum)) != 0) {
        rec->type = DNS_NAMEIP_HOST;
        add_record(rec);
        return (rec);
    }

    free_dnsrec(rec);
    return (NULL);
}
dnsrec_t *add_namesrv(char *domain, const int maxdomainlen, 
			 char *dns, const int maxdnslen, int pri)
{
    dnsrec_t *rec;
    
    rec = alloc_dnsrec(domain, maxdomainlen);
    mkstring(&rec->u.dns, dns, maxdnslen);
    rec->type = DNS_SRV;
    rec->pri = pri;
    add_record(rec);

    return (rec);
}
int reset_nameip(void)
{
    int i;
    for (i = 0; i < dbc; i++) {
        if(dbv[i]->type == DNS_NAMEIP)
        {
            free_dnsrec(dbv[i]);
            if((dbc-1) > i)
            {
                dbv[i] = dbv[dbc-1];
                i --;
            }
            dbc --;
        }
    }
    return 0;

}
int reset_nameip_x(void)
{
    int i;
    for (i = 0; i < dbc; i++) {
        if(dbv[i]->type == DNS_NAMEIP_X)
        {
            free_dnsrec(dbv[i]);
            if((dbc-1) > i)
            {
                dbv[i] = dbv[dbc-1];
                i --;
            }
            dbc --;
        }
    }
    return 0;

}
int reset_namesrv(void)
{
    int i;
    for (i = 0; i < dbc; i++) {
        if(dbv[i]->type == DNS_SRV)
        {
            free_dnsrec(dbv[i]);
            if((dbc-1) > i)
            {
                dbv[i] = dbv[dbc-1];
                i --;
            }
            dbc --;
        }
    }
    return 0;

}
int option15_lookup(char *domain)
{
    if(strlen(option15) && (strlen(domain) > strlen(option15)) && (strcmp(domain+(strlen(domain)-strlen(option15)), option15) == 0))
        return 1;
    return 0;
}
int update_option15(char *op_str)
{
    snprintf(option15, sizeof(option15), "%s", op_str);
    return 1;
}
#endif
static dnsrec_t *add_authority(char *domain, const int maxlen)
{
    dnsrec_t *rec;
    
    rec = alloc_dnsrec(domain, maxlen);
    rec->type = DNS_AUTHORITY;
    add_record(rec);
    log_debug(1, "added authority for %s", domain);

    return (rec);
}


	/*
	 * Here we read the DNS configuration file.
	 */

char *get_hostname(char **from, char *domain, char *name, int size)
{
    int	 len;
    char word[100];

    *name = 0;
    size -= 2;
    if (*get_word(from, word, sizeof(word)) == 0) {
	return (name);
    }

    if (*word == '+') {	/* Not a hostname but an option. */
	copy_string(name, word, size);
    }
    else if ((len = strnlen(word, sizeof(word))) > 0  &&  word[len-1] == '+') {
	word[len-1] = 0;
	snprintf (name, size, "%s%s%s", word, domain[0] ? "." : "", domain);
    }
    /* patch from maillinglist
       http://groups.yahoo.com/group/dnrd/message/231 

       This is to let dnrd resolve short hostnames in master file so a
       dialup line won't open a connection.
    */
    else if ((len = strnlen(word, sizeof(word))) > 0 && word[len-1] == '.') {
      word[len-1] = 0;
      size--;
      copy_string(name, word, size);
    }

    else if (strchr(word, '.') == NULL) {
	snprintf (name, size, "%s%s%s", word, domain[0] ? "." : "", domain);
    }
    else {
	copy_string(name, word, size);
    }

    return (name);
}

int read_hosts(char *filename, char *domain)
{
    int		count;
    char	*p, word[100], ipnum[100], line[300];
    FILE	*fp;
    
    if ((fp = fopen(filename, "r")) == NULL) {
	log_msg(LOG_NOTICE, "can't open file: %s", filename);
	return (1);
    }

    count = dbc;
    while (fgets(line, sizeof(line), fp) != NULL) {
	p = skip_ws((char *)noctrln(line, sizeof(line)));
	if (*p == 0  ||  *p == '#') continue;

	if (isdigit((int)(*p))) {
	    /*
	     * Usual hosts records start with an IP number.  This
	     * might be followed by one or more names.  Every
	     * name makes one DNS record.
	     */
	    get_word(&p, ipnum, sizeof(ipnum));
	    while (*get_hostname(&p, domain, word, sizeof(word)) != 0) {
		add_nameip(word, sizeof(word), ipnum);
	    }
	}
    }
	    
    fclose (fp);
    log_debug(1, "%s: %d records", filename, dbc - count);

    return (0);
}
	
int read_configuration(char *filename)
{
    int	count;
    char *p, word[100], ipnum[100], line[300];
    char domain[40];
    FILE *fp;
    
    if ((fp = fopen(filename, "r")) == NULL) {
	log_debug(1, "no master configuration: %s", filename);
	return (1);
    }

    count = dbc;
    while (fgets(line, sizeof(line), fp) != NULL) {
	p = skip_ws(noctrln(line,sizeof(line) ));
	if (*p == 0  ||  *p == '#') continue;

	if (isdigit((int)(*p))) {
	    /*
	     * Usual hosts records start with an IP number.  This
	     * might be followed by one or more names.  Every
	     * name makes one DNS record.
	     */
	    get_word(&p, ipnum, sizeof(ipnum));
	    while (*get_hostname(&p, domain, word, sizeof(word)) != 0)
		    add_nameip(word, sizeof(word), ipnum);
	}
	else if (strcmp(get_word(&p, word, sizeof(word)), "domain") == 0) {
	    get_word(&p, domain, sizeof(domain));
	    if (*domain == 0) continue;

	    while (*get_hostname(&p, domain, word, sizeof(word)) != 0) {
		if (strcmp(word, "+auth") == 0) {
		    add_authority(domain, sizeof(domain));
		}
		else {
		    add_dns(domain, sizeof(domain),  word, sizeof(word));
		}
	    }
	}
	else if (strcmp(word, "hosts") == 0) {
	    char	filename[200];

	    if (*get_word(&p, filename, sizeof(filename)) == 0) continue;
	    
	    if (strcmp(filename, "-") == 0) {
		strcpy(filename, "/etc/hosts");
	    }
	    
	    if (*get_word(&p, word, sizeof(word)) == 0) {
		copy_string(word, domain, sizeof(word));
	    }

	    read_hosts(filename, word);
	}
	else {
	    log_msg(LOG_NOTICE, "%s: unknown directive: %s", filename, word);
	}
    }
	    
    fclose (fp);
    log_debug(1, "%s: %d records", filename, dbc - count);

    return (0);
}


/*
 * DNS database lookups.
 */
static dnsrec_t *ptr_lookup(char *arpanum)
{
    unsigned int code;
    int	i;
    dnsrec_t *rec;

    code = get_stringcode(arpanum);
    for (i = 0; i < dbc; i++) {
	rec = dbv[i];
	if ((rec->type == DNS_NAMEIP) &&
	    (rec->u.nameip.arpa.code == code) &&
	    (strcmp(rec->u.nameip.arpa.string, arpanum) == 0)) {
	    return (rec);
	}
    }
	    
    return (NULL);
}

static dnsrec_t *name_lookup(char *name)
{
    unsigned int code;
    int	i;
    dnsrec_t *rec;

    code = get_stringcode(name);
    for (i = 0; i < dbc; i++) {
        rec = dbv[i];
        if ((rec->type == DNS_NAMEIP || rec->type == DNS_NAMEIP_X || rec->type == DNS_NAMEIP_HOST) &&
                (rec->object.code == code) &&
                (strcasecmp(rec->object.string, name) == 0)) {
            return (rec);
        }
    }

    return (NULL);
}
static dnsrec_t *srv_lookup(char *name)
{
    unsigned int code;
    int	i;
    dnsrec_t *rec;

    code = get_stringcode(name);
    for (i = 0; i < dbc; i++) {
	rec = dbv[i];
	if ((rec->type == DNS_SRV) &&
	    (rec->object.code == code) &&
	    (strcmp(rec->object.string, name) == 0)) {
	    return (rec);
	}
    }
		
    return (NULL);
}

static dnsrec_t *srv_backup_lookup(char *name)
{
    unsigned int code;
    int	i;
    dnsrec_t *rec;
    int count = 0;
    code = get_stringcode(name);
    for (i = 0; i < dbc; i++) 
    {
        rec = dbv[i];
        if ((rec->type == DNS_SRV) &&
                (rec->object.code == code) &&
                (strcmp(rec->object.string, name) == 0)) 
        {
            if(count == 1)
                return (rec);
            else
            {
                count++;
            }
        }
    }
		
    return (NULL);
}
static dnsrec_t *dns_lookup(char *name, int *last)
{
    int	i, k, len;
    dnsrec_t *rec;

    len = strlen(name);
    *last = (*last < 0) ? 0 : *last + 1;

    for (i = *last; i < dbc; i++) {
	rec = dbv[i];
	if (rec->type == DNS_DNS  &&
	    (k = len - strlen(rec->object.string)) >= 0) {
	    if (k > 0  &&  name[k-1] == '.'  &&
		strcmp(&name[k], rec->object.string) == 0) {
		*last = i;
		return (rec);
	    }
	    else if (k == 0  &&  strcmp(name, rec->object.string) == 0) {
		*last = i;
		return (rec);
	    }
	}
    }
	    
    return (NULL);
}

static dnsrec_t *authority_lookup(char *domain)
{
    unsigned int code;
    int	i;
    dnsrec_t *rec;

    code = get_stringcode(domain);
    for (i = dbc-1; i >= 0; i--) {
	rec = dbv[i];
	if ((rec->type == DNS_AUTHORITY)  &&
	    (rec->object.code == code)  &&
	    (strcmp(rec->object.string, domain) == 0)) {
	    return (rec);
	}
    }
	    
    return (NULL);
}


/*
 * Answer assembly.
 */
static int compile_name(dnsheader_t *x, char *name)
{
    unsigned int c;
    int	i, k, n, offset;

    offset = x->here - x->packet;
    k = 0;
    while ((c = name[k]) != 0) {
	n = 0;
	while ((c = name[k+n]) != 0  &&  c != '.') {
	    n++;
	}

	if (n == 0) break;

	*x->here++ = (unsigned char) (n & 0x3F);
	for (i = 0; i < n; i++) {
	    *x->here++ = name[k++];
	}

	if (name[k] == '.') {
	    k++;
	}
    }

    *x->here++ = 0;
    return (offset);
}

static int compile_namepointer(dnsheader_t *x, int offset)
{
    *x->here++ = 0xC0;
    *x->here++ = (unsigned int) (offset & 0x3F);

    return (0);
}

static unsigned char *compile_int(dnsheader_t *x, int number)
{
    unsigned short conv = htons((unsigned short) number);
    memcpy(x->here, &conv, 2);
    x->here += 2;

    return ((unsigned char *)(x->here));
}

static unsigned char *compile_long(dnsheader_t *x, long number)
{
    unsigned long conv = htonl((unsigned long) number);
    memcpy(x->here, &conv, 4);

    x->here += 4;

    return ((unsigned char *)(x->here));
}

static unsigned char *compile_objectname(dnsheader_t *x)
{
    *x->here++ = 0xC0;
    *x->here++ = 0x0C;

    return ((unsigned char *)(x->here));
}

static int end_rdata(dnsheader_t *x)
{
    unsigned short int conv;

    if (x->rdata != NULL) {
	int	rsize;

	rsize = x->here - (x->rdata + 2);
	conv = htons((unsigned short int) rsize);
	memcpy(x->rdata, &conv, 2);
    }
    
    x->rdata = NULL;
    return (0);
}

static int start_rdata(dnsheader_t *x)
{
    x->rdata = x->here;
    compile_int(x, 0);

    return (0);
}

static int end_assembly(dnsheader_t *x)
{
    unsigned short int *pkt;

    end_rdata(x);
/*  SET_AA(x->u, 0); */

    pkt = (unsigned short int *) x->packet;
    pkt[1] = htons(x->u);
    pkt[2] = htons(x->qdcount);
    pkt[3] = htons(x->ancount);
    pkt[4] = htons(x->nscount);
    pkt[5] = htons(x->arcount);
    
    x->len = x->here - x->packet;
    return (x->len);
}

static dnsheader_t *begin_assembly(rr_t *query)
{
    static dnsheader_t *x = NULL;

    if (x == NULL) {
	x = allocate(sizeof(dnsheader_t));
	x->packet = allocate(PACKET_ASSEMBLYSIZE);
    }

    /*
     * Reset the packet ...
     */

    x->id = 0;
    x->u  = 0;
    SET_QR(x->u, 1);

    x->qdcount = 1;
    x->ancount = 0;
    x->nscount = 0;
    x->arcount = 0;
    x->rdata   = NULL;

    memset(x->packet, 0, PACKET_ASSEMBLYSIZE);
    x->here = &x->packet[PACKET_DATABEGIN];

    /*
     * ... and write the original query data.
     */

    compile_name(x, query->name);
    compile_int(x, query->type);
    compile_int(x, query->class);
    
    return (x);
}

static int compile_dnsrecords(dnsheader_t *x, char *object)
{
    int	pos, last;
    dnsrec_t *rec;

    last = -1;
    pos = 0;
    while ((rec = dns_lookup(object, &last)) != NULL) {
	if (pos == 0) {
	    pos = compile_name(x, rec->object.string);
	}
	else {
	    compile_namepointer(x, pos);
	}

	compile_int(x, DNS_TYPE_NS);
	compile_int(x, DNS_CLASS_INET);
	compile_long(x, DEFAULT_TTL);
	start_rdata(x);
	compile_name(x, rec->u.dns.string);
	end_rdata(x);

	x->nscount++;
    }

    if (x->nscount > 0) SET_AA(x->u, 1);

    return (0);
}


/*
 * master_init()
 *
 * Create the DNS database with the data from the configuration
 * file.
 */
static int _master_init(void)
{
    if (master_onoff == 0) return (0);

    log_debug(1, "initialising master DNS database");

    add_nameip("localhost", sizeof("localhost"), "127.0.0.1");
    add_dns("0.0.127.in-addr.arpa", sizeof("0.0.127.in-addr.arpa"),
	    "localhost", sizeof("localhost"));
#ifndef __SC_BUILD__    
 
    if ((strcmp(master_param, "hosts") == 0) ||
	(read_configuration(config) != 0)) {
	char	domain[80];
	FILE	*fp;

	/*
	 * Hmm, no dnrd.conf - let's try to initialize with the
	 * /etc/hosts only.
	 */

	*domain = 0;
	if ((fp = fopen("/etc/resolv.conf", "r")) != NULL) {
	    char	*p, word[80], line[300];

	    /*
	     * We try to get the default domain name from here.
	     */

	    while (fgets(line, sizeof(line), fp) != NULL) {
		p = skip_ws(noctrln(line,sizeof(line)));
		if (*p == 0  ||  *p == '#') continue;

		get_word(&p, word, sizeof(word));
		if (strcmp(word, "domain") == 0) {
		    get_word(&p, domain, sizeof(domain));
		    break;
		}
	    }

	    fclose (fp);

	    log_debug(1, "initialising from /etc/hosts, domain= %s",
		      *domain == 0? "<none>": domain);
	    read_hosts("/etc/hosts", domain);
	}
    }

    if (auto_authority != 0) {
	int	i;
	char	*domain, arpaname[200];
	
	
	for (i = 0; i < dbc; i++) {
	    if (dbv[i]->type == DNS_NAMEIP) {
		snprintf (arpaname, sizeof(arpaname) - 2, "%s%s",
			  dbv[i]->u.nameip.arpa.string, ARPADOMAIN);
		if ((domain = strchr(arpaname, '.')) == NULL) continue;
		domain++;
		if (authority_lookup(domain) == 0) {
		    add_authority(domain,sizeof(arpaname)-(arpaname-domain));
		}
	    }
	}
    }

    log_debug(1, "%d records in master DNS database", dbc);
#endif
    master_initialised = 1;

    return (0);
}

/*
 * master_lookup()
 *
 * Get the query from the packet(msg, len) and look if it can
 * be answered by the local master.  If so assemble the response
 * and copy if to msg, return the answer length in the
 * function's return code.  0 means here that the master hasn't
 * any data.
 *
 * The function assumes that the buffer area answer points to
 * is large enough.
 *
 * The assembly functions do no size checking, so basically an
 * overflow might occur.  Otherwise the answer packets are
 * relatively small.  They should always fit into 512 bytes.
 */

#if (defined(__SC_BUILD__) && ((defined(CONFIG_SUPPORT_WEB_PRIVOXY)) || (defined(CONFIG_SUPPORT_FON))||(defined(CONFIG_SUPPORT_WEB_URL_FILTER))))
int master_lookup(unsigned char *msg, int len,const struct sockaddr_in *fromaddrp)
#else
int master_lookup(unsigned char *msg, int len)
#endif
{
    char	*domain;
    rr_t	query;
    dnsrec_t *rec;

    if (master_onoff == 0) return (0);


    if (master_initialised == 0) {
	_master_init();
    }

    if ((parse_query(&query, msg, len) == NULL) ||
	(query.class != DNS_CLASS_INET  ||  GET_OPCODE(query.flags) != 0)) {
	return (0);
    }
#if (defined(__SC_BUILD__) && ((defined(CONFIG_SUPPORT_WEB_PRIVOXY)) || (defined(CONFIG_SUPPORT_FON))))
    struct in_addr msubnet;
#ifdef CONFIG_SUPPORT_FON
    struct in_addr fsubnet;
    struct in_addr ofsubnet;
    int replylen = 0;
#endif
    unsigned int privoxy_ip = 16843235; //1.1.1.227                                                 
    char from_ip[32] = {0};
    snprintf(from_ip,sizeof(from_ip),"%s",inet_ntoa(fromaddrp->sin_addr));
    
    msubnet.s_addr = ((inet_addr(from_ip)) & (inet_addr(mainmask)));
#ifdef CONFIG_SUPPORT_FON
    fsubnet.s_addr = ((inet_addr(from_ip)) & (inet_addr(fonmask)));
    ofsubnet.s_addr = ((inet_addr(from_ip)) & (inet_addr(ofonmask)));
#endif
    /*Lan user and Main WIFI user if provisioncode is null, skip Reverse Proxy*/
    if( ((strlen(provisioncode) <= 0) && (strcmp(mainsubnet, inet_ntoa(msubnet)) == 0))
        || (strcmp(from_ip,"127.0.0.1") == 0))
    {
        if ((rec = name_lookup(query.name)) != NULL) 
        {
            if( rec->u.nameip.ipnum == privoxy_ip )
            {
                return 0;
            }
        }
    }
#ifdef CONFIG_SUPPORT_FON
    //EAP FON skip Reverse Proxy
    else if(strcmp(fonsubnet, inet_ntoa(fsubnet)) == 0)
    {
        return 0;
    }
    //Authenticated open FON user skip Reverse Proxy
    else if (filter_ip_lookup(from_ip) == 0) 
    {
        return 0;
    }
    else if(strcmp(ofonsubnet, inet_ntoa(ofsubnet)) == 0)
    {
        //check whilte list
        if(white_list_lookup(query.name) == 0)
        {
            //check Reverse Proxy
            if ((rec = name_lookup(query.name)) != NULL) 
            {
                if( rec->u.nameip.ipnum == privoxy_ip )
                {
                    return 0;
                }
            }
            else
                return 0;
        }
        else
        {
            replylen = send_redirect_fon(msg, len);
            return replylen;
        }
    }

#endif
#endif
#if (defined(__SC_BUILD__) && (defined(CONFIG_SUPPORT_WEB_URL_FILTER)))
    char s_ip[32] = {0};
    snprintf(s_ip,sizeof(s_ip),"%s",inet_ntoa(fromaddrp->sin_addr));
    if(filter_domain_lookup(s_ip,query.name) == 0)
    {
        //filter it
        return -1;
    }
#endif
    
    if (query.type == DNS_TYPE_PTR) {
	int	k, len;

	len = strlen(query.name);
	k = len - strlen(ARPADOMAIN);
	if (k < 0  ||  strcmp(&query.name[k], ARPADOMAIN) != 0) {
	    return (0);
	}

	query.name[k] = 0;
	if ((rec = ptr_lookup(query.name)) != NULL) {
	    dnsheader_t *x;
  
	    query.name[k] = '.';
	    log_debug(2, "master: found PTR %s\n", query.name);

	    x = begin_assembly(&query);
	    compile_objectname(x);
	    compile_int(x, DNS_TYPE_PTR);
	    compile_int(x, DNS_CLASS_INET);
	    compile_long(x, DEFAULT_TTL);
	    start_rdata(x);
	    compile_name(x, rec->object.string);
	    end_rdata(x);

	    compile_dnsrecords(x, query.name);

	    x->ancount = 1;
	    end_assembly(x);

	    dump_dnspacket("assembled", x->packet, x->len);

	    memcpy(msg + 2, x->packet + 2, x->len - 2);
	    return (x->len);
	}

	/* Repair query for later authority lookup. */
	query.name[k] = '.';
    }
    else if (query.type == DNS_TYPE_A) {
	if ((rec = name_lookup(query.name)) != NULL) {
	    dnsheader_t *x;
	    x = begin_assembly(&query);
	    compile_objectname(x);
	    compile_int(x, DNS_TYPE_A);
	    compile_int(x, DNS_CLASS_INET);
        if(rec->type == DNS_NAMEIP_X) // live time 10s to IP Phone local DNS
            compile_long(x, 10);
        else
            compile_long(x, DEFAULT_TTL);
	    start_rdata(x);
	    compile_long(x, rec->u.nameip.ipnum);
	    end_rdata(x);
	    
	    compile_dnsrecords(x, query.name);

	    x->ancount = 1;
	    end_assembly(x);

	    dump_dnspacket("assembled", x->packet, x->len);

	    memcpy(msg + 2, x->packet + 2, x->len - 2);
	    return (x->len);
	}
    }
    else if(query.type == DNS_TYPE_SRV)
    {
        if((rec = srv_lookup(query.name)) != NULL)
        {
            dnsheader_t *x;
            x = begin_assembly(&query);
            compile_objectname(x);
            compile_int(x, DNS_TYPE_SRV);
            compile_int(x, DNS_CLASS_INET);
            compile_long(x, 10); // live time 10s
            start_rdata(x);
            compile_int(x, rec->pri); //pri
            compile_int(x, 1); //weight
            compile_int(x, 5060); //port
            compile_name(x, rec->u.dns.string); //target
            end_rdata(x);

            compile_dnsrecords(x, query.name);
            if((rec = srv_backup_lookup(query.name)) != NULL)
            {
                compile_objectname(x);
                compile_int(x, DNS_TYPE_SRV);
                compile_int(x, DNS_CLASS_INET);
                compile_long(x, 10);
                start_rdata(x);
                compile_int(x, rec->pri); //pri
                compile_int(x, 1); //weight
                compile_int(x, 5060); //port
                compile_name(x, rec->u.dns.string); //target
                end_rdata(x);
                compile_dnsrecords(x, query.name);
                x->ancount = 2;
            }
            else
            {
                x->ancount = 1;
            }
            end_assembly(x);

            dump_dnspacket("assembled", x->packet, x->len);

            memcpy(msg + 2, x->packet + 2, x->len - 2);
            return (x->len);
        }
    }
    else if (query.type == DNS_TYPE_NS) {
	int	last;
	last = -1;
	if ((rec = dns_lookup(query.name, &last)) != NULL) {
	    dnsheader_t *x;
	    
	    x = begin_assembly(&query);
	    while (rec != NULL) {
		compile_objectname(x);
		compile_int(x, DNS_TYPE_NS);
		compile_int(x, DNS_CLASS_INET);
		compile_long(x, DEFAULT_TTL);

		start_rdata(x);
		compile_name(x, rec->u.dns.string);
		end_rdata(x);
		
		x->ancount++;
		rec = dns_lookup(query.name, &last);
	    }

	    SET_AA(x->u, 1);
	    end_assembly(x);

	    dump_dnspacket("assembled", x->packet, x->len);

	    memcpy(msg + 2, x->packet + 2, x->len - 2);
	    return (x->len);
	}
    }
    else {
	/*
	 * Return here if this type of request isn't handled
	 * by the master.
	 */
	return (0);
    }

    /*
     * At this point we have a request that would have been
     * answered by us if we only had the data for it.  Look
     * if we are authoritative for it and assemble a negative
     * response if yes.
     */

    if ((domain = strchr(query.name, '.')) == NULL) {
	return (0);
    }

    domain++;
    if (authority_lookup(domain) != NULL) {
	dnsheader_t *x;

	log_debug(2, "master: found AUTHORITY for %s\n", domain);

	x = begin_assembly(&query);
	x->ancount = 0;
	SET_AA(x->u, 1);
	end_assembly(x);

	dump_dnspacket("assembled", x->packet, x->len);

	memcpy(msg + 2, x->packet + 2, x->len - 2);
	return (x->len);
    }
#ifdef __SC_BUILD__
    if(option15_lookup(query.name))
    {
        return -1;
    }
#endif

    return (0);
}


/*
 * master_dontknow()
 *
 * Take the given packet and assemble a `we dont know' answer
 * for the client.  This answer isn't authoritative.  It's
 * tells the client that we are actually unable to process his
 * request.
 */ 
int master_dontknow(unsigned char *msg, int len, unsigned char *answer)
{
    rr_t	query;
    dnsheader_t	*x;

    if (master_onoff == 0) return (0);


    if (master_initialised == 0) {
	_master_init();
    }

    if (parse_query(&query, msg, len) == NULL) {
	return (0);
    }
    else if (query.class != DNS_CLASS_INET  ||  GET_OPCODE(query.flags) != 0) {
	return (0);
    }

    x = begin_assembly(&query);
    x->ancount = 0;
    SET_AA(x->u, 0);
    SET_RCODE(x->u, 1);
    end_assembly(x);

    dump_dnspacket("assembled", (unsigned char *)x->packet, x->len);

    memcpy(answer, x->packet, x->len);
    return (x->len);
}


/*
 * master_sighup(), master_reinit()
 *
 * master_reinit() erases the whole master DNS database and makes a
 * complete reread of the database definition.  master_sighup()
 * is the correponding signal handler that sets the reload flag.
 * The actual reload is done from run() in relay.c after return
 * from the select() function.
 */
static void master_sighup(int sig)
{
    if (master_onoff != 0) {
        master_reload = 1;
    }

    signal(sig, master_sighup);
    return;
}

int master_reinit(void)
{
    if (master_onoff == 0) return (0);

    if (master_reload != 0) {
        reset_master();
        _master_init();
    }

    master_reload = 0;
    return (0);
}


/*
 * master_init()
 *
 * Should be called once from main() to initialise the master
 * database.
 */ 
int master_init(void)
{
    if (strcmp(master_param, "off") == 0) master_onoff = 0;

    if (master_onoff == 0) {
	log_msg(LOG_NOTICE, "local DNS master turned off");
    }
    else {
	_master_init();
	/* This does not work at all. I turn it off...
	signal(SIGHUP, master_sighup);
	*/
    }

    /*
     * Since our root dir will have changed to /etc/dnrd, we need to change
     * the path of our config file so that future re-reads will continue to
     * work.
     */
    strcpy(config, "master");

    return (0);
}
#if ((defined __SC_BUILD__) && (defined CONFIG_SUPPORT_FON))
int enable_redirect_fon(char *ipnum)
{
    dnsrec_t *rec;

    rec = alloc_dnsrec(REDIRECT_FAKE_NAME, strlen(REDIRECT_FAKE_NAME) + 1);
    if ((rec->type = create_nameip(&rec->u.nameip, strlen(REDIRECT_FAKE_NAME) + 1, ipnum)) != 0) 
    {
        fon_redirect_dbv = rec;
        return 0;
    }
    else
    {
        free_dnsrec(rec);
        return -1;
    }
}
int send_redirect_fon(unsigned char *msg, int len)
{
    rr_t	query;
    dnsrec_t *rec;

    if(NULL == fon_redirect_dbv)
    {
        return (0);
    }
    if ((parse_query(&query, msg, len) == NULL) ||
            (query.class != DNS_CLASS_INET  ||  
             GET_OPCODE(query.flags) != 0)) 
    {
        return (0);
    }
    if(strstr(query.name, "captive.apple.com"))
    {
        return (0);
    }
    if(strstr(query.name, "www.apple.com"))
    {
        return (0);
    }
    if(strstr(query.name, "detectportal.firefox.com"))
    {
        return (0);
    }

    if (query.type == DNS_TYPE_A || query.type == DNS_TYPE_AAAA) 
    {
        rec = fon_redirect_dbv;
        dnsheader_t *x;
        x = begin_assembly(&query);
        compile_objectname(x);
        compile_int(x, DNS_TYPE_A);
        compile_int(x, DNS_CLASS_INET);
        compile_long(x, REACTIVATE_INTERVAL);
        start_rdata(x);
        compile_long(x, rec->u.nameip.ipnum);
        end_rdata(x);

        compile_dnsrecords(x, query.name);

        x->ancount = 1;
        end_assembly(x);

        dump_dnspacket("assembled", x->packet, x->len);

        memcpy(msg + 2, x->packet + 2, x->len - 2);
        return (x->len);
    }

    return (0);
}
#endif
#if (defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_REDIRECT))
/*if redirect mode is on, we will reply any DNS request with device's LAN IP*/
/*Denny Zhang @ Thu Oct 18 16:59:39 CST 2012*/
int send_redirect(unsigned char *msg, int len)
{
    rr_t	query;
    dnsrec_t *rec;

    if((0 == master_redirect_onoff) || (NULL == redirect_dbv))
        return (0);
    if ((parse_query(&query, msg, len) == NULL) ||
            (query.class != DNS_CLASS_INET  ||  
             GET_OPCODE(query.flags) != 0)) 
        return (0);
    if(strstr(query.name, "captive.apple.com"))
        return 0;
    if(strstr(query.name, "www.apple.com"))
        return 0;
    if(strstr(query.name, "detectportal.firefox.com"))
        return 0;

    if (query.type == DNS_TYPE_A || query.type == DNS_TYPE_AAAA) 
    {
        rec = redirect_dbv;
        dnsheader_t *x;
        x = begin_assembly(&query);
        compile_objectname(x);
        compile_int(x, DNS_TYPE_A);
        compile_int(x, DNS_CLASS_INET);
        compile_long(x, REACTIVATE_INTERVAL);
        start_rdata(x);
        compile_long(x, rec->u.nameip.ipnum);
        end_rdata(x);

        compile_dnsrecords(x, query.name);

        x->ancount = 1;
        end_assembly(x);

        dump_dnspacket("assembled", x->packet, x->len);

        memcpy(msg + 2, x->packet + 2, x->len - 2);
        return (x->len);
    }

    return (0);
}

int enable_redirect(char *ipnum)
{
    dnsrec_t *rec;

    rec = alloc_dnsrec(REDIRECT_FAKE_NAME, strlen(REDIRECT_FAKE_NAME) + 1);
    if ((rec->type = create_nameip(&rec->u.nameip, strlen(REDIRECT_FAKE_NAME) + 1, ipnum)) != 0) 
    {
        redirect_dbv = rec;
        master_redirect_onoff = 1;

        return 0;
    }
    else
    {
        master_redirect_onoff = 0;

        return -1;
    }
}

int disable_redirect(void)
{
    master_redirect_onoff = 0;
    if(redirect_dbv)
        free_dnsrec(redirect_dbv);
    redirect_dbv = NULL;

    return 0;
}

#endif

#endif
