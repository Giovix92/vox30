/* vi: set sw=4 ts=4: */
/*
 * Mini nslookup implementation for busybox
 *
 * Copyright (C) 1999,2000 by Lineo, inc. and John Beppu
 * Copyright (C) 1999,2000,2001 by John Beppu <beppu@codepoet.org>
 *
 * Correct default name server display and explicit name server option
 * added by Ben Zeckel <bzeckel@hmc.edu> June 2001
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include <resolv.h>
#include "libbb.h"
#ifdef __SC_BUILD__
#include <sal/sal_nslookup.h>
#endif

/*
 * I'm only implementing non-interactive mode;
 * I totally forgot nslookup even had an interactive mode.
 *
 * This applet is the only user of res_init(). Without it,
 * you may avoid pulling in _res global from libc.
 */

/* Examples of 'standard' nslookup output
 * $ nslookup yahoo.com
 * Server:         128.193.0.10
 * Address:        128.193.0.10#53
 *
 * Non-authoritative answer:
 * Name:   yahoo.com
 * Address: 216.109.112.135
 * Name:   yahoo.com
 * Address: 66.94.234.13
 *
 * $ nslookup 204.152.191.37
 * Server:         128.193.4.20
 * Address:        128.193.4.20#53
 *
 * Non-authoritative answer:
 * 37.191.152.204.in-addr.arpa     canonical name = 37.32-27.191.152.204.in-addr.arpa.
 * 37.32-27.191.152.204.in-addr.arpa       name = zeus-pub2.kernel.org.
 *
 * Authoritative answers can be found from:
 * 32-27.191.152.204.in-addr.arpa  nameserver = ns1.kernel.org.
 * 32-27.191.152.204.in-addr.arpa  nameserver = ns2.kernel.org.
 * 32-27.191.152.204.in-addr.arpa  nameserver = ns3.kernel.org.
 * ns1.kernel.org  internet address = 140.211.167.34
 * ns2.kernel.org  internet address = 204.152.191.4
 * ns3.kernel.org  internet address = 204.152.191.36
 */
#ifdef __SC_BUILD__
static int nslooukup_diag = 0;
static int id = 0;
static struct timeval btime;
#endif
static int print_host(const char *hostname, const char *header)
{
	/* We can't use xhost2sockaddr() - we want to get ALL addresses,
	 * not just one */
	struct addrinfo *result = NULL;
	int rc;
	struct addrinfo hint;
#ifdef __SC_BUILD__
    int len = 0;
    char buf[256] = "";
    struct timeval ctime;
#endif
    

	memset(&hint, 0 , sizeof(hint));
	/* hint.ai_family = AF_UNSPEC; - zero anyway */
	/* Needed. Or else we will get each address thrice (or more)
	 * for each possible socket type (tcp,udp,raw...): */
	hint.ai_socktype = SOCK_STREAM;
	// hint.ai_flags = AI_CANONNAME;
#ifdef __SC_BUILD__
    if(nslooukup_diag && (strcmp(header, "Name:") == 0))
        gettimeofday(&btime,NULL);
#endif
	rc = getaddrinfo(hostname, NULL /*service*/, &hint, &result);
#ifdef __SC_BUILD__
    if(nslooukup_diag && (strcmp(header, "Name:") == 0))
    {
        gettimeofday(&ctime,NULL);
        snprintf(buf, sizeof(buf), "%d", (ctime.tv_sec - btime.tv_sec)*1000 + (ctime.tv_usec - btime.tv_usec)/1000);//ms
        sal_nslookup_set_result_response_time(buf, id);
    }
#endif

	if (!rc) {
		struct addrinfo *cur = result;
		unsigned cnt = 0;

		printf("%-10s %s\n", header, hostname);
		// puts(cur->ai_canonname); ?
		while (cur) {
			char *dotted, *revhost;
			dotted = xmalloc_sockaddr2dotted_noport(cur->ai_addr);
			revhost = xmalloc_sockaddr2hostonly_noport(cur->ai_addr);

			printf("Address %u: %s%c", ++cnt, dotted, revhost ? ' ' : '\n');
#ifdef __SC_BUILD__
            if(id)
                len += sprintf(buf+len, "%s,", dotted);
#endif
			if (revhost) {
				puts(revhost);
				if (ENABLE_FEATURE_CLEAN_UP)
					free(revhost);
			}
			if (ENABLE_FEATURE_CLEAN_UP)
				free(dotted);
			cur = cur->ai_next;
		}
#ifdef __SC_BUILD__
        if(nslooukup_diag && id)
        {
            if(strcmp(header, "Name:") == 0)
            {
                sal_nslookup_set_result_ip(buf, id);
                sal_nslookup_set_result_answer_type("NonAuthoritative", id);
                sal_nslookup_set_result_status(SUCCESS, id);
            }
            else
                sal_nslookup_set_result_dns_server(buf, id);
        }
#endif
	} else {
#if ENABLE_VERBOSE_RESOLUTION_ERRORS
		bb_error_msg("can't resolve '%s': %s", hostname, gai_strerror(rc));
#else
		bb_error_msg("can't resolve '%s'", hostname);
#endif
#ifdef __SC_BUILD__
        if(nslooukup_diag)
        {
            if(rc == -2)
            {
                if(strcmp(header, "Name:") == 0)
                    sal_nslookup_set_result_status(HOSTNAME_NOTRESESOLVED, id);
                else
                    sal_nslookup_set_result_status(DNSSERVER_NOTALAILABLE, id);
            }
            else
                sal_nslookup_set_result_status(ERROR_OTHER, id);
        }
#endif
	}
	if (ENABLE_FEATURE_CLEAN_UP)
		freeaddrinfo(result);
#ifdef __SC_BUILD__
    if(nslooukup_diag)
        sal_nslookup_set_status(NSLOOKUP_EXIT);
#endif
	return (rc != 0);
}

