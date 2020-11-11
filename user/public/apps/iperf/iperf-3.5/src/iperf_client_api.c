/*
 * iperf, Copyright (c) 2014-2018, The Regents of the University of
 * California, through Lawrence Berkeley National Laboratory (subject
 * to receipt of any required approvals from the U.S. Dept. of
 * Energy).  All rights reserved.
 *
 * If you have questions about your rights to use or distribute this
 * software, please contact Berkeley Lab's Technology Transfer
 * Department at TTD@lbl.gov.
 *
 * NOTICE.  This software is owned by the U.S. Department of Energy.
 * As such, the U.S. Government has been granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable,
 * worldwide license in the Software to reproduce, prepare derivative
 * works, and perform publicly and display publicly.  Beginning five
 * (5) years after the date permission to assert copyright is obtained
 * from the U.S. Department of Energy, and subject to any subsequent
 * five (5) year renewals, the U.S. Government is granted for itself
 * and others acting on its behalf a paid-up, nonexclusive,
 * irrevocable, worldwide license in the Software to reproduce,
 * prepare derivative works, distribute copies to the public, perform
 * publicly and display publicly, and to permit others to do so.
 *
 * This code is distributed under a BSD style license, see the LICENSE
 * file for complete information.
 */
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <arpa/inet.h>

#include "iperf.h"
#include "iperf_api.h"
#include "iperf_util.h"
#include "iperf_locale.h"
#include "net.h"
#include "timer.h"

#ifdef __SC_BUILD__
#include <iperf_drv.h>
#include <sal/iperf_diagnostics.h>
#include "arp.h"
#include <sys/sysinfo.h>
#endif

#if defined(HAVE_TCP_CONGESTION)
#if !defined(TCP_CA_NAME_MAX)
#define TCP_CA_NAME_MAX 16
#endif /* TCP_CA_NAME_MAX */
#endif /* HAVE_TCP_CONGESTION */

int
iperf_create_streams(struct iperf_test *test)
{
    int i, s;
#if defined(HAVE_TCP_CONGESTION)
    int saved_errno;
#endif /* HAVE_TCP_CONGESTION */
    struct iperf_stream *sp;

    int orig_bind_port = test->bind_port;
    for (i = 0; i < test->num_streams; ++i) {

        test->bind_port = orig_bind_port;
	if (orig_bind_port)
	    test->bind_port += i;
        if ((s = test->protocol->connect(test)) < 0)
            return -1;

#if defined(HAVE_TCP_CONGESTION)
	if (test->protocol->id == Ptcp) {
	    if (test->congestion) {
		if (setsockopt(s, IPPROTO_TCP, TCP_CONGESTION, test->congestion, strlen(test->congestion)) < 0) {
		    saved_errno = errno;
		    close(s);
		    errno = saved_errno;
		    i_errno = IESETCONGESTION;
		    return -1;
		} 
	    }
	    {
		socklen_t len = TCP_CA_NAME_MAX;
		char ca[TCP_CA_NAME_MAX + 1];
		if (getsockopt(s, IPPROTO_TCP, TCP_CONGESTION, ca, &len) < 0) {
		    saved_errno = errno;
		    close(s);
		    errno = saved_errno;
		    i_errno = IESETCONGESTION;
		    return -1;
		}
		test->congestion_used = strdup(ca);
		if (test->debug) {
		    printf("Congestion algorithm is %s\n", test->congestion_used);
		}
	    }
	}
#endif /* HAVE_TCP_CONGESTION */

	if (test->sender)
	    FD_SET(s, &test->write_set);
	else
	    FD_SET(s, &test->read_set);
	if (s > test->max_fd) test->max_fd = s;

        sp = iperf_new_stream(test, s);
        if (!sp)
            return -1;

        /* Perform the new stream callback */
        if (test->on_new_stream)
            test->on_new_stream(sp);
    }

    return 0;
}

static void
test_timer_proc(TimerClientData client_data, struct timeval *nowP)
{
    struct iperf_test *test = client_data.p;

    test->timer = NULL;
    test->done = 1;
}

static void
client_stats_timer_proc(TimerClientData client_data, struct timeval *nowP)
{
    struct iperf_test *test = client_data.p;

    if (test->done)
        return;
    if (test->stats_callback)
	test->stats_callback(test);
}

static void
client_reporter_timer_proc(TimerClientData client_data, struct timeval *nowP)
{
    struct iperf_test *test = client_data.p;

    if (test->done)
        return;
    if (test->reporter_callback)
	test->reporter_callback(test);
}

