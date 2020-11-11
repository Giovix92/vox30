/**
 * @file   
 * @author 
 * @date   2011-08-11
 * @brief  
 *
 * Copyright - 2011 SerComm Corporation. All Rights Reserved.
 * SerComm Corporation reserves the right to make changes to this document without notice.
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm Corporation specifisally
 * disclaims any and all liability, including without limitation consequential or incidental damages;
 * neither does it convey any license under its patent rights, nor the rights of others.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/file.h>
#include <stdarg.h>
#include <utility.h>
#include "nvram.h"
#include "nvram_lock.h"

/* line terminator by 0x00 
 * data terminator by two 0x00
 * value separaed by 0x01
 */	
#define NVRAM_TMP_PATH "/tmp/nvram"		  /* ex:  /tmp/nvram     */
#define NVRAM_BCM_PATH "/tmp/nvram.bcm"		  /* ex:  /tmp/nvram     */
#define END_SYMBOL	        0x00		  	
#define DIVISION_SYMBOL	    0x01		  

/* NVRAM_HEADER MAGIC*/ 
#define NVRAM_MAGIC 		0x004E4F52		 /* RON */

/* used 12bytes, 28bytes reserved */
#define NVRAM_HEADER_SIZE   40       		 
/* max size in flash*/
#define NVRAM_SIZE          65535		  /* nvram size 64k bytes*/

/* each line max size*/
#define NVRAM_BUFF_SIZE           4096		 

/* errorno */
#define NVRAM_SUCCESS       	    0
#define NVRAM_FLASH_ERR           1 
#define NVRAM_MAGIC_ERR	    2
#define NVRAM_LEN_ERR	    3
#define NVRAM_CRC_ERR	    4
#define NVRAM_SHADOW_ERR	    5



#define RETRY_TIMES 3
#define BASE_PATH "/tmp/nv/"
#define BASE_PATH_LEN 8  // strlen(BASE_PATH) 

static struct nv_entry *nv_list=NULL;

struct nv_entry *find_nv_entry(const char *name , const char *path)
{
	struct nv_entry *p=nv_list;


	if(!name || !p || !path)
		return NULL;

	while(p){
		if( strcmp(p->name,name)==0 && strcmp(p->path,path)==0)
			break;
		p=p->next;		
	}
	return p;
}
int add_nv_entry(const char *name,char *value , char * path)
{
	struct nv_entry *p;

	if(!value)
		return -1;

	p=find_nv_entry(name, path);

	if(!name || ! path){
		free(value);
		return -1;
	}
	// exist
	if(p){
		if(p->value)
			free(p->value);
		p->value = value;
	}else{
		struct nv_entry *new_entry;
		new_entry = malloc(sizeof(struct nv_entry));
		if(!new_entry){
			free(value);
			return -1;
		}
		new_entry->name = strdup(name);
		if(!new_entry->name){
			free(value);
			free(new_entry);
			return -1;
		}

		new_entry->path = strdup(path);
		if( !new_entry->path){
			free(value);
			free(new_entry->name);
			free(new_entry);
			return -1;
		}


		new_entry->value = value;
		new_entry->next=nv_list;
		nv_list = new_entry;
	}
	return 0;	
}

static int readBackupFileBin_unlock(char *path, char **data) {
	int total;
	int fd=0;
	if((fd=open(path, O_RDONLY)) < 0)
		return -1;
	total=lseek(fd,0,SEEK_END);
	lseek(fd,0,0);
	
	if((*data=malloc(total))==NULL){
		close(fd);
		return -1;
	}
	if(read(fd,*data,total)<0){ 
		free(*data);
		close(fd);
		return -1;
	}
	close(fd);
   	return total;
}
static int readFileBin_unlock(char *path, char **data) {
	int total;
	int fd=0;
    char path_bak[128];
    *data = NULL;
	if((fd=open(path, O_RDONLY)) < 0)
		goto read_backup;
	total=lseek(fd,0,SEEK_END);
	lseek(fd,0,0);
	
	if((*data=malloc(total))==NULL){
		close(fd);
		return -1;
	}
	if(read(fd,*data,total)<0){
        close(fd); 
		goto read_backup;
	}
	close(fd);
#if 1
    if(total < 2 || !(*data) || *((*data)+total-1) != END_SYMBOL)  //file corruption
    {
read_backup:
        if(*data)
        {
            free(*data);
            *data = NULL;
        }
        snprintf(path_bak, sizeof(path_bak), "%s.bak", path);
        total = readBackupFileBin_unlock(path_bak, data);
    }
#endif
   	return total;
}
static ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

