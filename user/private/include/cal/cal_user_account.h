#ifndef CAL_USER_ACCOUNT_H
#define CAL_USER_ACCOUNT_H
#include <pwd.h>

enum
{
    UA_ACCOUNT_FULL_NAME = 0,
#ifdef CONFIG_SUPPORT_PRPL_HL_API
    UA_ACCOUNT_ENABLE,
    UA_ACCOUNT_NAME,
#endif
    UA_ACCOUNT_USER_NAME,
    UA_ACCOUNT_PASSWORD,
    UA_ACCOUNT_GROUP,
    UA_ACCOUNT_LANGUAGE,
    UA_ACCOUNT_PERMISSION,
    UA_ACCOUNT_ACCESSMODE,
#ifdef CONFIG_SUPPORT_PRPL_HL_API
    UA_ACCOUNT_HASHTYPE,
    UA_ACCOUNT_HASHSALT,
    UA_ACCOUNT_DESCRIPTION,
#endif
    UA_ACCOUNT_DEFAULTPASSWORDALERT
};

enum
{
   UA_ACCOUNT_LAN = 1,
   UA_ACCOUNT_WAN,
   UA_ACCOUNT_ALL
};

enum UA_encrypt
{
    UA_ENCRYPT_NONE = 0,
    UA_ENCRYPT_MD5,
    UA_ENCRYPT_CRYPT_MD5,
    UA_ENCRYPT_HMAC_SHA256
};

enum UA_Service
{
    UA_SERVICE_WEB = 0,
    UA_SERVICE_CLI,
    UA_SERVICE_FTP,
    UA_SERVICE_SMB
};

typedef struct 
{
    char username[256];
    char passwd[128];
    char group[128];
}UA_info;

#define SYSTEM_ACCOUNT_PATH "/var/passwd"
#define SYSTEM_ACCOUNT_PATH_REMOTE "/var/passwd_remote"

#define UA_Service_str_web  "web"
#define UA_Service_str_cli  "cli"
#define UA_Service_str_ftp  "ftp"
#define UA_Service_str_smb  "smb"

#define UA_VALID_USER_ID_BEGIN      1 
#define UA_MAX_USER_NUM             32
#ifdef CONFIG_SUPPORT_ADMIN_BACK_DOOR
/*This is not a proper way, plese fix me !!!*/
#define UA_ROOT_ID                  0
#else
#define UA_ROOT_ID                  1
#endif
#define UA_ADMIN_GROUP              "admin"
#define UA_SUPPORT_GROUP            "support"
#define UA_USER_GROUP               "user"
#define UA_LAN_ACCESS               "lan"
#define UA_WAN_ACCESS               "wan"
#define UA_ALL_ACCESS               "all"
#define UA_ADMIN_GROUP_ID           0
#define UA_SUPPORT_GROUP_ID         2002
#define UA_USER_GROUP_ID            2003
#define UA_ADMIN_ID_BASE            0 
#define UA_SUPPORT_ID_BASE          500 
#define UA_USER_ID_BASE             1000
int cal_UA_get_entry_index_list(int** index_array);
char *cal_UA_get_account_details(int id, int element);
int cal_UA_get_service_accounts(char *service, UA_info ** info_arr);
/* --------------------------------------------------------------------------*/
/**
 * @brief     set account details according to element
 *
 * @Param    id
 * @Param    value
 *
 * @Returns  0 success, else fail.  
 */
/* ----------------------------------------------------------------------------*/
int cal_UA_set_account_details(int id, int element, char *value);
int cal_UA_delete_account(int id);
int cal_UA_add_account();
int cal_UA_get_user_index(int element, const char *match);

int cal_UA_generate_passwd_file(char *service, char *path, int encrypt, int access_mode);
int cal_UA_end_passwd_changed(void);
struct passwd *cal_UA_getpwnam(char *path, const char * name);
struct passwd *cal_UA_getpwuid(char *path, uid_t uid);

int cal_UA_get_gui_permission(const char *name, int groupID);
#ifdef CONFIG_SUPPORT_WIZARD
int cal_UA_is_end_user_account_configured(void);
#endif

char* cal_UA_get_pwd_uri_by_id(int id);
#ifdef CONFIG_SUPPORT_PRPL_HL_API
int cal_UR_add_role();
int cal_UR_add_rule(int role_index);
char *cal_UR_get_id(int id);
char *cal_UR_get_name(int id);
char *cal_UR_get_description(int id);
char *cal_UR_get_rule_id(int id, int rule_id);
char *cal_UR_get_rule_enable_state(int id, int rule_id);
char *cal_UR_get_rule_service_id(int id, int rule_id);
char *cal_UR_get_rule_mode(int id, int rule_id);

int cal_UR_set_id(int id, char *value);
int cal_UR_set_name(int id, char *value);
int cal_UR_set_description(int id, char *value);
int cal_UR_set_rule_id(int id, int rule_id, char *value);
int cal_UR_set_rule_enable_state(int id, int rule_id, char *value);
int cal_UR_set_rule_service_id(int id, int rule_id, char *value);
int cal_UR_set_rule_mode(int id, int rule_id, char *value);
#endif
#endif

