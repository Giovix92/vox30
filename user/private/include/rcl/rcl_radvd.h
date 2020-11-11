
#ifndef _RCL_RADVD_H_
#define _RCL_RADVD_H_

#define RADVD_NAME "radvd"
#define RADVD_CONF "/var/radvd/radvd.conf"
#define RADVD_DIR  "/var/radvd/"

int rcl_lan_radvd_cfg_create(void);
int rcl_lan_radvd_start(void);
int rcl_lan_radvd_stop(void);

#endif /* _RCL_RADVD_H_ */
