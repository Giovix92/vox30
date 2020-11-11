/* script.c
 *
 * Functions to call the DHCP client notification scripts 
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

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#ifdef __SC_BUILD__
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <linux/limits.h>
#include <log/slog.h>
#include <sal/sal_wan.h>
#include <cal/cal_wan.h>
#endif

#include "options.h"
#include "dhcpd.h"
#include "dhcpc.h"
#include "packet.h"
#include "options.h"
#include "debug.h"
#ifdef __SC_BUILD__
static int scIpChanged = 0;
static int scRouteChanged = 0;
static int scSRouteChanged = 0;
static int scDnsChanged = 0;
static int wan_down = 1;
#endif

/* get a rough idea of how long an option will be (rounding up...) */
static int max_option_length[] = {
	[OPTION_IP] =		sizeof("255.255.255.255 "),
	[OPTION_IP_PAIR] =	sizeof("255.255.255.255 ") * 2,
	[OPTION_STRING] =	1,
	[OPTION_BOOLEAN] =	sizeof("yes "),
	[OPTION_U8] =		sizeof("255 "),
	[OPTION_U16] =		sizeof("65535 "),
	[OPTION_S16] =		sizeof("-32768 "),
	[OPTION_U32] =		sizeof("4294967295 "),
	[OPTION_S32] =		sizeof("-2147483684 "),
#ifdef __SC_BUILD__
	[OPTION_ROUTE] =	sizeof("255/255.255.255.255/255.255.255.255 "),
#endif
};

static int upper_length(int length, struct dhcp_option *option)
{
	return max_option_length[option->flags & TYPE_MASK] *
	       (length / option_lengths[option->flags & TYPE_MASK]);
}


static int sprintip(char *dest, char *pre, unsigned char *ip) {
	return sprintf(dest, "%s%d.%d.%d.%d ", pre, ip[0], ip[1], ip[2], ip[3]);
}

/* Fill dest with the text of option 'option'. */
static void fill_options(char *dest, unsigned char *option, struct dhcp_option *type_p)
{
	int type, optlen;
	u_int16_t val_u16;
	int16_t val_s16;
	u_int32_t val_u32;
	int32_t val_s32;
	int len = option[OPT_LEN - 2];

	dest += sprintf(dest, "%s=", type_p->name);
    
	type = type_p->flags & TYPE_MASK;
	optlen = option_lengths[type];
	for(;;) {
		switch (type) {
		case OPTION_IP_PAIR:
			dest += sprintip(dest, "", option);
			*(dest++) = '/';
			option += 4;
			optlen = 4;
		case OPTION_IP:	/* Works regardless of host byte order. */

			dest += sprintip(dest, "", option);
 			break;
		case OPTION_BOOLEAN:
			dest += sprintf(dest, *option ? "yes " : "no ");
			break;
		case OPTION_U8:
			dest += sprintf(dest, "%u ", *option);
			break;
		case OPTION_U16:
			memcpy(&val_u16, option, 2);
			dest += sprintf(dest, "%u ", ntohs(val_u16));
			break;
		case OPTION_S16:
			memcpy(&val_s16, option, 2);
			dest += sprintf(dest, "%d ", ntohs(val_s16));
			break;
		case OPTION_U32:
			memcpy(&val_u32, option, 4);
			dest += sprintf(dest, "%lu ", (unsigned long) ntohl(val_u32));
			break;
		case OPTION_S32:
			memcpy(&val_s32, option, 4);
			dest += sprintf(dest, "%ld ", (long) ntohl(val_s32));
			break;
#ifdef __SC_BUILD__
		case OPTION_ROUTE:
			dest += sprintf(dest, "%u ", *option);
			*(dest++) = '/';
			dest += sprintip(dest, "", &option[1]);
			*(dest++) = '/';
			dest += sprintip(dest, "", &option[5]);
			break;
#endif
		case OPTION_STRING:
			memcpy(dest, option, len);
			dest[len] = '\0';
			return;	 /* Short circuit this case */
		}
		option += optlen;
		len -= optlen;
		if (len <= 0) break;
	}
}


static char *find_env(const char *prefix, char *defaultstr)
{
	extern char **environ;
	char **ptr;
	const int len = strlen(prefix);

	for (ptr = environ; *ptr != NULL; ptr++) {
		if (strncmp(prefix, *ptr, len) == 0)
			return *ptr;
	}
	return defaultstr;
}

