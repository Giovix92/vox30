#ifndef _CAL_WEBAPI_H_
#define _CAL_WEBAPI_H_

#define SMARTAPP_CERT_DIR "/tmp/cert"
#define SMARTAPP_CERT_FORMAT SMARTAPP_CERT_DIR"/%s.pem"
typedef struct cal_cert_entry_t
{
    char content[4096];
    char date[256];
    char cname[256];
}cal_cert_entry;

typedef struct cal_webapi_secAccount_entry_t
{
    char username[64];
    char password[64];
    char friendlyName[64];
}cal_webapi_secAccount_entry;

int cal_cert_get_client_certificates_aviable_index(int **array);
int cal_cert_del_client_certificates(int index);
char *cal_cert_get_client_certificates_commonName(int id);
int cal_cert_set_client_certificates_commonName(char *value,int id);
char *cal_cert_get_client_certificates_content(int id);
int  cal_cert_set_client_certificates_content(char *value,int id);
char *cal_cert_get_client_certificates_lastusedate(int id);
int  cal_cert_set_client_certificates_lsatusetime(char *value,int id);
int  cal_cert_addonce_client_certificates_entry(cal_cert_entry* value);
int cal_cert_get_client_certificates_entry(int **array);
int cal_cert_del_client_certificates_index(int index);

char *cal_web_api_get_enable(void);
int cal_web_api_set_enable(char *value);
char *cal_web_api_get_accountLevel(void);
char *cal_web_api_get_accountName(void);
int cal_web_api_set_accountName(char *value);
int  cal_web_api_set_secAccountName(char *value,int accountid);
char *cal_web_api_get_secAccountName(int accountid);
char *cal_web_api_get_secAccountPasswd(int accountid);
char *cal_web_api_get_accountPassword(void);
int cal_web_api_set_accountPassword(char *value);
int cal_web_api_set_accountFriendlyName(char *value);
char *cal_web_api_get_primary_account_friendly_name();
char *cal_web_api_get_secondary_account_friendly_name(int accountid);
int cal_web_api_set_secAccountPassword(char *value,int accountid);
char *cal_web_api_get_remoteAccessEnable(void);
int  cal_web_api_set_remoteAccessEnable(char *value);
int  cal_web_api_addonce_secAccount_entry(cal_webapi_secAccount_entry* value);
int cal_web_api_get_secAccountList_available_index();
int cal_web_api_get_secAccount_index_avaiable(int **array);
int cal_web_api_del_secAccount_index(int index);
int cal_web_api_set_primary_extension_number(char *value);
char *cal_web_api_get_primary_extension_number(void); 
int cal_web_api_set_secondary_extension_number(char *value, int accountid);
char *cal_web_api_get_secondary_extension_number(int accountid);

#endif
