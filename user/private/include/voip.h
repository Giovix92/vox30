#ifndef _VOIP_H_
#define _VOIP_H_

#define VGW_APP_NAME                    "vgw_app"
#define VOICE_DATA_FILE                   "/var/voice/v000"
#define VOICE_STATISTIC_FILE                  "/mnt/1/v000"
#define CALL_LOG_FILE_PATH              "/mnt/1/call_log.log"
#define CALL_STATS_FILE_PATH            "/mnt/1/call_stats.log"
#define VGW_ROOT_PATH                   "/var/voice/"
#define VGW_LOCK                        VGW_ROOT_PATH"vgw.lock"
#define VOIP_CONF                       VGW_ROOT_PATH"voip.conf"
#define VOIP_PBX_CONF                   VGW_ROOT_PATH"pbx.conf"
#define SPYLOG_FILE	                    VGW_ROOT_PATH"voice.log"
#define SIPLOG_FILE                     VGW_ROOT_PATH"sip.log"
#define VOIP_CLI_DATA_FILE              VGW_ROOT_PATH"cgi_cli_data"
#define FILE_VGW_VOIP_STATUS            VGW_ROOT_PATH"voip"
#define FILE_VGW_VOIP_CALL_STATE	    VGW_ROOT_PATH"voip1"
#define VOIP_BLACK_LIST_FILE            "/var/voice/Black_List"
#define VOIP_WHITE_LIST_FILE            "/var/voice/White_List"
#define VOIP_ACTIVE_LIST_FILE           "/var/voice/Active_List"
#define VOIP_PBX_PHONE_LIST_FILE        "/var/voice/Pbx_phone_List"

enum VOICE_LINE_ID
{
    VOICE_LINE_ID_1 = 0,
#ifdef SUPPORT_TWO_FXS
    VOICE_LINE_ID_2,
#endif
    VOICE_LINE_ID_MAX,
};

int get_new_missed_call_information();
char * get_missed_call_number(int index);
char *get_missed_call_time(int index);
int get_given_missed_call_count(int index);
int get_all_missed_call_count();

int event_notify_missed_call();
int event_notify_voice_mail();
int event_notify_incoming_call(char *phone_number);
char *get_incomingcall_notify_deviceid();
void sal_voip_clean(void);

char *sal_voip_get_tx_codec(int line);
char *sal_voip_get_tx_vad(int line);
char *sal_voip_get_tx_ptime(int line);

char *sal_voip_get_session_startime(int line);
char *sal_voip_get_session_cutime(int line);
char *sal_voip_get_far_end_ip(int line);
char *sal_voip_get_local_UDP_port(int line);
char *sal_voip_get_far_UDP_port(int line);

char *sal_voip_get_packets_sent(int line);
char *sal_voip_get_packets_receive(int line);
char *sal_voip_get_bytes_sent(int line);
char *sal_voip_get_bytes_receive(int line);
char *sal_voip_get_packets_lost(int line);
char *sal_voip_get_overruns(int line);
char *sal_voip_get_underruns(int line);
char *sal_voip_get_rplr(int line);
char *sal_voip_get_feplr(int line);
char *sal_voip_get_rij(int line);
char *sal_voip_get_feij(int line);
char *sal_voip_get_rtd(int line);
char *sal_voip_get_arij(int line);
char *sal_voip_get_afeij(int line);
char *sal_voip_get_artd(int line);