/* put all the paramaters into an environment */
#ifdef __SC_BUILD__
static int _util_del_sub_str(char *head, char *sub)
{
    char *p;
    char *q;
    int len;
    p = strstr(head, sub);
    if(p)
    {
      q = p + strlen(sub);  
      len = strlen(q);
      if(len)
          strncpy(p, q, (len + 1));
      else
          *p = '\0';
      return 0;
    }
    return -1;
}
#define ADDON_OPTIONS 12   // if you add options, increase this const
#endif
static char **fill_envp(struct dhcpMessage *packet)
{
	int num_options = 0;
	int i, j;
	char **envp;
	unsigned char *temp;
	char over = 0;
	
	if (packet == NULL)
		num_options = 0;
	else {
		for (i = 0; options[i].code; i++)
			if (get_option(packet, options[i].code))
				num_options++;
		if (packet->siaddr) num_options++;
		if ((temp = get_option(packet, DHCP_OPTION_OVER)))
			over = *temp;
		if (!(over & FILE_FIELD) && packet->file[0]) num_options++;
		if (!(over & SNAME_FIELD) && packet->sname[0]) num_options++;		
	}
#ifdef __SC_BUILD__	
	envp = malloc((num_options + ADDON_OPTIONS) * sizeof(char *));
#else
	envp = malloc((num_options + 5) * sizeof(char *));
#endif
	envp[0] = malloc(sizeof("interface=") + strlen(client_config.interface));
	sprintf(envp[0], "interface=%s", client_config.interface);
	envp[1] = find_env("PATH", "PATH=/bin:/usr/bin:/sbin:/usr/sbin:/mnt/rootfs/bin");
	envp[2] = find_env("HOME", "HOME=/");
#ifdef __SC_BUILD__
	envp[3] = find_env("LD_LIBRARY_PATH", "LD_LIBRARY_PATH=/lib:/mnt/rootfs/lib");
	envp[4] = malloc(sizeof("wanid=") + sizeof(client_config.wanid));
	sprintf(envp[4], "wanid=%d", client_config.wanid); 
#endif
	if (packet == NULL) {
#ifdef __SC_BUILD__			
		envp[5] = NULL;
#else
		envp[3] = NULL;
#endif
		return envp;
	}
#ifdef __SC_BUILD__	
	envp[5] = malloc(sizeof("ip=255.255.255.255"));
	sprintip(envp[5], "ip=", (unsigned char *) &packet->yiaddr);
#else
	envp[3] = malloc(sizeof("ip=255.255.255.255"));
	sprint(envp[3], "ip=", (unsigned char *) &packet->yiaddr);
#endif	
#ifdef __SC_BUILD__	
	for (i = 0, j = 6; options[i].code; i++) {
#else
	for (i = 0, j = 4; options[i].code; i++) {
#endif
		if ((temp = get_option(packet, options[i].code))) {
			envp[j] = malloc(upper_length(temp[OPT_LEN - 2], &options[i]) + strlen(options[i].name) + 2);
			fill_options(envp[j], temp, &options[i]);
			j++;
		}
	}
	if (packet->siaddr) {
		envp[j] = malloc(sizeof("siaddr=255.255.255.255"));
		sprintip(envp[j++], "siaddr=", (unsigned char *) &packet->siaddr);
	}
	if (!(over & FILE_FIELD) && packet->file[0]) {
		/* watch out for invalid packets */
		packet->file[sizeof(packet->file) - 1] = '\0';
		envp[j] = malloc(sizeof("boot_file=") + strlen(packet->file));
		sprintf(envp[j++], "boot_file=%s", packet->file);
	}
	if (!(over & SNAME_FIELD) && packet->sname[0]) {
		/* watch out for invalid packets */
		packet->sname[sizeof(packet->sname) - 1] = '\0';
		envp[j] = malloc(sizeof("sname=") + strlen(packet->sname));
		sprintf(envp[j++], "sname=%s", packet->sname);
	}
#ifdef __SC_BUILD__		
    if (scIpChanged)
    {
        envp[j] = malloc(sizeof("IpChanged=1"));
		sprintf(envp[j++], "IpChanged=1");
    }
    if (scRouteChanged)
    {
        envp[j] = malloc(sizeof("RouteChanged=1"));
		sprintf(envp[j++], "RouteChanged=1");
    }
    else
    {
        if(scSRouteChanged)
        {
            envp[j] = malloc(sizeof("SRouteChanged=1"));
	        sprintf(envp[j++], "SRouteChanged=1");
        }
    }
    if (scDnsChanged)
    {
        envp[j] = malloc(sizeof("DnsChanged=1"));
	    sprintf(envp[j++], "DnsChanged=1");
    }
    if(wan_down)
    {
        envp[j] = malloc(sizeof("WANDown=1"));
	    sprintf(envp[j++], "WANDown=1");
    }
	envp[j++] = find_env("SERVER", "SERVER=");
#endif
	envp[j] = NULL;
	return envp;
}

#ifdef __SC_BUILD__
int get_sockfd(void)
{

	 int sockfd = -1;
	
		//if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			perror("user: socket creating failed");
			return (-1);
		}
	return sockfd;
}
#endif
#ifdef __SC_BUILD__

