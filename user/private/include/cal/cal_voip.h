
#ifndef _CAL_VOIP_H_
#define _CAL_VOIP_H_

#define CAL_VOIP_SERVICE_DEFAULT 1
typedef enum
{
    CAL_VOIP_PROFILE_DEFAULT = 1,
#ifdef SUPPORT_MULTI_VOICE_PROFILE
    CAL_VOIP_PROFILE_MAX = 2,
#else
    CAL_VOIP_PROFILE_MAX = 1,
#endif
} CAL_VOIP_PROFILE;

typedef enum
{
    CAL_VOIP_LINE_ID_1 = 1,
    CAL_VOIP_LINE_ID_2,
    CAL_VOIP_LINE_ID_END,
} CAL_VOIP_LINE_ID;

typedef enum
{
    CAL_VOIP_TONE_ID_DIAL = 1,
    CAL_VOIP_TONE_ID_BUSY,
    CAL_VOIP_TONE_ID_REORDER,
    CAL_VOIP_TONE_ID_ALTER,
    CAL_VOIP_TONE_ID_REMOTE_CALLWAITING,
    CAL_VOIP_TONE_ID_CALLWAITING,
    CAL_VOIP_TONE_ID_RINGBACK,
    CAL_VOIP_TONE_ID_REMOTE_CALLHOLD,
    CAL_VOIP_TONE_ID_CALLHOLD,
    CAL_VOIP_TONE_ID_REJECTION,
    CAL_VOIP_TONE_ID_CONFIRMATION,
    CAL_VOIP_TONE_ID_RELEASE,
    CAL_VOIP_TONE_ID_MWI,
    CAL_VOIP_TONE_ID_SPECIAL_DIAL,
    CAL_VOIP_TONE_ID_STUTTER_DIAL,
    CAL_VOIP_TONE_ID_BARGEIN,
    CAL_VOIP_TONE_ID_END
} CAL_VOIP_TONE_ID;

typedef enum
{
    CAL_VOIP_CODEC_ID_G711MULAW = 1,
    CAL_VOIP_CODEC_ID_G711ALAW,
    CAL_VOIP_CODEC_ID_G729,
    CAL_VOIP_CODEC_ID_G722,
    CAL_VOIP_CODEC_ID_AMR,
    CAL_VOIP_CODEC_ID_END
} CAL_VOIP_CODEC_ID;

/* The CAL_VOIP_RETURN_TYPE is just used in the cal_voip_get_xxx func. */
typedef enum
{
    CAL_VOIP_RETURN_TYPE_NORMAL = 0,    /* return the original value */
    CAL_VOIP_RETURN_TYPE_APPCONF,       /* voip application config file use */
} CAL_VOIP_RETURN_TYPE;


typedef enum
{
    CAL_VOIP_LOG_LEVEL_LOW = 0,
    CAL_VOIP_LOG_LEVEL_MID = 2,
    CAL_VOIP_LOG_LEVEL_HIGH = 4,
    CAL_VOIP_LOG_LEVEL_OFF = 6,
} CAL_VOIP_LOG_LEVEL;


#define CAL_VOIP_ENABLE_DEFAULT_LEN 1
#define CAL_VOIP_CONFIG_LEN_MAX     1024
#define VOICE_MAX_FXS                2

#define VOICE_MAX_LINE_NUM              15
#define VOICE_MAX_IPPHONE_NUM            4

#ifdef SC_VOIP_SUPPORT_PBX
#define VOICE_MAX_SIP_EXTENSION_NUM     4
#else
#define VOICE_MAX_SIP_EXTENSION_NUM     0
#endif

#define TOTAL_EXTENSION_NUM    (VOICE_MAX_SIP_EXTENSION_NUM + VOICE_LINE_ID_MAX)
typedef struct cal_line_map_s
{
    int  line_id;
    char directNumber[64];
    int  profile_id;
    int  inLineId;
}cal_line_map_t;

typedef struct cal_voice_callctrl_map_s
{
    char bEnable;
    int  extID;
    int  inMapNum;
    int  inMap[VOICE_MAX_LINE_NUM];
    int  outMap;
    char szAlternativeLineRef[33];
    int  unAlternativeLineIndex;
    char szExtenName[260];
    char szExtenType[12];
}cal_voice_callctrl_map_t;

typedef struct outgoing_entry
{
    char id[64];
    char connection[64];
    char internal_number[64];
    char name[64];
    char select_id[3][64];
}OUTGOING_ENTRY;

typedef struct incoming_entry
{
    char id[64];
    char type[64];
    char number[64];
    char name[64];
    char check_id[6][64];
}INCOMING_ENTRY;

typedef struct ipphone_setting_entry 
{
    char id[8];
    char ring[8];
}IPPHONE_SETTING_ENTRY;

