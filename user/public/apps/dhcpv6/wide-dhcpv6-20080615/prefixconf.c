/*	$KAME: prefixconf.c,v 1.33 2005/09/16 11:30:15 suz Exp $	*/

/*
 * Copyright (C) 2002 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/ioctl.h>

#include <net/if.h>
#ifdef __FreeBSD__
#include <net/if_var.h>
#endif

#include <netinet/in.h>

#ifdef __KAME__
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#endif
#ifdef __SC_BUILD__
#include <signal.h>
#include <sal/sal_wan.h>
#include "log/slog.h"
#endif
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dhcp6.h"
#include "config.h"
#include "common.h"
#include "timer.h"
#include "dhcp6c_ia.h"
#include "prefixconf.h"
#include "utility.h"

TAILQ_HEAD(siteprefix_list, siteprefix);
struct iactl_pd {
	struct iactl common;
	struct pifc_list *pifc_head;
	struct siteprefix_list siteprefix_head;
};
#define iacpd_ia common.iactl_ia
#define iacpd_callback common.callback
#define iacpd_isvalid common.isvalid
#define iacpd_duration common.duration
#define iacpd_renew_data common.renew_data
#define iacpd_rebind_data common.rebind_data
#define iacpd_reestablish_data common.reestablish_data
#define iacpd_release_data common.release_data
#define iacpd_cleanup common.cleanup

struct siteprefix {
	TAILQ_ENTRY (siteprefix) link;

	struct dhcp6_prefix prefix;
	time_t updatetime;
	struct dhcp6_timer *timer;
	struct iactl_pd *ctl;
	TAILQ_HEAD(, dhcp6_ifprefix) ifprefix_list; /* interface prefixes */
};

struct dhcp6_ifprefix {
	TAILQ_ENTRY(dhcp6_ifprefix) plink;

	/* interface configuration */
	struct prefix_ifconf *ifconf;

	/* interface prefix parameters */
	struct sockaddr_in6 paddr;
	int plen;

	/* address assigned on the interface based on the prefix */
	struct sockaddr_in6 ifaddr;
};

static struct siteprefix *find_siteprefix __P((struct siteprefix_list *,
    struct dhcp6_prefix *, int));
static void remove_siteprefix __P((struct siteprefix *));
static int isvalid __P((struct iactl *));
static u_int32_t duration __P((struct iactl *));
static void cleanup __P((struct iactl *));
static int renew_prefix __P((struct iactl *, struct dhcp6_ia *,
    struct dhcp6_eventdata **, struct dhcp6_eventdata *));
static void renew_data_free __P((struct dhcp6_eventdata *));

static struct dhcp6_timer *siteprefix_timo __P((void *));

static int add_ifprefix __P((struct siteprefix *,
    struct dhcp6_prefix *, struct prefix_ifconf *));

extern struct dhcp6_timer *client6_timo __P((void *));
static int pd_ifaddrconf __P((ifaddrconf_cmd_t, struct dhcp6_ifprefix *ifpfx));
#ifdef __SC_BUILD__
static int isPrefixChanged __P((struct siteprefix *));
static int updateRadvdConfFile __P((ifaddrconf_cmd_t, struct siteprefix *));
#endif

