#ifndef UPNP_DS_DEVICE_H
#define UPNP_DS_DEVICE_H
#include <upnp/upnp.h>

#define UPNP_PORT	49152

#define UPNP_DIR	"/usr/upnp/"
#define DESC_FILE	"mediaserver.xml"
#define UPNP_MAX_CONTENT_LENGTH 4096


#define	VIRTUAL_DIR	"/upnpav"
#define VIRTUAL_PIC	"/tm_pic"
#define VIRTUAL_PNG	"/tm_png"
#define VIRTUAL_LM     "/lm_pic"
typedef struct Node
{
	char*		name;    
	struct Node*	next;
}node;

typedef struct PLAYLIST {
	int	size;
	node*	head;
	node*	tail;	
} playlist;

struct MediaEnv
{
		UpnpDevice_Handle device_handle;
		char ifname[10]; /* interface name */
		// export variables
		char uuid[40];

		int advr_expire;
		int advr_ttl;
		int upnp_port;
		int device_port;
		// model related
		char friendly_name[65];
		char manufacturer[65];
		char manufacturer_url[256];
		char model_description[129];
		char model_name[64];
		char model_number[64];
		char serial_number[65];

		// State Variables
		char InternalIPAddress[20];
};
int PRO_GetStr(char *sect, char *key, char *val, int size, FILE * fp);
#endif
