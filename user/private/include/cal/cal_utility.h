#ifndef _CAL_UTILITY_H_
#define _CAL_UTILITY_H_

#ifdef VFIE
#define EASYBOX_CFG_NAME    "/mnt/1/configuration.cfg"
#define EASYBOX_USER_CFG_NAME    "/mnt/1/user_config.cfg"
#else
#define EASYBOX_CFG_NAME    "/mnt/0/configuration.cfg"
#define EASYBOX_USER_CFG_NAME    "/mnt/0/user_config.cfg"
#endif
/****************************************************************************
Function name : cal_commit
Description:
	Save configurations into flash
Return:
	0(success) or other value
*****************************************************************************/
int cal_commit(void);

int cal_default(int is_soft_reset);
int cal_partial_default(char *uri);

int cal_get_change_flag(void);

#endif /* _CAL_UTILITY_H_ */
