#ifndef _GATEWAY_H
#define _GATEWAY_H

#include "sqlite3.h"
#if 0
#define sqlite_open sqlite3_open
#define sqlite_close sqlite3_close
#define sqlite_exec sqlite3_exec
typedef sqlite3 sqlite;
#else
#include "mmsio.h"
#endif
#include <upnp/upnp.h>

#define DISALLOW	0	/* 0  not grant to change ; 1 grant */
#define DEBUG   0

#define	DEF_LAST_MOST_PLAYED_COUNT			20

/* SERVICE */
#define DS_SERVICE_SERVCOUNT1		                        2
#define DS_SERVICE_CONNECTIONMANAGER	          0  		/* Add to compatible for MS XP */
#define DS_SERVICE_CONTENTDIRECTORY                     1

/* SERVICE */
#define DS_SERVICE_SERVCOUNT                		6
#define DS_SERVICE_OSInfo                   		0           /* Add to compatible for MS XP */
#define DS_SERVICE_WANCommonInterfaceConfig 		1
#define DS_SERVICE_WANIPConnection          		2
#define DS_SERVICE_Layer3Forwarding                 3
#define DS_SERVICE_WANEthernetLinkConfig            4
#define DS_SERVICE_LANHostConfigManagement          5


/* ContentDirectory variable*/
#define DS_CONTENT_VARCOUNT  		20
/*Connect variable*/
#define DS_CONNECT_VARCOUNT			10
#define	DS_MAXVARS				19

#define     SEL_PATH			0
#define     SEL_MOVIES_ALL		1
#define     SEL_MOVIES_GENRE	2
#define     SEL_MOVIES_DATE		3
#define     SEL_MUSIC_ALL		4
#define     SEL_MUSIC_GENRE		5
#define     SEL_MUSIC_ARTIST	6
#define     SEL_MUSIC_ALBUM		7
#define     SEL_MUSIC_DATE		8
#define     SEL_MUSIC_PLAYLIST	9
#define     SEL_PHOTO_ALL		10
#define     SEL_PHOTO_ALBUM		11
#define     SEL_PHOTO_DATE		12
#define     SEL_NUL				13
#define     SEL_OBJECT			14
#define     SEL_ROOT			15
#define     SEL_MPATH			16
#define     SEL_TITLE			17
#define	    SEL_CREATOR			18
#define	    SEL_ALBUM 			19
#define     SEL_LAST_PLAYED		20
#define	    SEL_MOST_PLAYED		21
#define     SEL_MOVIES_FOLDER	22
#define     SEL_MUSIC_FOLDER	23
#define     SEL_PHOTO_FOLDER	24

#define		SEQ_AVI_PROTOCOL		2
#define		DS_MAX_VAL_LEN 50
#define		Packet_ContentDirectory_Len 1024
#define		PACKET_ITEM_LEN 1500


#define DBLEN sizeof(struct DataBaseInfo)
/*
typedef enum {
    sel_path,
    sel_movies_all,
    sel_movies_genre,
    sel_movies_date,
    sel_music_all,
    sel_music_genre,
    sel_music_artist,
    sel_music_album,
    sel_music_date,
    sel_music_playlist,
    sel_photo_all,
    sel_photo_album,
    sel_photo_date,
    sel_nul
}select_type;*/

extern char *MediaServiceType[];
//extern const struct MediaProtocolInfo media_protocolinfo[];


/* Structure for storing IGD Service identifiers and state table */
struct IGDService {
    int  VariableCount;
    char UDN[NAME_SIZE]; /* Universally Unique Device Name */
    char ServiceId[NAME_SIZE];
    char ServiceType[NAME_SIZE];
    char *VariableName[DS_MAXVARS]; 
    char *VariableStrVal[DS_MAXVARS];
};

/* Structure for storing Media catalog*/
struct MediaService{
    int n_type;
    char ch_containerid[256];
    char ch_parentid[256];
    char ch_restricted[256];
    char ch_title[256];
};

/*media protocolInfo type*/
struct MediaProtocolInfo{
	char ch_postfix[10];
	char *dlna_type;
	char ch_protocolinfo[50];
	char ch_objectitem[30];
};

struct DataBaseInfo{
    int nCountDataDb;
    char * count_sql;
    char * data_sql;
    char db_file_path[128];
};
/*Structre for storing TableVar index and value
 *   used in IGDDeviceSetServiceTableVars  
 *    2005.2.13 john qian
 * */
