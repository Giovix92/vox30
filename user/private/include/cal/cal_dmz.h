#ifndef _CAL_DMZ_H_
#define _CAL_DMZ_H_

#define CAL_DMZ_TMP_MAX_CHAR            8
#define CAL_DMZ_IP_MAX_CHAR             16
#define DMZ_ENABLE	1
#define DMZ_DISABLE	0


typedef struct
{
    char enable[CAL_DMZ_TMP_MAX_CHAR];/* 0, 1 */
    char ipaddr[CAL_DMZ_IP_MAX_CHAR];
    char time[CAL_DMZ_TMP_MAX_CHAR];    
}cal_dmz;

char* dmz_get_enable(void);
int cal_dmz_get(cal_dmz* dmz);
int cal_dmz_set(cal_dmz* dmz);   
    
#endif

