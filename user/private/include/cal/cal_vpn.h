#ifndef __CAL_VPN_H__
#define __CAL_VPN_H__
#if defined(CONFIG_SUPPORT_L2TP_SERVER)||defined(CONFIG_SUPPORT_L2TP_CLIENT)||defined(CONFIG_SUPPORT_IPSEC_CLIENT)||defined(CONFIG_SUPPORT_PPTP_CLIENT)||defined(CONFIG_SUPPORT_PPTP_SERVER)
char *cal_vpn_get_type();
int cal_vpn_set_type(char *value);
#endif
#ifdef CONFIG_SUPPORT_L2TP_SERVER
char *cal_vpn_l2tps_get_start_ip();
int cal_vpn_l2tps_set_start_ip(char *value);
char *cal_vpn_l2tps_get_end_ip();
int cal_vpn_l2tps_set_end_ip(char *value);
char *cal_vpn_l2tps_get_sharedsecret();
int cal_vpn_l2tps_set_sharedsecret(char *value);
char *cal_vpn_l2tps_get_users();
int cal_vpn_l2tps_set_users(char *value);
char *cal_vpn_l2tps_get_local_ip();
int cal_vpn_l2tps_set_users(char *value);
char *cal_vpn_l2tps_get_net_mask();
int cal_vpn_l2tps_set_net_mask(char *value);
#endif

#ifdef CONFIG_SUPPORT_L2TP_CLIENT
char *cal_vpn_l2tpc_get_enable();
char *cal_vpn_l2tpc_get_remote_endpoints();
char *cal_vpn_l2tpc_get_username();
char *cal_vpn_l2tpc_get_password();
char *cal_vpn_l2tpc_get_remoteip();
char *cal_vpn_l2tpc_get_netmask();
char *cal_vpn_l2tpc_get_requiredencryption();
char *cal_vpn_l2tpc_get_sharedsecret();
int cal_vpn_l2tpc_set_enable(char *value);
int cal_vpn_l2tpc_set_remote_endpoints(char *value);
int cal_vpn_l2tpc_set_username(char *value);
int cal_vpn_l2tpc_set_password(char *value);
int cal_vpn_l2tpc_set_remoteip(char *value);
int cal_vpn_l2tpc_set_netmask(char *value);
int cal_vpn_l2tpc_set_requiredencryption(char *value);
int cal_vpn_l2tpc_set_sharedsecret(char *value);
#endif

#ifdef CONFIG_SUPPORT_IPSEC_CLIENT
char *cal_vpn_ipsec_get_enable();
char *cal_vpn_ipsec_get_name();
char *cal_vpn_ipsec_get_remote_endpoints();
char *cal_vpn_ipsec_get_des_ip();
char *cal_vpn_ipsec_get_des_mask();
char *cal_vpn_ipsec_get_source_ip();
char *cal_vpn_ipsec_get_source_mask();
char *cal_vpn_ipsec_get_sharedsecret();
char *cal_vpn_ipsec_get_ike();
char *cal_vpn_ipsec_get_esp();
char *cal_vpn_ipsec_get_mode();
char *cal_vpn_ipsec_get_pfsenable();
char *cal_vpn_ipsec_get_lifetime();
char *cal_vpn_ipsec_get_ikelifetime();

int cal_vpn_ipsec_set_enable(char *value);
int cal_vpn_ipsec_set_name(char *value);
int cal_vpn_ipsec_set_remote_endpoints(char *value);
int cal_vpn_ipsec_set_des_ip(char *value);
int cal_vpn_ipsec_set_des_mask(char *value);
int cal_vpn_ipsec_set_source_ip(char *value);
int cal_vpn_ipsec_set_source_mask(char *value);
int cal_vpn_ipsec_set_sharedsecret(char *value);
int cal_vpn_ipsec_set_ike(char *value);
int cal_vpn_ipsec_set_esp(char *value);
int cal_vpn_ipsec_set_mode(char *value);
int cal_vpn_ipsec_set_pfsenable(char *value);
int cal_vpn_ipsec_set_lifetime(char *value);
int cal_vpn_ipsec_set_ikelifetime(char *value);
#endif
#ifdef CONFIG_SUPPORT_PPTP_CLIENT
int cal_vpn_pptpc_set_dest_host(char *value);
char *cal_vpn_pptpc_get_dest_host();
int cal_vpn_pptpc_set_remote_subnet_ip(char *value);
char *cal_vpn_pptpc_get_remote_subnet_ip();
int cal_vpn_pptpc_set_remote_subnet_mask(char *value);
char *cal_vpn_pptpc_get_remote_subnet_mask();
int cal_vpn_pptpc_set_username(char *value);
char *cal_vpn_pptpc_get_username();
int cal_vpn_pptpc_set_password(char *value);
char *cal_vpn_pptpc_get_password();
int cal_vpn_pptpc_set_encryption_required(char *value);
char *cal_vpn_pptpc_get_encryption_required();
#endif
#ifdef CONFIG_SUPPORT_PPTP_SERVER
int cal_vpn_pptps_set_local_ip(char *value);
char *cal_vpn_pptps_get_local_ip();
int cal_vpn_pptps_set_start_ip(char *value);
char *cal_vpn_pptps_get_start_ip();
int cal_vpn_pptps_set_end_ip(char *value);
char *cal_vpn_pptps_get_end_ip();
int cal_vpn_pptps_set_net_mask(char *value);
char *cal_vpn_pptps_get_net_mask();
int cal_vpn_pptps_set_users(char *value);
char *cal_vpn_pptps_get_users();
#endif
#ifdef CONFIG_SUPPORT_OPENVPN_CLIENT
char *cal_vpn_openvpn_get_remoteip();
int cal_vpn_openvpn_set_remoteip(char *value);
char *cal_vpn_openvpn_get_remoteport();
int cal_vpn_openvpn_set_remoteport(char *value);
#endif
#endif
