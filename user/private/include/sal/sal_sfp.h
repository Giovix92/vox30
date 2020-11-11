#ifndef __SAL_TR181_H__
#define __SAL_TR181_H__

#define LEN_MANUFACTURE_MAX 20
#define LEN_MODEL_NAME_MAX 20
#define LEN_PRODUCT_CLASS_MAX 24
#define LEN_DESCRIPTION_MAX 128
#define LEN_SERIAL_NUMBER_LEN 20
#define LEN_HW_VERSION_MAX 20
#define LEN_SW_VERSION_MAX 20

#define LEN_SFP_INFO 128
#define LEN_SFP_LOG  2048



typedef struct static_sfp_info
{
	char sfp_manufacturer[LEN_MANUFACTURE_MAX + 1];
	char sfp_model_name[LEN_MODEL_NAME_MAX + 1];
	char sfp_product_class[LEN_PRODUCT_CLASS_MAX + 1];
	char sfp_description[LEN_DESCRIPTION_MAX + 1];
	char sfp_sn[LEN_SERIAL_NUMBER_LEN + 1];
	char sfp_hw_version[LEN_HW_VERSION_MAX + 1];
	char sfp_sw_version[LEN_SW_VERSION_MAX + 1];
	char sfp_device_log[LEN_SFP_LOG];
}static_sfp_t;

typedef struct dyna_sfp_info
{
	char sfp_uptime[LEN_SFP_INFO];
	char sfp_last_change[LEN_SFP_INFO];
	char sfp_rx[LEN_SFP_INFO];
	char sfp_lower_rx[LEN_SFP_INFO];
	char sfp_upper_rx[LEN_SFP_INFO];
	char sfp_tx[LEN_SFP_INFO];
	char sfp_lower_tx[LEN_SFP_INFO];
	char sfp_upper_tx[LEN_SFP_INFO];
	char sfp_bias[LEN_SFP_INFO];
	char sfp_voltage[LEN_SFP_INFO];
	char sfp_temperature[LEN_SFP_INFO];
	char sfp_onu_id[LEN_SFP_INFO];
	char sfp_bytes_sent[LEN_SFP_INFO];
	char sfp_bytes_received[LEN_SFP_INFO];
	char sfp_packets_sent[LEN_SFP_INFO];
	char sfp_packets_received[LEN_SFP_INFO];
	char sfp_bip[LEN_SFP_INFO];
	char sfp_errors_sent[LEN_SFP_INFO];
	char sfp_errors_received[LEN_SFP_INFO];
	char sfp_discard_packets_sent[LEN_SFP_INFO];
	char sfp_discard_packets_received[LEN_SFP_INFO];
;
}dynamic_sfp_t;

//Device info
char *sal_get_sfp_manufacturer(void);
char *sal_get_sfp_model_name(void);
char *sal_get_sfp_hw_version(void);
char *sal_get_sfp_sw_version(void);
char *sal_get_sfp_description(void);
char *sal_get_sfp_product_class(void);
char *sal_get_sfp_sn(void);
char *sal_get_sfp_uptime(void);

//status
char *sal_get_sfp_last_change(void);
char *sal_get_sfp_rx(void);
char *sal_get_sfp_lower_rx(void);
char *sal_get_sfp_upper_rx(void);
char *sal_get_sfp_tx(void);
char *sal_get_sfp_lower_tx(void);
char *sal_get_sfp_upper_tx(void);
char *sal_get_sfp_bias(void);
char *sal_get_sfp_voltage(void);
char *sal_get_sfp_temperature(void);
char *sal_get_sfp_onuid(void);


char *sal_get_sfp_bytes_sent(void);
char *sal_get_sfp_bytes_received(void);
char *sal_get_sfp_packets_sent(void);
char *sal_get_sfp_packets_received(void);
char *sal_get_sfp_bip(void);
char *sal_get_sfp_errors_sent(void);
char *sal_get_sfp_errors_received(void);
char *sal_get_sfp_discard_packets_sent(void);
char *sal_get_sfp_discard_packets_received(void);

char *sal_get_sfp_status();
char *sal_get_sfp_oper_stat();
char *sal_get_sfp_ont_ready();
char *sal_get_sfp_device_log();
char *sal_get_sfp_onu_id();
#endif
