#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#define _GNU_SOURCE
#define __USE_GNU
#include <string.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <ctype.h>
#include "upnpd.h"
#include "mediaserver.h"
#include "http.h"
#include "mmsio.h"
#include "pic_scale.h"
#define PROTOCOL_TYPE_PRE_SZ  11   /* for the str length of "http-get:*:" */
#define PROTOCOL_TYPE_SUFF_SZ 2    /* for the str length of ":*" */

extern char db_file_path[];
extern playlist *g_playlist;
extern int last_played_num;
extern char cache_folder_path[];

struct web_file_t {
  char *fullpath;
  off_t pos;
  union {
    struct {
      int fd;
      struct upnp_entry_t *entry;
    } local;
  } detail;
};

static int db_query_callback(void *pUser, int nArg, char **azArg, char **NotUsed)
{
    int len=0;
    
    if(nArg==4) {
	    len = 15 + strlen(azArg[0]) + strlen(azArg[1])+strlen(azArg[2])+strlen(azArg[3]);
	    if ((*(char **)pUser = malloc(len)) == NULL) {
		    printf("%d:malloc failed.\n", __LINE__);
		    return -1;
	    }
	    if(strcmp(azArg[2],UNKNOWN_STR)!=0)
		    sprintf(*(char **)pUser, "%s/%s|%s|%s", azArg[0], azArg[1],azArg[2],azArg[3]);
	    else
		    sprintf(*(char **)pUser, "%s/%s|%s", azArg[0], azArg[1],azArg[3]);
    }
    else {
	    len = 15 + strlen(azArg[0]) + strlen(azArg[1])+strlen(azArg[2]);
	    if ((*(char **)pUser = malloc(len)) == NULL) {
		    printf("%d:malloc failed.\n", __LINE__);
		    return -1;
	    }
	    if(strcmp(azArg[2],UNKNOWN_STR)!=0)
		    sprintf(*(char **)pUser, "%s/%s|%s", azArg[0], azArg[1],azArg[2]);
	    else
		    sprintf(*(char **)pUser, "%s/%s", azArg[0], azArg[1]);
    }
    
    //printf("%s:fullpath: %s\n", __FUNCTION__, *pUser);
    return 0;
}
int UpdateFileTimes(const char *filename,unsigned long long times)
{
	sqlite *db;
	int ret = 0;
	char sql[512] = { 0 };
	char *zErrMsg = NULL;
	if (access(db_file_path, F_OK) != 0)
		return -1;
	chmod(db_file_path,0644);
	sqlite_open(db_file_path, &db);
	if (db == NULL) {
		return -1;
	}
#define SQL_UT "UPDATE Data SET PlayTimes= '%llu' where WebLocation = '%s'"
	sprintf(sql, SQL_UT, times+1,filename);
	//printf("sql===%s\n",sql);
	ret = sqlite_exec(db, sql, 0,0, &zErrMsg);
	if (SQLITE_OK != ret||zErrMsg!=NULL) {
		if(zErrMsg!=NULL){
			sqlite3_free(zErrMsg);
			sqlite_close(db);
			chmod(db_file_path,0444);
			return -1;
		}
	}
	sqlite_close(db);
	chmod(db_file_path,0444);
	del_node(g_playlist,filename);
	add_node(g_playlist,filename);
	//print_list(g_playlist);
	return 0;

}

