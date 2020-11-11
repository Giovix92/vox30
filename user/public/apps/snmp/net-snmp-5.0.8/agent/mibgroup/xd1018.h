#ifndef AD1018_H
#define AD1018_H

/****************************************************************************
*                               Includes                                    *
****************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/library/container.h>
#include <net-snmp/agent/table_array.h>

#include <sys/queue.h>

/****************************************************************************
*                          private MIB structures                           *
****************************************************************************/
static struct xd1018 {

//cpustatus	
    char cpuutilization[64];
//meminfo
    char memused[64];
//nat
    int nat_active;
//voice status
    char voice_status[128];
//fxs1
    char fxs1_status[128];
//fxs2
    char fxs2_status[128];
//signal to noise
    char signaltonoise_ratio_down[128];
    char signaltonoise_ratio_up[128];
//line attenuation
    char lineattenuation_down[128];
    char lineattenuation_up[128];
}nap, *ap = &nap;

/*
 * function declarations 
 */
void            init_xd1018(void);
FindVarMethod   var_xd1018;
#endif