static ssize_t full_read(int fd, void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_read(fd, buf, len);

		if (cc < 0)
			return cc;	/* read() returns -1 on failure. */

		if (cc == 0)
			break;

		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
	}

	return total;
}
static ssize_t safe_write(int fd, const void *buf, size_t count)
{
	ssize_t n;

	do {
    		lseek(fd, 0, SEEK_SET);
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}
int readBackupFileBin(char *path, char **data)
{
    int total;
    int fd = -1;
    int times = 0;
    struct flock flockptr = {.l_type = F_RDLCK,
                             .l_start = 0,
                             .l_whence = SEEK_SET,
                             .l_len = 0
    };

    *data = NULL;

    if ((fd = open(path, O_RDONLY)) < 0) {
        return -1;
    }
add_rlock:
    if(++times > RETRY_TIMES )
    {
         close(fd);
         return -1;
    }
    if (fcntl(fd, F_SETLKW, &flockptr) < 0)
    {
        if(errno == EINTR)
            goto add_rlock;
    }

    total = lseek(fd, 0, SEEK_END);

    /**
     * read zero is meaningful
     * Denny Zhang 2011年 06月 07日 星期二 11:40:05 CST
     */

    //if(total <= 0)
    if(total < 0)
    {
        flockptr.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW,&flockptr);
        close(fd);
        return -1;
    }
	
    lseek(fd, 0, 0);
    
    if ((*data = malloc(total)) == NULL) {
        flockptr.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW,&flockptr);
        close(fd);
        return -1;
    }

    if (full_read(fd, *data, total) != total) { 
        free(*data);
        *data = NULL;
        flockptr.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW,&flockptr);
        close(fd);
        return -1;
    }
    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &flockptr);    
    close(fd);
    
    return total;
}
int readFileBin(char *path, char **data)
{
    int total;
    int fd = -1;
    int times = 0;
    char path_bak[128];
    struct flock flockptr = {.l_type = F_RDLCK,
                             .l_start = 0,
                             .l_whence = SEEK_SET,
                             .l_len = 0
    };

    *data = NULL;

    if ((fd = open(path, O_RDONLY)) < 0) {
         goto read_backup;
    }
add_rlock:
    if(++times > RETRY_TIMES )
    {
         close(fd);
         goto read_backup;
    }
    if (fcntl(fd, F_SETLKW, &flockptr) < 0)
    {
        if(errno == EINTR)
            goto add_rlock;
    }

    total = lseek(fd, 0, SEEK_END);

    /**
     * read zero is meaningful
     * Denny Zhang 2011年 06月 07日 星期二 11:40:05 CST
     */

    //if(total <= 0)
    if(total < 0)
    {
        flockptr.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW,&flockptr);
        close(fd);
        goto read_backup;
    }
	
    lseek(fd, 0, 0);
    
    if ((*data = malloc(total)) == NULL) {
        flockptr.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW,&flockptr);
        close(fd);
        return -1;
    }

    if (full_read(fd, *data, total) != total) { 
        free(*data);
        *data = NULL;
        flockptr.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW,&flockptr);
        close(fd);
        goto read_backup;
    }
    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &flockptr);    
    close(fd);