int
update_prefix(ia, pinfo, pifc, dhcpifp, ctlp, callback)
	struct ia *ia;
	struct dhcp6_prefix *pinfo;
	struct pifc_list *pifc;
	struct dhcp6_if *dhcpifp;
	struct iactl **ctlp;
	void (*callback)__P((struct ia *));
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)*ctlp;
	struct siteprefix *sp;
	struct prefix_ifconf *pif;
	int spcreate = 0;
	struct timeval timo;

	/*
	 * A client discards any addresses for which the preferred
         * lifetime is greater than the valid lifetime.
	 * [RFC3315 22.6] 
	 */
	if (pinfo->vltime != DHCP6_DURATION_INFINITE &&
	    (pinfo->pltime == DHCP6_DURATION_INFINITE ||
	    pinfo->pltime > pinfo->vltime)) {
		debug_dprintf(LOG_INFO, FNAME, "invalid prefix %s/%d: "
		    "pltime (%lu) is larger than vltime (%lu)",
		    in6addr2str(&pinfo->addr, 0), pinfo->plen,
		    pinfo->pltime, pinfo->vltime);
		return (-1);
	}

	if (iac_pd == NULL) {
		if ((iac_pd = malloc(sizeof(*iac_pd))) == NULL) {
			debug_dprintf(LOG_NOTICE, FNAME, "memory allocation failed");
			return (-1);
		}
		memset(iac_pd, 0, sizeof(*iac_pd));
		iac_pd->iacpd_ia = ia;
		iac_pd->iacpd_callback = callback;
		iac_pd->iacpd_isvalid = isvalid;
		iac_pd->iacpd_duration = duration;
		iac_pd->iacpd_cleanup = cleanup;
		iac_pd->iacpd_renew_data =
		    iac_pd->iacpd_rebind_data =
		    iac_pd->iacpd_release_data =
		    iac_pd->iacpd_reestablish_data = renew_prefix;

		iac_pd->pifc_head = pifc;
		TAILQ_INIT(&iac_pd->siteprefix_head);
		*ctlp = (struct iactl *)iac_pd;
	}

	/* search for the given prefix, and make a new one if it fails */
	if ((sp = find_siteprefix(&iac_pd->siteprefix_head, pinfo, 1)) == NULL) {
		if ((sp = malloc(sizeof(*sp))) == NULL) {
			debug_dprintf(LOG_NOTICE, FNAME, "memory allocation failed");
			return (-1);
		}
		memset(sp, 0, sizeof(*sp));
		sp->prefix.addr = pinfo->addr;
		sp->prefix.plen = pinfo->plen;
		sp->ctl = iac_pd;
		TAILQ_INIT(&sp->ifprefix_list);

		TAILQ_INSERT_TAIL(&iac_pd->siteprefix_head, sp, link);

		spcreate = 1;
	}

	/* update the timestamp of update */
	sp->updatetime = time(NULL);

	/* update the prefix according to pinfo */
	sp->prefix.pltime = pinfo->pltime;
	sp->prefix.vltime = pinfo->vltime;
	debug_dprintf(LOG_DEBUG, FNAME, "%s a prefix %s/%d pltime=%lu, vltime=%lu",
	    spcreate ? "create" : "update",
	    in6addr2str(&pinfo->addr, 0), pinfo->plen,
	    pinfo->pltime, pinfo->vltime);

	/* update prefix interfaces if necessary */
	if (sp->prefix.vltime != 0 && spcreate) {
		for (pif = TAILQ_FIRST(iac_pd->pifc_head); pif;
		    pif = TAILQ_NEXT(pif, link)) {
			/*
			 * The requesting router MUST NOT assign any delegated
			 * prefixes or subnets from the delegated prefix(es) to
			 * the link through which it received the DHCP message
			 * from the delegating router.
			 * [RFC3633 Section 12.1]
			 */
			if (strcmp(pif->ifname, dhcpifp->ifname) == 0) {
				debug_dprintf(LOG_INFO, FNAME,
				    "skip %s as a prefix interface",
				    dhcpifp->ifname);
				continue;
			}

			add_ifprefix(sp, pinfo, pif);
		}
	}

	/*
	 * If the new vltime is 0, this prefix immediately expires.
	 * Otherwise, set up or update the associated timer.
	 */
	switch (sp->prefix.vltime) {
	case 0:
		remove_siteprefix(sp);
#ifdef __SC_BUILD__
		return (0);    
#else
        break;
#endif
	case DHCP6_DURATION_INFINITE:
		if (sp->timer)
			dhcp6_remove_timer(&sp->timer);
		break;
	default:
		if (sp->timer == NULL) {
			sp->timer = dhcp6_add_timer(siteprefix_timo, sp);
			if (sp->timer == NULL) {
				debug_dprintf(LOG_NOTICE, FNAME,
				    "failed to add prefix timer");
				remove_siteprefix(sp); /* XXX */
				return (-1);
			}
		}
		/* update the timer */
		timo.tv_sec = sp->prefix.vltime;
		timo.tv_usec = 0;

		dhcp6_set_timer(&timo, sp->timer);
		break;
	}