static int
create_client_timers(struct iperf_test * test)
{
    struct timeval now;
    TimerClientData cd;

    if (gettimeofday(&now, NULL) < 0) {
	i_errno = IEINITTEST;
	return -1;
    }
    cd.p = test;
    test->timer = test->stats_timer = test->reporter_timer = NULL;
    if (test->duration != 0) {
	test->done = 0;
        test->timer = tmr_create(&now, test_timer_proc, cd, ( test->duration + test->omit ) * SEC_TO_US, 0);
        if (test->timer == NULL) {
            i_errno = IEINITTEST;
            return -1;
	}
    } 
    if (test->stats_interval != 0) {
        test->stats_timer = tmr_create(&now, client_stats_timer_proc, cd, test->stats_interval * SEC_TO_US, 1);
        if (test->stats_timer == NULL) {
            i_errno = IEINITTEST;
            return -1;
	}
    }
    if (test->reporter_interval != 0) {
        test->reporter_timer = tmr_create(&now, client_reporter_timer_proc, cd, test->reporter_interval * SEC_TO_US, 1);
        if (test->reporter_timer == NULL) {
            i_errno = IEINITTEST;
            return -1;
	}
    }
    return 0;
}

static void
client_omit_timer_proc(TimerClientData client_data, struct timeval *nowP)
{
    struct iperf_test *test = client_data.p;

    test->omit_timer = NULL;
    test->omitting = 0;
    iperf_reset_stats(test);
    if (test->verbose && !test->json_output && test->reporter_interval == 0)
        iperf_printf(test, "%s", report_omit_done);

    /* Reset the timers. */
    if (test->stats_timer != NULL)
        tmr_reset(nowP, test->stats_timer);
    if (test->reporter_timer != NULL)
        tmr_reset(nowP, test->reporter_timer);
}

static int
create_client_omit_timer(struct iperf_test * test)
{
    struct timeval now;
    TimerClientData cd;

    if (test->omit == 0) {
	test->omit_timer = NULL;
        test->omitting = 0;
    } else {
	if (gettimeofday(&now, NULL) < 0) {
	    i_errno = IEINITTEST;
	    return -1;
	}
	test->omitting = 1;
	cd.p = test;
	test->omit_timer = tmr_create(&now, client_omit_timer_proc, cd, test->omit * SEC_TO_US, 0);
	if (test->omit_timer == NULL) {
	    i_errno = IEINITTEST;
	    return -1;
	}
    }
    return 0;
}

int
iperf_handle_message_client(struct iperf_test *test)
{
    int rval;
    int32_t err;

    /*!!! Why is this read() and not Nread()? */
    if ((rval = read(test->ctrl_sck, (char*) &test->state, sizeof(signed char))) <= 0) {
        if (rval == 0) {
            i_errno = IECTRLCLOSE;
            return -1;
        } else {
            i_errno = IERECVMESSAGE;
            return -1;
        }
    }

    switch (test->state) {
        case PARAM_EXCHANGE:
            if (iperf_exchange_parameters(test) < 0)
                return -1;
            if (test->on_connect)
                test->on_connect(test);
            break;
        case CREATE_STREAMS:
#ifdef __SC_BUILD__
            if (sc_iperf_create_streams(test) < 0)
#else
            if (iperf_create_streams(test) < 0)
#endif
                return -1;
            break;
        case TEST_START:
            if (iperf_init_test(test) < 0)
                return -1;
            if (create_client_timers(test) < 0)
                return -1;
            if (create_client_omit_timer(test) < 0)
                return -1;
#ifndef __SC_BUILD__
	    if (!test->reverse)
		if (iperf_create_send_timers(test) < 0)
		    return -1;
#endif
            break;
        case TEST_RUNNING:
#ifdef __SC_BUILD__
            util_scDrv_set_iperf_stream_run();
#endif
            break;
        case EXCHANGE_RESULTS:
            if (iperf_exchange_results(test) < 0)
                return -1;
            break;
        case DISPLAY_RESULTS:
            if (test->on_test_finish)
                test->on_test_finish(test);
            iperf_client_end(test);
            break;
        case IPERF_DONE:
            break;
        case SERVER_TERMINATE:
            i_errno = IESERVERTERM;

	    /*
	     * Temporarily be in DISPLAY_RESULTS phase so we can get
	     * ending summary statistics.
	     */
	    signed char oldstate = test->state;
	    cpu_util(test->cpu_util);
	    test->state = DISPLAY_RESULTS;
	    test->reporter_callback(test);
	    test->state = oldstate;
            return -1;
        case ACCESS_DENIED:
            i_errno = IEACCESSDENIED;
            return -1;
        case SERVER_ERROR:
            if (Nread(test->ctrl_sck, (char*) &err, sizeof(err), Ptcp) < 0) {
                i_errno = IECTRLREAD;
                return -1;
            }
	    i_errno = ntohl(err);
            if (Nread(test->ctrl_sck, (char*) &err, sizeof(err), Ptcp) < 0) {
                i_errno = IECTRLREAD;
                return -1;
            }
            errno = ntohl(err);
            return -1;
        default:
            i_errno = IEMESSAGE;
            return -1;
    }

    return 0;
}



