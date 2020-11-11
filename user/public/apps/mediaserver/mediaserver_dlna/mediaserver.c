#define	_GNU_SOURCE
#include <string.h>
#include "upnpd.h"
#include "upnptools.h"
#include "tool.h"
#include "sample_util.h"
#include <stdio.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <upnp/ithread.h>
#include "mediaserver.h"
#include "dlna.h"
#include "http.h"
#include <stdarg.h>
#include <signal.h>
#include "pic_scale.h"
//Define types of agent
#define	STANDARD	0
#define	XBOX360		1
extern playlist *g_playlist;
extern int most_played_num;

//int gUpdateID=1;//status of UpdateID
//char gContainerID[512];//change file name

char *xml_encode (char *src);
char *blank2str (char *src);
extern char db_file_path[];
static int db_query_data_callback (void *, int, char **, char **);
static int db_query_data_callback_xbox360 (void *, int, char **, char **);
static int db_query_data_callback_flag(void *, int, char **, char **, int);
static int db_query_db_by_browse_callback (void *, int, char **, char **);
static int db_query_count_callback (void *, int, char **, char **);
int MediaRegisterDevice (struct Upnp_Action_Request *ca_event);
int MediaIsAuthorized (struct Upnp_Action_Request *ca_event);
int MediaIsValidated (struct Upnp_Action_Request *ca_event);


void mBUG (char *format, ...)
{
#if 0
	va_list args;
	FILE *fp;

	fp = fopen ("/dev/ttyS0", "w");
	if (!fp) {
		return;
	}
	va_start (args, format);
	vfprintf (fp, format, args);
	va_end (args);
	fprintf (fp, "\n");
	fflush (fp);
	fclose (fp);
#endif
}

/********************************************************
 *		Global Variable Declaration 		*
 ********************************************************/

extern struct MediaEnv media;

char *MediaServiceType[] = {
	"urn:schemas-upnp-org:service:ConnectionManager:1",
	"urn:schemas-upnp-org:service:ContentDirectory:1"
};

/* Global arrays for storing <service> OSInfo  variable names, values, and defaults */
char *osi_varname[] = {
	"OSMajorVersion",
	"OSMinorVersion",
	"OSBuildNumber",
	"OSMachineName"
};

/*
   Content Directory Variables
*/
char *content_varname[] = {
	"A_ARG_TYPE_SortCriteria",
	"A_ARG_TYPE_TransferLength",
	"TransferIDs",
	"A_ARG_TYPE_UpdateID",
	"A_ARG_TYPE_SearchCriteria",	/* 5 */
	
	"A_ARG_TYPE_Filter",
	"ContainerUpdateIDs",
	"A_ARG_TYPE_Result",
	"A_ARG_TYPE_Index",
	"A_ARG_TYPE_TransferID",		/* 10 */
	
	"A_ARG_TYPE_TagValueList",
	"A_ARG_TYPE_URI",
	"A_ARG_TYPE_BrowseFlag",
	"A_ARG_TYPE_ObjectID",
	"SortCapabilities",				/* 15 */
	
	"A_ARG_TYPE_Count",
	"SearchCapabilities",
	"SystemUpdateID",
	"A_ARG_TYPE_TransferStatus",
	"A_ARG_TYPE_TransferTotal"		/* 20 */
};

char content_varval[DS_CONTENT_VARCOUNT][DS_MAX_VAL_LEN];
char *content_varval_def[] = {
	"0",
	"0",
	"0",
	"0",
	"0",			/* 5 */
	
	"0",
	"",
	"0",
	"0",
	"0",			/* 10 */
	
	"0",
	"0",
	"BrowseDirectChildren",
	"0",
	"dc:title",		/* 15 */
	
	"0",
	"dc:title",
	"1",
	"0",
	"0"				/* 20 */
};

/*
   Connection Manager Variables
*/
char *connect_varname[] = {
	"A_ARG_TYPE_ProtocolInfo",
	"A_ARG_TYPE_ConnectionStatus",
	"A_ARG_TYPE_AVTransportID",
	"A_ARG_TYPE_RcsID",
	"A_ARG_TYPE_ConnectionID",			/* 5 */
	
	"A_ARG_TYPE_ConnectionManager",
	"SourceProtocolInfo",
	"SinkProtocolInfo",
	"A_ARG_TYPE_Direction",
	"CurrentConnectionIDs"				/* 10 */
};

/*
   Connection Manager Default Values
*/
char connect_varval[DS_CONNECT_VARCOUNT][DS_MAX_VAL_LEN];
char *connect_varval_def[] = {
        "0",
        "OK",
        "0",
        "0",
        "0",						/* 5 */
        
        "0",
        "http-get:*:audio/mpeg:*",
        "0",
        "Output",
        "0"							/* 10 */
};

/* Global structure for storing the state table for this device */
struct IGDService ds_service_table[DS_SERVICE_SERVCOUNT1];

static const struct FilterStruct filter_table[] = {
	{0x4000, "dc:creator"},
	{0x0002, "upnp:artist"},
	{0x0004, "upnp:genre"},
	{0x0008, "upnp:album"},
	{0x0010, "dc:date"},
	{0x0020, "upnp:class"},
	{0x0040, "storageMedium"},
	{0x0080, "refID"},
  //{0x0080, "writeStatus"},
	{0x0100, "res@size"},
  //{0x0200, "res@importUri"},
	{0x0400, "res@duration"},
  //{0x0800, "res@bitrate"},
	{0x1000, "res@protocolInfo"},
	{0x2000, "res@resolution"},
	{0x1000, "res"},
	{0x1000, "desc"},
	{0x8000, "@childCount"},
	{0x0001, "dc:title"}
};

#define SORTCRITERIANUM	(sizeof(filter_table)/sizeof(struct FilterStruct))

static const struct MediaService media_catalog_table[] = {
	{SEL_NUL, "0", "-1", "1", "Root"},
	
	{SEL_NUL, "A2", "0", "1", "Music"},
	{SEL_NUL, "A3", "0", "1", "Pictures"},
	{SEL_NUL, "A1", "0", "1", "Videos"},	
	//{SEL_PATH,"A4","0","1","Contents",},
	
	{SEL_MOVIES_ALL, "B11", "A1", "1", "All Videos"},
	//{SEL_MOVIES_DATE, "B15", "A1", "1", "Date"},
	//{SEL_MOVIES_GENRE, "B12", "A1", "1", "Genre"},
	{SEL_MOVIES_FOLDER, "B13", "A1", "1", "Folder"},
	
	{SEL_MUSIC_ALBUM, "B24", "A2", "1", "Album"},
	{SEL_MUSIC_ALL, "B21", "A2", "1", "All Tracks"},
	{SEL_MUSIC_ARTIST, "B23", "A2", "1", "Artist"},
	//{SEL_MUSIC_DATE, "B25", "A2", "1", "Date"},
	{SEL_MUSIC_FOLDER, "B27", "A2", "1", "Folder"},
	{SEL_MUSIC_GENRE, "B22", "A2", "1", "Genre"},
	{SEL_MUSIC_PLAYLIST,"B26","A2","1","Playlists"},
	
	{SEL_PHOTO_ALBUM, "B34", "A3", "1", "Album"},
	{SEL_PHOTO_ALL, "B31", "A3", "1", "All Pictures"},
	{SEL_PHOTO_FOLDER, "B36", "A3", "1", "Folder"},
	{SEL_PHOTO_DATE, "B35", "A3", "1", "Date"},
	
	{SEL_LAST_PLAYED,"C21","B26","1","- Last Played -"},
	{SEL_MOST_PLAYED,"C22","B26","1","- Most Played -"}
};

#define NUM_Media_catalog (sizeof(media_catalog_table)/sizeof(struct MediaService))

static const struct MediaService yahoo_media_catalog_table[] = {
	{SEL_NUL, "0", "-1", "1", "Root"},
	{SEL_NUL, "A2", "0", "1", "Music"},
	{SEL_MUSIC_ALL, "B21", "A2", "1", "All Music"}

};

#define YAHOO_NUM_MEDIA_CATALOG (sizeof(yahoo_media_catalog_table)/sizeof(struct MediaService))

const struct MediaProtocolInfo media_protocolinfo[] = {
	/* Video files */
	{"3gp", NULL, "http-get:*:video/3gpp:", "videoItem.movie"},
	{"asf", NULL, "http-get:*:video/x-ms-asf:","videoItem.movie"},
	{"avi", "AVI"/*"MPEG4_P2_TS_SP_MPEG1_L3"*/, "http-get:*:video/avi:", "videoItem.movie"},
	{"dat", NULL, "http-get:*:video/vcd:", "videoItem.movie"},
	{"m4v", NULL, "http-get:*:video/mp4:","videoItem.movie"},
	{"mov", NULL, "http-get:*:video/quicktime:","videoItem.movie"},
	{"hdmov", NULL, "http-get:*:video/quicktime:","videoItem.movie"},
	{"mp2t", NULL, "http-get:*:video/mp2t:", "videoItem.movie"},
	{"mp4", NULL, "http-get:*:video/mp4:", "videoItem.movie"},
	{"mpa", NULL, "http-get:*:audio/mpeg:", "videoItem.movie"},
	{"mpeg",NULL, "http-get:*:video/mpeg:", "videoItem.movie"},
	{"mpg", NULL, "http-get:*:video/mpeg:", "videoItem.movie"},
	{"rmvb", NULL, "http-get:*:video/mpeg:", "videoItem.movie"},
	{"vob", NULL, "http-get:*:video/dvd:", "videoItem.movie"},
	{"wmv", NULL, "http-get:*:video/x-ms-wmv:", "videoItem.movie"},
	{"tts", NULL,"http-get:*:video/vnd.dlna.mpeg-tts:", "videoItem.movie"},
	{"ts", NULL,"http-get:*:video/mpeg2:", "videoItem.movie"},
	{"mpe",NULL, "http-get:*:video/mpeg:", "videoItem.movie"},
	{"m1v",NULL, "http-get:*:video/mpeg:", "videoItem.movie"},
	
	/* Audio files */
	{"3gp", NULL, "http-get:*:audio/3gpp:", "audioItem.musicTrack"},
	{"aac", NULL, "http-get:*:audio/aac:","audioItem.musicTrack"},
	{"ac3", NULL, "http-get:*:audio/ac3:", "audioItem.musicTrack"},
	{"asx", NULL, "http-get:*:audio/x-ms-asx:", "audioItem.musicTrack"},
	{"aif", NULL, "http-get:*:audio/aif:", "audioItem.musicTrack"},
	{"flac", NULL, "http-get:*:audio/flac:", "audioItem.musicTrack"},
	{"lpcm", NULL, "http-get:*:audio/l16;","audioItem.musicTrack"},
	{"m4a", NULL, "http-get:*:audio/x-pv-mp4:", "audioItem.musicTrack"},
	{"mp2", NULL, "http-get:*:audio/mp2:", "audioItem.musicTrack"},
	{"mp3", NULL, "http-get:*:audio/mpeg:", "audioItem.musicTrack"},
	{"mp4", NULL, "http-get:*:audio/mp4:", "audioItem.musicTrack"},
	{"pcm", NULL, "http-get:*:audio/l16:","audioItem.musicTrack"},
	{"wav", NULL, "http-get:*:audio/x-wav:", "audioItem.musicTrack"},
	{"wma", NULL, "http-get:*:audio/x-ms-wma:", "audioItem.musicTrack"},
	{"ogg", NULL, "http-get:*:audio/x-ogg:", "audioItem.musicTrack"},
	{"rm", NULL, "http-get:*:audio/x-pn-realaudio:", "audioItem.musicTrack"},
	{"ram", NULL, "http-get:*:audio/x-pn-realaudio:", "audioItem.musicTrack"},
	{"ra", NULL, "http-get:*:audio/x-pn-realaudio:", "audioItem.musicTrack"},
	 
	/* Images files */
//	{"ico", NULL, "http-get:*:image/x-icon:", "imageItem.photo"},
	{"bmp", NULL, "http-get:*:image/bmp:", "imageItem.photo"},
	{"gif", NULL, "http-get:*:image/gif:", "imageItem.photo"},
	{"jpeg", NULL, "http-get:*:image/jpeg:", "imageItem.photo"},
	{"jpg", NULL, "http-get:*:image/jpeg:", "imageItem.photo"},
	{"jpe", NULL, "http-get:*:image/jpeg:", "imageItem.photo"},
	{"png", NULL, "http-get:*:image/png:", "imageItem.photo"},
	{"tif", NULL, "http-get:*:image/tif:", "imageItem.photo"},
	{"tiff", NULL, "http-get:*:image/tif:", "imageItem.photo"},

	/* Playlist files */
	{"m3u", NULL, "http-get:*:audio/x-mpegurl:", "audioItem.musicTrack"},
	{"pls", NULL, "http-get:*:audio/x-scpls:", "audioItem.musicTrack"},
	{"asx", NULL, "http-get:*:audio/x-ms-asf:", "audioItem.musicTrack"}
	
	/* Subtitle Text files */
#ifdef _SRT_SUPPORT_
	,{"srt", NULL, "http-get:*:application/octet-stream:","object.item"}
#endif
	
};
const struct MediaProtocolInfo jpg_protocolinfo[] = {
	{"jpg", "JPEG_LRG","http-get:*:image/jpeg:", "imageItem.photo"},
	{"jpg", "JPEG_MED", "http-get:*:image/jpeg:", "imageItem.photo"},
	{"jpg", "JPEG_SM", "http-get:*:image/jpeg:", "imageItem.photo"},
	{"jpg", "JPEG_TN", "http-get:*:image/jpeg:", "imageItem.photo"},
	{"jpg", "JPEG_LRG_ICO", "http-get:*:image/jpeg:", "imageItem.photo"},
	{"jpg", "JPEG_SM_ICO", "http-get:*:image/jpeg:", "imageItem.photo"},
};
const struct MediaProtocolInfo png_protocolinfo[] = {
	{"png", "PNG_LRG","http-get:*:image/png:", "imageItem.photo"},
	{"png", "PNG_TN", "http-get:*:image/png:", "imageItem.photo"},
	{"png", "PNG_LRG_ICO", "http-get:*:image/png:", "imageItem.photo"},
	{"png", "PNG_SM_ICO", "http-get:*:image/png:", "imageItem.photo"},
};
/* Mutex for protecting the global state table data
   in a multi-threaded, asynchronous environment.
   All functions should lock this mutex before reading
   or writing the state table data. */
pthread_mutex_t DSDevMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MESDevMutex = PTHREAD_MUTEX_INITIALIZER;

extern char cache_folder_path[];

CONTAINER_UPDATE_ID *pContainerIDHead=NULL, *pContainerIDTail=NULL;

void UpdateOneContainer(char *pContainer)
{
	CONTAINER_UPDATE_ID *pCurrent=NULL, *pNew=NULL;
	
	if(!pContainer || !*pContainer)
		return;
	pCurrent=pContainerIDHead;
	while(pCurrent){ //Check if it exists or not
		if(!strcmp(pCurrent->pContainer, pContainer)){//Find it
			pCurrent->update_id++;
			break;
		}
		pCurrent=pCurrent->pNext;
	}	
	//Not find, insert it.
	pNew=malloc(sizeof(CONTAINER_UPDATE_ID));
	if(!pNew)
		return;
	pNew->pNext=NULL;
	pNew->update_id=1;
	pNew->pContainer=strdup(pContainer);
	if(!pNew->pContainer){
		free(pNew);
		pNew=NULL;
		return;	
	}
	if(!pContainerIDHead){
		pContainerIDTail=pContainerIDHead=pNew;
	}
	else{
		pContainerIDTail->pNext=pNew;
		pContainerIDTail=pNew;
	}
	return;
}

void FreeContainerList(void)
{
	CONTAINER_UPDATE_ID *pCurrent=NULL, *pPrevious=NULL;
	
	pCurrent=pContainerIDHead;
	while(pCurrent){ //Check if it exists or not
		pPrevious=pCurrent;
		pCurrent=pCurrent->pNext;
		if(pPrevious->pContainer)
			free(pPrevious->pContainer);
		free(pPrevious);
	}		
	return;
}

unsigned int GetUpdateIDByContainer(char *pContainer)
{
	CONTAINER_UPDATE_ID *pCurrent=NULL;
	
	if(!pContainer || !*pContainer)
		return 0;
	pCurrent=pContainerIDHead;
	while(pCurrent){ //Check if it exists or not
		if(!strcmp(pCurrent->pContainer, pContainer)){//Find it
			return(pCurrent->update_id);
		}
		pCurrent=pCurrent->pNext;
	}		
	return 0; //Use default 0
}


#define SQL_CHECK_RECORD_COMMAND "SELECT DISTINCT ObjectName from Data where ObjectPath like '%s%%' and Title = '%s' limit 1"
#define SQL_SUBDIR_FILE_ENTRY_COMMAND "SELECT DISTINCT Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where ObjectPath = '%s' and Title = '%s'"
#define SQL_SUBDIR_FOLDER_ENTRY_COMMAND "SELECT DISTINCT ObjectPath,ObjectName,ContainerID from Data where ObjectPath = '%s' and Title = '%s'"
#define SQL_QUERY_CONTAINERID_BY_NAME_COMMAND "SELECT DISTINCT ContainerID from Data where ObjectPath = '%s%s' and ObjectName = '%s' limit 1"
#define SQL_QUERY_NAME_BY_CONTAINERID_COMMAND "SELECT DISTINCT ObjectPath,ObjectName from Data where ContainerID = '%s' limit 1"


static int db_query_check_by_name_callback(void *pUser, int nArg, char **azArg, char **NotUsed)
{
	if(nArg==1){
		strcpy(*(char **)pUser, "1");
//    	printf("FileName: %s\n", azArg[0]);
    	return 0;
    }
    return -1;
}

int MediaFileInSubDir(sqlite *db, char *pFolder, int media_type)
{
    char sql_cmd[1024] = { 0 }, title[12]={0};
    char *sqlresult = NULL;
    char *zErrMsg = NULL;
    char *encoded_path=NULL;
	int len=0,ret=0;
	
	encoded_path=malloc(2*strlen(pFolder)+1);
	if(encoded_path){
		memset(encoded_path, 0, 2*strlen(pFolder)+1);
		ParseSpecialSQLChar(pFolder, encoded_path);
	}
	else
		encoded_path=pFolder;
	len=strlen(encoded_path);
	if(encoded_path[len-1]!='/')	
		strcat(encoded_path, "/");	
	if(media_type==MUSIC)
		strcpy(title,"audio");
	else if(media_type==VIDEO)
		strcpy(title,"video");
	else if(media_type==PICTURE)
		strcpy(title,"photo");		
	sqlresult=malloc(2);
	if(!sqlresult){
	    if(encoded_path && encoded_path!=pFolder){
	    	free(encoded_path);
	    	encoded_path=NULL;
	    }	
		return 0;
	}
	memset(sqlresult, 0, sizeof(sqlresult));
	sprintf(sql_cmd, SQL_CHECK_RECORD_COMMAND, encoded_path, title);
    ret = sqlite_exec(db, sql_cmd, db_query_check_by_name_callback, &sqlresult, &zErrMsg);
    if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
    }
    if(encoded_path && encoded_path!=pFolder){
    	free(encoded_path);
    	encoded_path=NULL;
    }
	if(ret == SQLITE_OK) { 
		if(sqlresult[0]=='1'){
			free(sqlresult);
			return 1;
		}
	}
	free(sqlresult);
	return 0;
}

static int db_query_folder_entry_callback(void *pUser, int nArg, char **azArg, char **NotUsed)
{
	FILE *fp=NULL;

	if(nArg==3){ //Query Folders
    	//printf("Folder Cache File: %s\n",*(char **)pUser);
		fp=fopen(*(char **)pUser, "at");
		if(fp){
			fprintf(fp,"%s%s*%s\n", azArg[0],azArg[1],azArg[2]);
			fclose(fp);
		}
	}
	else if(nArg==14){//Query Files
		//printf("File Cache File: %s\n",*(char **)pUser);
		//printf("azArg[DATABASE_INDEX_FILENAME]=%s\n",azArg[DATABASE_INDEX_FILENAME]);
		fp=fopen(*(char **)pUser, "at");
		if(fp){
			fprintf(fp,"%s\\%s\\%s\\%s\\%s\\%s\\%s\\%s\\%s\\%s\\%s\\%s\\%s\\%s\n",azArg[DATABASE_INDEX_TITLE],azArg[DATABASE_INDEX_PATH],azArg[DATABASE_INDEX_FILENAME],azArg[DATABASE_INDEX_SIZE],
			azArg[DATABASE_INDEX_ARTIST],azArg[DATABASE_INDEX_GENRE],azArg[DATABASE_INDEX_ALBUM],azArg[DATABASE_INDEX_YEAR],
			azArg[DATABASE_INDEX_MONTH],azArg[DATABASE_INDEX_DAY],azArg[DATABASE_INDEX_WEBPATH],azArg[DATABASE_INDEX_DURATION],
			azArg[DATABASE_INDEX_RESOLUTION],azArg[DATABASE_INDEX_PROTOCOL_INFO]);
			fclose(fp);
		}	
	}
    return 0;
}

static int db_query_containerid_by_name_callback(void *pUser, int nArg, char **azArg, char **NotUsed)
{
	if(nArg==1){
    	*(char **)pUser = malloc(strlen(azArg[0])+1);
    	if(!*(char **)pUser)
    		return -1;
    	strcpy(*(char **)pUser, azArg[0]);
    	return 0;
    }
    return -1;
}

static int db_query_name_by_containerid_callback(void *pUser, int nArg, char **azArg, char **NotUsed)
{
	if(nArg==2){
    	*(char **)pUser = malloc(strlen(azArg[0])+strlen(azArg[1])+4);
    	if(!*(char **)pUser)
    		return -1;
    	sprintf(*(char **)pUser, "%s%s", azArg[0], azArg[1]);
    	return 0;
    }
    return -1;
}

