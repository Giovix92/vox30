
#ifndef _DONGLE_SOCKET_H_
#define _DONGLE_SOCKET_H_

#define DONGLE_LOCAL_SOCKET           "/tmp/dongle_local.socket"
#define DONGLE_DATA_LOCAL_SOCKET      "/tmp/dongle_data_local.socket"
#define UUID_SIZE 64
#define UUID_PATH "/proc/sys/kernel/random/uuid"

#ifdef CONFIG_SUPPORT_SMS
#include <common/list.h>
#define UBUS_SOCKET "/var/ubus/ubus.sock"
#define MOBILE_OBJECT "v1.mobile"
#define DONGLE_SMS_ARRAY "dongles.sms.messages.info"
#define UBUS_EVENT_HANDLE_MESSAGE_SUCESS "sms_handle_message_sucess" 
#define UMTS_UBUS_MSG_TYPE "umts_ubus_msg_type"
#define UMTS_UBUS_VALUE1 "umts_ubus_value1"
#define UMTS_UBUS_VALUE2 "umts_ubus_value2"
#define UMTS_UBUS_VALUE3 "umts_ubus_value3"
#define UMTS_UBUS_VALUE4 "umts_ubus_value4"
#define UMTS_UBUS_PHONE_NUM "umts_ubus_phone_num"
#define UMTS_UBUS_SMS_DATA  "umts_ubus_sms_data"
#define UMTS_UBUS_SMS_TASK  "umts_ubus_sms_task"
#define UMTS_UBUS_TIME_STAMP "umts_ubus_time_stamp"
#define UMTS_UBUS_MSG_INDEX "umts_ubus_msg_index"
#define UMTS_UBUS_SMS_TYPE "umts_ubus_sms_type"
#define UMTS_UBUS_SMS_DELE "umts_ubus_sms_dele"
#define UMTS_UBUS_SMS_STATUS "umts_ubus_sms_status"
#define UMTS_UBUS_SMS_RELOAD "umts_ubus_sms_reload"
#define UMTS_UBUS_SMS_CONTENT "umts_ubus_sms_content"
#define UMTS_UBUS_SMS_STORAGE "umts_ubus_sms_storage"
#define UMTS_MOBILE_METHOD_SMS "G_MSG_SMS_Handle"
#define UMTS_MOBILE_METHOD_LOCK "G_MSG_LOCAL_LOCK"
#define UMTS_MOBILE_METHOD_UNLOCK "G_MSG_LOCAL_UNLOCK"
#define UMTS_MOBILE_METHOD_SET "G_MSG_LOCAL_SET"
#define UMTS_MOBILE_METHOD_GET "G_MSG_LOCAL_GET"
#endif

#define DONGLE_SOCK_WAIT_TIME         60

typedef enum {
    G_MSG_LOCAL_INIT = 0,
    G_MSG_LOCAL_CALL,
    G_MSG_LOCAL_ALART,
    G_MSG_LOCAL_CONNECT,
    G_MSG_LOCAL_RELEASE,
    G_MSG_CALL_MODE_CHANGE,
    G_MSG_LOCAL_DIGIT,
    G_MSG_LOCAL_SETUP_CONF,
#ifdef SUPPORT_PHONE_PIN_WIZARD
    G_MSG_LOCAL_SET_PIN,
#endif
    G_MSG_LOCAL_GET,
    G_MSG_LOCAL_SET,
    G_MSG_LOCAL_LOCK,
    G_MSG_LOCAL_UNLOCK,
#ifdef CONFIG_SUPPORT_SMS
    G_MSG_SEND_SMS,
    G_MSG_WRITE_SMS,
    G_MSG_LOAD_MSG,
    G_MSG_DELE_MSG,
    G_MSG_UPDATE_MSG_STATUS,
    G_MSG_RECEV_MSG,
#endif
    G_MSG_MAX,
}DONGLE_MSG_TYPE;
#ifdef CONFIG_SUPPORT_SMS
enum { //storage message status
    STORED_RECEIVE_UNREAD_MSG = 0,
    STORED_RECEIVE_READ_MSG,
    STORED_UNSENT_MSG,
    STORED_SENT_MSG,
    STORED_ALL_MSG,
};
enum {//used for AT+CMGD
    DELE_MSG_BY_INDEX = 0, //delete the message stored at the location specified by index
    DELE_READ_MSG ,//delete all the read message savede in the preferred storage ,and keep unread ,send,and,unsend ones;
    DELE_READ_SENT_MSG,
    DELE_READ_SENT_UNSENT_MESSAGE,
    DELE_ALL_MESSAGE,//include unread ones
};

