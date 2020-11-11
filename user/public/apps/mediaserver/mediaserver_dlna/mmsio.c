#include <stdio.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/version.h>
#include <errno.h>
#define _GNU_SOURCE
#define __USE_GNU
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/vfs.h>
#include <stdarg.h>
#include <iconv.h>
#include <getopt.h>
#include <signal.h>
#include <jpeglib.h>
#include <setjmp.h>
//#include <gif_lib.h>
#include <png.h>

#define	__LINUX_INOTIFY__

#define	__NAS_THUMBNAIL_FUN__

#ifdef __NAS_THUMBNAIL_FUN__
#define	THUMB_FOLDER	"/.Thumb_DB/"
#define	THUMB_FOLDER_NAME	".Thumb_DB"
#endif
#include "libdlna.h"	//From libdlna

#include "upnpd.h"
#include "mmsio.h"
#include "tool.h"
#include "mediaserver.h"
#include "./libinotifytools/inotifytools/inotifytools.h"
#include "./libinotifytools/inotifytools/inotify.h"
#define nasprintf(...) niceassert( -1 != asprintf(__VA_ARGS__), "out of memory")
#include "pic_scale.h"
extern char cache_folder_path[];
char media_conf_file[128]={0};

#ifdef _EARLY_ADD_
static int early_add = 0;
#endif

void StripSpace (char *pStr)
{
	int len = 0, i = 0;
	char *p;

	if (!pStr || !pStr[0])
		return;
	p = pStr;
	while (*p == ' ')
		p++;
	memmove (pStr, p, strlen (p));
	len = strlen (pStr);
	len--;
	i = len;
	while (pStr[i] == ' ')
		i--;
	if (i != len)
		pStr[i + 1] = '\0';
	return;
}

void ChangeSpecialChar (char *pStr)
{
	int len = 0, i = 0;
	char *p=NULL;
	
	if (!pStr || !pStr[0])
		return;
	len=strlen(pStr);
	while(i<len){
		if(pStr[i] == '<' || pStr[i] == '>')
			pStr[i]='_';
		i++;
	}
	while((p=strstr(pStr, "&lt;")) || (p=strstr(pStr, "&gt;"))){
		*p++=' ';
		*p++=' ';
		*p++=' ';
		*p=' ';	
	}
	
	return;
}

int CheckFileMediaType(char *pSuffix)
{
	char tmp_str[64]={0}, *p=NULL;
	int len=0,i=0;

	if(!pSuffix)
		return MEDIA_TYPE_NONE;
	p=strrchr(pSuffix, '.');
	if(!p)
		return MEDIA_TYPE_NONE;
	len=strlen(p);
	if(len > sizeof(tmp_str)-1)
		return MEDIA_TYPE_NONE;
	for(i=0;i<len;i++)
		tmp_str[i]=tolower(p[i]);
	tmp_str[i]='.';	
	tmp_str[i+1]=0;	
	if(strstr(SUPPORTED_PHOTO_FORMAT, tmp_str))
		return MEDIA_TYPE_PHOTO;
	else if(strstr(SUPPORTED_MUSIC_FORMAT, tmp_str))
		return MEDIA_TYPE_MUSIC;
	else if(strstr(SUPPORTED_VIDEO_FORMAT, tmp_str))
		return MEDIA_TYPE_VIDEO;
	else if(strstr(SUPPORTED_PLAYLIST_FORMAT, tmp_str))
		return MEDIA_TYPE_PLAYLIST;		
#ifdef _SRT_SUPPORT_
	else if(strstr(SUPPORTED_SUBTITLE_FORMAT, tmp_str))
		return MEDIA_TYPE_SUBTITLE;
#endif
	return MEDIA_TYPE_NONE;
}

struct my_error_mgr {                                               
	struct jpeg_error_mgr pub;  /* "public" fields      */          
	jmp_buf setjmp_buffer;      /* for return to caller */          
};                                                                  
typedef struct my_error_mgr * my_error_ptr;                         


int scan_sample_table[3][4] = {                             
	{ 44100, 48000, 32000, 0 },  /* MPEG 1 */               
	{ 22050, 24000, 16000, 0 },  /* MPEG 2 */               
	{ 11025, 12000, 8000, 0 }    /* MPEG 2.5 */             
};                                                          

int scan_br_table[5][16] = {                                                                   
	{ 0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0 }, /* MPEG1, Layer 1 */         
	{ 0,32,48,56,64,80,96,112,128,160,192,224,256,320,384,0 },    /* MPEG1, Layer 2 */         
	{ 0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0 },     /* MPEG1, Layer 3 */         
	{ 0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0 },    /* MPEG2/2.5, Layer 1 */     
	{ 0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0 }          /* MPEG2/2.5, Layer 2/3 */   
};                                                                                             

typedef struct tag_scan_frameinfo {                     
	int layer;               
	int bitrate;            
	int samplerate;        
	int stereo;         
	int frame_length;   
	int crc_protected;     
	int samples_per_frame;
	int padding;         
	int xing_offset;     
	int number_of_frames;    
	int frame_offset;       
	double version;         
	int is_valid;                                       
} SCAN_FRAMEINFO;                                       


typedef struct tag_scan_id3header {                     
	unsigned char id[3];                                
	unsigned char version[2];                           
	unsigned char flags;                                
	unsigned char size[4];                              
} __attribute((packed)) SCAN_ID3HEADER;                 


typedef struct create_event_list{
	char *pFileName;
	struct create_event_list *pNext;
}CREATE_EVENT_LIST;

CREATE_EVENT_LIST *pEventCreateList=NULL;
unsigned long long container_id=0;
#define	CONTAINER_ID_PREFIX	"D"
#define SQL_QUERY_FILE_BY_PATH_COMMAND "SELECT DISTINCT ObjectPath,WebLocation from Data where ObjectPath like '%s%%' and Title != 'Folder'"
#define SQL_QUERY_FOLDER_BY_PATH_COMMAND "SELECT DISTINCT ObjectPath,ContainerID from Data where ObjectPath like '%s%%' and Title = 'Folder'"

void ParseSpecialSQLChar(char *src, char *dest)
{
	int i=0,j=0,len=0;
	
	if(!strlen(src))
		return;
	len=strlen(src);
	dest[0]=0;
	while(i<len){
		if(src[i]=='\''){
			dest[j++]='\'';
			dest[j++]='\'';
		}
		else
			dest[j++]=src[i];
		dest[j]='\0';
		i++;
	}
	
	dest[j]='\0';
}

static int db_query_folder_and_file_entry_callback(void *pUser, int nArg, char **azArg, char **NotUsed)
{
	FILE *fp=NULL;
	
    //printf("Cache File: %s\n",*(char **)pUser);
	if(nArg==2){ //Query Folders
		fp=fopen(*(char **)pUser, "at");
		if(fp){
			fprintf(fp,"%s*%s\n", azArg[0], azArg[1]);
			fclose(fp);
		}
	}

    return 0;
}

static int UpdateFolderPrefixInDB(sqlite *db, char *pOldFolder, char *pNewFolder)
{
    char sql_cmd[1024] = { 0 };
    char *pFileCache = NULL, *pFolderCache=NULL;
    char *zErrMsg = NULL;
    char *pOldEncodedPath=NULL, *pOldEncodedName=NULL;
    char *pNewEncodedPath, *pNewEncodedName=NULL;    
	int len=0,ret=0,fd=0;
	FILE *fp=NULL;
	char buf[1024]={0}, *p=NULL, *pTempStr=NULL;

	pNewEncodedPath=malloc(2*strlen(pNewFolder)+3);
	if(pNewEncodedPath){
		memset(pNewEncodedPath, 0, 2*strlen(pNewFolder)+1);
		ParseSpecialSQLChar(pNewFolder, pNewEncodedPath);
	}
	else
		pNewEncodedPath=pNewFolder;	
		
	pOldEncodedPath=malloc(2*strlen(pOldFolder)+3);
	if(pOldEncodedPath){
		memset(pOldEncodedPath, 0, 2*strlen(pOldFolder)+1);
		ParseSpecialSQLChar(pOldFolder, pOldEncodedPath);
	}
	else
		pOldEncodedPath=pOldFolder;
	len=strlen(pOldEncodedPath);
	if(pOldEncodedPath[len-1]!='/')	
		strcat(pOldEncodedPath, "/");		
		
	pFileCache=malloc(256);
	if(!pFileCache){
		goto err2;
	}
	pFolderCache=malloc(256);
	if(!pFolderCache){
		free(pFileCache);
		goto err2;	
	}	
	sprintf(pFileCache, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
	fd=mkstemp(pFileCache);
	if(fd==-1){
		free(pFileCache);
		free(pFolderCache);
		goto err2;
	}
	close(fd);	
	//Query relative media files under current folder.
	sprintf(sql_cmd, SQL_QUERY_FILE_BY_PATH_COMMAND, pOldEncodedPath);
    ret = sqlite_exec(db, sql_cmd, db_query_folder_and_file_entry_callback, &pFileCache, &zErrMsg);
    if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
		remove(pFileCache);
		free(pFileCache);
		free(pFolderCache);
		goto err2;
    }
	sprintf(pFolderCache, "%s/%s", cache_folder_path, DB_CACHE_FILE_TMPLATE);
	fd=mkstemp(pFolderCache);
	if(fd==-1){
		remove(pFileCache);
		free(pFileCache);
		free(pFolderCache);
		goto err2;
	}
	close(fd);    
	//Query folders under current folder.
	sprintf(sql_cmd, SQL_QUERY_FOLDER_BY_PATH_COMMAND, pOldEncodedPath);
    ret = sqlite_exec(db, sql_cmd, db_query_folder_and_file_entry_callback, &pFolderCache, &zErrMsg);
    if (ret != SQLITE_OK) {
		if (zErrMsg)
			sqlite_free(zErrMsg);
		goto err1;
    }    

    //Update the entries
#define SQL_UPDATE_FILE_BY_WEBLOCATION "UPDATE Data SET ObjectPath= '%s%s' where WebLocation = '%s'"    
#define SQL_UPDATE_FOLDER_BY_CONTAINERID "UPDATE Data SET ObjectPath= '%s%s' where ContainerID = '%s'"    
#define SQL_UPDATE_FILE_BY_NAME "UPDATE Data SET ObjectName='%s' where ObjectPath= '%s%s' and ObjectName = '%s'"    
  
	fp=fopen(pFileCache, "rt");
	if(!fp){
		goto err1;
	}
	while(fgets(buf, sizeof(buf)-1,fp)){
		p=strrchr(buf, '\n');
		if(p)
			*p=0;
		p=strrchr(buf, '*');
		if(p){
			*p=0;
			p++;
		}
		else
			continue;
		pTempStr=malloc(2*strlen(buf)+3);
		if(pTempStr){
			memset(pTempStr, 0, 2*strlen(buf)+1);
			ParseSpecialSQLChar(buf+strlen(pOldFolder), pTempStr);		
		}	
		else
			pTempStr=buf+strlen(pOldFolder);			
		sprintf(sql_cmd, SQL_UPDATE_FILE_BY_WEBLOCATION, pNewEncodedPath, pTempStr,p);//FIXME: Need encode the folder names.
	    ret = sqlite_exec(db, sql_cmd, NULL, NULL, &zErrMsg);
	    if(pTempStr!=buf+strlen(pOldFolder)){
	    	free(pTempStr);
	    	pTempStr=NULL;
	    }		    	
	    if (ret != SQLITE_OK) {
			if (zErrMsg)
				sqlite_free(zErrMsg);
			fclose(fp); 
			goto err1;
	    }   	
	}
	fclose(fp);    
 	fp=fopen(pFolderCache, "rt");
	if(!fp){
		goto err1;
	}
	while(fgets(buf, sizeof(buf)-1,fp)){
		p=strrchr(buf, '\n');
		if(p)
			*p=0;		
		p=strrchr(buf, '*');	
		if(p){
			*p=0;
			p++;
		}
		else
			continue;
		pTempStr=malloc(2*strlen(buf)+3);
		if(pTempStr){
			memset(pTempStr, 0, 2*strlen(buf)+1);
			ParseSpecialSQLChar(buf+strlen(pOldFolder), pTempStr);		
		}	
		else
			pTempStr=buf+strlen(pOldFolder);
		sprintf(sql_cmd, SQL_UPDATE_FOLDER_BY_CONTAINERID, pNewEncodedPath, pTempStr,p);
	    ret = sqlite_exec(db, sql_cmd, NULL, NULL, &zErrMsg);
	    if(pTempStr!=buf+strlen(pOldFolder)){
	    	free(pTempStr);
	    	pTempStr=NULL;
	    }	
	    if (ret != SQLITE_OK) {
			if (zErrMsg)
				sqlite_free(zErrMsg);
			fclose(fp);
			goto err1;	
	    }   

	}
	fclose(fp);    
    //Update End
    remove(pFileCache);
    remove(pFolderCache);
	free(pFileCache);
	free(pFolderCache);

	len=strlen(pOldEncodedPath);
	if(pOldEncodedPath[len-1]=='/')	
		pOldEncodedPath[len-1]=0;
	if(pNewEncodedPath[len-1]=='/')	
		pNewEncodedPath[len-1]=0;		
	pNewEncodedName=strrchr(pNewEncodedPath, '/');	
	p=NULL;
	if(pNewEncodedName){
		pNewEncodedName++;
		p=strrchr(pNewEncodedName, '/');
		if(p)
			*p=0;
		else
			p=pNewEncodedName;
	}
    pOldEncodedName=strrchr(pOldEncodedPath, '/');
    if(pOldEncodedName){
    	*pOldEncodedName++=0;
    }
    len=strlen(pOldEncodedPath);
    if(pOldEncodedName && pNewEncodedName && p){
		//Modify the folder self.
		sprintf(sql_cmd, SQL_UPDATE_FILE_BY_NAME, p, pOldEncodedPath, pOldEncodedPath[len-1]=='/'?"":"/",pOldEncodedName);
	    ret = sqlite_exec(db, sql_cmd, NULL,NULL, &zErrMsg);
	    if (ret != SQLITE_OK) {
			if (zErrMsg)
				sqlite_free(zErrMsg);
			goto err2;
	    }    
    }
    
    if(pOldEncodedPath && pOldEncodedPath!=pOldFolder){
    	free(pOldEncodedPath);
    	pOldEncodedPath=NULL;
    }
    if(pNewEncodedPath && pNewEncodedPath!=pNewFolder){
    	free(pNewEncodedPath);
    	pNewEncodedPath=NULL;
    }            
    
	return 0;
err1:
	remove(pFileCache);
	remove(pFolderCache);
	free(pFileCache);
	free(pFolderCache);
err2:
    if(pOldEncodedPath && pOldEncodedPath!=pOldFolder){
    	free(pOldEncodedPath);
    	pOldEncodedPath=NULL;
    }
    if(pNewEncodedPath && pNewEncodedPath!=pNewFolder){
    	free(pNewEncodedPath);
    	pNewEncodedPath=NULL;
	}
	return -1;
}

