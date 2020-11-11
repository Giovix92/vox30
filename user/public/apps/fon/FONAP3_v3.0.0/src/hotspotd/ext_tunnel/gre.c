
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <byteswap.h>
#include <netdb.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef __SC_BUILD__
#include <log/slog.h>
#endif
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/types.h>
#include <linux/if_pppox.h>
#include <sys/types.h>

#include "core/hotspotd.h"
#include "lib/list.h"
#include "lib/event.h"
#include "lib/config.h"
#include "lib/urandom.h"
#include "gre.h"
#include "sal/sal_misc.h"
extern int nvram_bcm_set(const char* name,const char* value);
extern char *nvram_bcm_safe_get(const char* name);

#ifdef __UCLIBC__
#include <features.h>
#if __UCLIBC_MAJOR__ == 0 && __UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ <= 29
#define res_init __res_init
extern int __res_init();
#endif
#endif

#ifdef __SC_BUILD__
#define SIOCGIFFRAGMENTFLAGS 0x894D /* get fragment flags*/
#define SIOCSIFFRAGMENTFLAGS 0x894E /* set fragment flags*/

#endif


int gre_shutdown(struct gre_lac *lac);

static void gre_timer(struct event_timer *timer, int64_t now);
static inline void gre_notify(struct gre_lac *lac, enum gre_state state) 
{
	lac->cfg->cb(state, lac);
}
#define WAN_CFG_BASE                "/tmp/0/"
#define IPPING_DIAG_CMD "/usr/sbin/ipping_diag"
static char* get_ipping_cmd(int wan_id, int tunnel_id, int is_backup)
{
    static char cmd[64] = {0};
    snprintf(cmd, sizeof(cmd),"%s/%d/%s_%d_%d_%d", WAN_CFG_BASE, wan_id, "ipping_diag", wan_id, tunnel_id, is_backup);
    if(access(cmd ,F_OK) != 0)
    {
        symlink(IPPING_DIAG_CMD, cmd);
    }
    return cmd;
}
#define DHCPRELAY_CMD "/usr/sbin/dhcprelay"
static char* get_relay_cmd(int wan_id, int tunnel_id, char* interface)
{
    static char cmd[64] = {0};
    snprintf(cmd, sizeof(cmd),"%s/%d/%s_%d_%d_%s", WAN_CFG_BASE, wan_id, "dhcprelay", wan_id, tunnel_id, interface);
    if(access(cmd ,F_OK) != 0)
    {
        symlink(DHCPRELAY_CMD, cmd);
    }
    return cmd;
}
int SYSTEM(const char *format, ...)
{
    char buf[1024]="";
    va_list arg;
    va_start(arg, format);
    vsnprintf(buf,1024, format, arg);
    va_end(arg);
    system(buf);
    usleep(1);
    return 0;
}

static FILE *fp_shell = NULL;
static FILE *fp = NULL;
int SYSTEM_X(const char *format, ...)
{

    char buf[1024]="";
    va_list arg;
    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);
    if(fp_shell != NULL)
    {
        fwrite(buf, strlen(buf), 1,fp_shell);
        fwrite("\n", 1, 1, fp_shell);
    }
    else
    {
        SYSTEM(buf);
    }
    return 0;
}
int SYSTEM_Y(const char *format, ...)
{

    char buf[1024]="";
    va_list arg;
    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);
    if(fp != NULL)
    {
        fwrite(buf, strlen(buf), 1,fp);
        fwrite("\n", 1, 1, fp);
    }
    return 0;
}
typedef struct if_adv_info_s{
    char ifname[16];
    unsigned long int tx_bytes;
    unsigned long int rx_bytes;
    unsigned long int tx_packets;
    unsigned long int rx_packets;
    unsigned long int tx_drops;
    unsigned long int rx_drops;
    unsigned long int tx_errors;
    unsigned long int rx_errors;
    unsigned long int rx_crc_errors;
    unsigned long int tx_crc_errors;
    unsigned long int tx_multicast_packets;
    unsigned long int rx_multicast_packets;
    unsigned long int tx_multicast_bytes;
    unsigned long int rx_multicast_bytes;
    unsigned long int tx_broadcast_packets;
    unsigned long int rx_broadcast_packets;
    unsigned long int tx_unicast_packets;
    unsigned long int rx_unicast_packets;
    unsigned long int rx_unknownproto_packets;
}if_adv_info_t;