/* iperf_connect -- client to server connection function */
int
iperf_connect(struct iperf_test *test)
{
    FD_ZERO(&test->read_set);
    FD_ZERO(&test->write_set);

    make_cookie(test->cookie);

    /* Create and connect the control channel */
    if (test->ctrl_sck < 0)
	// Create the control channel using an ephemeral port
	test->ctrl_sck = netdial(test->settings->domain, Ptcp, test->bind_address, 0, test->server_hostname, test->server_port, test->settings->connect_timeout);
    if (test->ctrl_sck < 0) {
        i_errno = IECONNECT;
        return -1;
    }

    if (Nwrite(test->ctrl_sck, test->cookie, COOKIE_SIZE, Ptcp) < 0) {
        i_errno = IESENDCOOKIE;
        return -1;
    }

    FD_SET(test->ctrl_sck, &test->read_set);
    if (test->ctrl_sck > test->max_fd) test->max_fd = test->ctrl_sck;

    int opt;
    socklen_t len;

    len = sizeof(opt);
	if (getsockopt(test->ctrl_sck, IPPROTO_TCP, TCP_MAXSEG, &opt, &len) < 0) {
		test->ctrl_sck_mss = 0;
	}
	else {
		if (opt > 0 && opt <= MAX_UDP_BLOCKSIZE) {
			test->ctrl_sck_mss = opt;
		}
		else {
			char str[128];
			snprintf(str, sizeof(str),
				"Ignoring nonsense TCP MSS %d", opt);
			warning(str);

			test->ctrl_sck_mss = 0;
		}
	}

    if (test->verbose) {
	printf("Control connection MSS %d\n", test->ctrl_sck_mss);
    }

    /*
     * If we're doing a UDP test and the block size wasn't explicitly
     * set, then use the known MSS of the control connection to pick
     * an appropriate default.  If we weren't able to get the
     * MSS for some reason, then default to something that should
     * work on non-jumbo-frame Ethernet networks.  The goal is to
     * pick a reasonable default that is large but should get from
     * sender to receiver without any IP fragmentation.
     *
     * We assume that the control connection is routed the same as the
     * data packets (thus has the same PMTU).  Also in the case of
     * --reverse tests, we assume that the MTU is the same in both
     * directions.  Note that even if the algorithm guesses wrong,
     * the user always has the option to override.
     */
    if (test->protocol->id == Pudp) {
	if (test->settings->blksize == 0) {
	    if (test->ctrl_sck_mss) {
		test->settings->blksize = test->ctrl_sck_mss;
	    }
	    else {
		test->settings->blksize = DEFAULT_UDP_BLKSIZE;
	    }
	    if (test->verbose) {
		printf("Setting UDP block size to %d\n", test->settings->blksize);
	    }
	}

	/*
	 * Regardless of whether explicitly or implicitly set, if the
	 * block size is larger than the MSS, print a warning.
	 */
	if (test->ctrl_sck_mss > 0 &&
	    test->settings->blksize > test->ctrl_sck_mss) {
	    char str[128];
	    snprintf(str, sizeof(str),
		     "UDP block size %d exceeds TCP MSS %d, may result in fragmentation / drops", test->settings->blksize, test->ctrl_sck_mss);
	    warning(str);
	}
    }

    return 0;
}


int
iperf_client_end(struct iperf_test *test)
{
    struct iperf_stream *sp;

#ifndef __SC_BUILD__
    /* Close all stream sockets */
    SLIST_FOREACH(sp, &test->streams, streams) {
        close(sp->socket);
    }
#endif

    /* show final summary */
    test->reporter_callback(test);

    if (iperf_set_send_state(test, IPERF_DONE) != 0)
        return -1;

    /* Close control socket */
    if (test->ctrl_sck)
        close(test->ctrl_sck);

    return 0;
}