char *sal_voip_get_incall_received(int line);
char *sal_voip_get_incall_answered(int line);
char *sal_voip_get_incall_connected(int line);
char *sal_voip_get_incall_failed(int line);
char *sal_voip_get_outcall_attempted(int line);
char *sal_voip_get_outcall_answered(int line);
char *sal_voip_get_outcall_connected(int line);
char *sal_voip_get_outcall_failed(int line);
char *sal_voip_get_callsdropped(int line);
char *sal_voip_get_total_time(int line);
char *sal_voip_get_hal_capture_state(int line);
char *sal_voip_get_ab_ac_voltage(int line);
char *sal_voip_get_ab_dc_voltage(int line);
char *sal_voip_get_ab_resistance(int line);
char *sal_voip_get_ag_ac_voltage(int line);
char *sal_voip_get_ag_dc_voltage(int line);
char *sal_voip_get_ag_resistance(int line);
char *sal_voip_get_bg_ac_voltage(int line);
char *sal_voip_get_bg_dc_voltage(int line);
char *sal_voip_get_bg_resistance(int line);
char *sal_voip_get_ab_ren(int line);
char *sal_voip_get_ab_current(int line);
char *sal_voip_get_ab_capacity(int line);
char *sal_voip_get_ag_capacity(int line);
char *sal_voip_get_bg_capacity(int line);
char *sal_voip_get_off_hook(int line);

char *sal_voip_get_voice_test_state(int line);
char *sal_voip_get_voice_test_result(int line);
char *sal_voip_get_voice_test_lastcode(int line);

char *sal_voip_get_hook_state(int line);
char *sal_voip_get_session_state(int line);

char *sal_voip_get_call_state(int line);

char *sal_voip_get_reg_state(int line);

char *sal_voip_get_server_down_time(int line);
/*for lan call state*/
char *sal_voip_get_lan_call_state(int line);


char *sal_voip_get_lan_reg_number(int line);
char *sal_voip_get_lan_reg_state(int line);

#define VOICE_MAX_CALLLOG_NUM                 50
#define LOCAL_VOICE_BUFF_SIZE                 128
#define CALL_LOG_STATE_DIALLED_STR          "Dialled"
#define CALL_LOG_STATE_ACTIVE_STR           "Active"
#define CALL_LOG_STATE_COMPLETED_STR        "Completed"
#define CALL_LOG_STATE_MISSED_STR           "Missed"
#define CALL_LOG_DIR_IN_STR                 "in"
#define CALL_LOG_DIR_OUT_STR                "out"

typedef struct call_log_s
{
    int                     magicNumber;
    int                     port;
    char                    callState[LOCAL_VOICE_BUFF_SIZE];
    char                    unread[LOCAL_VOICE_BUFF_SIZE];
    char                    direction[LOCAL_VOICE_BUFF_SIZE];
    char                    farEndNumber[LOCAL_VOICE_BUFF_SIZE];
    char                    farEndIP[LOCAL_VOICE_BUFF_SIZE];
    char                    farEndPort[LOCAL_VOICE_BUFF_SIZE];
    char                    localNumber[LOCAL_VOICE_BUFF_SIZE];
    char                    localIP[LOCAL_VOICE_BUFF_SIZE];
    char                    localPort[LOCAL_VOICE_BUFF_SIZE];
    char                    codec[LOCAL_VOICE_BUFF_SIZE];
    char                    startTime[LOCAL_VOICE_BUFF_SIZE];
    char                    duration[LOCAL_VOICE_BUFF_SIZE];
    unsigned long           RTPPacketsReceived;
    unsigned long           RTPPacketsSent;
    unsigned long           RTPPacketsLost;
}CALL_LOG_T;

int sal_voip_call_log_load(CALL_LOG_T *log_array, int array_size);
int sal_voip_call_log_delete(char* delete_entry);
int sal_voip_call_log_update(int id);
int sal_voip_call_log_remove(int key);
int sal_voip_calllog_update_info(void);

#define VOICE_LINE_STATE_RESULT         VGW_ROOT_PATH"line_state_result"
#define VOICE_DIAGNOSIS_CFG             VGW_ROOT_PATH"voice_diagnosis_cfg" 
#define VOICE_DIAGNOSE_FILE	            VGW_ROOT_PATH"vgw_to_dignosis"
#define LOCAL_VOICE_DIAGNOSIS           VGW_ROOT_PATH"local_voice_diagnosis"
#define VOICE_DIAGNOSE_HOOK	            VGW_ROOT_PATH"vgw_hook"

