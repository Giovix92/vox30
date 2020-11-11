/**
 * @file   cal_trust_network.h
 * @author Denny_Zhang@sdc.sercomm.com
 * @date   2011-12-26
 * @brief
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifically
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 *
 *
 */

#ifndef _CAL_TRUST_NETWORK_H_
#define _CAL_TRUST_NETWORK_H_

#define TN_MAX_RULE_NUM             40
#define TN_ENTER_IP_MAX_CHAR        128
typedef struct 
{
	char ipstart[TN_ENTER_IP_MAX_CHAR];
	char ipend[TN_ENTER_IP_MAX_CHAR];
}cal_tn_entry;


int cal_TN_get_rule_index_list(int** index_array);
char *cal_TN_get_enable(void);
void cal_TN_set_enable(char *value);
char *cal_TN_get_min_address(int id);
void cal_TN_set_min_address(int id, char *value);
char *cal_TN_get_max_address(int id);
void cal_TN_set_max_address(int id, char *value);
int cal_TN_delete_rule(int id);
int cal_TN_delete_all_rule(void);
int cal_TN_add_rule(const char *min_address, const char *max_address);
char *cal_acl_get_tr069_mode(void);
char *cal_acl_get_tr069_tn_mode(void);
char *cal_acl_get_icmp_mode(void);
char *cal_acl_get_icmp_tn_mode(void);
char *cal_acl_get_telnet_mode(void);
char *cal_acl_get_telnet_tn_mode(void);
char *cal_acl_get_snmp_mode(void);
char *cal_acl_get_snmp_tn_mode(void);
char *cal_acl_get_http_mode(void);
char *cal_acl_get_http_tn_mode(void);
char *cal_acl_get_https_mode(void);
char *cal_acl_get_https_tn_mode(void);
char *cal_acl_get_ssh_mode(void);
char *cal_acl_get_ssh_tn_mode(void);
#ifdef SUPPORT_VOIP
char *cal_acl_get_voip_mode(void);
char *cal_acl_get_voip_tn_mode(void);
#endif
#ifdef SUPPORT_USB_STORAGE
char *cal_acl_get_ftp_mode(void);
char *cal_acl_get_ftp_tn_mode(void);
char *cal_acl_get_ftps_mode(void);
char *cal_acl_get_ftps_tn_mode(void);
#endif
#ifdef SUPPORT_USB_PRINTER
char *cal_acl_get_printers_mode(void);
char *cal_acl_get_printers_tn_mode(void);
#endif
int cal_acl_set_tr069_mode(char *value);
int cal_acl_set_tr069_tn_mode(char *value);
int cal_acl_set_icmp_mode(char *value);
int cal_acl_set_icmp_tn_mode(char *value);
int cal_acl_set_telnet_mode(char *value);
int cal_acl_set_telnet_tn_mode(char *value);
int cal_acl_set_snmp_mode(char *value);
int cal_acl_set_snmp_tn_mode(char *value);
int cal_acl_set_http_mode(char *value);
int cal_acl_set_http_tn_mode(char *value);
int cal_acl_set_https_mode(char *value);
int cal_acl_set_https_tn_mode(char *value);
int cal_acl_set_ssh_mode(char *value);
int cal_acl_set_ssh_tn_mode(char *value);
#ifdef SUPPORT_VOIP
int cal_acl_set_voip_mode(char *value);
int cal_acl_set_voip_tn_mode(char *value);
#endif
#ifdef SUPPORT_USB_STORAGE
int cal_acl_set_ftp_mode(char *value);
int cal_acl_set_ftps_mode(char *value);
int cal_acl_set_ftp_tn_mode(char *value);
int cal_acl_set_ftps_tn_mode(char *value);
#endif
#ifdef SUPPORT_USB_PRINTER
int cal_acl_set_printers_mode(char *value);
int cal_acl_set_printers_tn_mode(char *value);
#endif
char *cal_acl_get_tr069_status(void);
char *cal_acl_get_icmp_status(void);
char *cal_acl_get_telnet_status(void);
char *cal_acl_get_snmp_status(void);
char *cal_acl_get_http_status(void);
char *cal_acl_get_https_status(void);
char *cal_acl_get_ssh_status(void);
#ifdef SUPPORT_USB_STORAGE
char *cal_acl_get_ftp_status(void);
char *cal_acl_get_ftps_status(void);
#endif
#ifdef SUPPORT_USB_PRINTER
char *cal_acl_get_printers_status(void);
#endif
char *cal_acl_get_status(void);
char *cal_acl_get_list_num(void);
#endif

