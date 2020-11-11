#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <nvram.h>

#define SAL_DIAG_TMP_VALUE_MAX_LENGTH	256
#define SAL_DIAG_TMP_PATH_MAX_LENGTH	256

#define DIAG_CFG_BASE_DOWNLOAD "/tmp/sal/download_diag.sal"
#define DIAG_CFG_BASE_UPLOAD "/tmp/sal/upload_diag.sal"
#define DIAG_CFG_BASE_PER_DOWNLOAD "/tmp/sal/download_per_diag.sal"
#define DIAG_CFG_BASE_PER_UPLOAD "/tmp/sal/upload_per_diag.sal"

#define NVRAN_GET_DIAG_FUNC(funcname,name)\
char *funcname(void)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_DOWNLOAD);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(name, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_DIAG_FUNC(funcname, name)\
int funcname(char *value)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
		if(!value)\
			return -1;\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_DOWNLOAD);\
		return nvram_set_p(diag_nvram_path, name, value);\
	}\
}
#define NVRAN_GET_DIAG_FUNC2(funcname,name)\
char *funcname(void)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_UPLOAD);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(name, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_DIAG_FUNC2(funcname, name)\
int funcname(char *value)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
		if(!value)\
			return -1;\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_UPLOAD);\
		return nvram_set_p(diag_nvram_path, name, value);\
	}\
}
#define SAL_DIAG_UPLOAD_ROMTIME   "ul_rom_time"
#define SAL_DIAG_UPLOAD_BOMTIME   "ul_bom_time"
#define SAL_DIAG_UPLOAD_EOMTIME   "ul_eom_time"
#define SAL_DIAG_UPLOAD_TESTBS   "ul_test_bs"
#define SAL_DIAG_UPLOAD_TOTALBR   "ul_total_br"
#define SAL_DIAG_UPLOAD_TOTALBS   "ul_total_bs"
#define SAL_DIAG_UPLOAD_TCPOPEN_REQTIME   "ul_tcpopen_req"
#define SAL_DIAG_UPLOAD_TCPOPEN_RESTIME   "ul_tcpopen_res"
#define SAL_DIAG_UPLOAD_TESTBS_FL   "ul_testbs_fl"
#define SAL_DIAG_UPLOAD_TOTALBR_FL  "ul_totalbr_fl"
#define SAL_DIAG_UPLOAD_TOTALBS_FL  "ul_totalbs_fl"
#define SAL_DIAG_UPLOAD_PERIOD_FL  "ul_period_fl"

#define SAL_DIAG_DOWNLOAD_ROMTIME   "dl_rom_time"
#define SAL_DIAG_DOWNLOAD_BOMTIME   "dl_bom_time"
#define SAL_DIAG_DOWNLOAD_EOMTIME   "dl_eom_time"
#define SAL_DIAG_DOWNLOAD_TESTBR    "dl_test_br"
#define SAL_DIAG_DOWNLOAD_TOTALBR   "dl_total_br"
#define SAL_DIAG_DOWNLOAD_TOTALBS   "dl_total_bs"
#define SAL_DIAG_DOWNLOAD_TCPOPEN_REQTIME   "dl_tcpopen_req"
#define SAL_DIAG_DOWNLOAD_TCPOPEN_RESTIME   "dl_tcpopen_res"
#define SAL_DIAG_DOWNLOAD_TESTBR_FL   "dl_testbr_fl"
#define SAL_DIAG_DOWNLOAD_TOTALBR_FL  "dl_totalbr_fl"
#define SAL_DIAG_DOWNLOAD_TOTALBS_FL  "dl_totalbs_fl"
#define SAL_DIAG_DOWNLOAD_PERIOD_FL  "dl_period_fl"

#define SAL_DIAG_IPPING_COMPLETE "ipping_complete"
#define SAL_DIAG_WIFIDUMP_COMPLETE "WiFiDump_complete"
#define SAL_DIAG_DOWNLOAD_COMPLETE "download_complete"
#define SAL_DIAG_UPLOAD_COMPLETE "upload_complete"
#define SAL_DIAG_TRACEROUTE_COMPLETE "traceroute_complete"
#define SAL_DIAG_WIFIRADAR_COMPLETE "wifiradar_complete"
#define SAL_DIAG_NSLOOKUP_COMPLETE "nslookup_complete"
#ifdef CONFIG_SUPPORT_DSL
#define SAL_DIAG_DSL_COMPLETE "dsl_complete"
#define SAL_DIAG_ATMF5_COMPLETE "atmf5_complete"
#endif

