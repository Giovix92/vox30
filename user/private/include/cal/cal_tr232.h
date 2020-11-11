#ifndef _CAL_TR232_H_
#define _CAL_TR232_H_
char* cal_bulkdata_enable_get();
int cal_bulkdata_enable_set(char* val);
char* cal_bulkdata_status_get();
int cal_bulkdata_status_set(char* val);
char* cal_bulkdata_report_interval_get();
int cal_bulkdata_report_interval_set(char* val);
char* cal_bulkdata_protocol_get();
int cal_bulkdata_protocol_set(char* val);
char* cal_bulkdata_encode_type_get();
int cal_bulkdata_encode_type_set(char* val);
char* cal_bulkdata_profile_Enable_get(int id0);
int cal_bulkdata_profile_Enable_set(int id0, char* val);
char* cal_bulkdata_profile_Alias_get(int id0);
int cal_bulkdata_profile_Alias_set(int id0, char* val);
char* cal_bulkdata_profile_Protocol_get(int id0);
int cal_bulkdata_profile_Protocol_set(int id0, char* val);
char* cal_bulkdata_profile_EncodingType_get(int id0);
int cal_bulkdata_profile_EncodingType_set(int id0, char* val);
char* cal_bulkdata_profile_ReportingInterval_get(int id0);
int cal_bulkdata_profile_ReportingInterval_set(int id0, char* val);
char* cal_bulkdata_profile_timeReference_get(int id0);
int cal_bulkdata_profile_TimeReference_set(int id0, char* val);
char* cal_bulkdata_profile_StreamingHost_get(int id0);
int cal_bulkdata_profile_StreamingHost_set(int id0, char* val);
char* cal_bulkdata_profile_StreamingPort_get(int id0);
int cal_bulkdata_profile_StreamingPort_set(int id0, char* val);
char* cal_bulkdata_profile_StreamingSessionID_get(int id0);
int cal_bulkdata_profile_StreamingSessionID_set(int id0, char* val);
char* cal_bulkdata_profile_FileTransferURL_get(int id0);
int cal_bulkdata_profile_FileTransferURL_set(int id0, char* val);
char* cal_bulkdata_profile_FileTransferUsername_get(int id0);
int cal_bulkdata_profile_FileTransferUsername_set(int id0, char* val);
char* cal_bulkdata_profile_FileTransferPassword_get(int id0);
int cal_bulkdata_profile_FileTransferPassword_set(int id0, char* val);
char* cal_bulkdata_profile_ControlFileFormat_get(int id0);
int cal_bulkdata_profile_ControlFileFormat_set(int id0, char* val);
typedef struct cal_bulkdata_profile_s {
    char id[8];
    char Enable[9];
    char Alias[65];
    char Protocol[513];
    char EncodingType[513];
    char ReportingInterval[16];
    char TimeReference[257];
    char StreamingHost[257];
    char StreamingPort[16];
    char StreamingSessionID[16];
    char FileTransferURL[257];
    char FileTransferUsername[65];
    char FileTransferPassword[65];
    char ControlFileFormat[129];
} cal_bulkdata_profile_t;
int cal_bulkdata_profile_tab_add(int id0);
int cal_bulkdata_profile_tab_set(int id0, cal_bulkdata_profile_t *p_cal_bulkdata_profile);
int cal_bulkdata_profile_tab_del(int id0);
int cal_bulkdata_profile_tab_num_get(int **pp_index_array);
int cal_bulkdata_profile_tab_get(int id0, cal_bulkdata_profile_t **pp_cal_bulkdata_profile);
char *cal_bulkdata_profile_prameter_Name_get(int id0, int id1);
int cal_bulkdata_profile_prameter_Name_set(int id0, int id1, char *val);
char* cal_bulkdata_profile_prameter_Reference_get(int id0, int id1);
int cal_bulkdata_profile_prameter_Reference_set(int id0, int id1, char* val);
typedef struct cal_bulkdata_profile_prameter_s {
    char id[8];
    char name[513];
    char Reference[257];
} cal_bulkdata_profile_prameter_t;
typedef struct cal_bulkdata_profile_refer_parameter_s {
    char name[256];
    char value[256];
} cal_bulkdata_profile_refer_parameter_t;
int cal_bulkdata_get_profile_param_refername_tab(cal_bulkdata_profile_prameter_t *pro_parm, cal_bulkdata_profile_refer_parameter_t **cfg_refer_tab);
int cal_bulkdata_profile_prameter_tab_add(int id0, int id1);
int cal_bulkdata_profile_prameter_tab_set(int id0, int id1, cal_bulkdata_profile_prameter_t *p_cal_bulkdata_profile_prameter);
int cal_bulkdata_profile_prameter_tab_del(int id0, int id1);
int cal_bulkdata_profile_prameter_tab_num_get(int id0, int **pp_index_array);
int cal_bulkdata_profile_prameter_tab_get(int id0, int id1, cal_bulkdata_profile_prameter_t **pp_cal_bulkdata_profile_prameter);

