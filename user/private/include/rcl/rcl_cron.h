#ifndef _RCL_CRON_H
#define _RCL_CRON_H

int rcl_cron_start_pctrl(int argc, char** argv);
int rcl_cron_stop_pctrl(int argc, char** argv);

int rcl_cron_start_wlan_sch(int argc, char** argv);
int rcl_cron_stop_wlan_sch(int argc, char** argv);
int rcl_cron_start_tcpdump(int argc, char** argv);
int rcl_cron_stop_tcpdump(int argc, char** argv);


#endif

