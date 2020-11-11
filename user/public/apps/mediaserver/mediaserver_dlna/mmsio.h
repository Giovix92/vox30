#ifndef _MMSIO_H_
#define _MMSIO_H_
#include <iconv.h>

#define DEFSQLITE3

#if defined DEFSQLITE3
#include "sqlite3.h"
#include "global.h"
#include "md5.h"
#include "upnpd.h"

#define sqlite_open sqlite3_open
#define sqlite_close sqlite3_close
#define sqlite_free sqlite3_free
//#define sqlite_exec sqlite3_exec
typedef sqlite3 sqlite;
#else
#include "sqlite.h"
#define sqlite_open sqlite_open
#define sqlite_close sqlite_close
#define sqlite_exec sqlite_exec
typedef sqlite sqlite;
#endif

#define MS_CACHE_FOLDER "cache"
#define DB_FILE_NAME "media_server.db"
#define DB_CACHE_FILE_TMPLATE "media_server.XXXXXX"

#define DB_CACHE_SUFFIX "-journal"
#define DB_VSION_INFO "SQLite format 3"

#define	PROTOCOL_INFO	"protocolinfo.txt"
#define	CLIENTS_INFO	"clients.txt"

#define MEDIA_CONF	"/etc/media.conf"
#define CONTENT_NUM	8
/* media server action flag */
#define MS_SCANNING		"/tmp/ms_scanning"
#define MS_UPDATING		"/tmp/ms_updating"
#define MS_SCANOK		"/tmp/ms_scanok"
#define MS_SCANERROR	"/tmp/ms_scanerror"

#define	MEDIA_SCAN_PID	"/var/run/media_scan.pid"

#define	SUPPORTED_VIDEO_FORMAT	".avi.dat.mpg.mp4.m4v.wmv.vob.asf.hdmov.mov.mpeg.mp2t.rmvb.ts.tts.mpe.m1v."
#define	SUPPORTED_MUSIC_FORMAT	".aac.ac3.aif.lpcm.mpa.mp2.mp3.mp4.m4a.rm.ram.ra.pcm.wav.wma.3gp.flac.ogg."
#define	SUPPORTED_PHOTO_FORMAT	".bmp.gif.jpg.jpeg.jpe.png.tif.tiff."

#define	NO_TIMESEEK_FORMAT	".wmv.wma."

#define	SUPPORTED_PLAYLIST_FORMAT	".pls.m3u.asx."
#ifdef _SRT_SUPPORT_
#define SUPPORTED_SUBTITLE_FORMAT	".srt."
#endif
#define	MEDIA_TYPE_NONE		0
#define	MEDIA_TYPE_PHOTO	1
#define	MEDIA_TYPE_MUSIC	2
#define	MEDIA_TYPE_VIDEO	3
#define	MEDIA_TYPE_PLAYLIST	4
#ifdef _SRT_SUPPORT_
#define MEDIA_TYPE_SUBTITLE	5
#endif
/* media type */
#define	MUSIC	1
#define	VIDEO	2
#define	PICTURE	4
#ifdef _SRT_SUPPORT_
	#define SUBTITLE	8
#endif
#ifdef _SRT_SUPPORT_
	#define	ALL		(MUSIC|VIDEO|PICTURE|SUBTITLE)
#else
	#define	ALL		(MUSIC|VIDEO|PICTURE)
#endif

#define UNKNOWN_STR "-UNKNOWN-"

struct entry {
    char *path;
    int type;
};

struct media_file_properties{
    char *title;
    char *author;
    char *artist;
    char *album;
    char *genre;
    char *comment;
	char resolution[64];
	char duration[12];
    char year[5];
    char month[3];
    char day[3];
    char time[9];  
    char protocol_info[256];    
    int track;  
    int file_type; 
	unsigned long bitrate;
  	unsigned long sample_frequency;
  	unsigned long bps;
  	unsigned long channels;
    unsigned long long size;
};

#define	DATABASE_INDEX_TITLE		0
#define	DATABASE_INDEX_PATH			1
#define	DATABASE_INDEX_FILENAME		2
#define	DATABASE_INDEX_SIZE			3
#define	DATABASE_INDEX_ARTIST		4
#define	DATABASE_INDEX_GENRE		5
#define	DATABASE_INDEX_ALBUM		6
#define	DATABASE_INDEX_YEAR			7
#define	DATABASE_INDEX_MONTH		8
#define	DATABASE_INDEX_DAY			9
#define	DATABASE_INDEX_WEBPATH		10
#define	DATABASE_INDEX_DURATION		11
#define	DATABASE_INDEX_RESOLUTION	12
#define	DATABASE_INDEX_PROTOCOL_INFO	13
#define	DATABASE_INDEX_CONTAINER_ID		14
#define	DATABASE_INDEX_PLAY_TIMES		15

#ifdef __ICONV_SUPPORT__
extern iconv_t libiconv_open (const char* tocode, const char* fromcode);
extern size_t libiconv (iconv_t cd,  char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
extern int libiconv_close (iconv_t cd);
#endif

int del_record_from_db(sqlite *db, char *path, int file_type);
int insert_record_to_db(sqlite *db, char *path, int type, int fast_scan);
int get_media_info(char *file, struct media_file_properties *pMediaItem);
int check_msdb_version(char *file);
int md5_str(char *in, char *out);
int AddOneFile2DB(sqlite *db, char *filename, int type, int *media_type);
int AddOneFolder2DB(sqlite *db, char *filename);
//int get_wma_fileinfo(char *file, struct timeinfo *ptime, struct mp3info *pmp3);
//int get_aac_fileinfo(char *file, struct timeinfo *ptime, struct mp3info *pmp3);
//off_t aac_drilltoatom(FILE *aac_fp, char *atom_path, unsigned int *atom_length);
void UnicodeToUTF_8(char* pOut,wchar_t* pText);
int sqlite_exec(sqlite3 *db, const char *sql, sqlite3_callback cb, void *arg, char **errmsg);
int scan_main(char *pConfFile, int fast_scan);
int get_media_info(char *file, struct media_file_properties *pMediaItem);
int get_gif_info(char *file,struct media_file_properties *pMediaItem);
int get_jpeg_info(char *file,struct media_file_properties *pMediaItem);
int get_png_info(char *file,struct media_file_properties *pMediaItem);	
void ParseSpecialSQLChar(char *src, char *dest);
	
#endif

