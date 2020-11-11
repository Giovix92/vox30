
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#if defined(__SC_BUILD__) && defined(__SC_LEASE_RETAIN__) && defined(__SC_LEASE_VALIDITY_CHECK__)
#include <signal.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include "arpping.h"
#endif

#include "debug.h"
#include "dhcpd.h"
#include "hal_flash.h"
#include "leases.h"

#define HW_ADDRESS_LEN 16

#define LEASE_INFO_RETAINED_FILE "/var/lease_info_retained"

typedef enum
{
    NONEBOOT = 0,
    SOFTBOOT,
}BOOT_FLAG_E;

typedef struct fs_lease_info_header_s
{
    int flag;               /* BOOT_FLAG_E */
    unsigned int entry_num; /* entry num */
}fs_lease_info_header_t;

typedef struct fs_lease_info_entry_s
{
    unsigned char hw_addr[HW_ADDRESS_LEN];
    unsigned int ip;        /* network order */
    unsigned int expires;   /* host order */
    unsigned char hostname[MAX_HOSTNAME_LEN];   /* host order */
}fs_lease_info_entry_t;


#if defined(__SC_BUILD__) && defined(__SC_LEASE_RETAIN__) && defined(__SC_LEASE_VALIDITY_CHECK__)
static int lease_validity_check(void)
{
    int optval = 1;
    int	s;			        /* socket */
    struct sockaddr addr;	/* for interface name */
    struct arpMsg	arp;
    fd_set		fdset;
    struct dhcpOfferedAddr *lease;
    struct ifreq interface;
    struct timeval tv;
    int i, j;
    int valid_entry_num = 0;
    int timeout = 3;

    if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1) 
    {
        return -1;
    }
	
    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1) 
    {
        close(s);
	return -1;
    }

    strncpy(interface.ifr_ifrn.ifrn_name, server_config[0].interface, IFNAMSIZ);
    if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE,(char *)&interface, sizeof(interface)) < 0) {
    }
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    for (i = 0; i < no_of_ifaces; i++)
    {
        for (j = 0; j < server_config[i].max_leases; j++)
        {
            leases[i][j].valid = '\0';
            if (leases[i][j].expires > time(0))
            {
                /* send arp request */
                memset(&arp, 0, sizeof(arp));
                memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6);	        /* MAC DA */
                memcpy(arp.ethhdr.h_source, server_config[i].arp, 6);	/* MAC SA */
                arp.ethhdr.h_proto = htons(ETH_P_ARP);		            /* protocol type (Ethernet) */
                arp.htype = htons(ARPHRD_ETHER);		                /* hardware type */
                arp.ptype = htons(ETH_P_IP);			                /* protocol type (ARP message) */
                arp.hlen = 6;					                        /* hardware address length */
                arp.plen = 4;					                        /* protocol address length */
                arp.operation = htons(ARPOP_REQUEST);		            /* ARP op code */
                *((u_int *) arp.sInaddr) = server_config[i].server;	    /* source IP address */
                memcpy(arp.sHaddr, server_config[i].arp, 6);			/* source hardware address */
                *((u_int *) arp.tInaddr) = leases[i][j].yiaddr;		    /* target IP address */

                memset(&addr, 0, sizeof(addr));
                strcpy(addr.sa_data, server_config[i].interface);
                if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
                {
                    close(s);
                    return -1;
                }
            }
        }
    }

    while (1) 
    {
	FD_ZERO(&fdset);
	FD_SET(s, &fdset);
	    
        timeout = tv.tv_sec * 1000000 + tv.tv_usec; 
        if(timeout <= 0 || timeout > 3 * 1000000)
            break;
	if (select(s + 1, &fdset, (fd_set *) NULL, (fd_set *) NULL, &tv) < 0) 
        {
	    DEBUG(LOG_ERR, "Error on ARPING request: %s", strerror(errno));
            if (errno != EINTR)
            {
                break;
            }
            continue;
	} 
        else if (FD_ISSET(s, &fdset)) 
        {
	    if (recv(s, &arp, sizeof(arp), 0) > 0 &&
	        arp.operation == htons(ARPOP_REPLY)) 
            {
                for (i = 0; i < no_of_ifaces; i++)
                {
                    if (bcmp(arp.tHaddr, server_config[i].arp, 6) == 0 &&
                        (lease=find_lease_by_yiaddr(*((u_int *) arp.sInaddr), i)) &&
                        bcmp(arp.sHaddr, lease->chaddr, 6) == 0 &&
                        lease->expires > time(0))
                    {
                        lease->valid = '1';
                        valid_entry_num++;
                    }
                }
	    }
	}
    }
    close(s);
    return valid_entry_num;
}
#endif

