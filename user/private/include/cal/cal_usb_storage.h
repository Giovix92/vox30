#ifndef _CAL_USB_STORAGE_H_ 
#define _CAL_USB_STORAGE_H_

#define GROUP_SMB "smb"
#define GROUP_FTP "ftp"
#define STORAGESERVICE_ID 1
typedef struct _ustor_med
{
    char key[8];
    char name[64];
    char model[128];
    char sn[64];
    char capacity[64];
    char status[8];
    char vendor[64];
    char uptime[8];
    char share_enable[8];
}cal_ustor_med;


typedef struct _ustor_vol
{
    char key[8];
    char name[64];
    char sn[64];
    char capacity[64];
    char fs_type[8];
    char refer[256];    /* key, template */
    char enable[8];
    char status[8];
    char fold[8];
}cal_ustor_vol;

typedef struct _ustor_folder
{
    char key[16];
    char ns_showname[64];
    char ftp_showname[64];
    char name[64];
    char access[8];
    char enable[8];
    char usernum[8];
    char ftp_enable[8];
    char ftp_wan_enable[8];
    char smb_enable[8];
    char media_enable[8];
    char wan_access[8];
} cal_ustor_folder;

typedef struct _ustor_ua
{
    char key[16];
    char ref[256];
    char perm[8];
    char ftp_perm[8];
} cal_ustor_ua;

typedef struct _ustor_user
{
    char key[8];
    char enable[4];
    char username[64];
    char password[64];
} cal_ustor_user;
#if defined(CONFIG_SUPPORT_MEDIA_SERVER) || defined(CONFIG_SUPPORT_TWONKY_SERVER)
typedef struct _ustor_mediashare
{
    char key[8];
    char path[128];
    char name[128];
#ifndef CONFIG_SUPPORT_TWONKY_SERVER
    char dev_name[32];
    char connected[8];
#endif
} cal_ustor_mediashare;
#endif
int cal_ustor_get_level_id(char *key, int level);
// numberofentries
char *cal_ustor_get_num_useraccount(int s_id);
char *cal_ustor_get_num_volume(int s_id);
char *cal_ustor_get_num_medium(int s_id);
//capable

char *cal_ustor_get_capa_ftp(int s_id);
char *cal_ustor_get_capa_sftp(int s_id);
char *cal_ustor_get_capa_smb(int s_id);

//useraccess
#define H_DEF_USTOR_FUNC_FORTH(para) \
char *cal_ustor_get_##para(int id0, int id1, int id2, int id3);\
int cal_ustor_set_##para(char *value, int id0, int id1, int id2, int id3);
H_DEF_USTOR_FUNC_FORTH(useraccess_refer)
H_DEF_USTOR_FUNC_FORTH(useraccess_perm) 
int cal_ustor_get_one_ua(cal_ustor_ua *ua, int id0,int v_index, int f_index, int index);
int cal_ustor_get_all_ua(cal_ustor_ua **ua, int id0, int v_index, int f_index);
int cal_ustor_ua_set(cal_ustor_ua *ua, int id0, int v_index, int f_index, int index);
int cal_ustor_add_ua(cal_ustor_ua *ua, int id0, int v_index, int f_index);
int cal_ustor_add_up(cal_ustor_user *user, char *group);
int cal_ustor_modify_ua(cal_ustor_ua *ua, int id0, int p_index, int f_index, int index);
int cal_ustor_del_ua(int id0, int p_index, int f_index, int index);
// folder
#define H_DEF_USTOR_FUNC_THREE(para) \
char *cal_ustor_get_##para(int id0, int id1, int id2 );\
int cal_ustor_set_##para(char *value, int id0, int id1, int id2 );
H_DEF_USTOR_FUNC_THREE(folder_showname)
H_DEF_USTOR_FUNC_THREE(folder_show)
H_DEF_USTOR_FUNC_THREE(folder_enable)
H_DEF_USTOR_FUNC_THREE(folder_useraccess)
int cal_ustor_get_one_folder(cal_ustor_folder *p_folder, int id0, int p_index, int f_index);
int cal_ustor_get_all_folder(cal_ustor_folder **p_folder, int id0, int p_index);
int cal_ustor_folder_set(cal_ustor_folder *folder, int id0, int v_index, int f_index);
int cal_ustor_del_folder(int id0, int p_index, int f_index);

int cal_ustor_modify_folder(cal_ustor_folder *p_folder, int id0, int p_index, int f_index);
int cal_ustor_add_folder(cal_ustor_folder *p_folder, int id0, int d_index);
    //volume
#define H_DEF_USTOR_FUNC_TWO(para) \
char *cal_ustor_get_##para(int id0, int id1  );\
int cal_ustor_set_##para(char *value, int id0, int id1 );
H_DEF_USTOR_FUNC_TWO(volume_name)
H_DEF_USTOR_FUNC_TWO(volume_enable)
H_DEF_USTOR_FUNC_TWO(volume_sn)
H_DEF_USTOR_FUNC_TWO(volume_refer)
H_DEF_USTOR_FUNC_TWO(volume_capacity)
int cal_ustor_get_one_vol(cal_ustor_vol *p_part,int id0, int index);
int cal_ustor_get_all_vol(cal_ustor_vol **p_part, int id0);
int cal_ustor_add_vol(cal_ustor_vol *p_part,int id0);
int cal_ustor_del_vol(int id0, int index);

