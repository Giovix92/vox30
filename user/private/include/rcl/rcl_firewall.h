#ifndef __RCL_FIREWALL_H__
#define __RCL_FIREWALL_H__

/*--------------- COMMON -------------*/
#ifdef _SC_SPI_FIREWALL_
int rcl_init_spi_env(int (*system)(const char *format, ...));
int rcl_fw_start_sc_fw(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_sc_fw(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_start_dos_lan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_dos_lan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_start_dos_wan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_dos_wan(int argc, char** argv, int (*system)(const char *format, ...));
#endif
int rcl_fw_start_input_mgnt_lan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_input_mgnt_lan(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_input_mgnt_guest(int argc, char** argv, int (*system_func)(const char *format, ...));
int rcl_fw_stop_input_mgnt_guest(int argc, char** argv, int (*system_func)(const char *format, ...));

int rcl_fw_start_ping2wan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_ping2wan(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_lan_src(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_lan_src(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_dns_proxy(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_dns_proxy(int argc, char** argv, int (*system)(const char *format, ...));

/* alg */
int rcl_fw_start_alg_h323(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_alg_h323(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_start_alg_sip(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_alg_sip(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_start_alg_rtsp(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_alg_rtsp(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_start_alg_ftp(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_alg_ftp(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_start_alg_tftp(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_alg_tftp(int argc, char** argv, int (*system)(const char *format, ...));

/* vpn passthrough */
int rcl_fw_start_vps_ipsec(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_vps_ipsec(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_start_vps_pptp(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_vps_pptp(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_start_vps_l2tp(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_vps_l2tp(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_policy_lan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_policy_lan(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_pctrl(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_pctrl(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_url_filter(int argc,char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_url_filter(int argc,char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_protect_lan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_protect_lan(int argc, char** argv, int (*system)(const char *format, ...));
#ifdef CONFIG_SUPPORT_WIFI_MSSID
int rcl_fw_start_protect_wifi(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_protect_wifi(int argc, char** argv, int (*system)(const char *format, ...));
#endif
#ifdef CONFIG_SUPPORT_BOOSTER_MODE
int rcl_fw_start_booster_mode(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_booster_mode(int argc, char** argv, int (*system)(const char *format, ...));
#endif
/*---------------- WAN ---------------*/
int rcl_fw_start_input_mgnt_wan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_input_mgnt_wan(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_nat(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_nat(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_spi(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_spi(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_spi_lan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_spi_lan(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_mss(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_mss(int argc, char** argv, int (*system)(const char *format, ...));


int rcl_fw_start_dmz(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_dmz(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_port_trg(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_port_trg(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_port_map(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_port_map(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_policy_wan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_policy_wan(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_protect_wan(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_protect_wan(int argc, char** argv, int (*system)(const char *format, ...));

int rcl_fw_start_trust_network(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_trust_network(int argc, char** argv, int (*system)(const char *format, ...));

#ifdef CONFIG_SUPPORT_STATIC_NAT
int rcl_fw_start_static_nat(int argc, char** argv, int (*system)(const char *format, ...));
int rcl_fw_stop_static_nat(int argc, char** argv, int (*system)(const char *format, ...));
#endif

#endif
