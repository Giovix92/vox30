#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <nvram.h>

#define SAL_DIAG_TMP_VALUE_MAX_LENGTH   256
#define SAL_DIAG_TMP_PATH_MAX_LENGTH    256

#define DIAG_CFG_BASE "/tmp/sal/vendoroption.sal"

#define NVRAN_GET_DIAG_FUNC(funcname,name)\
char *funcname(void)\
{\
	{\
		static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
		char *p;\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
		snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE);\
		buffer[0] = '\0';\
		p = nvram_get_fun(name, diag_nvram_path);\
		if(p)\
		{\
			snprintf(buffer, sizeof(buffer), "%s", p);\
			free(p);\
		}\
		return buffer;\
	}\
}

#define NVRAN_SET_DIAG_FUNC(funcname,name)\
int funcname(char *value)\
{\
	{\
		char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
		if(!value)\
			return -1;\
		snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s",DIAG_CFG_BASE);\
		return nvram_set_p(diag_nvram_path, name, value);\
	}\
}

#define NVRAN_GET_HOP_FUNC(funcname, name)\
char *funcname(int id)\
 {\
     {\
         static char buffer[SAL_DIAG_TMP_VALUE_MAX_LENGTH];\
         char *p;\
         char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
         char buf[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
         snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE);\
         snprintf(buf, sizeof(buf), name, id);\
         buffer[0] = '\0';\
         p = nvram_get_fun(buf, diag_nvram_path);\
         if(p)\
         {\
             snprintf(buffer, sizeof(buffer), "%s", p);\
             free(p);\
         }\
         return buffer;\
     }\
 }

#define NVRAN_SET_HOP_FUNC(funcname, name)\
int funcname(char *value, int id)\
{\
    {\
        char diag_nvram_path[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        char buf[SAL_DIAG_TMP_PATH_MAX_LENGTH];\
        if(!value)\
            return -1;\
        snprintf(diag_nvram_path, sizeof(diag_nvram_path), "%s", DIAG_CFG_BASE);\
        snprintf(buf, sizeof(buf), name, id);\
        return nvram_set_p(diag_nvram_path, buf, value);\
     }\
}


#define SAL_DEVICEMANUFACTURE_OUI                   "Hop_%d_Oui"
#define SAL_DEVICESERIAL_NUMBER                     "Hop_%d_Number"
#define SAL_DEVICEPRODUCTCLASS                      "Hop_%d_Class"

NVRAN_GET_HOP_FUNC(sal_tr111_get_oui,SAL_DEVICEMANUFACTURE_OUI);
NVRAN_SET_HOP_FUNC(sal_tr111_set_oui,SAL_DEVICEMANUFACTURE_OUI);
NVRAN_GET_HOP_FUNC(sal_tr111_get_number,SAL_DEVICESERIAL_NUMBER);
NVRAN_SET_HOP_FUNC(sal_tr111_set_number,SAL_DEVICESERIAL_NUMBER);
NVRAN_GET_HOP_FUNC(sal_tr111_get_class,SAL_DEVICEPRODUCTCLASS);
NVRAN_SET_HOP_FUNC(sal_tr111_set_class,SAL_DEVICEPRODUCTCLASS);