#define	SQL_DUMP_COMMAND	"select * from Data "
#define	DB_DUMP_TMP_FILE	"/tmp/db_dump.txt"

static int db_dump_callback(void *pUser, int nArg, char **azArg, char **NotUsed)
{
	FILE *fp=NULL;

	if(nArg>=4){
		fp=fopen(DB_DUMP_TMP_FILE, "at");
		if(fp){
			if(nArg>8){
				//fprintf(fp,"%s %s %s %s %s\n", NotUsed[2], NotUsed[3], NotUsed[1], NotUsed[9] ,NotUsed[8]?NotUsed[8]:"");
				fprintf(fp,"%s%s %s %s %s\n", azArg[2], azArg[3], azArg[1], azArg[9] ,azArg[8]?azArg[8]:"");
			}
			else
				fprintf(fp,"%s%s %s\n", azArg[2], azArg[3], azArg[1]);
			fclose(fp);
		}
		return 0;
	}
    
	return -1;
}


int DumpDataBase(sqlite *db)
{
    char *zErrMsg = NULL;	
    int ret=0;
    
    remove(DB_DUMP_TMP_FILE);
    ret = sqlite_exec(db, SQL_DUMP_COMMAND, db_dump_callback, NULL, &zErrMsg);
    if (ret != SQLITE_OK) {
		if (zErrMsg){
			printf("zErrMsg: %s\n",zErrMsg);
			sqlite_free(zErrMsg);
		}
	}	
	return ret;
}

int GetContainerID(char *pContainerID)
{
	container_id++;
	sprintf(pContainerID, "%s%08llu", CONTAINER_ID_PREFIX, container_id);
	return 0;
}

static void my_error_exit(j_common_ptr cinfo)         
{                                                     
	my_error_ptr myerr=(my_error_ptr) cinfo->err;     
	char buf[JMSG_LENGTH_MAX];                        
	(*cinfo->err->format_message)(cinfo,buf);         
	longjmp(myerr->setjmp_buffer, 1);                 
}        
   
int get_png_info(char *pFileName,struct media_file_properties *pMediaItem)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	FILE *fp=NULL;
	if ((fp = fopen(pFileName, "rb")) == NULL)
		return -1;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,png_voidp_NULL,png_error_ptr_NULL, png_error_ptr_NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return -1;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return -1;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		fclose(fp);
		return -1;
	}
	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,&interlace_type, int_p_NULL, int_p_NULL);
	sprintf(pMediaItem->resolution,"%ldx%ld",width,height);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
#ifdef __PNG_DLNA_SUPPORT__	
	if(getPngProtocol(pMediaItem->protocol_info,pMediaItem->resolution)<0)
		return -1;
#else
	char *protocol_info=NULL;
	protocol_info=mime_get_protocol(pFileName, pMediaItem->file_type);
	if(protocol_info){
		strcpy(pMediaItem->protocol_info, protocol_info);
		free(protocol_info);
	}
#endif
	return 0 ;
}
#if 0
int get_gif_info(char *pFileName,struct media_file_properties *pMediaItem)
{
	GifFileType *GifFileIn = NULL;
	char *protocol_info=NULL;
	if ((GifFileIn = DGifOpenFileName(pFileName)) == NULL)
		return -1;
	sprintf(pMediaItem->resolution,"%dx%d",GifFileIn->SWidth,GifFileIn->SHeight);
	if (DGifCloseFile(GifFileIn) == GIF_ERROR)
		return -1;		 
	protocol_info=mime_get_protocol(pFileName, pMediaItem->file_type);
	if(protocol_info){
		strcpy(pMediaItem->protocol_info, protocol_info);
		free(protocol_info);
	}
	return 0;
}	
#else
int get_gif_info(char *pFileName,struct media_file_properties *pMediaItem)
{
	int fd=0, len=0, width=0, height=0;
	char *protocol_info=NULL, buf[11]={0};
	
	fd=open(pFileName, O_RDONLY);
	if(fd<=0)
		return -1;
	len=read(fd, buf, 10);
	close(fd);
	if(len!=10)
		return -1;
	width=(buf[7]<<8) + buf[6];
	height=(buf[9]<<8) + buf[8];
	if(!width || !height)
		return -1;
	sprintf(pMediaItem->resolution,"%dx%d",width,height);
	protocol_info=mime_get_protocol(pFileName, pMediaItem->file_type);
	if(protocol_info){
		strcpy(pMediaItem->protocol_info, protocol_info);
		free(protocol_info);
	}
	return 0;
}	
#endif

int get_bmp_info(char *pFileName,struct media_file_properties *pMediaItem)
{
	int fd=0, len=0, width=0, height=0;
	char *protocol_info=NULL, buf[27]={0};
	
	fd=open(pFileName, O_RDONLY);
	if(fd<=0)
		return -1;
	len=read(fd, buf, 26);
	close(fd);
	if(len!=26)
		return -1;
	width=(buf[21]<<24) + (buf[20]<<16) + (buf[19]<<8) + buf[18];
	height=(buf[25]<<24) + (buf[24]<<16) + (buf[23]<<8) + buf[22];
	if(!width || !height)
		return -1;
	sprintf(pMediaItem->resolution,"%dx%d",width,height);
	protocol_info=mime_get_protocol(pFileName, pMediaItem->file_type);
	if(protocol_info){
		strcpy(pMediaItem->protocol_info, protocol_info);
		free(protocol_info);
	}
	return 0;
}	
int get_jpeg_info(char *pFileName,struct media_file_properties *pMediaItem)         

{                                                                                     
	FILE *infile;                                                                     
	struct jpeg_decompress_struct cinfo;                                              
	struct my_error_mgr jerr;     
	                                                    
	if (NULL == (infile=fopen(pFileName,"rb"))) {                                      
		return -1;                                                                  
	}                                                                                 
	cinfo.err = jpeg_std_error(&jerr.pub);                                            
	jerr.pub.error_exit = my_error_exit;                                              
	if (setjmp(jerr.setjmp_buffer)) {                                                 
		jpeg_destroy_decompress(&cinfo);                                              
		fclose(infile);                                                               
		return -1;                                                                  
	}                                                                                 
	/* Now we can initialize the JPEG decompression object. */                        
	jpeg_create_decompress(&cinfo);                                                   
	jpeg_stdio_src(&cinfo,infile);                                                    
	jpeg_read_header(&cinfo,TRUE);                                                    
	sprintf(pMediaItem->resolution,"%dx%d",cinfo.image_width,cinfo.image_height);
	jpeg_destroy_decompress(&cinfo);                                                  
	fclose(infile);       
	if(getJpegProtocol(pMediaItem->protocol_info,pMediaItem->resolution)<0)
		return -1;
	return 0;	
}                        


void AddOneItem2EventList(CREATE_EVENT_LIST **pList, char *pFileName)
{
	CREATE_EVENT_LIST *pNew=NULL, *pCurrent=NULL;
	
	pCurrent=*pList;
	while(pCurrent){
		if(!strcmp(pCurrent->pFileName, pFileName))
			return;
		pCurrent=pCurrent->pNext;
	}
	pNew=(CREATE_EVENT_LIST *)malloc(sizeof(CREATE_EVENT_LIST));
	if(!pNew)
		return;
	memset(pNew, 0, sizeof(CREATE_EVENT_LIST));
	pNew->pFileName=strdup(pFileName);
	if(pNew->pFileName==NULL){
		free(pNew);
		pNew=NULL;
		return;
	}
	pNew->pNext=NULL;
	if(*pList==NULL){
		*pList=pNew;
		return;
	}
	pCurrent=*pList;
	while(pCurrent->pNext){
		pCurrent=pCurrent->pNext;
	}		
	pCurrent->pNext=pNew;
	return;
}

void DelOneItemFromEventList(CREATE_EVENT_LIST **pList, char *pFileName)
{
	CREATE_EVENT_LIST *pCurrent=NULL, *pPrevious=NULL;
	
	if(!*pList)
		return;
	pCurrent=pPrevious=*pList;
	while(pCurrent){
		if(!strcmp(pCurrent->pFileName, pFileName)){
			if(pPrevious==*pList){
				if(pPrevious==pCurrent)//First one
				*pList=pPrevious=pCurrent->pNext;
				else //Second one
					pPrevious->pNext=pCurrent->pNext;
			}
			else
				pPrevious->pNext=pCurrent->pNext;
			free(pCurrent->pFileName);
			free(pCurrent);
			pCurrent=NULL;
			return;
		}
		pPrevious=pCurrent;
		pCurrent=pCurrent->pNext;
	}

	return;
}

int IsItemInEventList(CREATE_EVENT_LIST *pList, char *pFileName)
{
	CREATE_EVENT_LIST *pCurrent=NULL;
	
	if(!pList)
		return 0;
	pCurrent=pList;
	while(pCurrent){
		if(!strcmp(pCurrent->pFileName, pFileName)){
			return 1;
		}
		pCurrent=pCurrent->pNext;
	}

	return 0;
}

void FreeEventList(CREATE_EVENT_LIST *pList)
{
	CREATE_EVENT_LIST *pCurrent=NULL, *pPrevious=NULL;
	
	if(!pList)
		return;	
	while(pCurrent){
		pPrevious=pCurrent;
		pCurrent=pCurrent->pNext;
		free(pPrevious->pFileName);
		free(pPrevious);
		pPrevious=NULL;
	}
	return;	
}

void mBUG(char *format,...);

int inotify_initialized=0;
/*
 * Modified History
 *
 	2007/11/01 Shearer Lu
 	1. Optimize the code and fix some bugs.
 	
 	2007/11/04	Shearer Lu
 	1. Support Linux Inotify.
*/



/*
   CREATE TABLE Data(
   Title VARCHAR(256),      //dir,audio,video,photo
   ObjectPath VARCHAR(256), //file or directory path
   ObjectName VARCHAR(256), //file or direcotry name
   BuildDateY VARCHAR(4), //year
   BuildDateM VARCHAR(2), //month
   BuildDateD VARCHAR(2), //day
   BuildTime VARCHAR(8),  //hour,minute,second
   Artist VARCHAR(256),   //artist
   Album VARCHAR(256),    //album
   Genre VARCHAR(256),    //genre
   Size VARCHAR(256),     //file size
   WebLocation VARCHAR(256), // web location
   */
/*
 * CREATE TABLE Data_Root(
 * Root VARCHAR(256), // root path
 */


#ifdef __LINUX_INOTIFY__
#define	WATCH_MASK	(IN_CREATE|IN_CLOSE_WRITE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO) //|IN_DELETE_SELF
#endif

#define	SCAN_REAL_TIME		0
#define	SCAN_FIXED_INTERVAL	1
#define	SCAN_DISABLED		2
int ScanDB(void);

dlna_t *dlna=NULL;


#define	DATABASE_SCAN_OK	"db_scan.ok"
#define	DATABASE_SCANNING	"/tmp/db_scaning"

void sig_term(int sig)
{
	printf("Exitting...\n");
#ifdef __LINUX_INOTIFY__    
	inotifytools_exit();
#endif	
	FreeEventList(pEventCreateList);
	dlna_uninit (dlna);
	exit(-1);
}

void sig_hup(int sig)
{
	if(access(DATABASE_SCANNING, F_OK) && inotify_initialized){
		signal(SIGHUP, SIG_IGN);	
		ScanDB();
		signal(SIGHUP, sig_hup);	
	}
}

