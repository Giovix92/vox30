/****************************************************************************
 *
 * <:copyright-BRCM:2014:proprietary:standard
 * 
 *    Copyright (c) 2014 Broadcom 
 *    All Rights Reserved
 * 
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 * 
 *  Except as expressly set forth in the Authorized License,
 * 
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 * 
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 * 
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 ****************************************************************************/
#ifndef _BCM_SLICSLAC_H
#define _BCM_SLICSLAC_H

#include <bcm_country.h>

/****************************************************************************
* Typedefs and Constants
****************************************************************************/
#define SSDEV_NAME         "slicslac"
#define SSDEV_MAJOR        3050
#define SSDEV_MINOR        1

#define SSDEV_KFIFO_SIZE   128 /* Must be a power of 2 */

#ifdef __SC_BUILD__
#define SS_CMD_INIT                   _IOW (SSDEV_MAJOR,  0, long)
#else
#define SS_CMD_INIT                   _IOW (SSDEV_MAJOR,  0, VRG_COUNTRY)
#endif
#define SS_CMD_DEINIT                 _IO  (SSDEV_MAJOR,  1)
#define SS_CMD_SETLOGLEVEL            _IOW (SSDEV_MAJOR,  2, unsigned int)
#define SS_CMD_GETDLP                 _IOW (SSDEV_MAJOR,  3, unsigned int)
#define SS_CMD_GETELP                 _IOW (SSDEV_MAJOR,  4, unsigned int)
#define SS_CMD_GETLOOPCURRENT         _IOW (SSDEV_MAJOR,  5, unsigned int)
#define SS_CMD_GETOVERCURRENTSTATUS   _IOW (SSDEV_MAJOR,  6, unsigned int)
#define SS_CMD_GETRINGPARMS           _IOWR(SSDEV_MAJOR,  7, SSARG_RINGPARMS)
#define SS_CMD_GETSLICPARMS           _IOWR(SSDEV_MAJOR,  8, SSARG_SLICPARMS)
#define SS_CMD_ISOFFHOOK              _IOW (SSDEV_MAJOR,  9, unsigned int)
#define SS_CMD_ISRINGACTIVE           _IOW (SSDEV_MAJOR, 10, unsigned int)
#define SS_CMD_LEDCONTROL             _IOW (SSDEV_MAJOR, 11, SSARG_LEDCONTROL)
#define SS_CMD_MODECONTROL            _IOW (SSDEV_MAJOR, 12, SSARG_MODECONTROL)
#define SS_CMD_PHREVCONTROL           _IOW (SSDEV_MAJOR, 13, SSARG_PHREVCONTROL)
#define SS_CMD_PROCESSEVENTS          _IOW (SSDEV_MAJOR, 14, unsigned int)
#define SS_CMD_SETBOOSTEDLOOPCURRENT  _IOW (SSDEV_MAJOR, 15, SSARG_SETBOOSTEDLOOPCURRENT)
#define SS_CMD_SETFASTSLICMODE        _IOW (SSDEV_MAJOR, 16, SSARG_SETFASTSLICMODE)
#define SS_CMD_SETPOWERSOURCE         _IOW (SSDEV_MAJOR, 17, SSARG_SETPOWERSOURCE)
#define SS_CMD_SETRINGPARMS           _IOW (SSDEV_MAJOR, 18, SSARG_RINGPARMS)
#define SS_CMD_SETSLICENABLE          _IOW (SSDEV_MAJOR, 19, SSARG_SETSLICENABLE)
#define SS_CMD_APM_HPFCONTROL         _IOW (SSDEV_MAJOR, 20, SSARG_APM_HPFCONTROL)
#define SS_CMD_APM_PULSEMETERING      _IOW (SSDEV_MAJOR, 21, SSARG_APM_PULSEMETERING)
#define SS_CMD_APM_UPDATEHOOKSTATUS   _IOW (SSDEV_MAJOR, 22, unsigned int)
#define SS_CMD_LINE_TEST              _IOW (SSDEV_MAJOR, 23, SSARG_LINE_TEST)


#define __compat  __attribute__ ((aligned(4),packed))

/* This enum defines the message the driver can report back to the host
 * application. */
enum ssevtid
{
   SS_EVT_NONE,
   SS_EVT_LINE_TEST_RESULTS,
};

/* This is the structure of data the driver passes back to the host whenever
 * there is an event. */
struct ssevt
{
   enum ssevtid   id;   /* event id */
   unsigned int   size; /* total size of extra data only in bytes */
   void          *data; /* any extra data appended to this event */
} __compat;

