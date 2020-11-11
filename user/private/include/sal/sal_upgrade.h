#ifndef __SAL_UPGRADE_H__
#define __SAL_UPGRADE_H__

#define RESUME_FILE "/tmp/upgrade_resume.sh"
enum UPGRADE_STATUS
{
    UPGRADE_FAIL_CALL_ONGOING = -5,
    UPGRADE_FAIL_INVALID_FW = -4,
    UPGRADE_FAIL_INVALID_FW_VERSION = -3,
    UPGRADE_FAIL_LIMIT_RESOURCE = -2,
    UPGRADE_FAIL_UNKNOW = -1,
    UPGRADE_SUCCESS
};

int sal_upgrade_set_active(int fw_index, char *value);
int sal_upgrade_set_valid(int fw_index, char *value);
int sal_upgrade_get_active(int fw_index);
int sal_upgrade_get_valid(int fw_index);
int sal_upgrade_get_running(int fw_index);
unsigned short sal_upgrade_get_version(int fw_index);
int sal_upgrade_free_ram_size_check(void);
int sal_on_going_call_check(void);
int util_get_upgrade_status(void);
int util_set_upgrade_status(int status);
int util_clear_upgrade_status(void);
#ifdef VOX30_SFP
int util_get_sfp_upgrade_status(void);
int util_set_sfp_upgrade_status(int status);
int util_clear_sfp_upgrade_status(void);
#endif

#endif

