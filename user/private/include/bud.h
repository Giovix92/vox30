/**
 * @file   main.c
 * @author David_Wang@sdc.sercomm.com
 * @date   2013-12-19
 * @brief  support service mangement if there is wan backup  
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */

#ifndef __BUD_H__
#define __BUD_H__
#include <net/if.h>

#define BUD_DEFAULT_SOCK_TIMEOUT   5
#define BUD_CFG_BASE     "/tmp/1/"
#define BUD_SOCK_NAME    BUD_CFG_BASE"bud"

#define BUD_BACKUP_AUTO_MODE "Auto"
#define BUD_BACKUP_MANUAL_MODE "Manual"
#define BUD_BACKUP_MANUAL_DATA_VOICE "Data_Voice"
#define BUD_BACKUP_MANUAL_DATA "Data"
#define BUD_BACKUP_MANUAL_VOICE "Voice"


enum
{
    BUD_SET_CMD,
    BUD_GET_CMD,
    BUD_REGISTER_CMD,
    BUD_UNREGISTER_CMD,
};
enum
{
    BUD_GET_ACTIVE_DATA_WAN_SUB_CMD,
    BUD_GET_ACTIVE_VOICE_WAN_SUB_CMD,
    BUD_GET_ACTIVE_IPTV_WAN_SUB_CMD,
    BUD_SET_PRIMARY_WAN_STATUS_SUB_CMD,
    BUD_SET_BACKUP_WAN_STATUS_SUB_CMD,
};

#define  BUD_MANUAL_BACKUP_DATA         (0x1)
#define  BUD_AUTO_BACKUP_DATA           (0x1 << 1)
#define  BUD_AUTO_BACKUP_VOICE_PS       (0x1 << 2)
#define  BUD_MANUAL_BACKUP_VOICE_PS     (0x1 << 3)
#define  BUD_AUTO_BACKUP_VOICE_CS       (0x1 << 4)
#define  BUD_MANUAL_BACKUP_VOICE_CS     (0x1 << 5)
#define  BUD_AUTO_DATA                  (0x1 << 6)
#define  BUD_AUTO_VOICE                 (0x1 << 7)
#define  BUD_AUTO_IPTV                  (0x1 << 8)
#define  BUD_AUTO_BACKUP_IPTV           (0x1 << 9)

typedef struct tag_bud_cfg_info{
    int cid; 
    int mode;
    int bid;
    int data_p2b_delay;
    int voice_p2b_delay;
    int data_b2p_delay;
    int voice_b2p_delay;
    int prio;
}BUD_CFG_INFO_t;

typedef struct tag_bud_sts_info{
    int cid; 
#define BUD_WAN_LINK_ACTIVE      0x1 
#define BUD_WAN_SERVICE_ACTIVE  (0x1 << 1) 
    int state;/*1, active, 0 inactive*/
}BUD_STS_INFO_t;

typedef struct tag_bud_srv_info{
    int cid; 
    int state;/*1, active, 0 inactive*/
}BUD_SRV_INFO_t;

typedef struct tag_BUD_COMMAND{
    int cmd;
    int sub_cmd;
    union
    {
        BUD_CFG_INFO_t cfg_info;
        BUD_STS_INFO_t sts_info;
        BUD_SRV_INFO_t srv_info;
        char data[256];
    }data;
}BUD_COMMAND_t;


int bud_register(BUD_CFG_INFO_t *info);
int bud_degister(BUD_CFG_INFO_t *info);
int bud_update_slave_wan_status(BUD_STS_INFO_t *info);
int bud_update_primary_wan_status(BUD_STS_INFO_t *info);
int bud_get_active_data_wan(void);
int bud_get_active_data_wan_r(void);
int bud_get_active_voip_wan(void);
int bud_get_active_voip_wan_r(void);
int bud_read_socket(int socket, char* buffer, int length, int timeout);
#endif


