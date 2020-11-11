/* serverpacket.c
 *
 * Constuct and send DHCP server packets
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

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <stdarg.h>
#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"
#include "static_leases.h"
#include "networkmap.h"
#if defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_TR111)
#include <sal/sal_tr111.h>
#define MAX_HOP_NUMBER   250
VENDOR_LIST *vendor_t=NULL;
char *entries_num_value="";
int entries_num=0;
#endif

/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload, int ifid)
{
    DEBUG(LOG_INFO, "Forwarding packet to relay");

    return kernel_packet(payload, server_config[ifid].server, SERVER_PORT,
            payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast, int ifid)
{
    unsigned char *chaddr;
    u_int32_t ciaddr;

    if (force_broadcast) {
        DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
        ciaddr = INADDR_BROADCAST;
        chaddr = MAC_BCAST_ADDR;
    } else if (payload->ciaddr) {
        DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
        ciaddr = payload->ciaddr;
#ifdef __SC_BUILD__
        chaddr = payload->chaddr+MAC2_OFFSET;
#else
        chaddr = payload->chaddr;
#endif
    } else if (ntohs(payload->flags) & BROADCAST_FLAG) {
        DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
        ciaddr = INADDR_BROADCAST;
        chaddr = MAC_BCAST_ADDR;
    } else {
        DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
        ciaddr = payload->yiaddr;
#ifdef __SC_BUILD__
        chaddr = payload->chaddr+MAC2_OFFSET;
#else
        chaddr = payload->chaddr;
#endif
    }
#ifdef __SC_BUILD__

    return raw_packet(payload, server_config[ifid].server, SERVER_PORT,
            ciaddr, CLIENT_PORT, chaddr, server_config[ifid].ifindex, 0, 0);
#else
    return raw_packet(payload, server_config[ifid].server, SERVER_PORT,
            ciaddr, CLIENT_PORT, chaddr, server_config[ifid].ifindex);
#endif
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast, int ifid)
{
    int ret;

    if (payload->giaddr)
        ret = send_packet_to_relay(payload, ifid);
    else ret = send_packet_to_client(payload, force_broadcast, ifid);
#ifdef __SC_BUILD__ 
    if(ret < 0)
        DEBUG(LOG_ERR, "Faied to send DHCP message (%02x) to client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]", 
                payload->op, NIPQUAD(payload->ciaddr),
                payload->chaddr[0], payload->chaddr[1], payload->chaddr[2], 
                payload->chaddr[3], payload->chaddr[4], payload->chaddr[5]); 
#endif

    return ret;
}

#ifdef __SC_BUILD__

static char *cpe_getSerialNumber(void)
{
    static char snStr[18] = { 0, };
    char *csn=server_config[0].serialnumber;
    if(csn && strlen(csn))
        snprintf(snStr, sizeof(snStr), "%s",csn);
    else
        snStr[0] == '\0';
    return snStr;
}
#define ADSL_FORUM_ENTERPRISE_NUMBER   3561
static void add_vendor_options(struct dhcpMessage *packet)
{
    unsigned char *data = NULL,*pt;
    unsigned char oui[9]="",sn[66]="",prds[66]="";
    int len = 0,rc=-1;

    static char ouiStr[18] = { 0, };
    static char classStr[18] = { 0, };
    char *sal_oui=server_config[0].manufacture_oui;
    char *sal_class=server_config[0].product_class;
    if(!sal_oui || !sal_class || sal_oui[0] == '\0' || sal_class[0] == '\0')
        return;

    snprintf(ouiStr, sizeof(ouiStr), "%s",sal_oui);
    unsigned char *CWMP_MANUFACTURER_OUI=ouiStr;
    snprintf(classStr, sizeof(classStr), "%s",sal_class);
    unsigned char *CWMP_MODEL_NAME=classStr;

    pt =cpe_getSerialNumber();
    if(pt[0] == '\0')
        return;
    len = sprintf(oui,"%c%c%s",(unsigned char)4,(unsigned char)strlen(CWMP_MANUFACTURER_OUI),CWMP_MANUFACTURER_OUI);
    len += sprintf(sn,"%c%c%s",(unsigned char)5,(unsigned char)strlen(pt),pt);
    len += sprintf(prds,"%c%c%s",(unsigned char)6,(unsigned char)strlen(CWMP_MODEL_NAME),CWMP_MODEL_NAME);
    data = (unsigned char *)malloc(len + 8);
    if(data==NULL)
        return;
    memset(data, 0, len+8);
    sprintf(data,"%c%c%c%c%c%c%c%s%s%s"
            ,(unsigned char)DHCP_VENDOR_SPECIFIC_INFO     /* option-code 8bits */
            ,(unsigned char)(len + 5)            /* option-len 8bits */
            /* enterprise-number 32bits */
            ,(unsigned char)(ADSL_FORUM_ENTERPRISE_NUMBER>>24)
            ,(unsigned char)(ADSL_FORUM_ENTERPRISE_NUMBER>>16)
            ,(unsigned char)(ADSL_FORUM_ENTERPRISE_NUMBER>>8)
            ,(unsigned char)ADSL_FORUM_ENTERPRISE_NUMBER
            ,(unsigned char)len                  /* data-len 8bits */
            /* sub-option*/
            ,oui
            ,sn
            ,prds);
    rc = add_option_string(packet->options, data);
    free(data);
    return;
}
#ifdef CONFIG_SUPPORT_TR111
static void update_vendor_options()
{
    int entries_count = 0;
    char cmd[64 ]= {0};
    VENDOR_LIST *vendor_tmp;
    vendor_tmp=vendor_t;

    while(vendor_tmp!=NULL)
    {
        entries_count+=1;
        sal_tr111_set_number(vendor_tmp->serialNumber,entries_count);
        sal_tr111_set_oui(vendor_tmp->oui,entries_count);
        sal_tr111_set_class(vendor_tmp->productClass,entries_count);

        snprintf(cmd,sizeof(cmd),"/usr/sbin/tr111_cmset -t %d",entries_count);
        system(cmd);

        vendor_tmp = vendor_tmp->next;
    }   
    entries_num = entries_count;
}