typedef struct phone_setting_entry 
{
    char id[64];
    char cw_enable[64];
    char cf_enable[64];
    char cf_mode[64];
    char cf_number[64];
    char vm_enable[64];
    char vm_mode[64];
    char noti_enable[64];
    char noti_number[64];
    char cordless_enable[64];
}PHONE_SETTING_ENTRY;

typedef struct phone_voip_entry 
{
    char profile_id[64];
    char type[64];
    char profile_name[64];
    char primary_registrar[64];
    char primary_registrar_port[64];
    char secondary_registrar[64];
    char secondary_registrar_port[64];
    char primary_proxy[64];
    char primary_proxy_port[64];
    char outbound_proxy[64];
    char outbound_proxy_port[64];
    char sip_domain[64];
    char local_port[64];
    char outbound_enable[64];
    char secondary_server_enable[64];
    char secondary_server_addr[64];
    char secondary_server_port[64];
    char line_id[64];
    char provider[64];
    char sip_number[64];
    char username[64];
    char password[128];
    char line_enable[64];
}PHONE_VOIP_ENTRY;

typedef struct phone_connections2_entry 
{
    char line_id[64];
    char codec[64];
    char dtml_mode[64];
    char fax_codec[64];
    char packetization_time[64];
    char comfort_noise[64];
    char silence_suppression[64];
    char area_code_enable[64];
    char area_code_number[64];
}PHONE_CONNECTIONS2_ENTRY;

typedef struct phone_settings2_entry 
{
    char inter_dig_tout[64];
    char dial_tone_duration[64];
    char min_hookflash_time[64];
    char max_hookflash_time[64];
}PHONE_SETTINGS2_ENTRY;
/*****************************************************************************

Function: cal_voip_get_xxx()
          Return: 1) NULL -- get error
                  2) A static buffer pointer, which include the value string.

Function: cal_voip_set_xxx()
          Return: 1) 0  -- success
                  2) !0 -- fail
                  
Parameters:
          int prof      -- defined in enum CAL_VOIP_PROFILE
                           This parameter is for functional extention in the future.
                           Maybe it is used to identify different type config(flash/ram).
                           Now there is just one value, and no practical use.
                           
          int line_id   -- defined in enum CAL_VOIP_LINE_ID
          
          int tone_id   -- defined in enum CAL_VOIP_TONE_ID
          
          int codec_id  -- defined in enum CAL_VOIP_CODEC_ID
          
          int ret_type  -- defined in enum CAL_VOIP_RETURN_TYPE
                           This parameter is just used in the cal_voip_get_xxx func.
          
******************************************************************************/
char *cal_voip_get_backup_fax_pos_enable();
char *cal_voip_set_backup_fax_pos_enable(char *val);
int cal_voice_get_vgw_all_line_id(int line[], int maxNum);
char *cal_incoming_get_check_id1(int id);
char *cal_incoming_get_check_id2(int id);
char *cal_voip_get_lan_sip_extension_number(int id);
int cal_voice_line_id_get(int id0, int id1, int id2);
int cal_voice_line_id_set(int id0, int id1, int id2, int val);
char *cal_outgoing_get_select_id1(int id); 
char *cal_outgoing_get_select_id2(int id); 
char *cal_outgoing_get_select_id3(int id); 
char *cal_extension_get_in_gsm(int id);
char *cal_extension_get_out_gsm(int id);
char *cal_voip_get_voice_profile_name(int prof,int service);
int cal_voip_set_voice_profile_name(int service,int prof, char *value);
int cal_phone_setting_add_entry(PHONE_SETTING_ENTRY *item);
int cal_phone_voip_provider_add_entry(PHONE_VOIP_ENTRY *item);
int cal_phone_connections2_add_entry(PHONE_CONNECTIONS2_ENTRY *item);
int cal_phone_connections2_set_codec(char *buf,int line_id);
int cal_phone_voip_number_add_entry(PHONE_VOIP_ENTRY *item,int username_set,int password_set);
int cal_phone_voip_number_delete(int index);
char *cal_voip_get_add_prefix0_enable(int prof,int id);
int cal_voip_set_add_prefix0_enable(int prof,int id,char  *val);
char *cal_voip_get_reset_voice_statistics_disable(int prof,int id);
int cal_voip_set_reset_voice_statistics_disable(int prof,int id,char  *val);


