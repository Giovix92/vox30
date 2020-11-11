/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#else
#include "lib_printf.h"
#endif

#ifndef _CFE_
#define PRINTK  printk
#else
#define PRINTK  xprintf
#endif
#include "pmc_drv.h"
#include "pmc_xrdp.h"
#include "BPCM.h"
#include "clk_rst.h"
#include "bcm_map_part.h"

int pmc_xrdp_init(void)
{

    int status = 0;
#if defined(_BCM96858_) || defined(CONFIG_BCM96858)

#if 0
	status =  pll_ch_freq_set(PMB_ADDR_RDPPLL, 1, 2);
	if (status)
	{
		PRINTK("UNIPLL Channel 2 Mdiv=5 failed\n");
		return status;
	}

	status =  pll_ch_freq_set(PMB_ADDR_RDPPLL, 2, 2);
	if (status)
	{
	 PRINTK("UNIPLL Channel 2 Mdiv=5 failed\n");
	 return status;
	}

	status =  pll_ch_freq_set(PMB_ADDR_RDPPLL, 3, 6);
	if (status)
	{
	 PRINTK("UNIPLL Channel 2 Mdiv=5 failed\n");
	 return status;
	}
#endif
    status =  PowerOnDevice(PMB_ADDR_XRDP);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_QM);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_QM\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC_QUAD0);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC_QUAD0\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC_QUAD1);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC_QUAD1\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC_QUAD2);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC_QUAD2\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC_QUAD3);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC_QUAD3\n");
        return status;
    }
#elif defined(_BCM96836_) || defined(CONFIG_BCM96836)
    status =  PowerOnDevice(PMB_ADDR_XRDP);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP status[%d]\n",status);
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC0);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC0\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC1);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC1\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC2);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC2\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC3);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC3\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC4);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC4\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC5);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC5\n");
        return status;
    }

    status = PowerOnDevice(PMB_ADDR_WAN);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_WAN\n");
        return status;
    }
#else
    PRINTK("%s is not implemented in platform yet!\n", __FUNCTION__);
#endif

    return status;
}

int pmc_xrdp_shutdown(void)
{
    int status = 0;
#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
    status =  PowerOffDevice(PMB_ADDR_XRDP, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_QM, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_QM\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_RC_QUAD0, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_RC_QUAD0\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_RC_QUAD1, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_RC_QUAD1\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_RC_QUAD2, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_RC_QUAD2\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_RC_QUAD3, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_RC_QUAD3\n");
       return status;
    }
#else
    PRINTK("%s is not implemented in platform yet!\n", __FUNCTION__);
#endif
    return status;
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_xrdp_init);
postcore_initcall(pmc_xrdp_init);
#endif
