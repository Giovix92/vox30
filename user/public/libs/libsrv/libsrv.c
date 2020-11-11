/* $Id: libsrv.c,v 1.4 2002/10/02 13:28:11 vanrein Exp $
 *
 * libsrv.c --- Make a connection to a service described with SRV records.
 *
 * A simple library to quickly allocate a socket on either side of a network
 * connection.  Much simpler than classical sockets, and much better too.
 * Tries IN SRV records first, continues to IN A if none defined.
 *
 * By default, it even forks a new server process for every server connection!
 * This can be overridden, and manual accept() and handling can then be done.
 * Do not expect too much -- for a more complex solution, consider RULI,
 *	http://ruli.sourceforge.net
 *
 * From: Rick van Rein <admin@FingerHosting.com>
 */


/* EXAMPLE USE:
 *
 *	In your DNS config, setup SRV records for a service, say:
 *		_hello._tcp.vanrein.org IN SRV 1234 0 0 bakkerij.orvelte.nep.
 *	Meaning:
 *	 - The hello protocol runs over tcp
 *	 - The server is on port 1234 of host bakkerij.orvelte.nep
 *	 - The zeroes are for load balancing
 *
 *	A hello client starting for this domain looks up the above record and
 *	returns a socket of a network client when invoked as follows:
 *		insrv_client ("finger", "tcp", "vanrein.org");
 *
 *	A hello server started on the server host looks up the same record
 *	and starts service for hello over TCP when it finds one of its names
 *	in the record; a client-connected socket is returned from:
 *		insrv_server ("finger", "tcp", "vanrein.org");
 *	Unless told otherwise, this routine forks off for every connection
 *	to the server.  So, the server can process the request on the
 *	returned socket, close it and exit.
 *
 *	When both calls have completed, they are connected and can start
 *	running the Jabber protocol over their respective sockets.
 */

#define BIND_4_COMPAT

//#include <arpa/srvname.h>
#include "./include/arpa/srvname.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include "libsrv.h"


/* Common offsets into an SRV RR */
#define SRV_COST    (RRFIXEDSZ+0)
#define SRV_WEIGHT  (RRFIXEDSZ+2)
#define SRV_PORT    (RRFIXEDSZ+4)
#define SRV_SERVER  (RRFIXEDSZ+6)
#define SRV_FIXEDSZ (RRFIXEDSZ+6)


/* Data structures */
typedef struct {
	unsigned char buf [PACKETSZ];
	int len;
} iobuf;
typedef unsigned char name [MAXDNAME];
#define MAXNUM_SRV PACKETSZ
#define MY_BUFFER_LENGTH    250

//#define DEBUG   1

/* Global variable for #listeners (no OS implements it, but let's play along) */
int _listen_nr = 10;

/* Local variable for SRV options */
static unsigned long int srv_flags = 0L;
char *sfile = "/tmp/SRVrecords.txt";



/* Setup the SRV options when initialising -- invocation optional */
void insrv_init (unsigned long flags) {
	srv_flags = flags;
	res_init ();
}


/* Test the given SRV options to see if all are set */
int srv_testflag (unsigned long flags) {
	return ((srv_flags & flags) == flags) ? 1 : 0;
}


/* Compare two SRV records by priority and by (scattered) weight */
static int srvcmp (const void *left, const void *right) {
	int lcost = ntohs (((unsigned short **) left ) [0][5]);
	int rcost = ntohs (((unsigned short **) right) [0][5]);
	if (lcost == rcost) {
		lcost = -ntohs (((unsigned short **) left ) [0][6]);
		rcost = -ntohs (((unsigned short **) right) [0][6]);
	}
	if (lcost < rcost) {
		return -1;
	} else if (lcost > rcost) {
		return +1;
	} else {
		return  0;
	}
}
/* Melinda, limited stack size so use global variables */
iobuf query, names;
// Storage for fallback SRV list, constructed when DNS gives no SRV
unsigned char fallbacksrv [2*(MAXCDNAME+SRV_FIXEDSZ+MAXCDNAME)];
unsigned char *srv[MAXNUM_SRV];
name srvname;	/* size: 1025 */

