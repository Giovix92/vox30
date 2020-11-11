#ifndef __SAL_LED_H__
#define __SAL_LED_H__

//color 0: Green; 1: Red
void sal_output_led_set_key(int key, int state, int color);
void sal_output_led_blink_set_key(int key, int color, unsigned int count, int speed, int last_state);
void sal_output_led_all_off(void);
#ifdef VOX25
void sal_output_led_set_key_forced(int key, int state, int color);
#endif
void sal_output_led_get_key(int key, int color, int *state, int *blink_speed);
#endif
