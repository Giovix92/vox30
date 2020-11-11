/**
 * @file ebt_opt_list.c
 * @author Phil Zhang
 * @date   2010-01-05
 * @brief  match the MAC list for the file. 
 *
 * Copyright - 2009 SerComm Corporation.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include "../include/ebtables_u.h"
#include "../include/ethernetdb.h"
#include <linux/netfilter_bridge/ebt_opt_list.h>
#include <linux/if_ether.h>

static struct option opts[] = {
	{"index", required_argument, NULL, '1'},
	{"select", required_argument, NULL, '2'},
	{ 0 }
};

static void print_help()
{
	printf(
    "ebt_opt_list match v%s options:\n"
    "--index    ID of QoS class rule\n"
    "--select   0: exclude, 1: include\n"
    ,"?");
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_opt_list_info *opt_list_info = (struct ebt_opt_list_info *) match->data;
	opt_list_info->index = 0;
	opt_list_info->select = 1;
}

static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_opt_list_info *opt_list_info = (struct ebt_opt_list_info *) (*match)->data;
	char *end;

	switch (c) {
	case '1':
		ebt_check_option2(flags, OPT_OPT_LIST_INDEX);
		opt_list_info->index = strtoul(optarg, &end, 10);
		if (opt_list_info->index > 19 || *end != '\0')
			ebt_print_error2("Invalid --index range ('%s')", optarg);
		opt_list_info->bitmask |= OPT_OPT_LIST_INDEX;
		break;
	case '2':
		ebt_check_option2(flags, OPT_OPT_LIST_SELECT);
		opt_list_info->select = strtoul(optarg, &end, 10);
		if (opt_list_info->select > 1 || *end != '\0')
			ebt_print_error2("Invalid --select range ('%s')", optarg);
		opt_list_info->bitmask |= OPT_OPT_LIST_SELECT;
		break;
	default:
		return 0;

	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match,
   const char *name, unsigned int hookmask, unsigned int time)
{
    return;
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_opt_list_info *opt_list_info = (struct ebt_opt_list_info *) match->data;

	if (opt_list_info->bitmask & OPT_OPT_LIST_INDEX) {
		printf("--index %d ", opt_list_info->index);
	}
	if (opt_list_info->bitmask & OPT_OPT_LIST_SELECT) {
		printf("--select %d ", opt_list_info->select);
	}
}

static int compare(const struct ebt_entry_match *opt_list1,
   const struct ebt_entry_match *opt_list2)
{
	struct ebt_opt_list_info *opt_list_info1 = (struct ebt_opt_list_info *) opt_list1->data;
	struct ebt_opt_list_info *opt_list_info2 = (struct ebt_opt_list_info *) opt_list2->data;

	if (opt_list_info1->bitmask != opt_list_info2->bitmask)
		return 0;
	if (opt_list_info1->index != opt_list_info2->index)
		return 0;
	if (opt_list_info1->select != opt_list_info2->select)
		return 0;
		
	return 1;
}

static struct ebt_u_match opt_list_match = {
	.name		= EBT_OPT_LIST_MATCH,
	.size		= sizeof(struct ebt_opt_list_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check = final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

void _init(void)
{
	ebt_register_match(&opt_list_match);
}