static void record_vendor_options(unsigned char *vendor_spc, u_int32_t caddr,int ifid)
{
    VENDOR_LIST *vd_p = vendor_t;
    unsigned char *oui,*sn,*prdt;
    int i,oui_len,sn_len,prdt_len,vendor_len;
    struct in_addr in;
    char cmd[64 ]= {0};
    int delete_flag = 0;
    char serialNumber[64];

    if(entries_num >= MAX_HOP_NUMBER)
    {
        delete_flag = delete_expired_options(ifid);
        if(!delete_flag)
            return;
    }
    vendor_len = *(vendor_spc - 1); 
    i = 6;
    oui_len = vendor_spc[i];
    oui = vendor_spc + i +1;
    i += 1 + oui_len + 1;
    if (i > vendor_len)
        return;
    sn_len = vendor_spc[i];
    sn = vendor_spc + i +1;
    i += 1 + sn_len + 1;
    if (i > vendor_len)
        return;
    prdt_len = vendor_spc[i];
    prdt = vendor_spc + i +1;

    snprintf(serialNumber,sn_len+1,"%s",sn);
    in.s_addr = caddr;

    while(vd_p!=NULL){
        /* already existed, re-write it because it may be modified */
        if(!strcmp(vd_p->serialNumber,serialNumber))
        {
            vd_p->client = caddr;
            strncpy(vd_p->ip,inet_ntoa(in),16);
            return;
        }

        vd_p = vd_p->next;
    }

    if(vendor_t == NULL){
        vendor_t=(VENDOR_LIST *)malloc(sizeof(VENDOR_LIST));
        vd_p = vendor_t;
    }
    else{
        vd_p = vendor_t;
        while(vd_p->next!=NULL)
            vd_p = vd_p->next;
        vd_p->next = (VENDOR_LIST *)malloc(sizeof(VENDOR_LIST));
        vd_p = vd_p->next;
    }

    memset(vd_p,0,sizeof(VENDOR_LIST));
    if(caddr!=NULL)
    {
        vd_p->client = caddr;   
    }
    strncpy(vd_p->ip,inet_ntoa(in),16);
    strncpy(vd_p->oui,oui,oui_len);
    strncpy(vd_p->serialNumber,sn,sn_len);
    strncpy(vd_p->productClass,prdt,prdt_len);
    vd_p->next = NULL;

    snprintf(cmd,sizeof(cmd),"/usr/sbin/tr111_cmset -m %d",entries_num);
    system(cmd);//delete all 

    update_vendor_options();

    return;
}
#endif
#endif

