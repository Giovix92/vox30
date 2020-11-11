/**warning
  *the max length of the pointer is MAX_NAT_SESSION_LEN,so one line of the file cann't overflow the max length
  *the caller should define the size of the pointer array (MAX_NAT_CONNTRACK)
**/
#ifndef __SAL_NAT_H__
#define __SAL_NAT_H__
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#define MAX_NAT_SESSION_LEN 512
struct sta_nat_conntrack
{
    int total;
    int used;
};
typedef struct upnp_natt_table_s
{
    char PortMappingEnabled[16];
    char Protocol[16];
    char ExternalPort[16];
    char InternalPort[16];
    char InternalClient[32];
    char lease[32];
    char description[256];
} upnp_natt_table_t;
struct rdr_desc {
    struct rdr_desc * next;
    struct rdr_desc * first;
    unsigned short eport;
    unsigned short iport;
    unsigned short rulenable;
    short proto;
    char addr[32];
    char str[256];
    /* Enhance UPnP with leaseTime, Douglas @ 20080521 */
    /* These 2 times can NOT combine into one time: expiredTime = serviceStartTime + duration */
    time_t serviceStartTime;  /* seconds */
    time_t duration;          /* seconds */
};
#define SHARE_UPNP_DESC "/tmp/share_upnp_desc"
#define SHARE_UPNP_SIZE (sizeof(struct rdr_desc)*MAX_UPNP_DESC)
#define MAX_UPNP_DESC   100
/**
 *@fn int get_nattable_statistic(struct sta_nat_conntrack *sta,char *nat_conntrack[]);
 *@brief get the nattable statistic include total and used
 *@param[in] pointer of structure 
 *@param[out] the structure of statistic
 *@warning
        the parameter MUST be assured to be valid memory by the caller
        the memory located must be free by the caller
**/
int get_nattable_statistic(struct sta_nat_conntrack *sta);
/**
  *@int get_nat_table_session(char *nat_conntrack[],int nat_entry_num);
  *@brief get the nattable session
  *@param[in] pointer array and the MAX_NAT_CONNTRACK 
  *@param[out] the pointer array
**/        
int get_nattable_number_of_open_ports();
typedef struct nat_conntrack_p_s
{
    char s_ip[64];
    char d_ip[64];
    char s_port[8];
    char d_port[8];
    char packets[16];
    char bytes[16];
    char in_index[8];
    char out_index[8];
	char time_expire[8];//time to expire
}nat_conntrack_p_t;
struct nat_conntrack_t
{
    char name[16];
    char pro_name[16];
    char state[16];
    nat_conntrack_p_t send;
    nat_conntrack_p_t reply;
    char local[2];
    struct nat_conntrack_t *next;
};
struct nat_conntrack_list {
    char ip[64];
    char eth_pro[16];
    struct nat_conntrack_t *entry;
    struct nat_conntrack_list *next;
};
#define sal_get_nat_table_next_entry(p)  ((p)->next)
int sal_get_nat_table_session(void);
struct nat_conntrack_t* sal_get_nat_table_list_by_ip(char *ip);
struct nat_conntrack_t* sal_get_nat_table_next_list_entry(void);
struct nat_conntrack_t* sal_get_nat_table_list_head(void);
struct nat_conntrack_t* sal_get_nat_table_next_list(void);
void sal_nat_table_destory(void);
int sal_get_upnp_natt_table(upnp_natt_table_t **upnp_natt_tb);
#endif
