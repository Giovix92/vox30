/*
   slog library

   Copyright (C) Martin Huang 2014

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

/*
 *  Name: slog
 *
 *  Component: syslog client
 *
 *  Description: - syslog client with special format
 *
 *  Author: Martin Huang
 */

#ifndef __SLOG_E_H__
#define __SLOG_E_H__

#include <syslog.h>

#define SYNC_CRIT_FLAG (1 << 16)
enum
{
    UNKNOWN_LOG = 0,
    CRIT_LOG,
    NORM_LOG
};
#define LOG_NONUSE_BLOCK_TIME  0
#define LOG_NONUSE_ID          0
#define LOG_WAN_LCP_ECHO_REQUEST_ID    1
#define LOG_WAN_LCP_ECHO_REPLY_ID      2
#define LOG_TR069_INIT_CONNECT_ID    1
#define LOG_TR069_DO_CONNECT_ID      2
#define LOG_TR069_FT_ID              3
#define LOG_TR069_CR_ID              4
#define LOG_DNS_QUERY_ID             1
#define LOG_DNS_RELAY_ID             2
#define LOG_DNS_CHECK_ID             3
#define LOG_FW_BLOCK_TIME           (5*60)
#define LOG_FW_UDP_FLOOD_WAN_ID     (1+1<<25)
#define LOG_FW_UDP_FLOOD_LAN_ID     (2+1<<25)
#define LOG_FW_ICMP_FLOOD_WAN_ID    (3+1<<25)
#define LOG_FW_ICMP_FLOOD_LAN_ID    (4+1<<25)
#define LOG_FW_WINNUKE_WAN_ID       (5+1<<25)
#define LOG_FW_WINNUKE_LAN_ID       (6+1<<25)
#define LOG_FW_SMURF_WAN_ID         (7+1<<25)
#define LOG_FW_SMURF_LAN_ID         (8+1<<25)
#define LOG_FW_FRAGGLE_WAN_ID       (9+1<<25)
#define LOG_FW_FRAGGLE_LAN_ID       (10+1<<25)
#define LOG_FW_LAN_SOURCE_ID        (11+1<<25)
#define LOG_BLOCK_UPNP_MSEARCH_ID        1
#define LOG_BLOCK_UPNP_MSEARCH_TIME     60
#define CRIT_PRE_SIGN (char)('{')
#define CRIT_SUF_SIGN (char)('}')
#define NORM_PRE_SIGN (char)('[')
#define NORM_SUF_SIGN (char)(']')
#define WARNING_SIGN (char)('$')
#define SYNC_CRIT_FLAG_SIGN (char)(';') 
#define MAX_SIZE_PER_LOG    (1024*4) 
#define MAX_SIZE_PER_SOCK   (32*1024)
#define LOG_MAX_SIZE        (64*1024 + 256*1024)
/******************* log api ***********************************************/
#define H_DEF_FUN_LOG_MODULE(module) \
void log_##module(int pri, int type, int id, int sec, const char *fmt, ...)


#ifdef  __cplusplus
extern "C" {
#endif

H_DEF_FUN_LOG_MODULE(sys);

H_DEF_FUN_LOG_MODULE(gpon);
H_DEF_FUN_LOG_MODULE(eth);
H_DEF_FUN_LOG_MODULE(wifi);
H_DEF_FUN_LOG_MODULE(usb);

H_DEF_FUN_LOG_MODULE(voip);
H_DEF_FUN_LOG_MODULE(iptv);
H_DEF_FUN_LOG_MODULE(dns);
H_DEF_FUN_LOG_MODULE(ntp);
H_DEF_FUN_LOG_MODULE(smb);
H_DEF_FUN_LOG_MODULE(ftp);
H_DEF_FUN_LOG_MODULE(ddns);
H_DEF_FUN_LOG_MODULE(umts);

H_DEF_FUN_LOG_MODULE(upnp);
H_DEF_FUN_LOG_MODULE(tr069);
H_DEF_FUN_LOG_MODULE(omci);
H_DEF_FUN_LOG_MODULE(web);
H_DEF_FUN_LOG_MODULE(ssh);
H_DEF_FUN_LOG_MODULE(telnet);
H_DEF_FUN_LOG_MODULE(console);

H_DEF_FUN_LOG_MODULE(pctrl);
H_DEF_FUN_LOG_MODULE(mfilter);
H_DEF_FUN_LOG_MODULE(ipfilter);
H_DEF_FUN_LOG_MODULE(wsch);

H_DEF_FUN_LOG_MODULE(lan);
H_DEF_FUN_LOG_MODULE(wan);
H_DEF_FUN_LOG_MODULE(firewall);
H_DEF_FUN_LOG_MODULE(safenetwork);
H_DEF_FUN_LOG_MODULE(vpn);
H_DEF_FUN_LOG_MODULE(fon);

#ifdef  __cplusplus
}
#endif

#endif

