
#ifndef _CAL_DHCPV6_H_
#define _CAL_DHCPV6_H_

#define DHCPV6_OPTION_ERR "0"
#define DHCPV6_OPTION_DNS "23"
#define DHCPV6_OPTION_IAPD "25"
#define DHCPV6_OPTION_REFRESH_TIME "32"

typedef enum
{
    CAL_LAN_DHCPV6_ID_DEFAULT = 1,
}CAL_LAN_DHCPV6_ID;

/************************Server********************************/
char *cal_dhcpv6_get_dhcpv6_server_enable(void);
int cal_dhcpv6_set_dhcpv6_server_enable(char *value);

/************************Server.Pool********************************/
char *cal_dhcpv6_get_server_pool_enable(int lanDev_id);
int cal_dhcpv6_set_server_pool_enable(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_order(int lanDev_id);
int cal_dhcpv6_set_server_pool_order(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_if(int lanDev_id);
int cal_dhcpv6_set_server_pool_if(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_duid(int lanDev_id);
int cal_dhcpv6_set_server_pool_duid(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_source_ip(int lanDev_id);
int cal_dhcpv6_set_server_pool_source_ip(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_iana_enable(int lanDev_id);
int cal_dhcpv6_set_server_pool_iana_enable(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_iana_prefix(int lanDev_id);
int cal_dhcpv6_set_server_pool_iana_prefix(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_iapd_enable(int lanDev_id);
int cal_dhcpv6_set_server_pool_iapd_enable(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_iapd_prefix(int lanDev_id);
int cal_dhcpv6_set_server_pool_iapd_prefix(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_iapd_prefixes(int lanDev_id);
int cal_dhcpv6_set_server_pool_iapd_prefixes(int lanDev_id, char *value);

char *cal_dhcpv6_get_server_pool_iapd_length(int lanDev_id);
int cal_dhcpv6_set_server_pool_iapd_length(int lanDev_id, char *value);

/************************Server.Pool.Option********************************/
int cal_dhcpv6_add_server_pool_option_entry(int lanDev_id, char *tag_value);
int cal_dhcpv6_del_server_pool_option_entry(int lanDev_id, char *tag_value);

char *cal_dhcpv6_get_server_pool_option_enable(int lanDev_id, char *tag_value);
int cal_dhcpv6_set_server_pool_option_enable(int lanDev_id, char *tag_value, char *value);

char *cal_dhcpv6_get_server_pool_option_tag(int lanDev_id, int option_id);
int cal_dhcpv6_set_server_pool_option_tag(int lanDev_id, int option_id, char *value);

char *cal_dhcpv6_get_server_pool_option_value(int lanDev_id, char *tag_value);
int cal_dhcpv6_set_server_pool_option_value(int lanDev_id, char *tag_value, char *value);

/************************Server.Pool.Client********************************/
int cal_dhcpv6_add_server_pool_client_entry(int lanDev_id, int client_id);
int cal_dhcpv6_del_server_pool_client_entry(int lanDev_id, int client_id);

char *cal_dhcpv6_get_server_pool_client_ip(int lanDev_id, int client_id);
int cal_dhcpv6_set_server_pool_client_ip(int lanDev_id, int client_id, char *value);

char *cal_dhcpv6_get_server_pool_client_active(int lanDev_id, int client_id);
int cal_dhcpv6_set_server_pool_client_active(int lanDev_id, int client_id, char *value);

/************************Server.Pool.Client.IP********************************/
int cal_dhcpv6_add_server_pool_client_ipv6_entry(int lanDev_id, int client_id, int ip_id);
int cal_dhcpv6_del_server_pool_client_ipv6_entry(int lanDev_id, int client_id, int ip_id);

char *cal_dhcpv6_get_server_pool_client_ipv6_ip(int lanDev_id, int client_id, int ip_id);
int cal_dhcpv6_set_server_pool_client_ipv6_ip(int lanDev_id, int client_id, int ip_id, char *value);

char *cal_dhcpv6_get_server_pool_client_ipv6_prefer_time(int lanDev_id, int client_id, int ip_id);
int cal_dhcpv6_set_server_pool_client_ipv6_prefer_time(int lanDev_id, int client_id, int ip_id, char *value);

char *cal_dhcpv6_get_server_pool_client_ipv6_valid_time(int lanDev_id, int client_id, int ip_id);
int cal_dhcpv6_set_server_pool_client_ipv6_valid_time(int lanDev_id, int client_id, int ip_id, char *value);

/************************Server.Pool.Client.Prefix********************************/
int cal_dhcpv6_add_server_pool_client_prefix_entry(int lanDev_id, int client_id, int prefix_id);
int cal_dhcpv6_del_server_pool_client_prefix_entry(int lanDev_id, int client_id, int prefix_id);

char *cal_dhcpv6_get_server_pool_client_prefix_prefix(int lanDev_id, int client_id, int prefix_id);
int cal_dhcpv6_set_server_pool_client_prefix_prefix(int lanDev_id, int client_id, int prefix_id, char *value);

char *cal_dhcpv6_get_server_pool_client_prefix_prefer_time(int lanDev_id, int client_id, int prefix_id);
int cal_dhcpv6_set_server_pool_client_prefix_prefer_time(int lanDev_id, int client_id, int prefix_id, char *value);

char *cal_dhcpv6_get_server_pool_client_prefix_valid_time(int lanDev_id, int client_id, int prefix_id);
int cal_dhcpv6_set_server_pool_client_prefix_valid_time(int lanDev_id, int client_id, int prefix_id, char *value);

/************************Server.Pool.Client.Option********************************/
int cal_dhcpv6_add_server_pool_client_option_entry(int lanDev_id, int client_id, int option_id);
int cal_dhcpv6_del_server_pool_client_option_entry(int lanDev_id, int client_id, int option_id);

char *cal_dhcpv6_get_server_pool_client_option_tag(int lanDev_id, int client_id, int option_id);
int cal_dhcpv6_set_server_pool_client_option_tag(int lanDev_id, int client_id, int option_id, char *value);

char *cal_dhcpv6_get_server_pool_client_option_value(int lanDev_id, int client_id, int option_id);
int cal_dhcpv6_set_server_pool_client_option_value(int lanDev_id, int client_id, int option_id, char *value);

#endif /* _CAL_DHCPV6_H_ */
