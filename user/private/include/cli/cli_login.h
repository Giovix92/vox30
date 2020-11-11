/** 
* @file   cli_login.h. 
* @module SERCOMM CLI
* @author Miracle Wang
* @version V1.00 
*
* 
* @copyright 2011 SerComm Corporation. All Rights Reserved. */

#ifndef _CLI_LOGIN_H
#define _CLI_LOGIN_H

#include <sys/types.h>
#include <pwd.h>
#include "cal/cal_user_account.h"

#define SC_CRYPT_SALT           "$1$SERCOMM$"
#define MAXHOSTNAMELEN          256 
#define CLI_ACCOUNT_PATH        "/var/cli_pw"
#define CLI_ACCOUNT_PATH_REMOTE        "/var/cli_pw_remote"
#define ROOT_GID                UA_ADMIN_GROUP_ID
#define SUPPORT_GID             UA_SUPPORT_GROUP_ID
#define USER_GID                UA_USER_GROUP_ID

#define CLI_HOSTNAME_ENV "cli_hostname_env"
#define CLI_MODULE_ENV "cli_module_env"
#define CLI_IPADDR_ENV "cli_ipaddr_env"

int cli_login_auth(char *username, int name_len, struct passwd **ppw, int no_passwd_flag, int accessmode);

#endif

