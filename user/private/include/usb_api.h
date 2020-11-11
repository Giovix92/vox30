#ifndef _USB_API_H_
#define _USB_API_H_

#include <cal/cal_usb_storage.h>
#ifdef CONFIG_SUPPORT_PACKAGE
#include <rcl/rcl_mgmt_package.h>
#endif

#define USTOR_MAX_DISK_COUNT        4
#define USTOR_MAX_PARTITION_COUNT   8
#define USTOR_MAX_FOLDER_COUNT      8 

#define USTOR_INFO_DIR  "/tmp/uxx_s/" 
#define USTOR_INFO_DIR_X  "/tmp/uxx_s" 
#define USTOR_BOOT_CACHE_INFO "/tmp/uxx_s/boot_cache"
#define USTOR_SCSI_DIR  "/proc/scsi/usb-storage/"
#define USTOR_MOUNT_DIR "/mnt/shares/"
#ifdef CONFIG_SUPPORT_PACKAGE
#define USTOR_MOUNT_DIR2 HOST_EE_ROOTFS"mnt/shares/"
#endif
#define USTOR_SHARE_INFO_DIR "/var/storage_info/"
#define USTOR_MEDIUM_TABLE "medium_table"

int usb_convert_sn_p2d(char *part_sn, char *disk_sn, int length);
int usb_convert_vol_refer(char * refer);

int usb_write_part_info(char *dev, cal_ustor_vol *p_part);
int usb_read_part_info(char *dev, cal_ustor_vol *p_part);
int usb_find_letter(cal_ustor_vol part, char *letter, int* is_campat);
int usb_read_conn_disks(cal_ustor_med **pdisk);
int usb_read_conn_parts(cal_ustor_med *pdisk, cal_ustor_vol **ppart);
int usb_read_disconn_disks(cal_ustor_med **pdisk);
int usb_read_disconn_parts(cal_ustor_med *pdisk, cal_ustor_vol **ppart);
int usb_read_all_folders(cal_ustor_folder **pfolder);
#ifdef CONFIG_SUPPORT_MEDIA_SERVER
int usb_read_all_mediashares(cal_ustor_mediashare **pfolder);
#endif
int usb_part_active(cal_ustor_vol* ppart, char *dev);
int usb_part_deactive(char *dev, int index);
int usb_disk_remove(char *dev,int index);
int usb_status_update(void);
int usb_device_connected(void);
int usb_get_medium_info(char *dev, cal_ustor_med *pdisk);
int usb_read_active_disks(cal_ustor_med *pdisk);

int usb_read_med_folder(cal_ustor_med *med, cal_ustor_folder **folder);
int usb_parse_given_folder(char *name, char *dir);
int usb_read_user_share(int foldernum, cal_ustor_folder *mfold, cal_ustor_user user, cal_ustor_folder **fold,cal_ustor_ua **ua);
int usb_read_public_share(int foldernum, cal_ustor_folder *mfold, cal_ustor_folder **fold);

int usb_write_medium_info(char *dev, cal_ustor_med *pdisk);
int usb_read_medium_info(char *dev, cal_ustor_med *pdisk);

//usb share info
int usb_add_medium_table(cal_ustor_med *med);
int usb_save_share_info(cal_ustor_vol *vol);
int usb_read_share_info(cal_ustor_vol *vol, cal_ustor_folder **folder);

int usb_disk_writable_check(char *dir);
int usb_disk_get_writable_dir(char *dir);
int usb_disk_find_file_dir(char *dir);
#endif