//#define TEST    1
#ifdef TEST	        
int add_test_srv(char *domain, pDnsSrv *slist, int *no,int *normal_dns);   
#endif                 


static int insrv_lookup (char *service, char *proto, char *domain,
			pDnsSrv *slist, int maxno, int *no,int query_ipv6,int *family);
pthread_t query_thread_id;
typedef struct{
    char domain[MY_BUFFER_LENGTH];
    pDnsSrv *slist;
    int maxno;
    int no;
    int query_ipv6;
    int ret;
    int query_done;    
    int family;
}query_data;
query_data my_query_data;

/* Melinda, 2013/02/22 */
int query_ip_by_name(char *domain,int query_ipv6,char *ip,int ip_size,int *family)
{
    int rc;
    struct addrinfo hints, *res=NULL;	
	
    //get ip from hostname
    if(!domain || !ip || !ip_size || !family){
        printf("$@@$ - %s Invalid input domain=%p ip=%s size=%d family=%p\n",__FUNCTION__,domain,ip,ip_size,family);
        return -1;
    }
    *family=AF_INET;  /* use ipv4 as default */
	memset(&hints, 0x00, sizeof(hints));
	if(query_ipv6){
		    printf("$@@$ - %s DNS SRV for domain %s getaddrinfo IPv6\n",__FUNCTION__,domain);
		    hints.ai_family = AF_INET6;
      	    rc = getaddrinfo(domain, NULL, &hints, &res);		    
	}
	else
    {
            rc=-1;
    }
    if (rc != 0){ /* Try IPv4 */
            printf("$@@$ - %s DNS SRV for domain %s getaddrinfo IPv4\n",__FUNCTION__,domain);		    
        	hints.ai_family = AF_INET;
        	rc = getaddrinfo(domain, NULL, &hints, &res);
        	if(rc!=0){
        		printf("$@@$ -%s %s getaddrinfo failed domain=%s rc=%d %s \n",__FILE__,__FUNCTION__,domain,rc,gai_strerror(rc));
        		return -1;
        	}
        	
    }
    if(res){
        	void *ptr;
                
            *family=hints.ai_family;    
        	memset(ip,0,ip_size);
        	if (res->ai_family==AF_INET6)
        	    ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
        	else
        	    ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
        	inet_ntop (res->ai_family, ptr, ip, ip_size);
        	if(ip[0]==0){        		
      			freeaddrinfo(res);
      			printf("$@@$ -%s %s inet_ntop failed domain=%s family=%d (DGRAM=%d STREAM=%d)\n",__FILE__,__FUNCTION__,domain,res->ai_family,SOCK_DGRAM,SOCK_STREAM);
        		return -2;
        	}
        	freeaddrinfo(res);    
		
		
    }  	
    else{
            printf("$@@$ -%s %s getaddrinfo no res domain=%s \n",__FILE__,__FUNCTION__,domain);        		
            return -3;
    }
        
    return 0;
        
}

