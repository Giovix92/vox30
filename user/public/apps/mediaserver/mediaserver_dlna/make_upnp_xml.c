#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "upnpd.h"
#include "mmsio.h"
#include "tool.h"
#include "if_info.h"
#include "sysdep.h"
#include "uuid.h"

// begin to add
char *strupr(char *str)
{
    char *string = str;

    if (str) {
        for ( ; *str; ++str)
            *str = toupper(*str);
    }
    return string;
}

void mac_str_to_array(char *array,char *str)
{
	char sa[6][20];
	//printf("orig_str=%s \n",str);
	sscanf(str,"%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%s",sa[0],sa[1],sa[2],sa[3],
		sa[4],sa[5]);
	array[0]=strtol(sa[0],NULL,16);
	array[1]=strtol(sa[1],NULL,16);
	array[2]=strtol(sa[2],NULL,16);
	array[3]=strtol(sa[3],NULL,16);
	array[4]=strtol(sa[4],NULL,16);
	array[5]=strtol(sa[5],NULL,16);	
	array[6]='\0';
}

int get_lan_mac(char *buf)
{
    if_info_t  if_info;
    FILE *fp = NULL;

    bzero(&if_info,sizeof(if_info_t));
    fp = fopen(MEDIA_CONF, "rt");
    if (fp == NULL)
		return -1;
    PRO_GetStr("main", "laninterface", if_info.ifname, 32, fp);
    if(getIFInfo(&if_info)==0){
        mac_str_to_array(buf,strupr(if_info.mac));
	 	return 0;
    }
    return -1;
}

#if 0
/*
	init uPnp device uuid v1
*/
char *init_dev_uuid(void)
{
    char uuid_str[40];
    char buf[200]="\0";
    int i = 0;
    uuid_upnp uuid;

    // get lan_uuid
    if (get_lan_mac(buf)==0) {
		uuid_create2(&uuid,buf);
    }
    else
		uuid_create(&uuid);
    uuid_unpack(&uuid,uuid_str);

    while (uuid_str[i] != '\0') {
	printf("%c->", uuid_str[i]);
	uuid_str[i] = tolower(uuid_str[i++]);
	printf("%c, ", uuid_str[i-1]);
    }
    printf("\n");

    PRO_SetStr("main", "uuid", uuid_str, MEDIA_CONF);

    return strdup(uuid_str);
}
#endif

int make_upnp_xml(char *pResourceFolder, struct MediaEnv *media) 
{
    char upnp_port[10]={0}, device_port[10]={0};
	char des_file_template[256]={0}, des_file[256]={0};
    sprintf(upnp_port, "%d", media->upnp_port);
	sprintf(device_port, "%d", media->device_port);
	
	if(strlen(pResourceFolder)> sizeof(des_file)-24)
		return -1;
	sprintf(des_file_template, "%s/mediaserver.mod", pResourceFolder);
	sprintf(des_file, "%s/mediaserver.xml", pResourceFolder);
    /* mediaserver.xml */
    substr(des_file_template, des_file, "@IPADDR#",media->InternalIPAddress);
    substr(des_file, des_file, "@UPNP_PORT#",upnp_port);
    substr(des_file, des_file, "@DEVICE_PORT#",device_port);
    substr(des_file, des_file, "@UUID#",media->uuid);
    substr(des_file,des_file,"@FRIENDLY_NAME#",media->friendly_name);
    substr(des_file,des_file,"@MANUFACTURER#",media->manufacturer);
    substr(des_file, des_file, "@MANUFACTURER_URL#",media->manufacturer_url);
    substr(des_file, des_file, "@MODEL_DESCRIPTION#",media->model_description);
    substr(des_file, des_file, "@MODEL_NAME#",media->model_name);
    substr(des_file, des_file, "@MODEL_NUMBER#",media->model_number);
    substr(des_file, des_file, "@SERIAL_NUMBER#",media->serial_number);
    
    /* mediaserver_wmc.xml - for windows media player 11 on vista */
	sprintf(des_file_template, "%s/mediaserver_wmc.mod", pResourceFolder);
	sprintf(des_file, "%s/mediaserver_wmc.xml", pResourceFolder);      
    substr(des_file_template, des_file, "@IPADDR#",media->InternalIPAddress);
    substr(des_file, des_file, "@UPNP_PORT#",upnp_port);
    substr(des_file, des_file, "@DEVICE_PORT#",device_port);
    substr(des_file, des_file, "@UUID#",media->uuid);
    substr(des_file,des_file,"@FRIENDLY_NAME#",media->friendly_name);
    substr(des_file,des_file,"@MANUFACTURER#",media->manufacturer);
    substr(des_file, des_file, "@MANUFACTURER_URL#",media->manufacturer_url);
    substr(des_file, des_file, "@MODEL_DESCRIPTION#",media->model_description);
    substr(des_file, des_file, "@MODEL_NAME#",media->model_name);
    substr(des_file, des_file, "@MODEL_NUMBER#",media->model_number);
    substr(des_file, des_file, "@SERIAL_NUMBER#",media->serial_number);
    
    
    /* mediaserver_xbox.mod */
	sprintf(des_file_template, "%s/mediaserver_xbox.mod", pResourceFolder);
	sprintf(des_file, "%s/mediaserver_xbox.xml", pResourceFolder);      
    substr(des_file_template, des_file, "@FRIENDLY_NAME#",media->friendly_name);
    substr(des_file,des_file,"@MANUFACTURER#",media->manufacturer);
    substr(des_file, des_file, "@MANUFACTURER_URL#",media->manufacturer_url);
    substr(des_file, des_file, "@MODEL_NAME#",media->model_name);
    substr(des_file, des_file, "@MODEL_NUMBER#",media->model_number);
    substr(des_file, des_file, "@UUID#",media->uuid);
    substr(des_file, des_file, "@MODEL_DESCRIPTION#",media->model_description);
    substr(des_file, des_file, "@SERIAL_NUMBER#",media->serial_number);
    
    
    /* msr.mod */
	sprintf(des_file_template, "%s/msr.mod", pResourceFolder);
	sprintf(des_file, "%s/msr.xml", pResourceFolder);     
    substr(des_file_template, des_file, "@UUID#",media->uuid);

    return 0;
}
// end of add