int load_retained_lease_info(int ifid)
{
    fs_lease_info_header_t header;
    fs_lease_info_entry_t entry;
    FILE *fp;
    int i;
    char lease_file[128];

    snprintf(lease_file, sizeof(lease_file), "%s.%d", LEASE_INFO_RETAINED_FILE, ifid);
    
    if (hal_flash_load_dhcp_lease_info(lease_file, ifid) < 0)
        return -1;

    memset(&header, 0, sizeof(header));
    memset(&entry, 0, sizeof(entry));

    if ((fp = fopen(lease_file, "r")) == NULL)
    {
        return -1;
    }

    if (fread(&header, sizeof(header), 1, fp) <= 0 || header.flag != SOFTBOOT)
    {
        fclose(fp);
        return -1;
    }
    
    if (header.entry_num > 0)
    {
        while (fread(&entry, sizeof(entry), 1, fp))
        {
            for (i = 0; i < no_of_ifaces; i++)
            {
                if (ntohl(entry.ip) != ntohl(server_config[i].server)
                    && ntohl(entry.ip) >= ntohl(server_config[i].start)
                    && ntohl(entry.ip) <= ntohl(server_config[i].end))
                {
                    entry.expires = server_config[i].lease;
#ifdef __SC_BUILD__
                    add_lease(entry.hw_addr, entry.ip, entry.expires, i, entry.hostname);
#else
                    add_lease(entry.hw_addr, entry.ip, entry.expires, i);
#endif
                    break;
                }
            }
        }
    }
    fclose(fp);

    return header.entry_num;
}

int clear_retained_lease_info(int ifid)
{
    fs_lease_info_header_t header;
    FILE *fp;
    
    char lease_file[128];

    snprintf(lease_file, sizeof(lease_file), "%s.%d", LEASE_INFO_RETAINED_FILE, ifid);

    if ((fp = fopen(lease_file, "w")) == NULL)
    {
        return -1;
    }
    header.flag = NONEBOOT;
    header.entry_num = 0;
    fwrite(&header, sizeof(header), 1, fp);
    fclose(fp);

    if (hal_flash_store_dhcp_lease_info(lease_file, ifid) < 0)
        return -1;

    return 0;
}

int retain_lease_info(int ifid)
{
    fs_lease_info_header_t header;
    fs_lease_info_entry_t entry;
    FILE *fp;
    time_t curr = time(0);
    int entry_num = 0;
    int max_size, sum_size = 0;
    int i, j;
    char lease_file[128];
#if defined(__SC_BUILD__) && defined(__SC_LEASE_RETAIN__) && defined(__SC_LEASE_VALIDITY_CHECK__)
    int lease_check_rv;
    int lease_check_status = 0;
#endif

    memset(&header, 0, sizeof(header));
    memset(&entry, 0, sizeof(entry));
   
    max_size = hal_flash_get_dhcp_lease_info_max_size();
    if (max_size < (sizeof(header) + sizeof(entry)))
    {
        return -1;
    }

#if defined(__SC_BUILD__) && defined(__SC_LEASE_RETAIN__) && defined(__SC_LEASE_VALIDITY_CHECK__)
    lease_check_rv = lease_validity_check();
    if (lease_check_rv == 0)
    {
        return -1;
    }
    else if (lease_check_rv > 0)
        lease_check_status = 1;
    else
        lease_check_status = 0;
#endif

    snprintf(lease_file, sizeof(lease_file), "%s.%d", LEASE_INFO_RETAINED_FILE, ifid);

    if ((fp = fopen(lease_file, "w")) == NULL)
    {
        return -1;
    }

    fwrite(&header, sizeof(header), 1, fp);
    sum_size = sizeof(header);
    for (i = 0; i < no_of_ifaces; i++)
    {
        for (j = 0; j < server_config[i].max_leases; j++)
        {
#if defined(__SC_BUILD__) && defined(__SC_LEASE_RETAIN__) && defined(__SC_LEASE_VALIDITY_CHECK__)
            if (1 == lease_check_status && '\0' == leases[i][j].valid)
                continue;
#endif
            if (leases[i][j].expires > curr)
            {
                if ((sum_size+sizeof(entry)) > max_size)
                {
                    break;
                }
                memcpy(entry.hw_addr, leases[i][j].chaddr, HW_ADDRESS_LEN);
                entry.ip = leases[i][j].yiaddr;
                if(leases[i][j].expires != 0xFFFFFFFF)
                    entry.expires = leases[i][j].expires - curr;
                else
                    entry.expires = leases[i][j].expires;
                memcpy(entry.hostname, leases[i][j].hostname, MAX_HOSTNAME_LEN);
                fwrite(&entry, sizeof(entry), 1, fp);
                entry_num++;
                sum_size += sizeof(entry);
            }
        }
        if (j != server_config[i].max_leases)
        {
            break;
        }
    }
    fseek(fp, 0, SEEK_SET);
    header.flag = SOFTBOOT;
    header.entry_num = entry_num;
    fwrite(&header, sizeof(header), 1, fp);
    fclose(fp);
    
    if (hal_flash_store_dhcp_lease_info(lease_file, ifid) < 0)
        return -1;

    return 0;
}