#define SAL_DIAG_NSLOOKUP_START  "nslookupDiag_Start"
#define SAL_DIAG_TRACEROUTE_START  "tracerouteDiag_Start"
#define SAL_DIAG_WIFIRADAR_START  "wifiradarDiag_Start"
#define SAL_DIAG_IPPING_START  "ippingDiag_Start"
#define SAL_DIAG_WIFIDUMP_START  "WiFiDumpDiag_Start"
#define SAL_DIAG_UPLOAD_START  "uploadDiag_Start"
#define SAL_DIAG_DOWNLOAD_START  "downloadDiag_Start"
#ifdef CONFIG_SUPPORT_DSL
#define SAL_DIAG_DSL_START  "dslDiag_Start"
#define SAL_DIAG_ATMF5_START  "atmf5Diag_Start"
#endif

#define SAL_DIAG_UPLOAD_IC_TESTBS   "uploadDiag_ic_test_bs"
#define SAL_DIAG_DOWNLOAD_IC_TESTBR   "dlDiag_ic_test_br"
#define SAL_DIAG_UPLOAD_IC_TOTALBR  "uploadDiag_ic_total_br"
#define SAL_DIAG_DOWNLOAD_IC_TOTALBR  "dlDiag_ic_total_br"
#define SAL_DIAG_UPLOAD_IC_TOTALBS  "uploadDiag_ic_total_bs"
#define SAL_DIAG_DOWNLOAD_IC_TOTALBS    "dlDiag_ic_total_bs"
#define SAL_DIAG_UPLOAD_IC_STARTTIME    "uploadDiag_ic_start_time"
#define SAL_DIAG_DOWNLOAD_IC_STARTTIME    "dlDiag_ic_start_time"
#define SAL_DIAG_UPLOAD_IC_ENDTIME    "uploadDiag_ic_end_time"
#define SAL_DIAG_DOWNLOAD_IC_ENDTIME    "dlDiag_ic_end_time"
#define SAL_DIAG_IPERF_RET              "iperf_ret"
NVRAN_GET_DIAG_FUNC2(sal_diag_get_iperf_ret, SAL_DIAG_IPERF_RET)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_iperf_ret, SAL_DIAG_IPERF_RET)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_rom_time, SAL_DIAG_UPLOAD_ROMTIME)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_rom_time, SAL_DIAG_UPLOAD_ROMTIME)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_bom_time, SAL_DIAG_UPLOAD_BOMTIME)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_bom_time, SAL_DIAG_UPLOAD_BOMTIME)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_eom_time, SAL_DIAG_UPLOAD_EOMTIME)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_eom_time, SAL_DIAG_UPLOAD_EOMTIME)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_test_bs, SAL_DIAG_UPLOAD_TESTBS)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_test_bs, SAL_DIAG_UPLOAD_TESTBS)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_total_bs, SAL_DIAG_UPLOAD_TOTALBS)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_total_bs, SAL_DIAG_UPLOAD_TOTALBS)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_total_br, SAL_DIAG_UPLOAD_TOTALBR)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_total_br, SAL_DIAG_UPLOAD_TOTALBR)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_tcpopen_reqtime, SAL_DIAG_UPLOAD_TCPOPEN_REQTIME)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_tcpopen_reqtime, SAL_DIAG_UPLOAD_TCPOPEN_REQTIME)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_tcpopen_restime, SAL_DIAG_UPLOAD_TCPOPEN_RESTIME)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_tcpopen_restime, SAL_DIAG_UPLOAD_TCPOPEN_RESTIME)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_testbs_fl, SAL_DIAG_UPLOAD_TESTBS_FL)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_testbs_fl, SAL_DIAG_UPLOAD_TESTBS_FL)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_totalbr_fl, SAL_DIAG_UPLOAD_TOTALBR_FL)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_totalbr_fl, SAL_DIAG_UPLOAD_TOTALBR_FL)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_totalbs_fl, SAL_DIAG_UPLOAD_TOTALBS_FL)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_totalbs_fl, SAL_DIAG_UPLOAD_TOTALBS_FL)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_period_fl, SAL_DIAG_UPLOAD_PERIOD_FL)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_period_fl, SAL_DIAG_UPLOAD_PERIOD_FL)