enum {
    OPT120ET_DOMAIN = 0,
    OPT120ET_IPADDR
};

static void sprintf_cname(const char *cname, int namesize, char *buf, int bufsize)
{
  const char *s = cname; /*source pointer */
  char *d = buf; /* destination pointer */

  if (cname == NULL) return;
    
  if ((strnlen(cname, namesize)+1) > (unsigned)bufsize) {
    if (bufsize > 11) {
      sprintf(buf, "(too long)");
    }
    else {
      buf[0] = 0;
    }
    return;
  }

  /* extract the pascal style strings */
  while (*s) {
    int i;
    int size = *s;

    /* Let us see if we are bypassing end of buffer.  Also remember
     * that we need space for an ending \0
     */
    if ((s + *s - cname) >= (bufsize)) {
      if (bufsize > 15 ) {
	sprintf(buf, "(malformatted)");
      } else {
	buf[0] = 0;
      }
      return;
    }

    /* delimit the labels with . */
    if (s++ != cname) sprintf(d++, ".");
   
    for(i = 0; i < size; i++) {
      *d++ = *s++;
    }
    *d=0;
  }
}

/* convert cname to ascii and return a static buffer */
static char *cname2asc(const char *cname) {
  static char buf[256];
  /* Note: we don't really check the size of the incomming cname. but
     according to RFC 1035 a name must not be bigger than 255 octets.
   */
  if (cname) 
    sprintf_cname(cname, sizeof(buf), buf, sizeof(buf));
  else
    strncpy(buf, "(default)", sizeof(buf));
  return buf;
}

static int dhcp_handle_option120(struct dhcpMessage *packet, char *output)
{
	unsigned char *pStr = NULL;
    unsigned char *temp = NULL;
    unsigned char *begin = NULL;
    char buf[256] = {0};
    char *cmd_line = output;
    SC_DEBUG(DEBUG_DHCPC, "Need check Option 120\n");
    if((pStr = get_option(packet, DHCP_SIP_SERVERS)) != NULL) {
        int opt_len = *(pStr - 1);
        SC_DEBUG(DEBUG_DHCPC, "enc_type = %d, length = %d\n", *pStr, opt_len);
        if(*pStr == OPT120ET_DOMAIN) {
            temp = begin = pStr + 1;
            opt_len -= 1;
            while(opt_len > 0) {
                if(*temp == 0) {
                    memset(buf, 0, sizeof(buf));
                    memcpy(buf, begin, (temp - begin));
                    if(begin == pStr + 1)
                        sprintf(cmd_line, "%s", cname2asc(buf));  
                    else
                        sprintf(cmd_line+strlen(cmd_line), ",%s", cname2asc(buf)); 

                    begin = temp + 1;
                }
                temp++;
                opt_len--;
            }
        }
        else if(*pStr == OPT120ET_IPADDR) {
            temp = pStr + 1;
            opt_len -= 1;

            SC_DEBUG(DEBUG_DHCPC, "length = %d, temp_addr = %p\n",  opt_len, temp);
            while(opt_len >= 4) {
                if(temp == (pStr + 1)) 
                    sprintf(cmd_line, "%u.%u.%u.%u", temp[0], temp[1], temp[2],temp[3]);  
                else
                    sprintf(cmd_line+strlen(cmd_line), ",%u.%u.%u.%u", temp[0], temp[1], temp[2],temp[3]); 

                SC_DEBUG(DEBUG_DHCPC, "cmd_line = %s\n",  cmd_line);
                temp += 4; 
                opt_len -= 4;
            }
        }
        else {
             log_wan(LOG_ERR, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "DHCP Option 120 Error: encoding type error on WAN%d\n", (client_config.wanid + 1));
            return -1;
        }
    }
    else {
        return -1;
    }

    return 0;
}

