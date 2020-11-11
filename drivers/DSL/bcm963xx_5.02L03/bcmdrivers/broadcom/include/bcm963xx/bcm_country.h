/****************************************************************************
 *
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
 *
 ****************************************************************************/
#ifndef _BCM_COUNTRY_H
#define _BCM_COUNTRY_H

#define COUNTRY_ARCHIVE_MAKE_NAME(country)   VRG_COUNTRY_##country,

#ifdef __SC_BUILD__
#ifndef VRG_COUNTRY_CFG_AUSTRALIA
#define VRG_COUNTRY_CFG_AUSTRALIA      1
#endif

#ifndef VRG_COUNTRY_CFG_AUSTRIA
#define VRG_COUNTRY_CFG_AUSTRIA        0
#endif

#ifndef VRG_COUNTRY_CFG_BELGIUM
#define VRG_COUNTRY_CFG_BELGIUM        1
#endif

#ifndef VRG_COUNTRY_CFG_BRAZIL
#define VRG_COUNTRY_CFG_BRAZIL         1
#endif

#ifndef VRG_COUNTRY_CFG_CHILE
#define VRG_COUNTRY_CFG_CHILE          1
#endif

#ifndef VRG_COUNTRY_CFG_CHINA
#define VRG_COUNTRY_CFG_CHINA          1
#endif

#ifndef VRG_COUNTRY_CFG_CZECH 
#define VRG_COUNTRY_CFG_CZECH          1
#endif

#ifndef VRG_COUNTRY_CFG_DENMARK
#define VRG_COUNTRY_CFG_DENMARK        1
#endif

#ifndef VRG_COUNTRY_CFG_ETSI
#define VRG_COUNTRY_CFG_ETSI           1
#endif

#ifndef VRG_COUNTRY_CFG_FINLAND
#define VRG_COUNTRY_CFG_FINLAND        1
#endif

#ifndef VRG_COUNTRY_CFG_FRANCE
#define VRG_COUNTRY_CFG_FRANCE         1
#endif

#ifndef VRG_COUNTRY_CFG_GERMANY
#define VRG_COUNTRY_CFG_GERMANY        1
#endif

#ifndef VRG_COUNTRY_CFG_HUNGARY
#define VRG_COUNTRY_CFG_HUNGARY        1
#endif

#ifndef VRG_COUNTRY_CFG_INDIA
#define VRG_COUNTRY_CFG_INDIA          1
#endif

#ifndef VRG_COUNTRY_CFG_IRELAND
#define VRG_COUNTRY_CFG_IRELAND        1
#endif

#ifndef VRG_COUNTRY_CFG_ITALY
#define VRG_COUNTRY_CFG_ITALY          1
#endif

#ifndef VRG_COUNTRY_CFG_JAPAN
#define VRG_COUNTRY_CFG_JAPAN          1
#endif

#ifndef VRG_COUNTRY_CFG_NETHERLANDS
#define VRG_COUNTRY_CFG_NETHERLANDS    1
#endif

#ifndef VRG_COUNTRY_CFG_NEW_ZEALAND
#define VRG_COUNTRY_CFG_NEW_ZEALAND    1
#endif

#ifndef VRG_COUNTRY_CFG_NORTH_AMERICA
#define VRG_COUNTRY_CFG_NORTH_AMERICA  1
#endif

#ifndef VRG_COUNTRY_CFG_POLAND
#define VRG_COUNTRY_CFG_POLAND         1
#endif

#ifndef VRG_COUNTRY_CFG_ROMANIA
#define VRG_COUNTRY_CFG_ROMANIA        0
#endif

#ifndef VRG_COUNTRY_CFG_SLOVAKIA
#define VRG_COUNTRY_CFG_SLOVAKIA       0
#endif

#ifndef VRG_COUNTRY_CFG_SLOVENIA
#define VRG_COUNTRY_CFG_SLOVENIA       0
#endif

#ifndef VRG_COUNTRY_CFG_SPAIN
#define VRG_COUNTRY_CFG_SPAIN          1
#endif

#ifndef VRG_COUNTRY_CFG_SWEDEN
#define VRG_COUNTRY_CFG_SWEDEN         1
#endif

#ifndef VRG_COUNTRY_CFG_NORWAY
#define VRG_COUNTRY_CFG_NORWAY         1
#endif

#ifndef VRG_COUNTRY_CFG_SWITZERLAND
#define VRG_COUNTRY_CFG_SWITZERLAND    1
#endif

#ifndef VRG_COUNTRY_CFG_TR57
#define VRG_COUNTRY_CFG_TR57           1
#endif

#ifndef VRG_COUNTRY_CFG_UK
#define VRG_COUNTRY_CFG_UK             1
#endif

#ifndef VRG_COUNTRY_CFG_TAIWAN
#define VRG_COUNTRY_CFG_TAIWAN             1
#endif

#ifndef VRG_COUNTRY_CFG_UNITED_ARAB_EMIRATES
#define VRG_COUNTRY_CFG_UNITED_ARAB_EMIRATES             1
#endif

#ifndef VRG_COUNTRY_CFG_MEXICO
#define VRG_COUNTRY_CFG_MEXICO             1
#endif

#ifndef VRG_COUNTRY_CFG_CYPRUS
#define VRG_COUNTRY_CFG_CYPRUS         1
#endif
#endif