NVRAN_GET_DIAG_FUNC(sal_diag_get_download_rom_time, SAL_DIAG_DOWNLOAD_ROMTIME)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_rom_time, SAL_DIAG_DOWNLOAD_ROMTIME)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_bom_time, SAL_DIAG_DOWNLOAD_BOMTIME)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_bom_time, SAL_DIAG_DOWNLOAD_BOMTIME)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_eom_time, SAL_DIAG_DOWNLOAD_EOMTIME)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_eom_time, SAL_DIAG_DOWNLOAD_EOMTIME)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_test_br,  SAL_DIAG_DOWNLOAD_TESTBR)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_test_br,  SAL_DIAG_DOWNLOAD_TESTBR)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_total_br, SAL_DIAG_DOWNLOAD_TOTALBR)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_total_br, SAL_DIAG_DOWNLOAD_TOTALBR)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_total_bs, SAL_DIAG_DOWNLOAD_TOTALBS)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_total_bs, SAL_DIAG_DOWNLOAD_TOTALBS)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_tcpopen_reqtime, SAL_DIAG_DOWNLOAD_TCPOPEN_REQTIME)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_tcpopen_reqtime, SAL_DIAG_DOWNLOAD_TCPOPEN_REQTIME)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_tcpopen_restime, SAL_DIAG_DOWNLOAD_TCPOPEN_RESTIME)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_tcpopen_restime, SAL_DIAG_DOWNLOAD_TCPOPEN_RESTIME)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_testbr_fl, SAL_DIAG_DOWNLOAD_TESTBR_FL)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_testbr_fl, SAL_DIAG_DOWNLOAD_TESTBR_FL)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_totalbr_fl, SAL_DIAG_DOWNLOAD_TOTALBR_FL)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_totalbr_fl, SAL_DIAG_DOWNLOAD_TOTALBR_FL)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_totalbs_fl, SAL_DIAG_DOWNLOAD_TOTALBS_FL)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_totalbs_fl, SAL_DIAG_DOWNLOAD_TOTALBS_FL)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_period_fl, SAL_DIAG_DOWNLOAD_PERIOD_FL)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_period_fl, SAL_DIAG_DOWNLOAD_PERIOD_FL)

