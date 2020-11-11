#ifndef _CAL_DIAGNOSTICS_H_
#define _CAL_DIAGNOSTICS_H_
#include <stdlib.h>
#include <utility.h>

typedef struct diagArg_s
{
    CHAR diagnosticsState[32];
    CHAR interface[256];
    CHAR URL[256];
    BYTE DSCP;
    BYTE ethernetPriority;
    CHAR ROMTime[32];
    CHAR BOMTime[32];
    CHAR EOMTime[32];
    DWORD testBytesReceived;
    DWORD testBytesSent;
    DWORD totalBytesReceived;
    DWORD testFileLength;
    DWORD totalBytesSent;
    CHAR  TCPOpenRequestTime[32];
    CHAR  TCPOpenResponseTime[32];
    INT16  completeFlag;
}diag_t;

char *cal_diag_upload_get_state(void);
int cal_diag_upload_set_state(char* value);
char *cal_diag_download_get_state(void);
int cal_diag_download_set_state(char* value);
#ifdef CONFIG_SUPPORT_DSL
char *cal_diag_dsl_get_state(char *uri);
int cal_diag_dsl_set_state(char* value, char *uri);
int cal_diag_dsl_init(void);
char *cal_diag_atmf5_get_state(char *uri);
int cal_diag_atmf5_set_state(char* value, char *uri);
char *cal_diag_atmf5_get_num_of_rept(char *uri);
char *cal_diag_atmf5_get_timeout(char *uri);
char *cal_dsllink_get_enable(char *uri);
char *cal_dsllink_get_destaddr(char *uri);
int cal_diag_atmf5_init(void);
#endif

int cal_diag_download_interface(char* value);
char* cal_diag_get_download_interface(void);
int cal_diag_set_download_url(char* value);
char* cal_diag_get_download_url(void);
int cal_diag_set_download_bandwidth(char* value);
char* cal_diag_get_download_bandwidth(void);
char* cal_diag_get_download_max_con(void);
int cal_diag_set_download_dscp(char* value);
char* cal_diag_get_download_dscp(void);
int cal_diag_set_download_ethernet_priority(char* value);
char* cal_diag_get_download_ethernet_priority(void);
int cal_diag_set_download_time_based_test_duration(char* value);
char* cal_diag_get_download_time_based_test_duration(void);
int cal_diag_set_download_time_based_test_minterval(char* value);
char* cal_diag_get_download_time_based_test_minterval(void);
int cal_diag_set_download_time_based_test_moffset(char* value);
char* cal_diag_get_download_time_based_test_moffset(void);
int cal_diag_set_download_con_num(char* value);
char* cal_diag_get_download_con_num(void);
char* cal_diag_get_download_en_percon_results(void);


int cal_diag_set_upload_interface(char* value);
char* cal_diag_get_upload_interface(void);
int cal_diag_set_upload_url(char* value);
char* cal_diag_get_upload_url(void);
int cal_diag_set_upload_bandwidth(char* value);
char* cal_diag_get_upload_bandwidth(void);
int cal_diag_set_upload_dscp(char* value);
char* cal_diag_get_upload_dscp(void);
int cal_diag_set_upload_ethernet_priority(char* value);
char* cal_diag_get_upload_ethernet_priority(void);
int cal_diag_set_upload_testfile_length(char* value);
char* cal_diag_get_upload_testfile_length(void);
int cal_diag_set_upload_time_based_test_duration(char* value);
char* cal_diag_get_upload_time_based_test_duration(void);
int cal_diag_set_upload_time_based_test_minterval(char* value);
char* cal_diag_get_upload_time_based_test_minterval(void);
int cal_diag_set_upload_time_based_test_moffset(char* value);
char* cal_diag_get_upload_time_based_test_moffset(void);
char* cal_diag_get_upload_max_con(void);
int cal_diag_set_upload_con_num(char* value);
char* cal_diag_get_upload_con_num(void);
char* cal_diag_get_upload_en_percon_results(void);

char* cal_diag_get_localport(void);
char* cal_diag_get_localport_range(void);

int cal_diag_set_udpecho_state(char* value);
char* cal_diag_get_udpecho_port(void);
char* cal_diag_get_udpecho_interface(void);
char* cal_diag_get_udpecho_host(void);
char* cal_diag_get_udpecho_number_of_repetitions(void);
char *cal_diag_get_udpecho_individual_results_enable(void);
char *cal_diag_get_udpecho_max_results(void);
char *cal_diag_get_udpecho_timeout(void);
char *cal_diag_get_udpecho_data_block_size(void);
char *cal_diag_get_udpecho_dscp(void);
char *cal_diag_get_udpecho_inter_transmission_time(void);

int cal_diag_hw_set_state(char* value);
char* cal_diag_hw_get_state(void);
char* cal_diag_hw_get_interval(void);
char* cal_diag_hw_get_times(void);
char* cal_diag_hw_get_time(void);
int cal_diag_hw_set_interval(char* value);
int cal_diag_hw_set_times(char* value);
int cal_diag_hw_set_time(char* value);
#endif

