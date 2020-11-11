/*
 * options.c -- DHCP server option packet tools
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"


/* supported options are easily added here */
struct dhcp_option options[] = {
	/* name[10]	flags					code */
	{"subnet",	OPTION_IP | OPTION_REQ,			0x01},
#ifdef __SC_BUILD__
	{"timezone",	OPTION_S32 | OPTION_REQ,				0x02},
#else
	{"timezone",	OPTION_S32,				0x02},
#endif
	{"router",	OPTION_IP | OPTION_LIST | OPTION_REQ,	0x03},
	{"timesvr",	OPTION_IP | OPTION_LIST,		0x04},
	{"namesvr",	OPTION_IP | OPTION_LIST,		0x05},
	{"dns",		OPTION_IP | OPTION_LIST | OPTION_REQ,	0x06},
	{"logsvr",	OPTION_IP | OPTION_LIST,		0x07},
	{"cookiesvr",	OPTION_IP | OPTION_LIST,		0x08},
	{"lprsvr",	OPTION_IP | OPTION_LIST,		0x09},
	{"hostname",	OPTION_STRING | OPTION_REQ,		0x0c},
	{"bootsize",	OPTION_U16,				0x0d},
	{"domain",	OPTION_STRING | OPTION_REQ,		0x0f},
	{"swapsvr",	OPTION_IP,				0x10},
	{"rootpath",	OPTION_STRING,				0x11},
	{"ipttl",	OPTION_U8,				0x17},
	{"mtu",		OPTION_U16,				0x1a},
	{"broadcast",	OPTION_IP | OPTION_REQ,			0x1c},

#ifdef __SC_BUILD__
    {"sroute",	OPTION_STRING,				0x21},
	{"ntpsrv",	OPTION_IP | OPTION_LIST | OPTION_REQ,	0x2a},
	{"vendorinfo",  OPTION_BASE64,           0x2b},
#else
	{"ntpsrv",	OPTION_IP | OPTION_LIST,		0x2a},
#endif
	{"wins",	OPTION_IP | OPTION_LIST,		0x2c},
	{"requestip",	OPTION_IP,				0x32},
	{"lease",	OPTION_U32,				0x33},
	{"dhcptype",	OPTION_U8,				0x35},
	{"serverid",	OPTION_IP,				0x36},
	{"message",	OPTION_STRING,				0x38},
#ifdef __SC_BUILD__
	{"t1",  OPTION_U32,                 0x3a},
	{"t2",  OPTION_U32,                 0x3b},
	{"vendorid",  OPTION_HEX,                 0x3c},
#endif
	{"tftp",	OPTION_STRING,				0x42},
	{"bootfile",	OPTION_STRING,				0x43},
	{"sipsrv",	OPTION_STRING,				0x78},
#ifdef __SC_BUILD__    
    {"clroute",	OPTION_STRING,				0x79},
    {"vendorspecific",	OPTION_BASE64,				0x7d},
    {"provisionserver",	OPTION_STRING,	    0x72},
    {"tftpname",	OPTION_IP | OPTION_LIST,	        0x96},
    {"msclroute",OPTION_STRING,				0xF9},
    {"160",     OPTION_STRING,				0xa0},
#endif
	{"pu240",	OPTION_STRING,		        0xf0},  // pu = private use
	{"pu241",	OPTION_STRING,		        0xf1},
	{"pu242",	OPTION_STRING,		        0xf2},
	{"pu243",	OPTION_STRING,		        0xf3},
	{"pu244",	OPTION_STRING,		        0xf4},
	{"pu245",	OPTION_STRING,		        0xf5},
	{"pu246",	OPTION_STRING,		        0xf6},
	{"pu247",	OPTION_STRING,		        0xf7},
	{"pu248",	OPTION_STRING,		        0xf8},
	{"pu250",	OPTION_STRING,		        0xfa},
	{"",		0x00,				0x00}
};