#define DIAGNOSE_STATE                 "/tmp/diagnose_state"

#define VOIP_TIME_INFO_FILE_NAME        VGW_ROOT_PATH"voip_time_info"
#define VOIP_WATCHDOG_INFO_NULL         "null"
#define DEFAULT_COREDUMP_PATH           "/tmp/core_dump/"
#define VOIP_DIAGNOSE_INFO_FILE         "/tmp/voip_diagnose_info.txt"
#define VOIP_DIAGNOSE_INFO_FILE_TAR     "/tmp/www-app/voip_diagnose_info.tar"
#define VOIP_AUDIO_RAW_FILE             "/tmp/*.raw"
#define VOIP_WATCHDOG_HEART_BEAT_INTERVAL   300


enum VOICE_APP_STATUS
{
    VOICE_APP_UP = 0,
    VOICE_APP_CONNECTING,
    VOICE_APP_DISABLED,
    VOICE_APP_ERROR,
};

typedef enum
{
    DONGLE_WAN,
    FIXED_WAN
}VOIP_WAN_TYPE;

typedef enum {
    CALL_WAITING_STAR_CODE,
    STAR_CODE_MAX
}STAR_CODE_ID;

/**
* Warning: keep the sequence same as typedef enum VGW_REG_STATUS_s 
*              in drivers/voip/vgw/include/vm_common.h
*                                       2011/06/27, Andic
*
**/
enum VOICE_VGW_STATUS
{
    VOICE_VGW_DISABLED = 0,
    VOICE_VGW_REGISTERING,
    VOICE_VGW_UP,
    VOICE_VGW_ERROR,
    VOICE_VGW_INITIALIZING,
    VOICE_VGW_UNREGISTERING,
    VOICE_VGW_QUIESCENT,
};

enum VOICE_EXTRA_ERROR_CODE
{
    VOICE_EXTRA_ERROR_NONE = 0,
    VOICE_EXTRA_ERROR_NET,
    VOICE_EXTRA_ERROR_PHONE,
};

/*
 * @fn int voice_app_status_get(int line, int *err_id)
 * @brief
 *          get line status, and set extra error info if error
 * @param[in] 
 *          line: 0/1 for line 0/1
 *          errno: keep error id here if error, which is defined 
 *                   in enum VOICE_EXTRA_ERROR_CODE
 * @return
 *          value defined in enum VOICE_APP_STATUS
 */
int voice_app_status_get(int line, int *err_id);

/*
 * @fn 
 * @brief
 * @param[in] 
 *          line: 0/1 for line 0/1
 * @return
 *          0: valid, -1: invalid
 */
int valid_line(int line);

int voice_line_register_ok(int line);

/*********************************
*  VOICE DIAGNOSIS
*
*********************************/
enum DIAGNOSIS_CFG_LIST
{
    DIAGNOSIS_CFG_WAN_IP = 0,
    DIAGNOSIS_CFG_WAN_MAC,
    DIAGNOSIS_CFG_WAN_IFACE,
    DIAGNOSIS_CFG_WAN_GW,
    DIAGNOSIS_CFG_DNS_HOST,
    DIAGNOSIS_CFG_CMD_PHONE_CONNECTION,
    DIAGNOSIS_CFG_CMD_RING_ON,
    DIAGNOSIS_CFG_CMD_RING_OFF,
    DIAGNOSIS_CFG_MAX
};

enum DIAGNOSIS_STATE_LIST
{
    DIAGNOSIS_STATE_NO_RUN = 0,
    DIAGNOSIS_STATE_INITIALIZING,
    DIAGNOSIS_STATE_EQUIPMENT,
    DIAGNOSIS_STATE_RINGTONE,
    DIAGNOSIS_STATE_DIAL_TONE,
    DIAGNOSIS_STATE_NUMBERING,
    DIAGNOSIS_STATE_FINISH,
};