H_DEF_USTOR_FUNC_TWO(medium_name);
H_DEF_USTOR_FUNC_TWO(medium_sn);
char * cal_ustor_get_medium_model(int id0, int id1);
int cal_ustor_get_one_med(cal_ustor_med *p_part, int id0, int index);
int cal_ustor_get_all_med(cal_ustor_med **p_part, int id0);
int cal_ustor_add_med(int id0, cal_ustor_med *p_part);
int cal_ustor_del_med(int id0, int index);

H_DEF_USTOR_FUNC_TWO(med_ftp_en)
H_DEF_USTOR_FUNC_TWO(med_ftp_wan_en)
H_DEF_USTOR_FUNC_TWO(med_smb_en)
H_DEF_USTOR_FUNC_TWO(med_smb_need_auth)
#if defined(CONFIG_SUPPORT_MEDIA_SERVER) || defined(CONFIG_SUPPORT_TWONKY_SERVER)
H_DEF_USTOR_FUNC_TWO(med_media_shareall)
H_DEF_USTOR_FUNC_TWO(med_media_share_en)
#endif
int cal_ustor_set_fold_smb_noauth(int id0, int id1, int id2,char *enable);

char * cal_ustor_get_med_smb_no_auth_shareall(int id0, int m_index);
int cal_ustor_set_med_smb_no_auth_shareall(int id0, int index, char *enable);

H_DEF_USTOR_FUNC_TWO(useraccount_name)
H_DEF_USTOR_FUNC_TWO(useraccount_enable)
H_DEF_USTOR_FUNC_TWO(useraccount_group)
H_DEF_USTOR_FUNC_TWO(useraccount_password)

int cal_ustor_add_account(cal_ustor_user *user, int id0);
int cal_ustor_set_user(cal_ustor_user *user, int id0);
char * cal_ustor_get_user_passwd(cal_ustor_user *user);
int cal_ustor_get_user_info(int id0, int index,cal_ustor_user *user);
int cal_ustor_get_all_user(cal_ustor_user **user,int id0);
int cal_ustor_get_service_user(int usernum, cal_ustor_user *user, char *group,cal_ustor_user **outuser);
int cal_ustor_del_user(int u_index,int id0);
char * cal_ustor_get_shareall(int u_index, int id0, int m_index);
int cal_ustor_set_shareall(int id0, int index,  int m_index, char *enable);









#if defined(CONFIG_SUPPORT_MEDIA_SERVER) || defined(CONFIG_SUPPORT_TWONKY_SERVER)
int cal_ustor_get_one_mediashare(cal_ustor_mediashare *p_folder, int p_index, int f_index);
int cal_ustor_get_all_mediashare(cal_ustor_mediashare **p_folder, int p_index);
int cal_ustor_add_mediashare(cal_ustor_mediashare *p_folder, int d_index);
int cal_ustor_modify_mediashare(cal_ustor_mediashare *p_folder, int p_index, int f_index);
int cal_ustor_del_mediashare(int p_index, int f_index);
char* cal_ustor_get_mediaserver_uri();
#endif
char *cal_ustor_get_auto_share_account(void);

#define H_DEF_USTOR_FUNC(para) \
char *cal_ustor_get_##para(int id);\
int cal_ustor_set_##para(char *value, int id);

H_DEF_USTOR_FUNC(en)
H_DEF_USTOR_FUNC(ftp_port)
H_DEF_USTOR_FUNC(ftp_idle)
H_DEF_USTOR_FUNC(ftp_en)
H_DEF_USTOR_FUNC(ftp_max_users)
H_DEF_USTOR_FUNC(ftp_anony_enable)
H_DEF_USTOR_FUNC(ftp_anony_startfolder)
H_DEF_USTOR_FUNC(ftp_anony_readonly)
H_DEF_USTOR_FUNC(sftp_en)
H_DEF_USTOR_FUNC(sftp_max_users)
H_DEF_USTOR_FUNC(sftp_idletime)
H_DEF_USTOR_FUNC(sftp_portnumber)
#if defined(CONFIG_SUPPORT_MEDIA_SERVER) || defined(CONFIG_SUPPORT_TWONKY_SERVER)
H_DEF_USTOR_FUNC(media_en)
H_DEF_USTOR_FUNC(media_friendly)
H_DEF_USTOR_FUNC(media_language)
#endif

H_DEF_USTOR_FUNC(domainname)
H_DEF_USTOR_FUNC(hostname)
H_DEF_USTOR_FUNC(smb_en)
H_DEF_USTOR_FUNC(smbsigning_en)
H_DEF_USTOR_FUNC(smb_no_auth)
H_DEF_USTOR_FUNC(workgroup)
H_DEF_USTOR_FUNC(servername)
H_DEF_USTOR_FUNC(netbiosname)
int cal_ustor_ftp_set_interface(char *value);
char *cal_ustor_ftp_get_interface(void);
int cal_ustor_ftps_set_interface(char *value);
char *cal_ustor_ftps_get_interface(void);
#endif