#ifdef __SC_BUILD__
    if(1 == spcreate && 1 == isPrefixChanged(sp))
        sal_misc_set_ipv6_prefix_changed_t("1");
    if(1 == spcreate || 0 == isPrefixChanged(sp))
	updateRadvdConfFile(IFADDRCONF_ADD, sp);
#endif
	return (0);
}

static struct siteprefix *
find_siteprefix(head, prefix, match_plen)
	struct siteprefix_list *head;
	struct dhcp6_prefix *prefix;
	int match_plen;
{
	struct siteprefix *sp;

	for (sp = TAILQ_FIRST(head); sp; sp = TAILQ_NEXT(sp, link)) {
		if (!IN6_ARE_ADDR_EQUAL(&sp->prefix.addr, &prefix->addr))
			continue;
		if (match_plen == 0 || sp->prefix.plen == prefix->plen)
			return (sp);
	}

	return (NULL);
}

static void
remove_siteprefix(sp)
	struct siteprefix *sp;
{
	struct dhcp6_ifprefix *ip;

	debug_dprintf(LOG_DEBUG, FNAME, "remove a site prefix %s/%d",
	    in6addr2str(&sp->prefix.addr, 0), sp->prefix.plen);

	if (sp->timer)
		dhcp6_remove_timer(&sp->timer);
#ifdef __SC_BUILD__
	debug_dprintf(LOG_DEBUG, FNAME, "update radvd conf");
	updateRadvdConfFile(IFADDRCONF_REMOVE, sp);
#endif

	/* remove all interface prefixes */
	while ((ip = TAILQ_FIRST(&sp->ifprefix_list)) != NULL) {
		TAILQ_REMOVE(&sp->ifprefix_list, ip, plink);
		pd_ifaddrconf(IFADDRCONF_REMOVE, ip);
		free(ip);
	}

	TAILQ_REMOVE(&sp->ctl->siteprefix_head, sp, link);
	free(sp);
}

static int
isvalid(iac)
	struct iactl *iac;
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;

	if (TAILQ_EMPTY(&iac_pd->siteprefix_head))
		return (0);	/* this IA is invalid */
	return (1);
}

static u_int32_t
duration(iac)
	struct iactl *iac;
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;
	struct siteprefix *sp;
	u_int32_t base = DHCP6_DURATION_INFINITE, pltime, passed;
	time_t now;

	/* Determine the smallest period until pltime expires. */
	now = time(NULL);
	for (sp = TAILQ_FIRST(&iac_pd->siteprefix_head); sp;
	    sp = TAILQ_NEXT(sp, link)) {
		passed = now > sp->updatetime ?
		    (u_int32_t)(now - sp->updatetime) : 0;
		pltime = sp->prefix.pltime > passed ?
		    sp->prefix.pltime - passed : 0;

		if (base == DHCP6_DURATION_INFINITE || pltime < base)
			base = pltime;
	}

	return (base);
}

static void
cleanup(iac)
	struct iactl *iac;
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;
	struct siteprefix *sp;

	while ((sp = TAILQ_FIRST(&iac_pd->siteprefix_head)) != NULL) {
		TAILQ_REMOVE(&iac_pd->siteprefix_head, sp, link);
		remove_siteprefix(sp);
	}

	free(iac);
}