static char is_space (unsigned char c)
{
    if ( c == ' '
            || c == '\f'
            || c == '\n'
            || c == '\r'
            || c == '\t'
            || c == '\v' )
        return 1;

    return 0;
}
int getIFAdvInfo(char *if_name, if_adv_info_t *if_info)
{
    FILE *fh;
    char buf[1024];
    struct net_device_info stats;

    fh = fopen("/proc/net/dev", "r");
    if (!fh) {
        return -1;
    }

    fgets(buf, sizeof(buf), fh);    /* eat line */
    fgets(buf, sizeof(buf), fh);
    fgets(buf, sizeof(buf), fh);

    while (fgets(buf, sizeof(buf), fh)) {
        char name[256];
        int i = 0;
        char *s = buf;
        buf[sizeof(buf)-1]=0;
        while(i < sizeof(name) && *s != ':')
        {
            if(!is_space(*s))
                name[i++] = *s;
            s++;
        }
        name[i] = 0;
        if(strcmp(if_name, name) == 0)
        {
            memset(&stats, 0, sizeof(struct net_device_info));
            sscanf(++s, "%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld",
                    &stats.rx_bytes, /* missing for 0 */
                    &stats.rx_packets,
                    &stats.rx_errors,
                    &stats.rx_dropped,
                    &stats.rx_fifo_errors,
                    &stats.rx_frame_errors,
                    &stats.rx_compressed, /* missing for <= 1 */
                    &stats.rx_multicast, /* missing for <= 1 */
                    &stats.tx_bytes, /* missing for 0 */
                    &stats.tx_packets,
                    &stats.tx_errors,
                    &stats.tx_dropped,
                    &stats.tx_fifo_errors,
                    &stats.collisions,
                    &stats.tx_carrier_errors,
                    &stats.tx_compressed, /* missing for <= 1 */
                    &stats.rx_multicast_packets,
                    &stats.rx_multicast_bytes,
                    &stats.tx_multicast_bytes,
                    &stats.rx_unicast_packets,
                    &stats.tx_unicast_packets,
                    &stats.rx_broadcast_packets,
                    &stats.tx_broadcast_packets,
                    &stats.rx_unknownproto_packets
                        );

            strcpy(if_info->ifname, name);
            if_info->rx_packets = stats.rx_packets;
            if_info->tx_packets = stats.tx_packets;
            if_info->rx_bytes = stats.rx_bytes;
            if_info->tx_bytes = stats.tx_bytes;
            if_info->rx_errors = stats.rx_errors;
            if_info->rx_crc_errors = stats.rx_errors;
            if_info->tx_errors = stats.tx_errors;
            if_info->tx_crc_errors = stats.tx_errors;
            if_info->rx_drops = stats.rx_dropped;
            if_info->tx_errors = stats.tx_errors;
            if_info->tx_drops = stats.tx_dropped;
            if_info->rx_multicast_packets = stats.rx_multicast_packets;
            if_info->rx_multicast_bytes = stats.rx_multicast_bytes;
            if_info->tx_multicast_bytes = stats.tx_multicast_bytes;
            if_info->rx_unicast_packets = stats.rx_unicast_packets;
            if_info->tx_unicast_packets = stats.tx_unicast_packets;
            if_info->rx_broadcast_packets = stats.rx_broadcast_packets;
            if_info->tx_broadcast_packets = stats.tx_broadcast_packets;
            if_info->rx_unknownproto_packets = stats.rx_unknownproto_packets;
            fclose(fh);
            return 0;
        }
    }/* end while */
    fclose(fh);
    return 1;
}

static int get_sockfd()
{
	int sockfd = -1;
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		return (-1);
	}
	return sockfd;
}
static int getIFIPaddress(char *if_name, char *ipaddr)
{
	struct ifreq ifr;
	struct sockaddr_in *saddr;
	int fd;
	int ret=0;

	if ((fd=get_sockfd())>=0)
	{
		strcpy(ifr.ifr_name, if_name);
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(fd, SIOCGIFADDR, &ifr)==0){
            saddr = (struct sockaddr_in *)&ifr.ifr_addr;
            strcpy(ipaddr,(char *)inet_ntoa(saddr->sin_addr));
		}else
			ret=-1;
		close(fd);
		return ret;
	}
	return -1;
}
int getIFMTU(char *if_name)
{
	struct ifreq ifr;
	int fd;
	int ret=0;

	if ((fd=get_sockfd())>=0)
	{
		strcpy(ifr.ifr_name, if_name);
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(fd, SIOCGIFMTU, &ifr)==0){
            close(fd);
            return ifr.ifr_mtu;
		}else
			ret=-1;
		close(fd);
		return ret;
	}
	return -1;
}