enum DIAGNOSIS_CMD_LIST
{
    DIAGNOSIS_CMD_SET_INITIALIZING = 0,
    DIAGNOSIS_CMD_SET_EQUIPMENT,
    DIAGNOSIS_CMD_SET_RINGTONE,
    DIAGNOSIS_CMD_SET_DIAL_TONE,
    DIAGNOSIS_CMD_SET_NUMBERING,
    DIAGNOSIS_CMD_SET_FINISH,
    DIAGNOSIS_CMD_GET_STATE,
    DIAGNOSIS_CMD_ALL,
};

enum DIAGNOSIS_STATE_STATUS
{
    DIAGNOSIS_STATE_STATUS_NOT_EXECUTE = 0,
    DIAGNOSIS_STATE_STATUS_IN_PROGRESS,
    DIAGNOSIS_STATE_STATUS_DONE,
};

enum DIAGNOSIS_ERROR_CODE
{
    DIAGNOSIS_ERROR_NONE = 0,
    DIAGNOSIS_ERROR_LINE_NO,
    DIAGNOSIS_ERROR_WAN_IFACE,
    DIAGNOSIS_ERROR_WAN_GATEWAY,
    DIAGNOSIS_ERROR_WAN_DNS,
    DIAGNOSIS_ERROR_VOICE_UNREG,
    DIAGNOSIS_ERROR_PHONE_UNCON,
    DIAGNOSIS_ERROR_PHONE_UNPICKUP,
    DIAGNOSIS_ERROR_PHONE_UNHANGUP,
    DIAGNOSIS_ERROR_WRONG_NUMBER,
    DIAGNOSIS_ERROR_WRONG_CMD,
};

typedef struct
{
    int line_no;
    int cmd_set;
}diagnosis_method_t;

typedef struct
{
    int line_no;
    int state;
    int status;
    int error;
    char exinfo[64];
}diagnosis_status_t;

#define LOCAL_VOICE_BUFF_SIZE 128


#ifdef VOICE_DIAGNOSIS_DEBUG
#define VOICE_DIAG_DEBUG(format,argument...) printf(format,##argument)
#else
#define VOICE_DIAG_DEBUG(format,argument...)
#endif

/*
 * @fn int voice_diagnosis_client(diagnosis_method_t *diag_in, diagnosis_status_t *diag_out)
 * @brief
 *          run diagnosis step and also get diagnosis result
 * @param[in] 
 *          diag_in: 
 *               client send diagnosis step cmd to server by diag_in
 *          diag_out: 
 *               if set cmd is DIAGNOSIS_CMD_GET_STATE, diag_out will be used to save result
 * @warning
 *          diag_in, and diag_out should be valid
 * @return
 *          if >= 0, client&server excute cmd ok;if < 0, error
 */
int voice_diagnosis_client(diagnosis_method_t *diag_in, diagnosis_status_t *diag_out);
int voice_get_line_test_status( int phy_line);
int voice_get_line_test_connectivity(int phy_line);
char *voice_get_line_test_conclusion(int phy_line);
char *voice_get_line_test_current(int phy_line);
char *voice_get_line_test_callstatus(int phy_line);

int voice_get_voice_test_status();
char *voice_get_voice_test_result();
void voice_get_voice_test_lastcode(char *lastcode);

/* --------------------------------------------------------------------------*/
/**
 * @brief    LED action
 */
/* --------------------------------------------------------------------------*/
typedef enum
{
    ST_DISABLED = 0,              /* Disabled */
    ST_REGISTER_UP = 1,           /* Register OK*/
    ST_REGISTER_DOWN = 2,         /* Register FAIL*/
    ST_REGISTERING = 3,           /* Registering */
    ST_GSM_ON_REGISTER_DOWN = 4,  /*Gsm on and register down*/
    ST_WAN_DOWN = 5,              /* Wan down */
    ST_GSM_DOWN = 6,              /* Gsm down*/
    ST_RINGING = 7,               /*Ringing*/
    ST_TALKING = 8,               /*Talking*/
    ST_CALLING = 9,               /*Ringing*/
    ST_OFF_HOOK = 10,             /* hook off */
    ST_ON_HOOK = 11,              /* hook on */

} ST_LED_ACTION;

