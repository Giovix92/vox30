/* dhcpd.c
 *
 * udhcp Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
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

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include "dhcpd.h"
#include "arpping.h"
#include "socket.h"
#include "options.h"
#include "files.h"
#include "leases.h"
#include "packet.h"
#include "serverpacket.h"
#include "pidfile.h"
#include "static_leases.h"
#include "option_action.h"
#if defined(__SC_BUILD__) && defined(__SC_LEASE_RETAIN__)
#include "lease_retain.h"
#endif
#include "debug.h"

/* globals */
struct dhcpOfferedAddr *leases[MAX_INTERFACES];
struct server_config_t server_config[MAX_INTERFACES];
int default_enable = TRUE;

struct client_option_s *vendor_header = NULL;
struct client_option_s *client_header = NULL;
struct client_option_s *user_class_header = NULL;
unsigned int lan_port_info_enable = 0;
struct lan_port_info_s *lan_port_header = NULL;
int gw_option_enable = TRUE;
int dns_option_enable = TRUE;

#define WRITE_LEASE_FILE_INTERVAL 10
#define WRITE_FILE_TO_FLASH_INTERVAL 30//30 *10 five minutes
int past_seconds = 0;
int past_seconds_flash = 0;
int no_of_ifaces = 0;

static char conf_file[256] = "";
static int signal_flag = 0;
static int signal_sig = -1;
static int reset_flag = 0;
#ifdef __SC_BUILD__
static int check_client = 0;
static int ifid;
#endif

/* Exit and cleanup */
static void exit_server(int retval, int ifid)
{
#ifdef __SC_BUILD__ 
    DEBUG(LOG_WARNING, "DHCP Server exited"); 
#endif
	//pidfile_delete(server_config[ifid].pidfile);
	unlink("/var/run/udhcpd.pid");
	CLOSE_LOG();
	exit(retval);
}

static void update_lease_files(void)
{
    int i;
    for (i = 0; i < no_of_ifaces; i++)
        write_leases(i);
}

/* -- Jeff Sun -- Apr.23.2005 -- add here for make ipaddr expire */ 
static void expire_action(u_int32_t ipaddr)
{
    int i; 
    unsigned int j;
    for(i=0;i<no_of_ifaces;i++)
	{
		if(server_config[i].active == FALSE)
				continue;
        
    	for(j=0;j<server_config[i].max_leases;j++)
    		if(leases[i][j].yiaddr == ipaddr)
   				leases[i][j].expires = time(0);
    }  
}
/* SIGUSR1 handler */
static void do_expire(void)
{
    FILE *fp;
    char ip[20];
    char *pp;
    struct in_addr addr;
    
    if(access("/tmp/dhcpd.delete",F_OK)==0)
    {
        if( (fp=fopen("/tmp/dhcpd.delete","r")) != NULL )
        {
            while(fgets(ip,20,fp)!=NULL)
            {
                if( (pp=strchr(ip,'#')) != NULL )
                {
                    *pp='\0';
                    inet_aton(ip, &addr);
                    expire_action(addr.s_addr);
                }
            }
            fclose(fp);
        }
        system("/bin/rm -rf /tmp/dhcpd.delete");
    }
}

static void print_lease_info(void)
{    
    int i, j;
    time_t curr = time(0);
    unsigned char *mac, *ip;

    printf("\nMAC Address\t\tIP Address\tRemaining Time\n");
    for(i = 0; i < no_of_ifaces; i++)
	{
    	for(j = 0; j < server_config[i].max_leases; j++)
        {
   			if (leases[i][j].expires > curr)
            {
                mac = (unsigned char *)leases[i][j].chaddr;
                ip = (unsigned char *)&leases[i][j].yiaddr;
                printf("%02x:%02x:%02x:%02x:%02x:%02x\t%u.%u.%u.%u\t%d\n",
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                        ip[0], ip[1], ip[2], ip[3], leases[i][j].expires-curr);
            }
        }
    }
}

/* signal handler */
static void udhcpd_recv_signal(int sig)
{
	//LOG(LOG_INFO, "udhcp server receive signal %d", sig);
	DEBUG(LOG_INFO, "udhcp server receive signal %d\n", sig);
    signal_sig = sig;
    signal_flag = 1;
}