void CleanFile(void)
{
	remove(MEDIA_SCAN_PID);
}

int sqlite_exec(sqlite3 *db, const char *sql, sqlite3_callback cb, void *arg, char **errmsg)
{
    int ret=0,num=0;

do_retry:
    ret = sqlite3_exec(db, sql, cb, arg, errmsg); 
	if(ret==SQLITE_BUSY){
		num++;
		if(num<15){
			if(*errmsg){
				sqlite_free(*errmsg);
				*errmsg=NULL;	
			}
			sleep(2);
			goto do_retry;
		}
	}	    
    return ret;
}


char *scan_winamp_genre[] = {
    "Blues",              // 0
    "Classic Rock",
    "Country",
    "Dance",
    "Disco",
    "Funk",               // 5
    "Grunge",
    "Hip-Hop",
    "Jazz",
    "Metal",
    "New Age",            // 10
    "Oldies",
    "Other",
    "Pop",
    "R&B",
    "Rap",                // 15
    "Reggae",
    "Rock",
    "Techno",
    "Industrial",
    "Alternative",        // 20
    "Ska",
    "Death Metal",
    "Pranks",
    "Soundtrack",
    "Euro-Techno",        // 25
    "Ambient",
    "Trip-Hop",
    "Vocal",
    "Jazz+Funk",
    "Fusion",             // 30
    "Trance",
    "Classical",
    "Instrumental",
    "Acid",
    "House",              // 35
    "Game",
    "Sound Clip",
    "Gospel",
    "Noise",
    "AlternRock",         // 40
    "Bass",
    "Soul",
    "Punk",
    "Space",
    "Meditative",         // 45
    "Instrumental Pop",
    "Instrumental Rock",
    "Ethnic",
    "Gothic",
    "Darkwave",           // 50
    "Techno-Industrial",
    "Electronic",
    "Pop-Folk",
    "Eurodance",
    "Dream",              // 55
    "Southern Rock",
    "Comedy",
    "Cult",
    "Gangsta",
    "Top 40",             // 60
    "Christian Rap",
    "Pop/Funk",
    "Jungle",
    "Native American",
    "Cabaret",            // 65
    "New Wave",
    "Psychadelic",
    "Rave",
    "Showtunes",
    "Trailer",            // 70
    "Lo-Fi",
    "Tribal",
    "Acid Punk",
    "Acid Jazz",
    "Polka",              // 75
    "Retro",
    "Musical",
    "Rock & Roll",
    "Hard Rock",
    "Folk",               // 80
    "Folk/Rock",
    "National folk",
    "Swing",
    "Fast-fusion",
    "Bebob",              // 85
    "Latin",
    "Revival",
    "Celtic",
    "Bluegrass",
    "Avantgarde",         // 90
    "Gothic Rock",
    "Progressive Rock",
    "Psychedelic Rock",
    "Symphonic Rock",
    "Slow Rock",          // 95
    "Big Band",
    "Chorus",
    "Easy Listening",
    "Acoustic",
    "Humour",             // 100
    "Speech",
    "Chanson",
    "Opera",
    "Chamber Music",
    "Sonata",             // 105 
    "Symphony",
    "Booty Bass",
    "Primus",
    "Porn Groove",
    "Satire",             // 110
    "Slow Jam",
    "Club",
    "Tango",
    "Samba",
    "Folklore",           // 115
    "Ballad",
    "Powder Ballad",
    "Rhythmic Soul",
    "Freestyle",
    "Duet",               // 120
    "Punk Rock",
    "Drum Solo",
    "A Capella",
    "Euro-House",
    "Dance Hall",         // 125
    "Goa",
    "Drum & Bass",
    "Club House",
    "Hardcore",
    "Terror",             // 130
    "Indie",
    "BritPop",
    "NegerPunk",
    "Polsk Punk",
    "Beat",               // 135
    "Christian Gangsta",
    "Heavy Metal",
    "Black Metal",
    "Crossover",
    "Contemporary C",     // 140
    "Christian Rock",
    "Merengue",
    "Salsa",
    "Thrash Metal",
    "Anime",              // 145
    "JPop",
    "SynthPop",
    "-UNKNOWN-"
};

#define WINAMP_GENRE_UNKNOWN 148

#ifdef __ICONV_SUPPORT__
#if 0
char *codepage_str[]={"CP950","CP949","CP936","CP932","CP866","CP861","CP852","CP850","CP737","CP437",NULL};
#endif
static size_t do_convert(const char* to_ces, const char* from_ces, 
			 char *inbuf,  size_t inbytesleft,
			 char *outbuf, size_t outbytesleft) {
  iconv_t cd  = libiconv_open(to_ces, from_ces);
  size_t ret = libiconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  libiconv_close(cd);
  return ret;
}

extern char codepage[10];
static int ConvertCP2UTF8(char *pSrc, char pDest[1024])
{
	int  ret=0;
	char converted_text[1024] = {0};
/*
	int i=0;	
	while(codepage_str[i]){
		ret=do_convert("UTF-8", codepage_str[i], pSrc, strlen(pSrc), converted_text, 1024);
		printf("Codepage: %s\n",codepage_str[i]);
		if(ret!=-1){
			if(strlen(converted_text)<1024)
				strcpy(pDest, converted_text);
			else
				strcpy(pDest, pSrc);
			return 0;	
		}
		i++;
	}
	return -1;
*/
	ret=do_convert("UTF-8", codepage, pSrc, strlen(pSrc), converted_text, 1024);
	if(ret!=-1){
		if(strlen(converted_text)<1024)
			strcpy(pDest, converted_text);
		else
			strcpy(pDest, pSrc);
		return 0;	
	}
	return -1;
}	
	
#endif

#define TAG_STR_SIZE 1024

#if 0
static unsigned char* get_utf8_text(const id3_ucs4_t* native_text)
{
    unsigned char* utf8_text = NULL;
    int codepage = 0;
    FILE *fp = NULL;
	char code_str[8]="437";
    char* const in_ptr  = (char*)id3_ucs4_latin1duplicate(native_text);
    char* const in8_ptr = (char*)id3_ucs4_utf8duplicate(native_text);
    char converted_text[TAG_STR_SIZE] = {0}, language[10]={0};

    fp = fopen(media_conf_file, "rt");
    if (!fp) {
		return NULL;
    }
    PRO_GetStr("main", "codepage", code_str, sizeof(code_str)-1,fp);
    fclose(fp);
	codepage=atoi(code_str);
	
    sprintf(language,"CP%d",codepage);
    if (in_ptr && in8_ptr) {

		/* (1) try utf8 -> cp936 etc. */
		size_t rc = do_convert(language, "UTF-8", in8_ptr, strlen(in8_ptr), converted_text, TAG_STR_SIZE - 1);
		if(rc == (size_t)-1) {
		    if (errno != E2BIG) {
				/* (2) try cp932 -> utf8 */
				memset(converted_text, '\0', TAG_STR_SIZE);
				rc = do_convert("UTF-8", language, in_ptr, strlen(in_ptr), converted_text, TAG_STR_SIZE - 1);
				if(rc == (size_t)-1) {
				    if (errno != E2BIG) {
						/* utf-8 including non-japanese char? fallback. */
						utf8_text=(char*)id3_ucs4_utf8duplicate(native_text);
				    }
				} else {
				    /* valid cp936 etc.: in_ptr */
				    utf8_text = (unsigned char*)calloc(strlen(converted_text) + 1, sizeof(unsigned char));
				    if(utf8_text) {
						memcpy(utf8_text, converted_text, strlen(converted_text));
				    }
				}
		    }
		    free(in8_ptr);
		}
		else {
		    /* valid utf8: in8_ptr */
		    utf8_text = in8_ptr;
		}
		free(in_ptr);
    }

    if(!utf8_text) {
		utf8_text = strdup(UNKNOWN_STR);
    }

    return utf8_text;
}
#endif

unsigned long long g_nindex = 0;

int GetFolderType(char *pPath, struct entry *content)
{
	int type=0, i=0;
	
	for (i = 0; i < CONTENT_NUM; i++) {
		if(content[i].path && strstr(pPath, content[i].path)){
			type|=content[i].type;
		}
	}		
	return type;
}

void FreeMediaItem(struct media_file_properties *pMediaItem)
{
	if(!pMediaItem)
		return;
	if(pMediaItem->title){
		free(pMediaItem->title);
		pMediaItem->title=NULL;
	}
	if(pMediaItem->author){
		free(pMediaItem->author);
		pMediaItem->author=NULL;
	}
	if(pMediaItem->artist){
		free(pMediaItem->artist);
		pMediaItem->artist=NULL;
	}
	if(pMediaItem->album){
		free(pMediaItem->album);
		pMediaItem->album=NULL;
	}	
	if(pMediaItem->genre){
		free(pMediaItem->genre);
		pMediaItem->genre=NULL;
	}		
	if(pMediaItem->comment){
		free(pMediaItem->comment);
		pMediaItem->comment=NULL;
	}				
}