#ifdef SC_VOIP_SUPPORT_PBX
char *cal_voip_get_ipphone_ringenable(int id);
int cal_voip_get_ipphone_number();
int cal_voip_set_ipphone_ringenable(IPPHONE_SETTING_ENTRY *item);
char *cal_voip_get_ipphone_username(int id);
int cal_voip_set_ipphone_username(int id,char  *val);
char *cal_voice_callConctrl_inMap_lineRef_get(int id0, int id1);
char *cal_voice_callConctrl_inMap_extensionRef_get(int id0, int id1);
char *cal_voice_callConctrl_outMap_lineRef_get(int id0, int id1);
char *cal_voice_callConctrl_outMap_extensionRef_get(int id0, int id1);
int cal_voice_get_total_in_out_map(cal_voice_callctrl_map_t * ctrlMap, int maxSize);
char *cal_voip_get_lan_sip_name(int id);
int cal_voip_set_lan_sip_name(int id, char *value);
char *cal_voip_get_lan_sip_password(int id);
int cal_voip_set_lan_sip_password(int id, char *value);
char *cal_voip_get_pbx_reg_auth(int id);
int cal_voip_set_pbx_reg_auth(int id, char *value);
char *cal_voip_get_pbx_enable();
int cal_voip_set_pbx_enable(char *value);
char *cal_voip_get_pbx_app_register_enable();
int cal_voip_set_pbx_app_register_enable(char *value);
char *cal_voip_get_pbx_max_register_counts();
int cal_voip_set_pbx_max_register_counts(char *value);
char *cal_voip_get_dynamic_callerid_enable(int id);
char *cal_voip_get_pbx_outgoing_trunk_id();
int cal_voip_set_pbx_outgoing_trunk_id(char *value);

int cal_voip_get_lan_sip_num(void);
char *cal_cfg_voice_extensionNumber_get(int prof, int id);
char *cal_cfg_voice_extensionType_get(int prof, int id);
char *cal_cfg_voice_extensionName_get(int prof, int id);
int cal_incoming_get_num(void);
int cal_incoming_del_all(void);
int cal_incoming_add_entry(INCOMING_ENTRY *item);
char *cal_incoming_get_check_id3(int id);
char *cal_incoming_get_check_id4(int id);
char *cal_incoming_get_check_id5(int id);
char *cal_incoming_get_check_id6(int id);
int cal_outgoing_del_all(void);
int cal_outgoing_add_entry(OUTGOING_ENTRY *item);

char *cal_voip_get_black_list_enable();
int cal_voip_set_black_list_enable(char *value);
int cal_voice_add_block_count(char *number);
int cal_voice_del_block_count_index(int index);
char *cal_voice_get_block_count_name(int accountid);
int cal_voice_get_block_count_index_avaiable(int **array);

char *cal_voip_get_white_list_enable();
int cal_voip_set_white_list_enable(char *value);
int cal_voice_add_white_count(char *number);
int cal_voice_del_white_count_index(int index);
char *cal_voice_get_white_count_name(int accountid);
int cal_voice_get_white_count_index_avaiable(int **array);

char *cal_voip_get_numberblock_status();
char *cal_voip_get_outgoing_blocking_enable();
int cal_voip_set_outgoing_blocking_enable(char *value);
char *cal_voip_get_outgoing_blocking_foreign_number_block_enable();
int cal_voip_set_outgoing_blocking_foreign_number_block_enable(char *value);
char *cal_voip_get_outgoing_blocking_special_rate_number_block_enable();
int cal_voip_set_outgoing_blocking_special_rate_number_block_enable(char *value);
char *cal_voip_get_outgoing_blocking_number();
int cal_voip_set_outgoing_blocking_number(char *number);

char *cal_voip_get_incoming_blocking_enable();
int cal_voip_set_incoming_blocking_enable(char *value);
char *cal_voip_get_incoming_blocking_number();
int cal_voip_set_incoming_blocking_number(char *number);
#endif

int cal_outgoing_get_num(void);
char *cal_voip_get_fxs_internal_call_enable(void);
int cal_voip_set_fxs_internal_call_enable(char *value);
#ifdef CONFIG_SUPPORT_FWA
char *cal_voip_get_fwa_mode_cs_call_enable(void);
#endif
int cal_voip_set_fwa_mode_cs_call_enable(char *value);

char *cal_voip_get_max_session_counts();
int cal_voip_set_max_session_counts(char *value);

char *cal_voip_get_extension_type(int id);
char *cal_voip_get_extension_name(int id);
int  cal_line_get_num(void); 
char *cal_voip_get_dtmf_method(int prof, int ret_type);
int   cal_voip_set_dtmf_method(int prof,  char *value);
char *cal_voip_get_dtmf_payload_type(int prof, int ret_type);
int cal_voip_set_dtmf_payload_type(int prof, char *value);
char *cal_voip_get_digit_map(int prof);
int   cal_voip_set_digit_map(int prof, char *value);
char *cal_voip_get_fast_dialing_digit(int prof);
int   cal_voip_set_fast_dialing_digit(int prof, char *value);
char *cal_voip_get_fast_dialing_enable(int prof);
int   cal_voip_set_fast_dialing_enable(int prof, char *value);
char *cal_voip_get_fax_mode(int prof,int line_id);
int   cal_voip_set_fax_mode(int prof, char *value,int line_id);
char *cal_voip_get_detect_cng_enable(int prof);
int cal_voip_set_detect_cng_enable(int prof, char *value);
char *cal_voip_get_detect_fax_enable(int prof);
int cal_voip_set_detect_fax_enable(int prof, char *value);