int getIFMac(char *if_name, char* mac)
{
    int ret = -1;
    int fd ;
    struct ifreq ifr;
    if((fd=get_sockfd())>=0)
    {
        strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);
        ifr.ifr_addr.sa_family = AF_INET;
        if (ioctl(fd, SIOCGIFHWADDR, &ifr)==0)
        {
            memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
            ret = 0;
        }
        close(fd);
    }
    return ret;
}
char* get_setup_script(int wan_id, int tunnel_id, int is_backup)
{
    static char script_up[64] = {0};
    snprintf(script_up,sizeof(script_up),"/tmp/0/%d/gre%d_%d_up",wan_id, tunnel_id, is_backup);
    return script_up;
}
static char* get_shutdown_script(int wan_id, int tunnel_id, int is_backup)
{
    static char script_down[64] = {0};
    snprintf(script_down,sizeof(script_down),"/tmp/0/%d/gre%d_%d_down",wan_id, tunnel_id, is_backup);
    return script_down;
}
static int get_max_mss(char* name)
{
    int mtu = 1492;
    mtu = getIFMTU(name);
    if(mtu < (1390+40))
        return mtu - 44;  
    else
        return 1390;
}
int gre_setup(struct gre_lac *lac)
{
    int ret = -1;
    struct VLAN vlan_buf[4096] = {{0}};
    int mss = 1390;
    unsigned char mac[6] = {0};
    char lan_ifname[64] = {0};
    char lan_ifnames[128] = {0};
    char br_name[64] = {0};
    int main_wifi_enable = 0;
#ifdef CONFIG_SUPPORT_WIFI_5G
    char* ptr = NULL;
    int main_5g_wifi_enable = 0;
#endif
    if(!lac)
        return ret;
    int bridge_id = 0;
    char ip[64] = {0};
    const char* status = NULL;
    memset(vlan_buf,0,sizeof(vlan_buf));
    fp_shell = fopen(get_setup_script(lac->wan_id, lac->tunnel_id, lac->is_backup), "w");
    fp = fopen(get_shutdown_script(lac->wan_id, lac->tunnel_id,lac->is_backup), "w");
    if(!fp || !fp_shell)
    {
        if(fp)
            fclose(fp);
        if(fp_shell)
            fclose(fp_shell);
        fp = NULL;
        fp_shell = NULL;
        return ret;
    }
    char *service_id = (char*)config_get_string("main", "service_id", "");
	main_wifi_enable = config_get_int("main","main_wifi_enable",0); 
#ifdef CONFIG_SUPPORT_WIFI_5G
	main_5g_wifi_enable = config_get_int("main","main_5g_wifi_enable",0);
#endif
    /* currently just support ipv4 gre tunnel setup*/
    snprintf(lac->server_ip, sizeof(lac->server_ip),"%s",inet_ntoa(((struct sockaddr_in *)(&(lac->addr.saddr)))->sin_addr));
    snprintf(lac->cmd, sizeof(lac->cmd),"%s", get_ipping_cmd(lac->wan_id, lac->tunnel_id, lac->is_backup));
    snprintf(lac->wan_if, sizeof(lac->wan_if),"%s", config_get_string("main","wan_iface",""));
    if(getIFIPaddress(lac->wan_if,ip) < 0)
    {
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "get wan interface %s ipaddr failed\n",
                lac->wan_if);
    }
    SYSTEM_X("ip link add name gre%d type gretap remote %s local %s ttl 64 nopmtudisc",lac->tunnel_id +1, lac->server_ip, ip);
    SYSTEM_X("ip link set gre%d mtu 1500",lac->tunnel_id +1);
    SYSTEM_X("ip link set gre%d up", lac->tunnel_id+1);
#ifdef CONFIG_SUPPORT_DSL
	int comparision_wan_type = config_get_int("main","cmpresult_wan_type_ptm",0); 
	if (comparision_wan_type == 1)
	{
		int wan_bw = config_get_int("main","wan_ubandwidth",-1);
		int maxp_bw = config_get_int("main", "max_percent",-1);
		int max_bw;
		if(maxp_bw)
			 max_bw = wan_bw * maxp_bw /100; 
		else
			 max_bw = config_get_int("main","max_egress",-1); 
    SYSTEM_X("tc qdisc add dev gre1 root tbf rate %dkbit latency 50ms burst 1540", max_bw);
	}
