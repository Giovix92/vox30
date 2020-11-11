/* author: kylin 
* e-mail: kylin_kang@sdc.sercomm.com 
* tn.c
* 2007å¹?7æœ?7æ—?æ˜ŸæœŸäº?13æ—?3åˆ?6ç§?*/

#include <stdio.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <strings.h>
#include <unistd.h>
#include <jpeglib.h>
#include <errno.h>
#include "pic_scale.h"

#ifndef errno
extern int errno; 
#endif

#if !HAVE_STRRCHR && HAVE_RINDEX
#define strrchr rindex
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#include <sys/stat.h>

#define MAXSIZE     2048                    /* Maximum size of thumbnail   */
#define MINSIZE     1                       /* Minimum size of thumbnail   */
#define DEFAULTSIZE 128                     /* Default size of thumbnail   */

#define GRAY 128

#define RSZ_AUTO   0
#define RSZ_WIDTH  1
#define RSZ_HEIGHT 2
extern char resource_path[256];
int GetPicScale(char *resolution);
static int file_exists(const char *path); 
enum file_type{
	TYPE_NONE,
	TYPE_GIF,
	TYPE_JPEG,
	TYPE_PNG
};

int CreateJPEGThumbFromFile(char *in_file, const char *out_file, char *resolution); 
int lock_flag=0;

int get_file_type (char *file_name)
{
	int type = TYPE_NONE;
	char *p=NULL;
	
	p=strrchr(file_name, '.');
	if(!p || p==file_name+strlen(file_name)-1)
		return type;
	if (!strcasecmp(".jpg",p))
		type = TYPE_JPEG;
	else if (!strcasecmp(".png",p))
		type = TYPE_PNG;
	else if (!strcasecmp(".gif",p))
		type = TYPE_GIF;
	return type;
}


int IsImageFile(char *pFileName)
{
	int file_type=TYPE_NONE;

	if(!pFileName || !strlen(pFileName))
		return 0;
	file_type=get_file_type(pFileName);
	if(file_type==TYPE_JPEG || file_type==TYPE_GIF || file_type==TYPE_PNG)
		return 1;
		
	return 0;
}
#define	WAIT_TIME	(60*60)
int CreateThumb(char *pFileName, char *pThumbName, char* resolution)
{
	int file_type=TYPE_NONE;
	int ret=0, num=0;
	
	if(!pFileName || !strlen(pFileName))
		return -1;
	if(!lock_flag)
		lock_flag=1;
	else{
		while(lock_flag){
			sleep(2);
			num++;
			if(num>=WAIT_TIME/2){
				break;
			}
		}
	}
	file_type=get_file_type(pFileName);
	
	if(file_type==TYPE_JPEG)
		ret=CreateJPEGThumbFromFile(pFileName, pThumbName, resolution);
	//else if(file_type==TYPE_GIF)
	//	ret=CreateGIFThumbFromFile(pFileName, pThumbName, THUMBNAIL_SIZE);
	//else if(file_type==TYPE_PNG)
	//	ret=CreateGIFThumbFromFile(pFileName, pThumbName, PNG_SMALL_SIZE);		
	else{
		lock_flag=0;
		return -1;	
	}
	if(!ret)
		chmod(pThumbName, 0664);
	lock_flag=0;
	return ret;
}