enum sim_storage_status{
   NOT_FULL,
   W_R_FULL,//if value = 1 means the memory storage used for writing and sending messages is full
   REC_FULL,//if  value = 1 means the memory storage used for receiving messages is full
   ALL_FULL,
};
enum sms_return_status{
    SMS_OK = 0,
    NOT_MATCH_RULES,
    MATCH_RULES,
    SMS_ERR,
};
enum{
    PHONE_NUMBER,
    SMS_DATA,
    TIME_STAMP,
    MSG_INDEX,
    MSG_HANDLE_TYPE,// define send or write msg
    MSG_DEL_FLAG,
    TRANSFER_STATUS,
    SMS_TASK,
    _SMS_MAX,
};
enum {
    UMTS_MSG_TYPE,
    UMTS_VALUE1,
    UMTS_VALUE2,
    UMTS_VALUE3,
    UMTS_VALUE4,
    _LOCAL_MAX,
};
#endif
enum {
    UMTS_CHECK_PIN,
    UMTS_CHECK_PUK,
    UMTS_CHANGE_PIN,
    UMTS_GET_IMEI,
    UMTS_GET_IMSI,
    UMTS_GET_SN,//5
    UMTS_GET_MODEL,
    UMTS_GET_HWVERSION,
    UMTS_GET_CARDNUMBER,
    UMTS_GET_SMSNUMBER,
    UMTS_GET_SWVERSION,//10
    UMTS_GET_SIMSTATUS,
    UMTS_GET_PINSTATE,
    UMTS_GET_PIN,
    UMTS_GET_LOCK,
    UMTS_GET_NETWORK,//15
    UMTS_GET_NETWORKMODE,
    UMTS_GET_SUB_NETWORKMODE,
    UMTS_GET_DATASTATUS,
    UMTS_GET_NETWORKBAND,
    UMTS_GET_PINLEFT,//20
    UMTS_GET_PINATTEMPTS,
    UMTS_GET_PUKLEFT,
    UMTS_GET_PUKATTEMPTS,
    UMTS_GET_SIGNALSTRENGTH,
    UMTS_GET_TRAFFIC,
    UMTS_GET_SIGNAL,
    UMTS_GET_SNR,
    UMTS_GET_OPERATOR,
    UMTS_SET_OPERATOR,
    UMTS_SET_LOG_LEVEL,
    UMTS_GET_MANUFACTURE,
    UMTS_GET_STAT_ALL,
    UMTS_SET_APN,
    UMTS_SET_PREFERMODE,
    UMTS_SET_SMS_PHONENUMBER,
    UMTS_SET_VOLTE,
    UMTS_SET_AMR_CODEC,
    UMTS_GET_SMS_MSISDN,//36
    UMTS_GET_CELL_ID,
    UMTS_GET_LTE_RSRP,
    UMTS_GET_LTE_RSRQ,
    UMTS_GET_LTE_RSSI,
#ifdef CONFIG_SUPPORT_SMS
    UMTS_GET_SMS_ISFULL,
    UMTS_GET_SMS_UNREAD_COUNT,//number of received unread msg
    UMTS_GET_SMS_READ_COUNT,//number of received read msg
    UMTS_GET_SMS_UNSENT_COUNT,// number of unsent msg
    UMTS_GET_SMS_SENT_COUNT,//number of sent msg
    UMTS_GET_SMS_CONTENT,//number of all message
#endif
    UMTS_NDISD_START,
    UMTS_NDISD_STOP,
    UMTS_VIK_ATTACH_START,
#ifdef CONFIG_SUPPORT_NEW_LED_BEHAV
    UMTS_SET_NEW_LED_BEHAV_TYPE,
    UMTS_SET_DEFAULT_WAN,
#endif
#ifdef CONFIG_SUPPORT_FWA
    UMTS_SET_FWA_USED,
#endif
    UMTS_GET_DATAPS_2G,
    UMTS_GET_DATAPS_3G,
    UMTS_GET_DATAPS_4G,
    UMTS_GET_VOICEPS_3G,
    UMTS_GET_VOICEPS_4G,
    UMTS_GET_3G_REGISTERED_STATE,
    UMTS_GET_4G_REGISTERED_STATE,
};
#ifdef CONFIG_SUPPORT_SMS
typedef struct msg_info{
    char phone_num[32];
    char time_stamp[64];
    char msg[1024];
    int index;
    int stat;
    
}MSG_INFO;
#endif
typedef struct tag_voice_msg{
    int gsm_idx;
    int call_idx;
    union 
    {
        char callid[32];
        int digit;
        int hold_type;
    }data; 
}voice_msg;
typedef struct tag_pin_data{
    char pin[16];
    char puk[16];
    char new_pin[16];
}pin_data;

