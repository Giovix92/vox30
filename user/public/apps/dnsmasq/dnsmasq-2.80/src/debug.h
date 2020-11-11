
/*
 * =====================================================================================
 *
 *       Filename:  debug.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/28/2016 04:40:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Xiaojunjie (ywsy), 570434392@qq.com
 *        Company:  CUG
 *
 * =====================================================================================
 */
//#define _DEBUG_
//#ifndef SC_CFPRINTF
//#ifdef _DEBUG_
//#define MY_CFPRINTF(fmt, args...) do{ FILE *fp = fopen("/dev/console", "a+"); if(fp) {fprintf(fp, "[%s:%d]", __FUNCTION__, __LINE__); fprintf(fp, fmt, ##args); fclose(fp);}}while(0)
//#else
#define MY_CFPRINTF(fmt, args...)
//#endif
//#endif
//#define SC_CFPRINTF(fmt, args...) do{ FILE *fp = fopen("/dev/console", "a+"); if(fp) {fprintf(fp, "[%s:%d]", __FUNCTION__, __LINE__); fprintf(fp, fmt, ##args); fclose(fp);}}while(0)