void *query_thread(void *arg)
{
    int ret=0,normal_dns;
    query_data *pdata=(query_data *)arg;
#ifdef TEST    
    if(strcmp(pdata->domain,"mytest.com.tw")==0)
        add_test_srv(pdata->domain, pdata->slist, &pdata->no,&normal_dns);
    else{
        printf("$@@$ - %s %s %d simulate lookup timeout==10 second...\n",__FILE__,__FUNCTION__,__LINE__,pdata->ret); 
        sleep(10);
    }
    
    /*sleep(DNS_SRV_TIMEOUT+1); */ /* wait for alarm to trigger */
#else    
    ret = insrv_lookup ("sip","udp", pdata->domain, pdata->slist, pdata->maxno, &pdata->no,pdata->query_ipv6,&pdata->family);  
#endif    
    pdata->ret=ret;
    printf("$@@$ - %s %s %d insrv_lookup ret=%d\n",__FILE__,__FUNCTION__,__LINE__,pdata->ret);    
    pdata->query_done=1;    
    return NULL;
    
    
}
void alarm_wakeup (int signum, siginfo_t* siginfo, void* context)
{
   printf("$@@$ - alarm_wakeup: call pthread_cancel of query_thread_id query_done=%d\n",my_query_data.query_done);
   if(!my_query_data.query_done){
        pthread_cancel(query_thread_id);
        my_query_data.query_done=2;
   }
   return;
}

/* Setup a client socket for the named service over the given protocol under
 * the given domain name.
 * Input:
 *		service - service name (e.g "sip")
 *		proto   - protocal (e.g "udp")
 *		domain  - domain name to query
 * Output:
 * 		slist - buffer to save pDnsSrv pointer array (provide by caller)
 *		maxno  - slist array detph
 *		no - how many items we get and copy to slist
 */
