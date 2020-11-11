/**
 * @file   bbu.c
 * @author Martin Huang martin_huang@sdc.sercomm.com
 * @date   2013-04-11
 * @brief  support bbu monitor interface abstract layer API.
 *
 * Copyright - 2013 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifisally
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */

#ifndef __SAL_BBU_H__
#define __SAL_BBU_H__

int sal_bbu_get_A_in_state(void);
int sal_bbu_get_B_in_state(void);
int sal_bbu_is_battery_on(void);
int sal_bbu_is_ac_off(void);
int sal_bbu_is_battery_low(void);
void sal_bbu_wait_event(void);
void sal_bbu_set_led_on(void);
void sal_bbu_set_led_off(void);
void sal_bbu_set_led_blink(void);
void sal_bbu_set_led_blink_fast(void);
#endif

