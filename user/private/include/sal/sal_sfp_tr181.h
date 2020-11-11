
#ifndef __SAL_TR181_H__
#define __SAL_TR181_H__


//Device info
char *sal_get_sfp_manufacture(void);

char *sal_get_sfp_sfp_model_name(void);

char *sal_get_sfp_hw_version(void);

char *sal_get_sfp_fw_version(void);

char *sal_get_sfp_description(void);

char *sal_get_sfp_product_class(void);

char *sal_get_sfp_sn(void);

char *sal_get_sfp_uptime(void);


//status
char *sal_get_sfp_status(void);

char *sal_get_sfp_last_change(void);

char *sal_get_sfp_optical_signal_level(void);

char *sal_get_sfp_lower_optical_threshold(void);

char *sal_get_sfp_upper_optical_threshold(void);

char *sal_get_sfp_transmit_optical_level(void);

char *sal_get_sfp_lower_transmit_power_threshold(void);

char *sal_get_sfp_upper_transmit_power_threshold(void);

char *sal_get_x_vodafone_bias(void);

char *sal_get_x_vodafone_voltage(void);

char *sal_get_x_vodafone_chipset_temperature(void);

char *sal_get_x_vodafone_operstate(void);

char *sal_get_x_vodafone_ontready(void);

char *sal_get_x_vodafone_onuid(void);

char *sal_get_sfp_bytes_sent(void);

char *sal_get_sfp_bytes_received(void);

char *sal_get_sfp_packets_sent(void);

char *sal_get_sfp_packets_received(void);

char *sal_get_x_vodafone_bip(void);

char *sal_get_sfp_errors_sent(void);

char *sal_get_sfp_errors_received(void);

char *sal_get_sfp_discard_packets_sent(void);

char *sal_get_sfp_discard_packets_received(void);

char *sal_get_sfp_device_log();
#endif