char *cal_voip_get_voice_test_state();
int cal_voip_set_voice_test_state(char *value);
char *cal_voip_get_voice_test_result();
int cal_voip_set_voice_test_result(char *value);
char *cal_voip_get_voice_test_lastcode();
int cal_voip_set_voice_test_result(char *value);

char *cal_voip_get_proxy_server(int prof);
int   cal_voip_set_proxy_server(int prof, char *value);
char *cal_voip_get_proxy_server_port(int prof);
int   cal_voip_set_proxy_server_port(int prof, char *value);
char *cal_voip_get_sec_proxy_server(int prof);
int   cal_voip_set_sec_proxy_server(int prof, char *value);
char *cal_voip_get_sec_proxy_server_port(int prof);
int   cal_voip_set_sec_proxy_server_port(int prof, char *value);
char *cal_voip_get_registrar_server(int prof);
int   cal_voip_set_registrar_server(int prof, char *value);
char *cal_voip_get_registrar_server_port(int prof);
int   cal_voip_set_registrar_server_port(int prof, char *value);
char *cal_voip_get_sec_registrar_server(int prof);
int   cal_voip_set_sec_registrar_server(int prof, char *value);
char *cal_voip_get_sec_registrar_server_port(int prof);
int   cal_voip_set_sec_registrar_server_port(int prof, char *value);
char *cal_voip_get_registration_period(int prof);
int   cal_voip_set_registration_period(int prof, char *value);
char *cal_voip_get_user_agent_domain(int prof);
int   cal_voip_set_user_agent_domain(int prof, char *value);
char *cal_voip_get_sec_user_agent_domain(int prof);
int   cal_voip_set_sec_user_agent_domain(int prof, char *value);
char *cal_voip_get_user_agent_port(int prof);
int   cal_voip_set_user_agent_port(int prof, char *value);
char  *cal_voip_get_transport(int prof, int line_id);
int   cal_voip_set_transport(int prof,int line_id, char *value);
char *cal_voip_get_proxy_management_method(int prof);
int   cal_voip_set_proxy_management_method(int prof, char *value);
char *cal_voip_get_proxy_override_by_dhcp_option_enable(int prof);
int   cal_voip_set_proxy_override_by_dhcp_option_enable(int prof, char *value);
char *cal_voip_get_sec_server_enable(int prof);
int  cal_voip_set_sec_server_enable(int prof, char *value);
char *cal_voip_get_outbound_proxy_enable(int prof);
int   cal_voip_set_outbound_proxy_enable(int prof, char *value);
char *cal_voip_get_outbound_proxy(int prof);
int   cal_voip_set_outbound_proxy(int prof, char *value);
char *cal_voip_get_outbound_proxy_port(int prof);
int   cal_voip_set_outbound_proxy_port(int prof, char *value);
char *cal_voip_get_secondary_outbound_proxy(int prof);
int   cal_voip_set_secondary_outbound_proxy(int prof, char *value);
char *cal_voip_get_secondary_outbound_proxy_port(int prof);
int   cal_voip_set_secondary_outbound_proxy_port(int prof, char *value);
char *cal_voip_get_dns_srv_enable(int prof);
char *cal_voip_get_deregister_enable(int prof);
int cal_voip_set_deregister_enable(int prof, char *value);
int   cal_voip_set_dns_srv_enable(int prof, char *value);
char *cal_voip_get_register_expires(int prof);
int   cal_voip_set_register_expires(int prof, char *value);
char *cal_voip_get_delta_register_expires(int prof);
int   cal_voip_set_delta_register_expires(int prof, char *value);
char *cal_voip_get_reg_retry_interval(int prof);
int   cal_voip_set_reg_retry_interval(int prof, char *value);
char *cal_voip_get_probe_retry_interval(int prof);
int   cal_voip_set_probe_retry_interval(int prof, char *value);
char *cal_voip_get_sip_dscp_mark(int prof, int ret_type);
int   cal_voip_set_sip_dscp_mark(int prof, char *value);
char *cal_voip_get_sip_ethernetpriority_mark(int prof, int ret_type);
int   cal_voip_set_sip_ethernetpriority_mark(int prof, char *value);
char *cal_voip_get_session_timers_type(int prof);
int   cal_voip_set_session_timers_type(int prof, char *value);
char *cal_voip_get_session_timers_enable(int prof);
int   cal_voip_set_session_timers_enable(int prof, char *value);
char *cal_voip_get_session_expires(int prof);
int   cal_voip_set_session_expires(int prof, char *value);
char *cal_voip_get_min_session_expires(int prof);
int   cal_voip_set_min_session_expires(int prof, char *value);

