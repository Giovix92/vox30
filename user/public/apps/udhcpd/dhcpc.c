/* dhcpd.c
 *
 * udhcp DHCP client
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include "dhcpd.h"
#include "dhcpc.h"
#include "options.h"
#include "clientpacket.h"
#include "packet.h"
#include "script.h"
#include "socket.h"
#include "debug.h"
#include "pidfile.h"
#ifdef __SC_BUILD__
#include "log/slog.h"
#include "cal_wan.h"
#include <sal/sal_wan.h>
#include "auth.h"
#include <utility.h>
#endif
static int state;
static unsigned long requested_ip; /* = 0 */
static unsigned long server_addr;
static unsigned long timeout;
static int packet_num; /* = 0 */
static int fd;
static int signal_pipe[2];
#define LISTEN_NONE 0
#define LISTEN_KERNEL 1
#define LISTEN_RAW 2
static int listen_mode;

#define DEFAULT_SCRIPT	"/usr/share/udhcpc/default.script"

#ifdef __SC_BUILD__
#define OPT57_MAXIMUM_VALUE             1500
#define OPT57_MINIMUM_LEGAL_VALUE       576
#ifdef CONFIG_SUPPORT_TR111
static char table64[]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#endif
static int broadcast_all;
#endif
struct client_config_t client_config = {
	/* Default options. */
	abort_if_no_lease: 0,
#ifdef __SC_BUILD__
	foreground: 1,
#else
	foreground: 0,
#endif
	quit_after_lease: 0,
	background_if_no_lease: 0,
	interface: "eth0",
	pidfile: NULL,
	script: DEFAULT_SCRIPT,
	clientid: NULL,
#ifdef __SC_BUILD__
	vendorid: NULL,
#ifdef CONFIG_SUPPORT_TR111
	vendorinfo: NULL,
#endif
#endif
	hostname: NULL,
#ifdef __SC_BUILD__
	server:NULL,
#endif
	ifindex: 0,
	arp: "\0\0\0\0\0\0",		/* appease gcc-3.0 */
#ifdef __SC_BUILD__
	user: NULL,
        option2_source_ntp : 0,
        option42_source_ntp : 0,
	wanid:1,
	opt57_maxlen: NULL,
    support_auth: 0,
    auth_key: NULL, //? option 90
    key_id: NULL,
    prevrd: 0,
	mark:0,
	sc_mark:0,
#endif
};

#ifndef BB_VER
static void show_usage(void)
{
	printf(
"Usage: udhcpc [OPTIONS]\n\n"
#ifdef __SC_BUILD__
"  -a, --autooption=id             auto add option basing on ntp/others \n"
#endif

"  -c, --clientid=CLIENTID         Client identifier\n"
"  -H, --hostname=HOSTNAME         Client hostname\n"
"  -h                              Alias for -H\n"
"  -f, --foreground                Do not fork after getting lease\n"
"  -b, --background                Fork to background if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -i, --interface=INTERFACE       Interface to use (default: eth0)\n"

#ifdef __SC_BUILD__
"  -m, --maxlen=LENGTH             Specify the maximum DHCP messages that it is willing to accept\n"
#endif
"  -n, --now                       Exit with failure if lease cannot be\n"
#ifdef __SC_BUILD__
"  -N, --nooption=id               Do not request option id \n"
#endif
"                                  immediately negotiated.\n"
"  -p, --pidfile=file              Store process ID of daemon in file\n"
"  -q, --quit                      Quit after obtaining lease\n"
"  -r, --request=IP                IP address to request (default: none)\n"
#ifdef __SC_BUILD__
"  -R, --nooption=id               Request option id \n"
#endif
"  -s, --script=file               Run file at dhcp events (default:\n"
#ifdef __SC_BUILD__
"  -S, --server=IP                 IP address of l2tp server(default: none)\n"

"  -u, --userclass=userclass       user class \n"
#endif
"                                  " DEFAULT_SCRIPT ")\n"
"  -v, --version                   Display version\n"
#ifdef __SC_BUILD__

"  -V, --vendorid=VendorID         The Vendor ID used for DHCP option 60\n"
#ifdef CONFIG_SUPPORT_TR111
"  -I, --vendorinfo=VendorInfo     The Vendor Info used for DHCP option 125\n"
#endif
"  -w, --wanid=WANID               The wanid of the WAN to use (default: 1)\n"
"  -B                              Broadcast all packets\n"
#endif
	);
	exit(0);
}
#endif


/* just a little helper */
static void change_mode(int new_mode)
{
	DEBUG(LOG_INFO, "entering %s listen mode",
		new_mode ? (new_mode == 1 ? "kernel" : "raw") : "none");
	close(fd);
	fd = -1;
	listen_mode = new_mode;
}