static int
renew_prefix(iac, iaparam, evdp, evd)
	struct iactl *iac;
	struct dhcp6_ia *iaparam;
	struct dhcp6_eventdata **evdp, *evd;
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;
	struct siteprefix *sp;
	struct dhcp6_list *ial = NULL, pl;

	TAILQ_INIT(&pl);
	for (sp = TAILQ_FIRST(&iac_pd->siteprefix_head); sp;
	    sp = TAILQ_NEXT(sp, link)) {
		if (dhcp6_add_listval(&pl, DHCP6_LISTVAL_PREFIX6,
		    &sp->prefix, NULL) == NULL)
			goto fail;
	}

	if ((ial = malloc(sizeof(*ial))) == NULL)
		goto fail;
	TAILQ_INIT(ial);
	if (dhcp6_add_listval(ial, DHCP6_LISTVAL_IAPD, iaparam, &pl) == NULL)
		goto fail;
	dhcp6_clear_list(&pl);

	evd->type = DHCP6_EVDATA_IAPD;
	evd->data = (void *)ial;
	evd->privdata = (void *)evdp;
	evd->destructor = renew_data_free;

	return (0);

  fail:
	dhcp6_clear_list(&pl);
	if (ial)
		free(ial);
	return (-1);
}

static void
renew_data_free(evd)
	struct dhcp6_eventdata *evd;
{
	struct dhcp6_list *ial;

	if (evd->type != DHCP6_EVDATA_IAPD) {
		debug_dprintf(LOG_ERR, FNAME, "assumption failure");
		exit(1);
	}

	if (evd->privdata)
		*(struct dhcp6_eventdata **)evd->privdata = NULL;
	ial = (struct dhcp6_list *)evd->data;
	dhcp6_clear_list(ial);
	free(ial);
}

static struct dhcp6_timer *
siteprefix_timo(arg)
	void *arg;
{
	struct siteprefix *sp = (struct siteprefix *)arg;
	struct ia *ia;
	void (*callback)__P((struct ia *));

	debug_dprintf(LOG_DEBUG, FNAME, "prefix timeout for %s/%d",
	    in6addr2str(&sp->prefix.addr, 0), sp->prefix.plen);

	ia = sp->ctl->iacpd_ia;
	callback = sp->ctl->iacpd_callback;

	if (sp->timer)
		dhcp6_remove_timer(&sp->timer);

	remove_siteprefix(sp);

	(*callback)(ia);

	return (NULL);
}

static int
add_ifprefix(siteprefix, prefix, pconf)
	struct siteprefix *siteprefix;
	struct dhcp6_prefix *prefix;
	struct prefix_ifconf *pconf;
{
	struct dhcp6_ifprefix *ifpfx = NULL;
	struct in6_addr *a;
	u_long sla_id;
	char *sp;
	int b, i;

	if ((ifpfx = malloc(sizeof(*ifpfx))) == NULL) {
		debug_dprintf(LOG_NOTICE, FNAME,
		    "failed to allocate memory for ifprefix");
		return (-1);
	}
	memset(ifpfx, 0, sizeof(*ifpfx));

	ifpfx->ifconf = pconf;

	ifpfx->paddr.sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
	ifpfx->paddr.sin6_len = sizeof(struct sockaddr_in6);
#endif
	ifpfx->paddr.sin6_addr = prefix->addr;
	ifpfx->plen = prefix->plen + pconf->sla_len;
	/*
	 * XXX: our current implementation assumes ifid len is a multiple of 8
	 */
	if ((pconf->ifid_len % 8) != 0) {
		debug_dprintf(LOG_ERR, FNAME,
		    "assumption failure on the length of interface ID");
		goto bad;
	}
	if (ifpfx->plen + pconf->ifid_len < 0 ||
	    ifpfx->plen + pconf->ifid_len > 128) {
		debug_dprintf(LOG_INFO, FNAME,
			"invalid prefix length %d + %d + %d",
			prefix->plen, pconf->sla_len, pconf->ifid_len);
		goto bad;
	}

	/* copy prefix and SLA ID */
	a = &ifpfx->paddr.sin6_addr;
	b = prefix->plen;
	for (i = 0, b = prefix->plen; b > 0; b -= 8, i++)
		a->s6_addr[i] = prefix->addr.s6_addr[i];
	sla_id = htonl(pconf->sla_id);
	sp = ((char *)&sla_id + 3);
	i = (128 - pconf->ifid_len) / 8;
	for (b = pconf->sla_len; b > 7; b -= 8, sp--)
		a->s6_addr[--i] = *sp;
	if (b)
		a->s6_addr[--i] |= *sp;

