// hello_server.c --- A simple demo of Rick van Rein's SRV library.
//
// (c) 2001 Rick van Rein, see README for licensing terms.
//
// Find the library at http://dns.vanrein.org/srv/lib
//
// This protocol awaits connections and receives a name from them, to which
// it responds by greeting the other side and mentioning its own name.
//
// Server note: server runs forever. Killing it keeps the bound-to socket
// reserved for a while, as TCP/IP needs to make sure no more traffic
// comes in over that socket.


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include <arpa/srvname.h>

int main (int argc, char *argv []) {
	int sox;
	int len;
	char msg [100];
	char name [21];
	int nmlen;

	// Verify/report cmdline usage
	if (argc != 3) {
		fprintf (stderr,
			"Usage:   %s domain yourname\n"
			"\n"
			"Example: %s dns.vanrein.org John\n"
			"Matches: _hello._tcp.dns.vanrein.org. IN SRV ...\n",
				argv [0],
				argv [0]
			);
		exit (1);
	}

	// Setup the connection, report any trouble
#ifdef DEBUG
	printf ("pid = %d\n", getpid ());
#endif

	sox = insrv_server ("hello", "tcp", argv [1], NULL, 0, NULL);

	if (sox <= 0) {
		perror ("Failed to connect server");
		exit (1);
	}

#ifdef DEBUG
	printf ("pid = %d, ppid = %d\n", getpid (), getppid ());
#endif


	// Start sending and receiving data over the connection
	nmlen = recv (sox, name, 20, 0);
	if (nmlen <= 0) {
		perror ("Did not receive name of person to greet");
	}
	name [nmlen] = '\0';
	snprintf (msg, 100,
			"Hello %s, pleased to meet you; I am %s\n",
			name, argv [2]);
	len = send (sox, msg, strlen (msg), 0);
	if (len < 0) {
		perror ("Error receiving data");
	}

	// Properly shutdown the server socket
	shutdown (sox, 2);
	close (sox);

	exit (0);
}
