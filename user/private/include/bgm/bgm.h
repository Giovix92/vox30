/**
 * @file   unim_client.h
 * @author Martin_Huang@sdc.sercomm.com
 * @date   2011-10-10
 * @brief  notify mechanism 
 *         client will notify unim that uni configration has been updated 
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */

#ifndef __BGM_H__
#define __BGM_H__
#include <net/if.h>

#define BGM_CFG_BASE     "/tmp/1/"
#define BGM_SOCK_NAME    BGM_CFG_BASE"sname"
#define BGM_SOCK_VOIP    BGM_CFG_BASE"svoip"
#define BGM_L2_DDB       BGM_CFG_BASE"ddb"
#define BGM_L2_DDB_LOCK  BGM_CFG_BASE"ddb_lock"
#define BGM_VOIP_APPLY_LOCK  BGM_CFG_BASE"voip_apply_lock"

#ifdef CONFIG_SUPPORT_DT_TEST
#if defined FD1018 || defined VOX25 || defined ESSENTIAL || defined(VOX30)
#define BGM_MAX_EMAC_NUM           5
#else
#define BGM_MAX_EMAC_NUM           4
#endif
#else
#if defined FD1018 || defined VOX25 || defined ESSENTIAL || defined(VOX30)
#define BGM_MAX_EMAC_NUM           4
#else
#ifdef CONFIG_SUPPORT_L2BRIDGING
#define BGM_MAX_EMAC_NUM           4
#else
#define BGM_MAX_EMAC_NUM           3
#endif
#endif
#endif
#define BGM_MAX_PORT_NUM_PER_EMAC  1
#define BGM_MAX_UNI_NUM            (BGM_MAX_EMAC_NUM * BGM_MAX_PORT_NUM_PER_EMAC )


#define BGM_PORT_BRIDGING          (0x1 << 1)
#define BGM_PORT_ROUTING           (0x1)
#define BGM_DHCP_PROVISION_IP      (0x1)
#define BGM_DHCP_PROVISION_GATEWAY (0x1 << 1)
#define BGM_DHCP_PROVISION_DNS     (0x1 << 2)
typedef struct tag_BGM_PORT_INFO{
    char used;
    char index;
    char state;
    char tag_num;
    char br_name[IFNAMSIZ];
    char port_name[IFNAMSIZ];
    char bridging;
    char dhcp_provision;
    int type_bit;
    int port_bit;
    int flood_map;
    int port_state_bit;
    char ether_type[256];
    int inner_vid;
    int inner_pbit;
    int outer_vid;
    int outer_pbit;
#ifdef CONFIG_SUPPORT_L2BRIDGING
    long related[BGM_MAX_UNI_NUM];
    int snooping;
    char sip[32];
#else
    struct tag_BGM_PORT_INFO *related[BGM_MAX_UNI_NUM];
#endif
}BGM_PORT_INFO_t;

typedef struct tag_BGM_COMMAND{
    int cmd;
    int sub_cmd;
    char data[256];
}BGM_COMMAND_t;

enum {
    BGM_CLIENT_NOTIFY_CMD,
    BGM_CLIENT_VOIP_NOTIFY_CMD,
    BGM_CLIENT_UNI_NOTIFY_CMD,
};
enum {
    BGM_CLIENT_USER,
    BGM_CLIENT_OMCI,
};

int bgm_get_port_num(int bit_map);
int bgm_uni_port_map2phyport(int uni_port);
int bgm_update_notified_by_omci(void);
int bgm_voip_update_notified_by_omci(void);
int bgm_uni_update_notified_by_omci(void);
int bgm_update_notified_by_user(void);
int bgm_client_get_non_bridging_lan_port_info(char *file_name_buf, int buf_len);
unsigned int *bgm_get_unconfigured_ports(int *port_num);
unsigned int *bgm_get_routing_ports(int *port_num);
void bgm_set_voip_under_configuring(void);
void bgm_clear_voip_under_configuring(void);
int bgm_is_voip_under_configuring(void);

#endif


