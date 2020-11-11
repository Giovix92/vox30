#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
 *
 *  drivers/mtd/brcmnand/bcm7xxx-nand.c
 *
    <:copyright-BRCM:2011:DUAL/GPL:standard
    
       Copyright (c) 2011 Broadcom 
       All Rights Reserved
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <asm/io.h>
#include <bcm_map_part.h>
#include <board.h>
#include "brcmnand_priv.h"
#include <linux/slab.h>
#include <flash_api.h>
#ifdef __SC_BUILD__
#include <linux/mtd/partitions.h>
#endif
#define PRINTK(...)
//#define PRINTK printk

#define DRIVER_NAME     "brcmnand"
#define DRIVER_INFO     "Broadcom NAND controller"
#ifdef __SC_BUILD__
int g_scm_mtd_num = -1;
EXPORT_SYMBOL(g_scm_mtd_num);
#if SC_NAND_FLASH_SIZE == 256
#define CONFIG_SUPPORT_256_FLASH
#define FLASH_BLOCK_SIZE    (128 * 1024)
#define FLASH_PAGE_SIZE     (2048)
#elif SC_NAND_FLASH_SIZE == 128

#define FLASH_BLOCK_SIZE    (128 * 1024)
#define FLASH_PAGE_SIZE     (2048)

#elif SC_NAND_FLASH_SIZE == 64

#define FLASH_BLOCK_SIZE    (16 * 1024)
#define FLASH_PAGE_SIZE     (512)

#else

#error "SC_NAND_FLASH_SIZE not defined!!!!"

#endif
#endif
extern int setup_mtd_parts(struct mtd_info* mtd);

static int brcmnanddrv_probe(struct platform_device *pdev);
static int brcmnanddrv_remove(struct platform_device *pdev);

static struct mtd_partition bcm63XX_nand_parts[] = 
{
#ifdef __SC_BUILD__
    {name: "cferom",        offset: 0, size: 0},
    {name: "cferam1",       offset: 0, size: 0},
    {name: "cferam2",       offset: 0, size: 0},
    {name: "mmap",          offset: 0, size: 0},
    {name: "serial",        offset: 0, size: 0},
    {name: "protect",       offset: 0, size: 0},
    {name: "kfs1",          offset: 0, size: 0},
    {name: "kfs2",          offset: 0, size: 0},
    {name: "bootflag1",     offset: 0, size: 0},
#ifdef CONFIG_SUPPORT_256_FLASH
    {name: "package",       offset: 0, size: 0},
#else
    {name: "bootflag2",     offset: 0, size: 0},
#endif
    {name: "xml_cfg",       offset: 0, size: 0},
    {name: "app_dat",       offset: 0, size: 0},
    {name: "kfs1_lib",      offset: 0, size: 0},
    {name: "kfs2_lib",      offset: 0, size: 0},
#ifdef CONFIG_SUPPORT_256_FLASH
    {name: "kfs1_info",     offset: 0, size: 0},
    {name: "kfs2_info",     offset: 0, size: 0},
#endif
#else
    {name: "rootfs",        offset: 0, size: 0},
    {name: "rootfs_update", offset: 0, size: 0},
    {name: "data",          offset: 0, size: 0},
    {name: "nvram",         offset: 0, size: 0},
    {name: "image",         offset: 0, size: 0},
    {name: "image_update",  offset: 0, size: 0},
    {name: "dummy1",        offset: 0, size: 0},
    {name: "dummy2",        offset: 0, size: 0},
    {name: "dummy3",        offset: 0, size: 0},
    {name: "dummy4",        offset: 0, size: 0},
    {name: "dummy5",        offset: 0, size: 0},
    {name: "dummy6",        offset: 0, size: 0},
#endif
    {name: NULL,            offset: 0, size: 0}

};

static struct platform_driver brcmnand_platform_driver =
{
	.probe		= brcmnanddrv_probe,
	.remove		= brcmnanddrv_remove,
	.driver		=
	{
		.name	= DRIVER_NAME,
	},
};

static struct resource brcmnand_resources[] =
{
	[0] = {
		.name	= DRIVER_NAME,
		.flags	= IORESOURCE_MEM,
	},
};

