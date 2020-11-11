#ifndef _RCL_VOICE_H_
#define _RCL_VOICE_H_

/* --------------------------------------------------------------------------*/
/**
 * @brief     check if voice service is running
 *
 * @Returns   0 - not running, 1 - running 
 */
/* ----------------------------------------------------------------------------*/
int rcl_voice_is_service_running(void);

/* --------------------------------------------------------------------------*/
/**
 * @brief     start voice service
 *
 * @Returns   0 - success,, -1 - fail to lock 
 */
/* ----------------------------------------------------------------------------*/
int  rcl_voice_waiting_for_up(int timeout);
/* --------------------------------------------------------------------------*/
/**
 * @brief     stop voice service 
 *
 * @Returns   0 - stop success, -1 - error
 */
/* ----------------------------------------------------------------------------*/
int rcl_voice_stop(void);

/* --------------------------------------------------------------------------*/
/**
 * @brief     update voice service's bind out interface
 *
 * @Param    ip             bind wan IP
 * @Param    ifName         bind wan interface name
 *
 * @Returns    
 */
/* ----------------------------------------------------------------------------*/
int rcl_voice_update_bind_interface(const char *ip, const char *ifName, const int wan_id);

/* --------------------------------------------------------------------------*/
/**
 * @brief     inform wan interface down to voice service
 */
/* ----------------------------------------------------------------------------*/
void rcl_voice_remove_interface(int wan_id);

void rcl_voice_send_ringing_schedule_message();
void rcl_voice_save_ringing_schedule_config();
void rcl_voice_ringing_schedule_config_init();

void rcl_voice_alarm_config_init();
void rcl_voice_start_alarm();
void rcl_voice_stop_alarm();


#endif//_RCL_VOICE_H_