#if 1 
    if(total < 2 || !(*data) || *((*data)+total-1) != END_SYMBOL)  //file corruption
    {
read_backup:
        if(*data)
        {
            free(*data);
            *data = NULL;
        }
        snprintf(path_bak, sizeof(path_bak), "%s.bak", path);
        total = readBackupFileBin(path_bak, data);
    }
#endif
    return total;
}
void writeBackupFileBin(char *path, char *data, int len)
{
    int fd;
    int ret = -1;
    int times = 0;
    struct flock flockptr = {.l_type = F_WRLCK,
                             .l_start = 0, 
                             .l_whence = SEEK_SET,
                             .l_len = 0   
    };
                           
    if ((fd = open(path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR)) < 0)
        return;

add_wlock:
    if(++times > RETRY_TIMES )
    {
        close(fd);
        return;
    }
    if (fcntl(fd, F_SETLKW, &flockptr) < 0)
    {
        perror("fcntl:");
        if(errno == EINTR)
            goto add_wlock;
    }

    if (ftruncate(fd, 0) < 0)
        perror("truncate file error:");

    ret = safe_write(fd, data, len);
    if (ret != len)
    {
        perror("WriteBackupFileBin write Error");
    }
    
    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW,&flockptr);    

    close(fd);
}

void writeFileBin(char *path, char *data, int len)
{
    int fd;
    int ret = -1;
    int times = 0;
    char path_bak[128];
    struct flock flockptr = {.l_type = F_WRLCK,
                             .l_start = 0, 
                             .l_whence = SEEK_SET,
                             .l_len = 0   
    };
                           
    if ((fd = open(path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR)) < 0)
        return;

add_wlock:
    if(++times > RETRY_TIMES )
    {
        close(fd);
        return;
    }
    if (fcntl(fd, F_SETLKW, &flockptr) < 0)
    {
        perror("fcntl:");
        if(errno == EINTR)
            goto add_wlock;
    }

    if (ftruncate(fd, 0) < 0)
        perror("truncate file error:");

    ret = safe_write(fd, data, len);
    if (ret != len)
    {
        perror("WriteFileBin write Error");
    }
    
    flockptr.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW,&flockptr);    

    close(fd);
    snprintf(path_bak, sizeof(path_bak), "%s.bak", path);
    writeBackupFileBin(path_bak, data, len);

}

extern char* nvram_get_fun(const char *name,char *path)
{
	char *bufspace = NULL;
	int size;
	char *s = NULL, *sp = NULL;
	
	if((size=readFileBin(path, &bufspace))<0) 
		return NULL;

	for (s = bufspace; *s; s++) {
		if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') {
			sp= calloc(1, (strlen(s)-strlen(name) + 1));
			if(sp == NULL)
			{
				free(bufspace);
				return NULL;
			}
			memcpy(sp,(s+strlen(name)+1),(strlen(s)-strlen(name)));
			free(bufspace);
			return sp;
		}
		while(*(++s));
	}
	free(bufspace);
	return NULL;
}
extern int nvram_set_p(char *path, const char* name,const char* value)
{
	char *bufspace, *targetspace;
	int size;
	char *sp, *s;
	int found=0;
		
	if((size = readFileBin(path, &bufspace)) >= 0) 
    {
	    targetspace = malloc(size+strlen(name)+strlen(value)+4);
	}
	else 
        {
        	
	    targetspace=malloc(strlen(name)+strlen(value)+4);

	}

	sp=targetspace;
	if(size > 0) {
	   for (s = bufspace; *s ; s++) {
		if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') {
			found=1;
  			strcpy(sp, name);
			sp+=strlen(name);
        		*(sp++) = '=';
       			strcpy(sp, value);
			sp+=strlen(value);		
			while (*(++s));
		}
		while(*s) *(sp++)=*(s++);
	        *(sp++)=END_SYMBOL;
	    }
	
       if(bufspace)
       {
           free(bufspace);
           bufspace = NULL;
       }
	}
	if(!found){
		strcpy(sp, name);
		sp+=strlen(name);
        	*(sp++) = '=';
	        strcpy(sp, value);
		sp+=strlen(value);
	        *(sp++) = END_SYMBOL;
	}
        
	*(sp) = END_SYMBOL;

	writeFileBin(path, targetspace, (sp-targetspace)+1);
	free(targetspace);
	
	return NVRAM_SUCCESS;
}
static void writeBackupFileBin_unlock(char *path, char *data, int len) {
   int fd;

	if((fd=open(path, O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR)) < 0)
   		return;
	write(fd, data, len);
	close(fd);
}
static void writeFileBin_unlock(char *path, char *data, int len) {
   int fd;
    char path_bak[128];
	if((fd=open(path, O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR)) < 0)
   		return;
	write(fd, data, len);
	close(fd);
    snprintf(path_bak, sizeof(path_bak), "%s.bak", path);
    writeBackupFileBin_unlock(path_bak, data, len);
}
static char* nvram_get_func_unlock(const char *name,char *path)
{
	char *bufspace;
	int size;
	char *s,*sp;
	
	if((size=readFileBin_unlock(path, &bufspace))<0) 
		return NULL;

	for (s = bufspace; *s ; s++) {
		if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') {
			sp=malloc(strlen(s)-strlen(name));
			memcpy(sp,(s+strlen(name)+1),(strlen(s)-strlen(name)));
			free(bufspace);

			add_nv_entry(name,sp, & path[BASE_PATH_LEN]);

			return sp;
		}
		while(*(++s));
	}
	free(bufspace);
	return NULL;
}