	/* configure the corresponding address */
	ifpfx->ifaddr = ifpfx->paddr;
	for (i = 15; i >= pconf->ifid_len / 8; i--)
		ifpfx->ifaddr.sin6_addr.s6_addr[i] = pconf->ifid[i];
	if (pd_ifaddrconf(IFADDRCONF_ADD, ifpfx))
		goto bad;

	/* TODO: send a control message for other processes */

	TAILQ_INSERT_TAIL(&siteprefix->ifprefix_list, ifpfx, plink);

	return (0);

  bad:
	if (ifpfx)
		free(ifpfx);
	return (-1);
}

#ifndef ND6_INFINITE_LIFETIME
#define ND6_INFINITE_LIFETIME 0xffffffff
#endif

static int
pd_ifaddrconf(cmd, ifpfx)
	ifaddrconf_cmd_t cmd;
	struct dhcp6_ifprefix *ifpfx;
{
	struct prefix_ifconf *pconf;

	pconf = ifpfx->ifconf;
#ifdef __SC_BUILD__
    if(ifpfx->plen < 64)
    {
        ifpfx->plen = 64;
        ifpfx->ifaddr.sin6_addr.s6_addr[7] |= 0x01;
    }
#endif
	return (ifaddrconf(cmd, pconf->ifname, &ifpfx->ifaddr, ifpfx->plen, 
	    ND6_INFINITE_LIFETIME, ND6_INFINITE_LIFETIME));
}
#ifdef __SC_BUILD__
const char *f_ip6Prefix = "/var/ip6prefix";
const char *f_radvdConf = "/etc/radvd.conf";
const char *f_radvdPid  = "/var/run/radvd.pid";

const char *radvdConf0 = "\
interface br0\n\
{\n\
  AdvSendAdvert on;\n\
  MinRtrAdvInterval 3;\n\
  MaxRtrAdvInterval 10;\n\
  AdvDefaultPreference low;\n\
";

const char *radvdConf1 = "\
  prefix %s\n\
  {\n\
    AdvPreferredLifetime %s;\n\
    AdvValidLifetime %s;\n\
    AdvOnLink on;\n\
    AdvAutonomous on;\n\
    AdvRouterAddr off;\n\
  };\n\
";

const char *radvdConf2 = "\
};\n\
";

extern char **environ;
int bcmSystemEx __P((char *, int));

/***************************************************************************
 * Function:
 *    int updateRadvdConfFile(ifaddrconf_cmd_t cmd, struct siteprefix *sp)
 * Description:
 *    This function creates a radvd.conf file containing the ipv6 prefix
 *    captured from dhcp6s reply from WAN interface and starts the radvd
 *    daemon.
 * Parameters:
 *    void
 * Returns:
 *    0 if SUCCESS otherwise -1
 ***************************************************************************/