#endif
    snprintf(lac->gre_if,sizeof(lac->gre_if),"gre%d",lac->tunnel_id+1);
    mss = get_max_mss(lac->wan_if);
    struct SSID* p;
    bridge_id = lac->tunnel_id * 2 + 1;
    list_for_each_entry(p, &(lac->cfg->ssid), _head)
    {
#ifdef CONFIG_SUPPORT_WIFI_5G
        if(p->is_5g)
        {
            if(!main_5g_wifi_enable)
                continue;
        }
        else
#endif
        {
            if(!main_wifi_enable)
                continue;
        }
        if(p->is_eap)
        {
#ifdef CONFIG_SUPPORT_WIFI_5G
            if(p->is_5g)
            {
                status = config_get_string("eapradserv", "eapstatus_5g", NULL);
                if(status && (strcmp(status,"off") == 0))
                    continue;
            }
            else
#endif
            {
                status = config_get_string("eapradserv", "eapstatus", NULL);
                if(status && (strcmp(status,"off") == 0))
                    continue;
            }
        }
        if(p->vlan < 0)
            p->vlan = 0;
        if(p->vlan >= 0)
        {
            if(vlan_buf[p->vlan].vlan_buf == 0)
            {
                bridge_id ++;
                vlan_buf[p->vlan].bridge_id = bridge_id;
                SYSTEM_X("brctl addbr br%d", bridge_id);
                SYSTEM_X("ip link set br%d up", bridge_id);
                SYSTEM_Y("ifconfig  br%d down", bridge_id);
                SYSTEM_Y("brctl delbr br%d", bridge_id);
                snprintf(lan_ifnames, sizeof(lan_ifnames), "lan%d_ifname", vlan_buf[p->vlan].bridge_id);
                nvram_bcm_set(lan_ifnames,"");
                snprintf(lan_ifnames, sizeof(lan_ifnames), "lan%d_ifnames", vlan_buf[p->vlan].bridge_id);
                nvram_bcm_set(lan_ifnames,"");
            }
            snprintf(p->bridge,sizeof(p->bridge),"br%d",vlan_buf[p->vlan].bridge_id);
            if(p->vlan > 0)
            {
                if(vlan_buf[p->vlan].vlan_buf == 0)
                {
                    SYSTEM_X("vconfig add gre%d %d", lac->tunnel_id+1, p->vlan);
                    getIFMac(p->interface,(char *) mac);
                    SYSTEM_X("ifconfig gre%d.%d hw ether %02x%02x%02x%02x%02x%02x", lac->tunnel_id+1, p->vlan,
                            mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
                    SYSTEM_X("ip link set gre%d.%d up", lac->tunnel_id+1, p->vlan);
                    SYSTEM_X("brctl addif br%d gre%d.%d",vlan_buf[p->vlan].bridge_id, lac->tunnel_id+1 , p->vlan);
                }
                SYSTEM_X("brctl addif br%d %s",vlan_buf[p->vlan].bridge_id, p->interface);
                if(service_id && strlen(service_id))
                {
                    SYSTEM_X("%s -i %s -o gre%d.%d -B br%d -s %s -S %s &", get_relay_cmd(lac->wan_id, lac->tunnel_id, p->interface), p->interface, lac->tunnel_id+1 , p->vlan, vlan_buf[p->vlan].bridge_id, p->name, service_id);
                    SYSTEM_Y("killall -SIGTERM %s_%d_%d_%s", "dhcprelay", lac->wan_id, lac->tunnel_id, p->interface);
                }
                else
                {
                    SYSTEM_X("%s -i %s -o gre%d.%d -B br%d -s %s &",get_relay_cmd(lac->wan_id, lac->tunnel_id, p->interface), p->interface, lac->tunnel_id+1 , p->vlan, vlan_buf[p->vlan].bridge_id, p->name);
                    SYSTEM_Y("killall -SIGTERM %s_%d_%d_%s", "dhcprelay", lac->wan_id, lac->tunnel_id, p->interface);
                }
                if(vlan_buf[p->vlan].vlan_buf == 0)
                    SYSTEM_Y("ip link del gre%d.%d", lac->tunnel_id+1, p->vlan);
            }
            else
            {
                if(vlan_buf[p->vlan].vlan_buf == 0)
                {
                    snprintf(lan_ifnames, sizeof(lan_ifnames), "lan%d_ifname", vlan_buf[p->vlan].bridge_id);
                    nvram_bcm_set(lan_ifnames,"");
                    snprintf(lan_ifnames, sizeof(lan_ifnames), "lan%d_ifnames", vlan_buf[p->vlan].bridge_id);
                    nvram_bcm_set(lan_ifnames,"");
                    SYSTEM_X("brctl addif br%d gre%d",vlan_buf[p->vlan].bridge_id, lac->tunnel_id+1);
                }
                SYSTEM_X("brctl addif br%d %s",vlan_buf[p->vlan].bridge_id, p->interface);
                if(service_id && strlen(service_id))
                {
                    SYSTEM_X("%s -i %s -o gre%d -B br%d -s %s -S %s &", get_relay_cmd(lac->wan_id, lac->tunnel_id, p->interface), p->interface, lac->tunnel_id+1 , vlan_buf[p->vlan].bridge_id, p->name, service_id);
                    SYSTEM_Y("killall -SIGTERM %s_%d_%d_%s", "dhcprelay", lac->wan_id, lac->tunnel_id, p->interface);
                }
                else
                {
                    SYSTEM_X("%s -i %s -o gre%d -B br%d -s %s &", get_relay_cmd(lac->wan_id, lac->tunnel_id, p->interface), p->interface, lac->tunnel_id+1 , vlan_buf[p->vlan].bridge_id, p->name);
                    SYSTEM_Y("killall -SIGTERM %s_%d_%d_%s", "dhcprelay", lac->wan_id, lac->tunnel_id, p->interface);
                }
                if(vlan_buf[p->vlan].vlan_buf == 0)
                {
                    SYSTEM_Y("ip link del gre%d", lac->tunnel_id+1);
                }
            }
            if(p->is_eap)
            {
                snprintf(br_name, sizeof(br_name), "br%d", vlan_buf[p->vlan].bridge_id);
                snprintf(lan_ifname, sizeof(lan_ifname), "%s ", p->interface);
                snprintf(lan_ifnames, sizeof(lan_ifnames), "lan%d_ifname", vlan_buf[p->vlan].bridge_id);
                nvram_bcm_set(lan_ifnames, br_name);   
                snprintf(lan_ifnames, sizeof(lan_ifnames), "lan%d_ifnames", vlan_buf[p->vlan].bridge_id);
#ifdef CONFIG_SUPPORT_WIFI_5G
                ptr = nvram_bcm_safe_get(lan_ifnames);
                if(ptr && strlen(ptr))
                {
                    strcat(lan_ifname,ptr);
                    nvram_bcm_set(lan_ifnames, lan_ifname);
                }
                else
                {
                    nvram_bcm_set(lan_ifnames, lan_ifname);   
                }
#else
                nvram_bcm_set(lan_ifnames,lan_ifname);   
#endif
            }
            if(!p->block_boardcast)
            {
                SYSTEM_X("ebtables -A FORWARD -o %s -p arp -j ACCEPT", p->interface);
                SYSTEM_X("ebtables -A FORWARD -o %s --pkttype-type broadcast -j DROP", p->interface);
                SYSTEM_X("ebtables -A FORWARD -o %s --pkttype-type multicast -j DROP", p->interface);
                SYSTEM_Y("ebtables -D FORWARD -o %s -p arp -j ACCEPT", p->interface);
                SYSTEM_Y("ebtables -D FORWARD -o %s --pkttype-type broadcast -j DROP", p->interface);
                SYSTEM_Y("ebtables -D FORWARD -o %s --pkttype-type multicast -j DROP", p->interface);
            }
            SYSTEM_X("ebtables -A FORWARD -o %s -p ipv4 --ip-proto udp --ip-dport 67:68 -j DROP", p->interface);
            SYSTEM_X("ebtables -A FORWARD -i %s -p ipv4 --ip-proto udp --ip-dport 67:68 -j DROP", p->interface);
            SYSTEM_X("ebtables -A FORWARD -o %s -p ipv4 --ip-proto tcp -j TCPMSS --set-mss %d", p->interface, mss );
            SYSTEM_X("ebtables -A FORWARD -i %s -p ipv4 --ip-proto tcp -j TCPMSS --set-mss %d", p->interface, mss);
            SYSTEM_Y("ebtables -D FORWARD -o %s -p ipv4 --ip-proto tcp -j TCPMSS --set-mss %d", p->interface, mss);
            SYSTEM_Y("ebtables -D FORWARD -i %s -p ipv4 --ip-proto tcp -j TCPMSS --set-mss %d", p->interface, mss);
            SYSTEM_Y("ebtables -D FORWARD -o %s -p ipv4 --ip-proto udp --ip-dport 67:68 -j DROP", p->interface);
            SYSTEM_Y("ebtables -D FORWARD -i %s -p ipv4 --ip-proto udp --ip-dport 67:68 -j DROP", p->interface);
            SYSTEM_Y("ifconfig %s down", p->interface);
#ifdef CONFIG_SUPPORT_5G_QD
            if(p->is_5g)
                SYSTEM_Y("qcsapi_sockraw br0 00:26:86:00:00:00 enable_interface %s 0", ethif_mapping_to_qd_if(p->interface));
            else
#endif
            SYSTEM_Y("wlctl -i %s bss down", p->interface);
            p->is_up = 0;
            vlan_buf[p->vlan].vlan_buf = 1;
        }
    }
    SYSTEM_X("killall eapd");
    SYSTEM_X("killall nas");
    SYSTEM_X("nas");
    SYSTEM_X("eapd");
    SYSTEM_Y("ip link del gre%d",lac->tunnel_id+1);
    if(fp_shell)
        fclose(fp_shell);
    if(fp)
        fclose(fp);
    fp = NULL;
    fp_shell = NULL;

    ret =0;
    return ret;
}