/*the assign total scale is 1/16-1, d_scale for djpeg,c_scale for cjpeg*/
int AssignScale(int scale,char *d_scale,char *c_scale)
{
	if(scale<=0)
		return -1;
	switch(scale){
		case 1:
			strcpy(c_scale,"1/1");
			strcpy(d_scale,"1/1");
			break;
		case 2:
			strcpy(c_scale,"1/2");
			strcpy(d_scale,"1/1");
			break;
		case 3:
			strcpy(c_scale,"8/9");
			strcpy(d_scale,"3/8");
			break;
		case 4:
			strcpy(c_scale,"8/16");
			strcpy(d_scale,"4/8");
			break;
		case 5:
			strcpy(c_scale,"8/15");
			strcpy(d_scale,"3/8");
			break;
		case 6:
			strcpy(c_scale,"8/12");
			strcpy(d_scale,"2/8");
			break;
		case 7:
			strcpy(c_scale,"8/14");
			strcpy(d_scale,"2/8");
			 break;
		case 8:
			strcpy(c_scale,"8/16");
			strcpy(d_scale,"2/8");
			break;
		case 9:
			strcpy(c_scale,"8/9");
			strcpy(d_scale,"1/8");
			break;
		case 10:
			strcpy(c_scale,"8/10");
			strcpy(d_scale,"1/8");
			break;		
		case 11:
			strcpy(c_scale,"8/11");
			strcpy(d_scale,"1/8");
			break;
		case 12:
			strcpy(c_scale,"8/12");
			strcpy(d_scale,"1/8");
			break;
		case 13:
			strcpy(c_scale,"8/13");
			strcpy(d_scale,"1/8");
			break;
		case 14:
			strcpy(c_scale,"8/14");
			strcpy(d_scale,"1/8");
			break;
		case 15:
			strcpy(c_scale,"8/15");
			strcpy(d_scale,"1/8");
			break;
		case 16:
			strcpy(c_scale,"8/16");
			strcpy(d_scale,"1/8");
			break;
		default:
			strcpy(c_scale,"1/2");
			strcpy(d_scale,"1/8");
			break;
	}
	return 0;
}
int CreateJPEGThumbFromFile(char *in_file, const char *out_file, char *resolution) {
	int ret,scale=-1;
	char cmdbuf[1024]={0}, tmp_file[256]={0};
	char *imgfile= NULL;
	char d_scale[20],c_scale[20];
	imgfile = (char *)out_file;
	if (file_exists(imgfile)) {
		fprintf(stderr, "Skipping: %s: File exists\n", imgfile);
		return 0;
	}
	scale=GetPicScale(resolution);
	if(scale<0)
		 return -1;
	if(scale==0)
	{
		snprintf(cmdbuf, sizeof(cmdbuf), "/bin/cp -af '%s' '%s' 2>/dev/null", in_file, out_file);
		ret=system(cmdbuf);
		if(ret)
			return -1;
		return 0;
	}
	ret=AssignScale(scale,d_scale,c_scale);
	if(ret)
		return -1;
#if 1		
	sprintf(tmp_file, "%s.tmp",out_file);
	sprintf(cmdbuf, "%s/djpeg -scale %s '%s' > %s 2>/dev/null", resource_path, d_scale, in_file, tmp_file);
	ret=system(cmdbuf);
	if(ret){
		remove(tmp_file);
		remove(out_file);
		return -1;
	}	
	sprintf(cmdbuf, "%s/cjpeg -scale %s '%s' '%s' 2>/dev/null",resource_path, c_scale,tmp_file,out_file);
#else
	snprintf(cmdbuf, sizeof(cmdbuf),"%s/djpeg -scale %s '%s' | %s/cjpeg -scale %s >'%s' 2>/dev/null",resource_path,d_scale,in_file,resource_path,c_scale,out_file);
#endif	
	//printf("cmdbuf==%s\n",cmdbuf);
	ret=system(cmdbuf);
	remove(tmp_file);
	
	if(ret)
		 return -1;
	return 0;
	
}

static int file_exists(const char *path) { 
    struct stat buf;
 
    /* Crude check. If we can stat it successfully, we assume it exists. */
    /* Otherwise we return zero, meaning that this function can't tell. */

    if (0 == stat(path, &buf)) {
        return(1);
    } else {
        return(0);
    }
}
int GetScaledPictureResolution(int org_width, int org_height, int *new_width, int *new_height)  
{
	float scale_f,width_f,height_f;
	int scale_i;
	
	scale_f=(((float)org_width/JPEG_SMALL_WIDTH)>=((float)org_height/JPEG_SMALL_HEIGHT))?((float)org_width/JPEG_SMALL_WIDTH):((float)org_height/JPEG_SMALL_HEIGHT);
	scale_i=(int)scale_f;
	if(scale_f!=scale_i)
		scale_i+=1;
	width_f=(float)org_width/(float)scale_i;
	height_f=(float)org_height/(float)scale_i;
	if((width_f>*new_width)||(height_f> *new_height))
		return -1;
	*new_width=(int)width_f;
	if(*new_width!=width_f)
		*new_width+=1;
	*new_height=(int)height_f;
	if(*new_height!=height_f)
		*new_height+=1;
	return 0;
}
/*get the reduce scale of the large and middle pic */
int GetPicScale(char *resolution)
{
	int width,higth,scale_i;
	float scale_w,scale_h,scale_f;
	char *p=NULL;
	if(!resolution)
		return -1;
	p=strchr(resolution, 'x');
	if(!p){
		return -1;
	}		

	*p=0;
	p++;
	width=atoi(resolution);
	higth=atoi(p);
	/* If picture is too small,  than don't need scale ,just copy it to thumbnail */
	if(width<=JPEG_SMALL_WIDTH ||higth<=JPEG_SMALL_HEIGHT)
	{
		return 0;
	}
	scale_w=(float)width/JPEG_SMALL_WIDTH;
	scale_h=(float)higth/JPEG_SMALL_HEIGHT;
	scale_f=(scale_w>=scale_h)?scale_w:scale_h;
	scale_i=(int)scale_f;
	if(scale_f!=scale_i)
		scale_i+=1;
	return scale_i;
}