/* perform a renew */
static void perform_renew(void)
{
#ifndef __SC_BUILD__
	LOG(LOG_INFO, "Performing a DHCP renew");
#else
    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Performing a DHCP renew");
#endif
	switch (state) {
	case BOUND:
		change_mode(LISTEN_KERNEL);
	case RENEWING:
	case REBINDING:
		state = RENEW_REQUESTED;
		break;
	case RENEW_REQUESTED: /* impatient are we? fine, square 1 */
		run_script(NULL, "deconfig");
	case REQUESTING:
	case RELEASED:
		change_mode(LISTEN_RAW);
		state = INIT_SELECTING;
		break;
	case INIT_SELECTING:
		break;
		default:
			break;
	}

	/* start things over */
	packet_num = 0;

	/* Kill any timeouts because the user wants this to hurry along */
	timeout = 0;
}


/* perform a release */
static void perform_release(void)
{
	char buffer[16];
	struct in_addr temp_addr;

	/* send release packet */
	if (state == BOUND || state == RENEWING || state == REBINDING) {
		temp_addr.s_addr = server_addr;
		sprintf(buffer, "%s", inet_ntoa(temp_addr));
		temp_addr.s_addr = requested_ip;

#ifndef __SC_BUILD__
		LOG(LOG_INFO, "Unicasting a release of %s to %s", 
				inet_ntoa(temp_addr), buffer);
#else
        log_wan(LOG_DEBUG, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Unicasting a release of %s to %s",inet_ntoa(temp_addr), buffer);
#endif
#ifdef __SC_BUILD__
        if(broadcast_all)
            send_release(0, requested_ip);//broadcast
        else
#endif
		send_release(server_addr, requested_ip); /* unicast */
		run_script(NULL, "deconfig");

	}

#ifndef __SC_BUILD__
	LOG(LOG_INFO, "Entering released state");
#else
    log_wan(LOG_DEBUG, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Entering released state");
#endif
	change_mode(LISTEN_NONE);
	state = RELEASED;
	timeout = 0x7fffffff;


}


/* Exit and cleanup */
static void exit_client(int retval)
{
#ifdef __SC_BUILD__
	sal_wan_set_con_client_pid_t(client_config.wanid, "0");
#endif
	pidfile_delete(client_config.pidfile);
	CLOSE_LOG();

	exit(retval);
}


/* Signal handler */
static void signal_handler(int sig)
{
	if (send(signal_pipe[1], &sig, sizeof(sig), MSG_DONTWAIT) < 0) {
#ifndef __SC_BUILD__
		LOG(LOG_ERR, "Could not send signal: %s",
			strerror(errno));
#else
        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Could not send signal: %s", strerror(errno));
#endif
	}
}


static void background(void)
{
	int pid_fd;

	pid_fd = pidfile_acquire(client_config.pidfile); /* hold lock during fork. */
	while (pid_fd >= 0 && pid_fd < 3) pid_fd = dup(pid_fd); /* don't let daemon close it */
	if (daemon(0, 0) == -1) {
		perror("fork");
		exit_client(1);
	}
	client_config.foreground = 1; /* Do not fork again. */
	pidfile_write_release(pid_fd);
#ifdef __SC_BUILD__
    char pid_buf[16];
    snprintf(pid_buf, sizeof(pid_buf), "%d", getpid());
    sal_wan_set_con_client_pid_t(client_config.wanid, pid_buf);
#endif

}
#ifdef __SC_BUILD__
#ifdef CONFIG_SUPPORT_TR111
static void ILibdecodeblock(unsigned char *in, unsigned char *out)
{
    out[0] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[1] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[2] = (unsigned char ) (in[2] << 6 | in[3]);
}
static int base64Decode_ext(unsigned char* input, unsigned char* output)
{
    unsigned short decodedLen = 0, outLen = 0;
    unsigned char code[4];
    int inputlen;
    int i;
    char *p;

    if(!input || !output)
    {
        return 0;
    }
    inputlen = strlen((char *)input);

    while(decodedLen + 4 <= inputlen)
    {
        for(i = 0; i < 4; i++)
        {
            p = strchr(table64, *(input + decodedLen + i));
            if(p)
                code[i] = (unsigned char)(p - &table64[0]);
            else
                break;
        }
        if(4 == i)
        {
            ILibdecodeblock(code, output + outLen);
            outLen += 3;
            decodedLen += 4;
        }
        else
        {
            memset(&code[i], 0, 4 - i);
            ILibdecodeblock(code, output + outLen);
            decodedLen += 4;
            outLen += (i * 6) / 8;
            *(output + outLen) = '\0';
            return outLen;
        }
     }

    if(inputlen > decodedLen)
    {
        int leftLen = inputlen - decodedLen;

        for(i = 0; i < leftLen; i++)
        {
            p = strchr(table64, *(input + decodedLen + i));
            if(p)
                code[i] = (unsigned char)(p - &table64[0]);
            else
                break;
        }
        memset(&code[i], 0, 4 - i);
        ILibdecodeblock(code, output + outLen);
        decodedLen += leftLen;
        outLen += (i * 6) / 8;
    }

    *(output + outLen) = '\0';
    return outLen;

}
#endif
#endif

#ifdef COMBINED_BINARY
int udhcpc_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	unsigned char *temp, *message;
	unsigned long t1 = 0, t2 = 0, xid = 0;
	unsigned long start = 0, lease;
	fd_set rfds;
	int retval;
	struct timeval tv;
	int c, len;
	struct dhcpMessage packet;
	struct in_addr temp_addr;
#ifdef __SC_BUILD__
        char zero_mac[16] = {0};
	struct in_addr serv_addr;
        unsigned int interval = 0;
#endif
	int pid_fd;
	time_t now;
	int max_fd;
	int sig;
#ifdef __SC_BUILD__	
    int do_detect = 0; //detect dhcp server
    int count_detect = 0;
    int i;
    int option_code = 0;
    char arg_str[256] = "a:c:dfbH:h:i:nN:p:qr:R:s:S:u:vV:w:m:A:k:M:C:B";
#ifdef CONFIG_SUPPORT_TR111
    snprintf(arg_str+strlen(arg_str), sizeof(arg_str)-strlen(arg_str), "I:");
#endif
    broadcast_all = 0;
#endif
	static struct option arg_options[] = {
#ifdef __SC_BUILD__
		{"autooption",	required_argument,	0, 'a'},
#endif
	        {"clientid",	required_argument,	0, 'c'},
#ifdef __SC_BUILD__
		{"detect",	no_argument,		0, 'd'},
#endif
		{"foreground",	no_argument,		0, 'f'},
		{"background",	no_argument,		0, 'b'},
		{"hostname",	required_argument,	0, 'H'},
		{"hostname",    required_argument,  0, 'h'},
		{"interface",	required_argument,	0, 'i'},
		
		{"now", 	no_argument,		0, 'n'},
#ifdef __SC_BUILD__
		{"nooption",	required_argument,	0, 'N'},
#endif
		{"pidfile",	required_argument,	0, 'p'},
		{"quit",	no_argument,		0, 'q'},
		{"request",	required_argument,	0, 'r'},
#ifdef __SC_BUILD__
		{"reqoption",	required_argument,	0, 'R'},
#endif
		{"script",	required_argument,	0, 's'},
#ifdef __SC_BUILD__
		{"server",	required_argument, 	0, 'S'},
#endif
		{"version",	no_argument,		0, 'v'},
#ifdef __SC_BUILD__
		{"vendorid",required_argument,	0, 'V'},
#ifdef CONFIG_SUPPORT_TR111
		{"vendorinfo",required_argument,  0, 'I'},
#endif
		{"wanid",	required_argument,	0, 'w'},
#endif
		{"help",	no_argument,		0, '?'},
#ifdef __SC_BUILD__
		{"userclass",	required_argument,	0, 'u'},
		//opt 57
        {"maxlen", required_argument, 0 ,'m'},
        {"mark", required_argument, 0 ,'M'},
        {"sc_mark", required_argument, 0 ,'C'},
		{"broadcast_all",	no_argument,		0, 'B'},
#endif
     
		{0, 0, 0, 0}
	};

	/* get options */
	while (1) {

		int option_index = 0;
#ifdef __SC_BUILD__
		c = getopt_long(argc, argv, arg_str, arg_options, &option_index);
#else
		c = getopt_long(argc, argv, "c:fbH:h:i:np:qr:s:v", arg_options, &option_index);
#endif
		if (c == -1) break;

		switch (c) {
#ifdef __SC_BUILD__
                case 'a':
			option_code = strtol(optarg, (char **)NULL, 16);
                        if(option_code == DHCP_TIME_OFFSET)
                            client_config.option2_source_ntp = 1;
                        else if(option_code == DHCP_NTP_SERVER)
                            client_config.option42_source_ntp = 1;
		        break;

            case 'm':
                if( client_config.opt57_maxlen) 
                {
                    free(client_config.opt57_maxlen );
                    client_config.opt57_maxlen = NULL;
                }
                
                if( (client_config.opt57_maxlen = malloc(4) ) == NULL )
                    break;
                option_code = strtol(optarg,(char **)NULL, 10);
                if( option_code < OPT57_MINIMUM_LEGAL_VALUE || option_code > OPT57_MAXIMUM_VALUE )
                {
                    option_code = OPT57_MAXIMUM_VALUE ;
                }
                client_config.opt57_maxlen[OPT_CODE] = 57;
                client_config.opt57_maxlen[OPT_LEN] = 2;
                *(unsigned short *)(&client_config.opt57_maxlen[OPT_DATA]) = (unsigned short )option_code ;
                break;
#endif
		case 'c':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.clientid) free(client_config.clientid);
#ifdef __SC_BUILD__
			client_config.clientid = malloc(len + 3);
#else
			client_config.clientid = malloc(len + 2);
#endif
			client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
#ifdef __SC_BUILD__
			client_config.clientid[OPT_LEN] = len + 1;
#else
			client_config.clientid[OPT_LEN] = len;
#endif
			client_config.clientid[OPT_DATA] = 0x0;//0 for non ethernet
#ifdef __SC_BUILD__
			strncpy(client_config.clientid + 3, optarg, len);
#else
			strncpy(client_config.clientid + OPT_DATA, optarg, len);
#endif
			break;
#ifdef __SC_BUILD__
		case 'd':
		    do_detect = 1;
		    break;
		case 'B':
		    broadcast_all = 1;
		    break;
#endif
		case 'f':
			client_config.foreground = 1;
			break;
		case 'b':
			client_config.background_if_no_lease = 1;
			break;
		case 'h':
		case 'H':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);

			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = malloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy(client_config.hostname + 2, optarg, len);
			break;
#ifdef __SC_BUILD__
		case 'S':
		    len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.server)
			    free(client_config.server);
			client_config.server = malloc(len + 1);
			client_config.server[len] = '\0';
			strncpy(client_config.server, optarg, len);
			break;
#endif
		case 'i':
			client_config.interface =  optarg;
			break;
		case 'n':
			client_config.abort_if_no_lease = 1;
			break;
#ifdef __SC_BUILD__
		case 'N':
			option_code = strtol(optarg, (char **)NULL, 16);
			for (i = 0; options[i].code; i++)
				if(option_code == options[i].code)
				{
					options[i].flags &= ~OPTION_REQ;
					break;
				}
			break;
#endif
#ifdef __SC_BUILD__
		case 'V':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.vendorid) free(client_config.vendorid);
			client_config.vendorid = malloc(len + 3);
			client_config.vendorid[OPT_CODE] = DHCP_VENDOR;
			client_config.vendorid[OPT_LEN] = len;
			strncpy(client_config.vendorid + OPT_DATA, optarg, len);
			break;