static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type, int ifid)
{
    init_header(packet, type);
    packet->xid = oldpacket->xid;
    memcpy(packet->chaddr, oldpacket->chaddr, 16);
    packet->flags = oldpacket->flags;
    packet->giaddr = oldpacket->giaddr;
    packet->ciaddr = oldpacket->ciaddr;
    add_simple_option(packet->options, DHCP_SERVER_ID, server_config[ifid].server);
#ifdef __SC_BUILD__
    int len;
    unsigned char *option_125;
    unsigned char *client_id;

    if (type != DHCPNAK)
    {
        add_vendor_options(packet);
        /* add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));*/
    }
    if ((client_id = get_option_x(oldpacket, DHCP_CLIENT_ID, &len)) != NULL)
    {
		add_options_directly(packet->options, DHCP_CLIENT_ID, client_id, len);
    }
#endif
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet, int ifid)
{
    packet->siaddr = server_config[ifid].siaddr;
    if (server_config[ifid].sname)
        strncpy(packet->sname, server_config[ifid].sname, sizeof(packet->sname) - 1);
    if (server_config[ifid].boot_file)
        strncpy(packet->file, server_config[ifid].boot_file, sizeof(packet->file) - 1);
}

/* add dns option, check if there is fix dns server */
static void add_dns_option(unsigned char *optionptr, unsigned char *pipe_dns, int ifid)
{
#define IP_LEN  4
    unsigned char dns[400] = "";
    u_int32_t dns_server;
    unsigned int len = 0;

    dns[OPT_CODE] = DHCP_DNS_SERVER;
    if (server_config[ifid].dns_server)
    {
        struct dhcp_ip_list_s *cur = server_config[ifid].dns_server;
        while(cur)
        {
            memcpy(dns + OPT_DATA + len, &cur->ip, sizeof(cur->ip));
            len += sizeof(cur->ip);
            cur = cur->next;
        }
    }

    /* dns_proxy enable add lan ip in the dns option
       lan ip is the first ip in the pipe_dns */
    if (server_config[ifid].dns_proxy && '1' == *server_config[ifid].dns_proxy)
    {
        memcpy(dns + OPT_DATA + len, pipe_dns + OPT_DATA, pipe_dns[OPT_LEN]);
        len += pipe_dns[OPT_LEN];
    }
    else if (pipe_dns[OPT_LEN] > IP_LEN)
    {
        memcpy(dns + OPT_DATA + len, pipe_dns + OPT_DATA + IP_LEN, pipe_dns[OPT_LEN] - IP_LEN);
        len += pipe_dns[OPT_LEN] - IP_LEN;
    }
    if (len > 0)
    {
        dns[OPT_LEN] = len;
        add_option_string(optionptr, dns);
    }
}

#define ADD_DNS_OPTIONS() \
    do{\
        add_dns_option(packet.options, curr->data, ifid);\
        \
    }while(0)