/* lookup the default nameserver and display it */
static void server_print(void)
{
	char *server;
	struct sockaddr *sa;

#if ENABLE_FEATURE_IPV6
	sa = (struct sockaddr*)_res._u._ext.nsaddrs[0];
	if (!sa)
#endif
		sa = (struct sockaddr*)&_res.nsaddr_list[0];
	server = xmalloc_sockaddr2dotted_noport(sa);

	print_host(server, "Server:");
	if (ENABLE_FEATURE_CLEAN_UP)
		free(server);
	bb_putchar('\n');
}

/* alter the global _res nameserver structure to use
   an explicit dns server instead of what is in /etc/resolv.conf */
static void set_default_dns(const char *server)
{
	len_and_sockaddr *lsa;

#ifdef __SC_BUILD__
    if(nslooukup_diag)
        sal_nslookup_set_status(DNS_RESOLVE);
#endif
	/* NB: this works even with, say, "[::1]:5353"! :) */
	lsa = xhost2sockaddr(server, 53);

	if (lsa->u.sa.sa_family == AF_INET) {
		_res.nscount = 1;
		/* struct copy */
		_res.nsaddr_list[0] = lsa->u.sin;
	}
#if ENABLE_FEATURE_IPV6
	/* Hoped libc can cope with IPv4 address there too.
	 * No such luck, glibc 2.4 segfaults even with IPv6,
	 * maybe I misunderstand how to make glibc use IPv6 addr?
	 * (uclibc 0.9.31+ should work) */
	if (lsa->u.sa.sa_family == AF_INET6) {
		// glibc neither SEGVs nor sends any dgrams with this
		// (strace shows no socket ops):
		//_res.nscount = 0;
		_res._u._ext.nscount = 1;
		/* store a pointer to part of malloc'ed lsa */
		_res._u._ext.nsaddrs[0] = &lsa->u.sin6;
		/* must not free(lsa)! */
	}
#endif
}

int nslookup_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int nslookup_main(int argc, char **argv)
{
	/* We allow 1 or 2 arguments.
	 * The first is the name to be looked up and the second is an
	 * optional DNS server with which to do the lookup.
	 * More than 3 arguments is an error to follow the pattern of the
	 * standard nslookup */
#ifdef __SC_BUILD__
	if (!argv[1] || argv[1][0] == '-' || ((argc == 4) && strcmp(argv[3], "-d")) || argc > 4)
#else
	if (!argv[1] || argv[1][0] == '-' || argc > 3)
#endif
		bb_show_usage();
#ifdef __SC_BUILD__
    if(argc == 4)
    {
        char buf[32] = "";
        char *value = NULL;
        snprintf(buf, sizeof(buf), "%d", getpid());
        sal_nslookup_set_pid(buf);
        nslooukup_diag = 1;
        value = sal_nslookup_get_result_num();
        if(value)
        {
            id = atoi(value);
            sal_nslookup_set_result_answer_type("NonAuthoritative", id);
        }
    }
#endif

	/* initialize DNS structure _res used in printing the default
	 * name server and in the explicit name server option feature. */
	res_init();
	/* rfc2133 says this enables IPv6 lookups */
	/* (but it also says "may be enabled in /etc/resolv.conf") */
	/*_res.options |= RES_USE_INET6;*/

	if (argv[2])
		set_default_dns(argv[2]);

	server_print();
	return print_host(argv[1], "Name:");
}