struct VarRec {
	unsigned int variable;
	char *value;
};

struct FilterStruct{
	int nflag;
	char strsortname[50];
};

struct ItemStruct{
	char * item_id;
	char * refID;
	char * parentID;
	char * title;
	char * protocolInfo;
	char * size;
	char * importUri;
	char * item_address;
	char * objectitem;
	char * item_year;
	char * item_month;
	char * item_day;
	char * album;
	char * genre;
	char * artist;
	char * duration;
	char * resolution;
};

typedef struct container_id{
	char *pContainer;
	unsigned int update_id;
	struct container_id *pNext;
}CONTAINER_UPDATE_ID;


extern struct IGDService ds_service_table[];
struct g_variable{
	char g_root[256];
	char g_parentid[256];
	char g_upnpclass[50];
	unsigned long g_numberreturned;
	unsigned long g_totalmatches;
	unsigned long g_count;
	unsigned int g_filterflag;
	unsigned long g_startingindex;
	unsigned long g_requestedcount;
	int g_nmallocLen;
	int select_type;
	sqlite *g_Db;
	char **temp_str;
};

/* Mutex for protecting the global state table data
   in a multi-threaded, asynchronous environment.
   All functions should lock this mutex before reading
   or writing the state table data. */
//extern pthread_mutex_t DSDevMutex;

void db_query_free(char **);
/*void db_check(const char *, const char *, char **,...);*/
void IGDDeviceShutdownHdlr(int);
int MakeSQLSentence(int,char *,char* ,char*,struct g_variable *);
int MakeCountSQLSentence(sqlite *, int, char *,char *,char*,struct g_variable *);
int MakeMetaDataSQLSentence(int,char *,char* ,char*,struct g_variable *);
int sel_from_db(sqlite *, const char * ,char ** , int,struct g_variable *);
int sel_count_from_db(sqlite *,const char*,struct g_variable *);
int sel_from_db_by_browse(sqlite *, const char *,char ** ,struct g_variable *);

//char **db_query(sqlite *, const char *, const char *, ...);

int MediaDeviceStateTableInit(char*,char*);
int makeitempacket(char *pResult,struct ItemStruct* pitem, int,struct g_variable *);
int MediaDeviceHandleActionRequest(struct Upnp_Action_Request *);
int MediaBrowseAction(struct Upnp_Action_Request *, char *, int,struct g_variable *);
int MediaSearchAction(struct Upnp_Action_Request *,struct g_variable *);
int MediaMetaDataBrowseAction(struct Upnp_Action_Request *, char *,int, int,struct g_variable *);
int MediaSQLAction(struct Upnp_Action_Request *, char *, int, int, int,struct g_variable *);
int MediaGetSearchCapabilitiesAction(struct Upnp_Action_Request *);
int MediaGetSortCapabilitiesAction(struct Upnp_Action_Request *);
int MediaGetSystemUpdateIDAction(struct Upnp_Action_Request *);
int MediaGetProtocolInfoAction(struct Upnp_Action_Request *);
int MediaGetCurrentConnectionIDsAction(struct Upnp_Action_Request *);
int MediaGetCurrentConnectionInfoAction(struct Upnp_Action_Request *);
int MediaDeviceCallbackEventHandler(Upnp_EventType, void*, void*);
int MakeObjectID(char* src,char *dst);

int MediaMetaDataParseObjectID(char *,char*,char*,char*,struct MediaService *,int);
int IGDDeviceHandleSubscriptionRequest(struct Upnp_Subscription_Request *sr_event);
char * mime_get_protocol (const char *filename, int file_type);
int  SQLGetSongLength(const char *weblocation,int *duration);
int getJpegProtocol(char *protocol,char *p);
int getPngProtocol(char *protocol,char *p);
int FindContainer(char *filename, int file_type, char **pContainerStr);	
void AddNotify(char *buf);
int checkContainerID(char *object_id);
void NotifyQueue(char *pContainerStr);
	
int GetFolderOfContainerID(sqlite *db, char *pContainerID, char *pFolder);
int GetContainerIDOfFolder(sqlite *db, char *pFolder, char *pContainerID);	
int GetSubDirEntries(char **ppResult, sqlite *db, char *pContainerID, int media_type, unsigned long start_index, unsigned long query_num, int special_agent, int browse_flag,struct g_variable *g_var);
int GetContainerTimes(char *pContainerStr);
	
#endif