static struct brcmnand_info {
	struct mtd_info mtd;
	struct brcmnand_chip brcmnand;
	int nr_parts;
	struct mtd_partition* parts;
} *gNandInfo[NUM_NAND_CS];

int gNandCS[NAND_MAX_CS];
/* Number of NAND chips, only applicable to v1.0+ NAND controller */
int gNumNand   = 0;
int gClearBBT  = 0;
char gClearCET = 0;
uint32_t gNandTiming1[NAND_MAX_CS], gNandTiming2[NAND_MAX_CS];
uint32_t gAccControl[NAND_MAX_CS],  gNandConfig[NAND_MAX_CS];

static unsigned long t1[NAND_MAX_CS] = { 0 };
static int nt1 = 0;
static unsigned long t2[NAND_MAX_CS] = { 0 };
static int nt2 = 0;
static unsigned long acc[NAND_MAX_CS] = { 0 };
static int nacc = 0;
static unsigned long nandcfg[NAND_MAX_CS] = { 0 };
static int ncfg = 0;
static void* gPageBuffer = NULL;
#ifdef __SC_BUILD__
#if 0
static unsigned long sc_mtd_info_addr = 0;
static int __init
set_sc_mtd_info_addr(char *str)
{
        int int_addr;
        get_option(&str, &int_addr);

        sc_mtd_info_addr = (unsigned long)int_addr;

        return 1;
}

__setup("sc_mtd_info_addr=", set_sc_mtd_info_addr);

#endif

struct _sc_mmap_info {
    __u32   i;
    __u32   b;
    __u32   l;
};

struct _sc_mmap {
    struct _sc_mmap_info cfe_rom;
    struct _sc_mmap_info mtd_map;
    struct _sc_mmap_info cferam1;
    struct _sc_mmap_info cferam2;
    struct _sc_mmap_info sn;
    struct _sc_mmap_info prot;
#ifdef CONFIG_SUPPORT_256_FLASH
    struct _sc_mmap_info kfs1_info;
    struct _sc_mmap_info kfs2_info;
#endif
    struct _sc_mmap_info kfs1;
    struct _sc_mmap_info kfs1_lib;
    struct _sc_mmap_info kfs2;
    struct _sc_mmap_info kfs2_lib;
    struct _sc_mmap_info bf1;
#ifndef CONFIG_SUPPORT_256_FLASH
    struct _sc_mmap_info bf2;
#endif
    struct _sc_mmap_info xml_cfg;
    struct _sc_mmap_info app_dat;
#ifdef CONFIG_SUPPORT_256_FLASH
    struct _sc_mmap_info package;
#endif
};


static void 
sc_set_running_rootfs(struct brcmnand_info *nandinfo, struct _sc_mmap *mmap)
{
        struct mtd_info *mtd = &nandinfo->mtd;
        size_t retlen;
        loff_t oft = 0x0;
        __u8 bf1[16] = {0};
#ifndef CONFIG_SUPPORT_256_FLASH
        __u8 bf2[16] = {0};
#else
        unsigned int flag = 0;
#endif
        /*
         * Read bootflag1
         */
        oft = mmap->bf1.b;
        while(mtd->_block_isbad(mtd, oft)) {
                printk("!!! oft = 0x%llx is in bad block.\n", oft);
                oft += FLASH_BLOCK_SIZE;
                /*
                 * TODO
                 * check out of boundary
                 */
        }

        if(mtd->_read(mtd, oft, sizeof(bf1), &retlen, bf1) < 0) {
                printk("%s read mtd failed, oft = 0x%llx\n",
                       __FUNCTION__, oft);
                return;
        }

#ifndef CONFIG_SUPPORT_256_FLASH
        /*
         * Read bootflag2
         */
        oft = mmap->bf2.b;
        while(mtd->_block_isbad(mtd, oft)) {
                printk("!!! oft = 0x%llx is in bad block.\n", oft);
                oft += FLASH_BLOCK_SIZE;
                /*
                 * TODO
                 * check out of boundary
                 */
        }

        if(mtd->_read(mtd, oft, sizeof(bf2), &retlen, bf2) < 0) {
                printk("%s read mtd failed, oft = 0x%llx\n",
                       __FUNCTION__, oft);
                return;
        }

        if(memcmp(bf1, bf2, sizeof(bf1)) < 0) {
                bcm63XX_nand_parts[6].name = "rootfs_inactive";
                bcm63XX_nand_parts[7].name = "rootfs";
                g_scm_mtd_num = 13;
                     } else {
                bcm63XX_nand_parts[6].name = "rootfs";
                bcm63XX_nand_parts[7].name = "rootfs_inactive";
                g_scm_mtd_num = 12;
                       }
#else
        flag = simple_strtoul(bf1+7, NULL, 10);  //skip the magic
        if(flag % 2)
        {
            bcm63XX_nand_parts[6].name = "rootfs_inactive";
            bcm63XX_nand_parts[7].name = "rootfs";
            g_scm_mtd_num = 13;
        } else {
            bcm63XX_nand_parts[6].name = "rootfs";
            bcm63XX_nand_parts[7].name = "rootfs_inactive";
            g_scm_mtd_num = 12;
        }
#endif
        return;
}

