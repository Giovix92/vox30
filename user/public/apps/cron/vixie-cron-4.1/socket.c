#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cron.h"

#define MAX_CONNECTION  3

int init_server_socket(void)
{
    struct sockaddr_un addr;
    int listen_fd = -1;
    
    if ( (listen_fd = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1 ) {
        Debug(DSOCK, ("Could not create socket\n"))
	    goto err;
    }

    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, CRON_LOCAL_SOCKET, sizeof(addr.sun_path));
    addr.sun_path[sizeof(addr.sun_path) -1 ] = '\0';
    
    unlink(CRON_LOCAL_SOCKET);
    if (bind(listen_fd, (struct sockaddr*) &addr, SUN_LEN(&addr)) != 0){
        Debug(DSOCK, ("Could not bind socket\n"))
	    goto err;
    }

    if ( listen(listen_fd, MAX_CONNECTION) != 0 ) {
        Debug(DSOCK, ("Cannot set socket in listen mode\n"))
	    goto err;
    }
    
#if 0
    /* The exec bit is not necessary and ignored on all systems but AIX */
    if ( chmod(CRON_LOCAL_SOCKET, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH) != 0 ){
        Debug(DSOCK, ("Cannot chmod() socket file\n"))
    }    
#endif

    Debug(DSOCK, ("listen_fd=%d\n", listen_fd))
        
    return listen_fd;

err:
    if (listen_fd >= 0)
        close(listen_fd);
    
    return -1;
}