#ifdef CONFIG_SUPPORT_TR111
        case 'I':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.vendorinfo) free(client_config.vendorinfo);
			client_config.vendorinfo = malloc(len + 1);
			if (NULL == client_config.vendorinfo)
				break;
			client_config.vendorinfo[OPT_CODE] = DHCP_VENDOR_SPECIFIC_INFO;	
			client_config.vendorinfo[OPT_LEN] = base64Decode_ext(optarg, client_config.vendorinfo + OPT_DATA);
			break;
#endif
#endif
		case 'p':
			client_config.pidfile = optarg;
			break;
		case 'q':
			client_config.quit_after_lease = 1;
			break;
		case 'r':
			requested_ip = inet_addr(optarg);
			break;
#ifdef __SC_BUILD__
		case 'R':
			option_code = strtol(optarg, (char **)NULL, 16);
			for (i = 0; options[i].code; i++)
				if(option_code == options[i].code)
				{
					options[i].flags |= OPTION_REQ;
					break;
				}

			break;
#endif
		case 's':
			client_config.script = optarg;
			break;
#ifdef __SC_BUILD__
		case 'u':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.user) free(client_config.user);
			client_config.user = malloc(len + 4);
			client_config.user[OPT_CODE] = DHCP_USER_CLASS_ID;
			client_config.user[OPT_LEN] = len + 1;
			client_config.user[OPT_DATA] = len;
			strncpy(client_config.user + 3, optarg, len);
			break;
