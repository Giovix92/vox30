#ifndef __NVRAM_H__
#define __NVRAM_H__

#include <string.h>

/*
 * nvram header struct 		            
 * magic    = 0x004E4F52 (RON)             
 * len      = 0~65495                      
 * crc      = use crc-32                    
 * reserved = reserved 	                    
 */
 
typedef struct nvram_header_s{
	unsigned long magic;
	unsigned long len;
	unsigned long crc;
	unsigned long reserved;
	
}nvram_header_t;

struct nv_entry{
	char *name;
	char *value;
    char *path;
	struct nv_entry *next;
};

char *nvram_get(const char *name);
/*
 * Match an NVRAM variable
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal to match or FALSE otherwise
 */
static inline int nvram_match(char *name, char *match) {
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

/*
 * IN_Match an NVRAM variable
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string equal to invmatch or FALSE otherwise
 */
static inline int nvram_invmatch(char *name, char *invmatch) {
	const char *value = nvram_get(name);
	return (value && strcmp(value, invmatch));
}

/*
 * Get the value of an NVRAM variable
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */
char* nvram_get_func(const char *name,char *path);
#ifdef CONFIG_SUPPORT_WIFI_5G
int nvram_bcm_set_x(const char* value,const char* name, ...);
#endif
int nvram_bcm_set(const char* name,const char* value);
char *nvram_bcm_get(const char* name);
char *nvram_bcm_safe_get(const char* name);
int nvram_bcm_unset(const char* name);
int nvram_unset(const char* name);
int nvram_set(const char* name,const char* value);

int readFileBin(char *path, char **data);
#endif