char *upnp_get_entry(const char *filename,int changetimes)
{
    char sql[1024] = { 0 };
    char *sqlresult = NULL;
    sqlite *db=NULL;
    int ret = 0;
    char *zErrMsg = NULL;
    char *p=NULL;
    int needcharge=0;
    char resolution[64]={0}; 
    
    sqlite_open(db_file_path, &db);
    if (db == NULL) {
		return NULL;
    }
#define SQL_1 "SELECT DISTINCT ObjectPath,ObjectName,Resolution,PlayTimes from Data where WebLocation = '%s'"
#define SQL_2 "SELECT DISTINCT ObjectPath,ObjectName,Resolution from Data where WebLocation = '%s'"    

    if((p=strstr(filename,VIRTUAL_DIR""VIRTUAL_PIC))==filename){
	    char picurl[512]={0};
	    
	    p=(char *)filename+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_PIC);
		sprintf(picurl, VIRTUAL_DIR"%s", p);
	    sprintf(sql, SQL_2, picurl);
    }
    else if((p=strstr(filename,VIRTUAL_DIR""VIRTUAL_LM))==filename){
	    char picurl[512]={0};
	    p=(char *)filename+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_LM);
	    sprintf(picurl, VIRTUAL_DIR"%s", p);
	    sprintf(sql, SQL_2, picurl);
	    needcharge=1;	    
    }
    else if((p=strstr(filename,VIRTUAL_DIR""VIRTUAL_PNG))==filename){
	    char picurl[512]={0};
	    
	    p=(char *)filename+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_PNG);
		sprintf(picurl, VIRTUAL_DIR"%s", p);
	    sprintf(sql, SQL_2, picurl);
    }
    else {
	    if(changetimes==1)
			sprintf(sql, SQL_1, filename);
	    else
	    	sprintf(sql, SQL_2, filename);
    }
    ret = sqlite_exec(db, sql, db_query_callback, &sqlresult, &zErrMsg);
    if (ret == SQLITE_SCHEMA) {
		if (zErrMsg)
			sqlite3_free(zErrMsg);
		ret = sqlite_exec(db, sql, db_query_callback, &sqlresult, &zErrMsg);
    }
    if (zErrMsg || !sqlresult) {
    	if(sqlresult){
    		free(sqlresult);
    		sqlresult=NULL;
    	}
    	if(zErrMsg){
		sqlite3_free(zErrMsg);
			zErrMsg=NULL;
		}
		sqlite_close(db);
		return NULL;
    }
	sqlite_close(db);
	if(changetimes==1) {
		unsigned long long times=0;
		p=strrchr(sqlresult,'|');
		if(p) {
			times=strtoull(p+1,NULL,10);
			*p=0;
		}
		UpdateFileTimes(filename,times);
	}
	p=strrchr(sqlresult,'|');
	if(p){
		strcpy(resolution,p+1);
		*p=0;
	}
	if(needcharge==1 && sqlresult!=NULL){
		char *pThumb=NULL;
		
		p=strrchr(filename,'/');
		if(p)
			p++;
		else
			p=(char *)filename;
		pThumb=(char*)malloc(strlen(p)+strlen(cache_folder_path)+2);
		if(!pThumb){
			free(sqlresult);
			sqlresult=NULL;
			return NULL;
		}
		sprintf(pThumb, "%s/%s", cache_folder_path, p);
		if(access(pThumb, F_OK) && CreateThumb(sqlresult,pThumb,resolution)!=0){
			free(pThumb);
			free(sqlresult);
			sqlresult=NULL;			
			return sqlresult;
		}
		if(sqlresult){
			free(sqlresult);
			sqlresult=NULL;
		}
		return pThumb;
	}
    return sqlresult;
}