typedef enum
{

#ifndef __SC_BUILD__
   #include <countryArchive.h>
#endif
#ifdef __SC_BUILD__
#ifdef VRG_COUNTRY_CFG_AUSTRALIA
     COUNTRY_ARCHIVE_MAKE_NAME( AUSTRALIA )
#endif
#ifdef VRG_COUNTRY_CFG_AUSTRIA
     COUNTRY_ARCHIVE_MAKE_NAME( AUSTRIA )
#endif
#ifdef VRG_COUNTRY_CFG_BELGIUM
     COUNTRY_ARCHIVE_MAKE_NAME( BELGIUM )
#endif
#ifdef VRG_COUNTRY_CFG_BRAZIL
     COUNTRY_ARCHIVE_MAKE_NAME( BRAZIL )
#endif
#ifdef VRG_COUNTRY_CFG_CHILE
     COUNTRY_ARCHIVE_MAKE_NAME( CHILE )
#endif
#ifdef VRG_COUNTRY_CFG_CHINA
     COUNTRY_ARCHIVE_MAKE_NAME( CHINA )
#endif
#ifdef VRG_COUNTRY_CFG_CYPRUS
     COUNTRY_ARCHIVE_MAKE_NAME( CYPRUS )
#endif
#ifdef VRG_COUNTRY_CFG_CZECH
     COUNTRY_ARCHIVE_MAKE_NAME( CZECH )
#endif
#ifdef VRG_COUNTRY_CFG_DENMARK
     COUNTRY_ARCHIVE_MAKE_NAME( DENMARK )
#endif
#ifdef VRG_COUNTRY_CFG_ETSI
     COUNTRY_ARCHIVE_MAKE_NAME( ETSI )
#endif
#ifdef VRG_COUNTRY_CFG_FINLAND
     COUNTRY_ARCHIVE_MAKE_NAME( FINLAND )
#endif
#ifdef VRG_COUNTRY_CFG_FRANCE
     COUNTRY_ARCHIVE_MAKE_NAME( FRANCE )
#endif
#ifdef VRG_COUNTRY_CFG_GERMANY
     COUNTRY_ARCHIVE_MAKE_NAME( GERMANY )
#endif
#ifdef VRG_COUNTRY_CFG_HUNGARY
     COUNTRY_ARCHIVE_MAKE_NAME( HUNGARY )
#endif
#ifdef VRG_COUNTRY_CFG_INDIA
     COUNTRY_ARCHIVE_MAKE_NAME( INDIA )
#endif
#ifdef VRG_COUNTRY_CFG_IRELAND
     COUNTRY_ARCHIVE_MAKE_NAME( IRELAND )
#endif
#ifdef VRG_COUNTRY_CFG_ITALY
     COUNTRY_ARCHIVE_MAKE_NAME( ITALY )
#endif
#ifdef VRG_COUNTRY_CFG_JAPAN
     COUNTRY_ARCHIVE_MAKE_NAME( JAPAN )
#endif
#ifdef VRG_COUNTRY_CFG_MEXICO
     COUNTRY_ARCHIVE_MAKE_NAME( MEXICO )
#endif
#ifdef VRG_COUNTRY_CFG_NETHERLANDS
     COUNTRY_ARCHIVE_MAKE_NAME( NETHERLANDS )
#endif
#ifdef VRG_COUNTRY_CFG_NEW_ZEALAND
     COUNTRY_ARCHIVE_MAKE_NAME( NEW_ZEALAND )
#endif
#ifdef VRG_COUNTRY_CFG_NORTH_AMERICA
     COUNTRY_ARCHIVE_MAKE_NAME( NORTH_AMERICA )
#endif
#ifdef VRG_COUNTRY_CFG_NORWAY
     COUNTRY_ARCHIVE_MAKE_NAME( NORWAY )
#endif
#ifdef VRG_COUNTRY_CFG_POLAND
     COUNTRY_ARCHIVE_MAKE_NAME( POLAND )
#endif
#ifdef VRG_COUNTRY_CFG_ROMANIA
     COUNTRY_ARCHIVE_MAKE_NAME( ROMANIA )
#endif
#ifdef VRG_COUNTRY_CFG_SLOVAKIA
     COUNTRY_ARCHIVE_MAKE_NAME( SLOVAKIA )
#endif
#ifdef VRG_COUNTRY_CFG_SLOVENIA
     COUNTRY_ARCHIVE_MAKE_NAME( SLOVENIA )
#endif
#ifdef VRG_COUNTRY_CFG_SPAIN
     COUNTRY_ARCHIVE_MAKE_NAME( SPAIN )
#endif
#ifdef VRG_COUNTRY_CFG_SWEDEN
     COUNTRY_ARCHIVE_MAKE_NAME( SWEDEN )
#endif
#ifdef VRG_COUNTRY_CFG_SWITZERLAND
     COUNTRY_ARCHIVE_MAKE_NAME( SWITZERLAND )
#endif
#ifdef VRG_COUNTRY_CFG_TAIWAN
     COUNTRY_ARCHIVE_MAKE_NAME( TAIWAN )
#endif
#ifdef VRG_COUNTRY_CFG_TR57
     COUNTRY_ARCHIVE_MAKE_NAME( TR57 )
#endif
#ifdef VRG_COUNTRY_CFG_UK
     COUNTRY_ARCHIVE_MAKE_NAME( UK )
#endif
#ifdef VRG_COUNTRY_CFG_UNITED_ARAB_EMIRATES
     COUNTRY_ARCHIVE_MAKE_NAME( UNITED_ARAB_EMIRATES )
#endif
#endif

   VRG_COUNTRY_MAX

} VRG_COUNTRY;

typedef struct
{
   /* current country code based on nvol settings */
   VRG_COUNTRY    country;
   int swap_two_fxs ;

} ENDPT2_SLIC_CFG;

#endif /* _BCM_COUNTRY_H */