static int insrv_lookup (char *service, char *proto, char *domain,
			pDnsSrv *slist, int maxno, int *no,int query_ipv6,int *family) {
	// 1. convert service/proto to svcnm
	// 2. construct SRV query for _service._proto.domain
	// 3. try connecting to all answers in turn
	// 4. if no SRV records exist, lookup A record to connect to on stdport
	// 5. return connection socket or error code


	name svcnm;
	int error=0,rc=0;
	int ctr;
	int n=0,i;
	HEADER *nameshdr;
	unsigned char *here;
	int num_srv=0;
	char *srvip;
	FILE *fd;
	struct addrinfo hints, *res=NULL;
	char   tmp_addr[MY_BUFFER_LENGTH];
    
#if 0
	int rnd;
#endif


	if(!slist || maxno<=0 || !no || !family)
		return -EINVAL;

    *family=-1;
	memset(slist,0,maxno*sizeof(pDnsSrv));
	srv_flags &= ~SRV_GOT_MASK;
	srv_flags |=  SRV_GOT_SRV;

	strcpy (svcnm, "_");
	strcat (svcnm, service);
	strcat (svcnm, "._");
	strcat (svcnm, proto);

	// Note that SRV records are only defined for class IN
	if (domain) {
		printf("$@@$ - Before calling res_querydomain sv=%s domain=%s\n",svcnm,domain);
		names.len=res_querydomain (svcnm, domain, C_IN, T_SRV, names.buf, PACKETSZ);
		printf("$@@$ - After res_querydomain names.len=%d\n",names.len);
	} else {
		names.len=res_query (svcnm, C_IN, T_SRV, names.buf, PACKETSZ);
	}
	if (names.len < 0) {
	    printf("$@@$ - errno=%d %s\n",errno,strerror(errno));
	    error = -ENOENT;
		goto fallback;
	}
	nameshdr=(HEADER *) names.buf;
	here=names.buf + HFIXEDSZ;
#if 0
	rnd=nameshdr->id; 	// Heck, gimme one reason why not!
#endif

	if ((names.len < HFIXEDSZ) || nameshdr->tc) {
		error = -EMSGSIZE;
	}
	switch (nameshdr->rcode) {
		case 1:
			error = -EFAULT;
			goto fallback;
		case 2:
			error = -EAGAIN;
			goto fallback;
		case 3:
			error = -ENOENT;
			goto fallback;
		case 4:
			error = -ENOSYS;
			goto fallback;
		case 5:
			error = -EPERM;
			goto fallback;
		default:
			break;
	}
	if (ntohs (nameshdr->ancount) == 0) {
		error = -ENOENT;
		goto fallback;
	}
	if (ntohs (nameshdr->ancount) > MAXNUM_SRV) {
		error = -ERANGE;
		goto fallback;
	}
	for (ctr=ntohs (nameshdr->qdcount); ctr>0; ctr--) {
		int strlen=dn_skipname (here, names.buf+names.len);
		here += strlen + QFIXEDSZ;
	}
	for (ctr=ntohs (nameshdr->ancount); ctr>0; ctr--) {
		int strlen=dn_skipname (here, names.buf+names.len);


		here += strlen;
		srv [num_srv++] = here;
		here += SRV_FIXEDSZ;
		here += dn_skipname (here, names.buf+names.len);
	}
	if(num_srv>=maxno){
		printf("$@@$ - query_sipsrv: num_srv=%d  exceed maxno=%d set to %d\n",num_srv,maxno,maxno-1);
		num_srv=maxno-1;

	}
	//printf("num_srv=%d\n",num_srv);
	// In case an error occurred, there are no SRV records.
	// Fallback strategy now is: construct two. One with the domain name,
	// the other with the /standard/ service name prefixed.
	// Note: Assuming a domain without the service name prefixed!
fallback:
	if (error) {
		return error; // First error returned
	}
	// End of fallback construction, making sure that variables are defined
	// srv[] points to the SRV RR, num_srv counts the number of entries.
	// Every SRV RR has at least cost, weight, port and servername set.

#ifdef DEBUG
	for (ctr=0; ctr<num_srv; ctr++) {


		if (ns_name_ntop (srv [ctr]+SRV_SERVER, srvname, MAXDNAME) < 0) {
			continue;
		}
		printf ("$@@$ - Considering SRV server %d %d %d %s\n",
				ns_get16 (srv [ctr]+SRV_COST),
				ns_get16 (srv [ctr]+SRV_WEIGHT),
				ns_get16 (srv [ctr]+SRV_PORT),
				srvname
			);
	}
	printf("\n");
#endif

#if 0
	// Overwrite weight with rnd-spread version to divide load over weights
	for (ctr=0; ctr<num_srv; ctr++) {
		*(unsigned short *)(srv [ctr]+SRV_WEIGHT)
			= rnd % (1+ns_get16 (srv [ctr]+SRV_WEIGHT));
	}
#endif
	qsort (srv, num_srv, sizeof (*srv), srvcmp);

#ifdef DEBUG
	printf("$@@$ - After sorting:\n");
	for (ctr=0; ctr<num_srv; ctr++) {


		if (ns_name_ntop (srv [ctr]+SRV_SERVER, srvname, MAXDNAME) < 0) {
			continue;
		}
		printf ("$@@$ - server %d %d %d %s\n",
				ns_get16 (srv [ctr]+SRV_COST),
				ns_get16 (srv [ctr]+SRV_WEIGHT),
				ns_get16 (srv [ctr]+SRV_PORT),
				srvname
			);
	}
	printf("\n");

#endif
	fd = fopen(sfile, "w+");
	for (ctr=0; ctr<num_srv; ctr++) {
		pDnsSrv p;
		int len;
		#if 0 /* IPv4 only */
		struct hostent *host;
		#endif

		if (ns_name_ntop (srv [ctr]+SRV_SERVER, srvname, MAXDNAME) < 0) {
			continue;  /* ignore this item */
		}
#if 1
		//get ip from hostname
		memset(&hints, 0x00, sizeof(hints));
		if(query_ipv6){
		    printf("$@@$ - %s DNS SRV for domain %s index %d %s getaddrinfo IPv6\n",__FUNCTION__,domain,ctr,srvname);
		    hints.ai_family = AF_INET6;
      	    rc = getaddrinfo(srvname, NULL, &hints, &res);
		    
		}
		else
        {
            rc=-1;
        }
        if (rc != 0){ /* Try IPv4 */
            printf("$@@$ - %s DNS SRV for domain %s index %d %s getaddrinfo IPv4\n",__FUNCTION__,domain,ctr,srvname);		    
        	hints.ai_family = AF_INET;
        	rc = getaddrinfo(srvname, NULL, &hints, &res);
        	if(rc!=0){
        		printf("$@@$ - %s getaddrinfo failed host=%s index=%d rc=%d %s \n",__FILE__,srvname,ctr,rc,gai_strerror(rc));
        		continue;
        	}
        	
        }
        if(res){
        	void *ptr;
     
            if(*family==-1){
                *family=res->ai_family;
                printf("$@@$ - %s host=%s index=%d family=%d set as default family\n",__FILE__,srvname,ctr,*family);
        		
            }           
        	memset(tmp_addr,0,MY_BUFFER_LENGTH);
        	if (res->ai_family==AF_INET6)
        	    ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
        	else
        	    ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
        	inet_ntop (res->ai_family, ptr, tmp_addr, MY_BUFFER_LENGTH);
        	if(tmp_addr[0]==0){        		
      			freeaddrinfo(res);
      			printf("$@@$ - %s inet_ntop failed host=%s index=%d family=%d (DGRAM=%d STREAM=%d)\n",__FILE__,srvname,ctr,res->ai_family,SOCK_DGRAM,SOCK_STREAM);
        		continue;
        	}
        	freeaddrinfo(res);
		    srvip = tmp_addr;
		
		
        }  	
        else{
            printf("$@@$ - %s getaddrinfo no res domain=%s srvname=%s index=%d \n",__FILE__,domain,srvname,ctr);        		
            continue;
        }
        #if 0 /* IPv4 only */
		if ((host=gethostbyname ((const char *)srvname)) && (host->h_addrtype == AF_INET)) {
			char **ip0=host->h_addr_list;
			while (*ip0) {
				char *ipnr=*ip0;
				struct sockaddr_in sin;
				memset (&sin, 0, sizeof (sin));
				sin.sin_family = AF_INET;
				memcpy (&sin.sin_addr,ipnr,sizeof (sin.sin_addr));
				srvip=inet_ntoa(sin.sin_addr);
				//printf ("\tget ip %s (0x%08lx, %d)\n",srvip,ntohl(*(unsigned long *)ipnr),ntohs (sin.sin_port));
				if(inet_aton(srvip,&sin.sin_addr))
					break;	//if address is valid use it and break
				ip0++;

			}

		}
		#endif
#else
		srvip=srvname;
#endif

		len=sizeof(DnsSrv)+strlen(srvip)+1;
		//printf("i=%d srvip_len=%d len=%d\n",ctr,strlen(srvip),len);
		p=(pDnsSrv)malloc(len);
		//printf("malloc %d bytes p=%lx\n",len,p);
		if(p==0){
			printf("$@@$ - query_sipsrv malloc failed\n");
			error=-ENOMEM;
			break;
		}
		memset(p,0,len);
		p->priority=ns_get16 (srv [ctr]+SRV_COST);
		p->weight=ns_get16 (srv [ctr]+SRV_WEIGHT);
		p->port=ns_get16 (srv [ctr]+SRV_PORT);
		//printf("Copy srvname %s %s to target\n",srvname,srvip);
		strcpy(p->target,srvip);
		//printf("Set slist %d == %lx\n",n,p);
		slist[n++]=p;
		fprintf(fd, "Set SRV server i=%d srvname=%s, srvip=%s\n",ctr,srvname,srvip);

//#ifdef DEBUG
		printf ("$@@$ - Set SRV server slist=%p, slist[%d]=%p prio=%d w=%d %s\n",slist,n-1,p,
				p->priority,
				p->weight,
				&p->target[0]
			);
//#endif



	}
	if(error)	{
		for(i=0;i<n;i++){
		   if(slist[i])
		   	free(slist[i]);
		}
	}
	*no=n;
	fclose(fd);


	return error;

}