int FilterSubDirEnties(sqlite *db, char *pCacheFile, int media_type)
{
	char pTempFile[256]={0};
	char buf[1024]={0}, *p=NULL;
	FILE *fp=NULL, *fh=NULL;
	int fd=0;
	
	if(!pCacheFile || !pCacheFile[0])
		return -1;
	sprintf(pTempFile, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
	fd=mkstemp(pTempFile);
	if(fd==-1){
		return -1;
	}	
	close(fd);		
	fp=fopen(pCacheFile, "rt");
	if(!fp){
		remove(pTempFile);
		return -1;
	}
	fh=fopen(pTempFile, "wt");
	if(!fh){
		fclose(fp);
		remove(pTempFile);
	}
	while(fgets(buf, sizeof(buf)-1,fp)){
		p=strrchr(buf, '*');//Remove Container ID.	
		if(p){
			*p=0;
			p++;
		}
		
		if(!MediaFileInSubDir(db, buf, media_type)){
			continue;
		}
		if(p)
			fprintf(fh, "%s*%s", buf, p);
		else
			fputs(buf, fh);
	}
	fflush(fh);
	fclose(fh);
	fclose(fp);
	remove(pCacheFile);
	rename(pTempFile, pCacheFile);
	return 0;
}

#define CONTAINER_FORMAT "container id=\"%s/%s\" parentID=\"%s\" restricted=\"1\" &gt;\n &lt;dc:title&gt;%s&lt;/dc:title&gt;\n &lt;upnp:class&gt;%s&lt;/upnp:class&gt;&lt;/container&gt;&lt;"
#define CONTAINER_FORMAT_CHLD_COUNT "container id=\"%s/%s\" parentID=\"%s\" restricted=\"1\" childCount=\"%lu\" &gt;\n &lt;dc:title&gt;%s&lt;/dc:title&gt;\n &lt;upnp:class&gt;%s&lt;/upnp:class&gt;&lt;/container&gt;&lt;"
#define	XML_TITLE	"<u:BrowseResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
#define	XML_STR_BEGIN	"<Result>&lt;DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"&gt;\n&lt;"
#define	XML_STR_END		"/DIDL-Lite&gt;</Result><NumberReturned>%lu</NumberReturned><TotalMatches>%lu</TotalMatches><UpdateID>%u</UpdateID></u:BrowseResponse>"

#define	XML_SEARCH_TITLE		"<u:SearchResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
#define	XML_SEARCH_STR_BEGIN	"<Result>&lt;DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0/dlna/\" xmlns:pv=\"http://www.pv.com/pvns/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"&gt;\n&lt;"
#define	XML_SEARCH_STR_END		"/DIDL-Lite&gt;</Result><NumberReturned>%lu</NumberReturned><TotalMatches>%lu</TotalMatches><UpdateID>%u</UpdateID></u:SearchResponse>"

int GetLineFromFile(char **lbuf,FILE *fp);
extern struct entry content_folders[];

void FreeMediaItemInfo(struct ItemStruct *pItem)
{
	if(!pItem)
		return;
	if(pItem->size)
		free(pItem->size);
	if(pItem->item_year)
		free(pItem->item_year);
	if(pItem->item_month)
		free(pItem->item_month);
	if(pItem->item_day)
		free(pItem->item_day);
	if(pItem->duration)
		free(pItem->duration);
	if(pItem->protocolInfo)
		free(pItem->protocolInfo);
	if(pItem->artist)
		free(pItem->artist);
	if(pItem->genre)
		free(pItem->genre);
	if(pItem->album)
		free(pItem->album);
	if(pItem->resolution)
		free(pItem->resolution);
	if(pItem->item_id)
		free(pItem->item_id);
	if(pItem->title)
		free(pItem->title);
	if(pItem->item_address)
		free(pItem->item_address);
	if(pItem->objectitem)
		free(pItem->objectitem);
	if(pItem->parentID)
		free(pItem->parentID);
}

int GetMediaInfoFromBuf(char *pContainerID, char *pBuffer, struct ItemStruct *pItem, unsigned long *buf_size, int special_agent, int file_type , struct g_variable *g_var)
{
	char *p=NULL, *p0=NULL,*p1=NULL;
	int i=0, find=0;
	char mediaurl[256]={0};
	char *pSId = NULL;
	char *pMId = NULL;
	char *pDId = NULL;
	char *pDId0 = NULL;
	char *pDId1 = NULL;
	//char *pDId2 = NULL;
	char *pDArtist = NULL;
	char *pDGenre = NULL;
	char *pDAlbum = NULL;
	char *pDParentid = NULL;
	char *pResolution = NULL;
	unsigned long mcount = 0;
	int nfilterflag = 0;
		
	sprintf (mediaurl, "http://%s:%d", media.InternalIPAddress, media.upnp_port);
	p=strrchr(pBuffer, '\n');
	if(p)
		*p=0;

	p=strchr(pBuffer, '\\');
	if(!p)
		return -1;
	*p++=0;

	//pBuffer	//Title
		
	p0=strchr(p, '\\');
	if(!p0)
		return -1;
	*p0++=0;
	//p			//Object Path

	p=strchr(p0, '\\');
	if(!p)
		return -1;
	*p++=0;	
	//p0		//Object Name
	pDId1 = xml_encode (p0);
	p1=strrchr(p0, '.');
	if(p1)
		p1++;
	else
		p1=p0;
	nfilterflag = g_var->g_filterflag;
	for (i = 0; i < sizeof (media_protocolinfo) / sizeof (struct MediaProtocolInfo); i++) {
		if (strcasecmp (media_protocolinfo[i].ch_postfix, p1) == 0) {
			if(file_type==MEDIA_TYPE_MUSIC && strstr(media_protocolinfo[i].ch_objectitem, "audioItem."))
				;
			else if(file_type==MEDIA_TYPE_VIDEO && strstr(media_protocolinfo[i].ch_objectitem, "videoItem."))
				;
			else if(file_type==MEDIA_TYPE_PHOTO && strstr(media_protocolinfo[i].ch_objectitem, "imageItem."))
				;
			else				
				continue;				
			find = 1;
			if(i==SEQ_AVI_PROTOCOL)
				g_var->g_filterflag = g_var->g_filterflag|0x0400;//Add duration for AVI
			break;
		}
	}
	if (find == 0){
		return -1;
	}
	p0=strchr(p, '\\');
	if(!p0)
		return -1;
	*p0++=0;
	//p			//Size	
	pItem->size = strdup(p);
	mcount = mcount + strlen (p);

	p=strchr(p0, '\\');
	if(!p)
		return -1;
	*p++=0;	
	//p0		//Artist
	pDArtist = xml_encode (p0);
	pItem->artist = pDArtist;//Need free
	mcount = mcount + strlen (pDArtist);

	p0=strchr(p, '\\');
	if(!p0)
		return -1;
	*p0++=0;
	//p			//Genre	
	pDGenre = xml_encode (p);
	pItem->genre = pDGenre;
	mcount = mcount + strlen (pDGenre);
		
	p=strchr(p0, '\\');
	if(!p)
		return -1;
	*p++=0;	
	//p0		//Album
	pDAlbum = xml_encode (p0);
	pItem->album = pDAlbum;
	mcount = mcount + strlen (pDAlbum);
	
	p0=strchr(p, '\\');
	if(!p0)
		return -1;
	*p0++=0;
	//p			//Year	
	pItem->item_year = strdup(p);
	mcount = mcount + strlen (p);
		
	p=strchr(p0, '\\');
	if(!p)
		return -1;
	*p++=0;	
	//p0		//Month	
	pItem->item_month = strdup(p0);
	mcount = mcount + strlen (p0);

	p0=strchr(p, '\\');
	if(!p0)
		return -1;
	*p0++=0;
	//p			//Day	
	pItem->item_day = strdup(p);
	mcount = mcount + strlen (p);
		
	p=strchr(p0, '\\');
	if(!p)
		return -1;
	*p++=0;	
	//p0		//Web Location	
	if ((pSId = (char *) malloc (strlen (mediaurl) + strlen (p0) + 4)) == NULL){
		return -1;
	}
	sprintf (pSId, "%s%s", mediaurl, p0);
	pMId = blank2str (pSId);
	free (pSId);
	pSId = NULL;
	pDId = xml_encode (pMId);
	if (pMId != NULL) {
		free (pMId);
		pMId = NULL;
	}
	if ((pSId = (char *) malloc (strlen (pContainerID) + strlen (p0) + 5)) == NULL){
		return -1;
	}
	sprintf (pSId, "%s%s", pContainerID, p0);
	pDId0 = xml_encode (pSId);
	free (pSId);
	pSId = NULL;
		
	p0=strchr(p, '\\');
	if(!p0)
		return -1;
	*p0++=0;
	//p			//Duration	
	pItem->duration = strdup(p);
	mcount = mcount + strlen (p);
		
	p=strchr(p0, '\\');
	if(!p)
		return -1;
	*p++=0;	
	//p0		//Resolution	
	//p			//Protocol Info	
	pResolution = xml_encode (p0);
	pItem->resolution = pResolution;
	mcount = mcount + strlen (pResolution);
	if(i==SEQ_AVI_PROTOCOL && special_agent==XBOX360){
		char dlna_info[128]={0};
	
		sprintf (dlna_info, "%s=%s;%s=%s;%s=%s;%s=%s", DLNA_ORG_PN,"AVI", DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_AV);
		pItem->protocolInfo = strdup(dlna_info);
	}
	else
		pItem->protocolInfo = strdup(p);
	mcount = mcount + strlen (p);
		
	//printf("Procotol Info: %s\n", pItem->protocolInfo);				
	
	pDParentid = xml_encode (pContainerID);
		
	mcount = mcount + PACKET_ITEM_LEN;
	pItem->item_id = pDId0;
	mcount = mcount + strlen (pDId0);
		
	pItem->refID = pDId0;
	mcount = mcount + strlen(pDId0);
		
	pItem->parentID = pDParentid;
	mcount = mcount + strlen (pDParentid);
		
	pItem->title = pDId1;	//Need free
	mcount = mcount + strlen (pDId1);
								
	//pItem->importUri = pDId2;
	//mcount = mcount + strlen(pDId2);
			
	pItem->item_address = pDId;
	mcount = mcount + strlen (pDId);
		
	pItem->objectitem = strdup (media_protocolinfo[i].ch_objectitem);
	mcount = mcount + strlen (media_protocolinfo[i].ch_objectitem);

	*buf_size=mcount;
	return 0;
}

int GetChildCountOfDir(sqlite *db, char *pFolder, int media_type)
{
    char sql_cmd[1024] = { 0 }, title[12]={0}, pTmpFolder[1024]={0};
    char *pFileCache = NULL, *pFolderCache=NULL, *pBuf=NULL;
    char *zErrMsg = NULL;
    char *encoded_path=NULL;
	int len=0,ret=0,fd=0,parent_id=0,i=0;
	FILE *fp=NULL;    
	unsigned long total_num=0;
	
	//printf("pFolder: %s\n",pFolder);
	if(!strcmp(pFolder, "/"))
		parent_id=1;
	if(parent_id){
		while(i<CONTENT_NUM){
			if(content_folders[i].path && (content_folders[i].type & media_type)){
				strcpy(pTmpFolder, content_folders[i].path);
				break;
			}
			else
				i++;		
		}
	}
check_content_folder:	
	if(parent_id){
		if(i>=CONTENT_NUM)
			goto out;
		i++;
	}
	else{
		strcpy(pTmpFolder, pFolder);
	}	
		
	encoded_path=malloc(2*strlen(pFolder)+3);
	if(encoded_path){
		memset(encoded_path, 0, 2*strlen(pFolder)+1);
		ParseSpecialSQLChar(pFolder, encoded_path);
	}
	else
		encoded_path=pFolder;
	len=strlen(encoded_path);
	if(encoded_path[len-1]!='/')	
		strcat(encoded_path, "/");		
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	pFileCache=malloc(256);
	if(!pFileCache){
		if(encoded_path && encoded_path!=pFolder){
			free(encoded_path);
			encoded_path=NULL;
		}			
		return 0;
	}
	memset(pFileCache, 0, sizeof(pFileCache));
	pFolderCache=malloc(256);
	if(!pFolderCache){
		free(pFileCache);
		if(encoded_path && encoded_path!=pFolder){
			free(encoded_path);
			encoded_path=NULL;
		}			
		return 0;	
	}	
	memset(pFolderCache, 0, sizeof(pFolderCache));		
	if(!strlen(pFolderCache)){
		sprintf(pFolderCache, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
		fd=mkstemp(pFolderCache);
		if(fd==-1){
			remove(pFileCache);
			free(pFileCache);
			free(pFolderCache);
			if(encoded_path && encoded_path!=pFolder){
			    free(encoded_path);
			    encoded_path=NULL;
			}					
			return 0;
		}
		close(fd);    
	}
	//Query folders under current folder.
	strcpy(title,"Folder");	
	sprintf(sql_cmd, SQL_SUBDIR_FOLDER_ENTRY_COMMAND, encoded_path, title);
    ret = sqlite_exec(db, sql_cmd, db_query_folder_entry_callback, &pFolderCache, &zErrMsg);
    if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
		remove(pFileCache);
		free(pFileCache);
		free(pFolderCache);
		if(encoded_path && encoded_path!=pFolder){
			free(encoded_path);
			encoded_path=NULL;
		}					
		return 0;			
    }  
 	//printf("%s: %d\n", __FUNCTION__, __LINE__);
 	if(!strlen(pFileCache)){
	 	sprintf(pFileCache, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
		fd=mkstemp(pFileCache);
		if(fd==-1){
			remove(pFileCache);
			free(pFileCache);
			free(pFolderCache);
			if(encoded_path && encoded_path!=pFolder){
			    free(encoded_path);
			    encoded_path=NULL;
			}			
			return 0;
		}
		close(fd);   
	}
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	//Query relative media files under current folder.
	if(media_type==MUSIC)
		strcpy(title,"audio");
	else if(media_type==VIDEO)
		strcpy(title,"video");
	else if(media_type==PICTURE)
		strcpy(title,"photo");	    	
	//printf("pFileCache=%s\n",pFileCache);
	sprintf(sql_cmd, SQL_SUBDIR_FILE_ENTRY_COMMAND, encoded_path, title);

	ret = sqlite_exec(db, sql_cmd, db_query_folder_entry_callback, &pFileCache, &zErrMsg);
	if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
		remove(pFileCache);
		remove(pFolderCache);
		free(pFileCache);
		free(pFolderCache);
		if(encoded_path && encoded_path!=pFolder){
			free(encoded_path);
			encoded_path=NULL;
		}					
		return 0;			
	}
	if(encoded_path && encoded_path!=pFolder){
	    free(encoded_path);
	    encoded_path=NULL;
	}
    if(parent_id){
		while(i<CONTENT_NUM){
			if(content_folders[i].path && (content_folders[i].type & media_type)){
				strcpy(pTmpFolder, content_folders[i].path);
				goto check_content_folder;
			}
			i++;		
		}    	
    }	

out:  		
    //Filter Directories
    FilterSubDirEnties(db, pFolderCache, media_type);
    fp=fopen(pFolderCache, "rt");
    if(fp){
    	while(!GetLineFromFile(&pBuf,fp)){
    		if(strlen(pBuf) && pBuf[0]!='\n')
    			total_num++;
    		free(pBuf);
    		pBuf=NULL;
    	}
    	fclose(fp);
    }    
    //printf("Folder Num=%lu\n",total_num);
    fp=fopen(pFileCache, "rt");
    if(fp){
    	while(!GetLineFromFile(&pBuf,fp)){
    		if(strlen(pBuf) && pBuf[0]!='\n')
    			total_num++;
    		free(pBuf);
    		pBuf=NULL;
    	}
    	fclose(fp);
    }
    //printf("File+Folder Num=%lu\n",total_num);  
    remove(pFileCache);
	remove(pFolderCache);	
	free(pFileCache);
	free(pFolderCache);
	return total_num;      
}
    
int GetSubDirEntries(char **ppResult, sqlite *db, char *pContainerID, int media_type, unsigned long start_index, unsigned long query_num, int special_agent, int browse_flag,struct g_variable *g_var)
{
    char sql_cmd[1024] = { 0 }, title[12]={0}, pFolder[1024];
    char *pFileCache = NULL, *pFolderCache=NULL, aResult[256]={0}, *pBuf=NULL;
    char *zErrMsg = NULL;
    char *encoded_path=NULL, *p=NULL, *p0=NULL, *pTmpBuf=NULL;
	int len=0,ret=0,fd=0,parent_id=0, i=0, first=1;
	FILE *fp=NULL, *fh=NULL;
	unsigned long return_num=0, total_num=0, buf_size=0,check_num=0;//, child_count=0;
	struct stat f_stat;
	struct ItemStruct media_item;
	int file_type=MEDIA_TYPE_NONE;
	
	if(browse_flag!=0 && browse_flag!=1)	
		return -1;
	p=strrchr(pContainerID, '/');
	if(!p){
		p=pContainerID;
		parent_id=1;
	}
	else
		p++;
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
	printf("pContainerID: %s\n", pContainerID);
#endif			
	if(parent_id){
		while(i<CONTENT_NUM){
			if(content_folders[i].path && (content_folders[i].type & media_type)){
				strcpy(pFolder, content_folders[i].path);
				break;
			}
			else
				i++;		
		}
	}
	pFileCache=malloc(256);
	if(!pFileCache)
		return -1;
	memset(pFileCache, 0, sizeof(pFileCache));
	pFolderCache=malloc(256);
	if(!pFolderCache){
		free(pFileCache);
		return -1;	
	}	
	memset(pFolderCache, 0, sizeof(pFolderCache));	
check_content_folder:	
	if(parent_id){
		if(i>=CONTENT_NUM)
			goto out;
		i++;
	}
	else{
		ret=GetFolderOfContainerID(db, p, pFolder);
		if(ret || !strlen(pFolder)){
			free(pFolderCache);
			free(pFileCache);
			return -1;
		}	
	}
#ifdef __DEBUG__	
	printf("pFolder: %s\n",pFolder);
#endif		
	encoded_path=malloc(2*strlen(pFolder)+3);
	if(encoded_path){
		memset(encoded_path, 0, 2*strlen(pFolder)+1);
		ParseSpecialSQLChar(pFolder, encoded_path);
	}
	else
		encoded_path=pFolder;
	len=strlen(encoded_path);
	if(encoded_path[len-1]!='/')	
		strcat(encoded_path, "/");		
	if(!strlen(pFolderCache)){
		sprintf(pFolderCache, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
		fd=mkstemp(pFolderCache);
		if(fd==-1){
			remove(pFileCache);
			free(pFileCache);
			free(pFolderCache);
			if(encoded_path && encoded_path!=pFolder){
	    		free(encoded_path);
	    		encoded_path=NULL;
			}
			return -1;
		}
		close(fd);    
	}
	//Query folders under current folder.
	strcpy(title,"Folder");	
	sprintf(sql_cmd, SQL_SUBDIR_FOLDER_ENTRY_COMMAND, encoded_path, title);
    ret = sqlite_exec(db, sql_cmd, db_query_folder_entry_callback, &pFolderCache, &zErrMsg);
    if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
    }  
 	
 	if(!strlen(pFileCache)){
	 	sprintf(pFileCache, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
		fd=mkstemp(pFileCache);
		if(fd==-1){
			remove(pFolderCache);
			remove(pFileCache);
			free(pFileCache);
			free(pFolderCache);
			if(encoded_path && encoded_path!=pFolder){
	    		free(encoded_path);
	   		 encoded_path=NULL;
			}
			return -1;
		}
		close(fd);   
	}
	//Query relative media files under current folder.
	if(media_type==MUSIC){
		strcpy(title,"audio");
		file_type=MEDIA_TYPE_MUSIC;
	}
	else if(media_type==VIDEO){
		strcpy(title,"video");
		file_type=MEDIA_TYPE_VIDEO;
	}
	else if(media_type==PICTURE){
		strcpy(title,"photo");	    	
		file_type=MEDIA_TYPE_PHOTO;
	}    	
	//printf("pFileCache=%s\n",pFileCache);
	sprintf(sql_cmd, SQL_SUBDIR_FILE_ENTRY_COMMAND, encoded_path, title);

	ret = sqlite_exec(db, sql_cmd, db_query_folder_entry_callback, &pFileCache, &zErrMsg);
	if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
		remove(pFolderCache);
		remove(pFileCache);
		free(pFileCache);
		free(pFolderCache);
		if(encoded_path && encoded_path!=pFolder){
	    	free(encoded_path);
			encoded_path=NULL;
		}
		return -1;			
	}
	if(encoded_path && encoded_path!=pFolder){
	    free(encoded_path);
	    encoded_path=NULL;
	}
    if(parent_id){
		while(i<CONTENT_NUM){
			if(content_folders[i].path && (content_folders[i].type & media_type)){
				strcpy(pFolder, content_folders[i].path);
				goto check_content_folder;
			}
			i++;		
		}    	
    }	

out:    
    //Filter Directories
    FilterSubDirEnties(db, pFolderCache, media_type);
    fp=fopen(pFolderCache, "rt");
    if(fp){
    	while(!GetLineFromFile(&pBuf,fp)){
    		if(strlen(pBuf) && pBuf[0]!='\n')
    			total_num++;
    		free(pBuf);
    		pBuf=NULL;
    	}
    	fclose(fp);
    }    
#ifdef __DEBUG__    
    printf("Folder Num=%lu\n",total_num);
#endif    
    fp=fopen(pFileCache, "rt");
    if(fp){
    	while(!GetLineFromFile(&pBuf,fp)){
    		if(strlen(pBuf) && pBuf[0]!='\n')
    			total_num++;
    		free(pBuf);
    		pBuf=NULL;
    	}
    	fclose(fp);
    }
#ifdef __DEBUG__    
    printf("File+Folder Num=%lu\n",total_num);
#endif    
	if(start_index<0 || start_index>=total_num){
		query_num=0;
	}
	else{
		if(query_num==0 || query_num > total_num)
			query_num=total_num-start_index;
	}
#ifdef __DEBUG__	
	printf("start_index=%lu\n",start_index);
	printf("query_num=%lu\n",query_num);
#endif	
	sprintf(aResult, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
	fd=mkstemp(aResult);
	if(fd==-1){
		remove(pFileCache);
		remove(pFolderCache);
		free(pFileCache);
		free(pFolderCache);
		return -1;
	}
	close(fd);     
	
    fh=fopen(aResult, "at");
    if(!fh){
		remove(pFileCache);
		remove(pFolderCache);
		free(pFileCache);
		free(pFolderCache);
		remove(aResult);
	return -1;
	}
	if(first){
		if(browse_flag)//Browse
			fprintf(fh, "%s%s",XML_TITLE, XML_STR_BEGIN);
		else if(browse_flag==1){//Search
			fprintf(fh, "%s%s",XML_SEARCH_TITLE, XML_SEARCH_STR_BEGIN);
		}
		first=0;
	}
	//Read Folders 
    fp=fopen(pFolderCache, "rt");
    if(!fp)
    	goto read_file;

	while(!GetLineFromFile(&pBuf,fp)){
		p=strrchr(pBuf, '\n');
		if(p)
			*p=0;
		p=strrchr(pBuf, '*');
		if(!p){
			free(pBuf);
			pBuf=NULL;
			continue;	
		}
		if(check_num>=start_index){
			check_num++;
			*p++=0;
			p0=strrchr(pBuf, '/');
			if(!p0){
				p0=pBuf;
			}
			else
				p0++;
#if 0				
			if(g_filterflag & 0x8000){
				child_count=GetChildCountOfDir(db, pBuf, media_type);
				printf("%s: %d\n", __FUNCTION__, __LINE__);
				printf("child_count: %lu\n", child_count);
				if(special_agent==XBOX360)
					fprintf(fh, CONTAINER_FORMAT_CHLD_COUNT, pContainerID, p, pContainerID, child_count, p0, "object.container.storageFolder");
				else
					fprintf(fh, CONTAINER_FORMAT_CHLD_COUNT, pContainerID, p, pContainerID, child_count, p0, "object.container");
			}
			else{
#endif			
				if(special_agent==XBOX360)
					fprintf(fh, CONTAINER_FORMAT, pContainerID, p, pContainerID, p0, "object.container.storageFolder");
				else
					fprintf(fh, CONTAINER_FORMAT, pContainerID, p, pContainerID, p0, "object.container");
//			}
			free(pBuf);
			pBuf=NULL;
			return_num++;
			if(return_num>=query_num)
				break;
		}
		else{
			check_num++;
			free(pBuf);
			pBuf=NULL;		
		}
	}
	fclose(fp);
read_file:	
	if(return_num < query_num){
	    fp=fopen(pFileCache, "rt");
	    if(!fp){
	    	remove(aResult);
	 		remove(pFileCache);
			remove(pFolderCache);
			free(pFileCache);
			free(pFolderCache);
			return -1;   
		}
		while(!GetLineFromFile(&pBuf,fp)){
			buf_size=0;
			if(check_num>=start_index){		
				check_num++;
				memset(&media_item, 0, sizeof(media_item));
				ret=GetMediaInfoFromBuf(pContainerID, pBuf, &media_item, &buf_size, special_agent, file_type,g_var);
				if(ret || !buf_size){
					FreeMediaItemInfo(&media_item);
					free(pBuf);
					pBuf=NULL;	
					continue;
				}
				pTmpBuf=malloc(buf_size);
				if(!pTmpBuf){
					FreeMediaItemInfo(&media_item);
					free(pBuf);
					pBuf=NULL;	
					continue;			
				}
				ret=makeitempacket (pTmpBuf, &media_item, special_agent,g_var);
				FreeMediaItemInfo(&media_item);
				if(ret){
					free(pBuf);
					pBuf=NULL;	
					if(pTmpBuf){
						free(pTmpBuf);
						pTmpBuf=NULL;
					}
					continue;
				}
				fputs(pTmpBuf, fh);
				free(pTmpBuf);
				pTmpBuf=NULL;	
				return_num++;			
				if(pBuf){
					free(pBuf);
					pBuf=NULL;
				}
				if(return_num>=query_num)
					break;				
			}		
			else{
				check_num++;
				free(pBuf);
				pBuf=NULL;			
			}
		}
		fclose(fp);
	}
	if(browse_flag)//Browse
		fprintf(fh, XML_STR_END, return_num, total_num, GetUpdateIDByContainer(pContainerID));
	else if(browse_flag==0)//Search
		fprintf(fh, XML_SEARCH_STR_END, return_num, total_num, GetUpdateIDByContainer(pContainerID));
	fflush(fh);
	fclose(fh);	
    remove(pFileCache);
	remove(pFolderCache);	
	free(pFileCache);
	free(pFolderCache);
#ifdef __DEBUG__	
	printf("return_num=%lu\n",return_num);
	printf("total_num=%lu\n",total_num);
#endif	
	if(stat(aResult,&f_stat) < 0 || f_stat.st_size==0){
		remove(aResult);  
		return -1;
	}

	*ppResult=(char *)malloc(f_stat.st_size+1);
   	if(!*ppResult){
   		remove(aResult);    		
		return -1;
	}
	memset(*ppResult,0,f_stat.st_size+1);
	fp=fopen(aResult,"rt");
	if(fp){
		fread(*ppResult,1,f_stat.st_size,fp);
   		fclose(fp);
   		remove(aResult);
   	}
   	else{
   		free(*ppResult);
   		*ppResult=NULL;
   		remove(aResult);   		
   		return -1;
   	}	
	return 0;
}

int GetContainerIDsOfFolder(sqlite *db, char *pFolder, char **ppContainerID, int file_type)
{
    char sql_cmd[1024] = { 0 }, *container_id_path=NULL;
    char *sqlresult = NULL, *pTemp=NULL, *pTemp1=NULL, *pTemp2=NULL;
    char *zErrMsg = NULL;
    char *encoded_path=NULL,*encoded_name=NULL,*p=NULL, *p0=NULL, *p1=NULL;
#define	MEM_SIZE	256    
	int len=0,ret=0,media_type=0, i=0, mem_num=MEM_SIZE, mem_num_1=MEM_SIZE;
	
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	if(!pFolder || !pFolder[0])
		return -1;
	pTemp1=malloc(mem_num_1);
	if(!pTemp1)
		return -1;	
	memset(pTemp1, 0, mem_num_1);
	container_id_path=malloc(mem_num);
	if(!container_id_path){
		free(pTemp1);
		pTemp1=NULL;
		return -1;		
	}	
	memset(container_id_path, 0, mem_num);
	pTemp=malloc(strlen(pFolder)+1);
	if(!pTemp){
		free(pTemp1);
		pTemp1=NULL;
		free(container_id_path);
		container_id_path=NULL;		
		return -1;		
	}	
	memset(pTemp, 0, strlen(pFolder)+1);
	if(file_type==MEDIA_TYPE_PHOTO){
		media_type=PICTURE;
		strcpy(container_id_path, "B36");
	}
	else if(file_type==MEDIA_TYPE_MUSIC){
		media_type=MUSIC;
		strcpy(container_id_path, "B27");
	}
	else if(file_type==MEDIA_TYPE_VIDEO){
		media_type=VIDEO;
		strcpy(container_id_path, "B13");
	}
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	while(i<CONTENT_NUM){
		if(!content_folders[i].path){
			i++;
			continue;
		}
		pTemp2=strdup(content_folders[i].path);
		if(!pTemp2){
			i++;
			continue;
		}		
		len=strlen(pTemp2);
		if(pTemp2[len-1]=='/')
			pTemp2[len-1]=0;
		if(pTemp[len-1]=='/')
			pTemp[len-1]=0;			
		strcpy(pTemp, pFolder);
		if((content_folders[i].type & media_type)  && strstr(pTemp, pTemp2)){
			p=strchr(pTemp, '/');
			if(p){
				*p++=0;	
			}			
			while(p){
				if(strlen(pTemp)<=strlen(pTemp2)){	//Too short path
					*(p-1)='/';
					p1=p;
					p=strchr(p1, '/');		//
					if(p){
						*p++=0;
						continue;
					}						
					//Reach tail.
				}
last_time:		
				//printf("pTemp: %s\n", pTemp);	
				encoded_path=malloc(2*strlen(pTemp)+1);
				if(encoded_path){
					memset(encoded_path, 0, 2*strlen(pTemp)+1);
					ParseSpecialSQLChar(pTemp, encoded_path);
				}
				else
					encoded_path=pTemp;
				len=strlen(encoded_path);
				if(encoded_path[len-1]=='/')	
					encoded_path[len-1]=0;
				encoded_name=strrchr(encoded_path, '/');
				if(!encoded_name){
					i++;
					break;	
				}
							
				*encoded_name++=0;
				len=strlen(encoded_path);
				sprintf(sql_cmd, SQL_QUERY_CONTAINERID_BY_NAME_COMMAND, encoded_path, encoded_path[len-1]=='/'?"":"/", encoded_name);
			    ret = sqlite_exec(db, sql_cmd, db_query_containerid_by_name_callback, &sqlresult, &zErrMsg);
			    if (ret != SQLITE_OK) {
					if (zErrMsg)
						sqlite_free(zErrMsg);
				    if(encoded_path && encoded_path!=pTemp){
				    	free(encoded_path);
				    	encoded_path=NULL;
				    }						
					i++;
					break;
			    }
			    if(encoded_path && encoded_path!=pTemp){
			    	free(encoded_path);
			    	encoded_path=NULL;
			    }
				if(sqlresult){
					p0=strrchr(container_id_path,',');
					if(p0)
						len=strlen(p0+1);
					else
						len=strlen(container_id_path);
					//printf("strlen(sqlresult)+len+32: %d\n", strlen(sqlresult)+len+32);
					//printf("mem_num_1: %d\n", mem_num_1);
					if(strlen(sqlresult)+len+32 >= mem_num_1){	//Need allocate more memorfy for pTemp1
						mem_num_1=strlen(sqlresult)+len+32;
						p0=NULL;						
						p0=realloc(pTemp1, mem_num_1);
						if(!p0){
							free(pTemp);
							pTemp=NULL;		
												
							free(pTemp1);
							pTemp1=NULL;
							
							free(container_id_path);
							container_id_path=NULL;		
							return -1;									
						}
						pTemp1=p0;							
					}

					p0=strrchr(container_id_path,',');
					if(p0)	
						sprintf(pTemp1,",%s/%s", p0+1,sqlresult);
					else{
						sprintf(pTemp1,",%s/%s", container_id_path,sqlresult);
					}
					//printf("strlen(container_id_path) + strlen(pTemp1) + 32: %d\n", strlen(container_id_path) + strlen(pTemp1) + 32);
					//printf("mem_num: %d\n", mem_num);					
					if(strlen(container_id_path) + strlen(pTemp1) + 32 >= mem_num){//Need allocate more memory for container_id_path.
						mem_num=strlen(container_id_path) + strlen(pTemp1) + 32;
						p0=NULL;
						p0=realloc(container_id_path, mem_num);
						if(!p0){
							free(pTemp);
							pTemp=NULL;		
												
							free(pTemp1);
							pTemp1=NULL;
							
							free(container_id_path);
							container_id_path=NULL;		
							return -1;									
						}
						container_id_path=p0;
					}
					strcat(container_id_path, pTemp1);
					free(sqlresult);	
					sqlresult=NULL;
				}	
				if(p){
					*(p-1)='/';	
					p1=p;
					p=strchr(p1, '/');		//
					if(p){
						*p++=0;
					}						
					else{	//Reach tail.
						goto last_time;	
					}				
				}		
			}
		}//if((content_folders[i].type & media_type)  && strstr(pTemp, pTemp2)){
		i++;	
		free(pTemp2);
		pTemp2=NULL;
	}	
	if(pTemp){
		free(pTemp);
		pTemp=NULL;	
	}
	*ppContainerID=malloc(strlen(container_id_path)+1);
	if(!*ppContainerID){
		free(pTemp1);
		pTemp1=NULL;
		free(container_id_path);
		container_id_path=NULL;		
		return -1;
	}
	memset(*ppContainerID, 0, strlen(container_id_path)+1);
	strcpy(*ppContainerID, container_id_path);
	free(pTemp1);
	pTemp1=NULL;
	free(container_id_path);
	container_id_path=NULL;
	return 0;
}

int GetContainerIDOfFolder(sqlite *db, char *pFolder, char *pContainerID)
{
    char sql_cmd[1024] = { 0 };
    char *sqlresult = NULL;
    char *zErrMsg = NULL;
    char *encoded_path=NULL,*encoded_name=NULL;
	int len=0,ret=0;
	
	if(!pFolder || !pFolder[0])
		return -1;
	encoded_path=malloc(2*strlen(pFolder)+1);
	if(encoded_path){
		memset(encoded_path, 0, 2*strlen(pFolder)+1);
		ParseSpecialSQLChar(pFolder, encoded_path);
	}
	else
		return -1;
	len=strlen(encoded_path);
	if(encoded_path[len-1]=='/')	
		encoded_path[len-1]=0;
	encoded_name=strrchr(encoded_path, '/');
	if(!encoded_name) {
		if(encoded_path){
			free(encoded_path);
			encoded_path=NULL;
		}
		return -1;	
	}
	*encoded_name++=0;
	len=strlen(encoded_path);
	sprintf(sql_cmd, SQL_QUERY_CONTAINERID_BY_NAME_COMMAND, encoded_path, encoded_path[len-1]=='/'?"":"/", encoded_name);
    ret = sqlite_exec(db, sql_cmd, db_query_containerid_by_name_callback, &sqlresult, &zErrMsg);
    if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
		if(encoded_path){
			free(encoded_path);
			encoded_path=NULL;
		}
		return -1;
    }
    if(encoded_path ){
    	free(encoded_path);
    	encoded_path=NULL;
    }

	if(sqlresult){
		strcpy(pContainerID, sqlresult);
		free(sqlresult);	
	}
	return 0;
}

int GetFolderOfContainerID(sqlite *db, char *pContainerID, char *pFolder)
{
    char sql_cmd[1024] = { 0 };
    char *sqlresult = NULL;
    char *zErrMsg = NULL;
	int ret=0;
	
	if(!pContainerID || !pContainerID[0])
		return -1;
	sprintf(sql_cmd, SQL_QUERY_NAME_BY_CONTAINERID_COMMAND, pContainerID);
    ret = sqlite_exec(db, sql_cmd, db_query_name_by_containerid_callback, &sqlresult, &zErrMsg);
    if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
		return -1;
    }

	if(sqlresult){
		strcpy(pFolder, sqlresult);
		free(sqlresult);	
	}
	return 0;
}

char * mime_get_protocol (const char *filename, int file_type)
{
	const char *extension;
	char protocol[512];
	int i = 0, find = 0;

	extension = strrchr (filename, '.');
	if (extension != NULL) {
		for (i = 0; i < sizeof (media_protocolinfo) / sizeof (struct MediaProtocolInfo); i++) {
			if (strcasecmp (media_protocolinfo[i].ch_postfix, extension + 1) == 0) {
				if(file_type==MEDIA_TYPE_MUSIC && strstr(media_protocolinfo[i].ch_objectitem, "audioItem."))
					sprintf (protocol, "%s", media_protocolinfo[i].ch_protocolinfo);
				else if(file_type==MEDIA_TYPE_VIDEO && strstr(media_protocolinfo[i].ch_objectitem, "videoItem."))
					sprintf (protocol, "%s", media_protocolinfo[i].ch_protocolinfo);
				else if(file_type==MEDIA_TYPE_PHOTO && strstr(media_protocolinfo[i].ch_objectitem, "imageItem."))
					sprintf (protocol, "%s", media_protocolinfo[i].ch_protocolinfo);
#ifdef _SRT_SUPPORT_
				else if(file_type==MEDIA_TYPE_SUBTITLE &&strstr(media_protocolinfo[i].ch_objectitem, "object."))
					sprintf (protocol, "%s", media_protocolinfo[i].ch_protocolinfo);	
#endif
				else				
					continue;
				find = 1;
				break;
			}
		}
	}
	if (!find)
		return NULL;
	if (media_protocolinfo[i].dlna_type) {
		char dlna_info[448] = { 0 };

		if((strncmp(media_protocolinfo[i].dlna_type,"JPEG_",5)==0)||(strcmp(media_protocolinfo[i].dlna_type,"PNG_TN")==0)){
			if(strcmp(media_protocolinfo[i].dlna_type,"JPEG_*")==0) {                                                        
				char dlna_type[20];                                  
				memset(dlna_type,0,sizeof(dlna_type));
				if(strstr(filename,VIRTUAL_PIC)!=NULL)
					strcpy(dlna_type,jpg_protocolinfo[2].dlna_type);
				//else
				//	getDlnaProtocol(dlna_type,resolution);
				sprintf (dlna_info, "%s=%s;%s=%s;%s=%s;%s=%s", DLNA_ORG_PN,dlna_type, DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);

			}
			else
				sprintf (dlna_info, "%s=%s;%s=%s;%s=%s;%s=%s", DLNA_ORG_PN,media_protocolinfo[i].dlna_type, DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		}
		else{
			if((strcmp(media_protocolinfo[i].dlna_type,"MP3")==0)||(strstr(media_protocolinfo[i].dlna_type,"WMA")==media_protocolinfo[i].dlna_type))
				sprintf (dlna_info, "%s=%s;%s=%s;%s=%s;%s=%s", DLNA_ORG_PN,media_protocolinfo[i].dlna_type, DLNA_ORG_OP, DLNA_ORG_OP_MP3, DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_AV);
			else
				sprintf (dlna_info, "%s=%s;%s=%s;%s=%s;%s=%s", DLNA_ORG_PN,media_protocolinfo[i].dlna_type, DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_AV);
		}
		strcat (protocol, dlna_info);
	}
	else
		strcat (protocol, "*");
	return strdup (protocol);
}

char *strchr_2 (char *src)
{
	char *p = NULL;
	p = src;
	if (!p) {
		return NULL;
	}
	while (p) {
		if (*p == 0) {
			return NULL;
		}
		if (*p >= 0x28 && *p <= 0x7E) {
			p = p + 1;
			continue;
		}
		else {
			return p;
		}

	}
	return NULL;
}

char *blank2str (char *src)
{
	char *dst = NULL;
	char ch;
	int i = 0;
	int j = 0;
	char *ptemp = NULL;
	char *ps = NULL;
	char *pt = NULL;
	char chtmp[10] = { 0 };
	ps = src;
	ptemp = (char *) malloc (strlen (src) + 1);
	strcpy (ptemp, src);
	do {
		ps = strchr_2 (ps);
		if (ps != NULL) {
			i = i + 1;			//HOW MUCH blank
			ps = ps + 1;
		}
	} while (ps != NULL);
	dst = (char *) malloc (strlen (src) + 2 * i + 1);
	memset (dst, 0, sizeof (dst));
	ps = ptemp;
	if (i == 0) {
		strcpy (dst, ps);
	}
	else {
		while (i > 0) {
			pt = strchr_2 (ps);
			if (pt != NULL) {
				ch = *pt;
				j = j + 1;
				*pt = 0;
				if (j == 1)
					strcpy (dst, ps);
				else
					strcat (dst, ps);
				sprintf (chtmp, "%%%0x", ch);
				strcat (dst, chtmp);
				//sprintf(dst,"%s%s\%20",dst,ps);
				ps = pt + 1;
			}

			else {
				strcat (dst, ps);
				break;
			}
			i = i - 1;
		}
		strcat (dst, ps);
	}
	if (ptemp != NULL) {
		free (ptemp);
		ptemp = NULL;
	}

	return dst;
}

int amp_decode (char *src, char *dst)
{
	char *p = NULL;
	char strtmp[1024] = { 0 };
	char *pt = NULL;
	
	strcpy (strtmp, src);
	p = strtmp;
	while (p && *p != '\0') {
		if ((pt = strstr (p, "&amp;")) != NULL) {
			*pt = '\0';
			sprintf (dst+strlen(dst), "%s&", p);
			p = pt + strlen ("&amp;");
		}
		else {
			sprintf (dst+strlen(dst), "%s",  p);
			break;
		}
	}
	return 0;
}

char *xml_encode (char *src)
{
	char *dst = NULL;
	char *p = NULL;
	int i = 0;
	int len = 0;
	//char del[]="&<>\"\'";

	for (i = 0, p = src; p && *p; p++) {
		switch (*p) {
		case '&':
			len = len + strlen ("&amp;amp;") - 1;
			break;
		case '<':
			len = len + strlen ("&lt;") - 1;
			break;
		case '>':
			len = len + strlen ("&gt;") - 1;
			break;
		case '"':
			len = len + strlen ("&quot;") - 1;
			break;
		case '\'':
			len = len + strlen ("&apos;") - 1;
			break;
		default:
			break;
		}
	}
	/*
	   p=strtok(src,del);
	   if(p)i++;
	   while(p=strtok(NULL,del))i++;
	   len=strlen(src)+i*6;
	 */

	if ((dst = (char *) malloc (strlen (src) + len + 4)) == NULL)
		return NULL;

	if (len == 0) {
		strcpy (dst, src);
		return dst;
	}
	//strcpy(dst, src);
	len = 0;
	for (i = 0, p = src; p && *p; p++) {
		switch (*p) {
		case '&':
			len = strlen ("&amp;amp;");
			memcpy (dst + i, "&amp;amp;", len);
			i += len;
			break;
		case '<':
			len = strlen ("&lt;");
			memcpy (dst + i, "&lt;", len);
			i += len;
			break;
		case '>':
			len = strlen ("&gt;");
			memcpy (dst + i, "&gt;", len);
			i += len;
			break;
		case '"':
			len = strlen ("&quot;");
			memcpy (dst + i, "&quot;", len);
			i += len;
			break;
		case '\'':
			len = strlen ("&apos;");
			memcpy (dst + i, "&apos;", len);
			i += len;
			break;
		default:
			*(dst + i) = *p;
			i = i + 1;
			break;
		}
	}
	*(dst + i) = 0;
	return dst;
}




/********************************************************************************
 * MediaDeviceStateTableInit
 *
 * Description:
 *       Initialize the device state table for
 * 	 this IGDDevice, pulling identifier info
 *       from the description Document.  Note that
 *       knowledge of the service description is
 *       assumed.  State table variables and default
 *       values are currently hard coded in this file
 *       rather than being read from service description
 *       documents.
 *
 * Parameters:
 *   DescDocName -- The description document name
 *   DescDocURL -- The description document URL
 *
 ********************************************************************************/
int MediaDeviceStateTableInit (char *DescDocURL, char *DescDocName)
{
	IXML_Document *DescDoc = NULL;
	int ret = UPNP_E_SUCCESS;
	char *servid_connection = NULL, *evnturl_connection =
		NULL, *ctrlurl_connection = NULL;
	char *servid_content = NULL, *evnturl_content = NULL, *ctrlurl_content =
		NULL;
	char *udn = NULL;
	char fullurl[256];
	int i;

	sprintf (fullurl, "%s/%s", DescDocURL, DescDocName);
	if (UpnpDownloadXmlDoc (fullurl, &DescDoc) != UPNP_E_SUCCESS) {
		printf ("DeviceStateTableInit -- Error Parsing %s\n", DescDocURL);
		ret = UPNP_E_INVALID_DESC;
	}

	/*
	   2005.5.10 john
	   OSInfo service lead to igd port mapping setting dialog in networking connection can not open
	 */

	/* Find the Common Interface Config Service identifiers */
	if (!SampleUtil_FindAndParseService (DescDoc, DescDocURL, MediaServiceType[DS_SERVICE_CONNECTIONMANAGER],
		    &servid_connection, &evnturl_connection, &ctrlurl_connection)) {
	    printf ("MediaDeviceStateTableInit -- Error: Could not find Service: %s\n", MediaServiceType[DS_SERVICE_CONNECTIONMANAGER]);

	    if (servid_connection)
			free (servid_connection);
	    if (evnturl_connection)
			free (evnturl_connection);
	    if (ctrlurl_connection)
			free (ctrlurl_connection);
	    ixmlDocument_free(DescDoc);
	    return (UPNP_E_INVALID_DESC);
	}

	/* Find the IP Connection identifiers */
	if (!SampleUtil_FindAndParseService (DescDoc, DescDocURL, MediaServiceType[DS_SERVICE_CONTENTDIRECTORY], &servid_content, &evnturl_content, &ctrlurl_content)) {
	    printf ("MediaDeviceStateTableInit -- Error: Could not find Service: %s\n", MediaServiceType[DS_SERVICE_CONTENTDIRECTORY]);

	    if (servid_content)
			free (servid_content);
	    if (evnturl_content)
			free (evnturl_content);
	    if (ctrlurl_content)
			free (ctrlurl_content);
	    ixmlDocument_free(DescDoc);
	    return (UPNP_E_INVALID_DESC);
	}
	udn = SampleUtil_GetDocumentItem (DescDoc, "UDN", 0);
	if(udn == NULL)
	{
		ixmlDocument_free(DescDoc); 
		return ret;
	}
	//Connection Manager	  
	strcpy (ds_service_table[DS_SERVICE_CONNECTIONMANAGER].UDN, udn);
	strcpy (ds_service_table[DS_SERVICE_CONNECTIONMANAGER].ServiceId, servid_connection);
	strcpy (ds_service_table[DS_SERVICE_CONNECTIONMANAGER].ServiceType, MediaServiceType[DS_SERVICE_CONNECTIONMANAGER]);
	ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableCount = DS_CONNECT_VARCOUNT;
	for (i = 0; i < ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableCount; i++) {
		ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableName[i] = connect_varname[i];
		ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableStrVal[i] = connect_varval[i];
		strcpy (ds_service_table[DS_SERVICE_CONNECTIONMANAGER]. VariableStrVal[i], connect_varval_def[i]);
	}
	
	//Content Directory
	strcpy (ds_service_table[DS_SERVICE_CONTENTDIRECTORY].UDN, udn);
	strcpy (ds_service_table[DS_SERVICE_CONTENTDIRECTORY].ServiceId, servid_content);
	strcpy (ds_service_table[DS_SERVICE_CONTENTDIRECTORY].ServiceType, MediaServiceType[DS_SERVICE_CONTENTDIRECTORY]);
	ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableCount = DS_CONTENT_VARCOUNT;
	for (i = 0; i < ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableCount; i++) {
		ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableName[i] = content_varname[i];
		ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableStrVal[i] = content_varval[i];
		strcpy (ds_service_table[DS_SERVICE_CONTENTDIRECTORY]. VariableStrVal[i], content_varval_def[i]);
	}
	
	if (udn)
		free (udn);
	if (servid_connection)
		free (servid_connection);
	if (evnturl_connection)
		free (evnturl_connection);
	if (ctrlurl_connection)
		free (ctrlurl_connection);
	if (servid_content)
		free (servid_content);
	if (evnturl_content)
		free (evnturl_content);
	if (ctrlurl_content)
		free (ctrlurl_content);
	ixmlDocument_free(DescDoc);
	return (ret);
}

/******************************************************************************
 *sel_count_from_db
 *
 *Description
 *       Get  how much do we need results.
 *Parameters:
 *        db -- The database handle.
 *        sql-- The sql sentence
 *         
 ******************************************************************************/
int sel_count_from_db (sqlite * db, const char *sql,struct g_variable *g_var)
{
//	char *result = NULL;
	int rc = 0;
	char *zErrMsg = NULL;
	
	rc = sqlite_exec (db, sql, db_query_count_callback, &g_var, &zErrMsg);
	if (rc != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free (zErrMsg);
		return -1;
	}

	return 0;
}

int sel_from_db_by_browse (sqlite * db, const char *sql, char **result,struct g_variable *g_var)
{
	int rc = 0;
	char *zErrMsg = NULL;

	g_var->temp_str=result;	
	rc = sqlite_exec (db, sql, db_query_db_by_browse_callback, &g_var, &zErrMsg);
	if (rc != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free (zErrMsg);
		return -1;
	}

	return 0;
}

/******************************************************************************
 *sel_from_db
 *
 *Description
 *       Get  Data from database,and packet it ,put them to buffer
 *Parameters:
 *        db -- The database handle.
 *        sql-- The sql sentence
 *       result-- put the results to it.
 *
 ******************************************************************************/
int sel_from_db (sqlite * db, const char *sql, char **result, int special_agent,struct g_variable *g_var)
{
	int rc=0, fd=-1;
	char *zErrMsg = NULL;
	char *pFileCache = NULL;
	FILE *fp=NULL;
	struct stat f_stat;
	
	pFileCache=malloc(256);
	if(!pFileCache)
		return -1;
		#ifdef __DEBUG__
			printf("%s: %d\n", __FUNCTION__, __LINE__);
		#endif		
	sprintf(pFileCache, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
	fd=mkstemp(pFileCache);
	if(fd==-1){
		free(pFileCache);
		pFileCache=NULL;
		return -1;
	}
	close(fd);	
	g_var->temp_str=&pFileCache;	
	if(special_agent==XBOX360){
		rc = sqlite_exec (db, sql, db_query_data_callback_xbox360, &g_var, &zErrMsg);
		if (rc != SQLITE_OK) {
			if (zErrMsg)
				sqlite_free (zErrMsg);
			free(pFileCache);
			pFileCache=NULL;
			return -1;				
		}
	}
	else{
		rc = sqlite_exec (db, sql, db_query_data_callback, &g_var, &zErrMsg);
		if (rc != SQLITE_OK) {
			if (zErrMsg)
				sqlite_free (zErrMsg);
			free(pFileCache);
			pFileCache=NULL;
			return -1;	
		}
	}

	if(stat(pFileCache,&f_stat) < 0 || f_stat.st_size==0){
		remove(pFileCache);
		free(pFileCache);		
		return 1;
	}
		#ifdef __DEBUG__
			printf("%s: %d\n", __FUNCTION__, __LINE__);
		#endif
	*result=(char *)malloc(f_stat.st_size+1);
   	if(!*result){
   		remove(pFileCache);
		free(pFileCache);
		pFileCache=NULL;      		
		return 1;
	}

	memset(*result,0,f_stat.st_size+1);
	fp=fopen(pFileCache,"rt");
	if(fp){
		fread(*result,1,f_stat.st_size,fp);
   		fclose(fp);
   		remove(pFileCache);
		free(pFileCache);
		pFileCache=NULL;      		
   	}
   	else{
   		free(*result);
   		*result=NULL;
   		remove(pFileCache);
		free(pFileCache);
		pFileCache=NULL;      		
   		return -1;
   	}
	return 0;
}

/*
 ** The callback function for db_query
 */
static int db_query_count_callback (void *pUser,	/* Pointer to the QueryResult structure */
						 int nArg,	/* Number of columns in this result row */
						 char **azArg,	/* Text of data in all columns */
						 char **NotUsed	/* Names of the columns */
	)
{
	struct g_variable **pResult=(struct g_variable **)pUser;

	(*pResult)->g_count = strtoul (azArg[0], NULL, 10);
	(*pResult)->g_totalmatches +=(*pResult)->g_count;//Number of macthes.

	return 0;
}

static int db_query_db_by_browse_callback (void *pUser,	/* Pointer to the QueryResult structure */
								int nArg,	/* Number of columns in this result row */
								char **azArg,	/* Text of data in all columns */
								char **NotUsed	/* Names of the columns */
	)
{
	struct g_variable **g_var=(struct g_variable **)pUser;
	char **pResult = (*g_var)->temp_str;
	int n_pResultLen = 0;
	int n_len;
	
	if (azArg == 0)
		return 0;
	if (nArg > 3)
		return 0;
	if (nArg == 1) {
		n_pResultLen =
			strlen (*pResult) + 15 + strlen (NotUsed[0]) + strlen (azArg[0]);
		if (n_pResultLen >= (*g_var)->g_nmallocLen) {
			if ((*pResult =
				 realloc (*pResult, ((*g_var)->g_nmallocLen + 1000))) == NULL)
				return 1;		//directory packet
			else
				(*g_var)->g_nmallocLen += 1000;
		}
		sprintf ((*pResult)+strlen(*pResult), " and %s<>\"%s\" ",  NotUsed[0],
				 azArg[DATABASE_INDEX_TITLE]);
	}
	else if (nArg == 2) {
		n_pResultLen =
			strlen (*pResult) + 15 + strlen (NotUsed[1]) + strlen (azArg[1]);
		if (n_pResultLen >(*g_var)->g_nmallocLen) {
			n_len = (*g_var)->g_nmallocLen + 1000;
			if ((*pResult = realloc (*pResult, n_len)) == NULL)
				return 1;		//directory packet
			else
				(*g_var)->g_nmallocLen += 1000;
		}
		sprintf (*pResult+strlen(*pResult), " and %s<>\"%s\" ", NotUsed[1], azArg[1]);
	}
	else if (nArg == 3) {
		n_pResultLen = strlen (*pResult) + 15 + strlen (NotUsed[2]) + strlen (azArg[2]);
		if (n_pResultLen > (*g_var)->g_nmallocLen) {
			n_len = (*g_var)->g_nmallocLen + 1000;
			if ((*pResult = realloc (*pResult, n_len)) == NULL)
				return 1;		//directory packet
			else
				(*g_var)->g_nmallocLen += 1000;
		}
		sprintf (*pResult+strlen(*pResult), " and %s<>\"%s\" ",  NotUsed[2], azArg[DATABASE_INDEX_FILENAME]);
	}

	return 0;
}

int strtoasc (char *psrc, char *pdst)
{
	//int nlen = 0;
	char *p0 = NULL;
	char *p1 = NULL;
	char tmp = '\0';
	p0 = psrc;
	p1 = pdst;
	
	while (p0 && *p0 && p1) {
		tmp = (*p0) & 0x0f;
		switch (tmp) {
		case 0x01:
			*p1 = '1';
			break;
		case 0x02:
			*p1 = '2';
			break;
		case 0x03:
			*p1 = '3';
			break;
		case 0x04:
			*p1 = '4';
			break;
		case 0x05:
			*p1 = '5';
			break;
		case 0x06:
			*p1 = '6';
			break;
		case 0x07:
			*p1 = '7';
			break;
		case 0x08:
			*p1 = '8';
			break;
		case 0x09:
			*p1 = '9';
			break;
		case 0x0a:
			*p1 = 'a';
			break;
		case 0x0b:
			*p1 = 'b';
			break;
		case 0x0c:
			*p1 = 'c';
			break;
		case 0x0d:
			*p1 = 'd';
			break;
		case 0x0e:
			*p1 = 'e';
			break;
		case 0x0f:
			*p1 = 'f';
			break;
		default:
			break;
		}
		p1 = p1 + 1;
		tmp = (*p0) & 0xf0;
		switch (tmp) {
		case 0x10:
			*p1 = '1';
			break;
		case 0x20:
			*p1 = '2';
			break;
		case 0x30:
			*p1 = '3';
			break;
		case 0x40:
			*p1 = '4';
			break;
		case 0x50:
			*p1 = '5';
			break;
		case 0x60:
			*p1 = '6';
			break;
		case 0x70:
			*p1 = '7';
			break;
		case 0x80:
			*p1 = '8';
			break;
		case 0x90:
			*p1 = '9';
			break;
		case 0xa0:
			*p1 = 'a';
			break;
		case 0xb0:
			*p1 = 'b';
			break;
		case 0xc0:
			*p1 = 'c';
			break;
		case 0xd0:
			*p1 = 'd';
			break;
		case 0xe0:
			*p1 = 'e';
			break;
		case 0xf0:
			*p1 = 'f';
			break;
		default:
			break;
		}
		p0 = p0 + 1;
		p1 = p1 + 1;

	}
	return 0;
}

int asctostr (char *psrc, char *pdst)
{
	//int nlen = 0;
	int nflag = 0;
	char *p0 = NULL;
	char *p1 = NULL;
	//char tmp = '\0';
	p0 = psrc;
	p1 = pdst;
	while (p0 && *p0 && p1) {
		switch (*p0) {
		case '1':
			if (0 == nflag) {
				*p1 = 0x01;
				nflag = 1;
			}
			else {
				*p1 = 0x10 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case '2':
			if (0 == nflag) {
				*p1 = 0x02;
				nflag = 1;
			}
			else {
				*p1 = 0x20 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case '3':
			if (0 == nflag) {
				*p1 = 0x03;
				nflag = 1;
			}
			else {
				*p1 = 0x30 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case '4':
			if (0 == nflag) {
				*p1 = 0x04;
				nflag = 1;
			}
			else {
				(*p1) = 0x40 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case '5':
			if (0 == nflag) {
				*p1 = 0x05;
				nflag = 1;
			}
			else {
				*p1 = 0x50 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case '6':
			if (0 == nflag) {
				*p1 = 0x06;
				nflag = 1;
			}
			else {
				*p1 = 0x60 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case '7':
			if (0 == nflag) {
				*p1 = 0x07;
				nflag = 1;
			}
			else {
				*p1 = 0x70 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case '8':
			if (0 == nflag) {
				*p1 = 0x08;
				nflag = 1;
			}
			else {
				*p1 = 0x80 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case '9':
			if (0 == nflag) {
				*p1 = 0x09;
				nflag = 1;
			}
			else {
				*p1 = 0x90 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case 'a':
			if (0 == nflag) {
				*p1 = 0x0a;
				nflag = 1;
			}
			else {
				*p1 = 0xa0 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case 'b':
			if (0 == nflag) {
				*p1 = 0x0b;
				nflag = 1;
			}
			else {
				*p1 = 0xb0 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case 'c':
			if (0 == nflag) {
				*p1 = 0x0c;
				nflag = 1;
			}
			else {
				*p1 = 0xc0 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case 'd':
			if (0 == nflag) {
				*p1 = 0x0d;
				nflag = 1;
			}
			else {
				*p1 = 0xd0 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case 'e':
			if (0 == nflag) {
				*p1 = 0x0e;
				nflag = 1;
			}
			else {
				*p1 = 0xe0 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		case 'f':
			if (0 == nflag) {
				*p1 = 0x0f;
				nflag = 1;
			}
			else {
				*p1 = 0xf0 | (*p1);
				p1 = p1 + 1;
				nflag = 0;
			}
			break;
		default:
			nflag = 0;
			break;
		}
		p0 = p0 + 1;
	}
	return 0;
}

int common_sprintf (char *result, char *p0, char *p1, char *p2, int special_agent,struct g_variable *g_var)
{
	//int nRet = 0; 
	int rc = 0;
	int nlen =0;
	char sql_cmd[1024] = { 0 };
	char *extrastr = NULL;
	char *pparentid = NULL;
	//char* zErrMsg =NULL;
	char* pcontainerid = NULL;
	
	pparentid = xml_encode (g_var->g_parentid);
	rc = MakeCountSQLSentence (g_var->g_Db, g_var->select_type, p0, sql_cmd, extrastr,g_var);
	if(special_agent!=XBOX360) {
		sprintf (result,
			 "container id=\"%s\" restricted=\"1\" parentID=\"%s\" searchable=\"1\" childCount=\"%lu\"&gt;"
			 "\n&lt;dc:title&gt;%s&lt;/dc:title&gt;"
			 "\n&lt;upnp:class&gt;object.container&lt;/upnp:class&gt;", p0,
			 pparentid, g_var->g_totalmatches, p2);
#if 0
		if(g_filterflag&0x0080)
			sprintf(result ,"%s\n&lt;upnp:writeStatus&gt;UNKNOWN&lt;/upnp:writeStatus&gt;",result);
#endif
		strcat (result, "\n&lt;/container&gt;\n&lt;");
	}
	else {
		nlen = 2*strlen(p0)+1;
		pcontainerid = (char *)malloc(nlen);
		memset(pcontainerid,0,nlen);
		strtoasc(p0,pcontainerid);
		sprintf(result,"container id=\"%s\" parentID=\"%s\" restricted=\"1\" &gt;&lt;dc:title&gt;%s&lt;/dc:title&gt;",p0,pparentid,p2);
		if(g_var->g_filterflag&0x0002){
			strcat(result,"\n&lt;upnp:artist&gt;"UNKNOWN_STR"&lt;/upnp:artist&gt;");
		}
		strcat(result,"\n&lt;upnp:class&gt;");
		strcat(result, g_var->g_upnpclass);
		strcat(result, "&lt;/upnp:class&gt;&lt;/container&gt;\n&lt;");
		if(pcontainerid) {
			free(pcontainerid);
			pcontainerid = NULL;
		}
	}
	g_var->g_totalmatches = 0;
	if (pparentid) {
		free (pparentid);
		pparentid = NULL;
	}

	return 0;
}


int makeitempacket (char *pResult, struct ItemStruct *pitem, int special_agent,struct g_variable *g_var)
{
	char item_end[50] = { 0 };
	char *pitem_begin = NULL;
	char *pitem_upnp = NULL;
	char *pitem_res = NULL;
	int nalocmem = 0;
	char tmp_str[24]={0};
	char *p=NULL;
	char title[256]={0};
	int ret=0;

#ifdef 	__DEBUG__
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	nalocmem = 256;
	if (pitem->item_id)
		nalocmem += strlen (pitem->item_id);
	if(pitem->refID)
	   nalocmem += strlen(pitem->refID);
	if (pitem->parentID)
		nalocmem += strlen (pitem->parentID);
	pitem_begin = malloc (nalocmem);
	memset (pitem_begin, 0, nalocmem);
	nalocmem = 1500;
	if (pitem->title) {
		strcpy(title,pitem->title);
		p=strrchr(title,'.');
		if(p) {
			*p=0;
		}
		nalocmem += strlen (title);
	}
	if (pitem->objectitem)
		nalocmem += strlen (pitem->objectitem);
	if (pitem->artist){
		nalocmem += strlen (pitem->artist);
	}
	if (pitem->genre)
		nalocmem += strlen (pitem->genre);
	if (pitem->album)
		nalocmem += strlen (pitem->album);
	if (pitem->duration)
		nalocmem += strlen (pitem->duration);
	if (pitem->resolution)
		nalocmem += strlen (pitem->resolution);
	if (pitem->item_year)
		nalocmem += strlen (pitem->item_year);
	if (pitem->item_month)
		nalocmem += strlen (pitem->item_month);
	if (pitem->item_day)
		nalocmem += strlen (pitem->item_day);
	pitem_upnp = malloc (nalocmem);
	memset (pitem_upnp, 0, nalocmem);
	
	nalocmem = 2048;
	if (pitem->protocolInfo){
		nalocmem += strlen (pitem->protocolInfo);
	}
	/*
	   if(pitem->importUri)
	   nalocmem += strlen(pitem->importUri);
	 */
	if (pitem->item_address)
		nalocmem += strlen (pitem->item_address);
	pitem_res = malloc (nalocmem);
	memset (pitem_res, 0, nalocmem);
	if((g_var->g_filterflag & 0x0080)&&(g_var->g_filterflag!=0xFFFF))
		sprintf (pitem_begin,"item id=\"%s\" refID=\"%s\" parentID=\"%s\" restricted=\"1\" &gt;", pitem->item_id,pitem->refID,pitem->parentID);
	else
		sprintf (pitem_begin,"item id=\"%s\" parentID=\"%s\" restricted=\"1\" &gt;", pitem->item_id,pitem->parentID);
	sprintf(pitem_upnp, "&lt;dc:title&gt;%s&lt;/dc:title&gt;\n", title);
	if (g_var->g_filterflag & 0x4000) {	//"dc:creator"
		if (pitem->artist && (*pitem->artist))
			sprintf (pitem_upnp+strlen(pitem_upnp),
					 "&lt;dc:creator&gt;%s&lt;/dc:creator&gt;\n",
					  pitem->artist);
		else
			sprintf (pitem_upnp+strlen(pitem_upnp),
					 "&lt;dc:creator&gt;%s&lt;/dc:creator&gt;\n",
					 UNKNOWN_STR);
	}
	if (g_var->g_filterflag & 0x0002){	//"upnp:artist"
		if (pitem->artist && (*pitem->artist))
			sprintf (pitem_upnp+strlen(pitem_upnp),
					 "&lt;upnp:artist&gt;%s&lt;/upnp:artist&gt;\n",
					  pitem->artist);
		else
			sprintf (pitem_upnp+strlen(pitem_upnp),
					 " &lt;upnp:artist&gt;%s&lt;/upnp:artist&gt;\n",
					 UNKNOWN_STR);
	}
	if (g_var->g_filterflag & 0x0004){	//upnp:genre
		if (pitem->genre && (*pitem->genre))
			sprintf (pitem_upnp+strlen(pitem_upnp),
					 "&lt;upnp:genre&gt;%s&lt;/upnp:genre&gt;\n",
					  pitem->genre);
		else
			sprintf (pitem_upnp+strlen(pitem_upnp),
					 "&lt;upnp:genre&gt;%s&lt;/upnp:genre&gt;\n",
					 UNKNOWN_STR);
	}
	if (g_var->g_filterflag & 0x0008){	// upnp:album
		if (pitem->album && (*pitem->album))
			sprintf (pitem_upnp+strlen(pitem_upnp),
					 "&lt;upnp:album&gt;%s&lt;/upnp:album&gt;\n",
					  pitem->album);
		else
			sprintf (pitem_upnp+strlen(pitem_upnp),
					 " &lt;upnp:album&gt;%s&lt;/upnp:album&gt;\n",
					 UNKNOWN_STR);
	}
	if (g_var->g_filterflag & 0x0010)	//dc:date
		sprintf (pitem_upnp+strlen(pitem_upnp), " &lt;dc:date&gt;%s-%s-%s&lt;/dc:date&gt;\n",
				  pitem->item_year, pitem->item_month,
				 pitem->item_day);

	
	/*if (g_var->g_filterflag & 0x0040)	//upnp:storageMedium
		sprintf (pitem_upnp,
				 "%s &lt;upnp:storageMedium&gt;UNKNOWN&lt;/upnp:storageMedium&gt;\n",
				 pitem_upnp);
	*/
#if 0
	if(g_var->g_filterflag&0x0080)//upnp:writeStatus
		sprintf(pitem_upnp,"%s &lt;upnp:writeStatus&gt;UNKNOWN&lt;/upnp:writeStatus&gt;\n",pitem_upnp);
#endif
	if (g_var->g_filterflag & (0x1000 | 0x0100 | 0x0200))
		sprintf (pitem_res, "&lt;res ");
	if(special_agent == XBOX360) {        
		if(g_var->g_filterflag & 0x0400){//Duration
			if(strcmp(pitem->duration, "00:00:00"))
				sprintf (pitem_res+strlen(pitem_res), " duration=\"%s.000\" ",  pitem->duration);
		}
	}
	else{
		if((g_var->g_filterflag & 0x0400 )&&(strcmp(pitem->objectitem,"imageItem.photo")!=0))//Duration
			sprintf (pitem_res+strlen(pitem_res), " duration=\"%s\" ", pitem->duration);		
	}
	if(g_var->g_filterflag & 0x2000 && strcmp(pitem->resolution,UNKNOWN_STR))
		sprintf (pitem_res+strlen(pitem_res), " resolution=\"%s\" ", pitem->resolution);
		
	if (g_var->g_filterflag & (0x1000 | 0x0100 | 0x0200)){
		sprintf (pitem_res+strlen(pitem_res), " protocolInfo=\"%s\"", pitem->protocolInfo);
	}
	if (g_var->g_filterflag & 0x0100)	//size
		sprintf (pitem_res+strlen(pitem_res), " size=\"%s\"", pitem->size);
		
#if 0		
	if((g_var->g_filterflag&0x0800)&&(g_var->g_nflag !=2)) { //Bitrate
		if(g_var->g_filterflag&0x0800)
			sprintf(pitem_res,"%s bitrate=\"0\"",pitem_res);
	}
	
	if(g_var->g_filterflag&0x0200)//importUri
		sprintf(pitem_res,"%s importUri=\"%s\"",pitem_res,pitem->importUri); 
#endif	 
	//if(g_var->g_filterflag&(0x1000|0x0100|0x0400|0x0800|0x0200))
	if (g_var->g_filterflag & (0x1000 | 0x0100 | 0x0200)) // desc|size|importUri
		sprintf (pitem_res+strlen(pitem_res), " &gt;%s&lt;/res&gt;\n",
				 pitem->item_address);
	if (g_var->g_filterflag & (0x1000 | 0x0100 | 0x0200)){
		strcpy(tmp_str, pitem->resolution);				
		p=strchr(tmp_str, 'x');
		if(p){		
			int width=0, higth=0;

			*p=0;
			p++;
			width=atoi(tmp_str);
			higth=atoi(p);
			if(width<=160 && higth<=160){//Thumbnail
				char sm_url[256];
				char *p=NULL;
				int len;

				strcpy(sm_url,pitem->item_address);
				p=strstr(pitem->item_address,VIRTUAL_DIR);
				p=p+strlen(VIRTUAL_DIR);
				len=strlen(pitem->item_address)-strlen(p);
				sm_url[len]='\0';
				strcat(sm_url,VIRTUAL_PIC);
				strcat(sm_url,p);
				if(strstr(pitem->protocolInfo,"DLNA.ORG_PN=JPEG")) {	
					sprintf (pitem_res, "%s &lt;res ",pitem_res);
					if(g_var->g_filterflag & 0x2000)
						sprintf (pitem_res+strlen(pitem_res), " resolution=\"%s\" ", pitem->resolution); 
					if (g_var->g_filterflag & 0x0100)	//size
						sprintf (pitem_res+strlen(pitem_res), " size=\"%s\"",  pitem->size);
					sprintf (pitem_res+strlen(pitem_res), " %s","protocolInfo=\"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM;DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=00F00000000000000000000000000000\"");
					sprintf (pitem_res+strlen(pitem_res), " &gt;%s&lt;/res&gt;\n", sm_url);
				}
#ifdef __PNG_DLNA_SUPPORT__			
				else if(strstr(pitem->protocolInfo,"DLNA.ORG_PN=PNG")) {
					sprintf (pitem_res+strlen(pitem_res), "%s"," &lt;res ");
					if(g_var->g_filterflag & 0x2000)
						sprintf (pitem_res+strlen(pitem_res), " resolution=\"%s\" ",  pitem->resolution);
					sprintf (pitem_res+strlen(pitem_res), "%s"," protocolInfo=\"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM;DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=00F00000000000000000000000000000\"");
					sprintf (pitem_res+strlen(pitem_res), " size=\"%s\"",  pitem->size);
					sprintf (pitem_res+strlen(pitem_res), " &gt;%s&lt;/res&gt;\n", sm_url);
					if(strstr(pitem->protocolInfo,"PNG_TN")!=NULL) {	
						memset(sm_url,0,sizeof(sm_url));
						p=NULL;
						strcpy(sm_url,pitem->item_address);
						p=strstr(pitem->item_address,VIRTUAL_DIR);
						p=p+strlen(VIRTUAL_DIR);
						len=strlen(pitem->item_address)-strlen(p);
						sm_url[len]='\0';
						strcat(sm_url,VIRTUAL_PNG);
						strcat(sm_url,p);	
						sprintf (pitem_res+strlen(pitem_res), "%s"," &lt;res ");
						if(g_var->g_filterflag & 0x2000)
							sprintf (pitem_res+strlen(pitem_res), " resolution=\"%s\" ", pitem->resolution); 
						sprintf (pitem_res+strlen(pitem_res), "%s"," protocolInfo=\"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN;DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=00F00000000000000000000000000000\"");
						sprintf (pitem_res+strlen(pitem_res), " &gt;%s&lt;/res&gt;\n", sm_url);
					}
				}
#endif			
			}
			if((pitem->protocolInfo!=NULL)&&((strstr(pitem->protocolInfo,"JPEG_LRG;")!=NULL)||(strstr(pitem->protocolInfo,"JPEG_MED;")!=NULL)))	{
				char sm_url[256];
				char *p=NULL;
				int len,new_width=JPEG_SMALL_WIDTH,new_height=JPEG_SMALL_HEIGHT;

				strcpy(sm_url,pitem->item_address);
				p=strstr(pitem->item_address,VIRTUAL_DIR);
				p=p+strlen(VIRTUAL_DIR);
				len=strlen(pitem->item_address)-strlen(p);
				sm_url[len]='\0';
				strcat(sm_url,VIRTUAL_LM);
				strcat(sm_url,p);
				ret=GetScaledPictureResolution(width,higth,&new_width,&new_height);
				if(!ret) {	
					sprintf (pitem_res+strlen(pitem_res), "%s"," &lt;res ");
					if(g_var->g_filterflag & 0x2000)
						sprintf (pitem_res+strlen(pitem_res), " resolution=\"%dx%d\" ", new_width,new_height); 
					if (g_var->g_filterflag & 0x0100)	//size
					{
						char *pThumb=NULL;
						struct stat filestat;
						if(pitem->item_address){

							p=strrchr(pitem->item_address,'/');
							if(p)
								p++;
							else
								p=(char *)pitem->item_address;
							pThumb=(char*)malloc(strlen(p)+strlen(cache_folder_path)+2);
							if(!pThumb)
							{
								if (pitem_begin) {
									free (pitem_begin);
									pitem_begin = NULL;
								}
								if (pitem_upnp) {
									free (pitem_upnp);
									pitem_upnp = NULL;
								}
								if (pitem_res) {
									free (pitem_res);
									pitem_res = NULL;
								}
								return -1;
							}
							sprintf(pThumb, "%s/%s", cache_folder_path, p);
							if(!stat(pThumb,&filestat))
								sprintf (pitem_res+strlen(pitem_res), " size=\"%llu\"",filestat.st_size);
							if(pThumb)
								free(pThumb);
						}
					}
					sprintf (pitem_res+strlen(pitem_res), "%s"," protocolInfo=\"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM;DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=00F00000000000000000000000000000\"");
					sprintf (pitem_res+strlen(pitem_res), " &gt;%s&lt;/res&gt;\n", sm_url);
				}
			}	
		}
	}
	if((special_agent==XBOX360) && (strcmp(pitem->objectitem,"videoItem.movie")==0))
		strcat(pitem_res,"&lt;upnp:class&gt;object.item.videoItem&lt;/upnp:class&gt;\n");
	else{
		strcat(pitem_res,"&lt;upnp:class&gt;object.item.");
		strcat(pitem_res,pitem->objectitem);
		strcat(pitem_res,"&lt;/upnp:class&gt;\n");
	}
	sprintf (item_end, "&lt;/item&gt;\n&lt;");//imageItem.photo
	sprintf (pResult, "%s%s%s%s", pitem_begin, pitem_upnp, pitem_res, item_end);
	if (pitem_begin) {
		free (pitem_begin);
		pitem_begin = NULL;
	}
	if (pitem_upnp) {
		free (pitem_upnp);
		pitem_upnp = NULL;
	}
	if (pitem_res) {
		free (pitem_res);
		pitem_res = NULL;
	}
	return 0;
}
#ifdef __PNG_DLNA_SUPPORT__	
int getPngProtocol(char *protocol,char *p)
{
	int wigth,height;
	char resolution[15];
	char *ext=NULL;
	strcpy(resolution,p);
	ext=strchr(resolution,'x');
	if(ext==NULL)
		return -1;
	height=atoi(ext+1);
	ext=0;
	wigth=atoi(resolution);
	if(wigth==48 && height==48)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",png_protocolinfo[3].ch_protocolinfo, DLNA_ORG_PN,png_protocolinfo[3].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 3;
	}
	else if(wigth==120 && height==120)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",png_protocolinfo[2].ch_protocolinfo, DLNA_ORG_PN,png_protocolinfo[2].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 2;	
	}	
	else if(wigth<=160 && height<=160)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",png_protocolinfo[1].ch_protocolinfo, DLNA_ORG_PN,png_protocolinfo[1].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 1;
	}
	else if(wigth<=4096 && height<=4096)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",png_protocolinfo[0].ch_protocolinfo, DLNA_ORG_PN,png_protocolinfo[0].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);

		return 0;
	}
	else 
		return -1;
}
#endif
int getJpegProtocol(char *protocol,char *p)
{
	int wigth,height;
	char resolution[15];
	char *ext=NULL;
	strcpy(resolution,p);
	ext=strchr(resolution,'x');
	if(ext==NULL)
		return -1;
	height=atoi(ext+1);
	ext=0;
	wigth=atoi(resolution);
	if(wigth==48 && height==48)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",jpg_protocolinfo[5].ch_protocolinfo, DLNA_ORG_PN,jpg_protocolinfo[5].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 5;
	}
	else if(wigth==120 && height==120)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",jpg_protocolinfo[4].ch_protocolinfo, DLNA_ORG_PN,jpg_protocolinfo[4].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 4;	
	}	
	else if(wigth<=160 && height<=160)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",jpg_protocolinfo[3].ch_protocolinfo, DLNA_ORG_PN,jpg_protocolinfo[3].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 3;
	}
	else if(wigth<=640 && height<=480)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",jpg_protocolinfo[2].ch_protocolinfo, DLNA_ORG_PN,jpg_protocolinfo[2].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 2;
	}
	else if(wigth<=1024 && height<=768)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",jpg_protocolinfo[1].ch_protocolinfo, DLNA_ORG_PN,jpg_protocolinfo[1].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 1;
	}
	else if(wigth<=4096 && height<=4096)
	{
		sprintf(protocol,"%s%s=%s;%s=%s;%s=%s;%s=%s",jpg_protocolinfo[0].ch_protocolinfo, DLNA_ORG_PN,jpg_protocolinfo[0].dlna_type,DLNA_ORG_OP, DLNA_ORG_OP_VAL,  DLNA_ORG_CI, DLNA_ORG_CI_VAL,DLNA_ORG_FLAGS, DLNA_ORG_FLAGS_PIC);
		return 0;
	}
	else 
		return -1;
}
static int db_query_data_callback (void *pUser,	/* Pointer to the QueryResult structure */
						int nArg,	/* Number of columns in this result row */
						char **azArg,	/* Text of data in all columns */
						char **NotUsed	/* Names of the columns */	)
{
		
	return(db_query_data_callback_flag(pUser, nArg, azArg, NotUsed, STANDARD));	
}

//Used global variables: g_root, g_parentid, g_Db

static int db_query_data_callback_flag (void *pUser,	/* Pointer to the QueryResult structure */
						int nArg,	/* Number of columns in this result row */
						char **azArg,	/* Text of data in all columns */
						char **NotUsed,	/* Names of the columns */
						int flag )
{

	struct g_variable **g_var=(struct g_variable **)pUser;
	char **pCacheFile = (*g_var)->temp_str;
	char *pTmpBuf = NULL;
	char *p = NULL;
	char *p2 = NULL;
	char *pSId = NULL;
	char *pMId = NULL;
	char *pDId = NULL;
	char *pDId0 = NULL;
	char *pDId1 = NULL;
	char *pDId2 = NULL;
	char *pDArtist = NULL;
	char *pDGenre = NULL;
	char *pDAlbum = NULL;
	char *pDParentid = NULL;
	char *pDuration = NULL;
	char *pResolution = NULL;
	struct ItemStruct item;
	int i = 0, ret=0;
	int mcount = 0;
	int nflag = 0;
	int nfilterflag = 0;
	int ch = '.';
	int ch2 = '/';
	char mediaurl[256]={0};
	FILE *fp=NULL;
	memset(&item,0,sizeof(item));	
	if (azArg == 0 || !(*pCacheFile))
		return 0;
	fp=fopen(*pCacheFile, "at");
	if(!fp)
		return 0;
#ifdef 	__DEBUG__
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif
	sprintf (mediaurl, "http://%s:%d", media.InternalIPAddress, media.upnp_port);
	if (nArg == 1 || strcasecmp (azArg[0], "dir") == 0) {
#ifdef 	__DEBUG__
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
		if (nArg == 1) {
			if (strcasecmp (NotUsed[0], "Root") == 0) {
				p2 = strrchr (azArg[0], ch2);
				p2 += 1;
			}
			else {
				p2 = azArg[0];
			}
			if ((pSId = (char *) malloc (strlen ((*g_var)->g_root) + strlen (azArg[0]) + 3)) == NULL){
				fclose(fp);
				return 1;
			}
			sprintf (pSId, "%s/%s", (*g_var)->g_root, azArg[0]);
			pDId = xml_encode (pSId);
			free (pSId);
			pSId = NULL;
			pDId1 = xml_encode (p2);
			pDId0 = xml_encode ((*g_var)->g_root);
			if ((pTmpBuf = (char *) malloc (Packet_ContentDirectory_Len +
									  strlen (pDId) + strlen (pDId0) +
									  strlen (pDId1) + strlen ((*g_var)->g_parentid) +
									  1)) == NULL) {
				if (pDId != NULL) {
					free (pDId);
					pDId = NULL;
				}
				if (pDId1 != NULL) {
					free (pDId1);
					pDId1 = NULL;
				}
				if (pDId0 != NULL) {
					free (pDId0);
					pDId0 = NULL;
				}
				fclose(fp);
				return 1;	//directory packet
			}
			common_sprintf (pTmpBuf, pDId, pDId0, pDId1, flag,*g_var);
		}
		else {
			if ((pSId = (char *) malloc (strlen ((*g_var)->g_root) + strlen (azArg[1]) + strlen (azArg[2]) + 5)) == NULL){
				fclose(fp);
				return 1;
			}
			sprintf (pSId, "%s/%s/%s", (*g_var)->g_root, azArg[1], azArg[2]);
			pDId = xml_encode (pSId);
			free (pSId);
			pSId = NULL;
			pDId0 = xml_encode ((*g_var)->g_root);
			pDId1 = xml_encode (azArg[2]);
			if ((pTmpBuf = (char *) malloc (Packet_ContentDirectory_Len +
									  strlen ((*g_var)->g_parentid) + strlen (pDId) +
									  strlen (pDId0) + strlen (pDId1) + 1)) == NULL) {
				if (pDId != NULL) {
					free (pDId);
					pDId = NULL;
				}
				if (pDId1 != NULL) {
					free (pDId1);
					pDId1 = NULL;
				}
				if (pDId0 != NULL) {
					free (pDId0);
					pDId0 = NULL;
				}
				fclose(fp);
				return 1;
			}
			common_sprintf (pTmpBuf, pDId, pDId0, pDId1, flag,*g_var);
		}
	}
	else if (nArg == 2) {
#ifdef 	__DEBUG__
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
		if ((pSId = (char *) malloc (strlen ((*g_var)->g_root) + strlen (azArg[0]) + strlen (azArg[1]) + 5)) == NULL){
			fclose(fp);
			return 1;
		}
		sprintf (pSId, "%s/%s/%s", (*g_var)->g_root, azArg[0], azArg[1]);
		pDId = xml_encode (pSId);
		free (pSId);
		pSId = NULL;
		pDId0 = xml_encode ((*g_var)->g_root);
		pDId1 = xml_encode (azArg[1]);
		if ((pTmpBuf =
				 (char *) malloc (Packet_ContentDirectory_Len +
								  strlen ((*g_var)->g_parentid) + strlen (pDId) +
								  strlen (pDId0) + strlen (pDId1) + 1)) == NULL) {
			if (pDId != NULL) {
				free (pDId);
				pDId = NULL;
			}
			if (pDId1 != NULL) {
				free (pDId1);
				pDId1 = NULL;
			}
			if (pDId0 != NULL) {
				free (pDId0);
				pDId0 = NULL;
			}
			fclose(fp);
			return 1;		//directory packet
		}
		common_sprintf (pTmpBuf, pDId, pDId0, pDId1, flag,*g_var);
	}
	else if (nArg == 3) {
#ifdef 	__DEBUG__
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
		if ((pSId = (char *) malloc (strlen ((*g_var)->g_root) + strlen (azArg[0]) +
								  strlen (azArg[1]) + strlen (azArg[2]) +
								  5)) == NULL){
			fclose(fp);
			return 1;
		}
		sprintf (pSId, "%s/%s/%s/%s", (*g_var)->g_root, azArg[0], azArg[1], azArg[DATABASE_INDEX_FILENAME]);
		pDId = xml_encode (pSId);
		free (pSId);
		pSId = NULL;
		pDId0 = xml_encode ((*g_var)->g_root);
		pDId1 = xml_encode (azArg[2]);
		if ((pTmpBuf = (char *) malloc (Packet_ContentDirectory_Len + strlen ((*g_var)->g_parentid) + strlen (pDId) + strlen (pDId0) + strlen (pDId1) + 1)) == NULL) {
			if (pDId != NULL) {
				free (pDId);
				pDId = NULL;
			}
			if (pDId1 != NULL) {
				free (pDId1);
				pDId1 = NULL;
			}
			if (pDId0 != NULL) {
				free (pDId0);
				pDId0 = NULL;
			}
			fclose(fp);
			return 1;		//directory packet
		}
		common_sprintf (pTmpBuf, pDId, pDId0, pDId1, flag,*g_var);
	}
	else {
		int file_type=MEDIA_TYPE_NONE;

#ifdef 	__DEBUG__
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
		if(!strcmp(azArg[DATABASE_INDEX_TITLE],"audio"))
			file_type=MEDIA_TYPE_MUSIC;
		else if(!strcmp(azArg[DATABASE_INDEX_TITLE],"video"))
			file_type=MEDIA_TYPE_VIDEO;
		else if(!strcmp(azArg[DATABASE_INDEX_TITLE],"photo"))
			file_type=MEDIA_TYPE_PHOTO;
		p = strrchr (azArg[DATABASE_INDEX_FILENAME], ch);
		if(!p){
			fclose(fp);
			return 1;
		}
		p = p + 1;
		nfilterflag = (*g_var)->g_filterflag;
		for (i = 0; i < sizeof (media_protocolinfo) / sizeof (struct MediaProtocolInfo); i++) {
			if (strcasecmp (media_protocolinfo[i].ch_postfix, p) == 0) {
				if(file_type==MEDIA_TYPE_MUSIC && strstr(media_protocolinfo[i].ch_objectitem, "audioItem."))
					;
				else if(file_type==MEDIA_TYPE_VIDEO && strstr(media_protocolinfo[i].ch_objectitem, "videoItem."))
					;
				else if(file_type==MEDIA_TYPE_PHOTO && strstr(media_protocolinfo[i].ch_objectitem, "imageItem."))
					;
				else				
					continue;				
				nflag = 1;
				if( i==SEQ_AVI_PROTOCOL )
					(*g_var)->g_filterflag = (*g_var)->g_filterflag|0x0400;
				break;
			}
		}
		if (nflag == 0){
			fclose(fp);
			return 0;
		}
		if ((pSId = (char *) malloc (strlen (mediaurl) + strlen (azArg[DATABASE_INDEX_WEBPATH]) + 4)) == NULL){
			fclose(fp);
			return 1;
		}
		sprintf (pSId, "%s%s", mediaurl, azArg[DATABASE_INDEX_WEBPATH]);
		pMId = blank2str (pSId);
		free (pSId);
		pSId = NULL;
		pDId = xml_encode (pMId);
		if (pMId != NULL) {
			free (pMId);
			pMId = NULL;
		}
		pDParentid = xml_encode ((*g_var)->g_parentid);
		if (strcmp ((*g_var)->g_root, media_catalog_table[4].ch_containerid) == 0) {//Contents
			if ((pSId = (char *) malloc (strlen ((*g_var)->g_root) + strlen (azArg[DATABASE_INDEX_WEBPATH]) + 5)) == NULL){
				fclose(fp);
				return 1;
			}
			sprintf (pSId, "%s%s", (*g_var)->g_root, azArg[DATABASE_INDEX_WEBPATH]);
		}
		else {
			if ((pSId = (char *) malloc (strlen ((*g_var)->g_parentid) + strlen (azArg[DATABASE_INDEX_WEBPATH]) + 5)) == NULL){
				fclose(fp);	
				return 1;
			}
			sprintf (pSId, "%s%s", (*g_var)->g_parentid, azArg[DATABASE_INDEX_WEBPATH]);
		}
		pDId0 = xml_encode (pSId);
		if (pSId != NULL) {
			free (pSId);
			pSId = NULL;
		}
		pDId1 = xml_encode (azArg[DATABASE_INDEX_FILENAME]);//File Name
		pDArtist = xml_encode (azArg[DATABASE_INDEX_ARTIST]);
		pDGenre = xml_encode (azArg[DATABASE_INDEX_GENRE]);
		pDAlbum = xml_encode (azArg[DATABASE_INDEX_ALBUM]);

		pDuration = azArg[DATABASE_INDEX_DURATION];
		
		pResolution = xml_encode (azArg[DATABASE_INDEX_RESOLUTION]);
		mcount = mcount + PACKET_ITEM_LEN;
		
		item.item_id = pDId0;//Object ID
		mcount = mcount + strlen (pDId0);
		
		item.refID = pDId0; //Object ID
		mcount = mcount + strlen(pDId0);
		
		item.parentID = pDParentid;
		mcount = mcount + strlen (pDParentid);
		
		item.title = pDId1;//File Name
		mcount = mcount + strlen (pDId1);
								
		item.protocolInfo = azArg[DATABASE_INDEX_PROTOCOL_INFO];
		mcount = mcount + strlen (item.protocolInfo);
		
		item.size = azArg[DATABASE_INDEX_SIZE];
		mcount = mcount + strlen (azArg[DATABASE_INDEX_SIZE]);
			
		//item.importUri = pDId2;
		//mcount = mcount + strlen(pDId2);
			
		item.item_address = pDId;
		mcount = mcount + strlen (pDId);
		item.objectitem = strdup (media_protocolinfo[i].ch_objectitem);
		mcount = mcount + strlen (media_protocolinfo[i].ch_objectitem);
		
		item.item_year = azArg[DATABASE_INDEX_YEAR];
		mcount = mcount + strlen (azArg[DATABASE_INDEX_YEAR]);
		
		item.item_month = azArg[DATABASE_INDEX_MONTH];
		mcount = mcount + strlen (azArg[DATABASE_INDEX_MONTH]);
		
		item.item_day = azArg[DATABASE_INDEX_DAY];
		mcount = mcount + strlen (azArg[DATABASE_INDEX_DAY]);
		
		item.album = pDAlbum;
		mcount = mcount + strlen (pDAlbum);
		
		item.genre = pDGenre;
		mcount = mcount + strlen (pDGenre);
		
		item.artist = pDArtist;
		mcount = mcount + strlen (pDArtist);
		
		item.duration = pDuration;
		mcount = mcount + strlen (pDuration);
		
		item.resolution = pResolution;
		mcount = mcount + strlen (pResolution);
		
		if ((pTmpBuf = (char *) malloc (mcount)) == NULL) {
			if (pDId != NULL) {
				free (pDId);
				pDId = NULL;
			}
			if (pDId1 != NULL) {
				free (pDId1);
				pDId1 = NULL;
			}
			if (pDId0 != NULL) {
				free (pDId0);
				pDId0 = NULL;
			}
			if (pDId2 != NULL) {
				free (pDId2);
				pDId2 = NULL;
			}
			if (pDParentid != NULL) {
				free (pDParentid);
				pDParentid = NULL;
			}
			if (pDArtist != NULL) {
				free (pDArtist);
				pDArtist = NULL;
			}
			if (pDAlbum != NULL) {
				free (pDAlbum);
				pDAlbum = NULL;
			}
			if (pDGenre != NULL) {
				free (pDGenre);
				pDGenre = NULL;
			}
			if (pResolution != NULL){
				free (pResolution);
				pResolution = NULL;
			}
			if(item.objectitem!=NULL){
				free(item.objectitem);
				item.objectitem=NULL;
			}
				
			fclose(fp);
			return 1;
		}

		ret=makeitempacket (pTmpBuf, &item, flag,*g_var);
#ifdef 	__DEBUG__
		printf("ret: %d\n", ret);
#endif			
		(*g_var)->g_filterflag = nfilterflag;
	}
	if(pTmpBuf){
		if(strlen(pTmpBuf))
			fputs(pTmpBuf, fp);	
		free(pTmpBuf);
		pTmpBuf=NULL;
	}
	fclose(fp);
	if (pDId != NULL) {
		free (pDId);
		pDId = NULL;
	}
	if (pDId1 != NULL) {
		free (pDId1);
		pDId1 = NULL;
	}
	if (pDId0 != NULL) {
		free (pDId0);
		pDId0 = NULL;
	}
	if (pDId2 != NULL) {
		free (pDId2);
		pDId2 = NULL;
	}
	if (pDParentid != NULL) {
		free (pDParentid);
		pDParentid = NULL;
	}
	if (pDArtist != NULL) {
		free (pDArtist);
		pDArtist = NULL;
	}
	if (pDAlbum != NULL) {
		free (pDAlbum);
		pDAlbum = NULL;
	}
	if (pDGenre != NULL) {
		free (pDGenre);
		pDGenre = NULL;
	}
	if(item.objectitem!=NULL){
		free(item.objectitem);
		item.objectitem=NULL;
	}
	(*g_var)->g_numberreturned = (*g_var)->g_numberreturned + 1;
	return 0;
}

static int db_query_data_callback_xbox360 (void *pUser,	/* Pointer to the QueryResult structure */
						int nArg,	/* Number of columns in this result row */
						char **azArg,	/* Text of data in all columns */
						char **NotUsed	/* Names of the columns */
	)
{
	return(db_query_data_callback_flag(pUser, nArg, azArg, NotUsed, XBOX360));	
}

/******************************************************************************
 *MakeCountSQLSentence
 *
 *Description
 *       Accounting as
 *        put result to buffer
 *Parameters:
 *        ca_event -- The control action request event structure
 *        parent_id-- The parent id get information content form database
 *       ntype--     The ntype is SQL sentence type
 *
 ******************************************************************************/
int MakeCountSQLSentence (sqlite * db, int ntype, char *objectid, char *sql, char *extrastr,struct g_variable *g_var)
{
	int i = 0;
	int nlen = 0;
	int ch = '/';
	char *p1;
	char *p2;
	char *p;
	char *tmp_sql = NULL;
	char view_sql[512] = { 0 };
	char temp_objectid[512];
	char *zErrMsg = 0,*encoded_objectid=NULL;
	int rc = 0, ret=0;
	
	if(!objectid)
		return -1;
		
	encoded_objectid=malloc(2*strlen(objectid)+1);
	if(encoded_objectid){
		memset(encoded_objectid, 0, 2*strlen(objectid)+1);
		ParseSpecialSQLChar(objectid, encoded_objectid);
	}
	else
		encoded_objectid=objectid;
		
	strcpy (temp_objectid, encoded_objectid);
	p = temp_objectid;
	while ((p = strchr (p, ch)) != NULL) {
		p = p + 1;
		i = i + 1;//How many '/'?
	}
	p = temp_objectid;
	p = strchr (p, ch);
	p = p + 1;
	switch (ntype) {
		case SEL_ALBUM:
			sprintf(sql,"select count(*) from Data where  Album= '%s'",encoded_objectid);
			break;	
		case SEL_CREATOR:
			sprintf(sql,"select count(*) from Data where  Artist= '%s'",encoded_objectid);
			break;
		case SEL_TITLE:
			sprintf(sql,"select count(*) from Data where  ObjectName like '%s%%'",encoded_objectid);
			break;
		case SEL_MOST_PLAYED:
			sprintf(sql,"select count(*) from Data where PlayTimes > 0");
			break;
		case SEL_LAST_PLAYED:
			break;
		case SEL_OBJECT:
			strcpy (temp_objectid, encoded_objectid);
			if(strstr(temp_objectid,"upnpav")==NULL) {
				if(encoded_objectid && encoded_objectid!=objectid)
					free(encoded_objectid);
				return 0;
			}
			p = temp_objectid;
			p=strchr(p,ch);
			if(p==NULL)
			{
				if(encoded_objectid && encoded_objectid!=objectid)
					free(encoded_objectid);
				return 0;
			}
			sprintf(sql,"select count(*) from Data where  WebLocation= '%s'",p+1);
			break;
		case SEL_PATH:				/* when select content directory */
			if (i == 0)				/*the number of partition in /share */
				sprintf (sql, "select count(*) from Data_Root;");
			else
				sprintf (sql,  "select count(*) from Data where ObjectPath = '%s' ", p);
			break;
		case SEL_MOVIES_ALL:
			sprintf (sql, "select count(*) from Data where Title='video' ");
			break;
		case SEL_MOVIES_GENRE:
			if (i == 0) {			/*number of genre */
				sprintf (view_sql, "create temp view tmp_view as select distinct Genre from Data where Title='video' ");
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view ");
			}
			else{
				sprintf (view_sql, "create temp view tmp_view as select distinct ObjectPath,ObjectName from Data where Title='video' and Genre = '%s'", p);
				rc=sqlite_exec (db, view_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				sprintf (sql, "select count(*) from tmp_view;");
			}
			rc = 1;
			break;
		case SEL_MOVIES_DATE:
			if (i == 0) {
				sprintf (view_sql,
						 "create temp view tmp_view as select distinct BuildDateY from Data where Title='video' ");
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 1) {
				sprintf (view_sql,
						 "create temp view tmp_view as select distinct BuildDateY,BuildDateM from Data where Title='video' and BuildDateY = '%s' ",
						 p);
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 2) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				sprintf (view_sql,
						 "create temp view tmp_view as select distinct BuildDateY,BuildDateM,BuildDateD from Data where Title='video' and BuildDateY = '%s' and BuildDateM = '%s' ",
						 p, p1);
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 3) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				p2 = strchr (p1, ch);
				*p2 = 0;
				p2 = p2 + 1;
				sprintf (view_sql,
						 "create temp view tmp_view as select distinct ObjectPath,ObjectName from Data where Title='video' and BuildDateY = '%s' and BuildDateM = '%s' and BuildDateD = '%s'",
						 p, p1, p2);
				rc=sqlite_exec (db, view_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				sprintf (sql, "select count(*) from tmp_view;");
			}
			rc = 1;
			break;
		case SEL_MUSIC_ALL:
			sprintf (sql, "select count(*) from Data where Title='audio'");
			break;
		case SEL_MUSIC_GENRE:
			if (i == 0) {
				sprintf (view_sql,
						 "create temp view tmp_view as select distinct Genre from Data where Title='audio' ");
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else {
				sprintf (sql, "create temp view tmp_view as select distinct ObjectPath,ObjectName from Data where Title='audio' and Genre = '%s'",p);
				rc=sqlite_exec (db, sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				sprintf (sql, "select count(*) from tmp_view;");
			}
			rc = 1;
			break;
		case SEL_MUSIC_ARTIST:
			if (i == 0) {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct Artist from Data where Title='audio'");
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct ObjectPath,ObjectName from Data where Title='audio' and Artist = '%s'",
						p);
				rc=sqlite_exec (db, view_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				sprintf (sql, "select count(*) from tmp_view;");
			}
			rc = 1;
			break;
		case SEL_MUSIC_ALBUM:
			if (i == 0) {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct Album from Data where Title='audio' ");
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct ObjectPath,ObjectName from Data where Title='audio' and Album = '%s' ",
						p);
				rc=sqlite_exec (db, view_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				sprintf (sql, "select count(*) from tmp_view");
			}
			rc = 1;
			break;
		case SEL_MUSIC_DATE:
			if (i == 0) {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct BuildDateY from Data where Title='audio' ");
				//if((tmp_sql = (char*)malloc(strlen(view_sql)+strlen(extrastr)+10))==NULL) return;
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 1) {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct BuildDateY,BuildDateM from Data where Title='audio' and BuildDateY = '%s' ",
						p);
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 2) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				sprintf (view_sql,
						"create temp view tmp_view as select distinct BuildDateY,BuildDateM,BuildDateD from Data where Title='audio' and BuildDateY = '%s' and BuildDateM = '%s' ",
						p, p1);
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 3) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				p2 = strchr (p1, ch);
				*p2 = 0;
				p2 = p2 + 1;
				sprintf (view_sql,
						"create temp view tmp_view as select distinct ObjectPath,ObjectName from Data where Title='audio' and BuildDateY = '%s' and BuildDateM = '%s' and BuildDateD = '%s'",
						p, p1, p2);
				rc=sqlite_exec (db, view_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				sprintf (sql, "select count(*) from tmp_view;");
			}
			rc = 1;
			break;

		case SEL_MUSIC_PLAYLIST:

			break;
		case SEL_PHOTO_ALL:
			sprintf (sql, "select count(*) from Data where Title='photo'");
			break;
		case SEL_PHOTO_ALBUM:
			if (i == 0) {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct Album from Data where Title='photo' ");
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct ObjectPath,ObjectName from Data where Title='photo' and Album = '%s'",
						p);
				rc=sqlite_exec (db, view_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				sprintf (sql, "select count(*) from tmp_view;");
			}
			rc = 1;
			break;
		case SEL_PHOTO_DATE:
			if (i == 0) {
				sprintf (view_sql,
						 "create temp view tmp_view as select distinct BuildDateY from Data where Title='photo' ");
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 1) {
				sprintf (view_sql,
						"create temp view tmp_view as select distinct BuildDateY,BuildDateM from Data where Title='photo' and BuildDateY = '%s' ",
						p);
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 2) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				sprintf (view_sql,
						"create temp view tmp_view as select distinct BuildDateY,BuildDateM,BuildDateD from Data where Title='photo' and BuildDateY = '%s' and BuildDateM = '%s' ",
						p, p1);
				if (extrastr == NULL) {
					nlen = strlen (view_sql) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s", view_sql);
				}
				else {
					nlen = strlen (view_sql) + strlen (extrastr) + 10;
					if ((tmp_sql = (char *) malloc (nlen)) == NULL)
					{
						if(encoded_objectid && encoded_objectid!=objectid)
							free(encoded_objectid);
						return -1;
					}
					sprintf (tmp_sql, "%s%s", view_sql, extrastr);
				}
				rc=sqlite_exec (db, tmp_sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				free (tmp_sql);
				tmp_sql = NULL;
				sprintf (sql, "select count(*) from tmp_view;");
			}
			else if (i == 3) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				p2 = strchr (p1, ch);
				*p2 = 0;
				p2 = p2 + 1;
				sprintf (sql,
						"create temp view tmp_view as select distinct ObjectPath,ObjectName from Data where Title='photo' and BuildDateY = '%s' and BuildDateM = '%s' and BuildDateD = '%s'",
						p, p1, p2);
				rc=sqlite_exec (db, sql, 0, 0, &zErrMsg);
				if(rc!=SQLITE_OK){
					if(zErrMsg){
						sqlite_free(zErrMsg);
						zErrMsg=NULL;	
					}
				}				
				sprintf (sql, "select count(*) from tmp_view;");
			}
			rc = 1;
			break;
		case SEL_NUL:
			sprintf (sql, "select count(*) from Data ");
			break;
		default:
			break;
	}
	if(encoded_objectid && encoded_objectid!=objectid)
		free(encoded_objectid);
#ifdef 	__DEBUG__
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
//	printf("%s:%s():%d:ntype=%d, objectid=%s, sql={%s}\n", __FILE__, __FUNCTION__, __LINE__, ntype, objectid, sql);
	if(ntype==SEL_LAST_PLAYED) {
		g_var->g_count=length(g_playlist);
		g_var->g_totalmatches=g_var->g_count;
	}
	else
		sel_count_from_db (db, sql,g_var);
	// Drop the tmp_view
	if(ntype==SEL_MOST_PLAYED){
		if(g_var->g_count>most_played_num)
			g_var->g_count=most_played_num;
		if(g_var->g_totalmatches>most_played_num)
			g_var->g_totalmatches=most_played_num;
	}
	sprintf(sql, "drop view if exists tmp_view");
	ret=sqlite_exec(db, sql, 0, 0, &zErrMsg);
	if(ret!=SQLITE_OK){
		if(zErrMsg){
			sqlite_free(zErrMsg);
			zErrMsg=NULL;	
		}
	}	

	return rc;
}


static int db_query_songlen_callback (void *pUser,	/* Pointer to the QueryResult structure */
		int nArg,	/* Number of columns in this result row */
		char **azArg,	/* Text of data in all columns */
		char **NotUsed	/* Names of the columns */
		)
{
	char **pResult = (char **) pUser;
	if(nArg!=1)
		return 1;
	if((strcasecmp(NotUsed[0], "Duration") == 0)&&(azArg[0]!=NULL)) {	
		*pResult=(char *) malloc (strlen(azArg[0])+1);
		sprintf(*pResult,"%s",azArg[0]);
		return 0;
	}
	return 1;
}

int  SQLGetSongLength(const char *weblocation, int *duration)
{
	sqlite *db = NULL;
	char *zErrMsg = NULL;
	char *result_str= NULL, *p=NULL, *p1=NULL;
	char sql[512]={ 0 };
	int hour=0, min=0, sec=0;
	int ret=0;
	
	if (access (db_file_path, F_OK) == 0)
		sqlite_open (db_file_path, &db);
	if (db == NULL)
		return SQLITE_ERROR;
	sprintf(sql,"select distinct Duration from Data where WebLocation= '%s'",weblocation);
	ret=sqlite_exec (db, sql, db_query_songlen_callback, &result_str, &zErrMsg);
	if(ret!=SQLITE_OK){
		if(zErrMsg){
			sqlite_free(zErrMsg);
			zErrMsg=NULL;
		}
	}
	if (result_str) {
		p=strchr(result_str, ':');
		if(p){
			*p++=0;
			hour=atoi(result_str);
			p1=strchr(p,':');
			if(p1){
				*p1++=0;
				min=atoi(p);
				p=strchr(p1,'.');
				if(p)
					*p=0;
				sec=atoi(p1);	
			}
		}
		*duration=hour*3600+min*60+sec;
		free(result_str);
	}
	sqlite_close (db);
	return 0;
}

/******************************************************************************
 *MakeSQLSentence
 *
 *Description
 *       Accounting as
 *        put result to buffer
 *Parameters:
 *        ca_event -- The control action request event structure
 *        parent_id-- The parent id get information content form database
 *       ntype--     The ntype is SQL sentence type
 *
 ******************************************************************************/
int MakeSQLSentence (int ntype, char *objectid, char *sql, char *extrastr,struct g_variable *g_var)
{
	int i = 0;
	int ch = '/';
	char *p1=NULL;
	char *p2=NULL;
	char *p=NULL,*encoded_objectid=NULL;
	char temp_objectid[1024];

	#ifdef __DEBUG__
		printf("%s: %d\n", __FUNCTION__, __LINE__);
	#endif	
	if(!objectid)
		return -1;
	encoded_objectid=malloc(2*strlen(objectid)+1);
	if(encoded_objectid){
		memset(encoded_objectid, 0, 2*strlen(objectid)+1);
		ParseSpecialSQLChar(objectid, encoded_objectid);
	}
	else
		encoded_objectid=objectid;
	strcpy (temp_objectid, encoded_objectid);
	if(SEL_TITLE == ntype)
	{
		sprintf (sql,"select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where ObjectName like '%s%%' ", encoded_objectid);
		if(encoded_objectid && encoded_objectid!=objectid)
			free(encoded_objectid);
		return 0;
	}
	else if (SEL_ALBUM == ntype)
	{
		sprintf (sql,"select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Album= '%s' ", encoded_objectid);
		if(encoded_objectid && encoded_objectid!=objectid)
			free(encoded_objectid);
		return 0;
	}
	else if(SEL_CREATOR == ntype)
	{
		sprintf (sql,"select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Artist= '%s' ", encoded_objectid);
		if(encoded_objectid && encoded_objectid!=objectid)
			free(encoded_objectid);
		return 0;
	}
	else if(SEL_MOST_PLAYED == ntype)
	{
		sprintf (sql,"select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where PlayTimes>0 order by PlayTimes DESC ");
		if(encoded_objectid && encoded_objectid!=objectid)
			free(encoded_objectid);
		return 0;
	}
	else if(SEL_LAST_PLAYED == ntype)
	{
		char *tempfile=NULL;
		node *p=g_playlist->head;
		if(p){
			if(g_playlist->size>0)
			{
				tempfile=((char *)malloc(g_playlist->size*70));
				if(tempfile==NULL)
				{
					if(encoded_objectid && encoded_objectid!=objectid)
						free(encoded_objectid);
					return 1;
				}
			}
			else
			{
				if(encoded_objectid && encoded_objectid!=objectid)
					free(encoded_objectid);
				return 0;
			}
			strcpy(tempfile,"WebLocation = '");
			while(p)
			{
				strcat(tempfile,p->name);
				strcat(tempfile,"'");
				if(p->next)
					strcat(tempfile," OR WebLocation='");
				p=p->next;
			} 
			sprintf (sql,"select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where %s ",tempfile);
			if(tempfile)
				free(tempfile);
		}
		if(encoded_objectid && encoded_objectid!=objectid)
			free(encoded_objectid);
		return 0;
	}
	else if (SEL_OBJECT == ntype) {
		strcpy (temp_objectid, encoded_objectid);
	#ifdef __DEBUG__
			printf("temp_objectid: %s\n", temp_objectid);
			printf("ch: %c\n", ch);
	#endif			
		if((p=strstr(temp_objectid,VIRTUAL_DIR))==NULL)
		{
			if(encoded_objectid && encoded_objectid!=objectid)
				free(encoded_objectid);
			return 0;
		}
#if 0			
	   if(strstr(p1,VIRTUAL_DIR""VIRTUAL_PIC)==p1){
		    char picurl[512]={0};
		    
		    p=(char *)p1+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_PIC);
			sprintf(picurl, VIRTUAL_DIR"%s", p);
	    	p=picurl;
	    }
	    else if(strstr(p1,VIRTUAL_DIR""VIRTUAL_LM)==p1){
		    char picurl[512]={0};
	
		    p=(char *)p1+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_LM);
		    sprintf(picurl, VIRTUAL_DIR"%s", p);
		    p=picurl;
	    }
	    else if(strstr(p1,VIRTUAL_DIR""VIRTUAL_PNG)==p1){
		    char picurl[512]={0};
		    
		    p=(char *)p1+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_PNG);
			sprintf(picurl, VIRTUAL_DIR"%s", p);
	    	p=picurl;
	    }		
	    else
	    	p=p1;
#endif	    	
		#ifdef __DEBUG__
			printf("%s: %d\n", __FUNCTION__, __LINE__);
		#endif		
					
		#ifdef __DEBUG__
			printf("%s: %d\n", __FUNCTION__, __LINE__);
		#endif				
		sprintf (sql, "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where  WebLocation= '%s'",p);		
		p1=encoded_objectid;p=encoded_objectid; 
		p=strchr(p,ch);
		if(p)
			*p=0;
		strcpy(g_var->g_parentid,p1);
		if(encoded_objectid && encoded_objectid!=objectid)
			free(encoded_objectid);
		return 0;
	}
	else if (SEL_ROOT == ntype) {
		sprintf (sql, "select distinct Root from Data_Root where Root = '%s' ", objectid);//Column==1
		if(encoded_objectid && encoded_objectid!=objectid)
			free(encoded_objectid);
		return 0;
	}
	
	strcpy (temp_objectid, encoded_objectid);
	if(encoded_objectid && encoded_objectid!=objectid)
		free(encoded_objectid);
	p = temp_objectid;
	while ((p = strchr (p, ch)) != NULL) {
		p = p + 1;
		i = i + 1;
	}
	p = temp_objectid;
	p = strchr (p, ch);
	p = p + 1;
	//mBUG("%s:%s():%d:ntype=%d, objectid=%s, i=%d", __FILE__, __FUNCTION__, __LINE__, ntype, objectid, i);
	switch (ntype) {
		case SEL_PATH:
			if (i == 0)
				sprintf (sql, "select distinct Root from Data_Root");//Column==1
			else
				sprintf (sql, "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo  from Data where ObjectPath = '%s' ", p);
			return 0;
		case SEL_MOVIES_ALL:
			sprintf (sql, "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='video' ");
			return 0;
		case SEL_MOVIES_GENRE:
			if (i == 0)
				sprintf (sql, "select distinct Genre from Data where Title='video' %s ", extrastr);//Column==1
			else {
				sprintf (sql, "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='video' and Genre = '%s' ", p);
		return 0;
			}
			break;
		case SEL_MOVIES_DATE:
			if (i == 0)
				sprintf (sql,
						 "select distinct BuildDateY from Data where Title='video' %s ",
						 extrastr);//Column==1
			else if (i == 1)
				sprintf (sql,
						 "select distinct BuildDateY,BuildDateM from Data where Title='video' and BuildDateY = '%s' %s ",
						 p, extrastr);
			else if (i == 2) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				sprintf (sql,
						 "select distinct BuildDateY,BuildDateM,BuildDateD from Data where Title='video' and BuildDateY = '%s' and BuildDateM = '%s' %s ",
						 p, p1, extrastr);
			}
			else if (i == 3) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				p2 = strchr (p1, ch);
				*p2 = 0;
				p2 = p2 + 1;
				sprintf (sql,
						"select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='video' and BuildDateY = '%s' and BuildDateM = '%s' and BuildDateD = '%s' ",
						p, p1, p2);
				return 0;
			}
	
			break;
		case SEL_MUSIC_ALL:
			sprintf (sql,
					 "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='audio' ");
			return 0;
		case SEL_MUSIC_GENRE:
			if (i == 0)
				sprintf (sql,
						 "select distinct Genre from Data where Title='audio' %s ",
						 extrastr);//Column==1
			else {
				sprintf (sql,
						 "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='audio' and Genre ='%s' ",
						 p);
				return 0;
			}
			break;
		case SEL_MUSIC_ARTIST:
			if (i == 0)
				sprintf (sql,
						 "select distinct Artist from Data where Title='audio' %s ",
						 extrastr);
			else {
				sprintf (sql,
						 "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='audio' and Artist ='%s' ",
						 p);
				return 0;
			}
			break;
		case SEL_MUSIC_ALBUM:
			if (i == 0)
				sprintf (sql,
						 "select distinct Album from Data where Title='audio' %s ",
						 extrastr);
			else {
				sprintf (sql,
						 "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='audio' and Album ='%s' ",
						 p);
				return 0;
			}
			break;
		case SEL_MUSIC_DATE:
			if (i == 0)
				sprintf (sql,
						 "select distinct BuildDateY from Data where Title='audio' %s ",
						 extrastr);
			else if (i == 1)
				sprintf (sql,
						 "select distinct BuildDateY,BuildDateM from Data where Title='audio' and BuildDateY = '%s' %s ",
						 p, extrastr);
			else if (i == 2) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				sprintf (sql,
						 "select distinct BuildDateY,BuildDateM,BuildDateD from Data where Title='audio' and BuildDateY = '%s'  and BuildDateM = '%s' %s ",
						 p, p1, extrastr);
			}
			else if (i == 3) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				p2 = strchr (p1, ch);
				*p2 = 0;
				p2 = p2 + 1;
				sprintf (sql,
						 "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='audio' and BuildDateY = '%s' and BuildDateM = '%s' and BuildDateD = '%s' ",
						 p, p1, p2);
				return 0;
			}
			break;
		case SEL_MUSIC_PLAYLIST:
			return 0;
		case SEL_PHOTO_ALL:
			sprintf (sql,
					 "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='photo' ");
			return 0;
		case SEL_PHOTO_ALBUM:
			if (i == 0)
				sprintf (sql,
						 "select distinct Album from Data where Title='photo' %s ",
						 extrastr);
			else {
				sprintf (sql,
						 "select distinct Title,ObjectPath,ObjectName ,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='photo' and Album ='%s' ",
						 p);
				return 0;
			}
			break;
		case SEL_PHOTO_DATE:
			if (i == 0)
				sprintf (sql,
						 "select distinct BuildDateY from Data where Title='photo' %s ",
						 extrastr);
			else if (i == 1)
				sprintf (sql,
						 "select distinct BuildDateY,BuildDateM from Data where Title='photo' and BuildDateY = '%s' %s ",
						 p, extrastr);
			else if (i == 2) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				sprintf (sql,
						 "select distinct BuildDateY,BuildDateM,BuildDateD from Data where Title='photo' and BuildDateY = '%s' and BuildDateM = '%s' %s ",
						 p, p1, extrastr);//Column==3
			}
			else if (i == 3) {
				p1 = strchr (p, ch);
				*p1 = 0;
				p1 = p1 + 1;
				p2 = strchr (p1, ch);
				*p2 = 0;
				p2 = p2 + 1;
				sprintf (sql,
						 "select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data where Title='photo' and BuildDateY = '%s' and BuildDateM = '%s' and BuildDateD = '%s' ",
						 p, p1, p2);
				return 0;
			}
			break;
		case SEL_NUL:
				sprintf (sql,"select distinct Title,ObjectPath,ObjectName,Size,Artist,Genre,Album,BuildDateY,BuildDateM,BuildDateD,WebLocation,Duration,Resolution,ProtocolInfo from Data ");
			return 0;
		default:
			return 0;
	}
	return 1;

}

