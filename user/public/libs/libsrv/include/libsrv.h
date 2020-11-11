#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#ifndef _LIBSRV_H_
#define _LIBSRV_H_

#define     DNS_SRV_TIMEOUT     5   /* seconds */

typedef struct dns_srv_item{
	unsigned short priority; /* smaller value has the higher priority */
	unsigned short weight;   /* larger value has the higher priority */
	unsigned short port;
	char target[1];		/* target ip address or host name */ 
}DnsSrv;


typedef DnsSrv *pDnsSrv;


/************************************************************************* 
 * Purpose: query sip server by domain
 * Input: 
 *		domain  - domain name to query
 * Output:
 * 		slist - buffer to save pDnsSrv pointer array (provide by caller)
 *		slist[0]	----> pointer to sip server 1
 *		slist[1]	----> pointer to sip server 2
 *		...
 *		slist[no-1]--> pointer to sip server no
 *
 *		maxno  - slist array detph		
 *		no - how many items we get and copy to slist
 *      normal_dns - When dns srv query fail we will put the domain into
 *                   slist[0] and set this flag to 1
 *      Melinda, 2012/12/07 DT request autodetect dns srv and normal dns
 *      Add normal_dns to meet DT's request
 *      query_ipv6 - =1 should query ipv6 and ipv4
 *                   =0 only query ipv4
 *************************************************************************/
int query_sipsrv(char *domain, pDnsSrv *slist, int maxno, int *no,int *normal_dns,int query_ipv6);


/************************************************************************* 
 * Purpose: free sip server list
 * Input: 
 *		slist  - server list point array, the contains are returned by
 *			    query_sipsrv()
 *		no - the total item number of slist which is return by query_sipsrv
 *************************************************************************/
int free_sipsrv(pDnsSrv *slist,int no);


int add_default_srv(char *domain, pDnsSrv *slist, int *no,int *normal_dns); /* Melinda, 2012/01/07, Mantis 35215 */

/* Melinda, 2013/02/22 */
int is_a_domain_name(char *domain,int *family,char *ret_ip,int ret_ip_size); 
int query_domain_ip_family(char *domain,int query_ip6,int *family,char *ip,int ip_size);

void melinda_test_fun(void);

#endif