int add_default_srv(char *domain, pDnsSrv *slist, int *no,int *normal_dns)
{
    pDnsSrv p;
	int len;
	
    len=sizeof(DnsSrv)+strlen(domain)+1;
	//printf("i=%d srvip_len=%d len=%d\n",ctr,strlen(srvip),len);
	p=(pDnsSrv)malloc(len);
	//printf("malloc %d bytes p=%lx\n",len,p);
	if(p==0){
			printf("$@@$ - add_default_srv malloc failed\n");
			*no=0;
			return -1;
	}
	memset(p,0,len);
	p->priority=0;
	p->weight=0;
	p->port=5060;
	printf("$@@$ - add_default_srv domain=%s to slist=%p slist[0]=%p\n",domain,slist,p);
	strcpy(p->target,domain);	
	//printf("Set slist %d == %lx\n",n,p);	
	slist[0]=p;
	*no=1;
	*normal_dns=1;
	return 0;
}
#ifdef TEST	
int add_test_srv(char *domain, pDnsSrv *slist, int *no,int *normal_dns)
{
    pDnsSrv p;
	int len;
	char ip[80]="192.168.1.100";
	
    len=sizeof(DnsSrv)+strlen(ip)+1;
	//printf("i=%d srvip_len=%d len=%d\n",ctr,strlen(srvip),len);
	p=(pDnsSrv)malloc(len);
	//printf("malloc %d bytes p=%lx\n",len,p);
	if(p==0){
			printf("$@@$ - add_test_srv malloc failed\n");
			*no=0;
			return -1;
	}
	memset(p,0,len);
	p->priority=0;
	p->weight=0;
	p->port=5060;
	printf("$@@$ - add_default_srv domain=%s ip=%s to slist=%p slist[0]=%p\n",domain,ip,slist,p);
	strcpy(p->target,ip);	
	//printf("Set slist %d == %lx\n",n,p);	
	slist[0]=p;
	*no=1;
	*normal_dns=0;
	return 0;
}
#endif
int is_a_domain_name(char *domain,int *family,char *ret_ip,int ret_ip_size)
{
    char   *a,*leftc=0,*rightc=0;
    struct in6_addr serveraddr;
    char   tmp_addr[MY_BUFFER_LENGTH];
    int    rc;

    *family=-1;
    strncpy(tmp_addr,domain,MY_BUFFER_LENGTH);
    tmp_addr[MY_BUFFER_LENGTH-1]=0;
    a=&tmp_addr[0];
    leftc = strchr(a,'[');
	if(leftc)
		rightc= strchr(a,']');
	if(leftc && rightc){
		*rightc=0;
		leftc++;
		a=leftc;
		
	}
	rc = inet_pton(AF_INET, a, &serveraddr);
    if (rc == 1)    /* valid IPv4 text address? */
    {        
           *family=AF_INET;
           if(ret_ip)
                snprintf(ret_ip,ret_ip_size,"%s",a);
           printf("$@@$ - domain=%s is a IPv4 address\n",domain);   
           return 0;
    }
    else
    {
           rc = inet_pton(AF_INET6, a, &serveraddr);
           if (rc == 1) /* valid IPv6 text address? */
           {
            *family=AF_INET6;
            if(ret_ip)
                snprintf(ret_ip,ret_ip_size,"%s",a);
            printf("$@@$ - domain=%s is a IPv6 address\n",domain); 
            return 0;
           }
     }
     printf("$@@$ - domain=%s len=%d is a domain name\n",domain,strlen(domain)); 
     return 1;
}
/*************************************************************************
 * Purpose: query sip server by domain
 * Input:
 *		domain  - domain name to query
 *      *normal_dns - if equals to 0xff then will do autodetect dns srv and normal dns
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
int query_sipsrv(char *domain, pDnsSrv *slist, int maxno, int *no,int *normal_dns,int query_ipv6) 
{
    
  
	int ret=1,auto_detect=0;
	struct itimerval tout_val;
	int family;
	
	printf("$@@$ - Enter %s - domain=%s no=%d query_ipv6=%d normal_dns=%x\n",__FUNCTION__,domain,*no,query_ipv6,*normal_dns);
	if(*normal_dns==0xff)
	    auto_detect=1;
	*normal_dns=0;
	
	if(*no>0){ //free previous slist
	    printf("$@@$ - %s %s %d - free slist no=%d\n",__FILE__,__FUNCTION__,__LINE__,*no);
	    free_sipsrv(slist,*no);
	    *no=0;
	}
	
    if (is_a_domain_name(domain,&family,NULL,0)) {/* Melinda, 2012/12/27, if not a domain name then skip dns srv query */
            struct sigaction act, oact;
            sigset_t set, oset;
            
            strncpy(my_query_data.domain,domain,MY_BUFFER_LENGTH);
            my_query_data.domain[MY_BUFFER_LENGTH-1]=0;
            my_query_data.slist=slist;
            my_query_data.maxno=maxno;
            my_query_data.no=0; /* will set in pthread */
            my_query_data.query_ipv6=query_ipv6;
            my_query_data.ret=1;
            my_query_data.query_done=0;
  
            tout_val.it_interval.tv_sec = 0;
            tout_val.it_interval.tv_usec = 0;
            tout_val.it_value.tv_sec = DNS_SRV_TIMEOUT; 
            tout_val.it_value.tv_usec = 0;
            setitimer(ITIMER_REAL, &tout_val,0);

            
            act.sa_sigaction = alarm_wakeup;
	        sigemptyset(&act.sa_mask);
	        act.sa_flags = SA_SIGINFO;
	        sigaction(SIGALRM, &act, &oact);    /* set the Alarm signal capture */
            
            
            pthread_create(&query_thread_id,NULL,(void *)query_thread,(void *)&my_query_data);
            printf("$@@$ - After query_thread create\n");
            pthread_join(query_thread_id,NULL);
            
            tout_val.it_interval.tv_sec = 0;
            tout_val.it_interval.tv_usec = 0;
            tout_val.it_value.tv_sec = 0; 
            tout_val.it_value.tv_usec = 0;
            setitimer(ITIMER_REAL, &tout_val,0);
            
            sigemptyset(&set);
		    sigaddset(&set, SIGALRM);
		    sigprocmask(SIG_UNBLOCK, &set, &oset);
        
            /*act.sa_handler = SIG_IGN;
	        act.sa_flags = 0; */
	        sigaction(SIGALRM, &oact, NULL);
        
            printf("$@@$ - After query_thread join, query_done=%d ret=%d no=%d\n",my_query_data.query_done,my_query_data.ret,my_query_data.no);	    
            ret=my_query_data.ret;
            *no=my_query_data.no;
        
	}
	else
	    auto_detect=1;  /* Melinda, 2013/01/14, Input is not domain name set it to first slist */
	if(ret && auto_detect){
	    ret=add_default_srv(domain, slist, no,normal_dns);	    
	}
	
	printf("$@@$ - Exit %s - domain=%s no=%d normal_dns=%d ret=%d\n",__FUNCTION__,domain,*no,*normal_dns,ret);
	
	return ret;

}