enum ss_slic_mode
{
   SLIC_MODE_LCFO,
   SLIC_MODE_STANDBY,
   SLIC_MODE_OHT,
   SLIC_MODE_OHTR,
   SLIC_MODE_TIPOPEN,
   SLIC_MODE_RING,
   SLIC_MODE_LCF,
   SLIC_MODE_RLCF,
   SLIC_MODE_RINGOPEN,
   SLIC_MODE_WINK,
   SLIC_MODE_NULL
};

enum ss_daa_mode
{
   DAA_MODE_ONHOOK,
   DAA_MODE_ONHOOK_DATA,
   DAA_MODE_OFFHOOK,
   DAA_MODE_RING_VALID_ON,
   DAA_MODE_RING_VALID_OFF
};

enum ss_daa_loop
{
   DAA_LOOP_OPEN,
   DAA_LOOP_CLOSED,
   DAA_LOOP_CLOSED_REVERSE
};

enum ss_slic_power_source
{
   SLIC_PS_AC,
   SLIC_PS_BATTERY
};

enum ss_ring_waveform
{
   RING_WAVEFORM_SINE,
   RING_WAVEFORM_TRAPEZOID
};

typedef struct __compat
{
   unsigned int ept_id;
   unsigned int value; /* 1 for on, 0 for off */
} SSARG_LEDCONTROL;

typedef struct __compat
{
   unsigned int ept_id;
   int          mode;
} SSARG_MODECONTROL;

typedef struct __compat
{
   unsigned int ept_id;
   unsigned int value; /* 1 for on, 0 for off */
} SSARG_PHREVCONTROL;

typedef struct __compat
{
   unsigned int ept_id;
   unsigned int value; /* 1 for on, 0 for off */
} SSARG_SETBOOSTEDLOOPCURRENT;

typedef struct __compat
{
   unsigned int ept_id;
   unsigned int value; /* 1 for on, 0 for off */
} SSARG_SETFASTSLICMODE;

typedef struct __compat
{
   unsigned int ept_id;
   int          source;
} SSARG_SETPOWERSOURCE;

typedef struct __compat
{
   unsigned int ept_id;
   int          frequency;  /* ringing frequency */
   int          waveshape;  /* ringing waveshape */
   int          voltage;    /* ringing voltage */
   int          offset;     /* ringing DC offset */
   int          offset_cal; /* ringing DC offset calibration */
} SSARG_RINGPARMS;

typedef struct __compat
{
   unsigned int ept_id;
   int          phase_reversal;
   int          loop_current;
   int          power_source;
   int          slic_mode;
} SSARG_SLICPARMS;

typedef struct __compat
{
   unsigned int ept_id;
   unsigned int value; /* 1 for on, 0 for off */
} SSARG_SETSLICENABLE;

typedef struct __compat
{
   unsigned int ept_id;
   unsigned int value; /* 1 for on, 0 for off */
} SSARG_APM_HPFCONTROL;

typedef struct __compat
{
   unsigned int ept_id;
   unsigned int duration;  /* duration of pulse signal (ms) */
   unsigned int period;    /* period of pulse signal (ms) */
   unsigned int reps;      /* number of repetitions */
   unsigned int amplitude; /* amplitude of pulse */
   unsigned int frequency; /* frequency of pulse */
} SSARG_APM_PULSEMETERING;

typedef struct __compat
{
   unsigned int ept_id;
   unsigned int test_id;   /* enum ss_line_test_id */
} SSARG_LINE_TEST;

enum ss_line_test_id
{
   TEST_VOLTAGE,
   TEST_IMPEDANCE,
   TEST_OFFHOOK,
   TEST_REN,
   TEST_SELF,
};

/* Resistive fault test results (deci-ohms) */
struct ss_lt_imp
{
   int tg; /* Tip-ground impedance (centiohms) */
   int rg; /* Ring-ground impedance */
   int tr; /* Tip-ring impedance */
} __compat;

/* Foreign EMF test results */
struct ss_lt_volt
{
   int tac; /* Tip, AC (millivolts) */
   int tdc; /* Tip, DC (mV) */
   int rac; /* Ring, AC (mV) */
   int rdc; /* Ring, DC (mV) */
} __compat;

/* Self-test results */
struct ss_lt_self
{
   int loop;    /* feed-current test result */
   int battery; /* on-hook voltage test result */
   int ring;    /* ring voltage (mV) */
   int cap;     /* capacitance (nF) */
} __compat;

struct ss_lt_result
{
   unsigned int ept_id;
   unsigned int test_id;
   int          ret;
   union
   {
      struct ss_lt_imp  imp;
      struct ss_lt_volt volt;
      struct ss_lt_self self;
      unsigned int      u32;
   } data;
};

#endif /* _BCM_SLICSLAC_H */
