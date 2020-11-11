/* options.h */
#ifndef _OPTIONS_H
#define _OPTIONS_H

#include "packet.h"

#define TYPE_MASK	0x0F

enum {
	OPTION_IP=1,
	OPTION_IP_PAIR,
	OPTION_STRING,
#ifdef __SC_BUILD__
	OPTION_HEX,
	OPTION_BASE64,
#endif
	OPTION_BOOLEAN,
	OPTION_U8,
	OPTION_U16,
	OPTION_S16,
	OPTION_U32,
	OPTION_S32,
#ifdef __SC_BUILD__
	OPTION_ROUTE
#endif
};

#define OPTION_REQ	0x10 /* have the client request this option */
#define OPTION_LIST	0x20 /* There can be a list of 1 or more of these */

struct dhcp_option {
#ifdef __SC_BUILD__
	char name[20];
#else
	char name[10];
#endif
	char flags;
	unsigned char code;
};
#if defined (__SC_BUILD__) && defined(CONFIG_SUPPORT_TR111) 
typedef struct vendor_list_t {
     u_int32_t client;       /* client IP, in network order */
     char ip[16];
     char oui[8];
     char serialNumber[64];
     char productClass[64];
     struct vendor_list_t *next;
 }VENDOR_LIST;

extern VENDOR_LIST *vendor_t;
extern int entries_num;
#endif

extern struct dhcp_option options[];
extern int option_lengths[];

unsigned char *get_option(struct dhcpMessage *packet, int code);
#ifdef __SC_BUILD__

int get_option_list(unsigned char *optionptr, int code, int len);
unsigned char *get_option_x(struct dhcpMessage *packet, int code, int *len);
unsigned int get_vendor_id(struct dhcpMessage *packet, char *vendor_id, unsigned int id_length);
int is_option_required(unsigned char code);
#endif
int end_option(unsigned char *optionptr);
int add_option_string(unsigned char *optionptr, unsigned char *string);
int add_simple_option(unsigned char *optionptr, unsigned char code, u_int32_t data);
struct option_set *find_option(struct option_set *opt_list, char code);
void attach_option(struct option_set **opt_list, struct dhcp_option *option, char *buffer, int length);
#ifdef __SC_BUILD__
int add_options_directly(unsigned char *optionptr, unsigned char code, char *data, unsigned int length);
#endif

#endif