int free_sipsrv(pDnsSrv *slist,int no)
{
	int i;

	if(slist==0 || no<=0)
		return -1;
	for(i=0;i<no;i++){
		if(slist[i]){
#ifdef DEBUG
			printf("$@@$ - free_sipsrv: slist=%p no=%d free slist[%d]=%p\n",slist,no,i,slist[i]);
#endif
		   	free(slist[i]);
		}
	}

	return 0;

}


/*
 * $Log: libsrv.c,v $
 * Revision 1.4  2002/10/02 13:28:11  vanrein
 * Added include file <sys/time.h> that is needed on Linux.
 *
 * Revision 1.3  2002/09/14 20:29:24  vanrein
 * Adapted libsrv to detect + shoot zombie child processes.  HTTPproxy adapted.
 *
 * Revision 1.2  2002/09/13 11:07:34  vanrein
 * Added more include-files to support portability to other Unices than Linux.
 *
 * Revision 1.1  2002/09/12 05:44:44  vanrein
 * The first checkin of libsrv -- this compiles to a good .so
 * The hello demo works properly.
 * Due to resolver differences, this only compiles on Linux for now.
 *
 */
void melinda_test_fun(void)
{
	printf("just for testing\n");
}
/*************************************************************************************/
/* Melinda, 2013/02/22                                                               */
/* NOTE: 1. The input must be a domain name                                          */
/*       2. Should not call query_domain_ip_family and query_sipsrv at the same time */
/*************************************************************************************/
int query_domain_ip_family(char *domain,int query_ipv6,int *family,char *ip,int ip_size)
{
    struct sigaction act, oact;
    sigset_t set, oset;
    int rc=0;
    pDnsSrv  slist[5];
    struct itimerval tout_val;
            
    if(!domain || !ip || !ip_size || !family){
        printf("$@@$ - %s Invalid input domain=%p ip=%s size=%d family=%p\n",__FUNCTION__,domain,ip,ip_size,family);
        return -1;
    }
    printf("$@@$ - %s domain=%s qipv6=%d\n",__FUNCTION__,domain,query_ipv6);
    strncpy(my_query_data.domain,domain,MY_BUFFER_LENGTH);
    my_query_data.domain[MY_BUFFER_LENGTH-1]=0;
    my_query_data.slist=slist;
    my_query_data.maxno=5;
    my_query_data.no=0; /* will set in pthread */
    my_query_data.query_ipv6=query_ipv6;
    my_query_data.ret=1;
    my_query_data.query_done=0;
  
    tout_val.it_interval.tv_sec = 0;
    tout_val.it_interval.tv_usec = 0;
    tout_val.it_value.tv_sec = DNS_SRV_TIMEOUT; 
    tout_val.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &tout_val,0);

            
    act.sa_sigaction = alarm_wakeup;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGALRM, &act, &oact);    /* set the Alarm signal capture */
            
            
    pthread_create(&query_thread_id,NULL,(void *)query_thread,(void *)&my_query_data);
    printf("$@@$ query_domain_ip_family %s - After query_thread create\n",domain);
    pthread_join(query_thread_id,NULL);
            
    tout_val.it_interval.tv_sec = 0;
    tout_val.it_interval.tv_usec = 0;
    tout_val.it_value.tv_sec = 0; 
    tout_val.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &tout_val,0);
            
    sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &set, &oset);
        
    /*act.sa_handler = SIG_IGN;
	act.sa_flags = 0; */
	sigaction(SIGALRM, &oact, NULL);
        
    printf("$@@$ query_domain_ip_family - After query_thread join, query_done=%d ret=%d no=%d\n",my_query_data.query_done,my_query_data.ret,my_query_data.no);	    
            
            
    if(my_query_data.ret || my_query_data.no<=0 || slist[0]==0){ /* dns srv query timeout, do normal dns query */
        rc=query_ip_by_name(domain,query_ipv6,ip,ip_size,family);
        printf("$@@$ - %s domain=%s qipv6=%d, do normal dns query, ip=%s family=%d, rc=%d\n",__FUNCTION__,domain,query_ipv6,ip,*family,rc);
        
    }
    else{
        snprintf(ip,ip_size,"%s",&slist[0]->target[0]);
        printf("$@@$ - %s domain=%s qipv6=%d dnssrv ok, ip of 1st srv=%s family=%d\n",__FUNCTION__,domain,query_ipv6,ip,my_query_data.family);
        if(my_query_data.family!=-1)
            *family=my_query_data.family;
        else
            *family=AF_INET; /* use ipv4 as default */
            
    }
    return rc;
}