static void 
sc_setup_mtd_partitions(struct brcmnand_info *nandinfo)
{
        struct mtd_info *mtd = &nandinfo->mtd;
        size_t retlen;
        static __u8 buf[FLASH_PAGE_SIZE];
        __u8 mmap_flag_found = 0;
        struct _sc_mmap *sc_mtd_info = (struct _sc_mmap *)buf;
        loff_t oft = FLASH_BLOCK_SIZE;


        while(1) {

                while(mtd->_block_isbad(mtd, oft)) {
                        oft += FLASH_BLOCK_SIZE;
                        if(unlikely(oft >= mtd->size)) {
                                printk("%s read beyond device size = %llu\n",
                                       __FUNCTION__, mtd->size);
                                return;
                        }
                }
                if(mtd->_read(mtd, oft, sizeof(buf), &retlen, buf) < 0) {
                        printk("%s read mtd failed, oft = 0x%llx\n",
                               __FUNCTION__, oft);
                        return;
                }
                if(!mmap_flag_found) {
                        if(memcmp(&buf, "SCFLMAPOK", 9) == 0) {
                                mmap_flag_found = 1;
                                oft += FLASH_PAGE_SIZE; /* data saved in next page */
                                continue;
                        }
                        /*
                         * Magic not there, read next block.
                         */
                        oft += FLASH_BLOCK_SIZE;
                } else {
                        break;
                }
        }
#ifdef CONFIG_SUPPORT_256_FLASH
        nandinfo->nr_parts = 16;
#else
        nandinfo->nr_parts = 14;
#endif
        bcm63XX_nand_parts[0].offset     = 0x0;
        bcm63XX_nand_parts[0].size       = sc_mtd_info->cfe_rom.l;
        bcm63XX_nand_parts[0].ecclayout  = mtd->ecclayout;
        bcm63XX_nand_parts[1].offset     = sc_mtd_info->cferam1.b;
        bcm63XX_nand_parts[1].size       = sc_mtd_info->cferam1.l;
        bcm63XX_nand_parts[1].ecclayout  = mtd->ecclayout;

        bcm63XX_nand_parts[2].offset     = sc_mtd_info->cferam2.b ;
        bcm63XX_nand_parts[2].size       = sc_mtd_info->cferam2.l ;
        bcm63XX_nand_parts[2].ecclayout  = mtd->ecclayout;
        bcm63XX_nand_parts[3].offset     = sc_mtd_info->mtd_map.b ;
        bcm63XX_nand_parts[3].size       = sc_mtd_info->mtd_map.l;
        bcm63XX_nand_parts[3].ecclayout  = mtd->ecclayout;

        bcm63XX_nand_parts[4].offset     = sc_mtd_info->sn.b ;
        bcm63XX_nand_parts[4].size       = sc_mtd_info->sn.l ;
        bcm63XX_nand_parts[4].ecclayout  = mtd->ecclayout;

        bcm63XX_nand_parts[5].offset     = sc_mtd_info->prot.b ;
        bcm63XX_nand_parts[5].size       = sc_mtd_info->prot.l ;
        bcm63XX_nand_parts[5].ecclayout  = mtd->ecclayout;
        bcm63XX_nand_parts[6].offset     = sc_mtd_info->kfs1.b ;
        bcm63XX_nand_parts[6].size       = sc_mtd_info->kfs1.l ;
        bcm63XX_nand_parts[6].ecclayout  = mtd->ecclayout;

        bcm63XX_nand_parts[7].offset     = sc_mtd_info->kfs2.b ;
        bcm63XX_nand_parts[7].size       = sc_mtd_info->kfs2.l ;
        bcm63XX_nand_parts[7].ecclayout  = mtd->ecclayout;

        bcm63XX_nand_parts[8].offset     = sc_mtd_info->bf1.b ;
        bcm63XX_nand_parts[8].size       = sc_mtd_info->bf1.l ;
        bcm63XX_nand_parts[8].ecclayout  = mtd->ecclayout;

#ifdef CONFIG_SUPPORT_256_FLASH
        bcm63XX_nand_parts[9].offset     = sc_mtd_info->package.b ;
        bcm63XX_nand_parts[9].size       = sc_mtd_info->package.l ;
        bcm63XX_nand_parts[9].ecclayout  = mtd->ecclayout;
#else
        bcm63XX_nand_parts[9].offset     = sc_mtd_info->bf2.b ;
        bcm63XX_nand_parts[9].size       = sc_mtd_info->bf2.l ;
        bcm63XX_nand_parts[9].ecclayout  = mtd->ecclayout;
#endif
        bcm63XX_nand_parts[10].offset     = sc_mtd_info->xml_cfg.b ;
        bcm63XX_nand_parts[10].size       = sc_mtd_info->xml_cfg.l ;
        bcm63XX_nand_parts[10].ecclayout  = mtd->ecclayout;
        bcm63XX_nand_parts[11].offset     = sc_mtd_info->app_dat.b ;
        bcm63XX_nand_parts[11].size       = sc_mtd_info->app_dat.l ;
        bcm63XX_nand_parts[11].ecclayout  = mtd->ecclayout;

        bcm63XX_nand_parts[12].offset     = sc_mtd_info->kfs1_lib.b ;
        bcm63XX_nand_parts[12].size       = sc_mtd_info->kfs1_lib.l ;
        bcm63XX_nand_parts[12].ecclayout  = mtd->ecclayout;
        bcm63XX_nand_parts[13].offset     = sc_mtd_info->kfs2_lib.b ;
        bcm63XX_nand_parts[13].size       = sc_mtd_info->kfs2_lib.l ;
        bcm63XX_nand_parts[13].ecclayout  = mtd->ecclayout;
#ifdef CONFIG_SUPPORT_256_FLASH
        bcm63XX_nand_parts[14].offset     = sc_mtd_info->kfs1_info.b ;
        bcm63XX_nand_parts[14].size       = sc_mtd_info->kfs1_info.l ;
        bcm63XX_nand_parts[14].ecclayout  = mtd->ecclayout;
        bcm63XX_nand_parts[15].offset     = sc_mtd_info->kfs2_info.b ;
        bcm63XX_nand_parts[15].size       = sc_mtd_info->kfs2_info.l ;
        bcm63XX_nand_parts[15].ecclayout  = mtd->ecclayout;
#endif
        sc_set_running_rootfs(nandinfo, sc_mtd_info);
        return;
}
#endif
#ifdef __SC_BUILD__
static void 
brcmnanddrv_setup_mtd_partitions(struct brcmnand_info* nandinfo)
{
    int boot_from_nand;
    if (flash_get_flash_type() == FLASH_IFC_NAND)
        boot_from_nand = 1;
    else
        boot_from_nand = 0;

    if( boot_from_nand == 0 )
    {
        nandinfo->nr_parts = 1;
        nandinfo->parts = bcm63XX_nand_parts;

        bcm63XX_nand_parts[0].name = "data";
        bcm63XX_nand_parts[0].offset = 0;
        if( device_size(&(nandinfo->mtd)) < NAND_BBT_THRESHOLD_KB )
        {
            bcm63XX_nand_parts[0].size =
                device_size(&(nandinfo->mtd)) - (NAND_BBT_SMALL_SIZE_KB*1024);
        }
        else
        {
            bcm63XX_nand_parts[0].size =
                device_size(&(nandinfo->mtd)) - (NAND_BBT_BIG_SIZE_KB*1024);
        }
        bcm63XX_nand_parts[0].ecclayout = nandinfo->mtd.ecclayout;

        PRINTK("Part[0] name=%s, size=%llx, ofs=%llx\n", bcm63XX_nand_parts[0].name,
                bcm63XX_nand_parts[0].size, bcm63XX_nand_parts[0].offset);
    }
    else
    {
        static NVRAM_DATA nvram;
        unsigned long rootfs_ofs;
        int rootfs, rootfs_update;
        kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs);
        kerSysNvRamGet((char *)&nvram, sizeof(nvram), 0);
        nandinfo->nr_parts = 6;
        nandinfo->parts = bcm63XX_nand_parts;

        /* Root FS.  The CFE RAM boot loader saved the rootfs offset that the
         * Linux image was loaded from.
         */
        rootfs = NP_ROOTFS_2;
        rootfs_update = NP_ROOTFS_1;//cferam
        sc_setup_mtd_partitions(nandinfo);
    }
}

