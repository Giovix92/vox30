#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define NVRAM_SIZE 10000      //MAX of nvram file;
#define NVRAM_TMP_PATH "/tmp/nvram"
char* value_parser_file(char *name)
{
    char *s;
    static char sp[128];
    char buf[NVRAM_SIZE];
    int fd;
    unsigned int file_size;
    
    *sp = '\0';
    
    if((fd=open(NVRAM_TMP_PATH, O_RDONLY))<0)
    {
        //printf("Open file error!\n");
        return "";				
    }
    memset(buf,0,sizeof(buf));
    file_size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    
    //printf("file zise:%d",file_size);
    if (file_size>NVRAM_SIZE)
    {
        //printf("To value_parser_file function,the file size of nvram is big than the buf size!\n");
        close(fd);
        return "";
    }
    if(read(fd,buf,file_size)<0)
    {
        //printf("Read file nvram error!\n");
        close(fd);
        return "";
    }
    close(fd);
    s = buf;
    
	while(*s) 
	{
		if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') 
		{
		    int val_len = strlen(s)-strlen(name);
		    
		    if(val_len <= 128)
		    {
			    memcpy(sp, (s+strlen(name)+1), val_len);					
			}
			return sp;	
		}
		while(*s++);
	}
	return "";
}