static void get_send_options(struct dhcpMessage *packet,unsigned char * buf)
{
    unsigned char *optionptr = NULL;
    int i = 0;
    int length = MAX_DHCP_OPTIONS_LEN;
    unsigned char *state = NULL;
    int ret = 0;
    optionptr = packet->options;
    if ((state = get_option(packet,DHCP_MESSAGE_TYPE))!=NULL)
    {
        if (state[0] == DHCPDISCOVER)
        {
            //get the all options of discover packet
            ret =  sprintf(buf,"%d,",DHCP_MESSAGE_TYPE);
            while (i  < length) 
            {
                i += optionptr[OPT_LEN + i] + 2;
                if (optionptr[i + OPT_CODE] ==DHCP_END)
                    break;
                ret += sprintf(buf+ret,"%d,",optionptr[i + OPT_CODE]);

            }
            buf[ret-1] = '\0';

        }

    }
    return 0;

}
static void get_req_options(struct dhcpMessage *packet,unsigned char * req)
{

    unsigned char  * tmp = NULL;
    int len = 0;
    int i = 0;
    int ret = 0;
    if((tmp = get_option_x(packet,DHCP_PARAM_REQ,&len))!= NULL)
    {
        for (i = 0; i < len; i++)
        {
            ret += sprintf(req+ret,"%d,",tmp[i]);
        }
        if (ret >= 1)
            req[ret-1] = '\0';
    }


}


/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket, int ifid)
{
    struct dhcpMessage packet;
    struct dhcpOfferedAddr *lease = NULL;
    u_int32_t req_align = 0, lease_time_align = server_config[ifid].lease;
    unsigned char *req, *lease_time;
    struct option_set *curr;
    struct in_addr addr;

    uint32_t static_lease_ip;

#ifdef __SC_BUILD__
    char hostname[MAX_HOSTNAME_LEN]="";
#endif

    init_packet(&packet, oldpacket, DHCPOFFER, ifid);

    static_lease_ip=getIpByMac(server_config[ifid].static_leases,oldpacket->chaddr);
#if defined(__SC_BUILD__) && defined(__SC_ARP__)
    if (static_lease_ip && !check_ip(static_lease_ip, ifid, packet.chaddr))
#else
        if (static_lease_ip)
#endif
        {
            /*It is a static lease... use it*/
            packet.yiaddr=static_lease_ip;
        }

    /* ADDME: if static, short circuit */
    if (!packet.yiaddr)
    {
        /* the client is in our lease/offered table */
#if defined(__SC_BUILD__) && defined(__SC_ARP__)
        if ((lease = find_lease_by_chaddr(oldpacket->chaddr, ifid))&& !check_ip(lease->yiaddr, ifid, packet.chaddr)
                && !reservedIp(server_config[ifid].static_leases,lease->yiaddr))
#else
            if ((lease = find_lease_by_chaddr(oldpacket->chaddr, ifid)))
#endif
            {
                if (!lease_expired(lease))
                {
#ifdef __SC_BUILD__
                    if(server_config[ifid].lease != 0xFFFFFFFF)
#endif
                        lease_time_align = lease->expires - time(0);
                }
                packet.yiaddr = lease->yiaddr;
                //check the lease has been occupied or not
#ifdef __SC_BUILD__
                DEBUG(LOG_INFO, "Find a lease info, and use the ip:%08x", lease->yiaddr); 
#endif
            }
#ifdef __SC_BUILD__
        /* Or the client has a requested ip */
            else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&
                    /* Don't look here (ugly hackish thing to do) */
                    memcpy(&req_align, req, 4) &&
                    /* and the ip is in the lease range */
                    ntohl(req_align) >= ntohl(server_config[ifid].start) &&
                    ntohl(req_align) <= ntohl(server_config[ifid].end) &&
                    ntohl(req_align) != ntohl(server_config[ifid].server) &&
                    !static_lease_ip &&
                    /* and its not already taken/offered */ /* ADDME: check that its not a static lease */
                    ((!(lease = find_lease_by_yiaddr(req_align, ifid)) ||

                      /* or its taken, but expired */ /* ADDME: or maybe in here */
                      lease_expired(lease))) && !reservedIp(server_config[ifid].static_leases,req_align))
#else
            else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&
                    /* Don't look here (ugly hackish thing to do) */
                    memcpy(&req_align, req, 4) &&
                    /* and the ip is in the lease range */
                    ntohl(req_align) >= ntohl(server_config[ifid].start) &&
                    ntohl(req_align) <= ntohl(server_config[ifid].end) &&
                    ntohl(req_align) != ntohl(server_config[ifid].server) &&
                    !static_lease_ip &&
                    /* and its not already taken/offered */ /* ADDME: check that its not a static lease */
                    ((!(lease = find_lease_by_yiaddr(req_align, ifid)) ||

                      /* or its taken, but expired */ /* ADDME: or maybe in here */
                      lease_expired(lease))))