int
iperf_run_client(struct iperf_test * test)
{
    int startup;
    int result = 0;
    fd_set read_set, write_set;
    struct timeval now;
    struct timeval* timeout = NULL;
    struct iperf_stream *sp;

    if (test->affinity != -1)
	if (iperf_setaffinity(test, test->affinity) != 0)
	    return -1;

    if (test->json_output)
	if (iperf_json_start(test) < 0)
	    return -1;

    if (test->json_output) {
	cJSON_AddItemToObject(test->json_start, "version", cJSON_CreateString(version));
	cJSON_AddItemToObject(test->json_start, "system_info", cJSON_CreateString(get_system_info()));
    } else if (test->verbose) {
	iperf_printf(test, "%s\n", version);
	iperf_printf(test, "%s", "");
	iperf_printf(test, "%s\n", get_system_info());
	iflush(test);
    }

    /* Start the client and connect to the server */
    if (iperf_connect(test) < 0)
        return -1;

    /* Begin calculating CPU utilization */
    cpu_util(NULL);

    startup = 1;
    while (test->state != IPERF_DONE) {
	memcpy(&read_set, &test->read_set, sizeof(fd_set));
	memcpy(&write_set, &test->write_set, sizeof(fd_set));
	(void) gettimeofday(&now, NULL);
	timeout = tmr_timeout(&now);
	result = select(test->max_fd + 1, &read_set, &write_set, NULL, timeout);
	if (result < 0 && errno != EINTR) {
  	    i_errno = IESELECT;
	    return -1;
	}
	if (result > 0) {
	    if (FD_ISSET(test->ctrl_sck, &read_set)) {
 	        if (iperf_handle_message_client(test) < 0) {
		    return -1;
		}
		FD_CLR(test->ctrl_sck, &read_set);
	    }
	}
#ifdef __SC_BUILD__
    if(test->state == TEST_RUNNING){
	    if (test->reverse) {
		// Reverse mode. Client receives.
		if (sc_iperf_recv(test) < 0)
		    return -1;
	    } else {
		// Regular mode. Client sends.
		if (sc_iperf_send(test) < 0)
		    return -1;
	    }
        /*select timeout will also here*/

            /* Run the timers. */
        (void) gettimeofday(&now, NULL);
        tmr_run(&now);

	    /* Is the test done yet? */
	    if ((!test->omitting) &&
	        ((test->duration != 0 && test->done) ||
	         (test->settings->bytes != 0 && test->bytes_sent >= test->settings->bytes) ||
	         (test->settings->blocks != 0 && test->blocks_sent >= test->settings->blocks))) {

		/* Yes, done!  Send TEST_END. */
		test->done = 1;
#ifdef __SC_BUILD__
        util_scDrv_set_iperf_stream_end();
#endif
		cpu_util(test->cpu_util);
		test->stats_callback(test);
		if (iperf_set_send_state(test, TEST_END) != 0)
		    return -1;
	    }
    }
#else
	if (test->state == TEST_RUNNING) {

	    /* Is this our first time really running? */
	    if (startup) {
	        startup = 0;

		// Set non-blocking for non-UDP tests
		if (test->protocol->id != Pudp) {
		    SLIST_FOREACH(sp, &test->streams, streams) {
			setnonblocking(sp->socket, 1);
		    }
		}
	    }

	    if (test->reverse) {
		// Reverse mode. Client receives.
		if (iperf_recv(test, &read_set) < 0)
		    return -1;
	    } else {
		// Regular mode. Client sends.
		if (iperf_send(test, &write_set) < 0)
		    return -1;
	    }

            /* Run the timers. */
            (void) gettimeofday(&now, NULL);
            tmr_run(&now);

	    /* Is the test done yet? */
	    if ((!test->omitting) &&
	        ((test->duration != 0 && test->done) ||
	         (test->settings->bytes != 0 && test->bytes_sent >= test->settings->bytes) ||
	         (test->settings->blocks != 0 && test->blocks_sent >= test->settings->blocks))) {

		// Unset non-blocking for non-UDP tests
		if (test->protocol->id != Pudp) {
		    SLIST_FOREACH(sp, &test->streams, streams) {
			setnonblocking(sp->socket, 0);
		    }
		}

		/* Yes, done!  Send TEST_END. */
		test->done = 1;
		cpu_util(test->cpu_util);
		test->stats_callback(test);
		if (iperf_set_send_state(test, TEST_END) != 0)
		    return -1;
	    }

	}
	// If we're in reverse mode, continue draining the data
	// connection(s) even if test is over.  This prevents a
	// deadlock where the server side fills up its pipe(s)
	// and gets blocked, so it can't receive state changes
	// from the client side.
	else if (test->reverse && test->state == TEST_END) {
	    if (iperf_recv(test, &read_set) < 0)
		return -1;

	}
#endif
    }

    if (test->json_output) {
	if (iperf_json_finish(test) < 0)
	    return -1;
    } else {
	iperf_printf(test, "\n");
	iperf_printf(test, "%s", report_done);
    }

    iflush(test);

    return 0;
}