#endif
static int brcmnanddrv_probe(struct platform_device *pdev)
{
	static int csi = 0;     // Index into dev/nandInfo array
	int cs = 0;             // Chip Select
	int err = 0;
	struct brcmnand_info* info = NULL;
	static struct brcmnand_ctrl* ctrl = (struct brcmnand_ctrl*)0;
	if (!gPageBuffer && (gPageBuffer = kmalloc(sizeof(struct brcmnand_buffers), GFP_KERNEL)) == NULL) {
		return -ENOMEM;
	}

	if ( (ctrl = kmalloc(sizeof(struct brcmnand_ctrl), GFP_KERNEL)) != NULL) {
		memset(ctrl, 0, sizeof(struct brcmnand_ctrl));
		ctrl->state = FL_READY;
		init_waitqueue_head(&ctrl->wq);
		spin_lock_init(&ctrl->chip_lock);

		if ((info = kmalloc(sizeof(struct brcmnand_info), GFP_KERNEL)) != NULL) {
			gNandInfo[csi] = info;
			memset(info, 0, sizeof(struct brcmnand_info));
			info->brcmnand.ctrl = ctrl;
			info->brcmnand.ctrl->numchips = gNumNand = 1;
			info->brcmnand.csi = csi;

			/* For now all devices share the same buffer */
			info->brcmnand.ctrl->buffers = (struct brcmnand_buffers*)gPageBuffer;

			info->brcmnand.ctrl->numchips = gNumNand;
			info->brcmnand.chip_shift = 0; // Only 1 chip
			info->brcmnand.priv = &info->mtd;
			info->mtd.name = dev_name(&pdev->dev);
			info->mtd.priv = &info->brcmnand;
			info->mtd.owner = THIS_MODULE;

			/* Enable the following for a flash based bad block table */
			info->brcmnand.options |= NAND_BBT_USE_FLASH;

			/* Each chip now will have its own BBT (per mtd handle) */
			if (brcmnand_scan(&info->mtd, cs, gNumNand) == 0) {
				PRINTK("Master size=%08llx\n", info->mtd.size);
#ifdef __SC_BUILD__
                brcmnanddrv_setup_mtd_partitions(info);
                mtd_device_register(&info->mtd, info->parts, info->nr_parts);
#else
				setup_mtd_parts(&info->mtd);
#endif
				dev_set_drvdata(&pdev->dev, info);
			}else
				err = -ENXIO;
		}else
			err = -ENOMEM;
	}else
		err = -ENOMEM;

	if (err) {
		if (gPageBuffer) {
			kfree(gPageBuffer);
			gPageBuffer = NULL;
		}

		if (ctrl) {
			kfree(ctrl);
			ctrl = NULL;
		}

		if (info) {
			kfree(info);
			info = NULL;
		}
	}

	return err;
}

