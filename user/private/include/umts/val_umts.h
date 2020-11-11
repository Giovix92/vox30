#ifndef _VAL_UMTS_H_
#define _VAL_UMTS_H_
#include <sys/socket.h>
#include <sys/un.h>
#include <umts/umts_socket.h>

#define DONGLE_AT_PORT "/tmp/at_port"
#define AT_RESULT_TMP  "/tmp/at_result"
#define AT_RESULT_OPT  "/tmp/at_opt"

enum umts_status_info
{
  UMTS_CONNECTION=0,
  UMTS_OPERATOR,
  UMTS_SIGNAL_STRENGTH,
  UMTS_SNR,
  UMTS_POWER,
  UMTS_TX_SPEED,
  UMTS_RX_SPEED,
  UMTS_MANUFACTURER,
  UMTS_MODEL,
  UMTS_IMEI,
  UMTS_IMSI,
  UMTS_FW_VERSION,
  UMTS_HW_VERSION,
};

enum umts_pin_status
{
    UMTS_PIN_INIT = 0,
    UMTS_PIN_READY,
    UMTS_PIN_INPUT,
    UMTS_PIN_ERROR,
    UMTS_PIN_PAUSE,
    UMTS_PUK_INPUT,
    UMTS_PINPUK_CHECKING,
};

enum umts_wizard_status
{
    UMTS_WIZARD_PIN_SUCCESS,
    UMTS_WIZARD_PIN_FAIL,
    UMTS_WIZARD_PIN_WAIT,
    UMTS_WIZARD_PIN_OVER,
    UMTS_WIZARD_PUK_SUCCESS,
    UMTS_WIZARD_PUK_FAIL,
    UMTS_WIZARD_PUK_WAIT,
    UMTS_WIZARD_PUK_OVER,
};

enum umts_connect_status
{
    UMTS_CONNECT,
    UMTS_NOT_CONNECT,
};

enum umts_signal_strength
{
    UMTS_SIGNAL_NULL,
    UMTS_SIGNAL_LOW,
    UMTS_SIGNAL_MIDDLE,
    UMTS_SIGNAL_HIGH,
    UMTS_SIGNAL_STRONG,
};

#define NETWORK_M_NOSIGNAL		0
#define NETWORK_M_GSM		    1
#define NETWORK_M_CDMA          2
#define NETWORK_M_WCDMA			3
#define NETWORK_M_TD_SCDMA		4
#define NETWORK_M_WIMAX         5
#define NETWORK_M_LTE           6

#define NETWORK_SINGAL_NOSIGNAL	0
#define NETWORK_SINGAL_LOW		1
#define NETWORK_SINGAL_MEDIUM	2
#define NETWORK_SINGAL_GOOD		3
#define NETWORK_SINGAL_STRONG	4

enum umts_sub_network
{
    UMTS_SUB_NETWORK_NOSIGNAL,
    UMTS_SUB_NETWORK_GSM,
    UMTS_SUB_NETWORK_GPRS,
    UMTS_SUB_NETWORK_EDGE,
    UMTS_SUB_NETWORK_WCDMA,
    UMTS_SUB_NETWORK_HSDPA,
    UMTS_SUB_NETWORK_HSUPA,
    UMTS_SUB_NETWORK_HSDPA_HSUPA,
    UMTS_SUB_NETWORK_TD_SCDMA,
    UMTS_SUB_NETWORK_HSPA_PLUS,
    UMTS_SUB_NETWORK_HSPA_64QAM,
    UMTS_SUB_NETWORK_HSPA_MIMO,
};

struct network_operator_map{
  char code[6];
  char network_operator[32];
};

struct umts_traffic_status
{
  int tx_rate;
  int rx_rate;
  int tx;
  int rx;
};

