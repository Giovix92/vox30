/**
 * @file ipt_opt_list.h
 * @author West Zhou
 * @date   2017-06-09
 * @brief  
 *
 * Copyright - 2017 SerComm Corporation. All Rights Reserved. 
 * SerComm Corporation reserves the right to make changes to this document without notice. 
 * SerComm Corporation makes no warranty, representation or guarantee regarding the suitability 
 * of its products for any particular purpose. SerComm Corporation assumes no liability arising 
 * out of the application or use of any product or circuit. SerComm Corporation specifically 
 * disclaims any and all liability, including without limitation consequential or incidental 
 * damages; neither does it convey any license under its patent rights, nor the rights of others.
 */

#ifndef _XT_OPT_LIST_H
#define _XT_OPT_LIST_H

#define MAX_INFO_LENGTH 64
struct xt_opt_list_info {
    __u32 match_option;
    __u8 is_src;
    __u8 invert;
    char match_info[MAX_INFO_LENGTH];
};

#endif /* _IPT_OPT_LIST_H */