char* nvram_get_func(const char *name,char *path)
{
	char *value = NULL;
	int lock = -1;

	lock = nvram_lock(path);
    if(lock == 0)
    {
	    value = nvram_get_func_unlock(name, path);
	    lock = nvram_unlock(path);
    }
	return value;
}

static int nvram_unset_func_unlock(const char* name,char *path)
{
	char *bufspace, *targetspace;
	int size;
	char *sp, *s;
	int found=0;

	if((size=readFileBin_unlock(path, &bufspace))>0)
	    targetspace=malloc(size);
	else
		return NVRAM_SUCCESS;
		
	sp=targetspace;
	if(size > 0) {
		for (s = bufspace; *s ; s++) {
			if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') {
				found=1;
				while (*(++s));
			}
			else{
				while(*s) *(sp++)=*(s++);
			    	*(sp++)=END_SYMBOL;
			}
		}

		free(bufspace);
	}
	if(!found){
		free(targetspace);
		return NVRAM_SUCCESS;
	}
	
	*(sp) = END_SYMBOL;
	
	writeFileBin_unlock(path, targetspace, (sp-targetspace)+1);
	
	free(targetspace);

	return NVRAM_SUCCESS;
}

int nvram_unset_func(const char* name,char *path)
{
	int lock;
	int err;

	lock = nvram_lock(path);
    if(lock == 0)
    {
	    err = nvram_unset_func_unlock(name, path);
	    lock = nvram_unlock(path);
    }
	return err;
}

static int nvram_set_func_unlock(const char* name,const char* value,char *path)
{
	char *bufspace, *targetspace;
	int size;
	char *sp, *s;
	int found=0;

	if((size=readFileBin_unlock(path, &bufspace))>0) {
	    targetspace=malloc(size+strlen(name)+strlen(value)+4);
	}
	else {
	    targetspace=malloc(strlen(name)+strlen(value)+4);
	}

	sp=targetspace;
	if(size > 0) {
	   for (s = bufspace; *s ; s++) {
		if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') {
			found=1;
  			strcpy(sp, name);
			sp+=strlen(name);
        		*(sp++) = '=';
       			strcpy(sp, value);
			sp+=strlen(value);		
			while (*(++s));
		}
		while(*s) *(sp++)=*(s++);
	        *(sp++)=END_SYMBOL;
	    }
	
		free(bufspace);
	}
	if(!found){
		strcpy(sp, name);
		sp+=strlen(name);
        	*(sp++) = '=';
	        strcpy(sp, value);
		sp+=strlen(value);
	        *(sp++) = END_SYMBOL;
	}

	*(sp) = END_SYMBOL;

	writeFileBin_unlock(path, targetspace, (sp-targetspace)+1);
	free(targetspace);

	return NVRAM_SUCCESS;
}