void register_vik();
void set_led_status(ST_LED_ACTION status);
void add_ipphone_reg_pbx_vocs_dns_rules( );
void star_code_status_update(STAR_CODE_ID type, int line_id, int enable);
int call_summary_update();
int add_iphone_incomingcall_notifylist(char *macaddress);
int change_format_host_mac_addr(char *src_addr,char *dest_addr, int addr_len);
#ifdef CONFIG_SUPPORT_PRPL_HL_API
typedef enum {
    HL_API_EVENT_REGISTRATION_UNUSED = 0,
    HL_API_EVENT_REGISTRATION_EXPIRED = 1,
    HL_API_EVENT_REGISTRATION_FAILED = 2,
    HL_API_EVENT_REGISTRATION_SUCCESSFUL = 3
}HL_API_SIP_EVENT_TYPE;
int prpl_hl_api_sip_registration_event_notice(HL_API_SIP_EVENT_TYPE code, int extension_id);
#endif

#define VOICE_LOCAL_SOCKET   "/tmp/gsm_local.socket"
#define VOICE_SOCK_WAIT_TIME         60
#define VOICE_SOCKET_TASK_NAME       "GSM"
#define G_TRUNK_MAX_ADDR_DIGITS         32

typedef enum {
    GSM_REMOTE_MSG_REMOTE_CALL = 0,
    GSM_REMOTE_MSG_REMOTE_CLIP,
    GSM_REMOTE_MSG_REMOTE_ALART,
    GSM_REMOTE_MSG_REMOTE_CONNECT,
    GSM_REMOTE_MSG_REMOTE_HANDUP,
    GSM_REMOTE_MSG_REMOTE_CWALL,
    GSM_REMOTE_MSG_SETUP_CONF,
    GSM_REMOTE_MSG_RELEASE_CONF,
#ifdef SUPPORT_PHONE_PIN_WIZARD
    GSM_DEVICE_STATE,
    GSM_DEVICE_RETURN,
#endif
    GSM_DEVICE_NUMBER,
    GSM_DONGLE_WAN_STATE,
    GSM_REMOTE_MSG_MAX
}GSM_REMOTE_MSG_TYPE;

typedef struct tag_gsm_msg{
    int gsm_idx;
    int call_idx;
    union 
    {
        char callid[G_TRUNK_MAX_ADDR_DIGITS];
        int digit;
        int hold_type;
    }data; 
}gsm_msg;

typedef struct tag_state_msg{
    int sub;
    union 
    {
        int pin;
    }data; 
}state_msg;

typedef	struct _gsm_remote_msg_entry
{
    GSM_REMOTE_MSG_TYPE type;       /**/    
    union 
    {
        gsm_msg vmsg;
        state_msg mmsg;
    }data;      
}gsm_remote_msg_entry;

int voice_send_msg(GSM_REMOTE_MSG_TYPE type, int op1,int op2, char* op3);

void voip_system_warning(const char *info);
void voip_system_logging(const char *info);

void vgw_watchdog_heart_beat();
void vgw_watchdog_register(void);
void vgw_watchdog_unregister(void);
void vgw_watchdog_assertion_fail(void);
void vgw_watchdog_init_fail(void);

void sal_voip_phone_settings_update(char  *cw_enable, int line);
void sal_voip_dial_plan_update();
void sal_voip_dial_plan_init();

/* Arvin_Gui, 2018-05-20 */

typedef struct sms_message_body{
    char index[32];
    char unread[2];               /* 0:unread 1:read */
    char time_stamp[64];
    char phone_number[128];    /* sender phone number*/
    char msg_data[161]; /* PDU decode standard: user data must <= 160 bytes after decode, and add '\0' */

}SMSMESSAGE;

void get_vgw_sms_message(SMSMESSAGE* sms_body);
int check_device_wifi_is_protected(char *app_ip);
#endif