#endif
		case 'v':
			printf("udhcpcd, version %s\n\n", VERSION);
			exit_client(0);
			break;
#ifdef __SC_BUILD__
		case 'w':
			client_config.wanid =  atoi(optarg);
			break;
        case 'A':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.auth_key) free(client_config.auth_key);
			client_config.auth_key = malloc(len);
			strncpy(client_config.auth_key, optarg, len);
            client_config.support_auth = 1;
			break;
		case 'k':
			memcpy(client_config.key_id, optarg, 4);
			break;
		case 'M':
			client_config.mark = atoi(optarg);
			break;
		case 'C':
			client_config.sc_mark = atoi(optarg);
			break;
#endif
		default:
			show_usage();
		}
	}

	OPEN_LOG("udhcpc");

#ifndef __SC_BUILD__
	LOG(LOG_INFO, "udhcp client (v%s) started", VERSION);
#else
    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "udhcp client (v%s) started", VERSION);
#endif
	pid_fd = pidfile_acquire(client_config.pidfile);
	pidfile_write_release(pid_fd);
#ifdef __SC_BUILD__
	char pid_buf[16];
	snprintf(pid_buf, sizeof(pid_buf), "%d", getpid());
	sal_wan_set_con_client_pid_t(client_config.wanid, pid_buf);
