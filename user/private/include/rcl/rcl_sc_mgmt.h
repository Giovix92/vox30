#ifndef __RCL_SC_MGMT_H__
#define __RCL_SC_MGMT_H__

int rcl_start_upnp(void);
int rcl_stop_upnp(void);
void rcl_init_tr069_env(void);
int rcl_update_tr069_connection_request_port(void);

#endif
