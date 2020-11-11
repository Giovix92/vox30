#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <linux/limits.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>    	
#include <sys/ioctl.h>
#include <dirent.h>
#include "upnpd.h"
#include "tool.h"
#include "http.h"
#include "mmsio.h"
#include "mediaserver.h"
#define DEFAULT_UUID "898f9738-d930-1001-94da"
#define	SERVER_VERSION	"1.04"

extern int MediaDeviceStateTableInit(char*,char*);
extern int substr(char *infile, char *outfile, char *str_from, char *str_to);
extern void mBUG(char *format, ...);
void FreeContainerList(void);

char db_file_path[256]={0};
char cache_folder_path[256]={0};
char resource_path[256]={0},protocolinfo_file[256]={0};
#ifdef __ICONV_SUPPORT__
char codepage[10]={0};
#endif
struct MediaEnv media;

playlist *g_playlist=NULL;
int custom_ttl;
int advertisement_expire=5;
int media_pipe[2];
struct entry content_folders[CONTENT_NUM];
int most_played_num=DEF_LAST_MOST_PLAYED_COUNT;
int last_played_num=DEF_LAST_MOST_PLAYED_COUNT;

extern int MediaDeviceCallbackEventHandler(Upnp_EventType, void*, void*);

#define	MAX_KEY_LEN			128
#define	MAX_SECTION_LEN		64

int DelFile(char *path)
{
	struct stat path_stat;

	if (stat(path, &path_stat) < 0) {
		return -1;
	}

	if (S_ISDIR(path_stat.st_mode)) {
		DIR *dp;
		struct dirent *d;
		int ret = 0;

		if ((dp = opendir(path)) == NULL) {
			return -1;
		}
		while ((d = readdir(dp)) != NULL) {
			char new_path[512]={0};

			if (strcmp(d->d_name, ".") == 0 ||strcmp(d->d_name, "..") == 0)
				continue;
			sprintf(new_path,"%s/%s",path, d->d_name);
			ret=DelFile(new_path);
		}
		closedir(dp);
		return ret;
	}
	else {
		if (remove(path) < 0) {
			return -1;
		}
	}
	return 0;
}
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
void sig_usr1(int sig)
{
#if 0	
	char buf[118000]={0};
	int ret=0;
	
	ret=read(media_pipe[0], buf, sizeof(buf)-1);
	if(ret>0){
		printf("String: %s\n",buf);	
		AddNotify(buf);
	}
#else
	char buf[256]={0}, *buf1=NULL, *p=NULL;
	int num=0;
	
	num=256;
	buf1=malloc(num);
	if(!buf1)
		return;
	memset(buf1, 0, 256);
	fcntl(media_pipe[0], F_SETFL, O_NONBLOCK);
	while(read(media_pipe[0], buf, sizeof(buf)-1) > 0){
		if(strlen(buf1)+strlen(buf) >= num){
			num=strlen(buf1)+strlen(buf)+256;
			p=realloc(buf1, num);
			if(!p){
				free(buf1);
				buf1=NULL;
				return;
			}
			strcat(buf1, buf);
		}
		else
			strcat(buf1, buf);
	}
	printf("String: %s\n",buf1);	
	AddNotify(buf1);
	free(buf1);
	buf1=NULL;
#endif	
	return;
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

void *handleExpire()
{
	return NULL;
}

int ReadContentFolders(char *pPathString, struct entry *pContentFolder)
{
	int i = 0, len=0, j=0;
	char *p=NULL, *p2=NULL, *p3=NULL;
	char chr=0;
	
	p = pPathString;
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
		pContentFolder[i].path = malloc(strlen(p)+2);
		memset(pContentFolder[i].path, 0, strlen(p)+2);
		strcpy(pContentFolder[i].path, p);
		len=strlen(pContentFolder[i].path);
		if(pContentFolder[i].path[len-1]!='/')
			strcat(pContentFolder[i].path, "/");
		switch (chr) {
			case 'M':
				pContentFolder[i].type = MUSIC;
				break;
			case 'V':
				pContentFolder[i].type = VIDEO;
				break;
			case 'P':
				pContentFolder[i].type = PICTURE;
				break;
			case 'A':
			default:
				pContentFolder[i].type = ALL;
				break;
		}
		p = p2;
		i++;
	} /* while (p != NULL) */
    for (i = 0; i < CONTENT_NUM; i++) {
    	if(!pContentFolder[i].path || !strlen(pContentFolder[i].path))
    		continue;
		for (j = 0; j < CONTENT_NUM; j++) {
	    	if(!pContentFolder[j].path || !strlen(pContentFolder[j].path) || i==j)
	    		continue;
	    	if(!pContentFolder[i].path)
	    		break;	    		
	    	if(!strcmp(pContentFolder[j].path, pContentFolder[i].path)){
	    		pContentFolder[i].type |= pContentFolder[j].type;	   
	    		free(pContentFolder[j].path);
	    		pContentFolder[j].path=NULL;
	    		pContentFolder[j].type=0;
	    		
	    	}		
	    	else if(strstr(pContentFolder[i].path, pContentFolder[j].path)){
	    		if((pContentFolder[j].type & pContentFolder[i].type) == pContentFolder[i].type){//Type of parent folder includes children's.
		    		free(pContentFolder[i].path);
		    		pContentFolder[i].path=NULL;   		
		    		pContentFolder[i].type=0;	
	    		}
	    		else
	    			pContentFolder[i].type |= pContentFolder[j].type;
	    	}
    	}	
    }
    return 0;
}   
   