#ifdef CONFIG_SUPPORT_CGN
    sal_wan_clean_ppp_tags(client_config.wanid);
#endif
#endif
	if (read_interface(client_config.interface, &client_config.ifindex, 
			   NULL, client_config.arp) < 0)
		exit_client(1);
		
	if (!client_config.clientid) {
		client_config.clientid = malloc(6 + 3);
		client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
		client_config.clientid[OPT_LEN] = 7;
		client_config.clientid[OPT_DATA] = 1;
		memcpy(client_config.clientid + 3, client_config.arp, 6);
	}

	/* setup signal handlers */
	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGTERM, signal_handler);

	state = INIT_SELECTING;
	run_script(NULL, "deconfig");
	change_mode(LISTEN_RAW);

	for (;;) {
#ifdef __SC_BUILD__
		if(timeout>0)
			tv.tv_sec = timeout;
		else
			tv.tv_sec = 0;
#else
		tv.tv_sec = timeout - time(0);
#endif
		tv.tv_usec = 0;
		FD_ZERO(&rfds);

		if (listen_mode != LISTEN_NONE && fd < 0) {
			if (listen_mode == LISTEN_KERNEL)
        #if defined(__SC_BUILD__)
                fd = listen_socket(INADDR_ANY, CLIENT_PORT, client_config.interface, 0);
        #else
				fd = listen_socket(INADDR_ANY, CLIENT_PORT, client_config.interface);
        #endif
			else
        #if defined(__SC_BUILD__)
				fd = raw_socket(client_config.ifindex, client_config.mark, client_config.sc_mark);
#else
				fd = raw_socket(client_config.ifindex);
#endif
			if (fd < 0) {
#ifndef __SC_BUILD__
				LOG(LOG_ERR, "FATAL: couldn't listen on socket, %s", strerror(errno));
#else
                log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "FATAL: couldn't listen on socket, %s", strerror(errno));
#endif
				exit_client(0);
			}
		}
		if (fd >= 0) FD_SET(fd, &rfds);
		FD_SET(signal_pipe[0], &rfds);		

		if (tv.tv_sec > 0) {
			DEBUG(LOG_INFO, "Waiting on select...\n");
			max_fd = signal_pipe[0] > fd ? signal_pipe[0] : fd;
			retval = select(max_fd + 1, &rfds, NULL, NULL, &tv);
		} else retval = 0; /* If we already timed out, fall through */
	
		now = time(0);
		if (retval == 0) {
			/* timeout dropped to zero */
			switch (state) {
			case INIT_SELECTING:
#ifdef __SC_BUILD__
			    if(do_detect)
			    {
			        if(count_detect++ == 1)
			        {
				        exit_client(0);
			        }
			    }
#endif
#ifdef __SC_BUILD__
				if (packet_num < 5) {
#else
				if (packet_num < 3) {
#endif
					if (packet_num == 0)
						xid = random_xid();

					/* send discover packet */
					send_discover(xid, requested_ip); /* broadcast */
#ifdef __SC_BUILD__
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "DHCP discover has been sent from  WAN%d\n", (client_config.wanid + 1));

				    if(packet_num == 0)
                        timeout = 4;
                    else
                        timeout = timeout * 2;
#else
					timeout = now + ((packet_num == 2) ? 4 : 2);
#endif

					packet_num++;
				} else {
					if (client_config.background_if_no_lease) {
#ifndef __SC_BUILD__
						LOG(LOG_INFO, "No lease, forking to background.");
#else
                        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "No lease, forking to background.");
#endif
						background();
					} else if (client_config.abort_if_no_lease) {
#ifndef __SC_BUILD__
						LOG(LOG_INFO, "No lease, failing.");
#else
                        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "No lease, failing.");
#endif
						exit_client(1);
				  	}
					/* wait to try again */
					packet_num = 0;
#ifdef __SC_BUILD__
					timeout = 0; // changed the time from 60 to 35 seconds
#else
					timeout = now + 60;
#endif
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
				if (packet_num < 3) {
					/* send request packet */
					if (state == RENEW_REQUESTED)
#ifdef __SC_BUILD__
                        if(broadcast_all)
                            send_renew(xid, 0, requested_ip); /* broadcast */
                        else
#endif
						send_renew(xid, server_addr, requested_ip); /* unicast */
					else send_selecting(xid, server_addr, requested_ip); /* broadcast */
#ifdef __SC_BUILD__
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "DHCP request  has been sent from WAN%d\n", (client_config.wanid + 1));
					timeout = ((packet_num == 2) ? 10 : 2);
#else
					timeout = now + ((packet_num == 2) ? 10 : 2);
#endif
					packet_num++;
				} else {

					/* timed out, go back to init state */
					if (state == RENEW_REQUESTED)
					{
					    run_script(NULL, "deconfig");
					}
					state = INIT_SELECTING;
#ifdef __SC_BUILD__
					timeout = 0;
#else
					timeout = now;
#endif
					packet_num = 0;
					change_mode(LISTEN_RAW);
				}
				break;
			case BOUND:
				/* Lease is starting to run out, time to enter renewing state */
				state = RENEWING;
				change_mode(LISTEN_KERNEL);
