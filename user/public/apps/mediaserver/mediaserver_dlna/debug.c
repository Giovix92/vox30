#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <linux/limits.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "upnpd.h"
#include "tool.h"
#include "http.h"
#include "mmsio.h"
#include "mediaserver.h"
#define	MAX_KEY_LEN			128
#define	MAX_SECTION_LEN		64
char db_file_path[256]={0};
char cache_folder_path[256]={0};
char resource_path[256]={0},protocolinfo_file[256]={0};
struct MediaEnv media;
char codepage[10]={0};
playlist *g_playlist=NULL;
int custom_ttl;
int advertisement_expire=5;
int media_pipe[2];
struct entry content_folders[CONTENT_NUM];
int most_played_num=DEF_LAST_MOST_PLAYED_COUNT;
int last_played_num=DEF_LAST_MOST_PLAYED_COUNT;
int GetLineFromFile(char **lbuf,FILE *fp)
{
	int	c,sz=128;
	char *p,*buf;

	buf=(char *) malloc(sz);
	if(!buf){
		*lbuf=NULL;
		return(-1);
	}
	p=buf;
	while ((c=fgetc(fp)) != '\n' && c!=EOF) {
		if (p-buf ==sz-2) {
			buf=(char *) realloc(buf, sz*2);
			if(!buf){
				*lbuf=NULL;
				return 1;
			}
			p=buf+sz-2;
			sz*=2;
		}
		*p++=c;
	}
	if(c=='\n')
	     *p++=c;
	if (c==EOF &&p==buf) {
		free(buf);
		*lbuf=NULL;
		return(-1);
	}
	*p++='\0';
	*lbuf=buf;
	return(0);
}
int PRO_GetStr(char *sect, char *key, char *val, int size, FILE * fp) 
{
    char *buf;
    char item[MAX_KEY_LEN + 2], section[MAX_SECTION_LEN + 3];
    int find_sect = 0, len = 0, len1 = 0;

    if (val == NULL || fp == NULL || key == NULL)
		return -1;
    if (sect != NULL) {
		if (sect[0] != '[' || sect[strlen(sect) - 1] != ']')
		    sprintf(section, "[%s]", sect);
		else
		    strcpy(section, sect);
		rewind(fp);
		len = strlen(section);
		while (!GetLineFromFile(&buf, fp)) {
		    if (!strncmp(buf, section, len)) {
			find_sect = 1;
			free(buf);
			break;
		    }
		    free(buf);
		}
    }
    else
		find_sect = 0;
    if (!find_sect)
		rewind(fp);
    sprintf(item, "%s=", key);
    len = strlen(item);
    while (!GetLineFromFile(&buf, fp)) {
		if (buf[0] == '[') {
		    free(buf);
		    break;		// end of section
		}
		if (!strncmp(buf, item, len)) {
		    len1 = strlen(buf);
		    len1--;
		    while (buf[len1] == 0x0d || buf[len1] == 0x0a)
			buf[len1--] = '\0';
		    if (strlen(buf + len) > size)
			buf[len + size] = 0;
		    strcpy(val, buf + len);
		    free(buf);
		    return 0;
		}
		free(buf);
    }
    return -1;
}
int GetGIFResolution(char *pFileName)
{
	int fd=0, len=0, width=0, height=0;
	char buf[11]={0};
	
	fd=open(pFileName, O_RDONLY);
	if(fd<=0)
		return -1;
	len=read(fd, buf, 10);
	close(fd);
	if(len!=10)
		return -1;
	width=(buf[7]<<8) + buf[6];
	height=(buf[9]<<8) + buf[8];
	printf("width=%d\n", width);
	printf("height=%d\n", height);
	if(!width || !height)
		return -1;
	return 0;
}	

int GetBMPResolution(char *pFileName)
{
	int fd=0, len=0, width=0, height=0;
	char buf[27]={0};
	
	fd=open(pFileName, O_RDONLY);
	if(fd<=0)
		return -1;
	len=read(fd, buf, 26);
	close(fd);
	if(len!=26)
		return -1;
		width=(buf[21]<<24) + (buf[20]<<16) + (buf[19]<<8) + buf[18];
	height=(buf[25]<<24) + (buf[24]<<16) + (buf[23]<<8) + buf[22];	
	printf("width=%d\n", width);
	printf("height=%d\n", height);
	if(!width || !height)
		return -1;
	return 0;
}		

int main(int argc, char** argv)
{
    char *zErr = NULL;
    int rc = 0;
    sqlite *db=NULL;
    int new_width=640, new_height=480;
    int width, height;
    
#if 0
	if(argc<2)
		return -1;
    if(!rc)
		rc=CreateThumb(argv[1], "tn.jpg", new_width, new_height);
		
	return rc;		    
    width=atoi(argv[1]);
    height=atoi(argv[2]);
    rc=GetScaledPictureResolution(width, height, &new_width, &new_height);
    printf("rc=%d\n",rc);
    printf("new_width=%d\n",new_width);
    printf("new_height=%d\n",new_height);
 
	
    return 0;

    if(argc<2)
    	return -1;
    if(strstr(argv[1], "gif"))
    	GetGIFResolution(argv[1]);
    else if(strstr(argv[1], "bmp"))
    	GetBMPResolution(argv[1]);
    return 0;	
#endif    
	strcpy(cache_folder_path, "/tmp");
	sqlite_open(argv[1],&db);
	if(db==NULL||zErr!=NULL){	
		printf("Fail to open the database.\n");
		return -1;
	}
	rc=DumpDataBase(db);
	
	//rc=GetSubDirEntries(db, argv[2], MEDIA_TYPE_MUSIC);
	if(rc){
		printf("Failed.\n");	
	}
	sqlite_close(db);
	return 0;
}