int ReadMediaFileInfo (char *pFileName, struct media_file_properties *pMediaItem)
{
	dlna_profile_t *p=NULL;
	dlna_org_flags_t flags;
	dlna_item_t *item=NULL;
	int dlna_compliant=0;
	char *protocol_info=NULL;
	struct stat filestat;
	struct tm *st=NULL;
	char *ext=NULL;
	char *p0=NULL;

	flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
		DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
		DLNA_ORG_FLAG_CONNECTION_STALL |
		DLNA_ORG_FLAG_DLNA_V15;
	ext = strrchr(pFileName,'.');
	if(ext==NULL)
		return -1;
	//printf ("pFileName: %s\n", pFileName);
	if(strcasecmp(ext+1,"jpg")==0 || strcasecmp(ext+1,"jpeg")==0 || strcasecmp(ext+1,"jpe")==0){
		get_jpeg_info(pFileName,pMediaItem);
	}
	else if(strcasecmp(ext+1,"gif")==0){
		get_gif_info(pFileName,pMediaItem);
	}
	else if(strcasecmp(ext+1,"png")==0){
		get_png_info(pFileName,pMediaItem);
	}
	else if(strcasecmp(ext+1,"bmp")==0){
		get_bmp_info(pFileName,pMediaItem);
	}	
	else{
		item = dlna_item_new (dlna, pFileName, &dlna_compliant);
		if (item) {
			if (item->properties) {
				pMediaItem->file_type=item->properties->file_type;
				pMediaItem->size=item->properties->size;
			  	p0=strrchr(pFileName, '.');
			  	if(p0 && !strcasecmp(p0+1, "pcm")){  
					p0=strrchr(pFileName, '/');
					if(p0)
						p0++;
					else
						p0=pFileName;
				  	if(!strcasecmp(p0, "B-LPCM-1.pcm") || !strcasecmp(p0, "B-LPCM-2.pcm")
				  	|| !strcasecmp(p0, "B-LPCM-3.pcm") || !strcasecmp(p0, "B-LPCM-4.pcm")){
						strcpy(pMediaItem->duration, "00:01:00");
				  	}
				  	else
				  		strcpy(pMediaItem->duration, "");
				}				  				
				else
					strcpy(pMediaItem->duration, item->properties->duration);
				p0=strrchr(pFileName, '.');
				if(p0)
					p0++;
				else
					p0=pFileName;
				if(!strcasecmp(p0, "mpg") && !strlen(pMediaItem->duration)){
					p0=strrchr(pFileName, '/');
					if(p0)
						p0++;
					else
						p0=pFileName;
					if(!strcmp(p0, "O-MP2TS_SE-2.mpg") || !strcmp(p0, "O-MP2TS_SE-4.mpg")
						|| !strcmp(p0, "O-MP2TS_SE_I-2.mpg") || !strcmp(p0, "O-MP2TS_SE_I-4.mpg")
						|| !strcmp(p0, "O-MP2TS_SET-9.mpg"))
						strcpy(pMediaItem->duration, "00:01:05");	
					if(!strcmp(p0, "O-MP2TS_SNT-11.mpg"))
						strcpy(pMediaItem->duration, "00:01:09");											
				}
				pMediaItem->bitrate=item->properties->bitrate;
				pMediaItem->sample_frequency=item->properties->sample_frequency;
				pMediaItem->bps=item->properties->bps;
				pMediaItem->channels=item->properties->channels;
				if((item->properties->resolution)&&(strcmp(item->properties->resolution,"")))
					strcpy(pMediaItem->resolution, item->properties->resolution);
				else
					strcpy(pMediaItem->resolution,UNKNOWN_STR);
	#if 0			
				printf ("Duration: %s\n", item->properties->duration);
				printf ("Bitrate: %d bytes/sec\n", item->properties->bitrate);
				printf ("SampleFrequency: %d Hz\n", item->properties->sample_frequency);
				printf ("BitsPerSample: %d\n", item->properties->bps);
				printf ("Channels: %d\n", item->properties->channels);
				printf ("Resolution:%s\n", item->properties->resolution);
	#endif		
			}
	
		    if (item->metadata) {
#ifdef __ICONV_SUPPORT__		    	
		    	int ret=0;
		    	char converted_text[1024] = {0};
#endif
		    	ChangeSpecialChar(item->metadata->title);
		    	StripSpace(item->metadata->title);
		    	if(strlen(item->metadata->title))
		    		pMediaItem->title=strdup(item->metadata->title);
		    	else
		    		pMediaItem->title=strdup(UNKNOWN_STR);

				ChangeSpecialChar(item->metadata->author);
				StripSpace(item->metadata->author);
		    	if(strlen(item->metadata->author)){
#ifdef __ICONV_SUPPORT__		    		
					ret=ConvertCP2UTF8(item->metadata->author, converted_text);
		    		if(!ret && strlen(converted_text))
		    			pMediaItem->artist=strdup(converted_text);
		    		else
#endif		    		
		    			pMediaItem->artist=strdup(item->metadata->author);
#ifdef __ICONV_SUPPORT__		    			
		    		converted_text[0]=0;
#endif		    		
		    	}
		    	else
					pMediaItem->artist=strdup(UNKNOWN_STR);	   
				ChangeSpecialChar(item->metadata->comment);
				StripSpace(item->metadata->comment);
		    	if(strlen(item->metadata->comment))
		    		pMediaItem->comment=strdup(item->metadata->comment);
		    	//else
		    	//	pMediaItem->comment=strdup(UNKNOWN_STR);	 
		    	ChangeSpecialChar(item->metadata->album);
		    	StripSpace(item->metadata->album);
				if(strlen(item->metadata->album)){
#ifdef __ICONV_SUPPORT__
		    		ret=ConvertCP2UTF8(item->metadata->album, converted_text);
		    		if(!ret && strlen(converted_text))
		    			pMediaItem->album=strdup(converted_text);
		    		else
#endif		    		
		    			pMediaItem->album=strdup(item->metadata->album);
#ifdef __ICONV_SUPPORT__		    			
		    		converted_text[0]=0;
#endif		    		
		    	}
		    	else
		    		pMediaItem->album=strdup(UNKNOWN_STR);	
				ChangeSpecialChar(item->metadata->genre);
				StripSpace(item->metadata->genre);
		    	if(strlen(item->metadata->genre)){
#ifdef __ICONV_SUPPORT__		    		
		    		ret=ConvertCP2UTF8(item->metadata->genre, converted_text);
		    		if(!ret && strlen(converted_text))
		    			pMediaItem->genre=strdup(converted_text);
		    		else		 
#endif		    		   		
		    			pMediaItem->genre=strdup(item->metadata->genre);
#ifdef __ICONV_SUPPORT__		    			
		    		converted_text[0]=0;
#endif		    		
		    	}
		    	else
		    		pMediaItem->genre=strdup(UNKNOWN_STR);	
				pMediaItem->track=item->metadata->track;	
				if(item->metadata->year)
					sprintf(pMediaItem->year, "%d", item->metadata->year);
	#if 0			  			    			    			    		
				printf ("Title: %s\n", item->metadata->title);
				printf ("Artist: %s\n", item->metadata->author);
				printf ("Description: %s\n", item->metadata->comment);
				printf ("Album: %s\n", item->metadata->album);
				printf ("Track: %d\n", item->metadata->track);
				printf ("Genre: %s\n", item->metadata->genre);
	#endif			
			}
		}
		else{
			if(dlna_compliant)
				printf("The file isn't DLMA compliant.\n");
			return -1; 
		}
		p=item->profile;
		if (p) {
	#if 0   
			printf ("ID: %s\n", p->id);
		    printf ("MIME: %s\n", p->mime);
		    printf ("Label: %s\n", p->label);
		    printf ("Class: %d\n", p->media_class);
		    printf ("UPnP Object Item: %s\n", dlna_profile_upnp_object_item (p));
	#endif
		    /*support timeseek of all video and audio*/
		    if((strstr(SUPPORTED_VIDEO_FORMAT,ext+1)!=NULL)||(strstr(SUPPORTED_MUSIC_FORMAT,ext+1)!=NULL)) {
		    	
		    	if(strstr(NO_TIMESEEK_FORMAT, ext+1))
					protocol_info = dlna_write_protocol_info (DLNA_PROTOCOL_INFO_TYPE_HTTP,
	                                              DLNA_ORG_PLAY_SPEED_NORMAL,
	                                              DLNA_ORG_CONVERSION_NONE,
	                                              DLNA_ORG_OPERATION_RANGE,
	                                              flags, p);		    	
		    	else
					protocol_info = dlna_write_protocol_info (DLNA_PROTOCOL_INFO_TYPE_HTTP,
	                                              DLNA_ORG_PLAY_SPEED_NORMAL,
	                                              DLNA_ORG_CONVERSION_NONE,
	                                              DLNA_ORG_OPERATION_RANGE|DLNA_ORG_OPERATION_TIMESEEK,
	                                              flags, p);
		    }
		    else {
			    protocol_info = dlna_write_protocol_info (DLNA_PROTOCOL_INFO_TYPE_HTTP,
					    DLNA_ORG_PLAY_SPEED_NORMAL,
					    DLNA_ORG_CONVERSION_NONE,
					    DLNA_ORG_OPERATION_RANGE,
					    flags, p);
		    }
	#if 0                                              
			printf ("Protocol Info: %s\n", protocol_info);
	#endif		
			strcpy(pMediaItem->protocol_info, protocol_info);
			free (protocol_info);
		}
		
		else{
			//Not DLNA Compliant File, construct the Procotol Informaiton.
			protocol_info=mime_get_protocol(pFileName, pMediaItem->file_type);
			if(protocol_info){
				strcpy(pMediaItem->protocol_info, protocol_info);
				free(protocol_info);
			}
#if 0			
			{
				FILE *ff=NULL;
				
				ff=fopen("/tmp/failed_file.txt", "at");	
				fprintf(ff,"Not DLNA Compliant: %s\n", pFileName);
				fclose(ff);
			}
#endif			
		}
		dlna_item_free (item);
	}
    if((!strlen(pMediaItem->year) || !pMediaItem->size)&& !stat(pFileName,&filestat)){ //Read Time Information.
	    if(!strlen(pMediaItem->year)){
			st = localtime(&filestat.st_mtime);
		    sprintf(pMediaItem->year, "%d", st->tm_year+1900);
		    sprintf(pMediaItem->month,"%02d", st->tm_mon+1);
		    sprintf(pMediaItem->day, "%02d", st->tm_mday);
		    sprintf(pMediaItem->time,"%02d:%02d:%02d", st->tm_hour,st->tm_min,st->tm_sec); 
	    }
	    if(!pMediaItem->size){
	    	pMediaItem->size=filestat.st_size;	
	    }
    }
	return 0;
}

