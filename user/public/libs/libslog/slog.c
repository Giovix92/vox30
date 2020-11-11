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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>

#include <log/slog.h>
#include <errno.h>

#define DEF_FUN_LOG_MODULE(module, module_str, index) \
    void log_##module(int pri, int type, int id, int sec, const char *fmt, ...) \
{\
    char buf[MAX_SIZE_PER_LOG] = ""; \
    char sync_flag[8] = ""; \
    int length = 0; \
    va_list arg; \
    char pre_sign = ((type & 0xFF) == CRIT_LOG) ? CRIT_PRE_SIGN : NORM_PRE_SIGN; \
    char suf_sign = ((type & 0xFF) == CRIT_LOG) ? CRIT_SUF_SIGN : NORM_SUF_SIGN; \
    if (((type & 0xFF) == CRIT_LOG) && (type & SYNC_CRIT_FLAG)) \
    snprintf(sync_flag, sizeof(sync_flag), "%c", SYNC_CRIT_FLAG_SIGN); \
    \
    if ( (id == LOG_NONUSE_ID) || (sec == LOG_NONUSE_BLOCK_TIME)) \
    {\
        length = snprintf(buf, sizeof(buf), "%c%s%s%c ", pre_sign, module_str, sync_flag, suf_sign); \
    }\
    else \
    {\
        id += (index << 16); \
        length = snprintf(buf, sizeof(buf), "%c%s-%d,%d%s%c ", pre_sign, module_str, id, sec, sync_flag, suf_sign); \
    }\
    \
    va_start(arg, fmt); \
    vsnprintf(buf+length, MAX_SIZE_PER_LOG-length, fmt, arg); \
    va_end(arg); \
    errno = 0;\
    syslog(pri, "%s", buf); \
    if(errno != 0)\
        syslog(pri,"%s", buf);\
}

DEF_FUN_LOG_MODULE(sys,     "System",       1)

DEF_FUN_LOG_MODULE(gpon,    "GPON",         2)
DEF_FUN_LOG_MODULE(eth,     "Ethernet",     3)
DEF_FUN_LOG_MODULE(wifi,    "WIFI",         4)
DEF_FUN_LOG_MODULE(usb,     "USB",          5)

DEF_FUN_LOG_MODULE(voip,    "VoIP",         6)
DEF_FUN_LOG_MODULE(iptv,    "IPTV",         7)
DEF_FUN_LOG_MODULE(dns,     "DNS",          8)
DEF_FUN_LOG_MODULE(ntp,     "NTP",          9)
DEF_FUN_LOG_MODULE(smb,     "Samba",        10)
DEF_FUN_LOG_MODULE(ftp,     "FTP",          11)
DEF_FUN_LOG_MODULE(ddns,    "DDNS",         12)
DEF_FUN_LOG_MODULE(umts,    "UMTS",         27)

DEF_FUN_LOG_MODULE(upnp,    "UPnP",         26)
DEF_FUN_LOG_MODULE(tr069,   "TR069",        13)
DEF_FUN_LOG_MODULE(omci,    "OMCI",         14)
DEF_FUN_LOG_MODULE(web,     "GUI",          15)
DEF_FUN_LOG_MODULE(ssh,     "SSH",          16)
DEF_FUN_LOG_MODULE(telnet,  "TELNET",       17)
DEF_FUN_LOG_MODULE(console, "CONSOLE",      18)

DEF_FUN_LOG_MODULE(pctrl,   "ParentControl",19)
DEF_FUN_LOG_MODULE(mfilter, "MacFilter",    20)
DEF_FUN_LOG_MODULE(ipfilter,"IPFilter",     21)
DEF_FUN_LOG_MODULE(wsch,    "WIFISchedule", 22)

DEF_FUN_LOG_MODULE(lan,     "LAN",          23)
DEF_FUN_LOG_MODULE(wan,     "WAN",          24)
DEF_FUN_LOG_MODULE(firewall, "Firewall",    25)
DEF_FUN_LOG_MODULE(safenetwork, "SafeNetwork",28)
DEF_FUN_LOG_MODULE(vpn,      "VPN",         26)
DEF_FUN_LOG_MODULE(fon,      "FON",         29)