extern int make_upnp_xml(char *pResourceFolder, struct MediaEnv *media) ;
extern char *init_dev_uuid(void);
#define	MEDIA_SERVER_PID	"/var/run/media_server.pid"
/*
 * get mac address by interface name.
 * param: MacAddr point to a string buf.
 */
static int getMacAddrByIf(IN char *ifname, OUT char *MacAddr)
{
	unsigned char *phwaddr = NULL;
	struct ifreq ifr;
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket");
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		perror("ioctl");
		return -1;
	}

	phwaddr = (unsigned char *) ifr.ifr_hwaddr.sa_data;

	sprintf(MacAddr, "%02x%02x%02x%02x%02x%02x", (phwaddr[0] & 0xff),
			(phwaddr[1] & 0xff), (phwaddr[2] & 0xff), (phwaddr[3] & 0xff),
			(phwaddr[4] & 0xff), (phwaddr[5] & 0xff));

	if (sock)
		close (sock);
	return 0;
}

static char * create_udn(char *nic)
{
	char *pUDN=NULL;
	char macaddr[13]="";


	if (!nic)
		return NULL;

	/* determine UDN according to MAC address */
	if(getMacAddrByIf(nic,macaddr))
	{
		return NULL;
	}

	pUDN = (char *) malloc (64 * sizeof (char));
	bzero(pUDN,64);


	snprintf (pUDN, 64, "%s-%s", DEFAULT_UUID,macaddr);

	return pUDN;
}

void ReloadIP(int sig)
{
    int ret=0;
    char fullurl[256],device_port[6]={0};
	FILE *fp=NULL;
	
    ret=getIPAddress(media.ifname, media.InternalIPAddress);
    if(ret){
		return;
    }
    sprintf(fullurl, "http://%s:%d/%s", media.InternalIPAddress, media.upnp_port, DESC_FILE);
    UpnpSetServerIpAddress(media.InternalIPAddress);
    UpnpSetHndDescUrl(media.device_handle, fullurl);
    fp = fopen(MEDIA_CONF, "rt");
    if (fp){
	    PRO_GetStr("main", "device_port", device_port, 5, fp); // device's HTTP port
	    if(atoi(device_port)==80 || atoi(device_port)>=1024)
	    	media.device_port=atoi(device_port);
	    fclose(fp);    
    }    
	make_upnp_xml(resource_path, &media);
	return;
}

int do_exit=0;
int handle_hup=0;