int GetFileProtocolInfo(const char *filename,char *pProtocolInfo)
{
    char sql[1024] = { 0 };
    char *sqlresult = NULL;
    sqlite *db=NULL;
    int ret = 0;
    char *zErrMsg = NULL;
    char *p=NULL;
    
    sqlite_open(db_file_path, &db);
    if (db == NULL) {
		return -1;
    }
    //printf("filename=%s\n",filename);
#define SQL_COMMAND_PROTOCOL "SELECT DISTINCT ObjectPath,ObjectName,ProtocolInfo from Data where WebLocation = '%s'"
    if((p=strstr(filename,VIRTUAL_DIR""VIRTUAL_PIC))==filename){
	    char picurl[512];
	    
	    p=(char *)filename+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_PIC);
		sprintf(picurl, VIRTUAL_DIR"%s", p);
    	sprintf(sql, SQL_COMMAND_PROTOCOL, picurl);
    }
    else if((p=strstr(filename,VIRTUAL_DIR""VIRTUAL_LM))==filename){
	    char picurl[512];

	    p=(char *)filename+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_LM);
	    sprintf(picurl, VIRTUAL_DIR"%s", p);
	    sprintf(sql, SQL_COMMAND_PROTOCOL, picurl);
    }
    else if((p=strstr(filename,VIRTUAL_DIR""VIRTUAL_PNG))==filename){
	    char picurl[512];
	    
	    p=(char *)filename+strlen(VIRTUAL_DIR)+strlen(VIRTUAL_PNG);
		sprintf(picurl, VIRTUAL_DIR"%s", p);
    	sprintf(sql, SQL_COMMAND_PROTOCOL, picurl);
    }
    else
    	sprintf(sql, SQL_COMMAND_PROTOCOL, filename);
    ret = sqlite_exec(db, sql, db_query_callback, &sqlresult, &zErrMsg);
    if (ret == SQLITE_SCHEMA) {
		if (zErrMsg)
			sqlite3_free(zErrMsg);
		ret = sqlite_exec(db, sql, db_query_callback, &sqlresult, &zErrMsg);
    }
    if (zErrMsg) {
	    sqlite3_free(zErrMsg);
	    sqlite_close(db);
	    return -1;
    }
    sqlite_close(db);
   //printf("%s: %d\n",__FUNCTION__, __LINE__);
    if(strstr(filename,VIRTUAL_PIC)!=0){
	    strcpy(pProtocolInfo,"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM;DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=00F00000000000000000000000000000");

    }
    else if(strstr(filename,VIRTUAL_LM)!=0)  {
	    strcpy(pProtocolInfo,"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM;DLNA.ORG_OP=01;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=00F00000000000000000000000000000");
    }
    else if(strstr(filename,VIRTUAL_PNG)!=0) {
	    strcpy(pProtocolInfo,"http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN;DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=00F00000000000000000000000000000");
    }
    else if(sqlresult){
	    p=strrchr(sqlresult,'|');
	    if(p!=NULL) {
		    if(pProtocolInfo!=NULL)
			    strcpy(pProtocolInfo,p+1);
		    *p=0;
	    }
    }
    if(sqlresult){
    	free(sqlresult);
    	sqlresult=NULL;
    }
    return 0;
}
#ifdef _SRT_SUPPORT_
static int db_query_subtitle_callback (void *pUser,	/* Pointer to the QueryResult structure */
		int nArg,	/* Number of columns in this result row */
		char **azArg,	/* Text of data in all columns */
		char **NotUsed	/* Names of the columns */
		)
{
	char **pResult = (char **) pUser;
	if(nArg!=1)
		return 1;
	if((strcasecmp(NotUsed[0], "WebLocation") == 0)&&(azArg[0]!=NULL)) {	
		*pResult=(char *) malloc (strlen(azArg[0])+1);
		sprintf(*pResult,"%s",azArg[0]);
		return 0;
	}
	return 1;
}
int get_subtitle_video(char *filename,char *ext)
{
	sqlite *db = NULL;
	char *zErrMsg = NULL;
	char *result_str= NULL,*p;
	char sql[1024]={ 0 };
	
	if (access (db_file_path, F_OK) == 0)
		sqlite_open (db_file_path, &db);
	if (db == NULL)
		return SQLITE_ERROR;
	sprintf(sql,"select distinct WebLocation from Data where WebLocation like '%s%%'",filename);
	sqlite_exec (db, sql, db_query_subtitle_callback, &result_str, &zErrMsg);
	if(zErrMsg)	{
		sqlite3_free(zErrMsg);	
		if(result_str)
			free(result_str);
		return -1;

	}
	if (result_str) {
		p=strrchr(result_str,'.');
		if(!p){
			free(result_str);
			result_str=NULL;
			return -1;
		}
		strcpy(ext,p);
		free(result_str);
		result_str=NULL;
	}
	sqlite_close (db);
	return 0;
}
#endif