/* Lengths of the different option types */
int option_lengths[] = {
	[OPTION_IP] =		4,
	[OPTION_IP_PAIR] =	8,
	[OPTION_BOOLEAN] =	1,
	[OPTION_STRING] =	1,
#ifdef __SC_BUILD__
	[OPTION_HEX]=		1,
	[OPTION_BASE64]=	1,
#endif
	[OPTION_U8] =		1,
	[OPTION_U16] =		2,
	[OPTION_S16] =		2,
	[OPTION_U32] =		4,
	[OPTION_S32] =		4,
#ifdef __SC_BUILD__
	[OPTION_ROUTE] =	9
#endif
};


/* get an option with bounds checking (warning, not aligned). */
unsigned char *get_option(struct dhcpMessage *packet, int code)
{
	int i, length;
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;

	optionptr = packet->options;
	i = 0;
#ifdef __SC_BUILD__
	length = MAX_DHCP_OPTIONS_LEN;
#else
	length = 308;
#endif
	while (!done) {
		if (i >= length) {
#ifndef __SC_BUILD__
			LOG(LOG_WARNING, "bogus packet, option fields too long.");
#endif
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
#ifndef __SC_BUILD__
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
#endif
				return NULL;
			}
			return optionptr + i + 2;
		}
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
#ifndef __SC_BUILD__
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
#endif
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}
#ifdef __SC_BUILD__
int get_option_list(unsigned char *optionptr, int code, int len)
{
    int j;
    unsigned char *p;
    for(j=0;j<len;j++)
    {
        p = optionptr + j;
        if(p[0] == code) 
            return 1;
    }
    return 0;
}
unsigned char *get_option_x(struct dhcpMessage *packet, int code, int *len)
{
	int i, length;
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;

	optionptr = packet->options;
	i = 0;
#ifdef __SC_BUILD__
	length = MAX_DHCP_OPTIONS_LEN;
#else
	length = 308;
#endif
        *len = 0;
	while (!done) {
		if (i >= length) {
			LOG(LOG_WARNING, "bogus packet, option fields too long.");
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
				return NULL;
			}
			*len = *(optionptr + i + 1);
			return optionptr + i + 2;
		}
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}
int is_option_required(unsigned char code)
{
    int i;
    for(i = 0; options[i].code; i++)
    {
        if(code == options[i].code)
        {
	   if(options[i].flags & OPTION_REQ)
		return 1;
	}
    }
    return 0;
}
#endif
/***********************************************************************
* Vendor class identifier( option 60 ) Format :                                                          
*                                                                                                                          
* 0                        7                          15                                                      31               
* ---------------+---------------+---------------+---------------+        
*         code(60)                Length                         Enterprise Code
* ---------------+---------------+---------------+---------------+
*       Field type              Field length                          Field value
*
************************************************************************/
#ifdef __SC_BUILD__
unsigned int get_vendor_id(struct dhcpMessage *packet, char *vendor_id, unsigned int id_length)
{
#define OPT60_ENTERPRISE_CODE_LEN		2

#define OPT60_FIELD_CODE				0
#define OPT60_FIELD_LEN					1
#define OPT60_FIELD_DATA				2

#define OPT60_FIELD_TYPE_VENDOR		1
#define OPT60_FIELD_TYPE_CATEGORY		2
#define OPT60_FIELD_TYPE_MODEL		3
#define OPT60_FIELD_TYPE_VERSION		4
#define OPT60_FIELD_TYPE_PROTO_PORT	5

	int i, length;
	unsigned char *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;
	
	unsigned int last_length = 0, offset = 0;
	unsigned int field_type = 0, field_length = 0;
	char enterprise_code[OPT60_ENTERPRISE_CODE_LEN];

	if(!vendor_id || id_length<32)
	{
		return 0;
	}
	strcpy(vendor_id, "Computer");

	optionptr = packet->options;
	i = 0;
#ifdef __SC_BUILD__
	length = MAX_DHCP_OPTIONS_LEN;
#else
	length = 308;
#endif
	// CTC is "00"
	enterprise_code[0] = 0;
	enterprise_code[1] = 0;
	
	while (!done) 
	{
		if (i >= length) 
		{
			LOG(LOG_WARNING, "bogus packet, option fields too long.");
			return 0;
		}
		if (optionptr[i + OPT_CODE] == DHCP_VENDOR)  //option 60
		{   
			if (i /*offset*/ + 1 /*size of option code*/+ optionptr[i + OPT_LEN] /*option len*/ >= length) // total length is overflow
			{ 
				LOG(LOG_WARNING, "bogus packet, the value of option 60 length is too long.");
				return 0;
			}
			if(optionptr[i + OPT_LEN] <= 2) // option length is invalid
			{
				LOG(LOG_WARNING, "bogus packet, the value of option 60 length is too short.");
				return 0;
			}

			if( memcmp(optionptr + i + OPT_DATA, enterprise_code, OPT60_ENTERPRISE_CODE_LEN)==0 ) // enterprise code, CTC is "00"
			{
				/* Notes: '1 + 1' means 'size of field code' + 'size of field length' */
				for(
					last_length	= optionptr[i + OPT_LEN] -OPT60_ENTERPRISE_CODE_LEN, //the length of the fields except enterprise code length
					offset 		= i + 1 + 1 + OPT60_ENTERPRISE_CODE_LEN // start from field head;
					;
					last_length >= (unsigned int)1+ 1 + optionptr[offset + OPT60_FIELD_LEN] // total length is overflow
					;
					last_length	-= (1 + 1 + field_length),
					offset		+= (1 + 1 + field_length)
				) 
				{
					field_type	= optionptr[offset+OPT60_FIELD_CODE];
					field_length	= optionptr[offset+OPT60_FIELD_LEN];

					if( field_type == OPT60_FIELD_TYPE_CATEGORY )
					{
						if(field_length>=1 && field_length<=32)
						{
							memset(vendor_id, 0, id_length);
							memcpy(vendor_id, optionptr+offset+OPT60_FIELD_DATA, field_length);
						}
						return 1;
					}
				}
			}
			else
			{
			//	LOG(LOG_WARNING, "bogus packet, enterprise code incorrect (Not CTC).");
				return 0;
			}
		}	
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				LOG(LOG_WARNING, "bogus packet, option fields too long.");
				return 0;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return 0;
}
#endif
/* return the position of the 'end' option (no bounds checking) */
int end_option(unsigned char *optionptr)
{
	int i = 0;

	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] == DHCP_PADDING) i++;
		else i += optionptr[i + OPT_LEN] + 2;
	}
	return i;
}