/*
NVRAN_SET_DIAG_FUNC(sal_diag_set_ipping_complete, SAL_DIAG_IPPING_COMPLETE)
NVRAN_GET_DIAG_FUNC(sal_diag_get_ipping_complete, SAL_DIAG_IPPING_COMPLETE)
NVRAN_SET_DIAG_FUNC(sal_diag_set_wifidump_complete, SAL_DIAG_WIFIDUMP_COMPLETE)
NVRAN_GET_DIAG_FUNC(sal_diag_get_wifidump_complete, SAL_DIAG_WIFIDUMP_COMPLETE)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_complete, SAL_DIAG_DOWNLOAD_COMPLETE)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_complete, SAL_DIAG_DOWNLOAD_COMPLETE)
NVRAN_SET_DIAG_FUNC2(sal_diag_set_upload_complete, SAL_DIAG_UPLOAD_COMPLETE)
NVRAN_GET_DIAG_FUNC2(sal_diag_get_upload_complete, SAL_DIAG_UPLOAD_COMPLETE)
NVRAN_SET_DIAG_FUNC(sal_diag_set_traceroute_complete, SAL_DIAG_TRACEROUTE_COMPLETE)
NVRAN_GET_DIAG_FUNC(sal_diag_get_traceroute_complete, SAL_DIAG_TRACEROUTE_COMPLETE)
NVRAN_SET_DIAG_FUNC(sal_diag_set_wifiradar_complete, SAL_DIAG_WIFIRADAR_COMPLETE)
NVRAN_GET_DIAG_FUNC(sal_diag_get_wifiradar_complete, SAL_DIAG_WIFIRADAR_COMPLETE)
NVRAN_SET_DIAG_FUNC(sal_diag_set_nslookup_complete, SAL_DIAG_NSLOOKUP_COMPLETE)
NVRAN_GET_DIAG_FUNC(sal_diag_get_nslookup_complete, SAL_DIAG_NSLOOKUP_COMPLETE)
#ifdef CONFIG_SUPPORT_DSL
NVRAN_SET_DIAG_FUNC(sal_diag_set_dsl_complete, SAL_DIAG_DSL_COMPLETE)
NVRAN_GET_DIAG_FUNC(sal_diag_get_dsl_complete, SAL_DIAG_DSL_COMPLETE)
NVRAN_SET_DIAG_FUNC(sal_diag_set_atmf5_complete, SAL_DIAG_ATMF5_COMPLETE)
NVRAN_GET_DIAG_FUNC(sal_diag_get_atmf5_complete, SAL_DIAG_ATMF5_COMPLETE)
#endif

NVRAN_SET_DIAG_FUNC(sal_diag_set_nslookup_start, SAL_DIAG_NSLOOKUP_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_nslookup_start, SAL_DIAG_NSLOOKUP_START)
NVRAN_SET_DIAG_FUNC(sal_diag_set_traceroute_start, SAL_DIAG_TRACEROUTE_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_traceroute_start, SAL_DIAG_TRACEROUTE_START)
NVRAN_SET_DIAG_FUNC(sal_diag_set_wifiradar_start, SAL_DIAG_WIFIRADAR_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_wifiradar_start, SAL_DIAG_WIFIRADAR_START)
NVRAN_SET_DIAG_FUNC(sal_diag_set_ipping_start, SAL_DIAG_IPPING_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_ipping_start, SAL_DIAG_IPPING_START)
NVRAN_SET_DIAG_FUNC(sal_diag_set_wifidump_start, SAL_DIAG_WIFIDUMP_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_wifidump_start, SAL_DIAG_WIFIDUMP_START)
*/
NVRAN_SET_DIAG_FUNC(sal_diag_set_upload_start, SAL_DIAG_UPLOAD_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_upload_start, SAL_DIAG_UPLOAD_START)
NVRAN_SET_DIAG_FUNC(sal_diag_set_download_start, SAL_DIAG_DOWNLOAD_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_download_start, SAL_DIAG_DOWNLOAD_START)
/*
#ifdef CONFIG_SUPPORT_DSL
NVRAN_SET_DIAG_FUNC(sal_diag_set_dsl_start, SAL_DIAG_DSL_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_dsl_start, SAL_DIAG_DSL_START)
NVRAN_SET_DIAG_FUNC(sal_diag_set_atmf5_start, SAL_DIAG_ATMF5_START)
NVRAN_GET_DIAG_FUNC(sal_diag_get_atmf5_start, SAL_DIAG_ATMF5_START)
#endif
*/
#define NVRAN_GET_DIAG_PER_FUNC(funcname,name)\
char *funcname(int idx)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char per_name[35];\
        snprintf(per_name, sizeof(per_name), "%s_%d", name, idx);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_PER_DOWNLOAD);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(per_name, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_DIAG_PER_FUNC(funcname, name)\
int funcname(char *value, int idx)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char per_name[35];\
		if(!value)\
			return -1;\
        snprintf(per_name, sizeof(per_name), "%s_%d", name, idx);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_PER_DOWNLOAD);\
		return nvram_set_p(diag_nvram_path, per_name, value);\
	}\
}
#define NVRAN_GET_DIAG_PER_FUNC2(funcname,name)\
char *funcname(int idx)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char per_name[35];\
        snprintf(per_name, sizeof(per_name), "%s_%d", name, idx);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_PER_UPLOAD);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(per_name, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_DIAG_PER_FUNC2(funcname, name)\
int funcname(char *value, int idx)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char per_name[35];\
		if(!value)\
			return -1;\
        snprintf(per_name, sizeof(per_name), "%s_%d", name, idx);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_PER_UPLOAD);\
		return nvram_set_p(diag_nvram_path, per_name, value);\
	}\
}
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_per_rom_time, SAL_DIAG_UPLOAD_ROMTIME)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_per_rom_time, SAL_DIAG_UPLOAD_ROMTIME)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_per_bom_time, SAL_DIAG_UPLOAD_BOMTIME)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_per_bom_time, SAL_DIAG_UPLOAD_BOMTIME)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_per_eom_time, SAL_DIAG_UPLOAD_EOMTIME)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_per_eom_time, SAL_DIAG_UPLOAD_EOMTIME)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_per_test_bs, SAL_DIAG_UPLOAD_TESTBS)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_per_test_bs, SAL_DIAG_UPLOAD_TESTBS)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_per_total_br, SAL_DIAG_UPLOAD_TOTALBR)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_per_total_br, SAL_DIAG_UPLOAD_TOTALBR)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_per_total_bs, SAL_DIAG_UPLOAD_TOTALBS)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_per_total_bs, SAL_DIAG_UPLOAD_TOTALBS)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_per_tcpopen_reqtime, SAL_DIAG_UPLOAD_TCPOPEN_REQTIME)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_per_tcpopen_reqtime, SAL_DIAG_UPLOAD_TCPOPEN_REQTIME)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_per_tcpopen_restime, SAL_DIAG_UPLOAD_TCPOPEN_RESTIME)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_per_tcpopen_restime, SAL_DIAG_UPLOAD_TCPOPEN_RESTIME)

NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_per_rom_time, SAL_DIAG_DOWNLOAD_ROMTIME)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_per_rom_time, SAL_DIAG_DOWNLOAD_ROMTIME)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_per_bom_time, SAL_DIAG_DOWNLOAD_BOMTIME)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_per_bom_time, SAL_DIAG_DOWNLOAD_BOMTIME)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_per_eom_time, SAL_DIAG_DOWNLOAD_EOMTIME)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_per_eom_time, SAL_DIAG_DOWNLOAD_EOMTIME)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_per_test_br,  SAL_DIAG_DOWNLOAD_TESTBR)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_per_test_br,  SAL_DIAG_DOWNLOAD_TESTBR)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_per_total_br, SAL_DIAG_DOWNLOAD_TOTALBR)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_per_total_br, SAL_DIAG_DOWNLOAD_TOTALBR)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_per_total_bs, SAL_DIAG_DOWNLOAD_TOTALBS)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_per_total_bs, SAL_DIAG_DOWNLOAD_TOTALBS)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_per_tcpopen_reqtime, SAL_DIAG_DOWNLOAD_TCPOPEN_REQTIME)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_per_tcpopen_reqtime, SAL_DIAG_DOWNLOAD_TCPOPEN_REQTIME)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_per_tcpopen_restime, SAL_DIAG_DOWNLOAD_TCPOPEN_RESTIME)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_per_tcpopen_restime, SAL_DIAG_DOWNLOAD_TCPOPEN_RESTIME)

NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_ic_test_bs, SAL_DIAG_UPLOAD_IC_TESTBS)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_ic_test_bs, SAL_DIAG_UPLOAD_IC_TESTBS)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_ic_total_br, SAL_DIAG_UPLOAD_IC_TOTALBR)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_ic_total_br, SAL_DIAG_UPLOAD_IC_TOTALBR)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_ic_total_bs, SAL_DIAG_UPLOAD_IC_TOTALBS)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_ic_total_bs, SAL_DIAG_UPLOAD_IC_TOTALBS)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_ic_start_time, SAL_DIAG_UPLOAD_IC_STARTTIME)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_ic_start_time, SAL_DIAG_UPLOAD_IC_STARTTIME)
NVRAN_GET_DIAG_PER_FUNC2(sal_diag_get_upload_ic_end_time, SAL_DIAG_UPLOAD_IC_ENDTIME)
NVRAN_SET_DIAG_PER_FUNC2(sal_diag_set_upload_ic_end_time, SAL_DIAG_UPLOAD_IC_ENDTIME)

NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_ic_test_br, SAL_DIAG_DOWNLOAD_IC_TESTBR)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_ic_test_br, SAL_DIAG_DOWNLOAD_IC_TESTBR)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_ic_total_br, SAL_DIAG_DOWNLOAD_IC_TOTALBR)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_ic_total_br, SAL_DIAG_DOWNLOAD_IC_TOTALBR)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_ic_total_bs, SAL_DIAG_DOWNLOAD_IC_TOTALBS)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_ic_total_bs, SAL_DIAG_DOWNLOAD_IC_TOTALBS)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_ic_start_time, SAL_DIAG_DOWNLOAD_IC_STARTTIME)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_ic_start_time, SAL_DIAG_DOWNLOAD_IC_STARTTIME)
NVRAN_GET_DIAG_PER_FUNC(sal_diag_get_download_ic_end_time, SAL_DIAG_DOWNLOAD_IC_ENDTIME)
NVRAN_SET_DIAG_PER_FUNC(sal_diag_set_download_ic_end_time, SAL_DIAG_DOWNLOAD_IC_ENDTIME)