static int brcmnanddrv_remove(struct platform_device *pdev)
{
	struct brcmnand_info *info = dev_get_drvdata(&pdev->dev);

	dev_set_drvdata(&pdev->dev, NULL);

	if (info) {
		mtd_device_unregister(&info->mtd);

		brcmnand_release(&info->mtd);
		kfree(gPageBuffer);
		kfree(info);
	}

	return 0;
}

static int __init brcmnanddrv_init(void)
{
	int ret = 0;
	int csi;
	int ncsi;
	char cmd[32] = "\0";
	struct platform_device *pdev;

	if (flash_get_flash_type() != FLASH_IFC_NAND)
		return -ENODEV;

	kerSysBlParmsGetStr(NAND_COMMAND_NAME, cmd, sizeof(cmd));
	PRINTK("%s: brcmnanddrv_init - NANDCMD='%s'\n", __FUNCTION__, cmd);

	if (cmd[0]) {
		if (strcmp(cmd, "rescan") == 0)
			gClearBBT = 1;
		else if (strcmp(cmd, "showbbt") == 0)
			gClearBBT = 2;
		else if (strcmp(cmd, "eraseall") == 0)
			gClearBBT = 8;
		else if (strcmp(cmd, "erase") == 0)
			gClearBBT = 7;
		else if (strcmp(cmd, "clearbbt") == 0)
			gClearBBT = 9;
		else if (strcmp(cmd, "showcet") == 0)
			gClearCET = 1;
		else if (strcmp(cmd, "resetcet") == 0)
			gClearCET = 2;
		else if (strcmp(cmd, "disablecet") == 0)
			gClearCET = 3;
		else
			printk(KERN_WARNING "%s: unknown command '%s'\n",
			       __FUNCTION__, cmd);
	}

	for (csi = 0; csi < NAND_MAX_CS; csi++) {
		gNandTiming1[csi] = 0;
		gNandTiming2[csi] = 0;
		gAccControl[csi]  = 0;
		gNandConfig[csi]  = 0;
	}

	if (nacc == 1)
		PRINTK("%s: nacc=%d, gAccControl[0]=%08lx, gNandConfig[0]=%08lx\n",
		       __FUNCTION__, nacc, acc[0], nandcfg[0]);

	if (nacc > 1)
		PRINTK("%s: nacc=%d, gAccControl[1]=%08lx, gNandConfig[1]=%08lx\n",
		       __FUNCTION__, nacc, acc[1], nandcfg[1]);

	for (csi = 0; csi < nacc; csi++)
		gAccControl[csi] = acc[csi];

	for (csi = 0; csi < ncfg; csi++)
		gNandConfig[csi] = nandcfg[csi];

	ncsi = max(nt1, nt2);

	for (csi = 0; csi < ncsi; csi++) {
		if (nt1 && csi < nt1)
			gNandTiming1[csi] = t1[csi];

		if (nt2 && csi < nt2)
			gNandTiming2[csi] = t2[csi];

	}

	printk(KERN_INFO DRIVER_INFO " (BrcmNand Controller)\n");

	if ( (pdev = platform_device_alloc(DRIVER_NAME, 0)) != NULL ) {
		platform_device_add(pdev);
		platform_device_put(pdev);
		ret = platform_driver_register(&brcmnand_platform_driver);

		brcmnand_resources[0].start = BPHYSADDR(BCHP_NAND_REG_START);
		brcmnand_resources[0].end = BPHYSADDR(BCHP_NAND_REG_END) + 3;

		if (ret >= 0)
			request_resource(&iomem_resource, &brcmnand_resources[0]);
		else
			printk("brcmnanddrv_init: driver_register failed, err=%d\n", ret);
	}else
		ret = -ENODEV;
	return ret;
}

static void __exit brcmnanddrv_exit(void)
{
	release_resource(&brcmnand_resources[0]);
	platform_driver_unregister(&brcmnand_platform_driver);
}

module_init(brcmnanddrv_init);
module_exit(brcmnanddrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ton Truong <ttruong@broadcom.com>");
MODULE_DESCRIPTION("Broadcom NAND flash driver");

#endif //CONFIG_BCM_KF_MTD_BCMNAND