int val_umts_create_local_socket(struct sockaddr_un* name, char *path);
int val_umts_write_local_socket(int socket, char *buffer, int length, struct sockaddr* addr, socklen_t addrlen);
char *val_umts_get_imei();
char *val_umts_get_imsi();
char *val_umts_get_cardmodel();
char *val_umts_get_hwversion();
char *val_umts_get_manufacturer();
char *val_umts_get_smsnumber();
char *val_umts_get_cardnumber();
char *val_umts_get_swversion();
char *val_umts_get_usb_port_used();
char *val_umts_get_simstatus();
int val_umts_get_is_vf_sim();
int val_umts_get_pinstatus();
char *val_umts_get_pinrequired();
char *val_umts_get_network();
char *val_umts_get_networkmode();
char *val_umts_get_sub_networkmode();
char *val_umts_get_datastatus();
int val_umts_get_signalstrength();
int val_umts_get_lte_rsrp();
int val_umts_get_lte_rsrq();
char* val_umts_get_sn();
char* val_umts_get_cellid();
char *val_umts_get_networkband();
char *val_umts_get_voice_working();
int val_umts_lock(char *pin);
int val_umts_unlock(char *pin);
char *val_umts_get_signal(void);
char *val_umts_get_snr(void);
int val_umts_get_traffic_status(struct umts_traffic_status *traffic);
int val_umts_pin_change(char *old_pin, char *new_pin);
int val_umts_get_pinleft();
int val_umts_get_pinattempts();
int val_umts_get_pukleft();
int val_umts_get_pukattempts();
int val_umts_connect();
int val_umts_disconnect();
int val_umts_get_phy_status();
int val_umts_set_phy_status(char *value);
int val_umts_get_wizard_pin_status();
int val_umts_get_wizard_puk_status();
int val_umts_clear_wizard_status();
int val_umts_trigger_pin_check(char *value);
int val_umts_trigger_puk_check(char *value1, char * value2);
int val_umts_set_pin_config(char *pin, char *save_pin, char *disable_pin);
int val_umts_get_pin_config(char *pin, char *save_pin, char *disable_pin);
char * val_umts_get_puk();
char * val_umts_get_pin();
char* sal_flash_3g_get_networkCount_2gto3g();
char* sal_flash_3g_get_networkCount_2gto4g();
char* sal_flash_3g_get_networkCount_3gto2g();
char* sal_flash_3g_get_networkCount_3gto4g();
char* sal_flash_3g_get_networkCount_4gto3g();
char* sal_flash_3g_get_networkCount_4gto2g();
char* val_umts_get_lte_rssi();
char* sal_3g_get_dongle_data_networkmode_change_time();
char* sal_3g_get_dongle_voice_networkmode_change_time();
char* sal_3g_get_dongle_csvoice_networkmode_change_time();
int val_umts_set_puk(char *puk);
char * val_umts_get_dongle_module_name();
int val_umts_get_modemstatus();
int val_umts_set_log_level(char *level, char *path);
char* val_umts_get_operatorlist();
int val_umts_set_operator(char *opt);
int val_umts_set_apn(char *value);
int val_get_umts_state(umts_stat_data *stat);
int val_umts_remove();
#ifdef CONFIG_SUPPORT_FWA
int val_umts_set_fwa_used(char *fwa);
#endif
int val_umts_set_prefermode(char *value);
#ifdef CONFIG_SUPPORT_NEW_LED_BEHAV
int val_umts_set_new_led_behav_type(char *type);
int val_umts_set_default_wanid(char *wan_id);
#endif
#ifdef CONFIG_SUPPORT_SMS  
char * val_umts_get_sms_unread_count();
int val_umts_get_sms_is_full();
#endif
int val_umts_set_sms_phonenumber(char *value);
int val_umts_set_volte(char *value);
int val_umts_set_amrcodec(char *value);
char * val_umts_get_sms_msisdn(void);
char * val_umts_ndisd_start(void);
char * val_umts_ndisd_stop(void);
char * val_umts_vik_attach_start(void);
long val_umts_get_dataps2g();
long val_umts_get_dataps3g();
long val_umts_get_dataps4g();
long val_umts_get_voiceps3g();
long val_umts_get_voiceps4g();
char * val_umts_get_4g_registered_state();
char * val_umts_get_3g_registered_state();
#endif