static int http_get_info (const char *filename, struct File_Info *info)
{
	struct stat st;
	char *content_type = NULL, *p=NULL;
	char protocol[256]={0};
	char *fullpath=NULL;
#ifdef _SRT_SUPPORT_
	char tmp_str[64]={0};
	int len=0,i=0;
#endif	
	if (!filename || !info)
		return -1;
#ifdef _SRT_SUPPORT_
	p=strrchr(filename, '.');
	if(!p)
		return -1;
	len=strlen(p);
	if(len > sizeof(tmp_str)-1)
		return -1;
	for(i=0;i<len;i++)
		tmp_str[i]=tolower(p[i]);
	tmp_str[i]=0;	
	if(strstr(SUPPORTED_SUBTITLE_FORMAT,tmp_str)) {
		char *aviname=NULL,ext[64]={0};
		
		aviname=(char *)malloc(strlen(filename)+1+3);
		if(aviname==NULL)
			return -1;
		memset(aviname,0,sizeof(aviname));
		strcpy(aviname,filename);
		p=strstr(aviname,tmp_str);	
		if(!p){
			free(aviname);
			aviname=NULL;
			return -1;
		}
		*p=0;
		if(get_subtitle_video(aviname,ext)!=0){
			free(aviname);
			aviname=NULL;
			return -1;
		}
		strcat(aviname,ext);
		p=mime_get_protocol(filename,MEDIA_TYPE_SUBTITLE);
		if(!p){
			free(aviname);
			aviname=NULL;
			return -1;
		}
		strcpy(protocol,p);
		fullpath = upnp_get_entry(aviname,0);
		if(fullpath) {
			p=strstr(fullpath,ext);
			if(!p){
				free(aviname);
				aviname=NULL;
				free(fullpath);
				fullpath=NULL;
				return -1;
			}
			*p=0;
			strcat(fullpath,tmp_str);
		}
		if(aviname) {
			free (aviname);
			aviname=NULL;
		}
	}
	else {
#endif	
		GetFileProtocolInfo(filename, protocol);
		fullpath = upnp_get_entry(filename,0);
#ifdef _SRT_SUPPORT_
	}	
#endif
	if(!fullpath)
		return -1;
	info->content_type = NULL;
	if (stat (fullpath, &st) < 0){
		free(fullpath);
		return -1;
	}
	if (access (fullpath, R_OK) < 0) {
    	if (errno != EACCES){
			free(fullpath);
			return -1;
    	}
		info->is_readable = 0;
	}
	else
    	info->is_readable = 1;
	/* file exist and can be read */
	info->file_length = st.st_size;
	info->last_modified = st.st_mtime;
	info->is_directory = S_ISDIR (st.st_mode);
	content_type = strdup (protocol + PROTOCOL_TYPE_PRE_SZ);
	if (content_type) {
		if(strstr(content_type, "DLNA.ORG_PN=AVI") || !strstr(content_type, "DLNA.ORG_PN")){
			p=strrchr(content_type, ':');
			if(p)
				*p=0;
		}
//		printf("content_type=%s\n",content_type);
    	info->content_type = ixmlCloneDOMString (content_type);
		free (content_type);
	}
	else
    	info->content_type = ixmlCloneDOMString ("");
	free(fullpath);
#ifdef _SRT_SUPPORT_
	if(strstr(SUPPORTED_SUBTITLE_FORMAT,tmp_str)!=NULL)
		info->duration=0;
	else
#endif
	SQLGetSongLength(filename,&info->duration);
//	printf("%s: %d,%s\n",__FUNCTION__, __LINE__,info->content_type);
#if 0 
	printf("%s:%d:readable=%d, length=%llu, directory=%d, last_mod=%s\b\tcontent_type=%s filename=%s,len==%d\n",
	  __FILE__, __LINE__, info->is_readable, info->file_length, info->is_directory,
	  asctime(gmtime(&info->last_modified)), info->content_type,filename,info->duration);
#endif
  return 0;
}

