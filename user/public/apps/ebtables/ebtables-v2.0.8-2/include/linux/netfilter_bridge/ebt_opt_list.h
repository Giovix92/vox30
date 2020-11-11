/**
 * @file ebt_opt_list.h
 * @author Phil Zhang
 * @date   2010-01-05
 * @brief  match the MAC list for the file. 
 *
 * Copyright - 2009 SerComm Corporation.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#ifndef __LINUX_BRIDGE_EBT_OPT_LIST_H
#define __LINUX_BRIDGE_EBT_OPT_LIST_H

#define EBT_OPT_LIST_MATCH "ebt_opt_list"

/*
 * option inverse flags definition 
 */
#define OPT_OPT_LIST_INDEX  0x01
#define OPT_OPT_LIST_SELECT 0x02
#define OPT_OPT_LIST_FLAGS	(OPT_OPT_LIST_INDEX | OPT_OPT_LIST_SELECT)

struct ebt_opt_list_info {
	int index;
	int select;
	uint8_t bitmask;
};

#endif
