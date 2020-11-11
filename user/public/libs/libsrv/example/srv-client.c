/* $Id: srv-client.c,v 1.1 2002/09/12 05:44:45 vanrein Exp $
 *
 * srv-client.c --- A shell to run SRV record based servers.
 *
 * Example:
 *	#!/usr/bin/srv-client http/tcp localhost
 *	echo "Content-Type: text/plain"
 *	echo
 *	cat /proc/$1
 * Which is a simple /proc web server.
 *
 * By Rick van Rein, as an example of his libsrv.
 */


#include <libsrv.h>