int scan_main(char *pConfFile, int fast_scan)
{
    char *zErr = NULL;
    int rc = 0, fd=0;
    sqlite *db=NULL;
    static char tmpbuf[1024];
    DIR *dir=NULL;
    char db_folder[256]={0}, db_file[256]={0},db_ok_flag[256]={0};
    char temp_db_file[256];
    char buf[2048]={0}, scan_str[8]={0};
    FILE *fp = NULL;
    struct entry content[CONTENT_NUM];
    int i = 0, j=0, len=0, scan_interval=60, scan_method=SCAN_FIXED_INTERVAL;
    int use_temp_db=0, media_type=MEDIA_TYPE_NONE;
	struct sigaction act;
#ifdef __LINUX_INOTIFY__    
	int timeout = 2;
	struct inotify_event * event;
	char moved_from[1024]={0}; 	
//	int from_to_event=0;
#endif

	act.sa_handler=sig_term;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;	
	sigaction(SIGINT, &act, NULL);	
	sigaction(SIGTERM, &act, NULL);	
	signal(SIGHUP, sig_hup);	
	
	strcpy(media_conf_file, pConfFile);
    fp = fopen(pConfFile, "rt");
    if (fp == NULL) {
		return -1;
    }
    PRO_GetStr("main", "contentdir", buf, sizeof(buf)-1, fp);
    if (strlen(buf) == 0) {
		mBUG("contentdir is null.\n");
		fclose(fp);
		return -1;
    }
	PRO_GetStr("main", "scan_interval", scan_str, sizeof(scan_str)-1,fp);	
	scan_interval=atoi(scan_str);
	if(scan_interval==-1)//Scan in real-time.
		scan_method=SCAN_REAL_TIME;
	else if(scan_interval==0)//No scan
		scan_method=SCAN_DISABLED;
	else if(scan_interval<0)
		scan_interval=60;
	else if(scan_interval<15)//Minmum value is 15 minutes.
		scan_interval=15;
    /* setting up db file */
    PRO_GetStr("main", "dbfile", db_folder, 256, fp);
    if (strlen(db_folder) == 0) {
		mBUG("dbfile is null.\n");
		fclose(fp);
		return -1;
    }
    fclose(fp);
 	fp=fopen(MEDIA_SCAN_PID, "wt");
	if(fp){
		fprintf(fp,"%d",getpid());
		fclose(fp);
	}   
	atexit(CleanFile);
#ifdef __LINUX_INOTIFY__	
	if ( inotifytools_initialize())
		inotify_initialized=1;
#endif    
    
    if(access(db_folder, F_OK)){
    	mkdir(db_folder, 0775);
    }
    else{
    	struct stat f_stat;
    	
    	stat(db_folder, &f_stat);
    	if(!S_ISDIR(f_stat.st_mode)){
    		remove(db_folder);
    		mkdir(db_folder, 0775);
    	}
    }
    sprintf(db_ok_flag, "%s/%s", db_folder, DATABASE_SCAN_OK);
    sprintf(db_file, "%s/%s", db_folder, DB_FILE_NAME);
    sprintf(temp_db_file, "%s.tmp", db_file);
    if(!fast_scan){
		remove(db_file); //Always delete it and re-create it.
		remove(db_ok_flag);
	}
	
	printf("db_file: %s\n",db_file);
	fd=creat(DATABASE_SCANNING, O_CREAT);
	if(fd>=0)
		close(fd);
    if(access(db_file,F_OK)==0) {//Database exists, open it.
#if defined DEFSQLITE3
		chmod(db_file,0644);
		sqlite_open(db_file,&db);
#else            	
		db = sqlite_open(db_file,0666, &zErr);
#endif		    	
		if(db==NULL||zErr!=NULL){
			if(zErr)
		    	sqlite_free(zErr);
#ifdef __LINUX_INOTIFY__		    
			if(inotify_initialized)
				inotifytools_exit();		 
#endif				   
			remove(DATABASE_SCANNING);
			remove(db_ok_flag);
		    return SQLITE_ERROR;
		}
    }
    else  {//Database doesn't exist, create it.
    	remove(db_ok_flag);
#if defined DEFSQLITE3
		sqlite_open(db_file,&db);
#else            	
		db = sqlite_open(db_file,0666, &zErr);
#endif		    	
		if(db==NULL || zErr!=NULL) {
			if(zErr)
		    	sqlite_free(zErr);
#ifdef __LINUX_INOTIFY__		    
			if(inotify_initialized)
				inotifytools_exit();
#endif					
			remove(DATABASE_SCANNING);	    
		    return SQLITE_ERROR;
		}
	
		sprintf(tmpbuf, "%s","CREATE TABLE Data(Index_Id INTEGER NOT NULL,Title VARCHAR(256),ObjectPath VARCHAR(256),ObjectName VARCHAR(256)\
		    ,BuildDateY VARCHAR(4),BuildDateM VARCHAR(2),BuildDateD VARCHAR(2),BuildTime VARCHAR(8)\
			,Artist VARCHAR(256),Album VARCHAR(256),Genre VARCHAR(256), Size VARCHAR(256),WebLocation VARCHAR(256),Duration VARCHAR(12),Resolution VARCHAR(256), ProtocolInfo VARCHAR(256),PlayTimes INTEGER, ContainerID VARCHAR(32),PRIMARY KEY(ObjectPath,ObjectName))");
		rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
		if(SQLITE_OK != rc || zErr != NULL) {
			if(zErr)
		    	sqlite_free(zErr);
		    sqlite_close(db);
		    remove(db_file);
#ifdef __LINUX_INOTIFY__		    
			if(inotify_initialized)
				inotifytools_exit();
#endif					
			remove(DATABASE_SCANNING);	    
		    return SQLITE_ERROR;
		}
		sprintf(tmpbuf,"%s", "CREATE INDEX index_Data ON Data(Index_Id);");
		rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
		if(SQLITE_OK != rc || zErr != NULL) {
			if(zErr)
		    	sqlite_free(zErr);
		    sqlite_close(db);
		    remove(db_file);
#ifdef __LINUX_INOTIFY__		    
			if(inotify_initialized)
				inotifytools_exit();	
#endif				
			remove(DATABASE_SCANNING);	    
		    return SQLITE_ERROR;
		}
		sprintf(tmpbuf,"%s","CREATE TABLE Data_Root(Root VARCHAR(256) PRIMARY KEY NOT NULL)");
		rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
		if(SQLITE_OK != rc || zErr != NULL) {
			if(zErr)
		    	sqlite_free(zErr);
		    sqlite_close(db);
		    remove(db_file);
#ifdef __LINUX_INOTIFY__		    
			if(inotify_initialized)
				inotifytools_exit();	
#endif				
			remove(DATABASE_SCANNING);	    
		    return SQLITE_ERROR;
		}
    }
    /* parse content dir and type */
    {
		char *p=NULL, *p2=NULL, *p3=NULL;
		char chr=0;
	
		memset(content, 0, sizeof(content));
		p = buf;
		while (p != NULL) {
		    p = strchr(p, '|');
		    if(p){
			    chr = *(p-1);
			    p++;
			    p3=strchr(p+1, '|');
		    }
		    else
		    	p3=NULL;
		    if(p3){
			    p2=strstr(p, ",+P|");
	 			if(p2+3!=p3)
			    	p2=NULL;	
				if(!p2){
					p2=strstr(p, ",+A|");
					if(p2 && p2+3!=p3)
						p2=NULL;
				}	
				if(!p2){
					p2=strstr(p, ",+V|");
					if(p2 && p2+3!=p3)
						p2=NULL;	
				}

				if(!p2){
					p2=strstr(p, ",+M|");
					if(p2 && p2+3!=p3)
						p2=NULL;	
				}		    					
			    if (p2) {
					*p2++ = '\0';
			    }		  
		    }  	
		    else
		    	p2=NULL;		    	
		    content[i].path = malloc(strlen(p)+2);
		    memset(content[i].path, 0, strlen(p)+2);
		    strcpy(content[i].path, p);
		    len=strlen(content[i].path);
		    if(content[i].path[len-1]!='/')
		    	strcat(content[i].path, "/");
			//printf("content[%d].path=%s\n", i, content[i].path);
		    switch (chr) {
				case 'M':
				    content[i].type = MUSIC;
				    break;
				case 'V':
				    content[i].type = VIDEO;
				    break;
				case 'P':
				    content[i].type = PICTURE;
				    break;
				case 'A':
				default:
				    content[i].type = ALL;
				    break;
		    }
		    p = p2;
		    i++;
		} /* while (p != NULL) */
    }
    for (i = 0; i < CONTENT_NUM; i++) {
    	if(!content[i].path || !strlen(content[i].path))
    		continue;
    	//printf("i content[%d].path=%s|Type:%d\n", i, content[i].path, content[i].type);
		for (j = 0; j < CONTENT_NUM; j++) {
	    	if(!content[j].path || !strlen(content[j].path) || i==j)
	    		continue;
	    	if(!content[i].path)
	    		break;		    		
	    	//printf("j content[%d].path=%s  Type: %d\n", j, content[j].path, content[j].type);
	    	if(!strcmp(content[j].path, content[i].path)){
	    		content[i].type |= content[j].type;	    
	    		free(content[j].path);
	    		content[j].path=NULL;
	    	}		
	    	else if(strstr(content[i].path, content[j].path)){
	    		if((content[j].type & content[i].type) == content[i].type){//Type of parent folder includes children's.
		    		free(content[i].path);
		    		content[i].path=NULL;   			
	    		}
	    		else
	    			content[i].type |= content[j].type;
	    	}
    	}	
    }
    for (i = 0; i < CONTENT_NUM; i++) {
    	if(!content[i].path || !strlen(content[i].path))
    		continue;
    	//printf("content[%d].path=%s|Type:%d\n", i, content[i].path, content[i].type);
	}    
	dlna = dlna_init ();
	dlna_register_all_media_profiles (dlna);	
do_scan:
	if(use_temp_db){
		if(access(DATABASE_SCANNING, F_OK)){
			fd=creat(DATABASE_SCANNING, O_CREAT);
			if(fd>=0)
				close(fd);
		}
		remove(temp_db_file);
		printf("Write temporary DB: %s\n",temp_db_file);
#if defined DEFSQLITE3
		chmod(db_file,0644);
		sqlite_open(temp_db_file,&db);
#else            	
		db = sqlite_open(temp_db_file,0666, &zErr);
#endif		    	
		if(db==NULL || zErr!=NULL) {
			if(zErr)
		    	sqlite_free(zErr);
		    remove(DATABASE_SCANNING);
		    remove(db_ok_flag);
		    goto scan_wait;
		}	
		sprintf(tmpbuf, "%s","CREATE TABLE Data(Index_Id INTEGER NOT NULL,Title VARCHAR(256),ObjectPath VARCHAR(256),ObjectName VARCHAR(256)\
		    ,BuildDateY VARCHAR(4),BuildDateM VARCHAR(2),BuildDateD VARCHAR(2),BuildTime VARCHAR(8)\
			,Artist VARCHAR(256),Album VARCHAR(256),Genre VARCHAR(256), Size VARCHAR(256), WebLocation VARCHAR(256),Duration VARCHAR(12),Resolution VARCHAR(256), ProtocolInfo VARCHAR(256),PlayTimes INTEGER, ContainerID  VARCHAR(32), PRIMARY KEY(ObjectPath,ObjectName))");
		rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
		if(SQLITE_OK != rc || zErr != NULL) {
			if(zErr)
		    	sqlite_free(zErr);
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    remove(db_ok_flag);
		    goto scan_wait;
		}
		sprintf(tmpbuf,"%s", "CREATE INDEX index_Data ON Data(Index_Id);");
		rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
		if(SQLITE_OK != rc || zErr != NULL) {
			if(zErr)
		    	sqlite_free(zErr);
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    remove(db_ok_flag);
		    goto scan_wait;
		}
#if 0		
		sprintf(tmpbuf,"%s","CREATE TABLE Data_Root(Root VARCHAR(256) PRIMARY KEY NOT NULL)");
		rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
		if(SQLITE_OK != rc || zErr != NULL) {
			if(zErr)
		    	sqlite_free(zErr);
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    remove(db_ok_flag);
		    goto scan_wait;
		}	
#endif				
	}
	//system("date >>/tmp/failed_file.txt");	
#ifdef _EARLY_ADD_
    for (i = 0; i < CONTENT_NUM; i++) {
    	if(!content[i].path || !strlen(content[i].path))
    		continue;
    		
		if ((dir = opendir(content[i].path)) == NULL)  {
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    remove(db_ok_flag);
		    goto out;
		}
		closedir(dir);
        early_add = 1;
		if(insert_record_to_db(db, content[i].path, content[i].type, fast_scan)) {
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    remove(db_ok_flag);
		    goto out;
		}
        early_add = 0;
        printf("#### final early add.\n");
    }
#endif

    for (i = 0; i < CONTENT_NUM; i++) {
    	if(!content[i].path || !strlen(content[i].path))
    		continue;
    		
		if ((dir = opendir(content[i].path)) == NULL)  {
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    remove(db_ok_flag);
		    goto out;
		}
		closedir(dir);
		if(insert_record_to_db(db, content[i].path, content[i].type, fast_scan)) {
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    remove(db_ok_flag);
		    goto out;
		}
    }
	sqlite_close(db);
	if(fast_scan)
		fast_scan=0;
	if(use_temp_db){
		use_temp_db=0;
		remove(db_file);
		rc=rename(temp_db_file, db_file);
		if(!rc){
			printf("Use temporary DB: %s\n",temp_db_file);
			if(fd>=0)
				close(fd);			
		}
		else
			printf("Fail to use temporary DB: %s\n",temp_db_file);		
	}
	else{
		fd=creat(db_ok_flag, 0644);
		if(fd>=0)
			close(fd);
	}
	chmod(db_file,0444);
	remove(DATABASE_SCANNING);
out:	
	//system("date >>/tmp/failed_file.txt");	
#ifdef __LINUX_INOTIFY__    
    if(scan_method==SCAN_REAL_TIME){
		while(1) {
			//from_to_event=0;
			event = inotifytools_next_event( timeout );
			if ( !event ) {
				if ( !inotifytools_error() ) {
					//sleep(60);
					continue;
				}
				else {
					mBUG("%s\n", strerror( inotifytools_error() ) );
					//sleep(60);
					continue;
				}
			}
check_event:			
			//inotifytools_printf( event, "%w %,e %f\n" );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 12)
			strcpy(buf, event->filename);
#else
			strcpy(buf, event->name);
#endif
			if (event->mask & IN_CREATE){ // New file - if it is a directory, watch it
				char new_file[1024]={0};
				
				printf("IN_CREATE: %s\n", buf);//File from outside
				//printf("event->wd: %d\n", event->wd);
				sprintf( new_file, "%s%s", inotifytools_filename_from_wd( event->wd ), buf );
#ifdef __NAS_THUMBNAIL_FUN__
				if(strstr(new_file, THUMB_FOLDER))//Ignore Thumbnails.
					continue;
#endif				
				if(strstr(new_file, cache_folder_path))//Ignore files in cache folder.
					continue;	
					
				if ( isdir(new_file)){
#if defined DEFSQLITE3
					chmod(db_file,0644);
					sqlite_open(db_file,&db);
#else
					db = sqlite_open(db_file,0666, &zErr);
#endif
					if(db) {
						if(insert_record_to_db(db, new_file, GetFolderType(new_file, content), 0)) {
							mBUG("Fail to add folder '%s' to DB\n", new_file);
						}
						rc=AddOneFolder2DB(db, new_file);
						if(!rc)
							inotifytools_watch_recursively( new_file, WATCH_MASK );
						sqlite_close(db);
					}
					chmod(db_file,0444);
				}
				else{
					AddOneItem2EventList(&pEventCreateList, buf);
				}
			} // IN_CREATE
			else if (event->mask & IN_MOVED_FROM) {
				
				printf("IN_MOVED_FROM: %s\n", buf);
				if(event->mask & IN_ISDIR){
					sprintf( moved_from, "%s%s/", inotifytools_filename_from_wd( event->wd ), buf );
					
					//Try to find IN_MOVED_TO
					event = inotifytools_next_event( timeout );
					if ( !event || !(event->mask & IN_MOVED_TO)) {
						
		#if defined DEFSQLITE3
						chmod(db_file,0644);
						sqlite_open(db_file,&db);
		#else   
						db = sqlite_open(db_file,0666, &zErr);
		#endif					
						if(db) {	
							DelFolderFromDBRecursive(db, moved_from);
							rc=del_record_from_db(db, moved_from, 1);//
							if(rc){
								printf("Fail to remove folder '%s' from DB\n", moved_from);
							}				
							//printf("Delete Folder: %s\n",moved_from);
							sqlite_close(db);	
							moved_from[0]=0;
						}	
						chmod(db_file,0444);
						moved_from[0]=0;
						if(!event)
							continue;
					}
					goto check_event;				
				}
				else{
					char suffix_str[256]={0},*p=NULL;
					int i=0;
						
					p=strrchr(buf,'.');
					if(p){
						len=strlen(p);
						for(i=0;i<len && i<255; i++)
							suffix_str[i]=tolower(*(p+i));
						suffix_str[i]=0;
					}	
					if(strlen(suffix_str) && CheckFileMediaType(suffix_str)!=MEDIA_TYPE_NONE){
						sprintf( moved_from, "%s%s", inotifytools_filename_from_wd( event->wd ), buf );
		#if defined DEFSQLITE3
						chmod(db_file,0644);
						sqlite_open(db_file,&db);
		#else   
						db = sqlite_open(db_file,0666, &zErr);
		#endif					
						if(db) {		
							char *pContainerStr=NULL;
							int ret=0;
							
							printf("moved_from: %s\n", moved_from);
							ret=FindContainer(moved_from, CheckFileMediaType(suffix_str), &pContainerStr);
							rc=del_record_from_db(db, moved_from,0);
							if(rc){
								mBUG("Fail to remove file '%s' from DB\n", moved_from);
							}				
							sqlite_close(db);	
							if(!ret && pContainerStr && !rc){
								//printf("%s: pContainerStr=%s\n",__FUNCTION__,pContainerStr);
								NotifyQueue(pContainerStr);
								free(pContainerStr);
								pContainerStr=NULL;
							}
						}							
						chmod(db_file,0444);	
						//Try to find IN_MOVED_TO
						event = inotifytools_next_event( timeout );
						if ( !event ) {
							moved_from[0]=0;
							continue;
						}
						else if(!(event->mask & IN_MOVED_TO)){
							moved_from[0]=0;
						}
						goto check_event;											
					}
					else
						moved_from[0]=0;
				}
			} // IN_MOVED_FROM
			else if (event->mask & IN_MOVED_TO) {
				printf("IN_MOVED_TO: %s\n", buf);//File from outside
				printf("moved_from=%s\n",moved_from);
				if (strlen(moved_from) ) {//Source and Destination files are both in watched folder.
					char new_name[1024]={0};
					int len=0;
					
					if(event->mask & IN_ISDIR){
						sprintf( new_name, "%s%s/", inotifytools_filename_from_wd( event->wd ), buf );
					}
					else{
						char *p=NULL;
						int i=0;
						
						p=strrchr(buf,'.');
						if(p){
							len=strlen(p);
							for(i=0;i<len;i++)
								new_name[i]=tolower(*(p+i));
							new_name[i]=0;
						}					
						if(strlen(new_name) && CheckFileMediaType(new_name)!=MEDIA_TYPE_NONE)					
							sprintf( new_name, "%s%s", inotifytools_filename_from_wd( event->wd ), buf );
						else
							new_name[0]=0;
					}
#ifdef __NAS_THUMBNAIL_FUN__
					if(strstr(new_name, THUMB_FOLDER))//Ignore Thumbnails.
						continue;
#endif						
					if(strstr(new_name, cache_folder_path))//Ignore files in cache folder.
						continue;
					if(strlen(new_name)){
						if(strlen(moved_from) && inotifytools_wd_from_filename(moved_from)!=-1)
							inotifytools_replace_filename( moved_from, new_name );
						len=strlen(new_name);
						if(new_name[len-1]=='/')
							new_name[len-1]=0;
						len=strlen(moved_from);
						if(moved_from[len-1]=='/')
							moved_from[len-1]=0;							
						if ( isdir(new_name)){
	#if defined DEFSQLITE3
							chmod(db_file,0644);
							sqlite_open(db_file,&db);
	#else   
							db = sqlite_open(db_file,0666, &zErr);
	#endif					
							if(db) {	
								printf("new_name=%s\n",new_name);
								rc=UpdateFolderPrefixInDB(db, moved_from, new_name);	
								if(rc)
									printf("Fail to change the name of '%s'.\n",moved_from);
								sqlite_close(db);
							}
							chmod(db_file,0444);
						}
						else{
	#if defined DEFSQLITE3
							chmod(db_file,0644);
							sqlite_open(db_file,&db);
	#else   
							db = sqlite_open(db_file,0666, &zErr);
	#endif		    	
							if(db) {														
								rc=AddOneFile2DB(db, new_name, GetFolderType(new_name, content),&media_type);
								if(!rc){
									char *pContainerStr=NULL;
									int ret=0;
									
									ret=FindContainer(new_name, CheckFileMediaType(new_name), &pContainerStr);
									if(!ret && pContainerStr){
										printf("%s: pContainerStr=%s\n",__FUNCTION__,pContainerStr);
										NotifyQueue(pContainerStr);
										free(pContainerStr);
										pContainerStr=NULL;
									}
								}
								sqlite_close(db);								
							}
							chmod(db_file,0444);
						}						
					}
				} // moved_from
				else{//Source folder/file isn't in watched folder.
					char new_name[1024]={0};
					int len=0;
					
					if(event->mask & IN_ISDIR){
						sprintf( new_name, "%s%s/", inotifytools_filename_from_wd( event->wd ), buf );
					}
					else{
						char *p=NULL;
						int i=0;
						
						p=strrchr(buf,'.');
						if(p){
							len=strlen(p);
							for(i=0;i<len;i++)
								new_name[i]=tolower(*(p+i));
							new_name[i]=0;
						}					
						if(strlen(new_name) && CheckFileMediaType(new_name)!=MEDIA_TYPE_NONE)					
							sprintf( new_name, "%s%s", inotifytools_filename_from_wd( event->wd ), buf );
						else
							new_name[0]=0;
					}
					//printf("new_name=%s\n",new_name);
#ifdef __NAS_THUMBNAIL_FUN__
					if(strstr(new_name, THUMB_FOLDER))//Ignore Thumbnails.
						continue;
#endif						
					if(strstr(new_name, cache_folder_path))//Ignore files in cache folder.
						continue;			
					len=strlen(new_name);
					if(new_name[len-1]=='/')
						new_name[len-1]=0;							
					if ( isdir(new_name)){
	#if defined DEFSQLITE3
						chmod(db_file,0644);
						sqlite_open(db_file,&db);
	#else   
						db = sqlite_open(db_file,0666, &zErr);
	#endif					
						if(db) {	
							if(insert_record_to_db(db, new_name, GetFolderType(new_name, content), 0))
								printf("Fail to create folder '%s'.\n",new_name);
							AddOneFolder2DB(db, new_name);
							sqlite_close(db);
						}
						chmod(db_file,0444);
					}
					else{
	#if defined DEFSQLITE3
						chmod(db_file,0644);
						sqlite_open(db_file,&db);
	#else   
						db = sqlite_open(db_file,0666, &zErr);
	#endif		    	
						if(db) {														
							rc=AddOneFile2DB(db, new_name, GetFolderType(new_name, content),&media_type);
							if(!rc){
								char *pContainerStr=NULL;
								int ret=0;
									
								ret=FindContainer(new_name, CheckFileMediaType(new_name), &pContainerStr);
								if(!ret && pContainerStr){
									printf("%s: pContainerStr=%s\n",__FUNCTION__,pContainerStr);
									NotifyQueue(pContainerStr);
									free(pContainerStr);
									pContainerStr=NULL;
								}
							}
							sqlite_close(db);								
						}
						chmod(db_file,0444);
					}											
				}
				moved_from[0]=0;
			}
			else if (event->mask & IN_CLOSE_WRITE) {//
					char new_name[1024]={0}, *p=NULL;
					int i=0;
					
					printf("IN_CLOSE_WRITE: %s\n", buf);
					if(!IsItemInEventList(pEventCreateList, buf)){
						continue;
					}
					DelOneItemFromEventList(&pEventCreateList, buf);
					p=strrchr(buf,'.');
					if(p){
						len=strlen(p);
						for(i=0;i<len;i++)
							new_name[i]=tolower(*(p+i));
						new_name[i]=0;
					}					
					if(strlen(new_name) && CheckFileMediaType(new_name)!=MEDIA_TYPE_NONE){
						sprintf( new_name, "%s%s", inotifytools_filename_from_wd( event->wd ), buf );
#ifdef __NAS_THUMBNAIL_FUN__
						if(strstr(new_name, THUMB_FOLDER))//Ignore Thumbnails.
							continue;
#endif							
						if(strstr(new_name, cache_folder_path))//Ignore files in cache folder.
							continue;				
	#if defined DEFSQLITE3
						chmod(db_file,0644);
						sqlite_open(db_file,&db);
	#else   
						db = sqlite_open(db_file,0666, &zErr);
	#endif		    	
						if(db) {	
							//printf("new_name=%s\n",new_name);
							rc=del_record_from_db(db, new_name, 0);
							if(!rc){		
								//printf("new_name=%s\n",new_name);
								rc=AddOneFile2DB(db, new_name, GetFolderType(new_name, content),&media_type);
								if(!rc){
									char *pContainerStr=NULL;
									int ret=0;
									
									sqlite_close(db);								
									db=NULL;
									ret=FindContainer(new_name, CheckFileMediaType(new_name), &pContainerStr);
									if(!ret && pContainerStr){
										//printf("%s: pContainerStr=%s\n",__FUNCTION__,pContainerStr);
										NotifyQueue(pContainerStr);
										free(pContainerStr);
										pContainerStr=NULL;
									}
								}
							}
							else{
								mBUG("Fail to remove file '%s' from DB\n", new_name);
							}
							if(db){
								sqlite_close(db);	
								db=NULL;
							}							
						}			
						chmod(db_file,0444);		
					}
			}
			else if (event->mask & IN_DELETE) {//
				char pFullPath[1024]={0},*p=NULL;
				int i=0;
					
				printf("IN_DELETE: %s\n", buf);
				if(!(event->mask & IN_ISDIR)){
					p=strrchr(buf,'.');
					if(p){
						len=strlen(p);
						for(i=0;i<len;i++)
							pFullPath[i]=tolower(*(p+i));
						pFullPath[i]=0;
					}		
				}			
				if(event->mask & IN_ISDIR || strlen(pFullPath)){		
					if(event->mask & IN_ISDIR || (strlen(pFullPath) && CheckFileMediaType(pFullPath)!=MEDIA_TYPE_NONE)){	
						sprintf( pFullPath, "%s%s", inotifytools_filename_from_wd( event->wd ), buf );
	#ifdef __NAS_THUMBNAIL_FUN__
						if(strstr(pFullPath, THUMB_FOLDER))//Ignore Thumbnails.
							continue;
	#endif						
						if(strstr(pFullPath, cache_folder_path))//Ignore files in cache folder.
							continue;					
						len=strlen(pFullPath);
						if(strlen(pFullPath)){
							if(event->mask & IN_ISDIR){
								if(pFullPath[len-1]!='/')
									strcat(pFullPath, "/");
								inotifytools_remove_watch_by_filename_1(pFullPath);
								len=strlen(pFullPath);
								pFullPath[len-1]=0;
							}
			#if defined DEFSQLITE3
							chmod(db_file,0644);
							sqlite_open(db_file,&db);
			#else   
							db = sqlite_open(db_file,0666, &zErr);
			#endif		    	
							if(db) {	
								char *pContainerStr=NULL;
								int ret=-1;
								
								if(event->mask & IN_ISDIR){
									rc=del_record_from_db(db, pFullPath, 1);
								}
								else{
									ret=FindContainer(pFullPath, CheckFileMediaType(pFullPath), &pContainerStr);
									rc=del_record_from_db(db, pFullPath, 0);
								}
								if(rc){													
									mBUG("Fail to remove file/folder '%s' from DB\n", pFullPath);
								}
								sqlite_close(db);	
								if(!ret && pContainerStr && !rc){
									//printf("%s: pContainerStr=%s\n",__FUNCTION__,pContainerStr);
									NotifyQueue(pContainerStr);
									free(pContainerStr);
									pContainerStr=NULL;
								}		
							}					
							chmod(db_file,0444);	
						}		
					}
				}	
			}	
			else if (event->mask & IN_DELETE_SELF) {//IN_ISDIR
				char pFullPath[1024]={0};
				
				continue;//Ignore it.
				
				if(!strlen(buf) || !strcmp(buf, "\""))
					continue;
				//printf("IN_DELETE_SELF: %s\n", buf);
				sprintf( pFullPath, "%s%s", inotifytools_filename_from_wd( event->wd ), buf );
#ifdef __NAS_THUMBNAIL_FUN__
				if(strstr(pFullPath, THUMB_FOLDER))//Ignore Thumbnails.
					continue;
#endif					
				if(strstr(pFullPath, cache_folder_path))//Ignore files in cache folder.
					continue;		
				if(strlen(pFullPath)){
	#if defined DEFSQLITE3
					chmod(db_file,0644);
					sqlite_open(db_file,&db);
	#else   
					db = sqlite_open(db_file,0666, &zErr);
	#endif		    	
					if(db) {	
						rc=del_record_from_db(db, pFullPath,1);
						if(rc){													
							mBUG("Fail to remove file/folder '%s' from DB\n", pFullPath);
						}
						sqlite_close(db);								
					}	
					chmod(db_file,0444);
				}
				//if(event->mask & IN_ISDIR)
				//	inotifytools_remove_watch_by_wd(event->wd);
			}
		} 
	}
	else{//Scan with interval
#endif	
scan_wait:
		if(scan_method!=SCAN_DISABLED){
			if(scan_interval<0)
				scan_interval=60;
			else if(scan_interval<15)
				scan_interval=15;
			while(1){
				sleep(scan_interval*60);
				use_temp_db=1;
				if(fast_scan)
					fast_scan=0;
				goto do_scan;
			}
		}
#ifdef __LINUX_INOTIFY__	
	}
#endif
	if(inotify_initialized)
		inotifytools_exit();    
    for (i = 0; i < CONTENT_NUM; i++) {
    	if(content[i].path){
    		free(content[i].path);
    		content[i].path=NULL;
    	}
    }		
    dlna_uninit (dlna);	
    return 0;   
}