char *cal_voip_get_rtp_local_port_min(int prof);
int   cal_voip_set_rtp_local_port_min(int prof, char *value);
char *cal_voip_get_rtp_local_port_max(int prof);
int   cal_voip_set_rtp_local_port_max(int prof, char *value);
char *cal_voip_get_rtp_dscp_mark(int prof, int ret_type);
int   cal_voip_set_rtp_dscp_mark(int prof, char *value);
char *cal_voip_get_rtp_ethernetpriority_mark(int prof, int ret_type);
int   cal_voip_set_rtp_ethernetpriority_mark(int prof, char *value);
char *cal_voip_get_rtcp_enable(int prof);
int cal_voip_set_rtcp_enable(int prof,char *value);
char *cal_voip_get_rtcp_tx_interval(int prof);
int cal_voip_set_rtcp_tx_interval(int prof,char *value);

char *cal_voip_get_cptone_freq_1(int prof, int tone_id);
int   cal_voip_set_cptone_freq_1(int prof, int tone_id, char *value);
char *cal_voip_get_cptone_power_1(int prof, int tone_id);
int   cal_voip_set_cptone_power_1(int prof, int tone_id, char *value);
char *cal_voip_get_cptone_freq_2(int prof, int tone_id);
int   cal_voip_set_cptone_freq_2(int prof, int tone_id, char *value);
char *cal_voip_get_cptone_power_2(int prof, int tone_id);
int   cal_voip_set_cptone_power_2(int prof, int tone_id, char *value);
char *cal_voip_get_cptone_duration_on_1(int prof, int tone_id);
int   cal_voip_set_cptone_duration_on_1(int prof, int tone_id, char *value);
char *cal_voip_get_cptone_duration_off_1(int prof, int tone_id);
int   cal_voip_set_cptone_duration_off_1(int prof, int tone_id, char *value);
char *cal_voip_get_cptone_duration_on_2(int prof, int tone_id);
int   cal_voip_set_cptone_duration_on_2(int prof, int tone_id, char *value);
char *cal_voip_get_cptone_duration_off_2(int prof, int tone_id);
int   cal_voip_set_cptone_duration_off_2(int prof, int tone_id, char *value);

char *cal_voip_get_faxT38_high_speed_redundancy(int prof);
int   cal_voip_set_faxT38_high_speed_redundancy(int prof, char *value);
char *cal_voip_get_faxT38_low_speed_redundancy(int prof);
int   cal_voip_set_faxT38_low_speed_redundancy(int prof, char *value);
char *cal_voip_get_line_faxdetecttype(int prof,int line_id);
int cal_voip_set_line_faxdetecttype(int prof,char *value,int line_id);
char *cal_voip_get_line_append_prefix_enable(int prof, int line_id);
int cal_voip_set_line_append_prefix_enable(int prof, char *value, int line_id);
char *cal_voip_get_line_append_prefix(int prof, int line_id);
int cal_voip_set_line_append_prefix(int prof, char *value, int line_id);
char *cal_voip_get_line_delete(int prof, int line_id);
int cal_voip_set_line_delete(int prof, char *value, int line_id);