#ifdef __SC_BUILD__
                log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Entering renew state");
#else
				DEBUG(LOG_INFO, "Entering renew state");
#endif
				/* fall right through */
			case RENEWING:
				/* Either set a new T1, or enter REBINDING state */
#ifdef __SC_BUILD__
                interval = (t2 - t1)/2;

                if(interval<60)
                    interval = 60;
#endif
                if ((t2 - t1) <= (lease / 14400 + 1)) {
					/* timed out, enter rebinding state */
					state = REBINDING;
#ifdef __SC_BUILD__
					timeout = (t2 - t1);
#else
					timeout = now + (t2 - t1);
#endif
#ifdef __SC_BUILD__
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Entering rebinding state");
#else
					DEBUG(LOG_INFO, "Entering rebinding state");
#endif
				} else {
					/* send a request packet */
#ifdef __SC_BUILD__
                        if(broadcast_all)
                            send_renew(xid, 0, requested_ip); /* broadcast */
                        else
#endif
					send_renew(xid, server_addr, requested_ip); /* unicast */

#ifdef __SC_BUILD__
                    if(t2 <= (t1 + interval))
                    {
                        state = REBINDING;
                        timeout = t2 - t1;
//                        DEBUG(LOG_INFO, "Entering rebinding state");
                        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Entering rebinding state");
                    }
                    else
                    {
                        t1 = interval + t1;
                        timeout = interval;
                    }
#else
                    t1 = (t2 - t1) / 2 + t1;
                    timeout = t1 + start;
#endif
				}
				break;
			case REBINDING:
				/* Either set a new T2, or enter INIT state */
#ifdef __SC_BUILD__
                interval = (lease - t2)/2;

                if(interval<60)
                    interval = 60;
#endif
                if ((lease - t2) <= (lease / 14400 + 1)) {
					/* timed out, enter init state */
					state = INIT_SELECTING;
#ifndef __SC_BUILD__
					LOG(LOG_INFO, "Lease lost, entering init state");
#else
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Lease lost, entering init state");
#endif
					run_script(NULL, "deconfig");
#ifdef __SC_BUILD__
					timeout = 0;
#else
					timeout = now;
#endif
					packet_num = 0;
					change_mode(LISTEN_RAW);
				} else {
					/* send a request packet */
					send_renew(xid, 0, requested_ip); /* boardcast, source ip should be leased ip */
#ifdef __SC_BUILD__
                    if(lease <= (t2 + interval))
                    {
                        timeout = lease-t2;
                        t2 = lease;
                    }
                    else
                    {
                        t2 = interval + t2;
                        timeout = interval;
                    }
#else
					t2 = (lease - t2) / 2 + t2;
					timeout = t2 + start;
#endif

				}
				break;
			case RELEASED:
				/* yah, I know, *you* say it would never happen */
				timeout = 0x7fffffff;
				break;
			}
		} else if (retval > 0 && listen_mode != LISTEN_NONE && FD_ISSET(fd, &rfds)) {
			/* a packet is ready, read it */

			if (listen_mode == LISTEN_KERNEL)
        #ifdef __SC_BUILD__
                len = get_packet(&packet, fd, NULL);
        #else
				len = get_packet(&packet, fd);
        #endif
			else len = get_raw_packet(&packet, fd);

			if (len == -1 && errno != EINTR) {
#ifdef __SC_BUILD__
                log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "error on read, %s, reopening socket", strerror(errno));
#else
				DEBUG(LOG_INFO, "error on read, %s, reopening socket", strerror(errno));
#endif
				change_mode(listen_mode); /* just close and reopen */
			}
#ifdef __SC_BUILD__
			timeout = tv.tv_sec;
#endif
			if (len < 0) continue;
#ifdef __SC_BUILD__
            if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
				DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
				continue;
			}
            if(*message != DHCPFORCERENEW)
#endif
			if (packet.xid != xid) {
				DEBUG(LOG_INFO, "Ignoring XID %lx (our xid is %lx)",
					(unsigned long) packet.xid, xid);
				continue;
			}
#ifdef __SC_BUILD__
                        if(memcmp(packet.chaddr, client_config.arp, 6) != 0 && memcmp(packet.chaddr, zero_mac, sizeof(packet.chaddr)) != 0)
                            continue;
#else
			if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
				DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
				continue;
			}