static void signal_action(void)
{
    signal_flag = 0;

    if (SIGUSR1 == signal_sig)
    {
        do_expire();
        print_lease_info();
    }
    else if (SIGUSR2 == signal_sig)
    {
        reconfig_dhcpd(conf_file);
    }
    else if (SIGTERM == signal_sig)
    {
    #if defined(__SC_BUILD__) && defined(__SC_LEASE_RETAIN__)
#ifdef CONFIG_SUPPORT_PLUME
    if(save_to_flash)
#endif
    {
        if(reset_flag)
            clear_retained_lease_info(ifid);
        else
            retain_lease_info(ifid);
        //LOG(LOG_INFO, "udhcp server retain lease end.");
    }
    #else
        update_lease_files();
    #endif
        exit_server(0, 0);
    }
    else if (SIGHUP == signal_sig || SIGINT == signal_sig) 
    {
        update_lease_files();
#ifdef __SC_BUILD__
        clear_retained_lease_info(ifid);
#endif
        exit_server(0, 0);
    }
    else if (SIGQUIT == signal_sig)
    {
        reset_flag = 1;
    }

    signal_sig = -1;
}



void SendLogToPC(u_int8_t *chaddr,u_int32_t yiaddr,int flag)
{
    struct in_addr leaseip;
    char mac[20]="";
    char *ip;
    char sendbuf[100];
    
    leaseip.s_addr=yiaddr;
    ip = inet_ntoa(leaseip);
    sprintf(mac,"%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n"
	    ,chaddr[0],chaddr[1],chaddr[2],chaddr[3],chaddr[4],chaddr[5]);
    if(flag == 1)	
	sprintf(sendbuf,"[SC][Dhcp]:%s %s Request", ip, mac);	
    else if(flag == 2)
	sprintf(sendbuf,"[SC][Dhcp]:%s %s Release", ip, mac);		
    else 
	return;
    syslog(4,sendbuf);
    return;   
}