char *cal_voip_get_line_wifi_cordless_enable(int prof, int line_id);
int cal_voip_set_line_wifi_cordless_enable(int prof, int line_id, char *value);
char *cal_voip_get_line_enable(int prof, int line_id, int ret_type);
int   cal_voip_set_line_enable(int prof, int line_id, char *value);
char *cal_voip_get_reinjection_enable(int prof, int line_id, int ret_type);
int cal_voip_set_reinjection_enable(int prof, int line_id, char *value);
char *cal_voip_get_prack_enable(int prof);
int cal_voip_set_prack_enable(int prof,char *value);
char *cal_voip_get_strict_sipmessage_check_enable(int prof);
int cal_voip_set_strict_sipmessage_check_enable(int prof,char *value);
char *cal_voip_get_update_fresh_session_enable(int prof);
int cal_voip_set_update_fresh_session_enable(int prof, char *value);
char *cal_voip_get_dual_homing_enable(int prof);
int cal_voip_set_dual_homing_enable(int prof, char *value);
char *cal_voip_get_requestline_use_useragent_enable(int prof);
int cal_voip_set_requestline_use_useragent_enable(int prof, char *value);
char *cal_voip_get_line_directory_number(int prof, int line_id);
int   cal_voip_set_line_directory_number(int prof, int line_id, char *value);
char *cal_voip_get_line_name(int prof, int line_id);
int cal_voip_set_line_name(int prof, int line_id, char *value);
char *cal_voip_get_line_status(int prof, int line_id);
int cal_voip_set_line_status(int prof, int line_id, char *value);
char *cal_voip_get_line_auth_username(int prof, int line_id);
int   cal_voip_set_line_auth_username(int prof, int line_id, char *value);
char *cal_voip_get_line_auth_password(int prof, int line_id);
int   cal_voip_set_line_auth_password(int prof, int line_id, char *value);
char *cal_voip_get_line_uri(int prof, int line_id);
int   cal_voip_set_line_uri(int prof, int line_id, char *value);
char *cal_voip_get_line_caller_id_name(int prof, int line_id);
int   cal_voip_set_line_caller_id_name(int prof, int line_id, char *value);
char *cal_voip_get_line_call_forward_provision(int prof, int line_id);
int   cal_voip_set_line_call_forward_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_call_forward_mode(int prof, int line_id, int ret_type);
int   cal_voip_set_line_call_forward_mode(int prof, int line_id, char *value);
char *cal_voip_get_line_call_forward_number(int prof, int line_id);
int   cal_voip_set_line_call_forward_number(int prof, int line_id, char *value);
char *cal_voip_get_line_call_forward_no_answer_time(int prof, int line_id);
int   cal_voip_set_line_call_forward_no_answer_time(int prof, int line_id, char *value);
char *cal_voip_get_line_anoncall_block_provision(int prof, int line_id);
int   cal_voip_set_line_anoncall_block_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_anoncall_block_activate(int prof, int line_id);
int   cal_voip_set_line_anoncall_block_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_caller_id_provision(int prof, int line_id);
int   cal_voip_set_line_caller_id_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_caller_id_activate(int prof, int line_id);
int   cal_voip_set_line_caller_id_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_call_waiting_provision(int prof, int line_id);
int   cal_voip_set_line_call_waiting_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_call_waiting_activate(int prof, int line_id);
int   cal_voip_set_line_call_waiting_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_call_hold_provision(int prof, int line_id);
int   cal_voip_set_line_call_hold_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_call_hold_activate(int prof, int line_id);
int   cal_voip_set_line_call_hold_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_conference_call_provision(int prof, int line_id);
int   cal_voip_set_line_conference_call_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_conference_call_activate(int prof, int line_id);
int   cal_voip_set_line_conference_call_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_dnd_provision(int prof, int line_id);
int   cal_voip_set_line_dnd_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_dnd_activate(int prof, int line_id);
int   cal_voip_set_line_dnd_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_mwi_activate(int prof, int line_id);
int   cal_voip_set_line_mwi_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_hotline_provision(int prof, int line_id);
int   cal_voip_set_line_hotline_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_hotline_activate(int prof, int line_id);
int   cal_voip_set_line_hotline_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_hotline_number(int prof, int line_id);
int   cal_voip_set_line_hotline_number(int prof, int line_id, char *value);
char *cal_voip_get_line_hotline_timer(int prof, int line_id);
int   cal_voip_set_line_hotline_timer(int prof, int line_id, char *value);
char *cal_voip_get_line_clip_activate(int prof, int line_id);
int   cal_voip_set_line_clip_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_call_transfer_provision(int prof, int line_id);
int   cal_voip_set_line_call_transfer_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_call_transfer_activate(int prof, int line_id);
int   cal_voip_set_line_call_transfer_activate(int prof, int line_id, char *value);
char *cal_voip_get_line_call_return_provision(int prof, int line_id);
int   cal_voip_set_line_call_return_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_repeat_dial_provision(int prof, int line_id);
int   cal_voip_set_line_repeat_dial_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_mwi_provision(int prof, int line_id);
int   cal_voip_set_line_mwi_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_clip_provision(int prof, int line_id);
int   cal_voip_set_line_clip_provision(int prof, int line_id, char *value);
char *cal_voip_get_line_polarity_reversal_provision(int prof, int line_id);
int   cal_voip_set_line_polarity_reversal_provision(int prof, int line_id, char *value);