/*
#define DIAG_CFG_BASE_UDPECHO "/tmp/sal/udpecho_diag.sal"
#define NVRAN_GET_UDPECHO_DIAG_FUNC(funcname,name)\
char *funcname(char *cb)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char name_cb[35];\
        snprintf(name_cb, sizeof(name_cb), "%s_%s", name, cb);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_UDPECHO);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(name_cb, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_UDPECHO_DIAG_FUNC(funcname, name)\
int funcname(char *value, char *cb)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char name_cb[35];\
		if(!value)\
			return -1;\
        snprintf(name_cb, sizeof(name_cb), "%s_%s", name, cb);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_UDPECHO);\
		return nvram_set_p(diag_nvram_path, name_cb, value);\
	}\
}

#define SAL_DIAG_UDPECHO_START   "start"
#define SAL_DIAG_UDPECHO_COMPLETE   "complete"
#define SAL_DIAG_UDPECHO_SUCCESS_COUNT   "success_count"
#define SAL_DIAG_UDPECHO_FAILURE_COUNT   "failure_count"
#define SAL_DIAG_UDPECHO_AVGRT   "average_response_time"
#define SAL_DIAG_UDPECHO_MINRT   "min_response_time"
#define SAL_DIAG_UDPECHO_MAXRT   "max_response_time"
NVRAN_GET_UDPECHO_DIAG_FUNC(sal_diag_get_udpecho_start,SAL_DIAG_UDPECHO_START)
NVRAN_GET_UDPECHO_DIAG_FUNC(sal_diag_get_udpecho_complete,SAL_DIAG_UDPECHO_COMPLETE)
NVRAN_GET_UDPECHO_DIAG_FUNC(sal_diag_get_udpecho_success_count,SAL_DIAG_UDPECHO_SUCCESS_COUNT)
NVRAN_GET_UDPECHO_DIAG_FUNC(sal_diag_get_udpecho_failure_count,SAL_DIAG_UDPECHO_FAILURE_COUNT)
NVRAN_GET_UDPECHO_DIAG_FUNC(sal_diag_get_udpecho_average_response_time,SAL_DIAG_UDPECHO_AVGRT)
NVRAN_GET_UDPECHO_DIAG_FUNC(sal_diag_get_udpecho_min_response_time,SAL_DIAG_UDPECHO_MINRT)
NVRAN_GET_UDPECHO_DIAG_FUNC(sal_diag_get_udpecho_max_response_time,SAL_DIAG_UDPECHO_MAXRT)

NVRAN_SET_UDPECHO_DIAG_FUNC(sal_diag_set_udpecho_start,SAL_DIAG_UDPECHO_START)
NVRAN_SET_UDPECHO_DIAG_FUNC(sal_diag_set_udpecho_complete,SAL_DIAG_UDPECHO_COMPLETE)
NVRAN_SET_UDPECHO_DIAG_FUNC(sal_diag_set_udpecho_success_count,SAL_DIAG_UDPECHO_SUCCESS_COUNT)
NVRAN_SET_UDPECHO_DIAG_FUNC(sal_diag_set_udpecho_failure_count,SAL_DIAG_UDPECHO_FAILURE_COUNT)
NVRAN_SET_UDPECHO_DIAG_FUNC(sal_diag_set_udpecho_average_response_time,SAL_DIAG_UDPECHO_AVGRT)
NVRAN_SET_UDPECHO_DIAG_FUNC(sal_diag_set_udpecho_min_response_time,SAL_DIAG_UDPECHO_MINRT)
NVRAN_SET_UDPECHO_DIAG_FUNC(sal_diag_set_udpecho_max_response_time,SAL_DIAG_UDPECHO_MAXRT)
*/
/*
#define SAL_DIAG_UDPECHO_PACKET_SUCCESS   "packet_success"
#define SAL_DIAG_UDPECHO_PACKET_SEND_TIME   "packet_send_time"
#define SAL_DIAG_UDPECHO_PACKET_RECEIVE_TIME   "packet_receive_time"
#define SAL_DIAG_UDPECHO_TEST_GENSN   "test_gensn"
#define SAL_DIAG_UDPECHO_TEST_RESPSN   "test_respsn"
#define SAL_DIAG_UDPECHO_TEST_RESPRCV_TIMESTAMP   "test_resprcv_timestamp"
#define SAL_DIAG_UDPECHO_TEST_RESPREPLY_TIMESTAMP   "test_respreply_timestamp"
#define SAL_DIAG_UDPECHO_TEST_RESPREPLY_FAILURECOUNT   "test_respreply_failurecount"
#define DIAG_CFG_BASE_PER_UDPECHO "/tmp/sal/udpecho_per_diag.sal"
#define NVRAN_GET_UDPECHO_DIAG_PER_FUNC(funcname,name)\
char *funcname(char *cb, int idx)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char per_cb_name[35];\
        snprintf(per_cb_name, sizeof(per_cb_name), "%s_%s_%d", name, cb, idx);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_PER_UDPECHO);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(per_cb_name, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_UDPECHO_DIAG_PER_FUNC(funcname, name)\
int funcname(char *value,char *cb, int idx)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char per_cb_name[35];\
		if(!value)\
			return -1;\
        snprintf(per_cb_name, sizeof(per_cb_name), "%s_%s_%d", name, cb, idx);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_PER_UDPECHO);\
		return nvram_set_p(diag_nvram_path, per_cb_name, value);\
	}\
}


NVRAN_GET_UDPECHO_DIAG_PER_FUNC(sal_diag_get_udpecho_per_packet_success,SAL_DIAG_UDPECHO_PACKET_SUCCESS)
NVRAN_GET_UDPECHO_DIAG_PER_FUNC(sal_diag_get_udpecho_per_packet_send_time,SAL_DIAG_UDPECHO_PACKET_SEND_TIME)
NVRAN_GET_UDPECHO_DIAG_PER_FUNC(sal_diag_get_udpecho_per_packet_receive_time,SAL_DIAG_UDPECHO_PACKET_RECEIVE_TIME)
NVRAN_GET_UDPECHO_DIAG_PER_FUNC(sal_diag_get_udpecho_per_test_gensn,SAL_DIAG_UDPECHO_TEST_GENSN)
NVRAN_GET_UDPECHO_DIAG_PER_FUNC(sal_diag_get_udpecho_per_test_respsn,SAL_DIAG_UDPECHO_TEST_RESPSN)
NVRAN_GET_UDPECHO_DIAG_PER_FUNC(sal_diag_get_udpecho_per_resprcv_timestamp,SAL_DIAG_UDPECHO_TEST_RESPRCV_TIMESTAMP)
NVRAN_GET_UDPECHO_DIAG_PER_FUNC(sal_diag_get_udpecho_per_respreply_timestamp,SAL_DIAG_UDPECHO_TEST_RESPREPLY_TIMESTAMP)
NVRAN_GET_UDPECHO_DIAG_PER_FUNC(sal_diag_get_udpecho_per_respreply_failurecount,SAL_DIAG_UDPECHO_TEST_RESPREPLY_FAILURECOUNT)

NVRAN_SET_UDPECHO_DIAG_PER_FUNC(sal_diag_set_udpecho_per_packet_success,SAL_DIAG_UDPECHO_PACKET_SUCCESS)
NVRAN_SET_UDPECHO_DIAG_PER_FUNC(sal_diag_set_udpecho_per_packet_send_time,SAL_DIAG_UDPECHO_PACKET_SEND_TIME)
NVRAN_SET_UDPECHO_DIAG_PER_FUNC(sal_diag_set_udpecho_per_packet_receive_time,SAL_DIAG_UDPECHO_PACKET_RECEIVE_TIME)
NVRAN_SET_UDPECHO_DIAG_PER_FUNC(sal_diag_set_udpecho_per_test_gensn,SAL_DIAG_UDPECHO_TEST_GENSN)
NVRAN_SET_UDPECHO_DIAG_PER_FUNC(sal_diag_set_udpecho_per_test_respsn,SAL_DIAG_UDPECHO_TEST_RESPSN)
NVRAN_SET_UDPECHO_DIAG_PER_FUNC(sal_diag_set_udpecho_per_resprcv_timestamp,SAL_DIAG_UDPECHO_TEST_RESPRCV_TIMESTAMP)
NVRAN_SET_UDPECHO_DIAG_PER_FUNC(sal_diag_set_udpecho_per_respreply_timestamp,SAL_DIAG_UDPECHO_TEST_RESPREPLY_TIMESTAMP)
NVRAN_SET_UDPECHO_DIAG_PER_FUNC(sal_diag_set_udpecho_per_respreply_failurecount,SAL_DIAG_UDPECHO_TEST_RESPREPLY_FAILURECOUNT)
*/
/*
#define DIAG_CFG_BASE_HW "/tmp/sal/hw_diag.sal"
#define NVRAN_GET_HW_DIAG_FUNC(funcname,name)\
char *funcname(int idx)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char per_name[35];\
        snprintf(per_name, sizeof(per_name), "%s_%d", name, idx);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_HW);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(per_name, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_HW_DIAG_FUNC(funcname, name)\
int funcname(char *value,int idx)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char per_name[35];\
		if(!value)\
			return -1;\
        snprintf(per_name, sizeof(per_name), "%s_%d", name, idx);\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_HW);\
		return nvram_set_p(diag_nvram_path, per_name, value);\
	}\
}

#define SAL_DIAG_HW_WIFI_STATE "wifi_state"
#define SAL_DIAG_HW_SLIC_STATE "slic_state"
#define SAL_DIAG_HW_USB_STATE  "usb_state"
#define SAL_DIAG_HW_USB_HOST_CTRL  "usb_hostctrl"
#define SAL_DIAG_HW_DSL_STATE "dsl_state"
#define SAL_DIAG_HW_SWITCH_STATE "switch_state"
#define SAL_DIAG_HW_SFP_STATE "sfp_state"
NVRAN_GET_HW_DIAG_FUNC(sal_diag_get_hw_wifi_state,SAL_DIAG_HW_WIFI_STATE)
NVRAN_GET_HW_DIAG_FUNC(sal_diag_get_hw_slic_state,SAL_DIAG_HW_SLIC_STATE)
NVRAN_GET_HW_DIAG_FUNC(sal_diag_get_hw_usb_state,SAL_DIAG_HW_USB_STATE)
NVRAN_GET_HW_DIAG_FUNC(sal_diag_get_hw_usb_hostctl,SAL_DIAG_HW_USB_HOST_CTRL)
NVRAN_GET_HW_DIAG_FUNC(sal_diag_get_hw_dsl_state,SAL_DIAG_HW_DSL_STATE)
NVRAN_GET_HW_DIAG_FUNC(sal_diag_get_hw_switch_state,SAL_DIAG_HW_SWITCH_STATE)
NVRAN_GET_HW_DIAG_FUNC(sal_diag_get_hw_sfp_state,SAL_DIAG_HW_SFP_STATE)

NVRAN_SET_HW_DIAG_FUNC(sal_diag_set_hw_wifi_state,SAL_DIAG_HW_WIFI_STATE)
NVRAN_SET_HW_DIAG_FUNC(sal_diag_set_hw_slic_state,SAL_DIAG_HW_SLIC_STATE)
NVRAN_SET_HW_DIAG_FUNC(sal_diag_set_hw_usb_state,SAL_DIAG_HW_USB_STATE)
NVRAN_SET_HW_DIAG_FUNC(sal_diag_set_hw_usb_hostctl,SAL_DIAG_HW_USB_HOST_CTRL)
NVRAN_SET_HW_DIAG_FUNC(sal_diag_set_hw_dsl_state,SAL_DIAG_HW_DSL_STATE)
NVRAN_SET_HW_DIAG_FUNC(sal_diag_set_hw_switch_state,SAL_DIAG_HW_SWITCH_STATE)
NVRAN_SET_HW_DIAG_FUNC(sal_diag_set_hw_sfp_state,SAL_DIAG_HW_SFP_STATE)
#define NVRAN_GET_HW_DIAG_FUNC2(funcname,name)\
char *funcname(void)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_HW);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(name, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_HW_DIAG_FUNC2(funcname, name)\
int funcname(char *value)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
		if(!value)\
			return -1;\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE_HW);\
		return nvram_set_p(diag_nvram_path, name, value);\
	}\
}
#define SAL_DIAG_HW_START "hw_diag_start"
#define SAL_DIAG_HW_COMPLETE "hw_diag_complete"

NVRAN_GET_HW_DIAG_FUNC2(sal_diag_get_hw_diag_start,SAL_DIAG_HW_START)
NVRAN_SET_HW_DIAG_FUNC2(sal_diag_set_hw_diag_start,SAL_DIAG_HW_START)
NVRAN_GET_HW_DIAG_FUNC2(sal_diag_get_hw_diag_complete,SAL_DIAG_HW_COMPLETE)
NVRAN_SET_HW_DIAG_FUNC2(sal_diag_set_hw_diag_complete,SAL_DIAG_HW_COMPLETE)
*/