/* add an option string to the options (an option string contains an option code,
 * length, then data) */
int add_option_string(unsigned char *optionptr, unsigned char *string)
{
	int end = end_option(optionptr);


	/* end position + string length + option code/length + end option */
	if (end + string[OPT_LEN] + 2 + 1 >= MAX_DHCP_OPTIONS_LEN) {
		LOG(LOG_ERR, "Option 0x%02x did not fit into the packet!", string[OPT_CODE]);
		return 0;
	}
	DEBUG(LOG_INFO, "adding option 0x%02x", string[OPT_CODE]);
	memcpy(optionptr + end, string, string[OPT_LEN] + 2);
	optionptr[end + string[OPT_LEN] + 2] = DHCP_END;
	return string[OPT_LEN] + 2;
}
#ifdef __SC_BUILD__
int add_options_directly(unsigned char *optionptr, unsigned char code, char *data, unsigned int length)
{

	unsigned char option[2 + length];

	option[OPT_CODE] = code;
	option[OPT_LEN] = length;
	memcpy(option + 2, data, length);
	return add_option_string(optionptr, option);
}
#endif
/* add a one to four byte option to a packet */
int add_simple_option(unsigned char *optionptr, unsigned char code, u_int32_t data)
{
	char length = 0;
	int i;
	unsigned char option[2 + 4];
	unsigned char *u8;
	u_int16_t *u16;
	u_int32_t *u32;
	u_int32_t aligned;
	u8 = (unsigned char *) &aligned;
	u16 = (u_int16_t *) &aligned;
	u32 = &aligned;

	for (i = 0; options[i].code; i++)
		if (options[i].code == code) {
			length = option_lengths[options[i].flags & TYPE_MASK];
		}

	if (!length) {
		DEBUG(LOG_ERR, "Could not add option 0x%02x", code);
		return 0;
	}

	option[OPT_CODE] = code;
	option[OPT_LEN] = length;

	switch (length) {
		case 1: *u8 =  data; break;
		case 2: *u16 = data; break;
		case 4: *u32 = data; break;
	}
	memcpy(option + 2, &aligned, length);
	return add_option_string(optionptr, option);
}