char *cal_bulkdata_profile_CSVEncoding_FieldSeparator_get(int id0);
int cal_bulkdata_profile_CSVEncoding_FieldSeparator_set(int id0, char *val);
char *cal_bulkdata_profile_CSVEncoding_RowSeparator_get(int id0);
int cal_bulkdata_profile_CSVEncoding_RowSeparator_set(int id0, char *val);
char *cal_bulkdata_profile_CSVEncoding_EscapeCharacter_get(int id0);
int cal_bulkdata_profile_CSVEncoding_EscapeCharacter_set(int id0, char *val);
char *cal_bulkdata_profile_CSVEncoding_ReportFormat_get(int id0);
int cal_bulkdata_profile_CSVEncoding_ReportFormat_set(int id0, char *val);
char *cal_bulkdata_profile_CSVEncoding_RowTimestamp_get(int id0);
int cal_bulkdata_profile_CSVEncoding_RowTimestamp_set(int id0, char *val);
char *cal_bulkdata_profile_JSONEncoding_ReportFormat_get(int id0);
int cal_bulkdata_profile_CSVEncoding_RowTimestamp_set(int id0, char *val);
char *cal_bulkdata_profile_JSONEncoding_ReportTimestamp_get(int id0);
int cal_bulkdata_profile_JSONEncoding_ReportTimestamp_set(int id0, char *val);

char *cal_bulkdata_profile_HTTP_URL_get(int id0);
int cal_bulkdata_profile_HTTP_URL_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_Username_get(int id0);
int cal_bulkdata_profile_HTTP_Username_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_Password_get(int id0);
int cal_bulkdata_profile_HTTP_Password_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_CompressionsSupported_get(int id0);
int cal_bulkdata_profile_HTTP_CompressionsSupported_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_Compression_get(int id0);
int cal_bulkdata_profile_HTTP_Compression_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_MethodsSupported_get(int id0);
int cal_bulkdata_profile_HTTP_MethodsSupported_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_Method_get(int id0);
int cal_bulkdata_profile_HTTP_Method_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_UserDateHeader_get(int id0);
int cal_bulkdata_profile_HTTP_UserDateHeader_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_RetryEnable_get(int id0);
int cal_bulkdata_profile_JSONEncoding_ReportTimestamp_set(int id0, char *val);
char *cal_bulkdata_profile_JSONEncoding_ReportTimestamp_get(int id0);
int cal_bulkdata_profile_JSONEncoding_ReportTimestamp_set(int id0, char *val);
char *cal_bulkdata_profile_JSONEncoding_ReportTimestamp_get(int id0);
int cal_bulkdata_profile_JSONEncoding_ReportTimestamp_set(int id0, char *val);
/*typedef struct http_req_uri_param {
    char name[64];
    char reference[256];
    char value[256];
} http_req_uri_param_t;*/
typedef struct cal_bulkdata_profile_http_request_uri_parameter_s {
    char id[8];
    char name[513];
    char reference[257];
    char value[256];
} cal_bulkdata_profile_http_request_uri_parameter_t;