char *cal_voip_get_feature_codes_enable(int prof);
int   cal_voip_set_feature_codes_enable(int prof, char *value);
char *cal_voip_get_keys_of_set_cf_number(int prof);
int   cal_voip_set_keys_of_set_cf_number(int prof, char *value);
char *cal_voip_get_keys_of_enable_cf_all(int prof);
int   cal_voip_set_keys_of_enable_cf_all(int prof, char *value);
char *cal_voip_get_keys_of_disable_cf_all(int prof);
int   cal_voip_set_keys_of_disable_cf_all(int prof, char *value);
char *cal_voip_get_keys_of_enable_cf_on_busy(int prof);
int   cal_voip_set_keys_of_enable_cf_on_busy(int prof, char *value);
char *cal_voip_get_keys_of_enable_cf_on_noanswer(int prof);
int   cal_voip_set_keys_of_enable_cf_on_noanswer(int prof, char *value);
char *cal_voip_get_keys_of_disable_cf_on_busy_and_noanswer(int prof);
int   cal_voip_set_keys_of_disable_cf_on_busy_and_noanswer(int prof, char *value);
char *cal_voip_get_keys_of_xfer_without_consultation(int prof);
int   cal_voip_set_keys_of_xfer_without_consultation(int prof, char *value);
char *cal_voip_get_keys_of_xfer_with_consultation(int prof);
int   cal_voip_set_keys_of_xfer_with_consultation(int prof, char *value);
char *cal_voip_get_keys_of_enable_anoncall_block(int prof);
int   cal_voip_set_keys_of_enable_anoncall_block(int prof, char *value);
char *cal_voip_get_keys_of_disable_anoncall_block(int prof);
int   cal_voip_set_keys_of_disable_anoncall_block(int prof, char *value);
char *cal_voip_get_keys_of_enable_caller_id_block(int prof);
int   cal_voip_set_keys_of_enable_caller_id_block(int prof, char *value);
char *cal_voip_get_keys_of_disable_caller_id_block(int prof);
int   cal_voip_set_keys_of_disable_caller_id_block(int prof, char *value);
char *cal_voip_get_keys_of_enable_call_waiting(int prof);
int   cal_voip_set_keys_of_enable_call_waiting(int prof, char *value);
char *cal_voip_get_keys_of_disable_call_waiting(int prof);
int   cal_voip_set_keys_of_disable_call_waiting(int prof, char *value);
char *cal_voip_get_keys_of_enable_call_hold(int prof);
int   cal_voip_set_keys_of_enable_call_hold(int prof, char *value);
char *cal_voip_get_keys_of_disable_call_hold(int prof);
int   cal_voip_set_keys_of_disable_call_hold(int prof, char *value);
char *cal_voip_get_keys_of_enable_dnd(int prof);
int   cal_voip_set_keys_of_enable_dnd(int prof, char *value);
char *cal_voip_get_keys_of_disable_dnd(int prof);
int   cal_voip_set_keys_of_disable_dnd(int prof, char *value);
char *cal_voip_get_keys_of_call_return(int prof);
int   cal_voip_set_keys_of_call_return(int prof, char *value);
char *cal_voip_get_keys_of_redial(int prof);
int   cal_voip_set_keys_of_redial(int prof, char *value);

char *cal_voip_get_tx_gain(int prof, int line_id);
int   cal_voip_set_tx_gain(int prof, int line_id, char *value);
char *cal_voip_get_rx_gain(int prof, int line_id);
int   cal_voip_set_rx_gain(int prof, int line_id, char *value);
char *cal_voip_get_slic_tx_gain(int prof, int ret_type);
int   cal_voip_set_slic_tx_gain(int prof, char *value);
char *cal_voip_get_slic_rx_gain(int prof, int ret_type);
int   cal_voip_set_slic_rx_gain(int prof, char *value);
char *cal_voip_get_jitter_buffer_mode(int prof,int ret_type);
int cal_voip_set_jitter_buffer_mode(int prof, char *value);
char *cal_voip_get_min_jitter_buffer(int prof);
int cal_voip_set_min_jitter_buffer(int prof, char *value);
char *cal_voip_get_max_jitter_buffer(int prof);
int cal_voip_set_max_jitter_buffer(int prof, char *value);
char *cal_voip_get_comfort_noise_enable(int prof, int line_id);
int cal_voip_set_comfort_noise_enable(int prof,int line_id, char *value);
char *cal_voip_get_echo_cancel_enable(int prof, int line_id);
int cal_voip_set_echo_cancel_enable(int prof, int line_id, char *value);