#endif

			switch (state) {
			case INIT_SELECTING:
				/* Must be a DHCPOFFER to one of our xid's */
				if (*message == DHCPOFFER) {
					if ((temp = get_option(&packet, DHCP_SERVER_ID))) {
					    memcpy(&server_addr, temp, 4);
#ifdef __SC_BUILD__
					    int len;
					    int i;
					    int lenx = 0;
					    char ntp_servers[256] = {0};
                                            int offset;
					    struct in_addr temp_addr;
					    temp_addr.s_addr = server_addr;
					    sal_wan_set_con_dhcp_server_t(client_config.wanid, inet_ntoa(temp_addr));
                                            if(is_option_required(DHCP_NTP_SERVER))
					    {
					        if((temp = get_option_x(&packet, DHCP_NTP_SERVER, &len)))
					        { 
					            for(i = 0; i < (len / 4); i++)
					            { 
					                 memcpy(&temp_addr.s_addr, (temp + i * 4), 4);
						         if(i)
						  	     lenx +=  snprintf(ntp_servers + lenx, sizeof(ntp_servers), ",%s", inet_ntoa(temp_addr));
						         else
							     lenx += snprintf(ntp_servers, sizeof(ntp_servers), "%s", inet_ntoa(temp_addr));
					            }
					            sal_wan_set_con_opt42_t(client_config.wanid, ntp_servers);
				 	        }
					    }
					    if(is_option_required(DHCP_TIME_OFFSET))
					    {
					        if((temp = get_option_x(&packet, DHCP_TIME_OFFSET, &len)))
					        { 
					            { 
					                 memcpy(&offset, temp, 4);
					                 snprintf(ntp_servers, sizeof(ntp_servers), "%d", ntohl(offset));
					            }
					            sal_wan_set_con_opt2_t(client_config.wanid, ntp_servers);
				 	        }
					    }	 
				        if(do_detect)
				        {
				            printf("server_ok");
				            exit_client(0);
				        }
#endif
				
			
						xid = packet.xid;
						requested_ip = packet.yiaddr;

						/* enter requesting state */
						state = REQUESTING;
#ifdef __SC_BUILD__
						timeout = 0;
#else
						timeout = now;
#endif
						packet_num = 0;
#ifdef __SC_BUILD__
                        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "DHCP offer  has been received on WAN%d\n", (client_config.wanid + 1));
#endif
					} else {
#ifdef __SC_BUILD__
                        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "No server ID in message");
#else
						DEBUG(LOG_ERR, "No server ID in message");
#endif
					}
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
			case RENEWING:
			case REBINDING:
				if (*message == DHCPACK) {
			
					if (!(temp = get_option(&packet, DHCP_LEASE_TIME))) {
#ifndef __SC_BUILD__
						LOG(LOG_ERR, "No lease time with ACK, using 1 hour lease");
#else
                        log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "No lease time with ACK, using 1 hour lease");
#endif
						lease = 60 * 60;
					} else {
						memcpy(&lease, temp, 4);
						lease = ntohl(lease);
                    }

#ifdef __SC_BUILD__
                    if ((temp = get_option(&packet, DHCP_T1))) {
                        memcpy(&t1, temp, 4);
                        t1 = ntohl(t1);
                        if (t1 >= lease)
                        {
                            /* enter bound state */
                            t1 = lease / 2;
                        }
                    } else {
                        /* enter bound state */
                        t1 = lease / 2;
                    }

                    if ((temp = get_option(&packet, DHCP_T2))) {
                        memcpy(&t2, temp, 4);
                        t2 = ntohl(t2);
                        if ((t2 >= lease) || (t2 <= t1))
                        {
                            /* little fixed point for n * .875 */
                            t2 = (lease * 0x7) >> 3;
                        }
                    } else {
                        /* little fixed point for n * .875 */
                        t2 = (lease * 0x7) >> 3;
                    }
#else
					/* enter bound state */
					t1 = lease / 2;
					/* little fixed point for n * .875 */
					t2 = (lease * 0x7) >> 3;

#endif

					temp_addr.s_addr = packet.yiaddr;
#ifndef __SC_BUILD__
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld",
						inet_ntoa(temp_addr), lease);
#endif
#ifdef __SC_BUILD__
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "DHCP ack  has been received on WAN%d\n", (client_config.wanid + 1));
#endif
					start = now;
#ifdef __SC_BUILD__
					timeout = t1;
#else
					timeout = t1 + start;
#endif
					requested_ip = packet.yiaddr;
					run_script(&packet,
						   ((state == RENEWING || state == REBINDING) ? "renew" : "bound"));

					state = BOUND;
#ifdef __SC_BUILD__
                    if(client_config.support_auth)
                        change_mode(LISTEN_KERNEL);
                    else