int del_record_from_db(sqlite *db, char *path, int folder_type)//folder_type: 0-File, 1-Folder
{
    char *zErr = 0;
    int  rc =0;
    char *encoded_path=NULL, *encoded_name=NULL;
	int len=0;
	char buf[1024]={0};

#if 0
	{
		FILE *ff=NULL;
		
		ff=fopen("/harddisk/volume_1/data/public/del_record_from_db.txt", "at");
		if(ff){
			fprintf(ff,"path=%s\n",path);	
			fprintf(ff,"folder_type=%d\n",folder_type);
			fclose(ff);
		}	
	}
#endif	
	encoded_path=malloc(2*strlen(path)+3);
	if(encoded_path){
		memset(encoded_path, 0, 2*strlen(path)+1);
		ParseSpecialSQLChar(path, encoded_path);
	}
	else
		return -1;
	if(folder_type){
		len=strlen(encoded_path);
		if(encoded_path[len-1]=='/')
			encoded_path[len-1]=0;
		
	}
	encoded_name=strrchr(encoded_path, '/');
	if(!encoded_name)
	{
		if(encoded_path)
		{
			free(encoded_path);
			encoded_path=NULL;
		}
		return -1;	
	}
	*encoded_name++=0;
	len=strlen(encoded_path);
	if(!folder_type)
    	sprintf(buf,"DELETE FROM Data WHERE ObjectPath =='%s%s' and ObjectName = '%s'",encoded_path, encoded_path[len-1]=='/'?"":"/", encoded_name);
    else
    	sprintf(buf,"DELETE FROM Data WHERE ObjectPath = '%s%s' and ObjectName = '%s'",encoded_path, encoded_path[len-1]=='/'?"":"/", encoded_name);
#if 0
	{
		FILE *ff=NULL;
		
		ff=fopen("/harddisk/volume_1/data/public/del_record_from_db.txt", "at");
		if(ff){
			fprintf(ff,"buf=%s\n",buf);	
			fclose(ff);
		}	
	}
#endif	    
    rc = sqlite_exec(db, buf,0,0,&zErr);
    if(SQLITE_OK != rc || zErr != NULL) {
    	if(zErr){
        	sqlite_free(zErr);
     	}
     	if(encoded_path)
		{
			free(encoded_path);
			encoded_path=NULL;
		}
	    return SQLITE_ERROR;
	}
#if 0	
    sprintf(buf,"Delete From Data_Root Where Root='%s%s%s'",encoded_path, encoded_path[len-1]=='/'?"":"/", encoded_name);
	rc = sqlite_exec(db, buf, 0, 0, &zErr);
	if(SQLITE_OK != rc || zErr != NULL) {
		if(zErr){
			printf("zErr=%s: %d\n",zErr, __LINE__);
			sqlite_free(zErr);
		}		
		if(encoded_path)
		{
			free(encoded_path);
			encoded_path=NULL;
		}
		return SQLITE_ERROR;
	}    
#endif
	if(encoded_path)
		{
			free(encoded_path);
			encoded_path=NULL;
		}
    return 0;
}