typedef struct cal_bulkdata_http_profile {
    char *oui;
    char *sn;
    char *product;
    char url[1024];
    char username[256];
    char password[256];
    char method[16];
    int userdataheader;
    int retry_enable;
    char json_encode_format[64];
    int req_uri_param_num;
    int min_wait;
    int multiplier_interval;
    cal_bulkdata_profile_http_request_uri_parameter_t *requestURIParameter;
} bulkdata_http_cfg_t;
int cal_bulkdata_profile_http_tab_num_get(int id , int **pp_index_array);
int cal_bulkdata_get_http_config(char *id, bulkdata_http_cfg_t **cfg);
char *cal_bulkdata_profile_HTTP_URL_get(int id0);
int cal_bulkdata_profile_HTTP_URL_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_Username_get(int id0);
int cal_bulkdata_profile_HTTP_Username_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_Password_get(int id0);
int cal_bulkdata_profile_HTTP_Password_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_CompressionsSupported_get(int id0);
int cal_bulkdata_profile_HTTP_CompressionsSupported_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_Compression_get(int id0);
int cal_bulkdata_profile_HTTP_Compression_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_MethodsSupported_get(int id0);
int cal_bulkdata_profile_HTTP_MethodsSupported_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_Method_get(int id0);
int cal_bulkdata_profile_HTTP_Method_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_UserDateHeader_get(int id0);
int cal_bulkdata_profile_HTTP_UserDateHeader_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_RetryEnable_get(int id0);
int cal_bulkdata_profile_HTTP_RetryEnable_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_RetryMinimumWaitInterval_get(int id0);
int cal_bulkdata_profile_HTTP_RetryMinimumWaitInterval_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_RetryIntervalMultiplier_get(int id0);
int cal_bulkdata_profile_HTTP_RetryIntervalMultiplier_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_RequestURIParameterNumberOfEntries_get(int id0);
int cal_bulkdata_profile_HTTP_RequestURIParameterNumberOfEntries_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_PersistAcrossReboot_get(int id0);
int cal_bulkdata_profile_HTTP_PersistAcrossReboot_set(int id0, char *val);
char *cal_bulkdata_profile_HTTP_RequestURIParameter_Name_get(int id0, int id1);
int cal_bulkdata_profile_HTTP_RequestURIParameter_Name_set(int id0, int id1, char *val);
char *cal_bulkdata_profile_HTTP_RequestURIParameter_Reference_get(int id0, int id1);
int cal_bulkdata_profile_HTTP_RequestURIParameter_Reference_set(int id0, int id1, char *val);
int cal_bulkdata_profile_HTTP_RequestURIParameter_tab_add(int id0, int id1);

/*typedef struct cal_bulkdata_profile_http_request_uri_parameter_s {
    char id[8];
    char name[513];
    char reference[257];
    char value[256];
} cal_bulkdata_profile_http_request_uri_parameter_t;*/
int cal_bulkdata_profile_http_request_uri_parameter_tab_set(int id0, int id1, cal_bulkdata_profile_http_request_uri_parameter_t *p_cal_bulkdata_profile_http_request_uri_parameter);
int cal_bulkdata_profile_http_request_uri_parameter_tab_del(int id0, int id1);
int cal_bulkdata_profile_http_request_uri_parameter_tab_num_get(int id0, int **pp_index_array);
int cal_bulkdata_profile_http_request_uri_parameter_tab_get(int id0, int id1, cal_bulkdata_profile_http_request_uri_parameter_t **pp_cal_bulkdata_profile_http_request_uri_parameter);


char* cfg_devinfo_ManufacturerOUI_get();
char* cfg_devinfo_ProductClass_get();
char* cfg_devinfo_SerialNumber_get();



int cal_bulkdata_get_profile_tab(cal_bulkdata_profile_t **cfg_bulk);
/*to add here*/
#endif //CAL_CFG_H_