#endif
            {
                /* check id addr is not taken by a static ip */
#ifdef __SC_BUILD__
#ifdef __SC_ARP__
                if(!check_ip(req_align, ifid, packet.chaddr))
#else
                    if(!check_ip(req_align, ifid))
#endif
#else
                        if(!check_ip(req_align))
#endif
                        {
                            packet.yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */
#ifdef __SC_BUILD__
                            DEBUG(LOG_DEBUG, "Use requested ip:%08x", req_align); 
#endif
                        }
                        else
                        {
                            /*is it now a static lease, no,beacause find_address skips static lease*/
                            packet.yiaddr = find_address(0, ifid);
                            /* try for an expired lease */
                            if (!packet.yiaddr)
                                packet.yiaddr = find_address(1, ifid);
#ifdef __SC_BUILD__
                            DEBUG(LOG_DEBUG, "Find an assignable address:%08x", packet.yiaddr); 
#endif
                        }
            }
        /* otherwise, find a free IP */ /*ADDME: is it a static lease? */
            else
            {
                packet.yiaddr = find_address(0, ifid);
                /* try for an expired lease */
                if (!packet.yiaddr)
                    packet.yiaddr = find_address(1, ifid);
#ifdef __SC_BUILD__
                DEBUG(LOG_DEBUG, "Find an free address:%08x", packet.yiaddr); 
#endif
            }
        if(!packet.yiaddr)
        {
            LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
            return -1;
        }
#ifdef __SC_BUILD__
        {
            char *tmp=get_option(oldpacket,DHCP_HOST_NAME);
            int len=0;
            if(tmp)
            {
                len=*(tmp-1);

                if(len > sizeof(hostname) - 1)
                    len = sizeof(hostname) - 1;

                strncpy(hostname,tmp,len);
                hostname[len] = '\0';
            }
            //printf("client->name==%s\n",hostname);
        }
        if (!add_lease(packet.chaddr, packet.yiaddr, server_config[ifid].offer_time, ifid,hostname))
        {
            LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
            return -1;
        }
#else
        if (!add_lease(packet.chaddr, packet.yiaddr, server_config[ifid].offer_time, ifid))
        {
            LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
            return -1;
        }
#endif
#ifndef __SC_BUILD__
        if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME)))
        {
            memcpy(&lease_time_align, lease_time, 4);
            lease_time_align = ntohl(lease_time_align);
            if (lease_time_align > server_config[ifid].lease)
                lease_time_align = server_config[ifid].lease;
        }
#endif
        /* Make sure we aren't just using the lease time from the previous offer */
        if (lease_time_align < server_config[ifid].min_lease)
            lease_time_align = server_config[ifid].lease;
        /* ADDME: end of short circuit */
    }

#ifdef __SC_BUILD__
    if (gw_option_enable == FALSE)
        lease_time_align = PROVISIONAL_LEASE_TIME;