char *cal_voip_get_callholdresumecall_enable(int prof);
int cal_voip_set_callholdresumecall_enable(int prof, char *value);
char *cal_voip_get_busytone_enable(int prof);
int cal_voip_set_busytone_enable(int prof, char *value);
char *cal_voip_get_options_enable(int prof);
int cal_voip_set_options_enable(int prof, char *value);
char *cal_voip_get_vbd_enable(int prof);
int cal_voip_set_vbd_enable(int prof, char *value);
char *cal_voip_get_offhook_alert_enable(int prof);
int cal_voip_set_offhook_alert_enable(int prof, char *value);
char *cal_voip_get_busytone_timer(int prof);
int cal_voip_set_busytone_timer(int prof, char *value);
char *cal_voip_get_fast_busytone_timer(int prof);
int cal_voip_set_fast_busy_timer(int prof, char *value);
char *cal_voip_get_ring_back_timer(int prof);
int cal_voip_set_ring_back_timer(int prof, char *value);
char *cal_voip_get_first_digit_timer(int prof);
int   cal_voip_set_first_digit_timer(int prof, char *value);
char *cal_voip_get_inter_digit_timer(int prof);
int   cal_voip_set_inter_digit_timer(int prof, char *value);
char *cal_voip_get_flash_hook_min_time(int prof);
int   cal_voip_set_flash_hook_min_time(int prof, char *value);
char *cal_voip_get_flash_hook_max_time(int prof);
int   cal_voip_set_flash_hook_max_time(int prof, char *value);
char *cal_voip_get_caller_id_detection_type(int prof, int ret_type);
int   cal_voip_set_caller_id_detection_type(int prof, char *value);
char *cal_voip_get_region(int prof, int ret_type);
int   cal_voip_set_region(int prof, char *value);

char *cal_voip_get_codec(int prof, int line, int codec_id);
char *cal_voip_get_codec_vad(int prof, int line_id);
int   cal_voip_set_codec_vad(int prof, int line_id,  char *value);
char *cal_voip_get_codec_packetization_period(int prof, int line, int codec_id);
int   cal_voip_set_codec_packetization_period(int prof, int line, int codec_id, char *value);
char *cal_voip_get_codec_enable(int prof, int line, int codec_id);
int   cal_voip_set_codec_enable(int prof, int line, int codec_id, char *value);
char *cal_voip_get_codec_priority(int prof, int line, int codec_id);
int   cal_voip_set_codec_priority(int prof, int line, int codec_id, char *value);
char *cal_voip_get_amrwb_payloadtype(int prof, int line);
char *cal_voip_get_amrwb_codectype(int prof, int line);
int cal_voip_set_amrwb_payloadtype(int prof, int line, char *value);
int cal_voip_set_amrwb_codectype(int prof, int line, char *value);
char *cal_voip_get_log_level(int prof, int ret_type);
int   cal_voip_set_log_level(int prof, char *value);

char *cal_voip_get_network_interface(int prof);
int   cal_voip_set_network_interface(int prof, char *value);
#ifdef CONFIG_SUPPORT_WAN_BACKUP
int cal_voip_get_binding_interface_id_p(int prof);
#endif
int cal_voip_get_binding_interface_id(int prof);
int cal_voip_get_reference_profile(const char *value);
int hcal_voip_check_available_service(void);
char *cal_voip_get_in_sip_map(int fxs_id);
int cal_voip_set_in_sip_map(int fxs_id, char *value);
char *cal_voip_get_out_sip_map(int fxs_id);
int cal_voip_set_out_sip_map(int fxs_id, char *value);
char *cal_voip_get_in_gsm_map(int fxs_id);
char *cal_voip_get_out_gsm_map(int fxs_id);
char *cal_voip_get_digit_map_enable(int prof);
int cal_voip_set_digit_map_enable(int prof, char *value);
int cal_voip_set_fxs_internal_number(int fxs_id, char *value);
char *cal_voip_get_fxs_internal_number(int fxs_id);

int cal_voip_del_all_calllog(void);
int cal_voip_get_calllog_num(int lind_id);
int cal_voip_del_calllog_entry(int line_id, int log_id);
int cal_voip_add_calllog_entry(int line_id, int log_id);

int cal_voip_set_x_list_callingnumber(int log_id, char *value,int line_id);
int cal_voip_set_x_list_callednumber(int log_id, char *value,int line_id);
int cal_voip_set_x_list_duration(int log_id, char *value,int line_id);
int cal_voip_set_x_list_sourceIP(int log_id, char *value,int line_id);
int cal_voip_set_x_list_sourcePort(int log_id, char *value,int line_id);
int cal_voip_set_x_list_destinationIP(int log_id, char *value,int line_id);
int cal_voip_set_x_list_destinationPort(int log_id, char *value,int line_id);
int cal_voip_set_x_list_stats(int log_id, char *value,int line_id);
int cal_voip_set_x_list_codec(int log_id, char *value,int line_id);

char *cal_voip_get_npc_url(void);
int cal_voip_set_npc_url(char *value);
char *cal_voip_get_npc_token_id(void);
int cal_voip_set_npc_token_id(char *value);

int cal_voip_set_unanswered_calls(char *value, int line);

char *cal_voip_get_profile_enable();
int cal_voip_set_profile_enable(char *value);
#endif  /* _CAL_VOIP_H_ */

