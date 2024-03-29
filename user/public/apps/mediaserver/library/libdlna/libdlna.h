/*
 * libdlna: reference DLNA standards implementation.
 * Copyright (C) 2007-2008 Benjamin Zores <ben@geexbox.org>
 *
 * This file is part of libdlna.
 *
 * libdlna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libdlna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libdlna; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef DLNA_H
#define DLNA_H

/**
 * @file dlna.h
 * external api header.
 */

#ifdef __cplusplus
extern "C" {
#if 0 /* avoid EMACS indent */
}
#endif /* 0 */
#endif /* __cplusplus */

#include <inttypes.h>
#include <sys/types.h>
#include "avcodec.h"
#include "avformat.h"

#define DLNA_STRINGIFY(s)         DLNA_TOSTRING(s)
#define DLNA_TOSTRING(s) #s

#define LIBDLNA_VERSION_INT  ((0<<16)+(2<<8)+3)
#define LIBDLNA_VERSION      0.2.3
#define LIBDLNA_BUILD        LIBDLNA_VERSION_INT

#define LIBDLNA_IDENT        "DLNA " DLNA_STRINGIFY(LIBDLNA_VERSION)

/* Status code for DLNA related functions */
typedef enum {
  DLNA_ST_OK,
  DLNA_ST_ERROR
} dlna_status_code_t;

/* Verbosity level: defines which kind of log can be displayed */
typedef enum {
  DLNA_MSG_NONE,          /* no error messages */
  DLNA_MSG_INFO,          /* working operations */
  DLNA_MSG_WARNING,       /* harmless failures */
  DLNA_MSG_ERROR,         /* may result in hazardous behavior */
  DLNA_MSG_CRITICAL,      /* prevents lib from working */
} dlna_verbosity_level_t;

typedef enum {
  DLNA_PROTOCOL_INFO_TYPE_UNKNOWN,
  DLNA_PROTOCOL_INFO_TYPE_HTTP,
  DLNA_PROTOCOL_INFO_TYPE_RTP,
  DLNA_PROTOCOL_INFO_TYPE_ANY
} dlna_protocol_info_type_t;

/* DLNA.ORG_PS: play speed parameter (integer)
 *     0 invalid play speed
 *     1 normal play speed
 */
typedef enum {
  DLNA_ORG_PLAY_SPEED_INVALID = 0,
  DLNA_ORG_PLAY_SPEED_NORMAL = 1,
} dlna_org_play_speed_t;

/* DLNA.ORG_CI: conversion indicator parameter (integer)
 *     0 not transcoded
 *     1 transcoded
 */
typedef enum {
  DLNA_ORG_CONVERSION_NONE = 0,
  DLNA_ORG_CONVERSION_TRANSCODED = 1,
} dlna_org_conversion_t;

/* DLNA.ORG_OP: operations parameter (string)
 *     "00" (or "0") neither time seek range nor range supported
 *     "01" range supported
 *     "10" time seek range supported
 *     "11" both time seek range and range supported
 */
typedef enum {
  DLNA_ORG_OPERATION_NONE                  = 0x00,
  DLNA_ORG_OPERATION_RANGE                 = 0x01,
  DLNA_ORG_OPERATION_TIMESEEK              = 0x10,
} dlna_org_operation_t;

/* DLNA.ORG_FLAGS, padded with 24 trailing 0s
 *     80000000  31  senderPaced
 *     40000000  30  lsopTimeBasedSeekSupported
 *     20000000  29  lsopByteBasedSeekSupported
 *     10000000  28  playcontainerSupported
 *      8000000  27  s0IncreasingSupported
 *      4000000  26  sNIncreasingSupported
 *      2000000  25  rtspPauseSupported
 *      1000000  24  streamingTransferModeSupported
 *       800000  23  interactiveTransferModeSupported
 *       400000  22  backgroundTransferModeSupported
 *       200000  21  connectionStallingSupported
 *       100000  20  dlnaVersion15Supported
 *
 *     Example: (1 << 24) | (1 << 22) | (1 << 21) | (1 << 20)
 *       DLNA.ORG_FLAGS=01700000[000000000000000000000000] // [] show padding
 */
typedef enum {
  DLNA_ORG_FLAG_SENDER_PACED               = (1 << 31),
  DLNA_ORG_FLAG_TIME_BASED_SEEK            = (1 << 30),
  DLNA_ORG_FLAG_BYTE_BASED_SEEK            = (1 << 29),
  DLNA_ORG_FLAG_PLAY_CONTAINER             = (1 << 28),
  DLNA_ORG_FLAG_S0_INCREASE                = (1 << 27),
  DLNA_ORG_FLAG_SN_INCREASE                = (1 << 26),
  DLNA_ORG_FLAG_RTSP_PAUSE                 = (1 << 25),
  DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE    = (1 << 24),
  DLNA_ORG_FLAG_INTERACTIVE_TRANSFERT_MODE = (1 << 23),
  DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE  = (1 << 22),
  DLNA_ORG_FLAG_CONNECTION_STALL           = (1 << 21),
  DLNA_ORG_FLAG_DLNA_V15                   = (1 << 20),
} dlna_org_flags_t;

