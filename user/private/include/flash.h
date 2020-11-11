#ifndef _FLASH_H_
#define _FLASH_H_


#define FLASH_SIZE        		0x01000000  /*16M*/
#define BIN_FILE_SIZE           FLASH_SIZE

#define	PARTITION_NUM	        4

#define	MTD_BOOT                0x01
#define	MTD_KERN                0x02
#define	MTD_RTFS                0x04
#define MTD_CALI                0x08

#define BOOT_SIZE               0x00100000   // 1M
#define KERN_SIZE               0x00200000   // 2M
#define RTFS_SIZE               0x00C80000   // 12.5M 
#define CALI_SIZE				(FLASH_SIZE - BOOT_SIZE  - KERN_SIZE - RTFS_SIZE) //0.5M

/* offset in bin file */
#define BOOT_BIN_OFF            0
#define KERN_BIN_OFF            (BOOT_BIN_OFF+BOOT_SIZE)
#define RTFS_BIN_OFF            (KERN_BIN_OFF+KERN_SIZE)
#define CALI_BIN_OFF            (RTFS_BIN_OFF+RTFS_SIZE)

/* offset in FLASH */
#define BOOT_FLASH_OFF          BOOT_BIN_OFF
#define KERN_FLASH_OFF          KERN_BIN_OFF
#define RTFS_FLASH_OFF          RTFS_BIN_OFF
#define CALI_FLASH_OFF          CALI_BIN_OFF

#define NO_PROTECT              0
#define PROTECT_BOOT            1
#define PROTECT_KERN            2
#define PROTECT_RTFS            3


#define PID_MTDOFFSET         	(BOOT_SIZE - 0x50 + 1)
#define PRODID_MTDOFFSET     	(KERN_SIZE - 0x20)

#define FLASH_ADDR_BASE      0xbf000000

#ifndef NVRAM_SIZE
#define NVRAM_SIZE           0x10000
#endif

#ifndef NODE_ADDRESS
#define NODE_ADDRESS        (BOOT_SIZE - 0x50 - 0x10)
#endif
#ifndef PID_OFFSET
#define PID_OFFSET          (BOOT_SIZE - 0x50 + 1)
#endif
#ifndef DOMAIN_OFFSET       
#define DOMAIN_OFFSET       (BOOT_SIZE - 0x80)
#endif
#ifndef COUNTRY_OFFSET       
#define COUNTRY_OFFSET      (BOOT_SIZE - 0x80 + 1)
#endif
#ifndef HWVER_OFFSET       
#define HWVER_OFFSET        (BOOT_SIZE - 0x80 + 2)
#endif
#ifndef WPS_OFFSET       
#define WPS_OFFSET          (BOOT_SIZE - 0x80 + 8)
#endif

#ifndef SN_OFFSET       
#define SN_OFFSET           (BOOT_SIZE - 0x90)
#ifdef LINKSYS
#define SN_LENGTH           12
#else
#define SN_LENGTH           13 
#endif
#endif

int mtd_erase(int mtd_fd);
int flash_write(char *path, void *buf, int length);
int flash_read(char *path, void *buf, int length);

#endif