/* add by jacob */
static int check_bind_mac(unsigned char *bind_mac, unsigned char *client_mac)
{
    unsigned int i, len;
    char mac[20];

    /* 00:c0:02:**:**:** length is 17 */
    len = strlen((char *)bind_mac);
    if (len > 20)
        return FALSE;

    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", client_mac[0], client_mac[1],
        client_mac[2], client_mac[3], client_mac[4], client_mac[5]);

    if (len < strlen(mac))
        return FALSE;

    len = strlen(mac);
    for (i = 0; i < len; i++)
    {
        if ('*' == bind_mac[i])
            continue;
        if (tolower(bind_mac[i]) != tolower(mac[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}
#define foreach(word, wordlist, next) \
    for (next = &wordlist[strspn(wordlist, ";")], \
            strncpy(word, next, sizeof(word)), \
            word[strcspn(word, ";")] = '\0', \
            word[sizeof(word) - 1] = '\0', \
            next = strchr(next, ';'); \
            strlen(word); \
            next = next ? &next[strspn(next, ";")] : "", \
            strncpy(word, next, sizeof(word)), \
            word[strcspn(word, ";")] = '\0', \
            word[sizeof(word) - 1] = '\0', \
            next = strchr(next, ';'))

static int check_bind_vendor(char *server_vendor, char *vendor)
{
    char conf_val[128] = {0};
    char* next = NULL;
    if(server_vendor && vendor && strlen(server_vendor) && strlen(vendor))
    {
        foreach(conf_val,server_vendor, next)
        {
            if(strcmp(conf_val,vendor) == 0)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}


static int get_subnet_id(u_int32_t client_ip)
{
    //struct option_set *option;
    int i;

    for (i = 0; i < no_of_ifaces; i++)
	{
        if ((server_config[i].netmask & server_config[i].server) == (server_config[i].netmask & client_ip))
            return i;
	}
	return -1;
}
/* end jacob */

#if defined(__SC_BUILD__) && defined(__SC_CONDITIONAL_SERVING_PER_IF__)
typedef struct bridge_port_mapping_entry_s
{
    int sc_port;
    int brg_port;
}bridge_port_mapping_entry_t;
static struct bridge_port_mapping_entry_s bridge_port_mapping_table[] = {
    {SC_PORT_LAN_1,     BRIDGE_PORT_LAN_1},
    {SC_PORT_LAN_2,     BRIDGE_PORT_LAN_2},
    {SC_PORT_LAN_3,     BRIDGE_PORT_LAN_3},
    {SC_PORT_LAN_4,     BRIDGE_PORT_LAN_4},
    {SC_PORT_WLAN_1,    BRIDGE_PORT_WLAN_1},
    {SC_PORT_WLAN_2,    BRIDGE_PORT_WLAN_2},
    {SC_PORT_WLAN_3,    BRIDGE_PORT_WLAN_3},
    {SC_PORT_WLAN_4,    BRIDGE_PORT_WLAN_4},
    {SC_PORT_WLAN_5,    BRIDGE_PORT_WLAN_5},
    {SC_PORT_WLAN_6,    BRIDGE_PORT_WLAN_6},
    {SC_PORT_WLAN_7,    BRIDGE_PORT_WLAN_7},
    {SC_PORT_WLAN_8,    BRIDGE_PORT_WLAN_8},
    {SC_PORT_IPPHONE,   BRIDGE_PORT_IPPONE_VLAN},
    {SC_PORT_NONE,      BRIDGE_PORT_NONE}
};

static int valid_source_bridge_port(int src_bridge_port)
{
    int i;

    for (i = 0; bridge_port_mapping_table[i].sc_port != SC_PORT_NONE; i++)
    {
        if (bridge_port_mapping_table[i].brg_port == src_bridge_port)
            return TRUE;
    }
    return FALSE;
}
static int check_bind_interface(int bind_if, int src_bridge_port)
{
    int i;

    if (SC_PORT_NONE == bind_if)
        return FALSE;

    for (i = 0; bridge_port_mapping_table[i].sc_port != SC_PORT_NONE; i++)
    {
        if (bridge_port_mapping_table[i].brg_port == src_bridge_port)
        {
            if (bridge_port_mapping_table[i].sc_port & bind_if)
                return TRUE;
            else
                return FALSE;
        }
    }
    return FALSE;
}
#endif /* __SC_CONDITIONAL_SERVING_PER_IF__ */
#if defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_TR111)
int delete_expired_options(int i)
{
	char cmd[64 ]= {0};
    int flag = 0;
    VENDOR_LIST *vendor_d,*vendor_tmp,*vendor_prior = NULL;
    vendor_tmp=vendor_t;
    struct dhcpOfferedAddr *lease = NULL;

	while(vendor_tmp!=NULL)
	{
		if(lease = find_lease_by_yiaddr(vendor_tmp->client,i))
		{
			if(lease_expired(lease))
			{
				if(vendor_tmp!=vendor_t)
				{
					vendor_d=vendor_tmp;
					vendor_prior->next=vendor_tmp->next;
					vendor_tmp=vendor_prior->next;
					free(vendor_d);
				}
				else
				{
					vendor_d=vendor_tmp;
					vendor_tmp=vendor_t=vendor_t->next;
					free(vendor_d);
				}
				flag = 1;
			}
			else
			{
				vendor_prior=vendor_tmp;
				vendor_tmp=vendor_tmp->next;
			}
		}
		else
		{
			if(vendor_tmp!=vendor_t)
			{
				vendor_d=vendor_tmp;
				vendor_prior->next=vendor_tmp->next;
				vendor_tmp=vendor_prior->next;
				free(vendor_d);
			}
			else
			{
				vendor_d=vendor_tmp;
				vendor_tmp=vendor_t=vendor_t->next;
				free(vendor_d);
			}
			flag = 1;
		}
	}
	return flag;
}
#endif

#ifdef COMBINED_BINARY	
int udhcpd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{	
    fd_set rfds;
    struct timeval tv;
    int server_socket;
    int bytes, retval;
    struct dhcpMessage packet;
    unsigned char *state;
    unsigned char *vendor_id;
    unsigned char *host_bind;
    unsigned char *server_id, *requested;
    u_int32_t server_id_align, requested_align;
    unsigned long timeout_end;
    struct option_set *option;
    struct dhcpOfferedAddr *lease;
    struct dhcpOfferedAddr static_lease;
    int pid_fd;
    int i;
    int active_count = 0;	
    uint32_t static_lease_ip;
#if defined(__SC_BUILD__)
    struct in_pktinfo  ptr_pktinfo;
    int src_bridge_port;
    int discard_packet = 0;
#endif
	
    OPEN_LOG("udhcpd");
    LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);

    for (i = 0; i < MAX_INTERFACES; i++)
        memset(&server_config[i], 0, sizeof(struct server_config_t));
	
    if (argc < 2)
    {
	snprintf(conf_file, sizeof(conf_file), "%s", DHCPD_CONF_FILE);
        ifid = 0;
    }
    else
    {
        snprintf(conf_file, sizeof(conf_file), "%s", argv[1]);
#ifdef __SC_BUILD__ 
        sscanf(conf_file, DHCPD_CONF_FILE".1.%d", &ifid);
#endif
    }
    read_config(conf_file);

    if (argc >= 3 && !strcmp(argv[2], "-n"))
    {
        default_enable = FALSE;
#ifdef __SC_BUILD__ 
        DEBUG(LOG_INFO, "DHCP Server Status is Off"); 
#endif
    }
    if (no_of_ifaces == 0)
        exit(0);

    DEBUG(LOG_INFO, "no_of_ifaces:%d\n", no_of_ifaces);

    for (i = 0; i < no_of_ifaces; i++)
    {
        pid_fd = pidfile_acquire(server_config[i].pidfile);
	pidfile_write_release(pid_fd);

	if ((option = find_option(server_config[i].options, DHCP_LEASE_TIME))) {
            memcpy(&server_config[i].lease, option->data + 2, 4);
	    server_config[i].lease = ntohl(server_config[i].lease);
	}
	else 
            server_config[i].lease = LEASE_TIME;
	
	if ((option = find_option(server_config[i].options, DHCP_SUBNET)))
	{
	    memcpy(&server_config[i].netmask, option->data + 2, 4);
	}
	else
            server_config[i].netmask = htonl(SUBNET_MASK);

	leases[i] = malloc(sizeof(struct dhcpOfferedAddr) * server_config[i].max_leases);
	memset(leases[i], 0, sizeof(struct dhcpOfferedAddr) * server_config[i].max_leases);

	read_leases(server_config[i].lease_file, i);
		
	if (read_interface(server_config[i].interface, &server_config[i].ifindex,
	    &server_config[i].server, server_config[i].arp) < 0)
            server_config[i].active = FALSE;
	else
	    server_config[i].active = TRUE;

#ifndef DEBUGGING
	    pid_fd = pidfile_acquire(server_config[i].pidfile); /* hold lock during fork. */
		/* cfgmr req: do not fork */
		/*
		if (daemon(0, 0) == -1) {
			perror("fork");
			exit_server(1, i);
		}
		*/

	    pidfile_write_release(pid_fd);
#endif
	}
	for (active_count = 0, i = 0; i < no_of_ifaces; i++)
	{
	    DEBUG(LOG_DEBUG, "interface:%s, active:%s, cds_enable:%s, vendor_bind:%s, mac_bind:%s", server_config[i].interface?:"",
	    (server_config[i].active == TRUE)?"1":"0", server_config[i].cds_enable?:"", server_config[i].vendor_bind?:"", server_config[i].mac_bind?:"");
	    if (TRUE == server_config[i].active)
	        active_count++;
	}
	if (0 == active_count)
	    exit_server(0, 0);

#if defined(__SC_BUILD__) && defined(__SC_LEASE_RETAIN__)
#ifdef CONFIG_SUPPORT_PLUME
    if(save_to_flash)
#endif
    if (load_retained_lease_info(ifid) > 0)
    {
        update_lease_files();
    }
#endif

    signal(SIGUSR2, (void *)udhcpd_recv_signal);
    signal(SIGUSR1, (void *)udhcpd_recv_signal);
    signal(SIGTERM, (void *)udhcpd_recv_signal);
    signal(SIGINT, (void *)udhcpd_recv_signal);
    signal(SIGHUP, (void *)udhcpd_recv_signal);
    signal(SIGQUIT, (void *)udhcpd_recv_signal);

    DEBUG(LOG_DEBUG, "enter the main loop\n");
    
    server_socket = -1;
    while(1)  /* loop until universe collapses */
    {
        if (signal_flag)
        {
            signal_action();
        }
        
        if (server_socket < 0)
        {
#if defined(__SC_BUILD__)
            if ((server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config[0].interface, 1)) < 0)
#else
            if ((server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config[0].interface)) < 0)
#endif
            {
	        LOG(LOG_ERR, "FATAL: couldn't create server socket, errno: %d", errno);
        	exit_server(0, 0);
            }
        }     
        /* fd set. */
	FD_ZERO(&rfds);
	FD_SET(server_socket, &rfds);

	/* set time */
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	retval = select(server_socket + 1, &rfds, NULL, NULL, &tv);
	if (retval == 0) {
	    if (WRITE_LEASE_FILE_INTERVAL == past_seconds)
            {
                update_lease_files();
    		past_seconds = 0;
#ifdef __SC_BUILD__
                if((WRITE_FILE_TO_FLASH_INTERVAL == past_seconds_flash) && (check_client == 1))
                {
#ifdef CONFIG_SUPPORT_PLUME
                    if(save_to_flash)
#endif
                        retain_lease_info(ifid);
                    past_seconds_flash = 0;
                    check_client = 0;
                }
                else
                    past_seconds_flash++;
#endif
	    }
	    else
	        past_seconds++;
	    continue;
	} else if (retval < 0) {
			DEBUG(LOG_INFO, "error on select");
			continue;
	}
#if defined(__SC_BUILD__)

        if ((bytes = get_packet(&packet, server_socket, &ptr_pktinfo)) < 0) { /* this waits for a packet - idle */
#else
		if ((bytes = get_packet(&packet, server_socket)) < 0) { /* this waits for a packet - idle */
#endif
			if (bytes == -1 && errno != EINTR) {
#ifndef __SC_BUILD__ 
				DEBUG(LOG_INFO, "error on read, errno %d, reopening socket", errno);
#else
                                DEBUG(LOG_ERR, "Failed to connect to client"); 
#endif
				close(server_socket);
				server_socket = -1;
			}
			DEBUG(LOG_DEBUG, "bytes = %d\n", bytes);
			continue;
		}

        gw_option_enable = TRUE;
        dns_option_enable = TRUE;
#if defined(__SC_BUILD__)
        memcpy(packet.chaddr + MAC2_OFFSET, &(ptr_pktinfo.ipi_addr.s_addr), sizeof(ptr_pktinfo.ipi_addr.s_addr));
        memcpy(packet.chaddr + MAC2_OFFSET+sizeof((ptr_pktinfo.ipi_addr.s_addr)), &(ptr_pktinfo.ipi_spec_dst.s_addr), MAC_LENGTH-sizeof(ptr_pktinfo.ipi_addr.s_addr));
        src_bridge_port = ptr_pktinfo.ipi_ifindex;

        if (lan_port_info_enable && BRIDGE_LAN_PORT(src_bridge_port))
        {
            struct lan_port_info_s *lan_port_entry = lan_port_header;
            while (lan_port_entry)
            {
                if (lan_port_entry->port_bit == src_bridge_port && lan_port_entry->distribute_ip_enable)
                {
                    break;
                }
                lan_port_entry = lan_port_entry->next;
            }
            if (!lan_port_entry)
            {
#ifdef __SC_BUILD__ 
                DEBUG(LOG_WARNING, "No lan port entry"); 
#endif
                continue;
            }
            if (lan_port_entry->distribute_gw_enable)
                gw_option_enable = TRUE;
            else
                gw_option_enable = FALSE;
            if (lan_port_entry->distribute_dns_enable)
                dns_option_enable = TRUE;
            else
                dns_option_enable = FALSE;
        }
#endif

	if ((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
#ifndef __SC_BUILD__ 
			DEBUG(LOG_ERR, "couldn't get option from packet, ignoring");
#else
            DEBUG(LOG_ERR, "Couldn't get option from client [%u.%u.%u.%u]"
                     ,NIPQUAD(packet.ciaddr)); 
#endif
			continue;
	}

        // init i, no server config will be used
        i = no_of_ifaces;

        if (DHCPRELEASE == state[0])
        {
            i = get_subnet_id(packet.ciaddr);
            if (i < 0)
                continue;
            goto FIND_DHCP_LEASE;
        }

#if defined(__SC_BUILD__) && defined(__SC_CONDITIONAL_SERVING_PER_IF__)
        // check which server config meet the src_bridge_port
        
        discard_packet = 0;
        if (i == no_of_ifaces)
        {
            if (valid_source_bridge_port(src_bridge_port))
            {
#ifdef __SC_BUILD__ 
                DEBUG(LOG_DEBUG, "packet port:%d", src_bridge_port); 
#endif
                for (i = 0; i < no_of_ifaces; i++)
                {
                    if (server_config[i].active == FALSE || !server_config[i].cds_enable || *server_config[i].cds_enable != '1')
                        continue;

                    if (check_bind_interface(server_config[i].brg_port_bind, src_bridge_port))
                    {
                        if(server_config[i].vendor_bind && strlen(server_config[i].vendor_bind)) // need to match vendor also when port matched
                        {
                            int len=0;
                            char vendor[512] = {'\0'}; 
                            if ((vendor_id = get_option_x(&packet, DHCP_VENDOR,&len)) != NULL)
                            {
                                if (len < (sizeof(vendor) - 1))
                                {

                                    memcpy(vendor,vendor_id,len);
                                    vendor[len] = '\0';
#ifdef __SC_BUILD__ 
                                    DEBUG(LOG_DEBUG, "packet DHCP_VENDOR:%s", vendor); 
#endif

                                    if (server_config[i].vendor_bind && check_bind_vendor(server_config[i].vendor_bind, vendor))
                                    {
                                        break;
                                    }
                                    else // port match, vendor not match, dismiss the packet
                                    {
                                        discard_packet = 1;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                discard_packet = 1;
                                break;
                            }
                        }
                        else
                            break;
                    }
                }
            }
        }
        if(discard_packet)
            continue;
#endif
        // check which server config meet the packet mac
		if (i == no_of_ifaces)
		{
#ifdef __SC_BUILD__ 
            DEBUG(LOG_DEBUG, "packet mac:%02x:%02x:%02x:%02x:%02x:%02x", 
                    packet.chaddr[0], packet.chaddr[1], packet.chaddr[2],
                    packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);
#endif
		    for (i = 0; i < no_of_ifaces; i++)
    		{
    		    if (server_config[i].active == FALSE || !server_config[i].cds_enable || *server_config[i].cds_enable != '1')
    				continue;
    			if (server_config[i].mac_bind && check_bind_mac(server_config[i].mac_bind, packet.chaddr))
    			{
    			    break;
    			}
    		}
		}
        
        // check which server config meet the DHCP_VENDOR_ID
        if (i == no_of_ifaces)
        {
	    int len = 0;
	    char vendor[512] = {'\0'}; 
            if ((vendor_id = get_option_x(&packet, DHCP_VENDOR,&len)) != NULL)
	    {
		if (len < (sizeof(vendor) - 1))
		{

                    memcpy(vendor,vendor_id,len);
                    vendor[len] = '\0';
#ifdef __SC_BUILD__ 
                    DEBUG(LOG_DEBUG, "packet DHCP_VENDOR:%s", vendor); 
#endif

                    for (i = 0; i < no_of_ifaces; i++)
                    {
                    	if (server_config[i].active == FALSE || !server_config[i].cds_enable || *server_config[i].cds_enable != '1')
                        	continue;
                    	if (server_config[i].vendor_bind && check_bind_vendor(server_config[i].vendor_bind, vendor))
                    	{
                        	break;
                    	}
                    }
		}
            }
        }
        // check which server config meet the DHCP_HOST_NAME, currently just support prefix mode
        if (i == no_of_ifaces)
        {
            if ((host_bind = get_option(&packet, DHCP_HOST_NAME)) != NULL)
            {
                for (i = 0; i < no_of_ifaces; i++)
                {
                    if (server_config[i].active == FALSE || !server_config[i].cds_enable || *server_config[i].cds_enable != '1')
                        continue;

                    if (server_config[i].host_bind && (strncmp(host_bind, server_config[i].host_bind, strlen(server_config[i].host_bind)) == 0))
                    {
                        break;
                    }
                }
            }
        }

        
        if (i == no_of_ifaces)
        {
            DEBUG(LOG_DEBUG, "nothing match\n");
            if (TRUE == default_enable && server_config[DEFAULT_SERVER_CONFIG].cds_enable && *server_config[DEFAULT_SERVER_CONFIG].cds_enable != '1')
                i = DEFAULT_SERVER_CONFIG;
            else
            {
                continue;
            }
        }

FIND_DHCP_LEASE:
		/* ADDME: look for a static lease */
		static_lease_ip = getIpByMac(server_config[i].static_leases, &packet.chaddr);

		if(static_lease_ip)
		{
#ifdef __SC_BUILD__ 
                    struct in_addr in;
                    in.s_addr = static_lease_ip;
                    DEBUG(LOG_DEBUG, "Found static lease: %s", inet_ntoa(in)); 
#endif

		    memcpy(&static_lease.chaddr, &packet.chaddr, 16);
		    static_lease.yiaddr = static_lease_ip;
		    static_lease.expires = 0;
		    lease = &static_lease;

		}
		else
		{
			lease = find_lease_by_chaddr(packet.chaddr, i);
		}
		
#ifdef __SC_BUILD__ 
        DEBUG(LOG_DEBUG, "Select interface:%s, and now deal the state", server_config[i].interface?:""); 
#endif
		
		switch (state[0]) {
			case DHCPDISCOVER:
#ifndef __SC_BUILD__ 
			   DEBUG(LOG_INFO,"received DISCOVER");
#else 
                           DEBUG(LOG_INFO, "Received DISCOVER from client mac [%02x:%02x:%02x:%02x:%02x:%02x]\n", 
                                packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
		                        packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
#endif
//				if(do_option_action(state[0], &packet, i) != 0)
//				    break;
				if (sendOffer(&packet, i) < 0) {
#ifdef __SC_BUILD__ 
                DEBUG(LOG_ERR, "Failed to send OFFER to client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]",
                        NIPQUAD(packet.ciaddr),
                        packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
                        packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
#else
                    LOG(LOG_ERR, "send OFFER failed");
#endif
				}
				break;			
	
			case DHCPREQUEST:
#ifndef __SC_BUILD__ 
				DEBUG(LOG_INFO, "received REQUEST");
#else
                DEBUG(LOG_INFO, "Received REQUEST from client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]\n", 
		                        NIPQUAD(packet.ciaddr),
                                packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
		                        packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
#endif
//				if(do_option_action(state[0], &packet, i) != 0)
//				    break;
#ifdef __SC_BUILD__
                check_client = 1;
#endif
				requested = get_option(&packet, DHCP_REQUESTED_IP);
				server_id = get_option(&packet, DHCP_SERVER_ID);
				if (requested) memcpy(&requested_align, requested, 4);
				if (server_id) memcpy(&server_id_align, server_id, 4);
	
				if (lease) { /*ADDME: or static lease */
					if (server_id) {
						/* SELECTING State */
						DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
						if (server_id_align == server_config[i].server && requested && 
				    		requested_align == lease->yiaddr) {
							sendACK(&packet, lease->yiaddr, i);
						}
					} else {
						if (requested) {
							/* INIT-REBOOT State */
                        #if defined(__SC_BUILD__) && defined(__SC_ARP__)
                            if (lease->yiaddr == requested_align && !check_ip(lease->yiaddr, i, packet.chaddr))
                        #else
                            if (lease->yiaddr == requested_align)
                        #endif
								sendACK(&packet, lease->yiaddr, i);
							else sendNAK(&packet, i);
						} else {
							/* RENEWING or REBINDING State */
                        #if defined(__SC_BUILD__) && defined(__SC_ARP__)
                            if (lease->yiaddr == packet.ciaddr && !check_ip(lease->yiaddr, i, packet.chaddr))
                        #else
                            if (lease->yiaddr == packet.ciaddr)
                        #endif
								sendACK(&packet, lease->yiaddr, i);
							else {
								/* don't know what to do!!!! */
								sendNAK(&packet, i);
							}	
						}						
					}
                    			/* what to do if we have no record of the client */
				        } else if (server_id) {
				                /* SELECTING State */
						sendNAK(&packet,i);

	                                } else if (requested) {
				               /* INIT-REBOOT State */
				        	if ((lease = find_lease_by_yiaddr(requested_align,i))) {
				            		if (lease_expired(lease)) {
				               			/* probably best if we drop this lease */
					               		memset(lease->chaddr, 0, 16);
					               		/* make some contention for this address */
					            	} else { 
							    sendNAK(&packet,i);
					    		}
		                        	} else if (requested_align < server_config[i].start ||
				                   requested_align > server_config[i].end) {
				                  sendNAK(&packet,i);
				       		 } else {
							/* else remain silent */
							sendNAK(&packet,i);
						 }

	                        	} else {
                           				/* RENEWING or REBINDING State */
						sendNAK(&packet,i);
						
                    			}
                if(lease != NULL)          		
              	{       			
          			SendLogToPC(packet.chaddr,lease->yiaddr,1);  
              	}
             	break;

			case DHCPDECLINE:
#ifndef __SC_BUILD__ 
				DEBUG(LOG_INFO,"received DECLINE");
#else
                DEBUG(LOG_INFO, "Received DECLINE from client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]\n", 
		                        NIPQUAD(packet.ciaddr),
                                packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
		                        packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
#endif
				if (lease) {
					memset(lease->chaddr, 0, 16);
					lease->expires = time(0) + server_config[i].decline_time;
				}			
				break;
			
			case DHCPRELEASE:
#ifdef __SC_BUILD__ 
                DEBUG(LOG_INFO, "Received RELEASE from client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]\n", 
		                        NIPQUAD(packet.ciaddr),
                                packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
		                        packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
                char cmd[128] = {0};
                unsigned char mac_buf[64];
                snprintf(mac_buf, sizeof(mac_buf), "%02X:%02X:%02X:%02X:%02X:%02X", packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);
                snprintf(cmd, sizeof(cmd), "/usr/sbin/ip neigh del %u.%u.%u.%u lladdr %s dev br0", NIPQUAD(packet.ciaddr), mac_buf);
                system(cmd);
                snprintf(cmd, sizeof(cmd), "/usr/sbin/cmset -m %s -i 0", mac_buf);
                system(cmd);
#else
				DEBUG(LOG_INFO,"received RELEASE");
#endif
#ifdef __SC_BUILD__ 
                if(!lease)
                {
                    for (i = 0; i < no_of_ifaces; i++)
                    {
                        if (server_config[i].active == FALSE || !server_config[i].cds_enable || *server_config[i].cds_enable != '1')
                            continue;

                        lease = find_lease_by_chaddr(packet.chaddr, i);
                        if(lease)
                            break;
                    }
                }
#endif
				if (lease) 
				{        			
					SendLogToPC(lease->chaddr,lease->yiaddr,2);
					lease->expires = time(0);
					//delete udhcp.lease
					FILE *fp;
					int buf_len;
					unsigned char clientmac[16] = "";
					unsigned char mac[18] = "";  
					char *buf, *s;
					char line[256];
					memcpy(clientmac, packet.chaddr, 16); 
					sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
								clientmac[0], clientmac[1], clientmac[2],
								clientmac[3], clientmac[4], clientmac[5]);
					if( (fp=fopen(VENDOR_FILE, "r"))!=NULL )
					{
						fseek(fp, 1, SEEK_END);
						buf_len = ftell(fp) + 2;
						rewind(fp);
						if( (buf = (char*)malloc(sizeof(char)*buf_len))!=NULL )
						{
							memset(buf, 0, sizeof(char)*buf_len);
							while(fgets(line, 255, fp))
							{
								s=strstr(line, mac);
								if(s==NULL)
								{
									strcat(buf, line);
								}
							}
							fclose(fp);
							if( (fp=fopen(VENDOR_FILE, "w"))!=NULL )
							{
								fprintf(fp, "%s", buf);
								fclose(fp);
							}
							free(buf);
						}
						else
						{
							fclose(fp);
						}
					}
				}
				break;
	
			case DHCPINFORM:
#ifdef __SC_BUILD__ 
                DEBUG(LOG_INFO, "Received INFORM from client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]\n", 
		                        NIPQUAD(packet.ciaddr),
                                packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
		                        packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
#else
				DEBUG(LOG_INFO,"received INFORM");
#endif
				send_inform(&packet, i);
				break;	
			
			default:
#ifdef __SC_BUILD__ 
                DEBUG(LOG_WARNING, "Received unsupported DHCP message (%02x) from client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]\n", 
		                        state[0], NIPQUAD(packet.ciaddr),
                                packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
		                        packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
#else
                LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
#endif
		}
	}
	return 0;
}