// Create a new LAC and initiate a connection
struct gre_lac* gre_create(int tunnel_id, const char *host, const struct gre_cfg *cfg, int is_backup) {
	struct gre_lac *lac = calloc(1, sizeof(*lac));
	if (!lac)
		return NULL;

    lac->cfg = (struct gre_cfg *)cfg;
    lac->tunnel_id = tunnel_id;
    lac->is_backup = is_backup;
    lac->wan_id = config_get_int("main","wan_id",-1);

	// DNS lookup
	struct addrinfo *result, *rp;
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM,
		.ai_flags = AI_ADDRCONFIG
	};

	// (older) uclibc versions do not handle changes in resolv.conf correctly
#ifdef __UCLIBC__
#if __UCLIBC_MAJOR__ == 0 && __UCLIBC_MINOR__ == 9 && __UCLIBC_SUBLEVEL__ <= 29
	res_init();
#endif
#endif

	int st = getaddrinfo(host, NULL, &hints, &result);
	if (st) {
#ifdef __SC_BUILD__
        log_wifi(LOG_WARNING, NORM_LOG, LOG_NONUSE_ID, LOG_NONUSE_BLOCK_TIME, "getaddrinfo() failed: %s\n",
                gai_strerror(st));
#else

		syslog(LOG_WARNING, "getaddrinfo() failed: %s",
						gai_strerror(st));
#endif
		goto err;
	}

	// Setup failover timer
    lac->timer.interval = 1 * 1000; // 1 second
    lac->timer.value = 0;
    lac->timer.context = lac;
    lac->timer.handler = gre_timer;
    event_ctl(EVENT_TIMER_ADD, &lac->timer);
	// Try all resolved IPs for one we have a route for
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if (rp->ai_addrlen > sizeof(lac->addr))
			continue;

		memcpy(&lac->addr.saddr, rp->ai_addr, rp->ai_addrlen);
		lac->state = GRE_CONNECTING;
		if (!gre_setup(lac))
			break;

		lac->state = GRE_NONE;
		lac->addr.saddr.sa_family = AF_INET;
	}

	freeaddrinfo(result);

    if(lac->state == GRE_NONE)
    {
        event_ctl(EVENT_TIMER_DEL, &lac->timer);
        goto err;
    }

	return lac;

