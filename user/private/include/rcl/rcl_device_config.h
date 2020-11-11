/**
 * @file   rcl_user_account.h
 * @author Denny_Zhang@sdc.sercomm.com
 * @date   2011-12-14
 * @brief  account RC API
 *
 * Copyright - 2010 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */

#ifndef _RCL_DEVICE_CONFIG_H_
#define _RCL_DEVICE_CONFIG_H_

struct config_head_t{
    char            hw_id[32];
    char            description[256];
    char            password[64];
    char            create_time[32];
    char            fw_version[32];
    char            group[16];
    unsigned int    cfg_length;
    unsigned int    call_log_length;
};
int rcl_backup_cfg(char *path, char *description, char *password, char *group);

int rcl_restore_cfg(char* path, char *module, char *ipaddr, char *password, char *group, char *url_list);
int rcl_init_ft_tool(void);
#ifdef CONFIG_SUPPORT_5G_QD 
int rcl_init_tftpd(void);
#endif
#endif