#endif
					change_mode(LISTEN_NONE);
					if (client_config.quit_after_lease) 
						exit_client(0);

					if (!client_config.foreground)
						background();

				} else if (*message == DHCPNAK) {
					/* return to init state */
#ifndef __SC_BUILD__
					LOG(LOG_INFO, "Received DHCP NAK");
#else
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Received DHCP NAK");
#endif
					run_script(&packet, "nak");
					if (state != REQUESTING)
					{

						run_script(NULL, "deconfig");
					}
					state = INIT_SELECTING;
#ifdef __SC_BUILD__
					timeout = 0;
#else
					timeout = now;
#endif
					requested_ip = 0;
					packet_num = 0;
					change_mode(LISTEN_RAW);
					sleep(3); /* avoid excessive network traffic */
					}
					break;
#ifdef __SC_BUILD__
            case BOUND:
                if (*message == DHCPFORCERENEW) {
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "DHCP FORCERENEW has been received on WAN%d\n", (client_config.wanid + 1));
                    if(client_config.support_auth)
                    {
                        if (temp = get_option(&packet, DHCP_OPTION_90)) {
                            struct auth_info auth;
                            int offset = 0;
                            int length = *(temp-1);
                            u_int64_t rd = 0;

                            auth.proto = temp[0];
                            auth.algorithm = temp[1];
                            auth.rdm = temp[2];
                            memcpy(auth.rdvalue, temp+3, sizeof(auth.rdvalue));

                            if(auth.rdm != DHCP_AUTHRDM_MONOCOUNTER)
                                goto end;

                            rd = strtoll(auth.rdvalue, NULL, 10);
                            if(client_config.prevrd)
                            {
                                if(dhcp_auth_replaycheck(auth.rdm, client_config.prevrd, rd))
                                    goto end;
                            }
                            client_config.prevrd = rd;

                            if(auth.proto == DHCP_AUTHPROTO_DELAYED)
                            {
                                memcpy(auth.secret_id, temp+11, sizeof(auth.secret_id));
                                memcpy(auth.hmac_md5, temp+15, sizeof(auth.hmac_md5));
                                if(auth.algorithm != DHCP_AUTHALG_HMACMD5)
                                    goto end;
                            }
                            else
                            {
                                memcpy(auth.info, temp+11, length - 11);
                                if(auth.algorithm != DHCP_AUTHALG_NONE)
                                    goto end;
                            }
                            if(auth.proto == DHCP_AUTHPROTO_DELAYED)
                            {
                                if(memcmp(client_config.key_id, auth.secret_id, 4))
                                {
                                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "secret_id(%s) is different from key_id(%s)\n",client_config.key_id, client_config.key_id);
                                    goto end;
                                }
                                offset = temp - (unsigned char*)(&packet) + 15;
                                if(dhcp_verify_mac((char*)&packet, len, offset, client_config.auth_key) == 0) 
                                    goto success;
                                else
                                {
                                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "invalid authentication message\n");
                                    goto end;
                                }
                            }
                            else
                            {
                                if((strlen(client_config.auth_key) == strlen(auth.info)) && 
                                    (memcmp(client_config.auth_key, auth.info, strlen(auth.info)) == 0))
                                    goto success;
                                else
                                    goto end;
                            }
                        }
                        else
                        {
                            log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "FORCERENEW packet not find option 90\n");
                            goto end;
                        }
                    }
                    else
                    {
                        log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "DHCP client not support option 90\n");
                        goto end;
                    }
success:
                    state = RENEW_REQUESTED;
                    packet_num = 0;
                    timeout = 0;
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Pass authentication,change state RENEWING\n");
                    break;
end:
                    log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Auth info is error\n");
                    
                }
                break;
#endif
				}
			}else if (retval > 0 && FD_ISSET(signal_pipe[0], &rfds))
			{
#ifdef __SC_BUILD__
				if (read(signal_pipe[0], &sig, sizeof(sig)) < 0) {
#else
				if (read(signal_pipe[0], &sig, sizeof(signal)) < 0) {
#endif
					DEBUG(LOG_ERR, "Could not read signal: %s",
						strerror(errno));
					continue; /* probably just EINTR */
				}
				switch (sig) {
				case SIGUSR1:
					perform_renew();
					break;
				case SIGUSR2:
					perform_release();
					break;
				case SIGTERM:
#ifndef __SC_BUILD__
				LOG(LOG_INFO, "Received SIGTERM");
#else
                log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Received SIGTERM");
#endif
                    sleep(1); // when reboot, for apps teardown before release ip
                    perform_release(); // when reboot, release ip
					exit_client(0);
				}
		
		} else if (retval == -1 && errno == EINTR) {
			/* a signal was caught */
#ifdef __SC_BUILD__
				timeout = tv.tv_sec;
#endif

		} else {
			/* An error occured */
#ifdef __SC_BUILD__
            log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "Error on select");
#else
			DEBUG(LOG_ERR, "Error on select");
#endif
		}

	}
	return 0;
}