static UpnpWebFileHandle http_open (char *filename, enum UpnpOpenFileMode mode)
{
	struct web_file_t *pFileHandle = NULL;
	int fd,len=0,flag=0;
	char *fullpath=NULL;
	char *ext=NULL;
	
	if (!filename)
		return NULL;
	len=strlen(filename);
	if(filename[len-1]=='|'){
		filename[len-1]=0;
		flag=1;
	}
	ext = strrchr(filename,'.');
	if(ext==NULL)
		return NULL;

	if (mode != UPNP_READ)
		return NULL;
	if(flag && strcasestr(SUPPORTED_MUSIC_FORMAT,ext+1))	
		fullpath = upnp_get_entry(filename,1);
	else
#ifdef _SRT_SUPPORT_
	{
		if(strcasestr(SUPPORTED_SUBTITLE_FORMAT,ext)) {
			char *aviname=NULL,*p=NULL,temp[64]={0};
			
			aviname=(char *)malloc(strlen(filename)+1+3);
			if(aviname==NULL)
				return NULL;
			memset(aviname,0,sizeof(aviname));
			strcpy(aviname,filename);
			p=strstr(aviname,ext);	
			if(p)
				*p=0;
			if(get_subtitle_video(aviname,temp)!=0){
				free(aviname);
				aviname=NULL;
				return NULL;
			}
			strcat(aviname,temp);
			fullpath = upnp_get_entry(aviname,0);
			if(fullpath) {
				p=strstr(fullpath,temp);
				if(p)
					*p=0;
				strcat(fullpath,ext);
			}
			if(aviname) {
				free(aviname);
				aviname=NULL;
			}
		}
		else {
#endif
			fullpath = upnp_get_entry(filename,0);
#ifdef _SRT_SUPPORT_
		}
	}
#endif
	if(!fullpath) {
		free(fullpath);
		return NULL;
	}
	//printf("fullpath: %s\n", fullpath);
	fd = open (fullpath, O_RDONLY | O_NONBLOCK | O_SYNC | O_NDELAY);
	if (fd < 0) {
		free(fullpath);
		return NULL;
	}
	pFileHandle = malloc (sizeof (struct web_file_t));
	pFileHandle->fullpath = strdup (fullpath);
	pFileHandle->pos = 0;
	pFileHandle->detail.local.fd = fd;
	free(fullpath);
	return ((UpnpWebFileHandle) pFileHandle);
}

static int http_read (UpnpWebFileHandle fh, char *buf, size_t buflen)
{
	struct web_file_t *file = (struct web_file_t *) fh;
	ssize_t len = -1;

	if (!file)
		return -1;

	len = read (file->detail.local.fd, buf, buflen);
	//printf("%s: %d\n",__FUNCTION__, __LINE__);printf("Read %zd bytes.\n", len);
	if (len >= 0)
		file->pos += len;

	return len;
}

static int http_write (UpnpWebFileHandle fh __attribute__((unused)),
            char *buf __attribute__((unused)),
            size_t buflen __attribute__((unused)))
{
//  printf ("http write\n");
  return 0;
}

static int http_seek (UpnpWebFileHandle fh, off_t offset, int origin)
{
	struct web_file_t *file = (struct web_file_t *) fh;
	off_t newpos = -1;
	struct stat sb;

	//printf ("http_seek\n");
	if (!file)
		return -1;

	switch (origin) {
		case SEEK_SET:
			newpos = offset;
			break;
		case SEEK_CUR:
			newpos = file->pos + offset;
			break;
		case SEEK_END:
			if (stat (file->fullpath, &sb) < 0) {
				printf ("%s: cannot stat: %s\n", file->fullpath, strerror (errno));
				return -1;
    		}
    		newpos = sb.st_size + offset;
  	}

    /* Just make sure we cannot seek before start of file. */
    if (newpos < 0) {
		printf ("%s: cannot seek: %s\n", file->fullpath, strerror (EINVAL));
		return -1;
    }

    /* Don't seek with origin as specified above, as file may have
       changed in size since our last stat. */
    if (lseek (file->detail.local.fd, newpos, SEEK_SET) == -1) {
		printf ("%s: cannot seek: %s\n", file->fullpath, strerror (errno));
		return -1;
    }

	file->pos = newpos;

	return 0;
}