int nvram_set_func(const char* name,const char* value,char *path)
{
	int lock = -1;
	int err;
	
	lock = nvram_lock(path);
    if(lock == 0)
    {
	    err = nvram_set_func_unlock(name, value, path);
	    lock = nvram_unlock(path);
    }
	return err;
}
int nvram_set_unlock(const char* name,const char* value)
{
	char *bufspace, *targetspace;
	int size;
	char *sp, *s;
	int found=0;

	if((size=readFileBin_unlock(NVRAM_BCM_PATH, &bufspace))>0) {
	    targetspace=malloc(size+strlen(name)+strlen(value)+4);
	}
	else {
	    targetspace=malloc(strlen(name)+strlen(value)+4);
	}

	sp=targetspace;
	if(size > 0) {
	   for (s = bufspace; *s ; s++) {
		if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') {
			found=1;
  			strcpy(sp, name);
			sp+=strlen(name);
        		*(sp++) = '=';
       			strcpy(sp, value);
			sp+=strlen(value);		
			while (*(++s));
		}
		while(*s) *(sp++)=*(s++);
	        *(sp++)=END_SYMBOL;
	    }
	
		free(bufspace);
	}
	if(!found){
		strcpy(sp, name);
		sp+=strlen(name);
        	*(sp++) = '=';
	        strcpy(sp, value);
		sp+=strlen(value);
	        *(sp++) = END_SYMBOL;
	}
        
	*(sp) = END_SYMBOL;

	writeFileBin_unlock(NVRAM_BCM_PATH, targetspace, (sp-targetspace)+1);
	free(targetspace);

	return NVRAM_SUCCESS;
}


int nvram_set(const char* name,const char* value)
{
	int lock = -1;

	lock = nvram_lock(NVRAM_BCM_PATH);	
    if(lock == 0)
    {
	    nvram_set_unlock(name, value);
	    lock = nvram_unlock(NVRAM_BCM_PATH);
    }   
	return NVRAM_SUCCESS;
}
#ifdef CONFIG_SUPPORT_WIFI_5G
int nvram_bcm_set_x(const char* value,const char* name, ...)
{
    char buf[512]="";
    va_list arg;
    va_start(arg, name);
    vsnprintf(buf, 512, name, arg);
    va_end(arg);
	return nvram_set_func(buf, value, NVRAM_BCM_PATH);	
}
#endif
int nvram_bcm_set(const char* name,const char* value)
{
	return nvram_set_func(name, value, NVRAM_BCM_PATH);	
}
int nvram_bcm_unset(const char* name)
{
	return nvram_unset_func(name,NVRAM_BCM_PATH);
}
int nvram_unset(const char* name)
{
	return nvram_unset_func(name,NVRAM_BCM_PATH);
}
char *nvram_bcm_get(const char *name)
{
    return nvram_get_func(name, NVRAM_BCM_PATH);
}
char *nvram_bcm_safe_get(const char *name)
{
    char *value = NULL;
    value = nvram_get_func(name, NVRAM_BCM_PATH);
    return value ? value : "";
}

char* nvram_get(const char *name)
{	
	char *pt;

	if((pt=nvram_get_func(name,NVRAM_BCM_PATH))==NULL){
				return NULL;
	}

		
	// [bcm_nvram] s
	if ( pt && !strncmp( pt, "*DEL*", 5 ) ) // check if this is the deleted var
	{
		pt = NULL;
	}
	// [bcm_nvram] e

	
	return pt;
}