#ifdef __SC_BUILD__

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
static int arp_list_get_num(void)
{
    int num = 0, i = 0, count = 0;
    sal_arp *arp_info = NULL;
    
    memset(arp_information, 0, sizeof(arp_information));
    num = sal_arp_get_arp_entry(&arp_info);
    for (i=0; i<num; i++)
    {
        memcpy(&(arp_information[count]), &(arp_info[i]), sizeof(sal_arp));
        count++;
    }
    return count;
}
static int _format_mac_addr(char *buf, int buflen, const unsigned char *addr, int len)
{
    int i;
    char *cp = buf;
    for (i = 0; i < len; i++) {
        cp += scnprintf(cp, buflen - (cp - buf), "%02x", addr[i]);
        if (i == len - 1)
            break;
        cp += scnprintf(cp, buflen - (cp - buf), ":");
    }
    return 0;
}
static int str_Null(char *str)
{
    if(str == NULL)
        return 1;
    if(*str == '\0')
        return 1;
    return 0;
}

int sc_iperf_create_streams(struct iperf_test *test)
{
    struct iperf_stream_setting iperf_stream_setting;
    struct iperf_stream *sp = NULL;

    char ifName[16] = "";
    char wan_mac[32] = {0};
    int i = 0;
    int count = 0;
    int arp_table_exist = 0;
    unsigned char mac[6] = {0};
    struct sysinfo info_1, info_2;

    memset(&iperf_stream_setting, 0 , sizeof(struct iperf_stream_setting));
    iperf_stream_setting.parallel = test->num_streams;/*only support one session now*/
    iperf_stream_setting.reverse = test->reverse;/*1: download, 0: upload*/
    iperf_stream_setting.packet_check = 0;

    snprintf(iperf_stream_setting.ifName, sizeof(iperf_stream_setting.ifName), "%s", test->iperf_stream_setting->ifName);
    snprintf(wan_mac, sizeof(wan_mac), "%s", test->iperf_stream_setting->src_mac);


    sscanf(wan_mac, "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0],&mac[1], &mac[2],&mac[3],&mac[4], &mac[5]);
    memcpy(iperf_stream_setting.src_mac, mac, 6);


    if(test->bind_address)
        iperf_stream_setting.local_ip = inet_addr(test->bind_address);
    else{
#ifdef SC_IPERF_DEBUG
        iperf_printf(test, "iperf_stream_setting.local ip is :%s\n", test->iperf_stream_setting->local_ip);
#endif
        iperf_stream_setting.local_ip = inet_addr(test->iperf_stream_setting->local_ip);
    }

    if(!strlen(test->iperf_stream_setting->dest_mac))
        return -1;
    /*gateway as dest mac*/
    sscanf(test->iperf_stream_setting->dest_mac, "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0],&mac[1], &mac[2],&mac[3],&mac[4], &mac[5]);
    memcpy(iperf_stream_setting.dest_mac, mac, 6);

    /*source ip and remote ip*/
    inet_aton(test->server_hostname, &iperf_stream_setting.remote_ip);
    iperf_stream_setting.remote_port = test->server_port;

    if(test->bind_port)
        iperf_stream_setting.local_port = test->bind_port;
    else
        iperf_stream_setting.local_port = 9999;/*default local port*/
    iperf_stream_setting.burst = test->settings->burst;
    iperf_stream_setting.rate = test->settings->rate;
    iperf_stream_setting.session = test->iperf_stream_setting->session;
    iperf_stream_setting.connection_mode = test->iperf_stream_setting->connection_mode;
    iperf_stream_setting.duration = test->duration;

    util_scDrv_set_iperf_stream_end();
    util_scDrv_set_iperf_create_stream_info(&iperf_stream_setting);
    sysinfo(&info_1);

    while(1){
        if(iperf_stream_setting.packet_check == test->num_streams)
            break;
        sysinfo(&info_2);
        if(info_2.uptime - info_1.uptime > 10)
        {
            i_errno = IESTREAMCONNECT;
            util_scDrv_set_iperf_stream_end();
#ifdef SC_IPERF_DEBUG
            iperf_printf(test, "iperf connection pkt timeout\n");
#endif
            return -1;
        }

        util_scDrv_get_iperf_create_stream_state(&iperf_stream_setting);
    }

    for(i = 0; i < test->num_streams; i++)
    {
        sp = iperf_new_stream(test, 0);
        if (!sp)
            return -1;
    }

    return 0;
}
#endif
