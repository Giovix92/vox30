#ifndef __RCL_FON_H__
#define __RCL_FON_H__
#define FON_NAME "hotspotd"
#define FON_BIN  "/usr/sbin/hotspotd"

int rcl_wan_fon_cfg_create(void);
int rcl_wan_fon_cfg_reload(void);
int rcl_wan_fon_start(void);
int rcl_wan_fon_stop(int need_reset_whitelist);

#endif
