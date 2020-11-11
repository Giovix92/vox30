#define TYPE_HOST_NAME 1
#define TYPE_STATUS    2
#define TYPE_SESSION   3
#define TYPE_AUTH_CLIENT      4
#define TYPE_CMAX      5
#define TYPE_AUTH_SUM  6

#ifdef __SC_BUILD__
#define ACTION_RELOAD 2
#define ACTION_START 1
#define ACTION_STOP  0

#ifdef CONFIG_SUPPORT_DSL
int fon_rpc_wan_action(int wan_id, char* wan_if, char* wan_mac, int ulink_speed, int dlink_speed,int action, int dsl_if );
#else
int fon_rpc_wan_action(int wan_id, char* wan_if, char* wan_mac, int dlink_speed,int action );
#endif
int fon_rpc_wlan_action(int action, char* main_wifi_enable, char* main_5g_wifi_enable);
char *fon_rpc_get_server_status();
int fon_rpc_update_auth_client_to_dnrd(void);
#endif
char *fon_rpc_get_client_info(int id, int type);
char *fon_rpc_get_client_count(int type);

