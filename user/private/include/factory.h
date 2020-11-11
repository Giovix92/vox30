#ifndef _FACTORY_H_
#define _FACTORY_H_

#define SN_LEN      16
#define RAM_SN_FILE     "tmp/hw_info/sn"
#define MAX_BUILD_TAG_LEN 128

/* --------------------------------------------------------------------------*/
/**
 * @brief     get factory SN assigned in Flash
 *
 * @Param    sn     OUTPUT 
 * @Param    len    INPUT  specify length of sn.
 *
 * @Returns  actually get sn length, -1 if any error  
 */
/* ----------------------------------------------------------------------------*/
int get_sn(char *sn, int len);
int get_version(char *version, int len);
int get_boot_version(char *version, int len);
int get_build_tag(char *buf, int len);


#endif