int DelFolderFromDBRecursive(sqlite *db, char *path)//folder_type: 0-File, 1-Folder
{
    char *zErr = 0;
    int  rc =0;
    char *encoded_path=NULL;
	int len=0;
	char buf[1024]={0};

	encoded_path=malloc(2*strlen(path)+3);
	if(encoded_path){
		memset(encoded_path, 0, 2*strlen(path)+1);
		ParseSpecialSQLChar(path, encoded_path);
	}
	else
		return -1;
	len=strlen(encoded_path);
	if(encoded_path[len-1]=='/')
		encoded_path[len-1]=0;

	len=strlen(encoded_path);
   	sprintf(buf,"DELETE FROM Data WHERE ObjectPath = '%s%s'",encoded_path, encoded_path[len-1]=='/'?"":"/");
#if 0
	{
		FILE *ff=NULL;
		
		ff=fopen("/harddisk/volume_1/data/public/DelFolderFromDBRecursive.txt", "at");
		if(ff){
			fprintf(ff,"buf=%s\n",buf);	
			fclose(ff);
		}	
	}
#endif	    
    rc = sqlite_exec(db, buf,0,0,&zErr);
    if(SQLITE_OK != rc || zErr != NULL) {
    	if(zErr){
        	sqlite_free(zErr);
     	}
     	if(encoded_path)
		{
			free(encoded_path);
			encoded_path=NULL;
		}
	    return SQLITE_ERROR;
	}
	if(encoded_path){
		free(encoded_path);
		encoded_path=NULL;
	}
    return 0;
}

int md5_str(char *in, char *out)
{
    MD5_CTX c;
    int i;
    unsigned char d[16]={0};

    MD5Init(&c);
    MD5Update(&c, in, strlen(in));
    MD5Final(d, &c);

    memset(out, 0x00, 35);
    for (i = 0; i < 16; i++)
		sprintf(out + (i*2), "%02x", d[i]);

    return 0;
}

int insert_record_to_db(sqlite *db, char *path, int type, int fast_scan)
{
    struct dirent *rdp=NULL;
    char dir_path[1024]={0};
    char value[1024]={0};
    struct stat ds;
    int rc = 0, len=0, media_type=MEDIA_TYPE_NONE;
	DIR *dir=NULL;

#ifdef __NAS_THUMBNAIL_FUN__
	char *p=NULL;

	strcpy(dir_path, path);
	len=strlen(dir_path);
	if(dir_path[len-1]=='/')	
		dir_path[len-1]=0;	
	p=strrchr(dir_path, '/');
	if(p)
		p++;
	else
		p=path;
	if(strstr(path, THUMB_FOLDER) || !strcmp(p, THUMB_FOLDER_NAME))//Ignore Thumbnails.
		return 0;
#endif

	if(strstr(path, cache_folder_path))//Ignore files in cache folder.
		return 0;
#ifdef __LINUX_INOTIFY__
	if(inotify_initialized){
		len=strlen(path);
		if(path[len-1]!='/')
			sprintf(value, "%s/",path);
		else
			strcpy(value, path);
		if(inotifytools_wd_from_filename(value) == -1){
			//printf("Watch on '%s'.\n", path);
			rc=inotifytools_watch_recursively(path, WATCH_MASK);
			if(!rc){
				printf("Fail to watch folder '%s'\n", path);
			}
		}
	}
#endif    
    if(fast_scan)
    	return 0;
	dir=opendir(path);
	if(!dir)
		return -1;
    while ((rdp = readdir(dir))) {
    	if (!strcmp(rdp->d_name, ".") || !strcmp(rdp->d_name, ".."))
    		continue;
    	len=strlen(path);
    	if(path[len-1]=='/')
			sprintf(dir_path, "%s%s", path, rdp->d_name);
		else
			sprintf(dir_path, "%s/%s", path, rdp->d_name);
		rc=lstat(dir_path, &ds);
		if(rc)
			continue;
		if (S_ISDIR(ds.st_mode)) {//Is Directory.
			rc=AddOneFolder2DB(db, dir_path);
			if(!rc)
				insert_record_to_db(db, dir_path, type, fast_scan);		
		}
		else if(S_ISREG(ds.st_mode)){
			rc=AddOneFile2DB(db,dir_path,type, &media_type);
			if(rc==-2){
				mBUG("Fail to add file <%s> to DB\n",rdp->d_name);
			}
		}//else if ((lstat(dir_path, &ds) == 0) && strcmp(rdp->d_name, ".") && strcmp(rdp->d_name, "..")){
    }
    closedir(dir);
    return SQLITE_OK;
}

int get_media_info(char *file, struct media_file_properties *pMediaItem)
{
    struct stat filestat;
    struct tm *st;
	char *p=NULL;
	
    if(stat(file,&filestat))
    	return -1;
    memset(pMediaItem, 0, sizeof(struct media_file_properties));
    st = localtime(&filestat.st_mtime);
    sprintf(pMediaItem->year, "%d", st->tm_year+1900);
    sprintf(pMediaItem->month,"%02d", st->tm_mon+1);
    sprintf(pMediaItem->day, "%02d", st->tm_mday);
    sprintf(pMediaItem->time,"%02d:%02d:%02d", st->tm_hour,st->tm_min,st->tm_sec);  
    pMediaItem->size=filestat.st_size; 
    
	pMediaItem->title = strdup(UNKNOWN_STR);
	pMediaItem->artist = strdup(UNKNOWN_STR);
	pMediaItem->album = strdup(UNKNOWN_STR);
	pMediaItem->genre=strdup(scan_winamp_genre[WINAMP_GENRE_UNKNOWN]);
    strcpy(pMediaItem->duration,"00:00:00");    
    p=mime_get_protocol(file, pMediaItem->file_type);
    if(p){                           
		strcpy(pMediaItem->protocol_info, p);
		free(p);
		p=NULL;
	}
    return 0;
}


int check_msdb_version(char *file)
{
	FILE *fp=NULL;
	char buf[20]={0};
	
	fp = fopen(file,"rb");
	if( fp == NULL )
		return 1;
	fread(buf,1,16,fp);
	fclose(fp);
	if(strcmp(buf,DB_VSION_INFO)==0)
		return 0;
	return 1;
}
int AddOneFolder2DB(sqlite *db, char *filename)
{
    char *zErr = NULL;
    char value[1024];
    static char title[10];
    int rc = 0, len=0;
    char *encoded_path = NULL, *encoded_name = NULL;
	char container_id_str[32]={0};
	
	if(strstr(filename, cache_folder_path))//Ignore files in cache folder.
		return 0;	
	strcpy(title,"Folder");
	g_nindex = g_nindex + 1; 
	if(strlen(filename)){
		encoded_path=malloc(2*strlen(filename)+1);
		if(encoded_path){
			memset(encoded_path, 0, 2*strlen(filename)+1);
			ParseSpecialSQLChar(filename, encoded_path);
		}
	}
	else
		encoded_path=filename;
	
	len=strlen(encoded_path);
	if(encoded_path[len-1]=='/')	
		encoded_path[len-1]=0;
	encoded_name=strrchr(encoded_path, '/');
	if(!encoded_name)
	{
		if(encoded_path&& encoded_path!=filename)
		{
			free(encoded_path);
			encoded_path=NULL;
		}
		return -1;	
	}
	*encoded_name++=0;		
#ifdef __NAS_THUMBNAIL_FUN__
	if(!strcmp(encoded_name, THUMB_FOLDER_NAME))//Ignore Thumbnails.
	{
		if(encoded_path&& encoded_path!=filename)
		{
			free(encoded_path);
			encoded_path=NULL;
		}
		return -1;
	}	
#endif		
	len=strlen(encoded_path);
	GetContainerID(container_id_str);
	sprintf(value,"REPLACE INTO Data(Index_Id,Title,ObjectPath,ObjectName,ContainerID) VALUES(%llu,'%s','%s%s','%s','%s')"
		,g_nindex,title, encoded_path, encoded_path[len-1]=='/'?"":"/", encoded_name, container_id_str);
	if(encoded_path && encoded_path!=filename){
		free(encoded_path);
		encoded_path=NULL;
	}

	rc = sqlite_exec(db, value, 0, 0, &zErr);
	if(SQLITE_OK != rc || zErr != NULL) {
		if(zErr){
			printf("zErr=%s: %d\n",zErr, __LINE__);
			sqlite_free(zErr);
		}		
		return SQLITE_ERROR;
	}			
	printf("Added folder '%s'.\n",filename);

	return 0;	
}

