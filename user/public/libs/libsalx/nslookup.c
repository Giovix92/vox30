#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <nvram.h>

#define SAL_DIAG_TMP_VALUE_MAX_LENGTH	256
#define SAL_DIAG_TMP_PATH_MAX_LENGTH	256

#define DIAG_CFG_BASE "/tmp/sal/nslookup.sal"

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

#define NVRAN_GET_RESULT_FUNC(funcname, name)\
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

#define NVRAN_SET_RESULT_FUNC(funcname, name)\
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

#define SAL_NSLOOKUP_PID                   "NSLookup_Pid"
#define SAL_NSLOOKUP_STATUS                "NSLookup_Status"
#define SAL_NSLOOKUP_SUCCESS_COUNT         "NSLookup_SuccessTime" 
#define SAL_NSLOOKUP_RESULT_NUM            "Result_Num"
#define SAL_NSLOOKUP_RESULT_STATUS         "Result_%d_Status"
#define SAL_NSLOOKUP_RESULT_ANSWER_TYPE    "Result_%d_AnswerType"
#define SAL_NSLOOKUP_RESULT_HOST_NAME      "Result_%d_HostName"
#define SAL_NSLOOKUP_RESULT_IP             "Result_%d_IP"
#define SAL_NSLOOKUP_RESULT_DNS_SERVER     "Result_%d_DNSServer"
#define SAL_NSLOOKUP_RESULT_RESPONSE_TIME  "Result_%d_ResponseTime"

NVRAN_GET_DIAG_FUNC(sal_nslookup_get_pid, SAL_NSLOOKUP_PID)
NVRAN_SET_DIAG_FUNC(sal_nslookup_set_pid, SAL_NSLOOKUP_PID)
NVRAN_GET_DIAG_FUNC(sal_nslookup_get_status, SAL_NSLOOKUP_STATUS)
NVRAN_SET_DIAG_FUNC(sal_nslookup_set_status, SAL_NSLOOKUP_STATUS)
NVRAN_GET_DIAG_FUNC(sal_nslookup_get_success_count, SAL_NSLOOKUP_SUCCESS_COUNT)
NVRAN_SET_DIAG_FUNC(sal_nslookup_set_success_count, SAL_NSLOOKUP_SUCCESS_COUNT)
NVRAN_GET_DIAG_FUNC(sal_nslookup_get_result_num, SAL_NSLOOKUP_RESULT_NUM)
NVRAN_SET_DIAG_FUNC(sal_nslookup_set_result_num, SAL_NSLOOKUP_RESULT_NUM)
NVRAN_GET_RESULT_FUNC(sal_nslookup_get_result_status, SAL_NSLOOKUP_RESULT_STATUS)
NVRAN_SET_RESULT_FUNC(sal_nslookup_set_result_status, SAL_NSLOOKUP_RESULT_STATUS)
NVRAN_GET_RESULT_FUNC(sal_nslookup_get_result_answer_type, SAL_NSLOOKUP_RESULT_ANSWER_TYPE)
NVRAN_SET_RESULT_FUNC(sal_nslookup_set_result_answer_type, SAL_NSLOOKUP_RESULT_ANSWER_TYPE)
NVRAN_GET_RESULT_FUNC(sal_nslookup_get_result_host_name, SAL_NSLOOKUP_RESULT_HOST_NAME)
NVRAN_SET_RESULT_FUNC(sal_nslookup_set_result_host_name, SAL_NSLOOKUP_RESULT_HOST_NAME)
NVRAN_GET_RESULT_FUNC(sal_nslookup_get_result_ip, SAL_NSLOOKUP_RESULT_IP)
NVRAN_SET_RESULT_FUNC(sal_nslookup_set_result_ip, SAL_NSLOOKUP_RESULT_IP)
NVRAN_GET_RESULT_FUNC(sal_nslookup_get_result_dns_server, SAL_NSLOOKUP_RESULT_DNS_SERVER)
NVRAN_SET_RESULT_FUNC(sal_nslookup_set_result_dns_server, SAL_NSLOOKUP_RESULT_DNS_SERVER)
NVRAN_GET_RESULT_FUNC(sal_nslookup_get_result_response_time, SAL_NSLOOKUP_RESULT_RESPONSE_TIME)
NVRAN_SET_RESULT_FUNC(sal_nslookup_set_result_response_time, SAL_NSLOOKUP_RESULT_RESPONSE_TIME)

int sal_nslookup_check_running(void)
{
    char *pid = sal_nslookup_get_pid();
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
                    if(strstr(buf, "nslookup"))
                        ret = 1;
                fclose(fp);
            }
        }
    }
    return ret;
}