static int isPrefixChanged(struct siteprefix *sp)
{
    char *prefix;
    int plen;

    prefix = in6addr2str(&sp->prefix.addr, 0);
    plen = sp->prefix.plen;
    if(0 == strcmp(sal_wan_get_ipv6_prefix_t(wan_id), prefix) &&
            (plen == atoi(sal_wan_get_ipv6_prefix_len_t(wan_id))))
        return 0;
    return 1;
}
int updateRadvdConfFile(ifaddrconf_cmd_t cmd, struct siteprefix *sp)
{
   char     *addrStr;
   long     addrStrLen, prefixLen;
   char prefix[40];
   char plen[8];
   char pltime[32];
   char vltime[32];
   char prefix_str[64];

   debug_dprintf(LOG_DEBUG, FNAME, "Enter updateRadvdConfFile");
   addrStr    = in6addr2str(&sp->prefix.addr, 0);
   addrStrLen = strlen(addrStr);

   sprintf(prefix, "%s", addrStr);
   sprintf(plen, "%d", sp->prefix.plen);
   if (cmd == IFADDRCONF_ADD)
   {
      /* if this is an update of an existing prefix, look for the entry in the file */
      //sprintf(prefix, "%s/%d pltime %d vltime %d", addrStr, sp->prefix.plen, sp->prefix.pltime, sp->prefix.vltime);
      //prefixLen = strlen(prefix);
      sprintf(pltime, "%d", sp->prefix.pltime);
      sprintf(vltime, "%d", sp->prefix.vltime);
      sal_wan_set_ipv6_prefix_t(wan_id, prefix);
      sal_wan_set_ipv6_prefix_len_t(wan_id, plen);
      sal_wan_set_ipv6_prefix_pltime_t(wan_id, pltime);
      sal_wan_set_ipv6_prefix_vltime_t(wan_id, vltime);
      sprintf(prefix_str, "%s/%s", prefix, plen);
      service_ctl_cmd_lock();
      hcal_wan_set_ipv6_prefix_prefix(wan_id, "DHCPv6", prefix_str);
      hcal_wan_set_ipv6_prefix_preferred_time(wan_id, "DHCPv6", pltime);
      hcal_wan_set_ipv6_prefix_valid_time(wan_id, "DHCPv6", vltime);
      service_ctl_cmd_unlock();
      SYSTEM("/usr/sbin/rc fw_wan restart %d spi", wan_id);
      if(strlen(prefix) > 0 && sp->prefix.plen < 64)
        SYSTEM("/usr/sbin/ip -6 route add to %s/%s dev null > /dev/null 2>&1\n", prefix, plen);

      debug_dprintf(LOG_DEBUG, FNAME, "write prefix to file--add %s\n",addrStr);
      log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "WAN%d has received a prefix %s/%s for prefix delegation on LAN\n", 
            wan_id+1, prefix, plen);
   }
   else if(cmd == IFADDRCONF_REMOVE)
   {
      if(strlen(prefix) > 0 && sp->prefix.plen < 64)
        SYSTEM("/usr/sbin/ip -6 route del to %s/%s dev null > /dev/null 2>&1\n", prefix, plen);
      sal_wan_set_ipv6_prefix_t(wan_id, "");
      sal_wan_set_ipv6_prefix_len_t(wan_id, "");
      sal_wan_set_ipv6_prefix_pltime_t(wan_id, "");
      sal_wan_set_ipv6_prefix_vltime_t(wan_id, "");
      sal_wan_set_ipv6_dns_t(wan_id, "");
      sal_wan_set_ipv6_domain_t(wan_id, "");
      sal_wan_set_ipv6_ntp_t(wan_id, "");
      sal_wan_set_ipv6_sip_t(wan_id, "");
      sal_wan_set_ipv6_sip_name_t(wan_id, "");
      log_wan(LOG_INFO, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "WAN%d has removed a prefix %s/%s for prefix delegation\n", 
            wan_id+1, prefix, plen);
   }
#ifndef CONFIG_SUPPORT_IPV6_LOGO
// Ethan
   system("/usr/sbin/rc radvd restart&");
   system("/usr/sbin/rc dhcp6s restart&");
#ifdef CONFIG_USE_DNSMASQ
   system("/usr/sbin/rc dnsmasq restart&");
#endif
#endif
   return 0;

}  /* End of updateRadvdConfFile() */

/***************************************************************************
// Function Name: bcmSystemEx().
// Description  : launch shell command in the child process.
// Parameters   : command - shell command to launch.
// Returns      : status 0 - OK, -1 - ERROR.
****************************************************************************/
int bcmSystemEx(char *command, int printFlag)
{
   int pid = 0, status = 0;
//   char *newCommand = NULL;

   if ( command == 0 )
      return 1;
   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
#ifdef BRCM_DEBUG
      if (printFlag)
         printf("app: %s\r\n", command);
#endif
//      if (printFlag) {
//        if ((newCommand = strdup(command)) != NULL) {
//           bcmHidePassword(newCommand);
//           free(newCommand);
//        }
//      }

      execve("/bin/sh", argv, environ);
      exit(127);
   }

   /* wait for child process return */
   do {
      if ( waitpid(pid, &status, 0) == -1 ) {
         if ( errno != EINTR )
            return -1;
      } else
         return status;
   } while ( 1 );

   return status;

}  /* End of bcmSystemEx() */
#endif