/* find option 'code' in opt_list */
struct option_set *find_option(struct option_set *opt_list, char code)
{
	while (opt_list && opt_list->data[OPT_CODE] < code)
		opt_list = opt_list->next;

	if (opt_list && opt_list->data[OPT_CODE] == code) return opt_list;
	else return NULL;
}

static int parse_dhcp_option_sip(char *src_buf, int src_len, char *sip_buf)
{
    int sip_len = 0;

    if (isdigit(*src_buf))
    {
        sip_buf[sip_len++] = 1; // encoding ip list
        struct in_addr addr;
        char delim[2] = ";";
        char *ip = strtok(src_buf, delim);
        while (ip)
        {
            if (inet_aton(ip, &addr))
            {
                memcpy(sip_buf + sip_len, (char *)&addr, sizeof(struct in_addr));
                sip_len += sizeof(struct in_addr);
            }
            ip = strtok(NULL, delim);
        }
    }
    else
    {
        sip_buf[sip_len++] = 0; // encoding domain name
        char delim[2] = ";";
        char *domain = strtok(src_buf, delim);
        while (domain)
        {
            char *pos = domain;
            char *str;
            while ((str = strchr(pos, '.')) && *pos != '\0')
            {
                sip_buf[sip_len++] = str - pos;
                memcpy(sip_buf + sip_len, pos, str - pos);
                sip_len += str - pos;
                pos = str + 1;
            }
            if (*pos != '\0')
            {
                sip_buf[sip_len++] = strlen(pos);
                memcpy(sip_buf + sip_len, pos, strlen(pos));
                sip_len += strlen(pos);
            }
            sip_buf[sip_len++] = 0; // one end
            domain = strtok(NULL, delim);
        }
    }

    return sip_len;
}

/* add an option to the opt_list */
void attach_option(struct option_set **opt_list, struct dhcp_option *option, char *buffer, int length)
{
	struct option_set *existing, *new, **curr;

	/* add it to an existing option */
	if ((existing = find_option(*opt_list, option->code))) {
		DEBUG(LOG_INFO, "Attaching option %s to existing member of list", option->name);
		if (option->flags & OPTION_LIST) {
			if (existing->data[OPT_LEN] + length <= 255) {
				existing->data = realloc(existing->data, 
						existing->data[OPT_LEN] + length + 2);
				memcpy(existing->data + existing->data[OPT_LEN] + 2, buffer, length);
				existing->data[OPT_LEN] += length;
			} /* else, ignore the data, we could put this in a second option in the future */
		} /* else, ignore the new data */
	} else {
		DEBUG(LOG_INFO, "Attaching option %s to list", option->name);
		
        char *opt;
	    int opt_len;
	    char sip_buf[300] = "";

	    if (DHCP_SIP_SERVERS == option->code)
	    {
	        opt = sip_buf;
	        opt_len = parse_dhcp_option_sip(buffer, length, sip_buf);
	    }
	    else
	    {
            opt = buffer;
            opt_len = length;
        }
		/* make a new option */
		new = malloc(sizeof(struct option_set));
		new->data = malloc(opt_len + 2);
		new->data[OPT_CODE] = option->code;
		new->data[OPT_LEN] = opt_len;
		memcpy(new->data + 2, opt, opt_len);
		
		curr = opt_list;
		while (*curr && (*curr)->data[OPT_CODE] < option->code)
			curr = &(*curr)->next;
			
		new->next = *curr;
		*curr = new;		
	}
}
