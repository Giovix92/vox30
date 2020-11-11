/**
 * @file service_timer_api.h
 * @author Phil Zhang
 * @date   2009-11-18
 * @brief  head file for service time out function.   
 *
 * Copyright - 2009 SerComm Corporation. All Rights Reserved. 
 * SerComm Corporation reserves the right to make changes to this document without notice. 
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability 
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising 
 * out of the application or use of any product or circuit. SerComm Corporation specifically 
 * disclaims any and all liability, including without limitation consequential or incidental damages; 
 * neither does it convey any license under its patent rights, nor the rights of others. 
 */
 
#ifndef __SERVICE_TIMER_API_H__
#define __SERVICE_TIMER_API_H__ 
 
#define TIME_STRING_LENGTH 16
#define ONE_YEAR_SECONDS (3600*24*365)

enum trigger_service_id{
    FW_TIMER_SERVICE_HTTP = 0,
    FW_TIMER_SERVICE_HTTPS,
    MAX_FW_TIMER_SERVICE_ID,
};

/*
 * @fn void trigger_finish_time(int rule_id, int interval_minutes)
 * @brief create a file to record the finish time of the service
 * @param[in] rule_id: type of service, see enum trigger_service_id, interval_minutes: the duration minutes of the service
 * @return 
 */
void trigger_finish_time(int rule_id, int interval_minutes);

/*
 * @fn long get_finish_time(int rule_id)
 * @brief get service finish time(total seconds)
 * @param[in] type of service, see enum trigger_service_id
 * @return service finish time(total seconds)
 */
long get_finish_time(int rule_id);

#endif //__SERVICE_TIMER_API_H__

void stop_finish_time(int rule_id);

int get_remote_service_id(void);