#endif
    add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));
    curr = server_config[ifid].options;
    while (curr) {
        if (curr->data[OPT_CODE] != DHCP_LEASE_TIME && curr->data[OPT_CODE] != DHCP_VENDOR)
        {
#ifdef __SC_BUILD__
            if (curr->data[OPT_CODE] == DHCP_DNS_SERVER)
            {
                if (dns_option_enable)
                    ADD_DNS_OPTIONS();
            }
            else if (curr->data[OPT_CODE] == DHCP_ROUTER)
            {
                if (gw_option_enable)
                    add_option_string(packet.options, curr->data);
            }
            else if (curr->data[OPT_CODE] == DHCP_OPTION_160)
            {
                int len;
                unsigned char *option_160;

                if ((option_160 = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(option_160, DHCP_OPTION_160, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_BOOT_FILE)
            {
                int len;
                unsigned char *boot_file;

                if ((boot_file = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(boot_file, DHCP_BOOT_FILE, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_OPTION_150)
            {
                int len;
                unsigned char *tftp_name;

                if ((tftp_name = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(tftp_name, DHCP_OPTION_150, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_SIP_SERVERS)
            {
                int len;
                unsigned char *outbound_proxy;

                if ((outbound_proxy = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(outbound_proxy, DHCP_SIP_SERVERS, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_DOMAIN_NAME)
            {
                int len;
                unsigned char *domain;

                if ((domain = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(domain, DHCP_DOMAIN_NAME, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_NTP_SERVER)
            {
                int len;
                unsigned char *ntp_server;

                if ((ntp_server = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(ntp_server, DHCP_NTP_SERVER, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_PROVISIONING_SERVER_URL)
            {
                int len;
                unsigned char *url;

                if ((url = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(url, DHCP_PROVISIONING_SERVER_URL, len))
                        add_option_string(packet.options, curr->data);
                }

            }
            else if (curr->data[OPT_CODE] == DHCP_TFTP)
            {
                int len;
                unsigned char *tftp;

                if ((tftp = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(tftp, DHCP_TFTP, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else
#endif
                add_option_string(packet.options, curr->data);
        }
        curr = curr->next;
    }

    add_bootp_options(&packet, ifid);

    addr.s_addr = packet.yiaddr;
#ifdef __SC_BUILD__ 
    LOG(LOG_INFO, "Send OFFER of ip [%s] to client mac [%02x:%02x:%02x:%02x:%02x:%02x]", 
            inet_ntoa(addr), 
            packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
            packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 

#else
    LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
#endif
    /* update networkmap list */
#ifdef __SC_BUILD__
    char cmd[64] = {0};
    unsigned char mac_buf[64];
    snprintf(mac_buf, sizeof(mac_buf), "%02X:%02X:%02X:%02X:%02X:%02X", packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);
    snprintf(cmd,sizeof(cmd),"/usr/sbin/cmset -m %s -i %lu", mac_buf, packet.yiaddr);
    system(cmd);
#if 0 //no need to parse option list from DHCP, networkmap knows
    /*update send options (list) and request options (list) into networkmap list*/
    unsigned char sendoption[MAX_DHCP_OPTIONS_LEN]={0};
    unsigned char reqoption[MAX_DHCP_OPTIONS_LEN]={0};
    char cmd_opt[512] = {0};
    memset(cmd_opt,0,sizeof(cmd_opt));
    memset(mac_buf,0,sizeof(mac_buf));
    get_send_options(oldpacket,sendoption);
    get_req_options(oldpacket,reqoption);
    snprintf(mac_buf, sizeof(mac_buf), "%02X:%02X:%02X:%02X:%02X:%02X", packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]);
    snprintf(cmd_opt,sizeof(cmd_opt),"/usr/sbin/cmset -m %s -s %s -r %s", mac_buf, sendoption, reqoption);
    system(cmd_opt);
#endif
#endif


    /* end */
    return send_packet(&packet, 0, ifid);
}


#ifdef __SC_BUILD__
static void add_message_options(struct dhcpMessage *packet)
{
    int len = 0;
    char pt[] = "Can't assign IP";
    len = strlen(pt);
    char data[65] = "";
    sprintf(data, "%c%c%s"
            ,(unsigned char)DHCP_MESSAGE
            ,(unsigned char)len
            , pt);
    add_option_string(packet->options, data);
    return;
}
#endif
int sendNAK(struct dhcpMessage *oldpacket, int ifid)
{
    struct dhcpMessage packet;

    init_packet(&packet, oldpacket, DHCPNAK, ifid);

#ifndef __SC_BUILD__
    DEBUG(LOG_INFO, "sending NAK");
#else
    add_message_options(&packet); 
    DEBUG(LOG_WARNING, "Send NAK to client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]",
            NIPQUAD(packet.ciaddr),
            packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
            packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
#endif
    return send_packet(&packet, 1, ifid);
}


int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr, int ifid)
{
    struct dhcpMessage packet;
    struct option_set *curr;

    struct in_addr addr;
    unsigned char *lease_time;
    char *buf, *s;
    u_int32_t lease_time_align = server_config[ifid].lease;
    FILE *fp, *timefp;
    //int vendor_len=0;
    int buf_len;
    unsigned char vendor_id[32]; // option 60
    long sys_time;
    struct in_addr client_addr;
    unsigned char client_mac[16] = "";
    unsigned char mac[18] = ""; 
    char line[256]; 
#if defined(__SC_BUILD__) && defined(CONFIG_SUPPORT_TR111)
    unsigned char *vendor_specific;
    vendor_specific=get_option(oldpacket,DHCP_VENDOR_SPECIFIC_INFO);
    if(vendor_specific){
        unsigned char *vendor_spc=NULL,*req=NULL;
        u_int32_t req_align;
        if((req = get_option(oldpacket, DHCP_REQUESTED_IP)))
            memcpy(&req_align, req, 4);
        if(vendor_specific && req)
            record_vendor_options(vendor_specific,req_align,ifid);
    }
#endif 
#ifdef __SC_BUILD__
    char hostname[MAX_HOSTNAME_LEN]="";

    memcpy(client_mac, oldpacket->chaddr, 16);
    bzero(vendor_id, sizeof(vendor_id));
    get_vendor_id(oldpacket, vendor_id, 32);

    memcpy(&(client_addr.s_addr), &yiaddr, 4);
#endif
    init_packet(&packet, oldpacket, DHCPACK, ifid);
    packet.yiaddr = yiaddr;
#ifndef __SC_BUILD__
    if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
        memcpy(&lease_time_align, lease_time, 4);
        lease_time_align = ntohl(lease_time_align);
        if (lease_time_align > server_config[ifid].lease)
            lease_time_align = server_config[ifid].lease;
        else if (lease_time_align < server_config[ifid].min_lease)
            lease_time_align = server_config[ifid].lease;
    }
#endif
#ifdef __SC_BUILD__
    if (gw_option_enable == FALSE)
        lease_time_align = PROVISIONAL_LEASE_TIME;
#endif
    add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));

    curr = server_config[ifid].options;
    while (curr) {
        if (curr->data[OPT_CODE] != DHCP_LEASE_TIME && curr->data[OPT_CODE] != DHCP_VENDOR)
        {
#ifdef __SC_BUILD__
            if (curr->data[OPT_CODE] == DHCP_DNS_SERVER)
            {
                if (dns_option_enable)
                    ADD_DNS_OPTIONS();
            }
            else if (curr->data[OPT_CODE] == DHCP_ROUTER)
            {
                if (gw_option_enable)
                    add_option_string(packet.options, curr->data);
            }
            else if (curr->data[OPT_CODE] == DHCP_OPTION_160)
            {
                int len;
                unsigned char *option_160;

                if ((option_160 = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(option_160, DHCP_OPTION_160, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_BOOT_FILE)
            {
                int len;
                unsigned char *boot_file;

                if ((boot_file = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(boot_file, DHCP_BOOT_FILE, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_OPTION_150)
            {
                int len;
                unsigned char *tftp_name;

                if ((tftp_name = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(tftp_name, DHCP_OPTION_150, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_SIP_SERVERS)
            {
                int len;
                unsigned char *outbound_proxy;

                if ((outbound_proxy = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(outbound_proxy, DHCP_SIP_SERVERS, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_DOMAIN_NAME)
            {
                int len;
                unsigned char *domain;

                if ((domain = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(domain, DHCP_DOMAIN_NAME, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_NTP_SERVER)
            {
                int len;
                unsigned char *ntp_server;

                if ((ntp_server = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(ntp_server, DHCP_NTP_SERVER, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_PROVISIONING_SERVER_URL)
            {
                int len;
                unsigned char *url;

                if ((url = get_option_x(oldpacket,DHCP_PARAM_REQ,&len)) != NULL)
                {
                    if(get_option_list(url, DHCP_PROVISIONING_SERVER_URL, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_TFTP)
            {
                int len;
                unsigned char *tftp;

                if ((tftp = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(tftp, DHCP_TFTP, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else
#endif
                add_option_string(packet.options, curr->data);
        }
        curr = curr->next;
    }

    add_bootp_options(&packet, ifid);

    addr.s_addr = packet.yiaddr;

#ifndef __SC_BUILD__
    LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));
#endif
    if (send_packet(&packet, 0, ifid) < 0)
        return -1;
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
            client_mac[0], client_mac[1], client_mac[2],
            client_mac[3], client_mac[4], client_mac[5]);

#ifdef __SC_BUILD__ 
    DEBUG(LOG_INFO, "Successfully assign IP [%s] to client mac [%s], lease time [%d]",
            inet_ntoa(addr), mac, lease_time_align); 
#endif

    if( (fp=fopen(VENDOR_FILE, "a+"))!=NULL )
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
                if(s == NULL)
                {
                    strcat(buf, line);
                }
            }
            fclose(fp);
            if((timefp = fopen("/proc/uptime","r")) != NULL)
            {
                char uptime[10];
                fscanf(timefp,"%s ", uptime);
                fclose(timefp);
                sys_time = atol(uptime);
            }
            else
                sys_time = time(0);
            if( (fp=fopen(VENDOR_FILE, "w"))!=NULL )
            {
                fprintf(fp, "%s", buf);
                fprintf(fp, "%s;%s;%s;%d;%d;\n",
                        vendor_id,
                        inet_ntoa(client_addr),
                        mac,
                        lease_time_align, sys_time);
                fclose(fp);
            }
            free(buf);
        }
        else
        {
            fclose(fp);
        }

    }
#ifdef __SC_BUILD__
    {
        char *tmp=get_option(oldpacket,DHCP_HOST_NAME);
        int len=0;
        if(tmp)
        {
            len=*(tmp-1);

            if(len > sizeof(hostname) - 1)
                len = sizeof(hostname) - 1;

            strncpy(hostname,tmp,len);
            hostname[len] = '\0';
        }
    }

    add_lease(oldpacket->chaddr, packet.yiaddr, lease_time_align, ifid, hostname);
    //write_leases(ifid);
#else
    add_lease(packet.chaddr, packet.yiaddr, lease_time_align, ifid);
#endif
    write_leases(ifid);
    return 0;
}


int send_inform(struct dhcpMessage *oldpacket, int ifid)
{
    struct dhcpMessage packet;
    struct option_set *curr;

    init_packet(&packet, oldpacket, DHCPACK, ifid);

    curr = server_config[ifid].options;
    while (curr) {
        if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
        {
#ifdef __SC_BUILD__
            if (curr->data[OPT_CODE] == DHCP_DNS_SERVER)
            {
                if (dns_option_enable)
                    ADD_DNS_OPTIONS();
            }
            else if (curr->data[OPT_CODE] == DHCP_ROUTER)
            {
                if (gw_option_enable)
                    add_option_string(packet.options, curr->data);
            }
            else if (curr->data[OPT_CODE] == DHCP_OPTION_160)
            {
                int len;
                unsigned char *option_160;

                if ((option_160 = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(option_160, DHCP_OPTION_160, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_BOOT_FILE)
            {
                int len;
                unsigned char *boot_file;

                if ((boot_file = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(boot_file, DHCP_BOOT_FILE, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else if (curr->data[OPT_CODE] == DHCP_TFTP)
            {
                int len;
                unsigned char *tftp;

                if ((tftp = get_option_x(oldpacket, DHCP_PARAM_REQ, &len)) != NULL)
                {
                    if(get_option_list(tftp, DHCP_TFTP, len))
                        add_option_string(packet.options, curr->data);
                }
            }
            else
#endif
                add_option_string(packet.options, curr->data);
        }
        curr = curr->next;
    }

    add_bootp_options(&packet, ifid);
#ifdef __SC_BUILD__ 
    DEBUG(LOG_INFO, "Send Information to client ip [%u.%u.%u.%u] mac [%02x:%02x:%02x:%02x:%02x:%02x]",
            NIPQUAD(packet.ciaddr),
            packet.chaddr[0], packet.chaddr[1], packet.chaddr[2], 
            packet.chaddr[3], packet.chaddr[4], packet.chaddr[5]); 
#endif
    return send_packet(&packet, 0, ifid);
}



