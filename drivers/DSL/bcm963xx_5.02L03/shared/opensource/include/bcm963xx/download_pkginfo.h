/*
    Copyright 2000-2016 Broadcom Limited

<:label-BRCM:2016:DUAL/GPL:standard

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

#ifndef _DOWNLOAD_PKGINFO_H_
#define _DOWNLOAD_PKGINFO_H_

#define KEY_LEN                            128                /* good for up to sha512 */
#define PACKAGE_DIR_PREFIX                 "/pkg_"            /* pkg AKA  DU in tr157 */
#define APP_DIR_PREFIX                     "/app_"            /* app AKA  EU in tr157 */
#define LIB_DIR_PREFIX                     "/lib"
#define PACKAGE_BEEP_FILE_FREFIX           "pkg_beep_"
#define PACKAGE_BEEP_FILE_SUFFIX           ".tar.gz"
#define PACKAGE_BEEP_MANI_SUFFIX           ".manifest"

#endif // _DOWNLOAD_PKGINFO_H_