typedef struct tag_pin_state{
    int state;
    int pin_left;
    int pin_attempts;
    int puk_left;
    int puk_attempts;
}pin_state;

typedef struct tag_opt_data{
    int opt_used;
    int opt_code;
    char opt_str[64];
}opt_data;

typedef struct tag_log_data{
    char level[16];
    char path[16];
}log_data;

typedef struct tag_stat_data{
    int signal;
    int rssi;
    int rscp;
    int ecio;
    int network_mode;
    char network_detailmode[20]; /* GSM, EDGE, HSDPA/HSUPA */
    unsigned long rx_rate;
    unsigned long tx_rate;
    unsigned long rx_flux;
    unsigned long tx_flux;
}umts_stat_data;

typedef struct tag_mgnt_msg{
    int subid;
    char res_path[64];
    union 
    {
        pin_data pin;
        pin_state pin_state;
        char opt_list[512];
        char manufacture[64];
        char hw_version[64];
        char sw_version[64];
        umts_stat_data stat;
        opt_data opt;
        log_data log;
        char imei[32];
        char imsi[32];
        char sn[32];
        char model[64];
        char apn[64];
        char card_number[32];
        char sim_state[8];
        char network[8];
        int network_mode;
        int data_status;
        int lte_rsrp;
        int lte_rsrq;
        int lte_rssi;
        char network_band[128];
        char sms_number[128];
        char amr_codec[8];
        char cell_id[32];
#ifdef CONFIG_SUPPORT_NEW_LED_BEHAV
        int new_led_behav_type;
#endif
        unsigned char lock_state;
    }data; 
}mgnt_msg;

#ifdef CONFIG_SUPPORT_SMS
typedef struct  sms_info{
    int msg_index;
    int msg_status;
    int reply_permission;
    char msg_data[256];
    char sms_pdu[512];
    char title[128];
    char time_stamp[64];
    char phone_number[128];//include sender name
    struct list_head list;
}SMS_Info;
struct sms_task{
    char uuid[UUID_SIZE];
    struct list_head list;
};
typedef struct tag_sms_msg
{
    char phone_num[32];
    char sms_date[32];
    char sms_msg[512];//256 is no enough
    char sms_task[UUID_SIZE];
    char time_stamp[64];
    int sms_index;
    int sms_flag;
    int result;
    int msg_content;
    int msg_handle_type;
    struct list_head list;
}sms_msg;
#endif
typedef	struct _dongle_msg_entry
{
    DONGLE_MSG_TYPE type;       /**/    
#ifdef CONFIG_SUPPORT_SMS
    struct list_head sms_head;
#endif
    union 
    {
#ifdef CONFIG_SUPPORT_SMS
        sms_msg smsg;  
#endif
        voice_msg vmsg;
        mgnt_msg mmsg;
    }data;      
}dongle_msg_entry;

#endif