static int
http_close (UpnpWebFileHandle fh)
{
	struct web_file_t *file = (struct web_file_t *) fh;

	//printf ("http_close\n");
	if (!file)
		return -1;
	close (file->detail.local.fd);
	if (file->fullpath)
		free (file->fullpath);
	free (file);
	return 0;
}

struct UpnpVirtualDirCallbacks virtual_dir_callbacks =
  {
    http_get_info,
    http_open,
    http_read,
    http_write,
    http_seek,
    http_close
  };
  
void list_destroy( playlist *l )
{
	if(l->size!=0) {
		node *p=l->head;
		for(;l->size>0&&p->next!=NULL;l->size--) {
			l->head=p->next;
			if(p)free(p);
			p=l->head;
		}
		l->head =l->tail = NULL;
		l->size = 0;
		if(p)
			free( p);
		if(l)
			free(l);
	}
	else {
		l->head=l->tail=NULL;
		l->size=0;
		if(l)free(l);
	}
}
playlist* list_init( playlist *l)
{
	l=( playlist*)malloc(sizeof( playlist));
	if(!l) {	
		printf("error malloc!\n");
		return NULL;
	}
	l->head=l->tail=NULL;
	l->size=0;
	return(l);
}
int length(playlist *l)  
{   
	return(l->size);
}

void add_node(playlist *l,const char *filename)
{
	node *p=NULL;
	
	p=( node*)malloc(sizeof(node));
	if(!p) {
		printf("error malloc!\n");
		return;
	}
	p->name=(char*)malloc((strlen(filename)+1)*sizeof(char));
	if(!p->name)
	{
		printf("error malloc!\n");
		return;
	}
	strcpy(p->name,filename);
	p->next=l->head;
	l->head=p;
	if(l->size==0)
		l->tail=l->head;
	l->size++;
	if(length(l)>last_played_num)
		del_last(l);
		
}
void print_list(playlist *l)
{
	node *p=l->head;
	
	while(p!=NULL) {
	       	printf("%s\n",p->name);
		p=p->next;
	} 
}
void del_last(playlist *l)
{
	node *p=l->head;
	
	if(l->size==0)
		return;
	if(l->size==1) {
		if(p->name)
			free(p->name);
		if(p)
			free(p);
		l->head=l->tail=NULL;
		l->size--;
		return;
	}
	while( p->next != l->tail)
		p = p->next;
	if(l->tail->name)
		free(l->tail->name);
	if(l->tail)
		free(l->tail);
	p->next=NULL;
	l->tail = p;
	l->size--;
}

int del_node(playlist *l,const char *filename)
{
	node *p=l->head;
	
	if(l->size==0)
		return 1;
	if(l->size==1) {
		if(strcmp(p->name,filename)==0) {
			if(p->name)
				free(p->name);
			free(p);
			l->size--;
			l->head=l->tail=NULL;
			return 0;
		}
		else
			return 1;
	}
	if(strcmp(p->name,filename)==0) {
		p=p->next;
		if(l->head->name)
			free(l->head->name);
		if(l->head)
			free(l->head);
		l->head=p;
		l->size--;
		return 0;
	}
	while(p->next) {
		if(strcmp(p->next->name,filename)==0) {
			if(p->next==l->tail) {
				del_last(l);
				return 0;
			}
			else {
				node *q=p->next;
				
				p->next=p->next->next;
				if(q->name)
					free(q->name);
				if(q)
					free(q);
				l->size--;
				return 0;
			}
		}
		p=p->next;
	}
	
	return 1;
}
	
