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
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <jpeglib.h>
#include <setjmp.h>
#include "libdlna.h"

int parse_media_files (int argc, char **argv);
int main (int argc, char **argv)
{
  dlna_t *dlna;
  dlna_profile_t *p;
  dlna_org_flags_t flags;
  dlna_item_t *item;
  int dlna_compliant=0;
  
	if (argc < 2) {
	    printf ("usage: %s media_filename\n", argv[0]);
	    return -1;
	}
	printf("%s: %d\n", __FUNCTION__, __LINE__);
	if(argc>2 && !strcmp(argv[2], "folder")){
		printf("%s: %d\n", __FUNCTION__, __LINE__);
		parse_media_files(argc, argv);
		return 0;
	}

  flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
    DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
    DLNA_ORG_FLAG_CONNECTION_STALL |
    DLNA_ORG_FLAG_DLNA_V15;
  
  printf ("Using %s\n", LIBDLNA_IDENT);
  
  dlna = dlna_init ();
  dlna_set_verbosity (dlna, DLNA_MSG_INFO);
  dlna_set_extension_check(dlna, 1);
  dlna_register_all_media_profiles (dlna);

  item = dlna_item_new (dlna, argv[1],&dlna_compliant);
  if (item)
  {
    if (item->properties)
    {
      printf ("Size: %lld bytes\n", item->properties->size);
      printf ("Duration: %s\n", item->properties->duration);
      printf ("Bitrate: %d bytes/sec\n", item->properties->bitrate);
      printf ("SampleFrequency: %d Hz\n", item->properties->sample_frequency);
      printf ("BitsPerSample: %d\n", item->properties->bps);
      printf ("Channels: %d\n", item->properties->channels);
      printf ("Resolution: %s\n", item->properties->resolution);
    }

    if (item->metadata)
    {
      printf ("Title: %s\n", item->metadata->title);
      printf ("Artist: %s\n", item->metadata->author);
      printf ("Description: %s\n", item->metadata->comment);
      printf ("Album: %s\n", item->metadata->album);
      printf ("Track: %d\n", item->metadata->track);
      printf ("Genre: %s\n", item->metadata->genre);
    }
    //dlna_item_free (item);
  }
  else{
	dlna_uninit (dlna);
  	printf ("Unknown format\n");
	return -1; 
  
  }
  	
  //p = dlna_guess_media_profile (NULL, dlna, argv[1]);
  p=item->profile;
  if (p)
  {
    char *protocol_info;
    
    printf ("ID: %s\n", p->id);
    printf ("MIME: %s\n", p->mime);
    printf ("Label: %s\n", p->label);
    printf ("Class: %d\n", p->media_class);
    printf ("UPnP Object Item: %s\n", dlna_profile_upnp_object_item (p));

    protocol_info = dlna_write_protocol_info (DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                              DLNA_ORG_PLAY_SPEED_NORMAL,
                                              DLNA_ORG_CONVERSION_NONE,
                                              DLNA_ORG_OPERATION_RANGE,
                                              flags, p);
    printf ("Protocol Info: %s.\n", protocol_info);
    free (protocol_info);
  }
  else
    printf ("Unknown format\n");
  dlna_item_free (item);
  dlna_uninit (dlna);
  #if 0
  {
  	char pResolution[32]={0};
  	get_jpeg_resolution(argv[1], pResolution);
  	printf("%s: %s\n", argv[1],pResolution);
  }
  #endif
  return 0;
}

