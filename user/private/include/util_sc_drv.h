#ifndef _UTIL_SC_DRV_H_
#define _UTIL_SC_DRV_H_

#include "sc_drv/sc_drv.h"
#include "sc_drv/sc_drv_eth.h"
#include "sc_drv/input.h"

//typedef unsigned long int	iperf_size_t;
int util_scDrv_get_eth_port_change(ETH_PORT_CHANGE *eth_port_change);
int util_scDrv_get_input_key_state(INPUT_KEY *key);
int util_scDrv_get_input_key_event(INPUT_EVENT *event);
int util_scDrv_set_output_key(OUTPUT_KEY *output);
int util_scDrv_get_output_key(OUTPUT_KEY *output);
int util_scDrv_add_dev_list(struct QoS_DEV* dev);
int util_scDrv_clear_qos_dev_list(void);
int util_scDrv_get_tcpackprio_status(int *status);
int util_scDrv_set_tcpackprio_enable(int enable);
int util_scDrv_set_tcpack_xtmprio(unsigned int xtm_prio);
int util_scDrv_set_tcpack_ethprio(unsigned int eth_prio);
#ifdef VOX25
int util_scDrv_get_sensor_state(int *state);	
int util_scDrv_get_sensor_event(int *state);
int util_scDrv_set_output_key_forced(OUTPUT_KEY *output);
void sal_output_led_blink_set_key_forced(int key, int color, unsigned int count, int speed, int last_state);
#endif
#ifdef CONFIG_SUPPORT_ENERGY_SAVING
int util_scDrv_set_energy_saving_mode(STR_ENERGY_SAVING *status);
#endif
#ifdef CONFIG_SUPPORT_5G_QD
int util_scDrv_reset_qd_wifi(void);
#endif
#if defined(SUPPORT_USB_STORAGE) || defined(SUPPORT_USB_PRINTER) || defined(CONFIG_SUPPORT_3G)  
int util_scDrv_set_output_usb_state(OUTPUT_USB_STATE_CMD *cmd);
int util_scDrv_get_specifytype_usb_state(OUTPUT_USB_STATE_DEV *usb_state);
#endif
#endif /* _UTIL_SC_DRV_H_ */