err:
	free(lac);
	return NULL;
}

enum gre_state gre_state(struct gre_lac *lac) {
	return (lac) ? lac->state : GRE_NONE;
}


// Destroy the LAC and free all resources
void gre_destroy(struct gre_lac *lac) {
	if (lac) {
		gre_shutdown(lac);
        if(lac->cfg->keepalive_interval != 0)
            event_ctl(EVENT_TIMER_DEL, &lac->timer);
		free(lac);
	}
}

int gre_shutdown(struct gre_lac *lac) {
    if (lac->state == GRE_SHUTDOWN) {
        errno = ENOTCONN;
        return -1;
    }
    if (lac->state != GRE_NONE) 
    {
        if(lac->is_up)
        {
            chmod(get_shutdown_script(lac->wan_id, lac->tunnel_id, lac->is_backup),755);
            SYSTEM(get_shutdown_script(lac->wan_id, lac->tunnel_id, lac->is_backup));
        }
        lac->is_up = 0;
        lac->state = GRE_SHUTDOWN;
    }

    return 0;
}
int gre_cleanup(struct gre_lac *lac) {
    if (lac->state == GRE_SHUTDOWN) {
        errno = ENOTCONN;
        return -1;
    }
    if (lac->state != GRE_NONE) 
    {
        if(lac->is_up)
        {
            chmod(get_shutdown_script(lac->wan_id, lac->tunnel_id, lac->is_backup),755);
            SYSTEM(get_shutdown_script(lac->wan_id, lac->tunnel_id, lac->is_backup));
        }
        lac->is_up = 0;
    }

    return 0;
}

