#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <nvram.h>

#define SAL_DIAG_TMP_VALUE_MAX_LENGTH	256
#define SAL_DIAG_TMP_PATH_MAX_LENGTH	256

#define DIAG_CFG_BASE "/tmp/sal/traceroute.sal"

#define NVRAN_GET_DIAG_FUNC(funcname,name)\
char *funcname(void)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE);\
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
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE);\
		return nvram_set_p(diag_nvram_path, name, value);\
	}\
}

#define NVRAN_GET_HOP_FUNC(funcname, name)\
char *funcname(int id)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
		char buf[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE);\
    	snprintf(buf, sizeof(buf), name, id);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(buf, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_HOP_FUNC(funcname, name)\
int funcname(char *value, int id)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
		char buf[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
		if(!value)\
			return -1;\
    	snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE);\
    	snprintf(buf, sizeof(buf), name, id);\
		return nvram_set_p(diag_nvram_path, buf, value);\
	}\
}

#define SAL_TRACEROUTE_PID                   "TraceRoute_Pid"
#define SAL_TRACEROUTE_STATUS                "TraceRoute_Status"
#define SAL_TRACEROUTE_RESPONSE_TIME         "TraceRoute_ResponseTime"
#define SAL_ROUTEHOPS_HOST                   "Hop_%d_Host"
#define SAL_ROUTEHOPS_HOSTADDR               "Hop_%d_HostAddr"
#define SAL_ROUTEHOPS_ERRCODE                "Hop_%d_ErrCode"
#define SAL_ROUTEHOPS_RTTIME                 "Hop_%d_RTTime"

NVRAN_GET_DIAG_FUNC(sal_traceroute_get_status, SAL_TRACEROUTE_STATUS)
NVRAN_SET_DIAG_FUNC(sal_traceroute_set_status, SAL_TRACEROUTE_STATUS)
NVRAN_GET_DIAG_FUNC(sal_traceroute_get_pid, SAL_TRACEROUTE_PID)
NVRAN_SET_DIAG_FUNC(sal_traceroute_set_pid, SAL_TRACEROUTE_PID)
NVRAN_GET_DIAG_FUNC(sal_traceroute_get_resp_time, SAL_TRACEROUTE_RESPONSE_TIME)
NVRAN_SET_DIAG_FUNC(sal_traceroute_set_resp_time, SAL_TRACEROUTE_RESPONSE_TIME)
NVRAN_GET_HOP_FUNC(sal_traceroute_get_hop_host, SAL_ROUTEHOPS_HOST)
NVRAN_SET_HOP_FUNC(sal_traceroute_set_hop_host, SAL_ROUTEHOPS_HOST)
NVRAN_GET_HOP_FUNC(sal_traceroute_get_hop_host_addr,SAL_ROUTEHOPS_HOSTADDR)
NVRAN_SET_HOP_FUNC(sal_traceroute_set_hop_host_addr,SAL_ROUTEHOPS_HOSTADDR)
NVRAN_GET_HOP_FUNC(sal_traceroute_get_hop_err_code,SAL_ROUTEHOPS_ERRCODE)
NVRAN_SET_HOP_FUNC(sal_traceroute_set_hop_err_code,SAL_ROUTEHOPS_ERRCODE)
NVRAN_GET_HOP_FUNC(sal_traceroute_get_hop_rt_time,SAL_ROUTEHOPS_RTTIME)
NVRAN_SET_HOP_FUNC(sal_traceroute_set_hop_rt_time,SAL_ROUTEHOPS_RTTIME)

#define SAL_TRACEROUTE_RESULT                "TraceRoute_Result"
#define SAL_TRACEROUTE_BUSY                  "TraceRoute_Busy"
#define SAL_TRACEROUTE_GUI_PID               "TraceRoute_GUI_Pid"

NVRAN_GET_DIAG_FUNC(sal_traceroute_get_result, SAL_TRACEROUTE_RESULT)
NVRAN_SET_DIAG_FUNC(sal_traceroute_set_result, SAL_TRACEROUTE_RESULT)
NVRAN_GET_DIAG_FUNC(sal_traceroute_get_busy, SAL_TRACEROUTE_BUSY)
NVRAN_SET_DIAG_FUNC(sal_traceroute_set_busy, SAL_TRACEROUTE_BUSY)
NVRAN_GET_DIAG_FUNC(sal_traceroute_get_gui_pid, SAL_TRACEROUTE_GUI_PID)
NVRAN_SET_DIAG_FUNC(sal_traceroute_set_gui_pid, SAL_TRACEROUTE_GUI_PID)

int sal_traceroute_check_running(void)
{
    char *pid = sal_traceroute_get_pid();
    int ret = 0;
    if(pid)
    {
        if(atoi(pid))
        {
            char command[100];
            char buf[100] = "";
            snprintf(command, sizeof(command), "/proc/%s/cmdline", pid);
            FILE *fp = fopen(command, "r");
            if(fp)
            {
                if(fgets(buf,sizeof(buf),fp))
                    if(strstr(buf, "traceroute"))
                        ret = 1;
                fclose(fp);
            }
        }
    }
    return ret;
}