#define ONE_OPTION_MAX_LEN (255)
int dhcp_handle_option121(struct dhcpMessage *packet, char *output)
{
    int route_count = 0;
    unsigned char *temp_op;
    unsigned char *temp;
    int len = 0;
    int mask_bitn, dest_n;
    struct in_addr dest;
    struct in_addr netmask;
    struct in_addr gw;
    int i;
    unsigned char long_op_buf[512];
    int long_option_len;
    char sub_option121[256];
    int len_x;
    char routep[2048] = {'\0'};
    char *old_install = sal_wan_get_con_opt121_t(client_config.wanid);
    if(packet == NULL)
        return -1;

    if((temp_op = get_option(packet, DHCP_CLASSLESS_ROUTE)) != NULL)
    {
        /*If one option length > 255, server will separate overflow bytes
          to another option 250. Luckly, we just need handle option 32 here. Pay
          attention when you add a option support if it may larger than 255 bytes
          For ex. option 32 len 256 separate to opt 32 with len 255 and opt 250 with
          len 1*/

        if(temp_op[OPT_LEN - 2] == ONE_OPTION_MAX_LEN) /* Len is 255*/
        {
            if(temp_op[ONE_OPTION_MAX_LEN] == DHCP_LONG_OPTION) /*next is option 250*/
            {
                long_option_len = temp_op[ONE_OPTION_MAX_LEN + OPT_LEN];
                memcpy(long_op_buf, temp_op, ONE_OPTION_MAX_LEN);
                memcpy(long_op_buf + ONE_OPTION_MAX_LEN, temp_op + ONE_OPTION_MAX_LEN
                       + OPT_LEN + 1, long_option_len);
                len = ONE_OPTION_MAX_LEN + long_option_len;
                temp = long_op_buf;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            len = temp_op[OPT_LEN - 2];
            temp = temp_op;
        }
    }
    else
    {
        return -1;
    }

    /* We should only add 32 routes, so when route_count > 31, we should ingore */
    while(len > 0 && route_count < 32)
    {
        route_count++;
        mask_bitn = *(temp++);
        /* The netmask must not greater than 32 */
        /* Destionation net and netmask */
        if(mask_bitn > 32)
            break;
        dest_n = mask_bitn / 8;
        if(mask_bitn % 8)
            dest_n++;

        if((len = len - (1 + dest_n + 4)) < 0)
            break;

        /* netmak - default is 255.255.255.255 */
        netmask.s_addr = 0xFFFFFFFF;
        for(i = 0; i < (32 - mask_bitn); i++)
            netmask.s_addr = netmask.s_addr << 1;

        netmask.s_addr = htonl(netmask.s_addr);

        dest.s_addr = *((uint32_t *)temp);
        dest.s_addr = dest.s_addr & netmask.s_addr;
        temp += dest_n;

        /* Route gateway */
        gw.s_addr = *((uint32_t *)temp);
        temp += 4;

        /* Add route, metric = ? */
        len_x = snprintf(sub_option121, sizeof(sub_option121),"%s,", inet_ntoa(dest));
        if(len_x <=  0)
           break;
        len_x += snprintf((sub_option121 + len_x), (sizeof(sub_option121) - len_x),"%s,", inet_ntoa(netmask));
        if(len_x <= 0)
           break;
        len_x += snprintf((sub_option121 + len_x), (sizeof(sub_option121) - len_x),"%s;", inet_ntoa(gw));
        if(len_x <=  0)
           break;

        /* added route */
        if(strlen(output) == 0)
            sprintf(output, "%s", sub_option121);
        else
            sprintf(output+strlen(output), "%s", sub_option121);
        if(strlen(old_install))
        {
            if(_util_del_sub_str(old_install,  sub_option121) != 0)
            {
                if(strlen(routep) == 0)
                    sprintf(routep, "%s", sub_option121);
                else
                    sprintf(routep+strlen(routep), "%s", sub_option121);
            }
        }
        else
        {

        }
        SC_DEBUG(DEBUG_DHCPC, "%s\n", output);
    }
    if(strlen(old_install) || strlen(routep))
        scSRouteChanged = 1;
    sal_wan_set_con_opt121n_t(client_config.wanid, old_install);
    sal_wan_set_con_opt121p_t(client_config.wanid, routep);
    return 0;
}
int dhcp_handle_option249(struct dhcpMessage *packet, char *output)
{
    int route_count = 0;
    unsigned char *temp_op;
    unsigned char *temp;
    int len = 0;
    int mask_bitn, dest_n;
    struct in_addr dest;
    struct in_addr netmask;
    struct in_addr gw;
    int i;
    unsigned char long_op_buf[512];
    int long_option_len;
    char sub_option249[256];
    int len_x;
    char routep[2048] = {'\0'};
    char *old_install = sal_wan_get_con_opt249_t(client_config.wanid);
    if(packet == NULL)
        return -1;

    SC_DEBUG(DEBUG_DHCPC, "DHCPC ---- dhcp_handle_option249\n");
    if((temp_op = get_option(packet, DHCP_MS_CLASSLESS_ROUTE)) != NULL)
    {
        /*If one option length > 255, server will separate overflow bytes
          to another option 250. Luckly, we just need handle option 32 here. Pay
          attention when you add a option support if it may larger than 255 bytes
          For ex. option 32 len 256 separate to opt 32 with len 255 and opt 250 with
          len 1*/

        if(temp_op[OPT_LEN - 2] == ONE_OPTION_MAX_LEN) /* Len is 255*/
        {
            if(temp_op[ONE_OPTION_MAX_LEN] == DHCP_LONG_OPTION) /*next is option 250*/
            {
                long_option_len = temp_op[ONE_OPTION_MAX_LEN + OPT_LEN];
                memcpy(long_op_buf, temp_op, ONE_OPTION_MAX_LEN);
                memcpy(long_op_buf + ONE_OPTION_MAX_LEN, temp_op + ONE_OPTION_MAX_LEN
                       + OPT_LEN + 1, long_option_len);
                len = ONE_OPTION_MAX_LEN + long_option_len;
                temp = long_op_buf;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            len = temp_op[OPT_LEN - 2];
            temp = temp_op;
        }
    }
    else
    {
        return -1;
    }

    /* We should only add 32 routes, so when route_count > 31, we should ingore */
    while(len > 0 && route_count < 32)
    {
        route_count++;
        mask_bitn = *(temp++);
        /* The netmask must not greater than 32 */
        /* Destionation net and netmask */
        if(mask_bitn > 32)
            break;
        dest_n = mask_bitn / 8;
        if(mask_bitn % 8)
            dest_n++;

        if((len = len - (1 + dest_n + 4)) < 0)
            break;

        /* netmak - default is 255.255.255.255 */
        netmask.s_addr = 0xFFFFFFFF;
        for(i = 0; i < (32 - mask_bitn); i++)
            netmask.s_addr = netmask.s_addr << 1;

        netmask.s_addr = htonl(netmask.s_addr);

        dest.s_addr = *((uint32_t *)temp);
        dest.s_addr = dest.s_addr & netmask.s_addr;
        temp += dest_n;

        /* Route gateway */
        gw.s_addr = *((uint32_t *)temp);
        temp += 4;

        /* Add route, metric = ? */
        len_x = snprintf(sub_option249, sizeof(sub_option249),"%s,", inet_ntoa(dest));
        if(len_x <=  0)
           break;
        len_x += snprintf((sub_option249 + len_x), (sizeof(sub_option249) - len_x),"%s,", inet_ntoa(netmask));
        if(len_x <= 0)
           break;
        len_x += snprintf((sub_option249 + len_x), (sizeof(sub_option249) - len_x),"%s;", inet_ntoa(gw));
        if(len_x <=  0)
           break;

        /* added route */
        if(strlen(output) == 0)
            sprintf(output, "%s", sub_option249);
        else
            sprintf(output+strlen(output), "%s", sub_option249);
        if(strlen(old_install))
        {
            if(_util_del_sub_str(old_install,  sub_option249) != 0)
            {
                if(strlen(routep) == 0)
                    sprintf(routep, "%s", sub_option249);
                else
                    sprintf(routep+strlen(routep), "%s", sub_option249);
            }
        }
        else
        {

        }
        SC_DEBUG(DEBUG_DHCPC, "    output:%s\n", output);
    }
    if(strlen(old_install) || strlen(routep))
        scSRouteChanged = 1;
    sal_wan_set_con_opt249n_t(client_config.wanid, old_install);
    sal_wan_set_con_opt249p_t(client_config.wanid, routep);
    SC_DEBUG(DEBUG_DHCPC, "    old_install:%s\n", old_install);
    SC_DEBUG(DEBUG_DHCPC, "    routep:%s\n", routep);
    return 0;
}
int dhcp_handle_option33(struct dhcpMessage *packet, char *output)
{
    int route_count = 0;
    unsigned char *temp_op;
    unsigned char *temp;
    int len = 0;
    int mask_bitn, dest_n;
    struct in_addr dest;
    struct in_addr netmask;
    struct in_addr gw;
    int i;
    char cmdbuf[256];
    unsigned char long_op_buf[512];
    int long_option_len;
    char sub_option33[256];
    int len_x;
    char routep[2048] = {'\0'};
    char *old_install = sal_wan_get_con_opt33_t(client_config.wanid);
    if(packet == NULL)
        return -1;

    SC_DEBUG(DEBUG_DHCPC, "DHCPC ---- dhcp_handle_option33\n");
    if((temp_op = get_option(packet, DHCP_STATIC_ROUTE)) != NULL)
    {
        /*If one option length > 255, server will separate overflow bytes
          to another option 250. Luckly, we just need handle option 32 here. Pay
          attention when you add a option support if it may larger than 255 bytes
          For ex. option 32 len 256 separate to opt 32 with len 255 and opt 250 with
          len 1*/

        if(temp_op[OPT_LEN - 2] == ONE_OPTION_MAX_LEN) /* Len is 255*/
        {
            if(temp_op[ONE_OPTION_MAX_LEN] == DHCP_LONG_OPTION) /*next is option 250*/
            {
                long_option_len = temp_op[ONE_OPTION_MAX_LEN + OPT_LEN];
                memcpy(long_op_buf, temp_op, ONE_OPTION_MAX_LEN);
                memcpy(long_op_buf + ONE_OPTION_MAX_LEN, temp_op + ONE_OPTION_MAX_LEN
                       + OPT_LEN + 1, long_option_len);
                len = ONE_OPTION_MAX_LEN + long_option_len;
                temp = long_op_buf;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            len = temp_op[OPT_LEN - 2];
            temp = temp_op;
        }
    }
    else
    {
        return -1;
    }

    /* We should only add 32 routes, so when route_count > 31, we should ingore */
    while(len > 0 && route_count < 32)
    {
        if((len = len - 8) < 0)
            break;

        dest.s_addr = *((uint32_t *)temp);
        if (dest.s_addr == 0)
            continue;
        route_count++;
    #if 0
        if ((*temp >> 7) == 0x00) // A class, leading bits: 0, mask length: 8
            netmask.s_addr = 0xFF000000;
        else if ((*temp >> 6) == 0x02) // B class, leading bits: 10, mask length: 16
            netmask.s_addr = 0xFFFF0000;
        else if ((*temp >> 5) == 0x06) // C class, leading bits: 110, mask length: 24
            netmask.s_addr = 0xFFFFFF00;
    #else
        netmask.s_addr = 0xFFFFFFFF;
        if (*(temp+3) == 0)
        {
            netmask.s_addr &= 0xFFFFFF00;
            if (*(temp+2) == 0)
            {
                netmask.s_addr &= 0xFFFF0000;
                if (*(temp+1) == 0)
                {
                    netmask.s_addr &= 0xFF000000;
                }
            }
        }
    #endif
        netmask.s_addr = htonl(netmask.s_addr);

        /* Route gateway */
        temp += 4;
        gw.s_addr = *((uint32_t *)temp);
        temp += 4;

        /* Add route, metric = ? */
        len_x = snprintf(sub_option33, sizeof(sub_option33),"%s,", inet_ntoa(dest));
        if(len_x <=  0)
           break;
        len_x += snprintf((sub_option33 + len_x), (sizeof(sub_option33) - len_x),"%s,", inet_ntoa(netmask));
        if(len_x <= 0)
           break;
        len_x += snprintf((sub_option33 + len_x), (sizeof(sub_option33) - len_x),"%s;", inet_ntoa(gw));
        if(len_x <=  0)
           break;

        /* added route */
        if(strlen(output) == 0)
            sprintf(output, "%s", sub_option33);
        else
            sprintf(output+strlen(output), "%s", sub_option33);
        if(strlen(old_install))
        {
            if(_util_del_sub_str(old_install,  sub_option33) != 0)
            {
                if(strlen(routep) == 0)
                    sprintf(routep, "%s", sub_option33);
                else
                    sprintf(routep+strlen(routep), "%s", sub_option33);
            }
        }
        else
        {

        }
        SC_DEBUG(DEBUG_DHCPC, "    output:%s\n", output);
    }
    if(strlen(old_install) || strlen(routep))
        scSRouteChanged = 1;
    sal_wan_set_con_opt33n_t(client_config.wanid, old_install);
    sal_wan_set_con_opt33p_t(client_config.wanid, routep);
    SC_DEBUG(DEBUG_DHCPC, "    old_install:%s\n", old_install);
    SC_DEBUG(DEBUG_DHCPC, "    routep:%s\n", routep);
    return 0;
}
/*
* 1) Check ip/gateway/dns changed or not
* 2) record the connection info
*/
#ifdef CONFIG_SUPPORT_CGN || CONFIG_SUPPORT_FWA
#define DHCP_ACS_URL       0x42
#define DHCP_LINK_ID       0x43
#define DHCP_IMSI          0x06
#define DHCP_MSISDN        0x07
unsigned char *get_sub_option(unsigned char *optionptr, int whole_len, int code, int *len)
{
	int i, length;
    int done = 0;
	i = 0;
	length = whole_len;
        *len = 0;
	while (!done) {
		if (i >= length) {
			LOG(LOG_WARNING, "sub option %d not found", code);
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, "bogus packet, sub option fields too long.");
				return NULL;
			}
			*len = *(optionptr + i + 1);
			return optionptr + i + 2;
		}
		switch (optionptr[i + OPT_CODE]) {
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}
#endif
static void
sc_ipGetHandler(struct dhcpMessage *packet, const char *name)
{
	unsigned char *pStr;
	unsigned long ipLong;
	int wanid = client_config.wanid;
	int i;
	int lease;
    struct sysinfo sys_info;
	char str_lease[32];
    char opt43_buf[512];
#ifdef CONFIG_SUPPORT_CGN
    char url_buf[512] = {0};
    char linkid_buf[512] = {0};
#endif
#ifdef CONFIG_SUPPORT_FWA
    char imsi_buf[512] = {0};
    char msisdn_buf[512] = {0};
#endif
    char opt120_buf[256];
    char opt121_buf[2048];
    char opt249_buf[2048];
    char opt33_buf[2048];
	WAN_CP_INFO_t cp_info;
	WAN_CP_INFO_t cp_info_n;
    int length = 0;
    scIpChanged = scRouteChanged = scSRouteChanged = scDnsChanged = 0;
	memset(&cp_info_n, 0, sizeof(cp_info_n));

	sal_wan_load_cp_info(wanid, &cp_info);

	if(cp_info.state && strcmp(cp_info.state, CAL_ENABLE) == 0)
		wan_down = 0;
	else
		wan_down = 1;
	/*
	* Check If IP changed
	*/
	cp_info_n.ip.s_addr = packet->yiaddr;
        if(wan_down || cp_info_n.ip.s_addr != cp_info.ip.s_addr)
        {
            scIpChanged = 1;
            scRouteChanged = 1;
        }

    /*
	* Check If IP subnet changed
	*/
	if ((pStr = get_option(packet, DHCP_SUBNET)) != NULL)
	{
	    memcpy(&ipLong, pStr, 4);
	    cp_info_n.ipmask.s_addr=ipLong;
        }else{
    	    cp_info_n.ipmask.s_addr=0;
        }
	if(wan_down || cp_info_n.ipmask.s_addr != cp_info.ipmask.s_addr)
	{
	    scIpChanged = 1;
	    scRouteChanged = 1;
	}

	/*
	* Check If Gateway changed
	*/
	if ((pStr = get_option(packet, DHCP_ROUTER)) != NULL) {
		memcpy(&ipLong, pStr, 4);
		cp_info_n.gw.s_addr=ipLong;
	}else{
		cp_info_n.gw.s_addr=0;
    }
    if(wan_down || cp_info_n.gw.s_addr != cp_info.gw.s_addr)
    {
	    scRouteChanged = 1;

    }

    /*
	* Check If DNS changed
	*/
	if ((pStr = get_option(packet, DHCP_DNS_SERVER)) != NULL) {
		memcpy(&ipLong, pStr, 4);
		cp_info_n.dns1.s_addr=ipLong;

		if(*(pStr - 1) >= 8)
		{
    	    memcpy(&ipLong, pStr+4, 4);
    	    cp_info_n.dns2.s_addr=ipLong;
    	}else{
    		cp_info_n.dns2.s_addr=0;
    	}

	}else{
		cp_info_n.dns2.s_addr = cp_info_n.dns1.s_addr = 0;
    }
	if(cp_info_n.dns2.s_addr != cp_info.dns2.s_addr || cp_info_n.dns1.s_addr != cp_info.dns1.s_addr)
	{
	    scDnsChanged = 1;
            scRouteChanged = 1;
	}
	if (!(pStr = get_option(packet, DHCP_LEASE_TIME))) {
		sprintf(str_lease, "%d", 60 * 60);
	} else {
		memcpy(&(lease), pStr, 4);
		lease = ntohl(lease);
		sprintf(str_lease, "%d", lease);
	}
	cp_info_n.lease_time = str_lease;
	if(is_option_required(DHCP_VENDOR_INFO))
	{
        memset(opt43_buf, 0, sizeof(opt43_buf));
	    if ((pStr = get_option_x(packet, DHCP_VENDOR_INFO, &length)) != NULL) {
            int len = 0;

            len = *(pStr - 1) + 1;
            if(len >= sizeof(opt43_buf))
                len = sizeof(opt43_buf);
            if(len)
                snprintf(opt43_buf, len, "%s", pStr);
#ifdef CONFIG_SUPPORT_CGN
            if (pStr = get_sub_option(opt43_buf, length, DHCP_ACS_URL, &len))
            {
                if(len < sizeof(url_buf)-1)
                {
					memcpy(url_buf, pStr, len);
                    url_buf[len] = '\0';
                    sal_wan_set_con_hurl_t(wanid, url_buf);
                    cp_info_n.opt43 = url_buf;
                }
                if (pStr = get_sub_option(opt43_buf, length, DHCP_LINK_ID, &len))
                {
                    if(len < sizeof(linkid_buf)-1)
                    {
                        memcpy(linkid_buf, pStr, len);
                        linkid_buf[len] = '\0';
                        sal_wan_set_con_provcode_t(wanid, linkid_buf);
                    }
                }
            }
            else
                cp_info_n.opt43 = opt43_buf;
#else
    		cp_info_n.opt43 = opt43_buf;
#endif
#ifdef CONFIG_SUPPORT_FWA
        if(wanid == FWA_DEFAULT_VOICE_WANID || wanid == FWA_DEFAULT_DATA_WANID)
        {
            if (pStr = get_sub_option(opt43_buf, length, DHCP_IMSI, &len))
            {
                if(len < sizeof(imsi_buf)-1)
                {
                    memcpy(imsi_buf, pStr, len);
                    imsi_buf[len] = '\0';
                    sal_wan_set_fwa_imsi(wanid,imsi_buf);
                }
            }
            if (pStr = get_sub_option(opt43_buf, length, DHCP_MSISDN, &len))
            {
                if(len < sizeof(msisdn_buf)-1)
                {
                    memcpy(msisdn_buf, pStr, len);
                    msisdn_buf[len] = '\0';
                }
            }
        }
#endif
	    }else{
	    }
    }
    if(is_option_required(DHCP_SIP_SERVERS))
    {
        memset(opt120_buf, 0, sizeof(opt120_buf));
        if(dhcp_handle_option120(packet, opt120_buf) == 0)
            cp_info_n.opt120 = opt120_buf;       
    }
    if(is_option_required(DHCP_CLASSLESS_ROUTE))
    {
        memset(opt121_buf, 0, sizeof(opt121_buf));
        if(dhcp_handle_option121(packet, opt121_buf) == 0)
            cp_info_n.opt121 = opt121_buf;       
    }
    if(is_option_required(DHCP_MS_CLASSLESS_ROUTE))
    {
        memset(opt249_buf, 0, sizeof(opt249_buf));
        if(dhcp_handle_option249(packet, opt249_buf) == 0)
            cp_info_n.opt249 = opt249_buf;
    }
    if(is_option_required(DHCP_STATIC_ROUTE))
    {
        memset(opt33_buf, 0, sizeof(opt33_buf));
        if(dhcp_handle_option33(packet, opt33_buf) == 0)
            cp_info_n.opt33 = opt33_buf;
    }
    char uptime[64];
    if(strcmp(name, "renew") == 0)
    {
        snprintf(uptime, sizeof(uptime), "%s", cp_info.uptime);
    }
    else
    {
        sysinfo(&sys_info);
        snprintf(uptime, sizeof(uptime), "%ld", sys_info.uptime);
    }
    cp_info_n.uptime = uptime;
    cp_info_n.state = CAL_ENABLE;
    sal_wan_store_cp_info_x(wanid, &cp_info_n);

}
#endif
/* Call a script with a par file and env vars */
void run_script(struct dhcpMessage *packet, const char *name)
{
	int pid;
	char **envp;
#ifdef __SC_BUILD__
    struct sysinfo Info;
    int up_time = 0;
    int past_time = 0;
    char time_tmp[256];
#endif
    
	if (client_config.script == NULL)
		return;

#ifdef __SC_BUILD__
		if(name)
		{
			if((strcmp(name, "renew") == 0)
					|| (strcmp(name, "bound") == 0))
			{
				sc_ipGetHandler(packet, name);
			}
			else if(strcmp(name, "deconfig") == 0)
			{
				if(0 == strcmp(sal_wan_get_con_state_t(client_config.wanid), CAL_ENABLE))
				{
                    up_time = atoi(sal_wan_get_con_uptime_t(client_config.wanid));
                    past_time = atoi(sal_wan_get_con_pasttime_t(client_config.wanid));
                    sysinfo(&Info);
                    snprintf(time_tmp, sizeof(time_tmp), "%d", (past_time+Info.uptime-up_time));
                    sal_wan_set_con_pasttime_t(client_config.wanid, time_tmp);
					
                    sal_wan_set_con_state_t(client_config.wanid, CAL_DISABLE);
					sal_wan_set_con_uptime_t(client_config.wanid, "");
				}
			}
		}
#endif
	/* call script */
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
		return;
	} else if (pid == 0) {
		envp = fill_envp(packet);

		/* close fd's? */
		/* exec script */
		DEBUG(LOG_INFO, "execle'ing %s", client_config.script);
		execle(client_config.script, client_config.script,
		       name, NULL, envp);
		LOG(LOG_ERR, "script %s failed: errno %d",
		    client_config.script, errno);
		exit(1);
	}			
}