// Timer (tunnel failover )
static void gre_timer(struct event_timer *timer, int64_t now) {
	struct gre_lac *lac = timer->context;
	int32_t nowsecs = now / 1000;
    static if_adv_info_t counter;
    memset(&counter, 0, sizeof(counter));
    char buf[32]  = {0};
    char* p = NULL;
    if(lac->cfg->keepalive_interval == 0)
    {
        if(lac->state != GRE_ESTABLISHED)
        {
            lac->state = GRE_ESTABLISHED;
            gre_notify(lac,GRE_ESTABLISHED);
        }
        event_ctl(EVENT_TIMER_DEL, &lac->timer);
        return ;
    }
    if(lac->state == GRE_ESTABLISHED || lac->state == GRE_CONNECTING)
    {
        if((lac->last_keepalive + lac->cfg->keepalive_interval) <= nowsecs)
        {
            SYSTEM("%s -m 2 -c 1 -h %s -W 1 -w %d -t %d -s %d -I %s &", lac->cmd, lac->server_ip, lac->wan_id, lac->tunnel_id, lac->is_backup, lac->wan_if);
            lac->last_keepalive = nowsecs;
            getIFAdvInfo(lac->gre_if, &counter);
            if(lac->rx_packets < counter.rx_packets)
            {
                if(lac->state != GRE_ESTABLISHED)
                {
                    lac->state = GRE_ESTABLISHED;
                    gre_notify(lac,GRE_ESTABLISHED);
                }
                lac->last_rx = nowsecs;
            }
            else
            {
                if((lac->last_rx + lac->cfg->timeout) <= nowsecs)
                {
                    snprintf(buf,sizeof(buf),"%d_%d_%d",lac->wan_id, lac->tunnel_id, lac->is_backup);
                    p = sal_misc_get_ipping_diag_succ_count_by_wan(buf);
                    if(p && *p == '1')
                    {
                        lac->lost_pings = 0;
                        if(lac->state != GRE_ESTABLISHED)
                        {
                            lac->state = GRE_ESTABLISHED;
                            gre_notify(lac,GRE_ESTABLISHED);
                        }
                    }
                    else
                        lac->lost_pings ++;
                    sal_misc_set_ipping_diag_succ_count_by_wan(buf,"");
                    if(lac->lost_pings > lac->cfg->timeout_num)
                    {
                        lac->state = GRE_UNREACHABLE;
                        gre_notify(lac, GRE_UNREACHABLE);
                    }
                }
            }
            lac->rx_packets = counter.rx_packets;
        }
    }
    else if(lac->state == GRE_UNREACHABLE)
    {
        if((lac->last_detectalive + lac->cfg->timeout_interval) <= nowsecs)
        {
            SYSTEM("%s -m 2 -c 1 -h %s -W 1 -w %d -t %d -s %d -I %s &", lac->cmd, lac->server_ip, lac->wan_id, lac->tunnel_id, lac->is_backup, lac->wan_if);
            lac->last_detectalive = nowsecs;
            snprintf(buf,sizeof(buf),"%d_%d_%d",lac->wan_id, lac->tunnel_id, lac->is_backup);
            p = sal_misc_get_ipping_diag_succ_count_by_wan(buf);
            if(p && *p == '1')
            {
                lac->state = GRE_CONNECTING;
            }
        }
    }
    return;
}