void do_sig_hup(int sig)
{
	mBUG("Received Signal: %d\n", sig);
	if(!handle_hup)
		handle_hup=1;
	else
		return;
	ReloadIP(sig);
	handle_hup=0;
}

void do_sig_term(int sig)
{
	do_exit=1;	
    printf("Shutting down on signal %d...\n", sig);
}

void usage(char *pFileName)
{
	printf("%s: [-C <configuraiton File>] [-h]\n", pFileName);
}

int main(int argc, char** argv)
{
    int ret=0,i=0;
    char fullurl[256]={0}, playlist_num[8]={0};
#ifdef __ICONV_SUPPORT__
    char code_str[8]="437";
#endif
    char desc_doc_url[256]={0},*p=NULL, upnp_port[6]={0},expire_str[8]={0};
    struct sigaction act;
    pthread_t pid;
    FILE *fp = NULL;
    int t = 0, argn=1;
	pid_t scan_pid=0;
	char db_folder[200]={0}, conf_file[128]={MEDIA_CONF}, buf[2048]={0},device_port[6]="80";
	int fast_scan=0;
//	int fd=0;
#ifdef SC_MUTIL_GROUP
	struct in_addr anyaddr;
#endif /* SC_MUTIL_GROUP */
    while (argn < argc && argv[argn][0] == '-') {
		if (strcmp(argv[argn], "-C") == 0 && argn + 1 < argc) {
	    	++argn;
            if(strlen(argv[argn])<sizeof(conf_file))
            	strcpy(conf_file, argv[argn]);	    	
	    }	
	    else if(strcmp(argv[argn], "-h") == 0) {
            usage(argv[0]);
            exit (0); 	    	
	    }
	    else if(strcmp(argv[argn], "-F") == 0) {
            fast_scan=1;//Don't scan the database during server starts.
	    }
	    ++argn;
	}	   
#if 0	
	fd=open("/dev/ttyS0", O_RDWR);
	if(fd>=0){
		dup2(fd, 1);
		dup2(fd, 2);
	}		
#endif		
	printf("DLNA Media Server %s.\n",SERVER_VERSION);
	if(argv[0][0]=='/'){//Absolute Path
		if(strlen(argv[0])>=sizeof (resource_path) - 24)
			return -1;
		strcpy(resource_path, argv[0]);
		p=strrchr(resource_path, '/');
		if(p)
			*p=0;
	}
	else{
		getcwd (resource_path, sizeof (resource_path) - 24);
/*		if(!resource_path)
		{
			return -1;
		}*/
		if(strlen(argv[0]) + strlen(resource_path) >= sizeof (resource_path) - 24)
		{
			return -1;
		}
		strcat(resource_path, "/");
		strcat(resource_path, argv[0]);
		p=strrchr(resource_path, '/');
		if(p)
			*p=0;		
	}
	
	strcat(resource_path, "/resource");
	sprintf(protocolinfo_file, "%s/%s",resource_path,PROTOCOL_INFO);
    memset(&media,0,sizeof(struct MediaEnv));
	/* Variable initialize */
    fp = fopen(conf_file, "rt");
    if (fp == NULL) {
		mBUG("open %s failed", MEDIA_CONF);
		return -1;
    }
    PRO_GetStr("main", "friendlyname", media.friendly_name, 64, fp);
    PRO_GetStr("main", "manufacturer", media.manufacturer, 64, fp);
    PRO_GetStr("main", "manufacturerurl", media.manufacturer_url, 256, fp);
    PRO_GetStr("main", "modeldescription", media.model_description, 128, fp);
    PRO_GetStr("main", "modelname", media.model_name, 32, fp);
    PRO_GetStr("main", "modelnumber", media.model_number, 64, fp);
    PRO_GetStr("main", "serialnumber", media.serial_number, 64, fp);
    PRO_GetStr("main", "dbfile", db_folder, 200, fp);
    PRO_GetStr("main", "contentdir", buf, sizeof(buf)-1, fp);
    sprintf(db_file_path, "%s/%s", db_folder, DB_FILE_NAME);
    sprintf(cache_folder_path, "%s/%s", db_folder, MS_CACHE_FOLDER);
#ifdef __ICONV_SUPPORT__
    PRO_GetStr("main", "codepage", code_str, sizeof(code_str)-1,fp);	
    sprintf(codepage,"CP%s",code_str);
#endif
    if(access(db_folder, F_OK)){
    	ret=mkdir(db_folder, 0775);
    }
    else{
    	struct stat f_stat;
    	
    	stat(db_folder, &f_stat);
    	if(!S_ISDIR(f_stat.st_mode)){
    		remove(db_folder);
    		mkdir(db_folder, 0775);
    	}
    }		
    if(access(cache_folder_path, F_OK)){
    	ret=mkdir(cache_folder_path, 0775);
    }
    else{
    	struct stat f_stat;
    	
    	stat(cache_folder_path, &f_stat);
    	if(!S_ISDIR(f_stat.st_mode)){
    		remove(cache_folder_path);
    		mkdir(cache_folder_path, 0775);
    	}
    }	
	
    PRO_GetStr("main", "upnpport", upnp_port, 5, fp); // http port
    media.upnp_port=atoi(upnp_port);
    PRO_GetStr("main", "uuid", media.uuid, 40, fp); // media uuid
    PRO_GetStr("main", "laninterface", media.ifname, 32, fp); // lan interface
	PRO_GetStr("main", "device_port", device_port, 5, fp); // device's HTTP port
	if(atoi(device_port)==80 || atoi(device_port)>=1024)
		media.device_port=atoi(device_port);   
    ret=PRO_GetStr("main", "playlist_num", playlist_num, 5, fp); // http port
    if(!ret && strlen(playlist_num) && atoi(playlist_num)>0)
    	last_played_num=most_played_num=atoi(playlist_num);
    //strcpy(media.uuid, create_udn(media.ifname));// XXX:Memory Leak
    p=create_udn(media.ifname);
    if(p){
    	strcpy(media.uuid, p);
    	free(p);
    }
    if (strlen(media.ifname) == 0) {
		printf("%s: LAN interface is not set.\n", argv[0]);
		fclose(fp);
		return -1;
    }
#ifndef SC_MUTIL_GROUP
    ret=getIPAddress(media.ifname, media.InternalIPAddress);
#else /* SC_MUTIL_GROUP */
    anyaddr.s_addr = htonl(INADDR_ANY);
    strcpy(media.InternalIPAddress,inet_ntoa(anyaddr));
    ret = 0;
#endif /* SC_MUTIL_GROUP */
    if(ret){
		fclose(fp);
		return -1;
    }
    PRO_GetStr("main", "advrexpire", expire_str, 7, fp);
    t=atoi(expire_str);
    media.advr_expire = t*60;
    fclose(fp);
    ReadContentFolders(buf, (struct entry *)&content_folders);
 	fp=fopen(MEDIA_SERVER_PID, "wt");
	if(fp){
		fprintf(fp,"%d",getpid());
		fclose(fp);
	}      

#ifdef SC_MUTIL_GROUP
	if ((ret = UpnpInit(NULL, media.upnp_port)) != UPNP_E_SUCCESS)
#else
	if ((ret = UpnpInit(media.InternalIPAddress, media.upnp_port)) != UPNP_E_SUCCESS)
#endif /* SC_MUTIL_GROUP */
	{
		printf("Error with UpnpInit -- %d\n", ret);
		UpnpFinish();
		remove(MEDIA_SERVER_PID);
		exit(1);
    }
    printf("UPnP Initialized\n");
    // DLNA request to support at least 20 KB.
    if (UpnpSetMaxContentLength(20*1024) != UPNP_E_SUCCESS) {
		printf("Could not set Max content UPnP\n");
		UpnpFinish();
		remove(MEDIA_SERVER_PID);
		exit(1);
    }

    if (media.upnp_port==0)
        media.upnp_port = UpnpGetServerPort();

    if (make_upnp_xml(resource_path, &media)<0) {
		printf("Error with formatting upnp description xml  \n");
		remove(MEDIA_SERVER_PID);
		exit(1);
	}
	//strcpy(media.InternalIPAddress,"192.168.3.1"); /* TODO:Jacky debug */
    sprintf(fullurl, "http://%s:%d/%s", media.InternalIPAddress, media.upnp_port, DESC_FILE);
    sprintf(desc_doc_url, "http://%s:%d",media.InternalIPAddress , media.upnp_port);
    
    printf("Intialized UPnP \n\twith fullurl=%s\n\t", fullurl);
    printf("\t     UPNP_DIR=%s\n", resource_path);
    printf("\t     desc_doc_url=%s\n", desc_doc_url);
    g_playlist=list_init(g_playlist);
    UpnpEnableWebserver(TRUE);

    ret = UpnpSetVirtualDirCallbacks(&virtual_dir_callbacks);
    if (ret != UPNP_E_SUCCESS) {
		printf("Cannot set virtual directory callbacks\n");
		UpnpFinish();
		remove(MEDIA_SERVER_PID);
		exit(1);
    }

    ret = UpnpAddVirtualDir (VIRTUAL_DIR);
    if (ret != UPNP_E_SUCCESS) {
		printf("Cannot add virtual directory for web server\n");
		UpnpFinish();
		remove(MEDIA_SERVER_PID);
		exit(1);
    }

	printf("Specifying the webserver root directory -- %s\n", resource_path);
    if ((ret = UpnpSetWebServerRootDir(resource_path)) != UPNP_E_SUCCESS) {
		printf("Error specifying webserver root directory -- %s: %d\n", resource_path, ret);
		UpnpFinish();
		remove(MEDIA_SERVER_PID);
		exit(1);
    }

    printf("Registering the RootDevice\n");

    if ((ret = UpnpRegisterRootDevice(fullurl, MediaDeviceCallbackEventHandler, &media.device_handle, &media.device_handle)) != UPNP_E_SUCCESS) {
		printf("Error registering the rootdevice : %d\n", ret);
		UpnpFinish();
		remove(MEDIA_SERVER_PID);
		exit(1);
    } else {
		printf("RootDevice Registered\n");
	
		printf("Initializing State Table\n");
		MediaDeviceStateTableInit(desc_doc_url,DESC_FILE);

		if ((ret = UpnpSendAdvertisement(media.device_handle, media.advr_expire)) != UPNP_E_SUCCESS) {
		    printf("Error sending advertisements : %d\n", ret);
		    UpnpFinish();
		    remove(MEDIA_SERVER_PID);
		    exit(1);
		}
		printf("Advertisements Sent\n");
    }
    if (pipe(media_pipe)<0) {   
	    goto out;

    } 
    if((scan_pid=fork())==0){//
	    close(media_pipe[0]);
	    if(!fast_scan)
	    	DelFile(cache_folder_path);
	    scan_main(conf_file, fast_scan);
	    exit(0);
    }
    close(media_pipe[1]);
    pthread_create(&pid, NULL, handleExpire, NULL);
    act.sa_handler=sig_usr1;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGUSR1, &act, NULL);		
			
	act.sa_handler=do_sig_term;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;	
    
	sigaction(SIGINT, &act, NULL);	
	sigaction(SIGTERM, &act, NULL);	    
	
	act.sa_handler=do_sig_hup;
	sigaction(SIGHUP, &act, NULL);	 
    while(1){
    	pause();
    	if(do_exit)
    		break;	
    }
out: 
    UpnpUnRegisterRootDevice(media.device_handle);
    UpnpFinish();
    list_destroy(g_playlist);
    kill(scan_pid, SIGTERM);
    //DelFile(cache_folder_path);
	remove(MEDIA_SERVER_PID);
	//Free allocated memory.
	for (i = 0; i < CONTENT_NUM; i++) {
		if(content_folders[i].path){
			free(content_folders[i].path);
			content_folders[i].path=NULL;
		}
	}		
	FreeContainerList();
    exit(0);
}