int AddOneFile2DB(sqlite *db, char *filename, int type, int *media_type)
{
	char *p=NULL, *p0=NULL,value[1500]={0};
	int rc=0, len=0;
    static char title[10];
	struct media_file_properties media_item;
	char path[1024]={0};
    char web_path[256];
    char *zErr = NULL;
	char *encoded_genre=NULL, *encoded_album=NULL, *encoded_artist=NULL;
    char *encoded_path=NULL, *encoded_filename=NULL;
	
	p = strrchr(filename,'.');//Not media files.
	if(p==NULL)
		return -1;
	strcpy(path, filename);
	p0=strrchr(path, '/');
	if(!p0)
		p0=path;
	else{
		*p0=0;
		p0++;
	}
	memset(&media_item, 0, sizeof(media_item)); 
	if((type == ALL || type & MUSIC) && CheckFileMediaType(p)==MEDIA_TYPE_MUSIC){
		strcpy(title,"audio");
		media_item.file_type=MEDIA_TYPE_MUSIC;
		*media_type=MEDIA_TYPE_MUSIC;
	}
	else if((type == ALL || type & VIDEO) && CheckFileMediaType(p)==MEDIA_TYPE_VIDEO){
		strcpy(title,"video");
		media_item.file_type=MEDIA_TYPE_VIDEO;
		*media_type=MEDIA_TYPE_VIDEO;
	}
	else if((type == ALL || type & PICTURE)&& CheckFileMediaType(p)==MEDIA_TYPE_PHOTO){
		strcpy(title,"photo");
		media_item.file_type=MEDIA_TYPE_PHOTO;
		*media_type=MEDIA_TYPE_PHOTO;
	}
	else if((type == ALL || type & MUSIC) && CheckFileMediaType(p)==MEDIA_TYPE_PLAYLIST){
		strcpy(title,"audio");
		media_item.file_type=MEDIA_TYPE_MUSIC;
		*media_type=MEDIA_TYPE_MUSIC;
	}
	else
		return -1;
		
	md5_str(filename, value);
	sprintf(web_path, "%s/%s/%s.%s", VIRTUAL_DIR, title, value, p+1);
	if(CheckFileMediaType(filename)==MEDIA_TYPE_PLAYLIST)
		rc=get_media_info(filename, &media_item);
	else{
#ifdef _EARLY_ADD_
        if (early_add == 1){
            rc = 0;
        }
        else{
		    rc=ReadMediaFileInfo(filename, &media_item);
        }
#else
		rc=ReadMediaFileInfo(filename, &media_item);
#endif
		if(rc){
			rc=get_media_info(filename, &media_item);
			if(rc)
				return -1;		
		}
		else{
			if(media_item.file_type==MEDIA_TYPE_MUSIC){//Audio
				strcpy(title,"audio");
				*media_type=MEDIA_TYPE_MUSIC;
			}
			else if(media_item.file_type==MEDIA_TYPE_VIDEO){//Video
				strcpy(title,"video");
				*media_type=MEDIA_TYPE_VIDEO;
			}			
		}
	}
	g_nindex = g_nindex + 1;
    /* Parse Special SQL char for string value*/
    if(strlen(path)){
		encoded_path=malloc(2*strlen(path)+1);
		if(encoded_path){
			memset(encoded_path, 0, 2*strlen(path)+1);
			ParseSpecialSQLChar(path, encoded_path);
		}
		else
			encoded_path=path;
    }
    else
		encoded_path=path;
    if(strlen(p0)){
		encoded_filename=malloc(2*strlen(p0)+1);
		if(encoded_filename){
			memset(encoded_filename, 0, 2*strlen(p0)+1);
			ParseSpecialSQLChar(p0, encoded_filename);
		}
		else
			encoded_filename=p0;
    }
    else
		encoded_filename=p0;
		
	if(!media_item.artist)
		media_item.artist=strdup(UNKNOWN_STR);	
	if(strlen(media_item.artist)){
		encoded_artist=malloc(2*strlen(media_item.artist)+1);
		if(encoded_artist){
			memset(encoded_artist, 0, 2*strlen(media_item.artist)+1);
			ParseSpecialSQLChar(media_item.artist, encoded_artist);
		}
	}
	else
		encoded_artist=media_item.artist;
		
	if(!media_item.album)
		media_item.album=strdup(UNKNOWN_STR);			
	if(strlen(media_item.album)){
		encoded_album=malloc(2*strlen(media_item.album)+1);
		if(encoded_album){
			memset(encoded_album, 0, 2*strlen(media_item.album)+1);
			ParseSpecialSQLChar(media_item.album, encoded_album);
		}
	}
	else
		encoded_album=media_item.album;
		
	if(!media_item.genre)
		media_item.genre=strdup(UNKNOWN_STR);	
	if(strlen(media_item.genre)){
		encoded_genre=malloc(2*strlen(media_item.genre)+1);
		if(encoded_genre){
			memset(encoded_genre, 0, 2*strlen(media_item.genre)+1);
			ParseSpecialSQLChar(media_item.genre, encoded_genre);
		}
	}
	else
		encoded_genre=media_item.genre;
		
	len=strlen(encoded_path);
	sprintf(value,"REPLACE INTO Data(Index_Id,Title,ObjectPath,ObjectName,BuildDateY,BuildDateM,BuildDateD,BuildTime,Artist,Album,Genre,Size,WebLocation,Duration,Resolution,ProtocolInfo,PlayTimes) VALUES(%llu,'%s','%s%s','%s','%s','%s','%s','%s','%s','%s','%s','%llu','%s','%s','%s','%s',%llu)" 
	    ,g_nindex,title,encoded_path, encoded_path[len-1]=='/'?"":"/",encoded_filename,media_item.year,media_item.month,media_item.day,media_item.time, encoded_artist?encoded_artist:"",encoded_album?encoded_album:"",encoded_genre?encoded_genre:"",media_item.size,web_path,media_item.duration,media_item.resolution,media_item.protocol_info,(unsigned long long)0);
	if(encoded_path && encoded_path!=path){
		free(encoded_path);
		encoded_path=NULL;
	}
	if(encoded_filename && encoded_filename!=p0){
		free(encoded_filename);
		encoded_filename=NULL;
	}	
	if(encoded_artist!=media_item.artist){
		free(encoded_artist);
		encoded_artist=NULL;
	}
	if(encoded_album!=media_item.album){
		free(encoded_album);
		encoded_album=NULL;
	}
	if(encoded_genre!=media_item.artist){
		free(encoded_genre);
		encoded_genre=NULL;
	}
	rc = sqlite_exec(db, value, 0, 0, &zErr);
	if(SQLITE_OK != rc || zErr != NULL) {
		if(zErr){
			printf("zErr=%s: %d\n",zErr, __LINE__);
			sqlite_free(zErr);
		}		
		FreeMediaItem(&media_item);
		return -2;
	}
	if((media_item.protocol_info)&&((strstr(media_item.protocol_info,"JPEG_LRG;")!=NULL)||(strstr(media_item.protocol_info,"JPEG_MED;")!=NULL))){
		char *p=NULL,*pThumb=NULL;
		
		p=strrchr(web_path,'/');
		if(p)
			p++;
		else
			p=(char *)web_path;
		pThumb=(char*)malloc(strlen(p)+strlen(cache_folder_path)+2);
		if(!pThumb) {
			FreeMediaItem(&media_item);
			return -1;
		}
		sprintf(pThumb, "%s/%s", cache_folder_path, p);
		//printf("Create thumbnail '%s'.\n",pThumb);
		if(CreateThumb(filename,pThumb,media_item.resolution)!=0) {
			printf("Failed\n");
		}
		if(pThumb)
			free(pThumb);
	}
	FreeMediaItem(&media_item);	
	printf("Added file '%s'.\n",filename);

	return 0;
}

int ScanDB(void)
{
    char *zErr = NULL;
    int rc = 0, fd=0;
    sqlite *db=NULL;
    static char tmpbuf[1024];
    DIR *dir=NULL;
    char db_folder[256]={0},db_file[256]={0};
    char temp_db_file[256]={0};
    char buf[2048]={0};
    FILE *fp = NULL;
    struct entry content[CONTENT_NUM];
    int i = 0, len=0, j=0;

    fp = fopen(media_conf_file, "rt");
    if (fp == NULL) {
		return -1;
    }
    PRO_GetStr("main", "contentdir", buf, sizeof(buf)-1, fp);
    if (strlen(buf) == 0) {
		mBUG("contentdir is null.\n");
		fclose(fp);
		return -1;
    }
    /* setting up db file */
    PRO_GetStr("main", "dbfile", db_file, 256, fp);
    if (strlen(db_file) == 0) {
		mBUG("dbfile is null.\n");
		fclose(fp);
		return -1;
    }
    fclose(fp);
	fd=creat(DATABASE_SCANNING, O_CREAT);
	if(fd>=0)
		close(fd);
    
    if(access(db_folder, F_OK)){
    	mkdir(db_folder, 0775);
    }
    else{
    	struct stat f_stat;
    	
    	stat(db_folder, &f_stat);
    	if(!S_ISDIR(f_stat.st_mode)){
    		remove(db_folder);
    		mkdir(db_folder, 0775);
    	}
    }
    sprintf(db_file, "%s/%s", db_folder, DB_FILE_NAME);
	sprintf(temp_db_file, "%s.tmp", db_file);	   
    /* parse content dir and type */
    {
		char *p=NULL, *p2=NULL, *p3=NULL;
		char chr=0;
	
		memset(content, 0, sizeof(content));
		p = buf;
		while (p != NULL) {
		    p = strchr(p, '|');
		    if(p){
			    chr = *(p-1);
			    p++;
			    p3=strchr(p, '|');
		    }
		    else
		    	p3=NULL;
		    if(p3){
			    p2=strstr(p, ",+P|");
	 			if(p2+3!=p3)
					p2=NULL;	

				if(!p2){
					p2=strstr(p, ",+A|");
					if(p2 && p2+3!=p3)
						p2=NULL;
				}	
				if(!p2){
					p2=strstr(p, ",+V|");
					if(p2 && p2+3!=p3)
						p2=NULL;	
				}

				if(!p2){
					p2=strstr(p, ",+M|");
					if(p2 && p2+3!=p3)
						p2=NULL;	
				}						
			    if (p2) {
					*p2++ = '\0';
			    }		  
		    }  	
		    else
		    	p2=NULL;	
		    content[i].path = malloc(strlen(p)+2);
		    memset(content[i].path, 0, strlen(p)+2);
		    strcpy(content[i].path, p);
		    len=strlen(content[i].path);
		    if(content[i].path[len-1]!='/')
		    	strcat(content[i].path, "/");
		    switch (chr) {
				case 'M':
				    content[i].type = MUSIC;
				    break;
				case 'V':
				    content[i].type = VIDEO;
				    break;
				case 'P':
				    content[i].type = PICTURE;
				    break;
				case 'A':
				default:
				    content[i].type = ALL;
				    break;
		    }
		    p = p2;
		    i++;
		} /* while (p != NULL) */
    }
    for (i = 0; i < CONTENT_NUM; i++) {
    	if(!content[i].path || !strlen(content[i].path))
    		continue;
//    	printf("i content[%d].path=%s, %d\n", i, content[i].path, content[i].type);
		for (j = 0; j < CONTENT_NUM; j++) {
	    	if(!content[j].path || !strlen(content[j].path) || i==j)
	    		continue;
	    	if(!content[i].path)
	    		break;		    		
//	    	printf("j content[%d].path=%s, %d\n", j, content[j].path, content[j].type);
	    	if(!strcmp(content[j].path, content[i].path)){
	    		content[i].type |= content[j].type;	    
	    		free(content[j].path);
	    		content[j].path=NULL;
	    	}		
	    	else if(strstr(content[i].path, content[j].path)){
	    		if((content[j].type & content[i].type) == content[i].type){//Type of parent folder includes children's.
		    		free(content[i].path);
		    		content[i].path=NULL;   			
	    		}
	    		else
	    			content[i].type |= content[j].type;
	    	}
    	}	
    }
    for (i = 0; i < CONTENT_NUM; i++) {
    	if(!content[i].path || !strlen(content[i].path))
    		continue;
//    	printf("content[%d].path=%s, %d\n", i, content[i].path, content[i].type);
    }    
	remove(temp_db_file);
	printf("Write temporary DB: %s\n",temp_db_file);
#if defined DEFSQLITE3
	sqlite_open(temp_db_file,&db);
	chmod(temp_db_file,0644);
#else            	
	db = sqlite_open(temp_db_file,0666, &zErr);
#endif		    	
	if(db==NULL || zErr!=NULL) {
		if(zErr)
		    sqlite_free(zErr);
		remove(DATABASE_SCANNING);
		return -1;
	}	
	sprintf(tmpbuf, "%s","CREATE TABLE Data(Index_Id INTEGER NOT NULL,Title VARCHAR(256),ObjectPath VARCHAR(256),ObjectName VARCHAR(256)\
		    ,BuildDateY VARCHAR(4),BuildDateM VARCHAR(2),BuildDateD VARCHAR(2),BuildTime VARCHAR(8)\
			,Artist VARCHAR(256),Album VARCHAR(256),Genre VARCHAR(256), Size VARCHAR(256), WebLocation VARCHAR(256),Duration VARCHAR(12),Resolution VARCHAR(256), ProtocolInfo VARCHAR(256),PlayTimes INTEGER, ContainerID VARCHAR(32), PRIMARY KEY(ObjectPath,ObjectName))");
	rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
	if(SQLITE_OK != rc || zErr != NULL) {
		if(zErr)
		   	sqlite_free(zErr);
		sqlite_close(db);
		remove(temp_db_file);
		remove(DATABASE_SCANNING);
		return -1;
	}
	sprintf(tmpbuf,"%s", "CREATE INDEX index_Data ON Data(Index_Id);");
	rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
	if(SQLITE_OK != rc || zErr != NULL) {
		if(zErr)
		   	sqlite_free(zErr);
		sqlite_close(db);
		remove(temp_db_file);
		remove(DATABASE_SCANNING);
		return -1;
	}
#if 0	
	sprintf(tmpbuf,"%s","CREATE TABLE Data_Root(Root VARCHAR(256) PRIMARY KEY NOT NULL)");
	rc = sqlite_exec(db,tmpbuf,0,0,&zErr);
	if(SQLITE_OK != rc || zErr != NULL) {
		if(zErr)
		    sqlite_free(zErr);
		sqlite_close(db);
		remove(temp_db_file);
		remove(DATABASE_SCANNING);
		return -1;
	}
#endif	
    for (i = 0; i < CONTENT_NUM; i++) {
    	if(!content[i].path || !strlen(content[i].path))
    		continue;
		if ((dir = opendir(content[i].path)) == NULL)  {
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    for (i = 0; i < CONTENT_NUM; i++) {
		    	if(content[i].path){
		    		free(content[i].path);
		    		content[i].path=NULL;
		    	}
		    }		    
		    return -1;
		}
#if 0
	    sprintf(tmpbuf,"INSERT INTO Data_Root(Root) VALUES('%s')", encoded_path);
	    if(encoded_path && encoded_path!=content[i].path){
	    	free(encoded_path);
	    	encoded_path=NULL;
	    }		
		rc = sqlite_exec(db, tmpbuf, 0, 0, &zErr);
		if(SQLITE_OK != rc || zErr != NULL) {
			if(zErr)
		    	sqlite_free(zErr);
		    closedir(dir);
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    for (i = 0; i < CONTENT_NUM; i++) {
		    	if(content[i].path){
		    		free(content[i].path);
		    		content[i].path=NULL;
		    	}
		    }			    
		    return -1;
		}
#endif		
		closedir(dir);
		if(insert_record_to_db(db, content[i].path, content[i].type, 0)) {
		    sqlite_close(db);
		    remove(temp_db_file);
		    remove(DATABASE_SCANNING);
		    for (i = 0; i < CONTENT_NUM; i++) {
		    	if(content[i].path){
		    		free(content[i].path);
		    		content[i].path=NULL;
		    	}
		    }
		    return -1;
		}
    }
	sqlite_close(db);
	remove(db_file);
	chmod(temp_db_file,0444);
	rc=rename(temp_db_file, db_file);
	if(!rc)
		printf("Use temporary DB: %s\n",temp_db_file);
	else
		printf("Fail to use temporary DB: %s\n",temp_db_file);		
	chmod(db_file,0444);	
	remove(DATABASE_SCANNING);
	for (i = 0; i < CONTENT_NUM; i++) {
		if(content[i].path){
			free(content[i].path);
			content[i].path=NULL;
		}
	}		
	return rc;
}	



