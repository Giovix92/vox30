// hello_client.c --- A simple demo of Rick van Rein's SRV library.
//
// (c) 2001 Rick van Rein, see README for licensing terms.
//
// Find the library at http://dns.vanrein.org/srv/lib
//
// This protocol awaits connections and receives a name from them, to which
// it responds by greeting the other side and mentioning its own name.


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include "libsrv.h"

#define	MaxSipSrv		10
int main (int argc, char *argv []) 
{
	int ret;
	int i;
	pDnsSrv   slist[MaxSipSrv];
	int no=0;
	struct hostent *host;

	// Verify/report cmdline usage
	if (argc != 2) {
		fprintf (stderr,
			"Usage:   %s domain \n"
			"\n"
			"Example: %s dns.vanrein.org\n"
			"Matches: _sip._udp.dns.vanrein.org. IN SRV ...\n",
			argv [0],
			argv [0]
			);
		exit (1);
	}

	// Setup the connection, report any trouble
	ret=query_sipsrv(argv[1], slist,MaxSipSrv,&no);
	
	if (ret < 0) {
		perror ("Failed to connect client");
		exit (1);
	}
	printf("\nquery_sip_srv return server no=%d\n",no);
	for(i=0;i<no;i++){
		pDnsSrv p;		
		p=slist[i];
		printf("i=%d server %s pri=%d weight=%d port=%d\n",i,(char *)&p->target[0],p->priority,p->weight,p->port );
#if 0		
		if ((host=gethostbyname ((const char *)&p->target[0])) && (host->h_addrtype == AF_INET)) {
			char **ip=host->h_addr_list;
			while (*ip) {
				char *ipnr=*ip;
				struct sockaddr_in sin;
				memset (&sin, 0, sizeof (sin));
				sin.sin_family = AF_INET;
				memcpy (&sin.sin_addr,
						ipnr,
						sizeof (sin.sin_addr));
				

				printf ("\tget ip %s (0x%08lx, %d)\n",inet_ntoa(sin.sin_addr),					
					ntohl(*(unsigned long *)ipnr),
					ntohs (sin.sin_port));
				ip++;

			}
			
		}
#endif		
			
	}
	if(no>0)
		free_sipsrv(slist,no);

	
	exit (0);
}
