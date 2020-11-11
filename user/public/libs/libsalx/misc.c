/**
 * @file   misc.c
 * @author Martin Huang martin_huang@sdc.sercomm.com
 * @date   2011-08-11
 * @brief  support multiple wan configuration abstract layer API.
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifisally
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <nvram.h>
#include <utility.h>
#include <sal/sal_misc.h>

#define SAL_MISC_TMP_VALUE_MAX_LENGTH	256
#define SAL_MISC_TMP_PATH_MAX_LENGTH	256

#define MISC_CFG_BASE "/tmp/sal/misc.sal"

#define NVRAN_GET_MISC_FUNC(funcname, name, buffer)\
char *funcname(void)\
{\
	{\
		static char buffer[SAL_MISC_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];\
    	snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);\
    	buffer[0] = '\0';\
		p = nvram_get_fun(name, misc_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_MISC_FUNC(funcname, name)\
int funcname(char *value)\
{\
	{\
		char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];\
		if(!value)\
			return -1;\
    	snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);\
		return nvram_set_p(misc_nvram_path, name, value);\
	}\
}

#define NVRAN_GET_MISC_FUNC1(funcname, name, buffer)\
char *funcname(char* remote_ip)\
{\
	{\
		static char buffer[SAL_MISC_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];\
        char name_buf[SAL_MISC_TMP_PATH_MAX_LENGTH] = {0}; \
    	snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);\
    	buffer[0] = '\0';\
        snprintf(name_buf, sizeof(name_buf), "%s_%s",name,remote_ip);\
		p = nvram_get_fun(name_buf, misc_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_MISC_FUNC1(funcname, name)\
int funcname(char* remote_ip, char *value)\
{\
	{\
		char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];\
        char name_buf[SAL_MISC_TMP_PATH_MAX_LENGTH] = {0}; \
		if(!value)\
			return -1;\
    	snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);\
        snprintf(name_buf, sizeof(name_buf), "%s_%s",name,remote_ip);\
		return nvram_set_p(misc_nvram_path, name_buf, value);\
	}\
}
#define SAL_MISC_BOARD_CPU_INFO          "Board_CPU_info"
#define SAL_MISC_BOARD_CPU               "Board_CPU"
#define SAL_MISC_BOARD_MANUFACTURE       "Board_Manufacture"
#define SAL_MISC_BOARD_WIFI_PSK          "Board_WIFI_PSK"
#define SAL_MISC_BOARD_WIFI_PIN          "Board_WIFI_PIN"
#define SAL_MISC_BOARD_WIFI_SSID         "Board_WIFI_SSID"
#define SAL_MISC_BOARD_MODEL_DES         "Board_Model_DES"
#define SAL_MISC_BOARD_MODEL_NAME        "Board_Model_Name"
#define SAL_MISC_BOARD_MODEL_URL         "Board_Model_URL"
#define SAL_MISC_BOARD_MODEL_NUMBER      "Board_Model_number"
#define SAL_MISC_BOARD_MANUFACTURE_URL   "Board_Manufacture_URL"
#define SAL_MISC_BOARD_PID               "Board_Manufacture_PID"
#define SAL_MISC_BOARD_HW_ID             "Board_HW_ID"
#define SAL_MISC_BOARD_CSN               "Board_CSN"
#define SAL_MISC_BOARD_PSN               "Board_PSN"
#define SAL_MISC_BOARD_HW_VERSION        "Board_HW_VERSION"
#define SAL_MISC_BOARD_HW_MAC            "Board_HW_MAC"
#define SAL_MISC_BOARD_FW_VERSION        "Board_FW_version"
#define SAL_MISC_BOARD_LIB_VERSION       "Board_LIB_version"
#define SAL_MISC_BOARD_BOOT_VERSION      "Board_BOOT_version"
#define SAL_MISC_BOARD_SOFT_VENDOR       "Board_soft_vendor"
#define SAL_MISC_BOARD_PRODUCT_CLASS     "Board_product_class"
#define SAL_MISC_BOARD_PRODUCT_OUI       "Board_OUI"
#define SAL_MISC_BOARD_FRIEND_NAME       "Board_friend_name"
#define SAL_MISC_BOARD_BUILD_TIME        "Board_build_time"
#define SAL_MISC_BOARD_BUILD_TAG         "Board_build_tag"
#define SAL_MISC_WPS_CLIENT_PIN          "WPS_client_pin"
#define SAL_MISC_WPS_STATUS              "WPS_status"
#ifdef CONFIG_SUPPORT_ZIGBEE
#define SAL_MISC_ZIGBEE_STATUS           "Zigbee_status"
#endif
#define SAL_MISC_BOARD_MODEM_FW_VERSION  "Board_modem_fw_version"
#ifdef CONFIG_SUPPORT_WEBAPI
#define SAL_MISC_SPEED_TEST_UPLOAD_SPEED "Speed_test_upload_speed"
#define SAL_MISC_SPEED_TEST_DOWNLOAD_SPEED "Speed_test_download_speed"
#define SAL_MISC_SPEED_TEST_STATUS       "Speed_test_status"
#endif
#define SAL_MISC_BOARD_START_UP     "Board_start_up"
NVRAN_GET_MISC_FUNC(sal_misc_get_board_start_up, SAL_MISC_BOARD_START_UP, board_start_up)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_start_up, SAL_MISC_BOARD_START_UP)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_cpu_info, SAL_MISC_BOARD_CPU_INFO, board_cpu_info)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_cpu_info, SAL_MISC_BOARD_CPU_INFO)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_cpu, SAL_MISC_BOARD_CPU, board_cpu)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_cpu, SAL_MISC_BOARD_CPU)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_manufacture, SAL_MISC_BOARD_MANUFACTURE, board_manufacture)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_manufacture, SAL_MISC_BOARD_MANUFACTURE )
NVRAN_GET_MISC_FUNC(sal_misc_get_board_wifi_psk, SAL_MISC_BOARD_WIFI_PSK, board_wifi_psk)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_wifi_psk, SAL_MISC_BOARD_WIFI_PSK)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_pin, SAL_MISC_BOARD_WIFI_PIN, board_wifi_pin)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_pin, SAL_MISC_BOARD_WIFI_PIN)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_model_des, SAL_MISC_BOARD_MODEL_DES , board_menu_des)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_model_des, SAL_MISC_BOARD_MODEL_DES )
NVRAN_GET_MISC_FUNC(sal_misc_get_board_manufacture_url, SAL_MISC_BOARD_MANUFACTURE_URL, board_url)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_manufacture_url, SAL_MISC_BOARD_MANUFACTURE_URL)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_pid, SAL_MISC_BOARD_PID, board_pid)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_pid, SAL_MISC_BOARD_PID)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_hw_id, SAL_MISC_BOARD_HW_ID, board_pid)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_hw_id, SAL_MISC_BOARD_HW_ID)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_csn, SAL_MISC_BOARD_CSN, board_csn)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_csn, SAL_MISC_BOARD_CSN )
NVRAN_GET_MISC_FUNC(sal_misc_get_board_pcba_sn, SAL_MISC_BOARD_PSN, board_csn)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_pcba_sn, SAL_MISC_BOARD_PSN )
NVRAN_GET_MISC_FUNC(sal_misc_get_board_hw_version, SAL_MISC_BOARD_HW_VERSION, board_hw_version)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_hw_version, SAL_MISC_BOARD_HW_VERSION)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_fw_version, SAL_MISC_BOARD_FW_VERSION, board_fw_version)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_fw_version, SAL_MISC_BOARD_FW_VERSION)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_lib_version, SAL_MISC_BOARD_LIB_VERSION, board_lib_version)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_lib_version, SAL_MISC_BOARD_LIB_VERSION)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_model_name, SAL_MISC_BOARD_MODEL_NAME , board_model_name)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_model_name, SAL_MISC_BOARD_MODEL_NAME )
NVRAN_GET_MISC_FUNC(sal_misc_get_board_boot_version, SAL_MISC_BOARD_BOOT_VERSION, board_fw_version)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_boot_version, SAL_MISC_BOARD_BOOT_VERSION)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_wifi_ssid, SAL_MISC_BOARD_WIFI_SSID, board_wifi_psk)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_wifi_ssid, SAL_MISC_BOARD_WIFI_SSID)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_model_url, SAL_MISC_BOARD_MODEL_URL , board_model_name)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_model_url, SAL_MISC_BOARD_MODEL_URL )
NVRAN_GET_MISC_FUNC(sal_misc_get_board_hw_mac, SAL_MISC_BOARD_HW_MAC, board_pid)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_hw_mac, SAL_MISC_BOARD_HW_MAC)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_soft_vendor, SAL_MISC_BOARD_SOFT_VENDOR, board_pid)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_soft_vendor, SAL_MISC_BOARD_SOFT_VENDOR)   
NVRAN_GET_MISC_FUNC(sal_misc_get_board_product_class, SAL_MISC_BOARD_PRODUCT_CLASS, board_pid)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_product_class, SAL_MISC_BOARD_PRODUCT_CLASS)   
NVRAN_GET_MISC_FUNC(sal_misc_get_board_manufacture_oui, SAL_MISC_BOARD_PRODUCT_OUI, board_pid)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_manufacture_oui, SAL_MISC_BOARD_PRODUCT_OUI)   
NVRAN_GET_MISC_FUNC(sal_misc_get_board_model_number, SAL_MISC_BOARD_MODEL_NUMBER , board_model_name)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_model_number, SAL_MISC_BOARD_MODEL_NUMBER)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_friend_name, SAL_MISC_BOARD_FRIEND_NAME , board_model_name)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_friend_name, SAL_MISC_BOARD_FRIEND_NAME)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_build_time, SAL_MISC_BOARD_BUILD_TIME, board_model_name)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_build_time, SAL_MISC_BOARD_BUILD_TIME)
NVRAN_GET_MISC_FUNC(sal_misc_get_board_build_tag, SAL_MISC_BOARD_BUILD_TAG, board_model_name)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_build_tag, SAL_MISC_BOARD_BUILD_TAG)
NVRAN_GET_MISC_FUNC(sal_misc_get_wps_client_pin, SAL_MISC_WPS_CLIENT_PIN, wps_client_pin)
NVRAN_SET_MISC_FUNC(sal_misc_set_wps_client_pin, SAL_MISC_WPS_CLIENT_PIN)
NVRAN_GET_MISC_FUNC(sal_misc_get_wps_status, SAL_MISC_WPS_STATUS, wps_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_wps_status, SAL_MISC_WPS_STATUS)
#ifdef CONFIG_SUPPORT_ZIGBEE
NVRAN_GET_MISC_FUNC(sal_misc_get_zigbee_status, SAL_MISC_ZIGBEE_STATUS, zigbee_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_zigbee_status, SAL_MISC_ZIGBEE_STATUS)
#endif
#ifdef CONFIG_SUPPORT_WEBAPI
NVRAN_GET_MISC_FUNC(sal_misc_get_speed_test_upload_speed, SAL_MISC_SPEED_TEST_UPLOAD_SPEED,            speed_test_upload_speed)
NVRAN_SET_MISC_FUNC(sal_misc_set_speed_test_upload_speed, SAL_MISC_SPEED_TEST_UPLOAD_SPEED)
NVRAN_GET_MISC_FUNC(sal_misc_get_speed_test_download_speed, SAL_MISC_SPEED_TEST_DOWNLOAD_SPEED,        speed_test_download_speed)
NVRAN_SET_MISC_FUNC(sal_misc_set_speed_test_download_speed, SAL_MISC_SPEED_TEST_DOWNLOAD_SPEED)
NVRAN_GET_MISC_FUNC(sal_misc_get_speed_test_status, SAL_MISC_SPEED_TEST_STATUS, speed_test_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_speed_test_status, SAL_MISC_SPEED_TEST_STATUS)
#endif
NVRAN_GET_MISC_FUNC(sal_misc_get_board_modem_fw_version, SAL_MISC_BOARD_MODEM_FW_VERSION, modem_fw_version)
NVRAN_SET_MISC_FUNC(sal_misc_set_board_modem_fw_version, SAL_MISC_BOARD_MODEM_FW_VERSION)

#define SAL_MISC_IPPING_SUCCESS_COUNT   "IPPing_SuccessCount"
#define SAL_MISC_IPPING_FAILURE_COUNT   "IPPing_FailureCount"        
#define SAL_MISC_IPPING_AVG_RESP_TIME   "IPPing_AverageResponseTime"
#define SAL_MISC_IPPING_MIN_RESP_TIME   "IPPing_MinimumResponseTime"
#define SAL_MISC_IPPING_MAX_RESP_TIME   "IPPing_MaximumResponseTime"

#ifdef CONFIG_SUPPORT_DSL
#define SAL_MISC_DSL_DIAG_PATH       "DSLDiagnostics_path"
#define SAL_MISC_DSL_DIAG_ACTPSDds       "DSLDiagnostics_ACTPSDds"
#define SAL_MISC_DSL_DIAG_ACTPSDus       "DSLDiagnostics_ACTPSDus"
#define SAL_MISC_DSL_DIAG_ACTATPds       "DSLDiagnostics_ACTATPds"
#define SAL_MISC_DSL_DIAG_ACTATPus       "DSLDiagnostics_ACTATPus"
#define SAL_MISC_DSL_DIAG_HLINSCds      "DSLDiagnostics_HLINSCds"
#define SAL_MISC_DSL_DIAG_HLINGds       "DSLDiagnostics_HLINGds"
#define SAL_MISC_DSL_DIAG_HLINGus       "DSLDiagnostics_HLINGus"
#define SAL_MISC_DSL_DIAG_HLOGGds       "DSLDiagnostics_HLOGGds"
#define SAL_MISC_DSL_DIAG_HLOGGus       "DSLDiagnostics_HLOGGus"
#define SAL_MISC_DSL_DIAG_QLNGds       "DSLDiagnostics_QLNGds"
#define SAL_MISC_DSL_DIAG_QLNGus       "DSLDiagnostics_QLNGus"
#define SAL_MISC_DSL_DIAG_SNRGds       "DSLDiagnostics_SNRGds"
#define SAL_MISC_DSL_DIAG_SNRGus       "DSLDiagnostics_SNRGus"
#define SAL_MISC_DSL_DIAG_HLOGpsds       "DSLDiagnostics_HLOGpsds"
#define SAL_MISC_DSL_DIAG_HLOGpsus       "DSLDiagnostics_HLOGpsus"
#define SAL_MISC_DSL_DIAG_LATNpbds       "DSLDiagnostics_LATNpbds"
#define SAL_MISC_DSL_DIAG_LATNpbus       "DSLDiagnostics_LATNpbus"
#define SAL_MISC_DSL_DIAG_SATNds       "DSLDiagnostics_SATNds"
#define SAL_MISC_DSL_DIAG_SATNus       "DSLDiagnostics_SATNus"
#define SAL_MISC_DSL_DIAG_HLOGMTds      "DSLDiagnostics_HLOGMTds"
#define SAL_MISC_DSL_DIAG_HLOGMTus      "DSLDiagnostics_HLOGMTus"
#define SAL_MISC_DSL_DIAG_HLINpsds      "DSLDiagnostics_HLINpsds"
#define SAL_MISC_DSL_DIAG_HLINpsus      "DSLDiagnostics_HLINpsus"
#define SAL_MISC_DSL_DIAG_QLNMTds       "DSLDiagnostics_QLNMTds"
#define SAL_MISC_DSL_DIAG_QLNMTus       "DSLDiagnostics_QLNMTus"
#define SAL_MISC_DSL_DIAG_SNRMTds       "DSLDiagnostics_SNRMTds"
#define SAL_MISC_DSL_DIAG_SNRMTus       "DSLDiagnostics_SNRMTus"
#define SAL_MISC_DSL_DIAG_QLNpsds       "DSLDiagnostics_QLNpsds"
#define SAL_MISC_DSL_DIAG_QLNpsus       "DSLDiagnostics_QLNpsus"
#define SAL_MISC_DSL_DIAG_SNRpsds       "DSLDiagnostics_SNRpsds"
#define SAL_MISC_DSL_DIAG_SNRpsus       "DSLDiagnostics_SNRpsus"
#define SAL_MISC_DSL_DIAG_BITSpsds      "DSLDiagnostics_BITSpsds"
#define SAL_MISC_DSL_DIAG_GAINSpsds      "DSLDiagnostics_GAINSpsds"

#define SAL_MISC_ATMF5_DIAG_PATH            "ATMF5Diagnostics_path"
#define SAL_MISC_ATMF5_DIAG_SUC_COUNT       "ATMF5Diagnostics_succ_count"
#define SAL_MISC_ATMF5_DIAG_FAL_COUNT       "ATMF5Diagnostics_fail_count"
#define SAL_MISC_ATMF5_DIAG_AVG_RESP_TIME   "ATMF5Diagnostics_avg_resp_time"
#define SAL_MISC_ATMF5_DIAG_MIN_RESP_TIME   "ATMF5Diagnostics_min_resp_time"
#define SAL_MISC_ATMF5_DIAG_MAX_RESP_TIME   "ATMF5Diagnostics_max_resp_time"
#endif

#define SAL_MISC_DDNS_STATUS            "DDNS_Status"
#define SAL_MISC_UPGRADE_STATUS         "Upgrade_Status"
#define SAL_MISC_SUPERUSER_LANGUAGE     "SuperUser_Language"
#define SAL_MISC_GPON_VIF_STATUS        "GPON_VIF_Status"
#define SAL_MISC_PING_STATUS            "Ping_Status"
#define SAL_MISC_PING_RESULT_LINE       "Ping_ResultLine"
#define SAL_MISC_PING_PID               "Ping_Pid"
#define SAL_MISC_AUTOSENSE_START_TIME  "Autosense_start_time"
#define SAL_MISC_DSL_POSTTIME_ST_NAME  "dsl_posttime"
#define SAL_MISC_ETHER_POSTTIME_ST_NAME  "ether_posttime"
#define SAL_MISC_REBOOT_CAUSE           "reboot_cause"
#ifdef VOX30_SFP
#define SAL_MISC_SFP_UPGRADE_STATUS     "SFP_Upg_Status"
#endif
#ifdef CONFIG_SUPPORT_MEDIA_SERVER
#define SAL_MISC_MEDIA_STATUS           "Media_Status"
#endif
#ifdef CONFIG_SUPPORT_BBU
#define SAL_MISC_BBU_STATUS            "BBU_Status"
#endif
#define SAL_MISC_RESET_BUTTON_STATUS   "ResetButton_Status"
#define SAL_MISC_RESET_STATUS   "Reset_Status"
#define SAL_MISC_WPS_BUTTON_STATUS     "WPSButton_Status"
#define SAL_MISC_WIFI_BUTTON_STATUS    "WiFiButton_Status"
#define SAL_MISC_CONFIG_COMPUTER_RESTOR      "Config_Computer_Restore"
#define SAL_MISC_CONFIG_COMPUTER_RESTOR_TYPE      "Config_Computer_Restore_Group"
#define SAL_MISC_TR_BOOT_EVENT          "TR_Boot_Event"

#define SAL_LOGIN_SID_NAME          "session_id"
#define SAL_LOGIN_LAST_SID1_NAME     "last_session_id1"
#define SAL_LOGIN_LAST_SID2_NAME     "last_session_id2"
#define SAL_LOGIN_LAST_SID3_NAME     "last_session_id3"
#define SAL_LOGIN_LAST_SID4_NAME     "last_session_id4"
#define SAL_LOGIN_LAST_SID5_NAME     "last_session_id5"
#define SAL_LOGIN_LAST_SID1_UPTIME     "last_session_id1_uptime"
#define SAL_LOGIN_LAST_SID2_UPTIME     "last_session_id2_uptime"
#define SAL_LOGIN_LAST_SID3_UPTIME     "last_session_id3_uptime"
#define SAL_LOGIN_LAST_SID4_UPTIME     "last_session_id4_uptime"
#define SAL_LOGIN_LAST_SID5_UPTIME     "last_session_id5_uptime"
#define SAL_LOGIN_CSRF_TOKEN        "csrf_token"
#ifdef CONFIG_SUPPORT_PASSWORD_PROTECTION
#define SAL_LOGIN_RSTPWD_CSRF_TOKEN        "rstpwd_csrf_token"
#define SAL_LOGIN_RSTPWD_SESSION_ID      "rstpwd_id"
#endif
#define SAL_LOGIN_LAST_CSRF_TOKEN   "last_csrf_token"
#define SAL_LOGIN_CSRF_TOKEN_HIDDEN "csrf_token_hidden"
#define SAL_LOGIN_FAILED_TIMES      "login_failed"
#define SAL_LOGIN_FAILED_TIMES_SETTING      "login_failed_setting"
#define SAL_LOGIN_KEY               "login_key"
#ifdef CONFIG_SUPPORT_SJCL_ENCRYPT
#define SAL_LOGIN_SALT              "login_salt"
#define SAL_LOGIN_DK                "login_dk"
#endif
#define SAL_LOGIN_SSH_FAILED_TIMES  "ssh_login_failed"

#define SAL_MISC_TR_ACS_STATUS          "TR_ACS_Status"
#ifdef SUPPORT_USB_STORAGE
#define SAL_MISC_TWONKY_SERVER_DB_DIR  "TwonkyServer_Db_Dir"
NVRAN_GET_MISC_FUNC(sal_misc_get_twonky_server_db_dir,SAL_MISC_TWONKY_SERVER_DB_DIR,twonky_dir)
NVRAN_SET_MISC_FUNC(sal_misc_set_twonky_server_db_dir,SAL_MISC_TWONKY_SERVER_DB_DIR)
#endif
NVRAN_GET_MISC_FUNC(sal_misc_get_ipping_diag_succ_count, SAL_MISC_IPPING_SUCCESS_COUNT, ipping_succ_count)
NVRAN_SET_MISC_FUNC(sal_misc_set_ipping_diag_succ_count, SAL_MISC_IPPING_SUCCESS_COUNT)
NVRAN_GET_MISC_FUNC(sal_misc_get_ipping_diag_fail_count, SAL_MISC_IPPING_FAILURE_COUNT, ipping_fail_count)
NVRAN_SET_MISC_FUNC(sal_misc_set_ipping_diag_fail_count, SAL_MISC_IPPING_FAILURE_COUNT)
NVRAN_GET_MISC_FUNC(sal_misc_get_ipping_diag_avg_resp_time, SAL_MISC_IPPING_AVG_RESP_TIME, ipping_avg_resp_time)
NVRAN_SET_MISC_FUNC(sal_misc_set_ipping_diag_avg_resp_time, SAL_MISC_IPPING_AVG_RESP_TIME)
NVRAN_GET_MISC_FUNC(sal_misc_get_ipping_diag_min_resp_time, SAL_MISC_IPPING_MIN_RESP_TIME, ipping_min_resp_time)
NVRAN_SET_MISC_FUNC(sal_misc_set_ipping_diag_min_resp_time, SAL_MISC_IPPING_MIN_RESP_TIME)
NVRAN_GET_MISC_FUNC(sal_misc_get_ipping_diag_max_resp_time, SAL_MISC_IPPING_MAX_RESP_TIME, ipping_max_resp_time)
NVRAN_SET_MISC_FUNC(sal_misc_set_ipping_diag_max_resp_time, SAL_MISC_IPPING_MAX_RESP_TIME)

#ifdef CONFIG_SUPPORT_DSL
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_path, SAL_MISC_DSL_DIAG_PATH, dsl_diag_path)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_path, SAL_MISC_DSL_DIAG_PATH)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_ACTPSDds, SAL_MISC_DSL_DIAG_ACTPSDds, dsl_diag_ACTPSDds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_ACTPSDds, SAL_MISC_DSL_DIAG_ACTPSDds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_ACTPSDus, SAL_MISC_DSL_DIAG_ACTPSDus, dsl_diag_ACTPSDus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_ACTPSDus, SAL_MISC_DSL_DIAG_ACTPSDus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_ACTATPds, SAL_MISC_DSL_DIAG_ACTATPds, dsl_diag_ACTATPds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_ACTATPds, SAL_MISC_DSL_DIAG_ACTATPds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_ACTATPus, SAL_MISC_DSL_DIAG_ACTATPus, dsl_diag_ACTATPus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_ACTATPus, SAL_MISC_DSL_DIAG_ACTATPus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLINSCds, SAL_MISC_DSL_DIAG_HLINSCds, dsl_diag_HLINSCds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLINSCds, SAL_MISC_DSL_DIAG_HLINSCds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLINGds, SAL_MISC_DSL_DIAG_HLINGds, dsl_diag_HLINGds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLINGds, SAL_MISC_DSL_DIAG_HLINGds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLINGus, SAL_MISC_DSL_DIAG_HLINGus, dsl_diag_HLINGus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLINGus, SAL_MISC_DSL_DIAG_HLINGus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLOGGds, SAL_MISC_DSL_DIAG_HLOGGds, dsl_diag_HLOGGds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLOGGds, SAL_MISC_DSL_DIAG_HLOGGds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLOGGus, SAL_MISC_DSL_DIAG_HLOGGus, dsl_diag_HLOGGus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLOGGus, SAL_MISC_DSL_DIAG_HLOGGus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_QLNGds, SAL_MISC_DSL_DIAG_QLNGds, dsl_diag_QLNGds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_QLNGds, SAL_MISC_DSL_DIAG_QLNGds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_QLNGus, SAL_MISC_DSL_DIAG_QLNGus, dsl_diag_QLNGus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_QLNGus, SAL_MISC_DSL_DIAG_QLNGus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_SNRGds, SAL_MISC_DSL_DIAG_SNRGds, dsl_diag_SNRGds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_SNRGds, SAL_MISC_DSL_DIAG_SNRGds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_SNRGus, SAL_MISC_DSL_DIAG_SNRGus, dsl_diag_SNRGus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_SNRGus, SAL_MISC_DSL_DIAG_SNRGus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLOGpsds, SAL_MISC_DSL_DIAG_HLOGpsds, dsl_diag_HLOGpsds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLOGpsds, SAL_MISC_DSL_DIAG_HLOGpsds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLOGpsus, SAL_MISC_DSL_DIAG_HLOGpsus, dsl_diag_HLOGpsus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLOGpsus, SAL_MISC_DSL_DIAG_HLOGpsus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_LATNpbds, SAL_MISC_DSL_DIAG_LATNpbds, dsl_diag_LATNpbds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_LATNpbds, SAL_MISC_DSL_DIAG_LATNpbds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_LATNpbus, SAL_MISC_DSL_DIAG_LATNpbus, dsl_diag_LATNpbus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_LATNpbus, SAL_MISC_DSL_DIAG_LATNpbus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_SATNds, SAL_MISC_DSL_DIAG_SATNds, dsl_diag_SATNds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_SATNds, SAL_MISC_DSL_DIAG_SATNds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_SATNus, SAL_MISC_DSL_DIAG_SATNus, dsl_diag_SATNus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_SATNus, SAL_MISC_DSL_DIAG_SATNus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLOGMTds, SAL_MISC_DSL_DIAG_HLOGMTds, dsl_diag_HLOGMTds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLOGMTds, SAL_MISC_DSL_DIAG_HLOGMTds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLOGMTus, SAL_MISC_DSL_DIAG_HLOGMTus, dsl_diag_HLOGMTus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLOGMTus, SAL_MISC_DSL_DIAG_HLOGMTus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLINpsds, SAL_MISC_DSL_DIAG_HLINpsds, dsl_diag_HLINpsds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLINpsds, SAL_MISC_DSL_DIAG_HLINpsds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_HLINpsus, SAL_MISC_DSL_DIAG_HLINpsus, dsl_diag_HLINpsus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_HLINpsus, SAL_MISC_DSL_DIAG_HLINpsus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_QLNMTds, SAL_MISC_DSL_DIAG_QLNMTds, dsl_diag_QLNMTds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_QLNMTds, SAL_MISC_DSL_DIAG_QLNMTds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_QLNMTus, SAL_MISC_DSL_DIAG_QLNMTus, dsl_diag_QLNMTus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_QLNMTus, SAL_MISC_DSL_DIAG_QLNMTus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_SNRMTds, SAL_MISC_DSL_DIAG_SNRMTds, dsl_diag_SNRMTds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_SNRMTds, SAL_MISC_DSL_DIAG_SNRMTds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_SNRMTus, SAL_MISC_DSL_DIAG_SNRMTus, dsl_diag_SNRMTus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_SNRMTus, SAL_MISC_DSL_DIAG_SNRMTus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_QLNpsds, SAL_MISC_DSL_DIAG_QLNpsds, dsl_diag_QLNpsds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_QLNpsds, SAL_MISC_DSL_DIAG_QLNpsds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_QLNpsus, SAL_MISC_DSL_DIAG_QLNpsus, dsl_diag_QLNpsus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_QLNpsus, SAL_MISC_DSL_DIAG_QLNpsus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_SNRpsds, SAL_MISC_DSL_DIAG_SNRpsds, dsl_diag_SNRpsds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_SNRpsds, SAL_MISC_DSL_DIAG_SNRpsds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_SNRpsus, SAL_MISC_DSL_DIAG_SNRpsus, dsl_diag_SNRpsus)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_SNRpsus, SAL_MISC_DSL_DIAG_SNRpsus)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_BITSpsds, SAL_MISC_DSL_DIAG_BITSpsds, dsl_diag_BITSpsds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_BITSpsds, SAL_MISC_DSL_DIAG_BITSpsds)
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_diag_GAINSpsds, SAL_MISC_DSL_DIAG_GAINSpsds, dsl_diag_GAINSpsds)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_diag_GAINSpsds, SAL_MISC_DSL_DIAG_GAINSpsds)

NVRAN_GET_MISC_FUNC(sal_misc_get_atmf5_diag_path, SAL_MISC_ATMF5_DIAG_PATH, atmf5_diag_path)
NVRAN_SET_MISC_FUNC(sal_misc_set_atmf5_diag_path, SAL_MISC_ATMF5_DIAG_PATH)
NVRAN_GET_MISC_FUNC(sal_misc_get_atmf5_diag_succ_count, SAL_MISC_ATMF5_DIAG_SUC_COUNT, atmf5_diag_succ_count)
NVRAN_SET_MISC_FUNC(sal_misc_set_atmf5_diag_succ_count, SAL_MISC_ATMF5_DIAG_SUC_COUNT)
NVRAN_GET_MISC_FUNC(sal_misc_get_atmf5_diag_fail_count, SAL_MISC_ATMF5_DIAG_FAL_COUNT, atmf5_diag_fail_count)
NVRAN_SET_MISC_FUNC(sal_misc_set_atmf5_diag_fail_count, SAL_MISC_ATMF5_DIAG_FAL_COUNT)
NVRAN_GET_MISC_FUNC(sal_misc_get_atmf5_diag_avg_resp_time, SAL_MISC_ATMF5_DIAG_AVG_RESP_TIME, atmf5_diag_avg_resp_time)
NVRAN_SET_MISC_FUNC(sal_misc_set_atmf5_diag_avg_resp_time, SAL_MISC_ATMF5_DIAG_AVG_RESP_TIME)
NVRAN_GET_MISC_FUNC(sal_misc_get_atmf5_diag_min_resp_time, SAL_MISC_ATMF5_DIAG_MIN_RESP_TIME, atmf5_diag_min_resp_time)
NVRAN_SET_MISC_FUNC(sal_misc_set_atmf5_diag_min_resp_time, SAL_MISC_ATMF5_DIAG_MIN_RESP_TIME)
NVRAN_GET_MISC_FUNC(sal_misc_get_atmf5_diag_max_resp_time, SAL_MISC_ATMF5_DIAG_MAX_RESP_TIME, atmf5_diag_max_resp_time)
NVRAN_SET_MISC_FUNC(sal_misc_set_atmf5_diag_max_resp_time, SAL_MISC_ATMF5_DIAG_MAX_RESP_TIME)
#endif

NVRAN_GET_MISC_FUNC(sal_misc_get_ddns_status, SAL_MISC_DDNS_STATUS, ddns_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_ddns_status, SAL_MISC_DDNS_STATUS)
NVRAN_GET_MISC_FUNC(sal_misc_get_upgrade_status, SAL_MISC_UPGRADE_STATUS, upgrade_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_upgrade_status, SAL_MISC_UPGRADE_STATUS)
#ifdef VOX30_SFP
NVRAN_GET_MISC_FUNC(sal_misc_get_sfp_upgrade_status, SAL_MISC_SFP_UPGRADE_STATUS, sfp_upgrade_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_sfp_upgrade_status, SAL_MISC_SFP_UPGRADE_STATUS)
#endif
#ifdef CONFIG_SUPPORT_MEDIA_SERVER
NVRAN_GET_MISC_FUNC(sal_misc_get_media_status, SAL_MISC_MEDIA_STATUS, media_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_media_status, SAL_MISC_MEDIA_STATUS)
#endif
#ifdef CONFIG_SUPPORT_BBU
NVRAN_GET_MISC_FUNC(sal_misc_get_bbu_status, SAL_MISC_BBU_STATUS, bbu_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_bbu_status, SAL_MISC_BBU_STATUS)
#endif
NVRAN_GET_MISC_FUNC(sal_gpon_get_vif_status, SAL_MISC_GPON_VIF_STATUS, gpon_vif)
NVRAN_SET_MISC_FUNC(sal_gpon_set_vif_status, SAL_MISC_GPON_VIF_STATUS)

#ifdef CONFIG_SUPPORT_ADMIN_BACK_DOOR
NVRAN_SET_MISC_FUNC(sal_misc_set_superuser_language,  SAL_MISC_SUPERUSER_LANGUAGE)
NVRAN_GET_MISC_FUNC(sal_misc_get_superuser_language, SAL_MISC_SUPERUSER_LANGUAGE, superuser_language)
#endif
NVRAN_GET_MISC_FUNC(sal_misc_get_dsl_posttime_t,SAL_MISC_DSL_POSTTIME_ST_NAME,dsl_posttime)
NVRAN_SET_MISC_FUNC(sal_misc_set_dsl_posttime_t,SAL_MISC_DSL_POSTTIME_ST_NAME)
NVRAN_GET_MISC_FUNC(sal_misc_get_ether_posttime_t,SAL_MISC_ETHER_POSTTIME_ST_NAME,ether_posttime)
NVRAN_SET_MISC_FUNC(sal_misc_set_ether_posttime_t,SAL_MISC_ETHER_POSTTIME_ST_NAME)


NVRAN_SET_MISC_FUNC(sal_misc_set_ping_status, SAL_MISC_PING_STATUS)
NVRAN_GET_MISC_FUNC(sal_misc_get_ping_status, SAL_MISC_PING_STATUS, ping_status)
NVRAN_SET_MISC_FUNC(sal_misc_set_ping_result_line, SAL_MISC_PING_RESULT_LINE)
NVRAN_GET_MISC_FUNC(sal_misc_get_ping_result_line, SAL_MISC_PING_RESULT_LINE, ping_result_line)
NVRAN_SET_MISC_FUNC(sal_misc_set_ping_pid, SAL_MISC_PING_PID)
NVRAN_GET_MISC_FUNC(sal_misc_get_ping_pid, SAL_MISC_PING_PID, ping_pid)

NVRAN_GET_MISC_FUNC(sal_misc_get_reset_button_status, SAL_MISC_RESET_BUTTON_STATUS, reset_bt)
NVRAN_SET_MISC_FUNC(sal_misc_set_reset_button_status, SAL_MISC_RESET_BUTTON_STATUS)
NVRAN_GET_MISC_FUNC(sal_misc_get_reset_status, SAL_MISC_RESET_STATUS, reset)
NVRAN_SET_MISC_FUNC(sal_misc_set_reset_status, SAL_MISC_RESET_STATUS)
NVRAN_GET_MISC_FUNC(sal_misc_get_wps_button_status, SAL_MISC_WPS_BUTTON_STATUS, wps_bt)
NVRAN_SET_MISC_FUNC(sal_misc_set_wps_button_status, SAL_MISC_WPS_BUTTON_STATUS)
NVRAN_GET_MISC_FUNC(sal_misc_get_wifi_button_status, SAL_MISC_WIFI_BUTTON_STATUS, wifi_bt)
NVRAN_SET_MISC_FUNC(sal_misc_set_wifi_button_status, SAL_MISC_WIFI_BUTTON_STATUS)

NVRAN_GET_MISC_FUNC(sal_misc_get_config_computer_restore, SAL_MISC_CONFIG_COMPUTER_RESTOR, computer_do)
NVRAN_SET_MISC_FUNC(sal_misc_set_config_computer_restore, SAL_MISC_CONFIG_COMPUTER_RESTOR)
NVRAN_GET_MISC_FUNC(sal_misc_get_config_computer_restore_type, SAL_MISC_CONFIG_COMPUTER_RESTOR_TYPE, group)
NVRAN_SET_MISC_FUNC(sal_misc_set_config_computer_restore_type, SAL_MISC_CONFIG_COMPUTER_RESTOR_TYPE)

NVRAN_GET_MISC_FUNC(sal_misc_get_autosense_start_time, SAL_MISC_AUTOSENSE_START_TIME, auto_sense_start_time)
NVRAN_SET_MISC_FUNC(_sal_misc_set_autosense_start_time, SAL_MISC_AUTOSENSE_START_TIME)

NVRAN_GET_MISC_FUNC(sal_misc_get_reboot_cause, SAL_MISC_REBOOT_CAUSE, reboot_cause)
NVRAN_SET_MISC_FUNC(sal_misc_set_reboot_cause, SAL_MISC_REBOOT_CAUSE)

NVRAN_GET_MISC_FUNC(sal_misc_get_tr069_boot_event, SAL_MISC_TR_BOOT_EVENT, tr_boot_event)
NVRAN_SET_MISC_FUNC(sal_misc_set_tr069_boot_event, SAL_MISC_TR_BOOT_EVENT)
NVRAN_GET_MISC_FUNC(sal_misc_get_tr069_connect_to_acs_server, SAL_MISC_TR_ACS_STATUS, tr_acs_connect_stats)
NVRAN_SET_MISC_FUNC(sal_misc_set_tr069_connect_to_acs_server, SAL_MISC_TR_ACS_STATUS)

NVRAN_GET_MISC_FUNC1(sal_login_get_session_id, SAL_LOGIN_SID_NAME,sal_session_id)
NVRAN_SET_MISC_FUNC1(sal_login_set_session_id, SAL_LOGIN_SID_NAME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id1, SAL_LOGIN_LAST_SID1_NAME,sal_last_session_id1)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id1, SAL_LOGIN_LAST_SID1_NAME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id2, SAL_LOGIN_LAST_SID2_NAME,sal_last_session_id2)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id2, SAL_LOGIN_LAST_SID2_NAME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id3, SAL_LOGIN_LAST_SID3_NAME,sal_last_session_id3)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id3, SAL_LOGIN_LAST_SID3_NAME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id4, SAL_LOGIN_LAST_SID4_NAME,sal_last_session_id4)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id4, SAL_LOGIN_LAST_SID4_NAME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id5, SAL_LOGIN_LAST_SID5_NAME,sal_last_session_id5)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id5, SAL_LOGIN_LAST_SID5_NAME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id1_uptime, SAL_LOGIN_LAST_SID1_UPTIME,sal_last_session_id1_uptime)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id1_uptime, SAL_LOGIN_LAST_SID1_UPTIME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id2_uptime, SAL_LOGIN_LAST_SID2_UPTIME,sal_last_session_id2_uptime)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id2_uptime, SAL_LOGIN_LAST_SID2_UPTIME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id3_uptime, SAL_LOGIN_LAST_SID3_UPTIME,sal_last_session_id3_uptime)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id3_uptime, SAL_LOGIN_LAST_SID3_UPTIME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id4_uptime, SAL_LOGIN_LAST_SID4_UPTIME,sal_last_session_id4_uptime)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id4_uptime, SAL_LOGIN_LAST_SID4_UPTIME)
NVRAN_GET_MISC_FUNC1(sal_login_get_last_session_id5_uptime, SAL_LOGIN_LAST_SID5_UPTIME,sal_last_session_id5_uptime)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_session_id5_uptime, SAL_LOGIN_LAST_SID5_UPTIME)
NVRAN_GET_MISC_FUNC1(sal_login_get_csrf_token, SAL_LOGIN_CSRF_TOKEN,sal_csrf_token)
NVRAN_SET_MISC_FUNC1(sal_login_set_csrf_token, SAL_LOGIN_CSRF_TOKEN)
#ifdef CONFIG_SUPPORT_PASSWORD_PROTECTION
NVRAN_GET_MISC_FUNC1(sal_login_get_rstpwd_csrf_token, SAL_LOGIN_RSTPWD_CSRF_TOKEN,sal_csrf_token_rst)
NVRAN_SET_MISC_FUNC1(sal_login_set_rstpwd_csrf_token, SAL_LOGIN_RSTPWD_CSRF_TOKEN)
NVRAN_GET_MISC_FUNC1(sal_login_get_rstpwd_session_id, SAL_LOGIN_RSTPWD_SESSION_ID,sal_session_id_rst)
NVRAN_SET_MISC_FUNC1(sal_login_set_rstpwd_session_id, SAL_LOGIN_RSTPWD_SESSION_ID)
#endif
NVRAN_GET_MISC_FUNC1(sal_login_get_last_csrf_token, SAL_LOGIN_LAST_CSRF_TOKEN,sal_last_csrf_token)
NVRAN_SET_MISC_FUNC1(sal_login_set_last_csrf_token, SAL_LOGIN_LAST_CSRF_TOKEN)
NVRAN_GET_MISC_FUNC1(sal_login_get_csrf_token_hidden, SAL_LOGIN_CSRF_TOKEN_HIDDEN,sal_csrf_token_hidden)
NVRAN_SET_MISC_FUNC1(sal_login_set_csrf_token_hidden, SAL_LOGIN_CSRF_TOKEN_HIDDEN)
NVRAN_GET_MISC_FUNC1(sal_login_get_random_key, SAL_LOGIN_KEY,sal_login_key)
NVRAN_SET_MISC_FUNC1(sal_login_set_random_key, SAL_LOGIN_KEY)
NVRAN_GET_MISC_FUNC1(sal_misc_get_ipping_diag_succ_count_by_wan, SAL_MISC_IPPING_SUCCESS_COUNT,ipping_succ_count_wan)
NVRAN_SET_MISC_FUNC1(sal_misc_set_ipping_diag_succ_count_by_wan, SAL_MISC_IPPING_SUCCESS_COUNT)

NVRAN_GET_MISC_FUNC(sal_login_get_failed_times, SAL_LOGIN_FAILED_TIMES,sal_failed_times)
NVRAN_SET_MISC_FUNC(sal_login_set_failed_times, SAL_LOGIN_FAILED_TIMES)
NVRAN_GET_MISC_FUNC(sal_login_get_failed_times_setting, SAL_LOGIN_FAILED_TIMES_SETTING,sal_failed_times_setting)
NVRAN_SET_MISC_FUNC(sal_login_set_failed_times_setting, SAL_LOGIN_FAILED_TIMES_SETTING)
NVRAN_GET_MISC_FUNC(sal_login_get_ssh_failed_times, SAL_LOGIN_SSH_FAILED_TIMES,sal_ssh_failed_times)
NVRAN_SET_MISC_FUNC(sal_login_set_ssh_failed_times, SAL_LOGIN_SSH_FAILED_TIMES)

#ifdef CONFIG_SUPPORT_SJCL_ENCRYPT
NVRAN_GET_MISC_FUNC1(sal_login_get_decrypt_salt, SAL_LOGIN_SALT,sal_login_salt)
NVRAN_SET_MISC_FUNC1(sal_login_set_decrypt_salt, SAL_LOGIN_SALT)
NVRAN_GET_MISC_FUNC1(sal_login_get_decrypt_dk, SAL_LOGIN_DK,sal_login_dk)
NVRAN_SET_MISC_FUNC1(sal_login_set_decrypt_dk, SAL_LOGIN_DK)
#endif
#define SAL_MISC_AUTO_DIAG_RESULT                "Auto_Diag_Result"
#define SAL_MISC_AUTO_DIAG_BUSY                  "Auto_Diag_Busy"

NVRAN_GET_MISC_FUNC(sal_misc_get_auto_diag_result, SAL_MISC_AUTO_DIAG_RESULT, auto_diag_result)
NVRAN_SET_MISC_FUNC(sal_misc_set_auto_diag_result, SAL_MISC_AUTO_DIAG_RESULT)
NVRAN_GET_MISC_FUNC(sal_misc_get_auto_diag_busy, SAL_MISC_AUTO_DIAG_BUSY, auto_diag_busy)
NVRAN_SET_MISC_FUNC(sal_misc_set_auto_diag_busy, SAL_MISC_AUTO_DIAG_BUSY)

#define SAL_MISC_AUTO_PING_RESULT                "Auto_Ping_Result"
#define SAL_MISC_AUTO_PING_BUSY                  "Auto_Ping_Busy"

NVRAN_GET_MISC_FUNC(sal_misc_get_auto_ping_result, SAL_MISC_AUTO_PING_RESULT, auto_ping_result)
NVRAN_SET_MISC_FUNC(sal_misc_set_auto_ping_result, SAL_MISC_AUTO_PING_RESULT)
NVRAN_GET_MISC_FUNC(sal_misc_get_auto_ping_busy, SAL_MISC_AUTO_PING_BUSY, auto_ping_busy)
NVRAN_SET_MISC_FUNC(sal_misc_set_auto_ping_busy, SAL_MISC_AUTO_PING_BUSY)

#ifdef CONFIG_SUPPORT_IPV6
#define SAL_MISC_IPV6_PREFIX_CHANGED          "wan_ipv6_prefix_changed"
NVRAN_GET_MISC_FUNC(sal_misc_get_ipv6_prefix_changed_t, SAL_MISC_IPV6_PREFIX_CHANGED, wan_ipv6_prefix_changed)
NVRAN_SET_MISC_FUNC(sal_misc_set_ipv6_prefix_changed_t, SAL_MISC_IPV6_PREFIX_CHANGED)
#endif
#define SAL_MISC_FON_ENABLE                 "fon_enable"
#define SAL_MISC_GUEST_ENABLE               "guest_enable"
NVRAN_GET_MISC_FUNC(sal_misc_get_fon_enable, SAL_MISC_FON_ENABLE, fon_enable)
NVRAN_SET_MISC_FUNC(sal_misc_set_fon_enable, SAL_MISC_FON_ENABLE)
NVRAN_GET_MISC_FUNC(sal_misc_get_guest_enable, SAL_MISC_GUEST_ENABLE, guest_enable)
NVRAN_SET_MISC_FUNC(sal_misc_set_guest_enable, SAL_MISC_GUEST_ENABLE)
#define SAL_MISC_WIZARD_RESET     "wizard_reset"
NVRAN_GET_MISC_FUNC(sal_misc_get_wizard_reset, SAL_MISC_WIZARD_RESET, wizard_reset)
NVRAN_SET_MISC_FUNC(sal_misc_set_wizard_reset, SAL_MISC_WIZARD_RESET)
int sal_misc_set_autosense_start_time(char *value)
{
	FILE *fp = NULL;
	char tmp[128] = "";
	long time_now = 0;

	if(value == NULL)
    {
        if((fp = fopen("/proc/uptime", "r")) == NULL)
        {
            return -1;
        }
        fgets(tmp, sizeof(tmp), fp);
        sscanf(tmp, "%ld", &time_now);
        fclose(fp);
        snprintf(tmp, sizeof(tmp), "%ld", time_now);
        return _sal_misc_set_autosense_start_time(tmp);
    }
    else
    {
        return _sal_misc_set_autosense_start_time(value);
    }
}
int sal_misc_get_autosense_running_time(void)
{
    FILE *fp = NULL;
	char tmp[128] = "";
	long time_now = 0;
	long time_old = 0;
    char *atime;
	if((fp = fopen("/proc/uptime", "r")) == NULL)
	{
		return -1;
	}
	fgets(tmp, sizeof(tmp), fp);
	sscanf(tmp, "%ld", &time_now);
	fclose(fp);
    atime =  sal_misc_get_autosense_start_time();
    if(strlen(atime))
	    sscanf(atime, "%ld", &time_old);
    return time_now - time_old;
}
char *sal_misc_get_current_user(char *remote_ip)
{
	{
		static char buffer[SAL_MISC_TMP_VALUE_MAX_LENGTH];
		char *p;
		char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];
    	snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);
    	buffer[0] = '\0';
		p = nvram_get_fun(remote_ip, misc_nvram_path);
		if(p)
		{
			snprintf(buffer, sizeof(buffer), "%s", p);
			free(p);
		}
		return buffer;
	}
}

int sal_misc_set_logout_user(char *remote_ip, char *value)
{
	{
		char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];
        char name[256];
		if(!value)
			return -1;
    	snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);
        snprintf(name, sizeof(name), "logout_%s", remote_ip);
		return nvram_set_p(misc_nvram_path, name, value);
	}
}
char *sal_misc_get_logout_user(char *remote_ip)
{
	{
		static char buffer[SAL_MISC_TMP_VALUE_MAX_LENGTH];
		char *p;
        char name[256];
		char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];
    	snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);
    	buffer[0] = '\0';
        snprintf(name, sizeof(name), "logout_%s", remote_ip);
		p = nvram_get_fun(name, misc_nvram_path);
		if(p)
		{
			snprintf(buffer, sizeof(buffer), "%s", p);
			free(p);
		}
		return buffer;
	}
}

int sal_misc_set_current_user(char *remote_ip, char *value)
{
	{
		char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];
		if(!value)
			return -1;
    	snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);
		return nvram_set_p(misc_nvram_path, remote_ip, value);
	}
}
  
char *sal_misc_get_rc_status(const char *format, ...)
{
    {
        static char buffer[SAL_MISC_TMP_VALUE_MAX_LENGTH];
        char *p;
        char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];
        snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);
        buffer[0] = '\0';
        char name[SAL_MISC_TMP_PATH_MAX_LENGTH];
        va_list arg;
        va_start(arg, format);
        memset(name, 0, sizeof(name));
        vsnprintf(name, sizeof(name), format, arg);
        va_end(arg);
        p = nvram_get_fun(name, misc_nvram_path);
        if(p)
        {
            snprintf(buffer, sizeof(buffer), "%s", p);
            free(p);
        }
        return buffer;
    }
}

int sal_misc_set_rc_status(char *value, const char *format, ...)
{
    {
        char misc_nvram_path[SAL_MISC_TMP_PATH_MAX_LENGTH];
        if(!value)
            return -1;
        snprintf(misc_nvram_path, sizeof(misc_nvram_path), MISC_CFG_BASE);
        char name[SAL_MISC_TMP_PATH_MAX_LENGTH];
        va_list arg;
        va_start(arg, format);
        memset(name, 0, sizeof(name));
        vsnprintf(name, sizeof(name), format, arg);
        va_end(arg);
        return nvram_set_p(misc_nvram_path, name, value);
    }
}
#ifdef CONFIG_SUPPORT_HA
#define SAL_MISC_HA_ENABLE                 "ha_enable"
NVRAN_GET_MISC_FUNC(sal_misc_get_ha_status, SAL_MISC_HA_ENABLE, ha_enable)
NVRAN_SET_MISC_FUNC(sal_misc_set_ha_status, SAL_MISC_HA_ENABLE)
char *sal_misc_get_ha_state(void)
{
    static char ha_state[128] = "UnInitialized";
    FILE * fp;
    char *p;
    fp = fopen("/tmp/hybridaccess/status", "r");
    if(fp)
    {
        fgets(ha_state, sizeof(ha_state), fp); 
        p = strchr(ha_state, '\n');
        if(p)
            *p = '\0';
        fclose(fp);
    }
    return ha_state;
}
#endif
#ifdef CONFIG_SUPPORT_REPEATER
#ifdef CONFIG_SUPPORT_SUPERWIFI
#ifndef CONFIG_SUPPORT_PLUME
#define SAL_MISC_SUPERWIFI_ACTIVE_EVENT   "superwifi_active_event"
NVRAN_GET_MISC_FUNC(sal_misc_get_superwifi_active_event, SAL_MISC_SUPERWIFI_ACTIVE_EVENT, superwifi_event)
NVRAN_SET_MISC_FUNC(sal_misc_set_superwifi_active_event, SAL_MISC_SUPERWIFI_ACTIVE_EVENT)
#endif
#endif
#endif