typedef enum {
  DLNA_CLASS_UNKNOWN,
  DLNA_CLASS_IMAGE,
  DLNA_CLASS_AUDIO,
  DLNA_CLASS_AV,
  DLNA_CLASS_COLLECTION
} dlna_media_class_t;

typedef enum {
  /* Image Class */
  DLNA_PROFILE_IMAGE_JPEG,
  DLNA_PROFILE_IMAGE_PNG,
  /* Audio Class */
  DLNA_PROFILE_AUDIO_AC3,
  DLNA_PROFILE_AUDIO_AMR,
  DLNA_PROFILE_AUDIO_ATRAC3,
  DLNA_PROFILE_AUDIO_LPCM,
  DLNA_PROFILE_AUDIO_MP3,
  DLNA_PROFILE_AUDIO_MPEG4,
  DLNA_PROFILE_AUDIO_WMA,
  /* AV Class */
  DLNA_PROFILE_AV_MPEG1,
  DLNA_PROFILE_AV_MPEG2,
  DLNA_PROFILE_AV_MPEG4_PART2,
  DLNA_PROFILE_AV_MPEG4_PART10, /* a.k.a. MPEG-4 AVC */
  DLNA_PROFILE_AV_WMV9
} dlna_media_profile_t;

/**
 * DLNA profile.
 * This specifies the DLNA profile one file/stream is compatible with.
 */
typedef struct dlna_profile_s {
  /* Profile ID, part of DLNA.ORG_PN= string */
  const char *id;
  /* Profile MIME type */
  const char *mime;
  /* Profile Label */
  const char *label;
  /* Profile type: IMAGE / AUDIO / AV */
  dlna_media_class_t media_class;
} dlna_profile_t;

/**
 * DLNA Media Object item metadata
 */
typedef struct dlna_metadata_s {
  char     *title;                /* <dc:title> */
  char     *author;               /* <dc:artist> */
  char     *comment;              /* <upnp:longDescription> */
  char     *album;                /* <upnp:album> */
  uint32_t track;                 /* <upnp:originalTrackNumber> */
  char     *genre;                /* <upnp:genre> */
  uint32_t year;                  /**< ID3 year, 0 if none */
} dlna_metadata_t;

/**
 * DLNA Media Object item properties
 */
typedef struct dlna_properties_s {
  int64_t  size;                  /* res@size */
  char     duration[64];          /* res@duration */
  uint32_t bitrate;               /* res@bitrate */
  uint32_t sample_frequency;      /* res@sampleFrequency */
  uint32_t bps;                   /* res@bitsPerSample */
  uint32_t channels;              /* res@nrAudioChannels */
  char     resolution[64];        /* res@resolution */
  int	   file_type;			  /* 0-Video, 1-Audio, 2-Image*/
} dlna_properties_t;

/**
 * DLNA Media Object item
 */
typedef struct dlna_item_s {
  char *filename;
  dlna_media_class_t media_class;
  dlna_properties_t *properties;
  dlna_metadata_t *metadata;
  dlna_profile_t *profile;
} dlna_item_t;
 

typedef struct av_codecs_s {
  /* audio stream and codec */
  AVStream *as;
  AVCodecContext *ac;
  /* video stream and codec */
  AVStream *vs;
  AVCodecContext *vc;
  char file_name[256];
} av_codecs_t;
 
/**
 * DLNA Library's controller.
 * This controls the whole library.
 */
typedef struct dlna_s dlna_t;

/**
 * Initialization of library.
 *
 * @warning This function must be called before any libdlna function.
 * @return DLNA library's controller.
 */
dlna_t *dlna_init (void);

/**
 * Uninitialization of library.
 *
 * @param[in] dlna The DLNA library's controller.
 */
void dlna_uninit (dlna_t *dlna);

/**
 * Register all known/supported DLNA profiles.
 *
 * @param[in] dlna  The DLNA library's controller.
 */
void dlna_register_all_media_profiles (dlna_t *dlna);

/**
 * Register one specific known/supported DLNA profiles.
 *
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] profile  The profile ID to be registered.
 */
void dlna_register_media_profile (dlna_t *dlna, dlna_media_profile_t profile);

/**
 * Guess which DLNA profile one input file/stream is compatible with.
 *
 * @warning This function returns a pointer, do _NOT_ free it.
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] filename The file to be checked for compliance.
 * @return A pointer on file's DLNA profile if compatible, NULL otherwise.
 */
dlna_profile_t *dlna_guess_media_profile (AVFormatContext *ctx, dlna_t *dlna, av_codecs_t *codecs, const char *filename);

/**
 * Create a new DLNA media object item.
 *
 * @param[in] dlna     The DLNA library's controller.
 * @param[in] filename The input file to be added.
 * @return A new DLNA object item if compatible, NULL otherwise.
 */
dlna_item_t *dlna_item_new (dlna_t *dlna, const char *filename, int *not_compliant);

/**
 * Free an existing DLNA media object item.
 *
 * @param[in] item     The DLNA object item to be freed.
 */
void dlna_item_free (dlna_item_t *item);


#ifdef __cplusplus
#if 0 /* avoid EMACS indent */
{
#endif /* 0 */
}
#endif /* __cplusplus */

#endif /* DLNA_H */