int parse_media_files (int argc, char **argv)
{
	dlna_t *dlna;
	dlna_profile_t *p;
	dlna_org_flags_t flags;
	dlna_item_t *item;
	int dlna_compliant=0;
	DIR *dir=NULL;
	struct dirent *dirp=NULL;
	struct stat f_stat;
	char tmp_file[512]={0};	

	flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
	    DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
	    DLNA_ORG_FLAG_CONNECTION_STALL |
	    DLNA_ORG_FLAG_DLNA_V15;
	  
	  printf ("Using %s\n", LIBDLNA_IDENT);
  
	dlna = dlna_init ();
	dlna_set_verbosity (dlna, DLNA_MSG_INFO);
	dlna_set_extension_check(dlna, 1);
	dlna_register_all_media_profiles (dlna);
	dir=opendir(argv[1]);
	system("date >>/tmp/debug.txt");
	if(dir!=NULL){
		while( (dirp=readdir(dir)) !=NULL ){
			if(!strcmp(dirp->d_name,".")||!strcmp(dirp->d_name,".."))
				continue;
			sprintf(tmp_file,"%s/%s",argv[1],dirp->d_name);	
			if(lstat(tmp_file, &f_stat))
				continue;
			if (!S_ISREG(f_stat.st_mode))
				continue;
			item = dlna_item_new (dlna, tmp_file,&dlna_compliant);
			if (item) {
			    if (item->properties) {
			      printf ("Size: %lld bytes\n", item->properties->size);
			      printf ("Duration: %s\n", item->properties->duration);
			      printf ("Bitrate: %d bytes/sec\n", item->properties->bitrate);
			      printf ("SampleFrequency: %d Hz\n", item->properties->sample_frequency);
			      printf ("BitsPerSample: %d\n", item->properties->bps);
			      printf ("Channels: %d\n", item->properties->channels);
			      printf ("Resolution: %s\n", item->properties->resolution);
			    }
		
			    if (item->metadata) {
			      printf ("Title: %s\n", item->metadata->title);
			      printf ("Artist: %s\n", item->metadata->author);
			      printf ("Description: %s\n", item->metadata->comment);
			      printf ("Album: %s\n", item->metadata->album);
			      printf ("Track: %d\n", item->metadata->track);
			      printf ("Genre: %s\n", item->metadata->genre);
			    }
			}
			else{
				{
					FILE *ff=NULL;
					
					ff=fopen("/tmp/debug.txt","at");
					fprintf(ff,"%s:%s:%d\n", dirp->d_name,"Unknown format",__LINE__);	
					fclose(ff);
				}				
			  	printf ("Unknown format\n");
				continue;
			}
		  	
			p=item->profile;
			if (p) {
				char *protocol_info;
		    
			    printf ("ID: %s\n", p->id);
			    printf ("MIME: %s\n", p->mime);
			    printf ("Label: %s\n", p->label);
			    printf ("Class: %d\n", p->media_class);
			    printf ("UPnP Object Item: %s\n", dlna_profile_upnp_object_item (p));
		
				protocol_info = dlna_write_protocol_info (DLNA_PROTOCOL_INFO_TYPE_HTTP,
		                                              DLNA_ORG_PLAY_SPEED_NORMAL,
		                                              DLNA_ORG_CONVERSION_NONE,
		                                              DLNA_ORG_OPERATION_RANGE,
		                                              flags, p);
				printf ("Protocol Info: %s.\n", protocol_info);
				free (protocol_info);
				{
					FILE *ff=NULL;
					
					ff=fopen("/tmp/debug.txt","at");
					fprintf(ff,"%s:%s\n", dirp->d_name,p->id);	
					fclose(ff);
				}
			}
			else{
				{
					FILE *ff=NULL;
					
					ff=fopen("/tmp/debug.txt","at");
					fprintf(ff,"%s:%s:%d\n", dirp->d_name,"Unknown format",__LINE__);	
					fclose(ff);
				}				
		    	printf ("Unknown format\n");
			}		   
			dlna_item_free (item);
			
		}
		closedir(dir);
	}
	dlna_uninit (dlna);
	
	system("date >>/tmp/debug.txt");
	return 0;
}
struct my_error_mgr {                                               
	struct jpeg_error_mgr pub;  /* "public" fields      */          
	jmp_buf setjmp_buffer;      /* for return to caller */          
};    
typedef struct my_error_mgr * my_error_ptr;                         

static void my_error_exit(j_common_ptr cinfo)         
{                                                     
	my_error_ptr myerr=(my_error_ptr) cinfo->err;     
	char buf[JMSG_LENGTH_MAX];                        
	(*cinfo->err->format_message)(cinfo,buf);         
	longjmp(myerr->setjmp_buffer, 1);                 
}   

int get_jpeg_resolution(const char *file,char *resolution)
{
	struct jpeg_decompress_struct cinfo;
	FILE *infile; 
	struct my_error_mgr jerr;
	
	if (NULL == (infile=fopen(file,"rb"))) {                          
		return -1;                                                    
	}                                                                 
	cinfo.err = jpeg_std_error(&jerr.pub);                            
	jerr.pub.error_exit = my_error_exit;                              
	if (setjmp(jerr.setjmp_buffer)) {                                 
		jpeg_destroy_decompress(&cinfo);                              
		fclose(infile);                                               
		return -1;                                                    
	}                                                  
	printf("%s: %d\n", __FUNCTION__, __LINE__);            	                                                  
	jpeg_create_decompress(&cinfo);      
	jpeg_stdio_src(&cinfo,infile);                                                    
	jpeg_read_header(&cinfo,TRUE);    
	printf("%s: %d\n", __FUNCTION__, __LINE__);                                      
	memset(resolution,0,sizeof(resolution));   
	printf("%s: %d\n", __FUNCTION__, __LINE__);                               
	sprintf(resolution,"%dx%d",cinfo.image_width,cinfo.image_height); 
	printf("%s: %d\n", __FUNCTION__, __LINE__);        
	jpeg_destroy_decompress(&cinfo);    
	fclose(infile);                     
	return 0;
}