/******************************************************************************
 *MediaSQLAction
 *
 *Description
 *       deal with Browse action from database by Sql sentence
 *        put result to buffer
 *Parameters:
 *        ca_event -- The control action request event structure
 *        parent_id-- The parent id get information content form database
 *        ntype--     The ntype is SQL sentence type
 *	  nflag --	 Search  or Browse. Search is 0 Browse is 1;
 *
 ******************************************************************************/
int MediaSQLAction (struct Upnp_Action_Request *ca_event, char *object_id,
				int ntype, int nflag, int special_agent,struct g_variable *g_var)
{
	//int i = 0;
	unsigned long totalmatches = 0;
	//int nUpdataID = 0;
	int rc = 0;
	//int nCountDB = 0;
	unsigned long nstart = 0;
	unsigned long nrequest = 0;
	//int nallocmem = 0;
	char *psql;
	char sql_cmd[1024] = { 0 };
	char *sql_set=NULL;
	char strtmp[128] = { 0 };
	char *result_str = NULL;
	char *sqlresult = NULL;
	char *sqlcontain = NULL;
	char *extrastr = NULL;
	char ch_endformat[256] = { 0 };
	char ch_title[128] = { 0 };
	char ch_beginformat[350] = { 0 };
	unsigned long response_num=0;

#ifdef __DEBUG__	
	printf("%s:%s():%d:object_id=%s, ntype=%d, g_parentid=%s\n",__FILE__,__FUNCTION__,__LINE__, object_id, ntype,g_var->g_parentid);
#endif	
	if (1 == nflag){				//BrowseResponse
		sprintf (ch_title, "<u:BrowseResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">");
		sprintf (ch_beginformat, "<Result>&lt;DIDL-Lite \
		xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" \
		xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\
		xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"&gt;\n&lt;");
	}
	else if (0 == nflag){		//SearchResponse
		sprintf (ch_title, "<u:SearchResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">");
		sprintf (ch_beginformat, "<Result>&lt;DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0/dlna/\" xmlns:pv=\"http://www.pv.com/pvns/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"&gt;\n&lt;");
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	g_var->g_totalmatches = 0;
	g_var->g_numberreturned = 0;
	g_var->g_nmallocLen = 1024;
	if (access (db_file_path, F_OK)) {
		return -1;
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
	if ((extrastr = (char *) malloc (g_var->g_nmallocLen)) == NULL) {
		return -1;
	}
	memset (extrastr, 0, g_var->g_nmallocLen);
	sqlite_open (db_file_path, &g_var->g_Db);
	if (g_var->g_Db == NULL) {
		free(extrastr);
		extrastr=NULL;
		return SQLITE_ERROR;
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
	rc = MakeCountSQLSentence (g_var->g_Db, ntype, object_id, sql_cmd, extrastr,g_var);
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
	//sel_count_from_db(g_Db,countsql);
	if(g_var->g_count>most_played_num && ntype==SEL_MOST_PLAYED)
		response_num = most_played_num;
	else
		response_num = g_var->g_count;	//num from Database though Sql
	g_var->g_count = 0;
	if(ntype!=SEL_LAST_PLAYED)
	{
		sql_set=(char*)malloc(1024);
		if(sql_set==NULL)
		{
			if(extrastr)
			{
				free(extrastr);
				extrastr=NULL;
			}
			sqlite_close (g_var->g_Db);
			return -1;
		}
		memset(sql_set,0,sizeof(sql_set));
	}	
	else
	{
		sql_set=((char *)malloc(g_playlist->size*70+256));
		if(sql_set==NULL)
		{
			if(extrastr)
			{
				free(extrastr);
				extrastr=NULL;
			}
			sqlite_close (g_var->g_Db);
			return -1;
		}
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
	MakeSQLSentence (ntype, object_id, sql_set, extrastr,g_var);
#ifdef __DEBUG__	
	printf("%s:%d: object_id=%s, extrastr={%s}, sql={%s}\n", __FUNCTION__, __LINE__, object_id, extrastr, sql_set);
#endif	

	if (extrastr) {
		free (extrastr);
		extrastr = NULL;
	}
	if(ntype==SEL_OBJECT) {
		totalmatches=1;
	}
	else if((ntype==SEL_MOST_PLAYED) && (g_var->g_totalmatches > most_played_num))
		totalmatches=most_played_num;	
	else
		totalmatches = g_var->g_totalmatches;	//Total mached number in DB.
#ifdef __DEBUG__			
	printf("%s: %d\n", __FUNCTION__, __LINE__);
	printf("g_var->g_numberreturned: %lu\n", g_var->g_numberreturned);		
#endif			
	g_var->g_totalmatches = totalmatches;
	nstart = g_var->g_startingindex;
	if (g_var->g_requestedcount == 0){
		nrequest = totalmatches;	//Total Matches
	}
	else
		nrequest = g_var->g_requestedcount;	//Requested number from client.
#ifdef __DEBUG__		
	printf("%s: %d\n", __FUNCTION__, __LINE__);
	printf("nstart: %lu\n", nstart);
	printf("response_num: %lu\n", response_num);		
#endif	
	if (nstart >= 0 && nstart < response_num){
		if(nrequest <= response_num-nstart) {
			sprintf (strtmp, " limit %lu,%lu", nstart, nrequest);//Ignore first 'nstart - nbeforeCountData', request 'nrequest' entries.
			psql = (char *) malloc (strlen (strtmp) + strlen (sql_set) + 1);
			if (psql == NULL) {			
				if(sql_set){
					free(sql_set);
					sql_set=NULL;
				}
				sqlite_close (g_var->g_Db);
				return -1;
			}
			sprintf (psql, "%s%s", sql_set, strtmp);
			sel_from_db (g_var->g_Db, psql, &sqlresult, special_agent,g_var);
			free (psql);
			psql = NULL;
#ifdef __DEBUG__			
			printf("%s: %d\n", __FUNCTION__, __LINE__);
			printf("g_var->totalmatches: %lu\n", g_var->g_totalmatches);
			printf("g_var->g_numberreturned: %lu\n", g_var->g_numberreturned);		
#endif				
		}
		else{ //nrequest > response_num-nstart
			sprintf (strtmp, " limit %lu,%lu", nstart, (response_num - nstart));////Ignore first 'nstart - nbeforeCountData', request 'response_num - (nstart - nbeforeCountData)' entries.
			psql = (char *) malloc (strlen (strtmp) + strlen (sql_set) + 1);
			if (psql == NULL) {			
				if(sql_set){
					free(sql_set);
					sql_set=NULL;
				}
				sqlite_close (g_var->g_Db);
				return -1;
			}
			memset (psql, 0, (strlen (strtmp) + strlen (sql_set) + 1));
			sprintf (psql, "%s%s", sql_set, strtmp);
			sel_from_db (g_var->g_Db, psql, &sqlresult, special_agent,g_var);
			free (psql);
			psql = NULL;
#ifdef __DEBUG__			
			printf("%s: %d\n", __FUNCTION__, __LINE__);
			printf("g_var->totalmatches: %lu\n", g_var->g_totalmatches);
			printf("g_var->g_numberreturned: %lu\n", g_var->g_numberreturned);
#endif							
		}
	}
	sqlite_close (g_var->g_Db);
	if (sqlresult == NULL) {
		if ((sqlresult = (char *) malloc (2)) == NULL) {		
			if(sql_set){
				free(sql_set);
				sql_set=NULL;
			}
			return -1;
		}
		memset (sqlresult, 0, 2);
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	if((nflag==0) && (g_var->g_filterflag & 0x0020) && (g_var->g_filterflag!=0xFFFF)) {
		if ((sqlcontain = (char *) malloc (256)) == NULL){
			if(sqlresult)
			{
				free(sqlresult);
				sqlresult=NULL;
			}
			if(sql_set){
				free(sql_set);
				sql_set=NULL;
			}
			return -1;
		}
		memset(sqlcontain,0,256);
		strcpy (sqlcontain,"container id=\"0\" parentID=\"-1\" restricted=\"1\" &gt;\n &lt;dc:title&gt;\"root\"&lt;/dc:title&gt;\n &lt;upnp:class&gt;object.container&lt;/upnp:class&gt;&lt;/container&gt;&lt;");
	}
	else {
		sqlcontain= (char *) malloc (2);
		memset (sqlcontain, 0, 2);
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	if (1 == nflag) {//BrowseResponse
		sprintf (ch_endformat, "/DIDL-Lite&gt;</Result><NumberReturned>%lu</NumberReturned><TotalMatches>%lu</TotalMatches><UpdateID>%u</UpdateID></u:BrowseResponse>",
				g_var->g_numberreturned, totalmatches,GetUpdateIDByContainer(object_id));
	}
	else if (0 == nflag)
		sprintf (ch_endformat, "/DIDL-Lite&gt;</Result><NumberReturned>%lu</NumberReturned><TotalMatches>%lu</TotalMatches><UpdateID>%u</UpdateID></u:SearchResponse>",
			g_var->g_numberreturned, totalmatches,GetUpdateIDByContainer(object_id));
	if ((result_str = (char *) malloc (strlen (ch_title) + strlen (ch_beginformat) + strlen (ch_endformat) + strlen (sqlresult)+strlen(sqlcontain) + 1)) ==	NULL) {
		if (sqlresult) {
			free (sqlresult);
			sqlresult = NULL;
		}
		if(sql_set){
			free(sql_set);
			sql_set=NULL;
		}
		if(sqlcontain){
			free(sqlcontain);
			sqlcontain=NULL;
		}
		return -1;
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	sprintf (result_str, "%s%s%s%s%s", ch_title, ch_beginformat, sqlresult,sqlcontain, ch_endformat);
	if (sqlresult) {
		free (sqlresult);
		sqlresult = NULL;
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	ca_event->ActionResult = ixmlParseBuffer (result_str);
	if (result_str) {
		free (result_str);
		result_str = NULL;
	}
	if(sqlcontain){
		free(sqlcontain);
		sqlcontain=NULL;
	}
	if(sql_set){
		free(sql_set);
		sql_set=NULL;
	}
	return 0;
}

int MediaMetaDataParseObjectID (char *object_id, char *pch_parentid,
							char *pch_objectname, char *pch_objectpath,
							struct MediaService *pMediaService,
							int ncatalogcount)
{

	int i = 0;
	char strtemp[1024] = { 0 };
	char strtemp2[1024] = { 0 };
	char *pchstart = NULL;
	char *pchend = NULL;

	strcpy (strtemp2, object_id);
	if ((pchstart = strchr (strtemp2, '/')) != NULL) {
		*pchstart = '\0';
		pchstart = NULL;
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
	for (i = 0; i <= ncatalogcount; i++) {
		if (i == ncatalogcount) {
			return -1;
		}
#ifdef __DEBUG__			
		printf("%s: %d pMediaService[i].ch_containerid:%s\n",__FUNCTION__,__LINE__,pMediaService[i].ch_containerid);    
#endif
		if (strstr (strtemp2, pMediaService[i].ch_containerid) == strtemp2)
			break;
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif		
	strcpy (strtemp, object_id);
	if ((pchstart = strchr (strtemp, '/')) == NULL) {
		strcpy (pch_objectname, pMediaService[i].ch_title);
		strcpy (pch_parentid, pMediaService[i].ch_parentid);
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif			
		//printf("FILE:%s LINE:%d pch_objectname:%s pch_parentid:%s",__FILE__,__LINE__,pch_objectname,pch_parentid);
		return pMediaService[i].n_type;
	}
	else {
		if (SEL_MOVIES_ALL == pMediaService[i].n_type
			|| SEL_MUSIC_ALL == pMediaService[i].n_type
			|| SEL_PHOTO_ALL == pMediaService[i].n_type) {
			pchend = strrchr (strtemp, '/');
			*pchend = '\0';
			strcpy (pch_objectpath, (pchstart + 1));
			strcpy (pch_parentid, pMediaService[i].ch_containerid);
			strcpy (pch_objectname, (strrchr (object_id, '/') + 1));
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif				
			//printf("%s: %d pch_objectname:%s pch_parentid:%s",__FUNCTION__,__LINE__,pch_objectname,pch_parentid);
			return SEL_OBJECT;
		}
		else if (SEL_MOVIES_DATE == pMediaService[i].n_type
				|| SEL_MUSIC_DATE == pMediaService[i].n_type
				|| SEL_PHOTO_DATE == pMediaService[i].n_type) {
				pchend = strrchr (strtemp, '/');
				*pchend = '\0';
				strcpy (pch_parentid, strtemp);
				strcpy (pch_objectname, (strrchr (object_id, '/') + 1));
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif					
		}
		else  if (strstr(object_id, "B13") == object_id /*Movie Folder*/
					|| strstr(object_id, "B27") == object_id /*Music Folder*/
					|| strstr(object_id, "B36") == object_id/*Photo Folder*/){	
				int ret=0;
				char folder_name[1024]={0};
				sqlite *pDB=NULL;

#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
				sqlite_open (db_file_path, &pDB);
				ret=-1;
				if (pDB){
					pchend=strrchr(object_id, '/');
					pchend++;
					ret=GetFolderOfContainerID(pDB, pchend, folder_name);
					sqlite_close (pDB);	
					if(!ret){
						pchend = strrchr (strtemp, '/');
						if(pchend)
							*pchend = '\0';
						strcpy (pch_parentid, strtemp);		
						
						pchend = strrchr (folder_name, '/');
						if(pchend)
							pchend++;	
						else
							pchend=folder_name;									
						strcpy (pch_objectname, pchend);						
					}	
				}
				if(ret){
					strcpy (pch_parentid, pMediaService[i].ch_containerid);
					strcpy (pch_objectname, (object_id + strlen (pMediaService[i].ch_containerid) + 1));
				}
				
		}			
		else{	
			strcpy (pch_parentid, pMediaService[i].ch_containerid);
			strcpy (pch_objectname, (object_id + strlen (pMediaService[i].ch_containerid) + 1));
		}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif			
		if(strstr(object_id,VIRTUAL_DIR))
			return SEL_OBJECT;		
		return pMediaService[i].n_type;
	}
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	return 0;
}

int MediaMetaDataBrowseAction (struct Upnp_Action_Request *ca_event, char *object_id, int nflag, int special_agent,struct g_variable *g_var)
{
	int ntype = 0;
	int ncatalogcount = 0;
	char ch_endformat[256] = { 0 };
	char strparentid[1024] = { 0 };
	char strobjectname[256] = { 0 };
	char strobjectpath[256] = { 0 };
	char ch_title[128] = { 0 };
	char ch_beginformat[350] = { 0 };
	char *strsql = NULL;
	char strtemp[1024] = { 0 };
	char *extrastr = NULL;
	char *pobjectid = NULL;
	char *pparentid = NULL;
	char *pobjectname = NULL;
	char *sqlresult = NULL;
	char *result_str = NULL;

	sqlite *db = NULL;
	char *pch = NULL;
	struct MediaService *pMediaService = NULL;

#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	if (1 == nflag){			//BrowseResponse
		sprintf (ch_title, "<u:BrowseResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">");
		sprintf (ch_beginformat, "<Result>&lt;DIDL-Lite "
		"xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" "
		"xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
		"xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"&gt;\n&lt;");
	}
	else if (0 == nflag){	//SearchResponse
		sprintf (ch_title, "<u:SearchResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">");
		sprintf (ch_beginformat, "<Result>&lt;DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0/dlna/\" xmlns:pv=\"http://www.pv.com/pvns/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"&gt;\n&lt;");
	}
	ncatalogcount = NUM_Media_catalog;
	pMediaService = (struct MediaService *)media_catalog_table;
#ifdef __DEBUG__	
	printf("%s: %d\n", __FUNCTION__, __LINE__);
#endif	
	// parse objectid
	ntype = MediaMetaDataParseObjectID (object_id, strparentid, strobjectname,
										strobjectpath, pMediaService,
										ncatalogcount);
	if (ntype == -1)
		return -1;
	if (SEL_OBJECT != ntype) {
#if 0		
		if (SEL_NUL == ntype) {
			for (i = 0; i < ncatalogcount; i++) {
				if (strcmp (object_id, pMediaService[i].ch_parentid) == 0)
					nChildCount +=1;
			}
		}
		else if(SEL_MUSIC_PLAYLIST == ntype){
			for (i = 0; i < ncatalogcount; i++) {
				if (strcmp (object_id, pMediaService[i].ch_parentid) == 0)
					nChildCount +=1;
			}
		}
		else {
			sqlite_open (db_file_path, &db);
			if (db == NULL) {
				return SQLITE_ERROR;
			}

			MakeCountSQLSentence (db, ntype, object_id, countsql, extrastr,g_var);
			//sel_count_from_db(db,countsql);
			//mBUG("file:%s line:%d countsql:%s g_totalmatches:%d",__FILE__,__LINE__,countsql,g_totalmatches);
			//mBUG("FILE:%s LINE:%d  countsql:%s ntype:%d object_id:%s extrastr:%s g_totalmatches:%d",__FILE__,__LINE__,countsql,ntype,object_id,extrastr,g_totalmatches);
			if (i == 0) {
				if ((data_sql = (char *) malloc (1000)) == NULL) {
					sqlite_close (db);
					return -1;
				}
			}
			else {
				if (extrastr) {
					if ((data_sql =
						 (char *) malloc (1000 + strlen (extrastr) + 3)) == NULL) {
						sqlite_close (db);
						return -1;
					}
				}
				else {
					if ((data_sql = (char *) malloc (1000 + 1)) == NULL) {
						sqlite_close (db);
						return -1;
					}
				}
			}
			if (MakeSQLSentence(pMediaService[i].n_type, object_id, data_sql, extrastr) == 1) {
				sel_from_db_by_browse (db, data_sql, &extrastr);
			}
			free (data_sql);
			data_sql = NULL;
			sqlite_close (db);
			nChildCount = g_totalmatches;
			g_var->g_totalmatches = 0;
		}
#endif		
		mBUG("FILE:%s LINE:%d object_id:%s pch_parentid:%s",__FILE__,__LINE__,object_id,strparentid);
		if ((sqlresult = (char *) malloc (1000)) == NULL)
			return -1;
		pobjectid = xml_encode (object_id);
		pparentid = xml_encode (strparentid);
		pobjectname = xml_encode (strobjectname);
		if(special_agent != XBOX360){
			if(g_var->g_filterflag == 0xFFFF)
				sprintf (sqlresult,
						 "container id=\"%s\" parentID=\"%s\" restricted=\"1\"  &gt;\n"
						 "&lt;dc:title&gt;%s&lt;/dc:title&gt;\n"
						 "&lt;upnp:searchClass includeDerived=\"0\"&gt;object.item.audioItem&lt;/upnp:searchClass&gt;\n"
						 "&lt;upnp:searchClass includeDerived=\"0\"&gt;object.item.imageItem&lt;/upnp:searchClass&gt;\n"
						 "&lt;upnp:searchClass includeDerived=\"0\"&gt;object.item.videoItem&lt;/upnp:searchClass&gt;\n"
						 "&lt;upnp:class&gt;object.container&lt;/upnp:class&gt;\n",
						 pobjectid, pparentid,  pobjectname);
			else
				sprintf (sqlresult,
						"container id=\"%s\" parentID=\"%s\" restricted=\"1\"  &gt;\n"
						"&lt;dc:title&gt;%s&lt;/dc:title&gt;\n"
						"&lt;upnp:class&gt;object.container&lt;/upnp:class&gt;\n",
						pobjectid, pparentid,  pobjectname);
		}
		else{
			sprintf(sqlresult,"container id=\"%s\" parentID=\"%s\" restricted=\"1\" &gt;\n"
				   "&lt;dc:title&gt;%s&lt;/dc:title&gt;&lt;upnp:artist&gt;%s&lt;/upnp:artist&gt;\n"
				   "&lt;upnp:class&gt;%s&lt;/upnp:class&gt;",pobjectid,pparentid,pobjectname,pobjectname,g_var->g_upnpclass);
		}
		if (pobjectid) {
			free (pobjectid);
			pobjectid = NULL;
		}
		if (pparentid) {
			free (pparentid);
			pparentid = NULL;
		}
		if (pobjectname) {
			free (pobjectname);
			pobjectname = NULL;
		}
#if 0			
		if((g_var->g_filterflag&0x0080)&&(g_var->g_nflag !=2 )){
			if(g_var->g_filterflag&0x0080)
				sprintf(sqlresult ,"%s\n&lt;upnp:writeStatus&gt;UNKNOWN&lt;/upnp:writeStatus&gt;\n",sqlresult);
		}
#endif			 
		sprintf (sqlresult, "%s&lt;/container&gt;&lt;", sqlresult);
	}
	else {
		strcpy (strtemp, object_id);
		pch = strchr (strtemp, '/');
		if(pch)
			*pch = '\0';
		strcpy (g_var->g_root, strtemp);
		*pch = '/';
		strcpy (g_var->g_parentid, strparentid);
		#ifdef __DEBUG__
			printf("%s: %d\n", __FUNCTION__, __LINE__);
			printf("object_id: %s\n", object_id);
		#endif
		if(ntype!=SEL_LAST_PLAYED) {
			strsql=(char*)malloc(1024);
			if(strsql==NULL)
				return -1;
			memset(strsql,0,sizeof(strsql));
		}	
		else {
			strsql=((char *)malloc(g_playlist->size*70+256));
			if(strsql==NULL) {
				return -1;
			}
		}
		MakeSQLSentence (ntype, object_id, strsql, NULL,g_var);
		//printf("%s:%d ntype:%d strobjectname:%s strsql:%s strobjectpath:%s",__FUNCTION__,__LINE__,ntype,strobjectname,strsql,strobjectpath);
		sqlite_open (db_file_path, &db);
		if (db == NULL){
			free(strsql);
			strsql=NULL;
			return SQLITE_ERROR;
		}
		sel_from_db (db, strsql, &sqlresult, special_agent,g_var);//Column>3
		if (sqlresult == NULL) {
			sqlresult = (char *) malloc (2);
			memset (sqlresult, 0, 2);
		}
		sqlite_close (db);
		#ifdef __DEBUG__
			printf("sqlresult: %s\n", sqlresult);
		#endif		
	}
	if (1 == nflag)			//Browse Response
		sprintf (ch_endformat, "/DIDL-Lite&gt;</Result><NumberReturned>1</NumberReturned><TotalMatches>1</TotalMatches><UpdateID>%u</UpdateID></u:BrowseResponse>", GetUpdateIDByContainer(object_id));
	else if (0 == nflag)	//Search Response
		sprintf (ch_endformat, "/DIDL-Lite&gt;</Result><NumberReturned>1</NumberReturned><TotalMatches>1</TotalMatches><UpdateID>%u</UpdateID></u:SearchResponse>", GetUpdateIDByContainer(object_id));
	if (extrastr) {
		free (extrastr);
		extrastr = NULL;
	}
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	if ((result_str = (char *) malloc (strlen (ch_title) + strlen (ch_beginformat) + strlen (ch_endformat) + strlen (sqlresult) + 1)) == NULL) {
		if (sqlresult) {
			free (sqlresult);
			sqlresult = NULL;
		}
		if(strsql){
			free(strsql);
			strsql=NULL;
		}
		return -1;
	}

	sprintf (result_str, "%s%s%s%s", ch_title, ch_beginformat, sqlresult, ch_endformat);
	if (sqlresult) {
		free (sqlresult);
		sqlresult = NULL;
	}
	ca_event->ActionResult = ixmlParseBuffer (result_str);
	if (result_str) {
		free (result_str);
		result_str = NULL;
	}
	if(strsql){
		free(strsql);
		strsql=NULL;
	}
	return 0;
}

int MediaSearchAction (struct Upnp_Action_Request *ca_event,struct g_variable *g_var)
{
	int i = 0;
	char strtmp[1024] = { 0 };
	//char strtmp2[256]={0};
	char ContainerID_str[1024] = { 0 };
	char *pch_ContainerID = NULL;
	char *pch_SearchCriteria = NULL;
	char *pch_Filter = NULL;
	char *pch_StartingIndex = NULL;
	char *pch_RequestedCount = NULL;
	//char *pch_SortCriteria = NULL;
	char *pch_upnpartist = NULL;
	char *pch_tmphead = NULL;
	char *pch_tmpend = NULL;
	int special_agent=STANDARD;
//	printf("ca_event->Xbox360=%d\n",ca_event->Xbox360);
	if(ca_event->Xbox360)
		special_agent=XBOX360;	

	memset (g_var->g_upnpclass, 0, 50);
	pch_ContainerID = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "ContainerID", 0);
	pch_SearchCriteria = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "SearchCriteria", 0);
	pch_Filter = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "Filter", 0);
	pch_StartingIndex = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "StartingIndex", 0);
	pch_RequestedCount = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "RequestedCount", 0);
	/*
	pch_SortCriteria = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "SortCriteria", 0);
	*/
	if((pch_ContainerID==NULL)||(pch_SearchCriteria==NULL)||(pch_Filter==NULL)||(pch_StartingIndex==NULL)||(pch_RequestedCount==NULL)){//||(pch_SortCriteria==NULL))
		if (pch_StartingIndex) {
			free (pch_StartingIndex);
			pch_StartingIndex = NULL;
		}

		if (pch_RequestedCount) {
			free (pch_RequestedCount);
			pch_RequestedCount = NULL;
		}

		if (pch_Filter) {
			free (pch_Filter);
			pch_Filter = NULL;
		}
		if (pch_ContainerID) {
			free (pch_ContainerID);
			pch_ContainerID = NULL;
		}
		if (pch_SearchCriteria) {
			free (pch_SearchCriteria);
			pch_SearchCriteria = NULL;
		}
		return -10;
	}
	//printf("%d===pch_ContainerID=%s,pch_Filter=%s,pch_SearchCriteria=%s\n",__LINE__, pch_ContainerID,pch_Filter,pch_SearchCriteria);
	if (pch_StartingIndex) {
		g_var->g_startingindex = strtoul (pch_StartingIndex, NULL, 10);
		free (pch_StartingIndex);
		pch_StartingIndex = NULL;
	}
	if (pch_RequestedCount) {
		g_var->g_requestedcount = strtoul (pch_RequestedCount, NULL, 10);
		free (pch_RequestedCount);
		pch_RequestedCount = NULL;
	}
	if(pch_Filter){
		if (strcmp (pch_Filter, "*") == 0) {
			g_var->g_filterflag = 0xFFFF;
		}
		else {
			for (i = 0; i < SORTCRITERIANUM; i++) {
				if ((strstr (pch_Filter, filter_table[i].strsortname)) != NULL)
					g_var->g_filterflag = filter_table[i].nflag | g_var->g_filterflag;
			}
		}
		free (pch_Filter);
		pch_Filter = NULL;
	}
	else
		g_var->g_filterflag = 0xFFFF;

	mBUG("%s:%s():%d:ContainerID=%s\n", __FILE__, __FUNCTION__, __LINE__, pch_ContainerID);
	if (strcmp (pch_ContainerID, "7") == 0) { // Xbox: Music Album
		strcpy (ContainerID_str, "B24");
		strcpy (g_var->g_root, ContainerID_str);
		strcpy (g_var->g_parentid, ContainerID_str);
		g_var->select_type = SEL_MUSIC_ALBUM;
		sprintf (g_var->g_upnpclass, "object.container.album.musicAlbum");
		MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
	}
	else if (strcmp (pch_ContainerID, "6") == 0) {	// Xbox: Music Artist
		strcpy (ContainerID_str, "B23");
		strcpy (g_var->g_root, ContainerID_str);
		strcpy (g_var->g_parentid, ContainerID_str);
		g_var->select_type = SEL_MUSIC_ARTIST;
		sprintf (g_var->g_upnpclass, "object.container.person.musicArtist");
		MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
	}
	else if (strcmp (pch_ContainerID, "5") == 0) {	// Xbox: Music Genre
		strcpy (ContainerID_str, "B22");
		strcpy (g_var->g_root, ContainerID_str);
		strcpy (g_var->g_parentid, ContainerID_str);
		g_var->select_type = SEL_MUSIC_GENRE;
		sprintf (g_var->g_upnpclass, "object.container.genre.musicGenre");
		MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
	}
	else if (strcmp (pch_ContainerID, "4") == 0) {	// Xbox: All Music
		strcpy (ContainerID_str, "B21");
		strcpy (g_var->g_root, ContainerID_str);
		strcpy (g_var->g_parentid, ContainerID_str);
		g_var->select_type = SEL_MUSIC_ALL;
		sprintf (g_var->g_upnpclass, "object.item.audioItem");
		MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
	}
	else if (strcmp (pch_ContainerID, "F") == 0) {	// Xbox: Music PLAYLIST
		sprintf (g_var->g_upnpclass, "object.container.playlistContainer");
	}
	else if (strcmp (pch_ContainerID, "1") == 0) { // Xbox: Music
	    if (strstr (pch_SearchCriteria, "object.container.album.musicAlbum") != NULL)
			sprintf (g_var->g_upnpclass, "object.container.album.musicAlbum");
	    if ((pch_upnpartist = strstr (pch_SearchCriteria, "upnp:artist")) != NULL) {
			mBUG ("FILE:%s LINE:%d pch_upnpartist:%s", __FILE__, __LINE__,
				pch_upnpartist);
			if ((pch_tmphead = strstr (pch_upnpartist, "\"")) != NULL)
			    pch_tmphead = pch_tmphead + strlen ("\"");
			else
			    goto ERROR;
			mBUG ("FILE:%s LINE:%d pch_tmphead:%s", __FILE__, __LINE__,
				pch_tmphead);
			if ((pch_tmpend = strstr (pch_tmphead, "\"")) != NULL)
			    *pch_tmpend = '\0';
			else
			    goto ERROR;
			mBUG ("FILE:%s LINE:%d pch_ContainerID:%s", __FILE__,
				__LINE__, pch_ContainerID);
			amp_decode (pch_tmphead, strtmp);
			sprintf (ContainerID_str, "B23/%s", strtmp);
			mBUG ("file:%s line:%d Search 1 ContainerID_str:%s ",
				__FILE__, __LINE__, ContainerID_str);
			MediaMetaDataBrowseAction (ca_event, ContainerID_str, 0, special_agent,g_var);
			mBUG ("file:%s line:%d Search 1 end", __FILE__, __LINE__);
	    }
	}
	else if (strcmp (pch_ContainerID, "0") == 0) { // Windows Media Player 11 for Vista
	    if (strstr(pch_SearchCriteria, "object.item.audioItem") != NULL) {
			strcpy (ContainerID_str, "B21");
			strcpy (g_var->g_root, ContainerID_str);
			strcpy (g_var->g_parentid, ContainerID_str);
			g_var->select_type = SEL_MUSIC_ALL;
			sprintf (g_var->g_upnpclass, "object.item.audioItem");
			MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
	    }
	    else if (strstr(pch_SearchCriteria, "object.item.videoItem") != NULL) {
			strcpy (ContainerID_str, "B11");
			strcpy (g_var->g_root, ContainerID_str);
			strcpy (g_var->g_parentid, ContainerID_str);
			g_var->select_type = SEL_MOVIES_ALL;
			sprintf (g_var->g_upnpclass, "object.item.videoItem");
			MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
	    }
	    else if (strstr(pch_SearchCriteria, "object.item.imageItem") != NULL) {
			strcpy (ContainerID_str, "B31");
			strcpy (g_var->g_root, ContainerID_str);
			strcpy (g_var->g_parentid, ContainerID_str);
			g_var->select_type = SEL_PHOTO_ALL;
			sprintf (g_var->g_upnpclass, "object.item.imageItem");
			MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
	    }
	    else {
		    g_var->select_type = SEL_NUL;
		    if (((pch_upnpartist =strstr (pch_SearchCriteria, "@id =")) != NULL)||((pch_upnpartist =strstr (pch_SearchCriteria, "@refID =")) != NULL)) {
			    if ((pch_tmphead = strstr (pch_upnpartist, "\"")) != NULL)
				    pch_tmphead = pch_tmphead + strlen ("\"");
			    else
				    goto ERROR;
			    if ((pch_tmpend = strstr (pch_tmphead, "\"")) != NULL)
				    *pch_tmpend = '\0';
			    else
				    goto ERROR;
			    amp_decode (pch_tmphead, strtmp);
			    sprintf (ContainerID_str, "%s", strtmp);
			    g_var->select_type=SEL_OBJECT;
		    	    MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
		    else if(((pch_upnpartist =strstr (pch_SearchCriteria, "@id exists false")) != NULL)||((pch_upnpartist =strstr (pch_SearchCriteria, "upnp:class exists false")) != NULL)||(pch_upnpartist =strstr (pch_SearchCriteria, "title exists false"))) {

			    g_var->select_type=-1;
			    MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
		    
		    else if((pch_upnpartist =strstr (pch_SearchCriteria, "upnp:class exists true")) != NULL)   {
			    amp_decode (pch_ContainerID, strtmp);
			    MakeObjectID (strtmp, ContainerID_str);
			    sprintf (g_var->g_upnpclass, "object.container");
			    if(g_var->g_filterflag==0xFFFF)
				    g_var->g_filterflag=0;
			    g_var->g_filterflag= g_var->g_filterflag|0x0020;
			    MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
		    else if((pch_upnpartist =strstr (pch_SearchCriteria, "object.container")) != NULL) {
			    amp_decode (pch_ContainerID, strtmp);
			    MakeObjectID (strtmp, ContainerID_str);
			    sprintf (g_var->g_upnpclass, "object.container");
				MediaMetaDataBrowseAction (ca_event, ContainerID_str, 0, special_agent,g_var);
		    }
		    else if((pch_upnpartist =strstr (pch_SearchCriteria, "album =")) != NULL) {
			    if ((pch_tmphead = strstr (pch_upnpartist, "\"")) != NULL)
				    pch_tmphead = pch_tmphead + strlen ("\"");
			    else
				    goto ERROR;
			    if ((pch_tmpend = strstr (pch_tmphead, "\"")) != NULL)
				    *pch_tmpend = '\0';
			    else
				    goto ERROR; 

			    if(g_var->g_filterflag==0xFFFF)
				    g_var->g_filterflag=0;
			    g_var->g_filterflag= g_var->g_filterflag|0x0008;
			    strcpy(ContainerID_str,pch_tmphead);
			    g_var->select_type=SEL_ALBUM;
			    MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
		    else if((pch_upnpartist =strstr (pch_SearchCriteria, "creator =")) != NULL) {
			    if ((pch_tmphead = strstr (pch_upnpartist, "\"")) != NULL)
				    pch_tmphead = pch_tmphead + strlen ("\"");
			    else
				    goto ERROR;
			    if ((pch_tmpend = strstr (pch_tmphead, "\"")) != NULL)
				    *pch_tmpend = '\0';
			    else
				    goto ERROR; 

			    if(g_var->g_filterflag==0xFFFF)
				    g_var->g_filterflag=0;
			    g_var->g_filterflag= g_var->g_filterflag|0x4000;
			    strcpy(ContainerID_str,pch_tmphead);
			    g_var->select_type=SEL_CREATOR;
			    MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
		    else if(((pch_upnpartist =strstr (pch_SearchCriteria, "title contains")) != NULL)||((pch_upnpartist =strstr (pch_SearchCriteria, "title = ")) != NULL))  {
			    if ((pch_tmphead = strstr (pch_upnpartist, "\"")) != NULL)
				    pch_tmphead = pch_tmphead + strlen ("\"");
			    else
				    goto ERROR;
			    if ((pch_tmpend = strstr (pch_tmphead, "\"")) != NULL)
				    *pch_tmpend = '\0';
			    else
				    goto ERROR; 
			   strcpy(ContainerID_str,pch_tmphead);
			   g_var->select_type=SEL_TITLE;
			   MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
		    else if((pch_upnpartist =strstr (pch_SearchCriteria,"@refID exists true")) != NULL){
			    if(g_var->g_filterflag==0xFFFF)
				    g_var->g_filterflag=0;
			    g_var->g_filterflag= g_var->g_filterflag|0x0080;
			    amp_decode (pch_ContainerID, strtmp);
			    MakeObjectID (strtmp, ContainerID_str);
			    MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
		    else if((pch_upnpartist =strstr (pch_SearchCriteria,"@refID exists false")) != NULL){
			    amp_decode (pch_ContainerID, strtmp);
			    MakeObjectID (strtmp, ContainerID_str);
		//	    if(g_var->g_filterflag==0xFFFF)
		//		    g_var->g_filterflag=0;
		//	    g_var->g_filterflag= g_var->g_filterflag|0x0080;
			    MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
		   // else if(strstr (pch_SearchCriteria,"upnp:class derivedfrom \"object\"")){
		    else if(strstr (pch_SearchCriteria,"upnp:class derivedfrom \"object\"")){
			    MediaMetaDataBrowseAction (ca_event, pch_ContainerID, 0, special_agent,g_var);
		    }
		    else  {
			    amp_decode (pch_ContainerID, strtmp);
			    MakeObjectID (strtmp, ContainerID_str);
			    MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
		    }
	    }
	}
	else {
		if (pch_ContainerID) {
			amp_decode (pch_ContainerID, strtmp);
		}
		else
			goto ERROR;
		MakeObjectID (strtmp, ContainerID_str);
		
		if (strstr(ContainerID_str, "B13") == ContainerID_str /*Movie Folder*/
			|| strstr(ContainerID_str, "B27") == ContainerID_str /*Music Folder*/
			|| strstr(ContainerID_str, "B36") == ContainerID_str/*Photo Folder*/){	
			int media_type=MEDIA_TYPE_NONE,ret=0;
			char *pResult=NULL;
			sqlite *pDB=NULL;
					
			if (strstr(ContainerID_str, "B13") == ContainerID_str)
				media_type=VIDEO;
			else if (strstr(ContainerID_str, "B27") == ContainerID_str)
				media_type=MUSIC;
			else if (strstr(ContainerID_str, "B36") == ContainerID_str)
				media_type=PICTURE;	
			sqlite_open (db_file_path, &pDB);
			if (pDB == NULL){
				goto ERROR;
			}
			ret=GetSubDirEntries(&pResult, pDB, ContainerID_str, media_type, g_var->g_startingindex, g_var->g_requestedcount,special_agent,0,g_var);
			if(ret){
				sqlite_close (pDB);	
				goto ERROR;
			}
			ca_event->ActionResult = ixmlParseBuffer (pResult);
			free (pResult);
			pResult = NULL;
			sqlite_close (pDB);		
		}	
		else if(strstr (pch_SearchCriteria,"upnp:class derivedfrom \"object\"")){
		    	if (pch_ContainerID[0]=='A' || pch_ContainerID[0]=='B' || pch_ContainerID[0]=='C'){
				MediaMetaDataBrowseAction (ca_event, pch_ContainerID, 0, special_agent,g_var);
		    	}
		    	else{
				    amp_decode (pch_ContainerID, strtmp);
				MakeObjectID (strtmp, ContainerID_str);
				MediaSQLAction (ca_event, ContainerID_str, g_var->select_type, 0, special_agent,g_var);
			}
	    }
		else{
			for (i = 4; i < NUM_Media_catalog;) {
				if (strstr(ContainerID_str,media_catalog_table[i].ch_containerid) != NULL) {
					strcpy (g_var->g_root, media_catalog_table[i].ch_containerid);
					strcpy (g_var->g_parentid, ContainerID_str);
					g_var->select_type = media_catalog_table[i].n_type;
					MediaSQLAction (ca_event, ContainerID_str,media_catalog_table[i].n_type, 0, special_agent,g_var);
					break;
				}
				i = i + 1;
			}
		}
	}
ERROR:
	if (pch_StartingIndex) {
		free (pch_StartingIndex);
		pch_StartingIndex = NULL;
	}

	if (pch_RequestedCount) {
		free (pch_RequestedCount);
		pch_RequestedCount = NULL;
	}

	if (pch_Filter) {
		free (pch_Filter);
		pch_Filter = NULL;
	}
	if (pch_ContainerID) {
		free (pch_ContainerID);
		pch_ContainerID = NULL;
	}
	if (pch_SearchCriteria) {
		free (pch_SearchCriteria);
		pch_SearchCriteria = NULL;
	}

	return 0;
}

/******************************************************************************
 *MediaBrowseAction
 *
 *Description
 *       deal with Browse action
 *        put result to buffer
 *Parameters:
 *        ca_event -- The control action request event structure
 *        parent_id-- The parent id get information content form media_catalog_table
 *        special_agent - The flag for agent, standard, xbox, or others
 *
 ******************************************************************************/
int MediaBrowseAction (struct Upnp_Action_Request *ca_event, char *parent_id, int special_agent,struct g_variable *g_var)
{
	char ch_title[] = "<u:BrowseResponse  xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">";
	char ch_beginformat[] = "<Result>&lt;DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"&gt;\n&lt;";
	char ch_endformat[200] = { 0 };
	char tmp_string[128] = { 0 };
	char *pch = NULL;
	char *pch_temp = NULL;
	char *result_str = NULL;
	char *p_chtemp = NULL;
	unsigned long n_TotalMatche = 0;
	int n_flag = 0;
	int n_tmplength = 0;
	int n_TotalNameLength = 0;
	unsigned long n_NumberReturned = 0;
	int n_memsize = 0;
	int n_consttmplength = 1024;
	int i = 0;
	int j = 0;
	unsigned long nChildCount = 0;
	int count = 0;
	char ObjectID_str[1024] = { 0 };
	int ncatalogcount = 0;
	int rc = 0;
	char *extrastr = NULL;
	//char* zErrMsg=NULL;
	struct MediaService *pMediaService = NULL;
	char upnp_class[32] = { 0 };

	strcpy (ObjectID_str, parent_id);
	ncatalogcount = NUM_Media_catalog;
	pMediaService = (struct MediaService *)media_catalog_table;
	if (special_agent==XBOX360)
	    sprintf(upnp_class, "%s", "object.container.storageFolder");
	else
	    sprintf(upnp_class, "%s", "object.container");
	pch = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "Filter", 0);
	if(pch==NULL) {
		ca_event->ErrCode = 402;
		strcpy (ca_event->ErrStr, "Invalid Args");
		ca_event->ActionResult = NULL;
		return (ca_event->ErrCode);
	}
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	if (strcmp (pch, "*") == 0)
		n_tmplength = 80;
	else
		n_tmplength = 0;
	for (i = 0; i < ncatalogcount; i++) {
		if (strcmp (pMediaService[i].ch_parentid, parent_id) == 0) {//Find one child
			if (special_agent==XBOX360 && pMediaService[i].n_type == SEL_PHOTO_FOLDER)
				continue;
			n_TotalMatche = n_TotalMatche + 1;
			if (g_var->g_startingindex > count) { //Not reach first one.
				count = count + 1;
				continue;
			}
			if (g_var->g_requestedcount != 0 && n_NumberReturned == g_var->g_requestedcount) //Reach maximum entries
				continue;//Continue to count the total matches.
			n_TotalNameLength =
						n_TotalNameLength +
						strlen (pMediaService[i].ch_containerid) +
						strlen (pMediaService[i].ch_parentid) +
						strlen (pMediaService[i].ch_restricted) +
						strlen (pMediaService[i].ch_title) + n_tmplength;;
			count +=1;
			n_NumberReturned +=1;
		}
	}
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	sprintf (ch_endformat, "/DIDL-Lite&gt;</Result><NumberReturned>%lu</NumberReturned><TotalMatches>%lu</TotalMatches><UpdateID>%u</UpdateID></u:BrowseResponse>", n_NumberReturned, n_TotalMatche, GetUpdateIDByContainer(ObjectID_str));
	pch_temp = (char *) malloc (n_consttmplength * n_TotalMatche + n_TotalNameLength + 1);
	memset (pch_temp, 0, (n_consttmplength * n_TotalMatche + n_TotalNameLength + 1));
	count = 0;
	n_NumberReturned = 0;
	for (i = 0; i < ncatalogcount; i++) {
		if (strcmp (pMediaService[i].ch_parentid, parent_id) == 0) {
			if (special_agent==XBOX360 && pMediaService[i].n_type == SEL_PHOTO_FOLDER)
				continue;			
			nChildCount = 0;
			if (g_var->g_startingindex > count) {//Not reach first one.
				count = count + 1;
				continue;
			}
			if (g_var->g_requestedcount != 0 && n_NumberReturned == g_var->g_requestedcount)//Reach max entries
				break;
			g_var->g_nmallocLen = 1000;
			if ((extrastr = (char *) malloc (g_var->g_nmallocLen)) == NULL)
				return -1;
			memset (extrastr, 0, g_var->g_nmallocLen);
			//printf("%s:%d: pch=%s, parent_id=%s, ch_containerid=%s, n_flag=%d\n",  __FUNCTION__, __LINE__, pch, parent_id, pMediaService[i].ch_containerid, n_flag);
			if (strcmp (pch, "*") == 0 || g_var->g_filterflag & 0x8000) {
				if (strcmp (parent_id, "0") == 0 && strcmp (pMediaService[i].ch_containerid, "A004") != 0) {
					for (j = i + 1; j < NUM_Media_catalog; j++){
						if (strcmp(pMediaService[i].ch_containerid, pMediaService[j].ch_parentid) == 0)
							nChildCount = nChildCount + 1;
					}
				}
				else {
					sqlite *db1=NULL;
					char countsql[500] = { 0 };
	//				int media_type=MEDIA_TYPE_NONE;
					
					sqlite_open (db_file_path, &db1);
					if (db1 == NULL){
						if (pch) {
							free (pch);
							pch = NULL;
						}						
						return SQLITE_ERROR;
					}

#if 0					
					if (!strcmp(pMediaService[i].ch_containerid, "B13"))
						media_type=VIDEO;
					else if (!strcmp(pMediaService[i].ch_containerid, "B27"))
						media_type=MUSIC;
					else if (!strcmp(pMediaService[i].ch_containerid, "B36"))
						media_type=PICTURE;		
					printf("pMediaService[i].ch_containerid=%s\n",pMediaService[i].ch_containerid);				
					//Shearer
					if(media_type!=MEDIA_TYPE_NONE && g_filterflag & 0x8000){
						nChildCount=0;
						nChildCount=GetChildCountOfDir(db1,"/", media_type);
					}
					else{
#endif					
						rc = MakeCountSQLSentence (db1, pMediaService[i].n_type, ObjectID_str, countsql, extrastr,g_var);
						nChildCount = g_var->g_totalmatches;
					//}
					sqlite_close (db1);
#ifdef __DEBUG__					
					printf("%s:%d  g_var->g_totalmatches:%lu\n",__FUNCTION__,__LINE__,g_var->g_totalmatches);
#endif					
					g_var->g_totalmatches = 0;
				}
				sprintf (tmp_string, "childCount=\"%lu\"", nChildCount);
			}
			if (n_flag == 0) { //Header
				sprintf (pch_temp, "container id=\"%s\"  parentID=\"%s\" restricted=\"%s\" %s &gt;\
					\n&lt;dc:title&gt;%s&lt;/dc:title&gt;\
					\n&lt;upnp:class&gt;%s&lt;/upnp:class&gt;",
							 pMediaService[i].ch_containerid,  pMediaService[i].ch_parentid, pMediaService[i].ch_restricted, tmp_string, pMediaService[i].ch_title, upnp_class);
#if 0
				if(g_var->g_filterflag&0x0080)
					sprintf(pch_temp,"%s\n&lt;upnp:writeStatus&gt;UNKNOWN&lt;/upnp:writeStatus&gt;",pch_temp);
#endif
				sprintf (pch_temp, "%s\n&lt;/container&gt;\n&lt;", pch_temp);
				n_flag = 1;
			}
			else {
				p_chtemp = (char *) malloc (n_consttmplength +
										 strlen (pMediaService[i].
												 ch_containerid) +
										 strlen (pMediaService[i].
												 ch_parentid) +
										 strlen (pMediaService[i].
												 ch_restricted) +
										 strlen (pMediaService[i].ch_title));
				sprintf (p_chtemp, "container id=\"%s\"  parentID=\"%s\" restricted=\"%s\" %s &gt;\
				\n&lt;dc:title&gt;%s&lt;/dc:title&gt;\
				\n&lt;upnp:class&gt;%s&lt;/upnp:class&gt;",
							 pMediaService[i].ch_containerid,  pMediaService[i].ch_parentid, pMediaService[i].ch_restricted, tmp_string, pMediaService[i].ch_title, upnp_class);
#if 0
				if(g_var->g_filterflag&0x0080)
					   sprintf(p_chtemp,"%s\n&lt;upnp:writeStatus&gt;UNKNOWN&lt;/upnp:writeStatus&gt;",p_chtemp);
#endif
				sprintf (p_chtemp, "%s\n&lt;/container&gt;\n&lt;",
						 p_chtemp);
				strcat (pch_temp, p_chtemp);
				if (p_chtemp)
					free (p_chtemp);
			}
			if (extrastr) {
				free (extrastr);
				extrastr = NULL;
			}
			count = count + 1;
			n_NumberReturned = n_NumberReturned + 1;
#ifdef __DEBUG__			
			printf("%s: %d***n_NumberReturned=%lu\n", __FUNCTION__, __LINE__,n_NumberReturned);
#endif			
		}
	}
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	if (pch) {
		free (pch);
		pch = NULL;
	}	
	n_memsize = strlen (ch_title) + strlen (ch_beginformat) + strlen (ch_endformat) + n_TotalMatche * n_consttmplength + n_TotalNameLength + 1;
	if ((result_str = (char *) malloc (n_memsize)) == NULL) {
		if (pch_temp)
			free (pch_temp);
		return -1;
	}
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	sprintf (result_str, "%s%s%s%s", ch_title, ch_beginformat, pch_temp, ch_endformat);
	 //mBUG("%s:%s():%d:result={%s}\n", __FILE__, __FUNCTION__, __LINE__, result_str);
	if (pch_temp) {
		free (pch_temp);
		pch_temp = NULL;
	}
	//printf("%s: %d\n", __FUNCTION__, __LINE__);
	ca_event->ActionResult = ixmlParseBuffer (result_str);
	if (result_str) {
		free (result_str);
		result_str = NULL;
	}
	return 0;
}

int MakeObjectID (char *src, char *dst)
{
	char strtmp[1024] = { 0 };
	char strdst[1024] = { 0 };
	char *pchObjectID = NULL;
	int i = 0;

	strcpy (strtmp, src);
	if ((pchObjectID = strstr (strtmp, "/..\0")) != NULL) {
		*pchObjectID = '\0';
		MakeObjectID (strtmp, strdst);
		strcpy (strtmp, strdst);
		for (i = 0; i < NUM_Media_catalog; i++) {
			if (strcmp (strtmp, media_catalog_table[i].ch_containerid) == 0)
				strcpy (dst, media_catalog_table[i].ch_parentid);
			else {
			    strcpy (dst, strtmp);
			}
		}
	}
	else {
	    strcpy (dst, strtmp);
	}
	return 0;
}

/*******************************************************************************
 * MediaDeviceHandleActionRequest
 *
 * Description:
 *       Called during an action request callback.
 *	 Check which service is requested.
 *	 Perform the action :
 * 			 set/acquire status table, system calls, set/acquire
 * 			 portmapping table
 *	 Respond.
 *
 * Parameters:
 *   ca_event -- The control action request event structure
 *
 ******************************************************************************/

int MediaDeviceHandleActionRequest (struct Upnp_Action_Request *ca_event)
{
	char input_string[DS_MAX_VAL_LEN];
	char ObjectID_str[1024] = { 0 };
	char strtmp[1024] = { 0 };
	char *pch = NULL;
	char *pchBrowseFlag = NULL;
	int i = 0,ret=0;
	int ch;
	int action_succeeded = -1;
	int err = 401,special_agent=STANDARD;
	struct g_variable g_var;
	
	memset(&g_var,0,sizeof(g_var));//init g_var	
//#define __DEBUG__
#ifdef __DEBUG__
	char val[256];
	uint32_t ip;

	ip = ca_event->CtrlPtIPAddr.s_addr;
	ip = ntohl (ip);
	sprintf (val, "%d.%d.%d.%d", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
	printf("%s:%d: CtrlPtIP=%s\n", __FUNCTION__, __LINE__, val);
	printf("%d: Agent=%s\n", __LINE__, ca_event->user_agent);
#endif
#ifdef SC_MUTIL_GROUP
	if(ca_event && strlen(ca_event->HOST))
	{
		char *p;
		strncpy(media.InternalIPAddress,ca_event->HOST,sizeof(media.InternalIPAddress));
		if(p = strchr(media.InternalIPAddress,':'))
		{
			*p = '\0';
		}
	}
#endif /* SC_MUTIL_GROUP */
	ca_event->ErrCode = 0;
	ca_event->ActionResult = NULL;
	if(strstr(ca_event->user_agent,"Xbox")){
		ca_event->Xbox360=1;
	}
	if(ca_event->Xbox360)
		special_agent=XBOX360;
	memset (input_string, 0, DS_MAX_VAL_LEN);
	/********************************
     * Action 1  *
     ********************************/
	if ((strcmp(ca_event->DevUDN,ds_service_table[DS_SERVICE_CONNECTIONMANAGER].UDN) == 0)
		&&(strcmp(ca_event->ServiceID,ds_service_table[DS_SERVICE_CONNECTIONMANAGER].ServiceId) == 0)) {
		//printf("ca_event->ActionName=%s\n",ca_event->ActionName);
		if (strcmp (ca_event->ActionName, "GetProtocolInfo") == 0) {
			MediaGetProtocolInfoAction (ca_event);
			action_succeeded = 1;
		}
		else if (strcmp (ca_event->ActionName, "GetCurrentConnectionIDs") == 0) {
			MediaGetCurrentConnectionIDsAction (ca_event);
			action_succeeded = 1;
		}
		else if (strcmp (ca_event->ActionName, "GetCurrentConnectionInfo") == 0) {
			MediaGetCurrentConnectionInfoAction (ca_event);
			action_succeeded = 1;
		}
		else {
			action_succeeded = -1;
			mBUG("Not support action: %s", ca_event->ActionName);
			goto out;
		}
	}
	else if ((strcmp (ca_event->DevUDN, ds_service_table[DS_SERVICE_CONTENTDIRECTORY].UDN) == 0)
			 && (strcmp (ca_event->ServiceID, ds_service_table[DS_SERVICE_CONTENTDIRECTORY].ServiceId) == 0)) {//Equal to local server.
		if (strcmp (ca_event->ActionName, "Browse") == 0) {
			pch = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "Filter", 0);
			if(pch==NULL){
				ca_event->ErrCode = 402;
				strcpy (ca_event->ErrStr, "Invalid Args");
				ca_event->ActionResult = NULL;
				return (ca_event->ErrCode);
			}
			//printf("pch=%s\n",pch);
			if (strcmp (pch, "*") == 0) {
				g_var.g_filterflag = 0xFFFF;
			}
			else {
				for (i = 0; i < SORTCRITERIANUM; i++) {
					//printf("filter_table[%d].strsortname=%s\n",i,filter_table[i].strsortname);
					if ((strstr (pch, filter_table[i].strsortname)) != NULL)
					g_var.g_filterflag |= filter_table[i].nflag;
				}
			}
			if (pch) {
				free (pch);
				pch = NULL;
			}
			if(special_agent==XBOX360)
				pch = SampleUtil_GetDocumentItem(ca_event->ActionRequest, "ContainerID", 0);
			else
				pch = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "ObjectID", 0);
				
			if(pch==NULL) {
				action_succeeded=-4;
				goto out;
			}
			else if(!strlen(pch)){ //Empty Container ID.
				action_succeeded=-6;
				free (pch);
				pch = NULL;
				goto out;
			}
			amp_decode (pch, strtmp);
			if (pch) {
				free (pch);
				pch = NULL;
			}
			MakeObjectID (strtmp, ObjectID_str);
#ifdef __DEBUG__			
			printf("ObjectID_str=%s\n",ObjectID_str);
#endif			
			pchBrowseFlag = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "BrowseFlag", 0);
			if(pchBrowseFlag==NULL){
				action_succeeded=-10;
			}
			else if (strcmp (pchBrowseFlag, "BrowseMetadata") == 0) {
				pch = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "StartingIndex", 0);
				if (pch == NULL){
					action_succeeded=-10;
					goto out;
				}
				g_var.g_startingindex = strtoul (pch, NULL, 10);
				if (pch) {
					free (pch);
					pch = NULL;
				}
				pch = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "RequestedCount", 0);
				if (pch == NULL){
					action_succeeded=-10;
					goto out;
				}
				g_var.g_requestedcount = strtoul (pch, NULL, 10);
				if (pch) {
					free (pch);
					pch = NULL;
				}
#ifdef __DEBUG__				
				printf("BrowseMetadata: g_startingindex=%lu\n",g_var.g_startingindex);
				printf("BrowseMetadata: g_requestedcount=%lu\n",g_var.g_requestedcount);
#endif				
				ret=MediaMetaDataBrowseAction (ca_event, ObjectID_str, 1, special_agent,&g_var);
				if(ret){
					action_succeeded = -4;
					goto out;	
				}
				action_succeeded = 1;
			}
			else if (strcmp (pchBrowseFlag, "BrowseDirectChildren") == 0) {
#ifdef __DEBUG__				
				printf("%s:%d :ObjectID=%s, VIRTUAL_DIR=%s\n", __FUNCTION__, __LINE__, ObjectID_str, VIRTUAL_DIR);
#endif					
				if(strstr(ObjectID_str, VIRTUAL_DIR)){
					action_succeeded=-1;
					goto out;	
				}
				ch = '/';
				pch = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "StartingIndex", 0);
				if (pch == NULL) {
					action_succeeded=-10;
					goto out;
				}
				g_var.g_startingindex = strtoul (pch, NULL, 10);
				if (pch) {
					free (pch);
					pch = NULL;
				}
				pch = SampleUtil_GetDocumentItem (ca_event->ActionRequest, "RequestedCount", 0);
				
				if (pch == NULL) {
					action_succeeded=-10;
					goto out;

				}
				g_var.g_requestedcount = strtoul (pch, NULL, 10);
				if (pch) {
					free (pch);
					pch = NULL;
				}
#ifdef __DEBUG__				
				printf("BrowseDirectChildren: g_startingindex=%lu\n",g_var.g_startingindex);
				printf("BrowseDirectChildren: g_requestedcount=%lu\n",g_var.g_requestedcount);				
				printf("%s:%d :ObjectID=%s, special_agent=%d\n", __FUNCTION__, __LINE__, ObjectID_str, special_agent);
#endif				
				if (strstr(ObjectID_str, "B13") == ObjectID_str /*Movie Folder*/
					|| strstr(ObjectID_str, "B27") == ObjectID_str /*Music Folder*/
					|| strstr(ObjectID_str, "B36") == ObjectID_str/*Photo Folder*/){	
					int media_type=MEDIA_TYPE_NONE;
					char *pResult=NULL;
					sqlite *pDB=NULL;
					
					if (strstr(ObjectID_str, "B13") == ObjectID_str)
						media_type=VIDEO;
					else if (strstr(ObjectID_str, "B27") == ObjectID_str)
						media_type=MUSIC;
					else if (strstr(ObjectID_str, "B36") == ObjectID_str)
						media_type=PICTURE;	
					sqlite_open (db_file_path, &pDB);
					if (pDB == NULL){
						action_succeeded = -4;
						goto out;
					}
					ret=GetSubDirEntries(&pResult, pDB, ObjectID_str, media_type, g_var.g_startingindex, g_var.g_requestedcount,special_agent,1,&g_var);
					if(ret){
						sqlite_close (pDB);	
						action_succeeded = -4;
						goto out;
					}
#if 0
	{
		FILE *ff=NULL;
		
		ff=fopen("pResult.txt", "at");
		if(ff){
			fprintf(ff,"pResult=%s\n",pResult);	
			fclose(ff);
		}	
	}
#endif						
					ca_event->ActionResult = ixmlParseBuffer (pResult);
					free (pResult);
					pResult = NULL;
					sqlite_close (pDB);		
				}
				else{
					if(special_agent==XBOX360){
						if(strcmp(ObjectID_str,"15") ==0 ){
							MediaBrowseAction (ca_event, "A1", special_agent,&g_var);//Videos
						}
						else if(strcmp(ObjectID_str,"16") == 0){//
							MediaBrowseAction (ca_event, "A3", special_agent,&g_var);//Pictures
						}
						else if(strcmp(ObjectID_str, "B26") == 0) {	//Playlists
							MediaBrowseAction (ca_event, ObjectID_str, special_agent,&g_var);		
						}			
						else{
							for( i = 4 ; i<NUM_Media_catalog ;) {
								if( strstr(ObjectID_str,media_catalog_table[i].ch_containerid) != NULL ) {
									strcpy(g_var.g_root,media_catalog_table[i].ch_containerid);
									strcpy(g_var.g_parentid,ObjectID_str);
									g_var.select_type = media_catalog_table[i].n_type;
									strcpy(g_var.g_upnpclass,"object.container.storageFolder");
									MediaSQLAction(ca_event,ObjectID_str,media_catalog_table[i].n_type,1, special_agent,&g_var);
									break;
								}
								i = i + 1;
							}
						}
					}
					else{
						if (strcmp (ObjectID_str, "0") == 0
								|| strcmp (ObjectID_str, "A1") == 0 //Videos
								|| strcmp (ObjectID_str, "A2") == 0	//Music
								|| strcmp (ObjectID_str, "A3") == 0	//Pictures
								|| strcmp (ObjectID_str, "B26") == 0) {	//Playlists
							MediaBrowseAction (ca_event, ObjectID_str, special_agent,&g_var);
						}
						else if(strcmp(ObjectID_str,"0\\Movie\\")==0)
						{
							strcpy(ObjectID_str,"A1");
							MediaBrowseAction (ca_event, ObjectID_str, special_agent,&g_var);
						}
						else if(strcmp(ObjectID_str,"0\\Photo\\")==0)
						{
							strcpy(ObjectID_str,"A3");
							MediaBrowseAction (ca_event, ObjectID_str, special_agent,&g_var);
						}
						else if(strcmp(ObjectID_str,"0\\Music\\")==0)
						{
							strcpy(ObjectID_str,"A2");
							MediaBrowseAction (ca_event, ObjectID_str, special_agent,&g_var);
						}
						else {
#ifdef __DEBUG__							
							printf("%s: %d ObjectID_str=%s\n", __FUNCTION__, __LINE__, ObjectID_str);
#endif							
							for (i = 4; i < NUM_Media_catalog;) {
								if (strstr (ObjectID_str, media_catalog_table[i].ch_containerid) != NULL) {
									strcpy (g_var.g_root, media_catalog_table[i]. ch_containerid); //Root Container
									strcpy (g_var.g_parentid, ObjectID_str);						 //Current Container
									g_var.select_type = media_catalog_table[i].n_type;			 //Container Type
									MediaSQLAction (ca_event, ObjectID_str, media_catalog_table[i].n_type, 1, special_agent,&g_var);
									break;
								}
								i = i + 1;
							}
							if(i>=NUM_Media_catalog) {
								action_succeeded = -4;
								goto out;
							}
						}
					}
				}
				action_succeeded = 1;
			}
			if (pchBrowseFlag) {
				free (pchBrowseFlag);
				pchBrowseFlag = NULL;
			}
			g_var.g_filterflag = 0;
			/*
			pch = SampleUtil_GetDocumentItem(ca_event->ActionRequest, "SortCriteria", 0);
			if(pch == NULL || !strlen(pch)) {
				action_succeeded=-8;
				goto out;
			}//test browse with an invalid sort criteria. 
			*/
		}
		else if (strcmp (ca_event->ActionName, "GetSearchCapabilities") == 0) {
			//mBUG("GetSearchCapabilities");
			MediaGetSearchCapabilitiesAction (ca_event);
			action_succeeded = 1;
		}
		else if (strcmp (ca_event->ActionName, "GetSortCapabilities") ==  0) {
			//mBUG("GetSortCapabilities");
			MediaGetSortCapabilitiesAction (ca_event);
			action_succeeded = 1;
		}
		else if (strcmp (ca_event->ActionName, "GetSystemUpdateID") == 0) {
			MediaGetSystemUpdateIDAction (ca_event);
			action_succeeded = 1;
		}
		else if (strcmp (ca_event->ActionName, "Search") == 0) {
			char *p1 = NULL, *p2 = NULL, *p3 = NULL;// *p4 = NULL;
			
			p1 = SampleUtil_GetDocumentItem(ca_event->ActionRequest, "ContainerID", 0);
			p2 = SampleUtil_GetDocumentItem(ca_event->ActionRequest, "SearchCriteria", 0);
			p3 = SampleUtil_GetDocumentItem(ca_event->ActionRequest, "Filter", 0);
			/*
			p4 = SampleUtil_GetDocumentItem(ca_event->ActionRequest, "SortCriteria", 0);
			*/
			if( p1 == NULL || p2 == NULL || p3 == NULL)// || p4 == NULL)
				action_succeeded=-10;
			if (!strlen(p1) && !strlen(p2) && !strlen(p3))// && strcmp (p4, "") == 0)
				action_succeeded = -4;
			else{			// exec actual search function.
				action_succeeded=MediaSearchAction (ca_event,&g_var);
				if(action_succeeded!=0)
				{
					if(p1){
						free(p1);
						p1=NULL;
					}
					if(p2){
						free(p2);
						p2=NULL;
					}
					if(p3){
						free(p3);
						p3=NULL;
					}
					goto out;
				}
				action_succeeded = 1;
				g_var.g_filterflag = 0;
			}
			if(p1){
				free(p1);
				p1=NULL;
			}
			if(p2){
				free(p2);
				p2=NULL;
			}
			if(p3){
				free(p3);
				p3=NULL;
			}

		}
		else {
			action_succeeded = -1;
			mBUG("Not support action: %s", ca_event->ActionName);
			goto out;
		}
	}
	else if ((strcmp(ca_event->DevUDN, ds_service_table[DS_SERVICE_CONTENTDIRECTORY].UDN) == 0)
			 && (strcmp(ca_event->ServiceID, "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar") == 0)) {
	    mBUG("%s:%d:ActionName=%s", __FILE__, __LINE__, ca_event->ActionName);
		if (strcmp (ca_event->ActionName, "IsAuthorized") == 0) {
			MediaIsAuthorized (ca_event);
			action_succeeded = 1;
		}
		else if (strcmp (ca_event->ActionName, "IsValidated") == 0) {
			MediaIsValidated (ca_event);
			action_succeeded = 1;
		}
		else if (strcmp (ca_event->ActionName, "RegisterDevice") == 0) {
			MediaRegisterDevice (ca_event);
			action_succeeded = 1;
		}
	}
out:
	if (pchBrowseFlag) {
		free (pchBrowseFlag);
		pchBrowseFlag = NULL;
	}
	if (action_succeeded > 0) {
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else if (action_succeeded == -1) {
		ca_event->ErrCode = err;
		strcpy (ca_event->ErrStr, "Invalid Action");
		ca_event->ActionResult = NULL;
	}
	else if (action_succeeded == -2) {
		ca_event->ErrCode = 713;
		strcpy (ca_event->ErrStr, "SpecifiedArrayIndexInvalid");
		ca_event->ActionResult = NULL;
	}
	else if (action_succeeded == -3) {
		ca_event->ErrCode = 714;
		strcpy (ca_event->ErrStr, "NoSuchEntryInArray");
		ca_event->ActionResult = NULL;
	}
	else if (action_succeeded == -4) {
		ca_event->ErrCode = 701;
		strcpy (ca_event->ErrStr, "No such object");
		ca_event->ActionResult = NULL;
	}
	else if (action_succeeded == -5) {
		ca_event->ErrCode = 718;
		strcpy (ca_event->ErrStr, "ConflictInMappingEntry");
		ca_event->ActionResult = NULL;
	}
	else if(action_succeeded == -6){
		ca_event->ErrCode = 710;
		strcpy (ca_event->ErrStr, "No such container");
		ca_event->ActionResult = NULL;
	}
	else if(action_succeeded==-7){
		ca_event->ErrCode = 708;
		strcpy (ca_event->ErrStr, "Unsupported or invalid search criteria");
		ca_event->ActionResult = NULL;
	}
	else if(action_succeeded==-8){
		ca_event->ErrCode = 709;
		strcpy (ca_event->ErrStr, "Unsupported or invalid sort criteria");
		ca_event->ActionResult = NULL;
	}

	else {
#ifdef __DEBUG__		
		printf ("Error in UPNP_CONTROL_ACTION_REQUEST callback:\n");
		printf ("Failure while running %s\n", ca_event->ActionName);
#endif		
		ca_event->ErrCode = 402;
		strcpy (ca_event->ErrStr, "Invalid Args");
		ca_event->ActionResult = NULL;
	}
	return (ca_event->ErrCode);
}

int MediaGetSearchCapabilitiesAction (struct Upnp_Action_Request
										  *ca_event)
{
	char str[] ="<u:GetSearchCapabilitiesResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\"><SearchCaps>upnp:artist,upnp:genre,upnp:album,upnp:class,res@protocolInfo,dc:title,dc:creator,@refID</SearchCaps></u:GetSearchCapabilitiesResponse>";
	ca_event->ActionResult = ixmlParseBuffer (str);
	return 1;
}

int MediaGetSortCapabilitiesAction (struct Upnp_Action_Request *ca_event)
{
	char str[] = "<u:GetSortCapabilitiesResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\"><SortCaps></SortCaps></u:GetSortCapabilitiesResponse>";
	ca_event->ActionResult = ixmlParseBuffer (str);
	return 1;
}

int MediaGetSystemUpdateIDAction (struct Upnp_Action_Request *ca_event)
{
	char str[] = "<u:GetSystemUpdateIDResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\"><Id>1</Id></u:GetSystemUpdateIDResponse>";
	ca_event->ActionResult = ixmlParseBuffer (str);
	return 1;
}

int MediaIsAuthorized (struct Upnp_Action_Request *ca_event)
{
	char str[] = "<m:IsAuthorizedResponse xmlns:m=\"urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1\"><Result>1</Result></m:IsAuthorizedResponse>";
	ca_event->ActionResult = ixmlParseBuffer (str);
	return 1;
}

int MediaIsValidated (struct Upnp_Action_Request *ca_event)
{
	char str[] = "<m:IsValidatedResponse xmlns:m=\"urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1\"><Result>1</Result></m:IsValidatedResponse>";
	ca_event->ActionResult = ixmlParseBuffer (str);
	return 1;
}

int MediaRegisterDevice (struct Upnp_Action_Request *ca_event)
{
	return 1;
}

extern char protocolinfo_file[];

int MediaGetProtocolInfoAction (struct Upnp_Action_Request *ca_event)
{
	FILE *fp = NULL;
	char buf[256]={0};
	char str1[]=" <u:GetProtocolInfoResponse xmlns:u=\"urn:schemas-upnp-org:service:ConnectionManager:1\"><Source>";
	char str2[]="</Source><Sink></Sink></u:GetProtocolInfoResponse>";
	char *buf1=NULL, *p=NULL; 
	struct stat f_stat;
	
	if(stat(protocolinfo_file, &f_stat) || !f_stat.st_size)
		return -1;
	fp = fopen(protocolinfo_file, "rt");
	if (fp == NULL) {
		mBUG("open file failed");
		return -1;
	}
	buf1=(char *)malloc(f_stat.st_size+strlen(str1)+strlen(str2)+32);
	if(!buf1){
		fclose(fp);	
		return -1;
	}
	strcpy(buf1,str1); 
	while(fgets(buf,256,fp)!=NULL){
		p=strrchr(buf, '\n');
		if(p)
			*p=0;
		strcat(buf1,buf);
	}
	strcat(buf1,str2);
	fclose(fp);
	ca_event->ActionResult = ixmlParseBuffer (buf1);
	free(buf1);
	buf1=NULL;
	return 1;
}

int MediaGetCurrentConnectionIDsAction (struct Upnp_Action_Request *ca_event)
{
	char str[] = "<u:GetCurrentConnectionIDsResponse xmlns:u=\"urn:schemas-upnp-org:service:ConnectionManager:1\">\
		<ConnectionIDs>0</ConnectionIDs></u:GetCurrentConnectionIDsResponse>";
	ca_event->ActionResult = ixmlParseBuffer (str);
	return 1;
}

int MediaGetCurrentConnectionInfoAction (struct Upnp_Action_Request
											 *ca_event)
{
	char str[] = "<u:GetCurrentConnectionInfoResponse xmlns:u=\"urn:schemas-upnp-org:service:ConnectionManager:1\">\
		<RcsID>-1</RcsID><AVTransportID>-1</AVTransportID><ProtocolInfo></ProtocolInfo><PeerConnectionManager></PeerConnectionManager>\
		<PeerConnectionID>-1</PeerConnectionID><Direction>Output</Direction><Status>Unknown</Status></u:GetCurrentConnectionInfoResponse>";
	ca_event->ActionResult = ixmlParseBuffer (str);
	return 1;
}

/********************************************************************************
 * MediaDeviceCallbackEventHandler
 *
 * Description:
 *       The callback handler registered with the SDK while registering
 *       root device or sending a search request.  Detects the type of
 *       callback, and passes the request on to the appropriate procedure.
 *
 * Parameters:
 *   EventType -- The type of callback event
 *   Event -- Data structure containing event data
 *   Cookie -- Optional data specified during callback registration
 *
 ********************************************************************************/
int MediaDeviceCallbackEventHandler (Upnp_EventType EventType, void *Event, void *Cookie)
{
#ifdef __DEBUG__	
	printf("%s: %d: EventType=%d\n", __FUNCTION__, __LINE__, EventType);
#endif	
	//pthread_mutex_lock (&MESDevMutex);
	switch (EventType) {
	    case UPNP_EVENT_SUBSCRIPTION_REQUEST:
			IGDDeviceHandleSubscriptionRequest((struct Upnp_Subscription_Request *) Event);
		break;
	    case UPNP_CONTROL_ACTION_REQUEST:
			MediaDeviceHandleActionRequest ((struct Upnp_Action_Request *) Event);
		break;
	    case UPNP_CONTROL_ACTION_COMPLETE:
	    case UPNP_CONTROL_GET_VAR_REQUEST:
	    default:
			mBUG ("Error in MediaDeviceCallbackEventHandler: unknown event type %d\n", EventType);
		break;
	}
	//pthread_mutex_unlock (&MESDevMutex);
#ifdef __DEBUG__	
	printf("%s: %d \n", __FUNCTION__, __LINE__);
#endif	
	return (0);
}

/********************************************************************************
 * IGDDeviceHandleSubscriptionRequest
 *
 * Description:
 *       Called during a subscription request callback.  If the
 *       subscription request is for this device and either its
 *       control service or picture service, then accept it.
 *
 * Parameters:
 *   sr_event -- The subscription request event structure
 *
 ********************************************************************************/
int IGDDeviceHandleSubscriptionRequest(struct Upnp_Subscription_Request *sr_event)
{
    int i;
    IXML_Document *PropSet;

    pthread_mutex_lock(&DSDevMutex);

    for (i=0; i<DS_SERVICE_SERVCOUNT1; i++) {
	if ((strcmp(sr_event->UDN,ds_service_table[i].UDN) == 0) &&
		(strcmp(sr_event->ServiceId,ds_service_table[i].ServiceId) == 0)) {
	    switch(i){
			case  DS_SERVICE_CONNECTIONMANAGER:
			    PropSet = NULL;
			    UpnpAddToPropertySet(&PropSet, ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableName[6],
				    ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableStrVal[6]);
			    UpnpAddToPropertySet(&PropSet, ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableName[7],
				    ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableStrVal[7]);
	
			    UpnpAddToPropertySet(&PropSet, ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableName[9],
				    ds_service_table[DS_SERVICE_CONNECTIONMANAGER].VariableStrVal[9]);
	
			    UpnpAcceptSubscriptionExt(media.device_handle, sr_event->UDN, sr_event->ServiceId,PropSet,sr_event->Sid);
			    ixmlDocument_free(PropSet);
			    break;
			case  DS_SERVICE_CONTENTDIRECTORY:
			    PropSet = NULL;
			    //gUpdateID=1;
			    //memset(gContainerID,0,sizeof(gContainerID)+1);
			    UpnpAddToPropertySet(&PropSet, ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableName[17], ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableStrVal[17]);
			    UpnpAddToPropertySet(&PropSet, ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableName[6], ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableStrVal[6]);
			    UpnpAcceptSubscriptionExt(media.device_handle, sr_event->UDN, sr_event->ServiceId,PropSet,sr_event->Sid);
			    ixmlDocument_free(PropSet);
			    break;

	    	}
		}
    }

    pthread_mutex_unlock(&DSDevMutex);
    return 0;
}

static int db_query_image_callback (void *pUser,	/* Pointer to the QueryResult structure */
		int nArg,	/* Number of columns in this result row */
		char **azArg,	/* Text of data in all columns */
		char **NotUsed	/* Names of the columns */
		)
{
	char **pResult = (char **) pUser;
	
	if(nArg!=4)
		return 1;
	if((azArg[0]!=NULL)&&(azArg[1]!=NULL)&&(azArg[2]!=NULL)&&(azArg[3]!=NULL)){	
		*pResult=(char *) malloc (strlen(azArg[0])+64+3*strlen(azArg[1])+2*strlen(azArg[2])+strlen(azArg[3]));
		if(!*pResult)
			return 1;	
		sprintf(*pResult,"B31,B34/%s,B35,B35/%s,B35/%s/%s,B35/%s/%s/%s",azArg[0],azArg[1],azArg[1],azArg[2],azArg[1],azArg[2],azArg[3]);
	
		strcpy(*pResult,"B31,B34");
		if(strlen(azArg[0])){
			strcat(*pResult,",B34/");
			strcat(*pResult,azArg[0]);
		}
		strcat(*pResult,",B35");
		if(strlen(azArg[1])){
			strcat(*pResult,",B35/");
			strcat(*pResult,azArg[1]);
		}			
		if(strlen(azArg[1]) && strlen(azArg[2])){
			strcat(*pResult,",B35/");
			strcat(*pResult,azArg[1]);
			strcat(*pResult,"/");
			strcat(*pResult,azArg[2]);
		}	
		if(strlen(azArg[1]) && strlen(azArg[2]) && strlen(azArg[3])){
			strcat(*pResult,",B35/");
			strcat(*pResult,azArg[1]);
			strcat(*pResult,"/");
			strcat(*pResult,azArg[2]);
			strcat(*pResult,"/");
			strcat(*pResult,azArg[3]);			
		}					

		return 0;
	}
	return 1;
}
#if 0

static int db_query_video_callback (void *pUser,	/* Pointer to the QueryResult structure */
		int nArg,	/* Number of columns in this result row */
		char **azArg,	/* Text of data in all columns */
		char **NotUsed	/* Names of the columns */
		)
{
	char **pResult = (char **) pUser;
	
	if(nArg!=4)
		return 1;
	if((azArg[0]!=NULL)&&(azArg[1]!=NULL)&&(azArg[2]!=NULL)&&(azArg[3]!=NULL)){	
		*pResult=(char *) malloc (strlen(azArg[0])+64+strlen(azArg[1])+strlen(azArg[2])+strlen(azArg[3]));
		if(!*pResult)
			return 1;		
		sprintf(*pResult,"B11,B12/%s,B15/%s/%s/%s",azArg[0],azArg[1],azArg[2],azArg[3]);
		return 0;
	}
	return 1;
}
#endif

static int db_query_audio_callback (void *pUser,	/* Pointer to the QueryResult structure */
		int nArg,	/* Number of columns in this result row */
		char **azArg,	/* Text of data in all columns */
		char **NotUsed	/* Names of the columns */
		)
{
	char **pResult = (char **) pUser;
	
	if(nArg!=3)
		return 1;
	if((azArg[0]!=NULL)&&(azArg[1]!=NULL)&&(azArg[2]!=NULL)){	
		*pResult=(char *) malloc (strlen(azArg[0])+64+strlen(azArg[1])+strlen(azArg[2]));
		if(!*pResult)
			return 1;
		strcpy(*pResult,"B21,B22");
		if(strlen(azArg[0])){
			strcat(*pResult,",B22/");
			strcat(*pResult,azArg[0]);
		}
		strcat(*pResult,",B23");	
		if(strlen(azArg[1])){
			strcat(*pResult,",B23/");
			strcat(*pResult,azArg[1]);
		}	
		strcat(*pResult,",B24");	
		if(strlen(azArg[2])){
			strcat(*pResult,",B24/");
			strcat(*pResult,azArg[2]);
		}						
		return 0;
	}
	return 1;
}
int SQLGetInfo(const char *pFileName,int type, char **ppResult)
{
	sqlite *db = NULL;
	char *zErrMsg = NULL;
	char *result_str= NULL;
	char sql[1024]={ 0 };
    char *encoded_path=NULL, *encoded_name=NULL;
	int len=0,ret=0;
	
	sqlite_open (db_file_path, &db);
	if (db == NULL)
		return SQLITE_ERROR;
	encoded_path=malloc(2*strlen(pFileName)+3);
	if(encoded_path){
		memset(encoded_path, 0, 2*strlen(pFileName)+1);
		ParseSpecialSQLChar((char *)pFileName, encoded_path);
	}
	else{
		sqlite_close (db);
		return -1;	
	}
	encoded_name=strrchr(encoded_path, '/');
	if(!encoded_name){
		sqlite_close (db);
		free(encoded_path);
		encoded_path=NULL;
		return -1;	
	}
	*encoded_name++=0;
	len=strlen(encoded_path);	
	if(type==MEDIA_TYPE_VIDEO){
		result_str=malloc(8);
		if(result_str)
			strcpy(result_str, "B11");		
		 //sprintf(sql,"select distinct Genre,BuildDateY,BuildDateM,BuildDateD from Data where ObjectPath= '%s%s' and ObjectName= '%s'",encoded_path, encoded_path[len-1]=='/'?"":"/",encoded_name);
		 //sqlite_exec (db, sql, db_query_video_callback, &result_str, &zErrMsg);
		 ret=SQLITE_OK;
	}
	else if(type==MEDIA_TYPE_MUSIC){
		sprintf(sql,"select distinct Genre,Artist,Album from Data where ObjectPath= '%s%s' and ObjectName='%s'",encoded_path, encoded_path[len-1]=='/'?"":"/",encoded_name);
		ret=sqlite_exec (db, sql, db_query_audio_callback, &result_str, &zErrMsg);
	}
	else if(type==MEDIA_TYPE_PHOTO){
		sprintf(sql,"select distinct Album,BuildDateY,BuildDateM,BuildDateD from Data where ObjectPath= '%s%s' and ObjectName= '%s'",encoded_path, encoded_path[len-1]=='/'?"":"/",encoded_name);
		ret=sqlite_exec (db, sql, db_query_image_callback, &result_str, &zErrMsg);
	}
	if(ret!=SQLITE_OK){
		if(zErrMsg){
			sqlite_free(zErrMsg);
			zErrMsg=NULL;
		}
	}
	if (result_str) {
		*ppResult=malloc(strlen(result_str)+1);
		if(!*ppResult){
			free(encoded_path);
			encoded_path=NULL;			
			sqlite_close (db);
			return -1;
		}
		memset(*ppResult, 0, strlen(result_str)+1);
		strcpy(*ppResult, result_str);		
		free(result_str);
	}
	free(encoded_path);
	encoded_path=NULL;	
	sqlite_close (db);
	return 0;
}

int FindContainer(char *filename, int file_type, char **pContainerStr)
{
	const char *extension;
	char protocol[32]={0}, *p=NULL, *pContainerPathStr=NULL, *pTemp=NULL;
	int i=0, find = 0,ret=0, len=0;
	sqlite *db = NULL;
	
	if(file_type==MEDIA_TYPE_NONE){
		extension = strrchr (filename, '.');
		if (extension != NULL) {
			for (i = 0; i < sizeof (media_protocolinfo) / sizeof (struct MediaProtocolInfo); i++) {
				if (strcasecmp (media_protocolinfo[i].ch_postfix, extension + 1) == 0) {
					sprintf (protocol, "%s", media_protocolinfo[i].ch_objectitem);
					find = 1;
					break;
				}
			}
		}
		if (!find)
			return 0;
		if(strcmp(protocol,"videoItem.movie")==0){
			file_type=MEDIA_TYPE_VIDEO;
		}
		else if(strcmp(protocol,"audioItem.musicTrack")==0){
			file_type=MEDIA_TYPE_MUSIC;
		}
		else if(strcmp(protocol,"imageItem.photo")==0){
			file_type=MEDIA_TYPE_PHOTO;
		}
	}
	SQLGetInfo(filename,file_type,pContainerStr);
	p=strrchr(filename, '/');
	if(p)
		*p=0;
	sqlite_open (db_file_path, &db);
	if (db == NULL)
		return 0;
	ret=GetContainerIDsOfFolder(db, filename, &pContainerPathStr, file_type);
	sqlite_close (db);
	if(p)
		*p='/';	
	if(!ret && pContainerPathStr){
		//printf("%s: %d, pContainerPathStr=%s\n", __FUNCTION__, __LINE__, pContainerPathStr);	
		if(*pContainerStr)
			len=strlen(*pContainerStr);
		len+=strlen(pContainerPathStr);
		pTemp=malloc(len+3);
		if(!pTemp)
			return 0;
		if(*pContainerStr){
			sprintf(pTemp, "%s,%s", *pContainerStr, pContainerPathStr);
			free(*pContainerStr);
			*pContainerStr=NULL;
		}
		else
			strcpy(pTemp, pContainerPathStr);
		*pContainerStr=pTemp;
	}
	return 0;
}

extern int media_pipe[];

#define	NOTIFYING_FLAG	"/tmp/notifying"

void NotifyQueue(char *pContainerStr)
{
	int num=0;
	
	if(!pContainerStr || !*pContainerStr)
		return;
	while(!access(NOTIFYING_FLAG, F_OK)){
		sleep(2);	
		num++;
		if(num>5)
			break;
	}
	write(media_pipe[1], pContainerStr, strlen(pContainerStr));
	fsync(media_pipe[1]);
	kill(getppid(), SIGUSR1);
	return;	
}

int GetContainerTimes(char *pContainerStr)
{
	int times=0;
	char *p=NULL,*p1=NULL;
	
	if(!pContainerStr || !strlen(pContainerStr))
		return -1;

	p=strchr(pContainerStr,',');
	while(p){
		times++;
		p1=p+1;
		p=strchr(p1,',');
	}
	times++;

	return times;
}

void AddNotify(char *buf)
{
	int i=0,len=0,fd=0,num=0;
	char *p=NULL, *p1=NULL, *pContainerStr=NULL;
	IXML_Document *PropSet=NULL;
	char *varstr=NULL, tmp_str[12]={0};
	unsigned int update_id=0;
	
	while(!access(NOTIFYING_FLAG, F_OK)){
		sleep(2);	
		num++;
		if(num>5)
			break;
	}
	fd=creat(NOTIFYING_FLAG, 0644);
	if(fd>=0)
		close(fd);
	pContainerStr=strdup(buf);
	//FindContainer(buf,file_type,&pContainerStr);
	if(!pContainerStr || !*pContainerStr){
		remove(NOTIFYING_FLAG);
		return;
	}
	len=GetContainerTimes(pContainerStr);
	if(len<0){
		remove(NOTIFYING_FLAG);
		return;
	}
	else{
		len=strlen(pContainerStr)+8*len+1;
	}
	varstr=malloc(len);
	if(!varstr){
		remove(NOTIFYING_FLAG);
		return;
	}
	memset(varstr,0,len);	
	pthread_mutex_lock(&DSDevMutex);
	//ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableStrVal[17]: SystemUpdateID
	i=atoi(ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableStrVal[17])+1;
	sprintf(ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableStrVal[17],"%d",i);
	PropSet = NULL;	
	if(pContainerStr && strlen(pContainerStr)) {
		//ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableStrVal[6]: ContainerUpdateIDs
		p1=pContainerStr;
		p=strchr(pContainerStr,',');
		while(p) {
			*p=0;
			strcat(varstr,p1);//ContainerUpdateIDs
			UpdateOneContainer(p1);
			update_id=GetUpdateIDByContainer(p1);		
			sprintf(tmp_str, ",%u,",update_id);	
			strcat(varstr,tmp_str);
			p1=p+1;
			p=strchr(p1,',');
		}
		if(p1 && strlen(p1)){
			UpdateOneContainer(p1);
			update_id=GetUpdateIDByContainer(p1);		
			sprintf(tmp_str, ",%u",update_id);	
			strcat(varstr,p1);	
			strcat(varstr,tmp_str);		
		}		
	}	
	if(pContainerStr){
		free(pContainerStr);
		pContainerStr=NULL;
	}
	UpnpAddToPropertySet(&PropSet, ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableName[17],ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableStrVal[17]);
	UpnpAddToPropertySet(&PropSet, ds_service_table[DS_SERVICE_CONTENTDIRECTORY].VariableName[6],varstr);
#if 0
	{
		FILE *ff=NULL;
		
		ff=fopen("notify.txt", "at");
		if(ff){
			fprintf(ff,"varstr=%s\n",varstr);	
			fclose(ff);
		}	
	}
#endif	
	UpnpNotifyExt(media.device_handle, ds_service_table[DS_SERVICE_CONTENTDIRECTORY].UDN,ds_service_table[DS_SERVICE_CONTENTDIRECTORY].ServiceId,PropSet);
	
	if(PropSet){
		ixmlDocument_free(PropSet);
		PropSet=NULL;
	}
	if(varstr){
		free(varstr);
		varstr=NULL;
	}
	pthread_mutex_unlock(&DSDevMutex);
	remove(NOTIFYING_FLAG);
}

#if 0
int checkContainerID(char *object_id)
{
	if(!strlen(gContainerID))
		return 0;
	else {
		char *p=NULL;
		char tempContainerID[512];
		
		strcpy(tempContainerID,gContainerID);
		p=strrchr(tempContainerID,',');
		if(p) {
			if(strcmp(object_id,p+1)==0)
				return 0;
			*p=0;
			p=strrchr(tempContainerID,',');
		}
		while(p) {
			if(strcmp(object_id,p+1)==0)
				return 0;
			*p=0;
			p=strrchr(tempContainerID,',');
		}
		if(strcmp(object_id,tempContainerID)==0)
			return 0;
		if(gUpdateID==1)
			return 0;
	}
	return 1;
}
#endif

