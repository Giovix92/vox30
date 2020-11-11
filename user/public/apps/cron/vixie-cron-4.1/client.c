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

#include <cron_api.h>

int main(int argc, char ** argv)
{
    int type = -1;

    if (argc < 3)
    {
        printf("Arguments too less\n");
        return 0;
    }
    /* argv[0]: process name
       argv[1]: request type
       argv[2]: group
       argv[3]: id
       argv[4]: stime(min)
       argv[5]: etime(hour)
       argv[6]: week
       argv[7]: in_cmd
       argv[8]: out_cmd
    */ 
    type = atoi(argv[1]);
   
    if (type == 0)
    {
        /* min, hour, week, group, command */
        cron_request_circle(argv[3], argv[4], argv[5], argv[2], argv[6]);
    }
    else if (type == 2)  /* flush */
    {
        cron_request_flush(argv[2], argv[7]);
    }
    else
    {
    //    cron_request_once(argv[3], argv[4], argv[5], argv[2], argv[6]);
    }        
        
            
    return 0;
}
