/**
 * @file libipt_opt_list.c
 * @author West Zhou
 * @date   2017-06-09
 * @brief  parse the duration time of a rule. 
 *
 * Copyright - 2009 SerComm Corporation.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/xt_opt_list.h>

/* Function which prints out usage message. */
static void opt_list_help(void)
{
	printf(
    "ipt_opt_list match v%s options:\n"
    "[!] --opt dhcp_id   ID of DHCP option code\n"
    "--info string       Match info\n"
    "--src               Match src mac \n"
    "\n",
    XTABLES_VERSION);
}
enum
{
    O_DHCP_ID = 0,
    O_MATCH_INFO,
    O_IS_SRC,
};
static const struct xt_option_entry opt_list_opts[] = {
	{.name = "opt",     .id = O_DHCP_ID, .type = XTTYPE_UINT32,
	 .flags = XTOPT_MAND | XTOPT_INVERT| XTOPT_PUT, XTOPT_POINTER(struct xt_opt_list_info, match_option)},
	{.name = "info",    .id = O_MATCH_INFO, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_PUT , XTOPT_POINTER(struct xt_opt_list_info, match_info)},
	{.name = "is_src",    .id = O_IS_SRC, .type = XTTYPE_UINT8,
	 .flags = XTOPT_PUT  , XTOPT_POINTER(struct xt_opt_list_info, is_src)},
	XTOPT_TABLEEND,
};



/* Function which parses command options; returns true if it ate an option */
static void opt_list_parse(struct xt_option_call *cb)
{
	struct xt_opt_list_info *info = cb->data;
    xtables_option_parse(cb);
    if (cb->invert)
        info->invert = true;
	return;
}

/* Prints out the info. */
static void opt_list_print(const void *ip, const struct xt_entry_match *match,
                        int numeric)
{
    struct xt_opt_list_info *info = (struct xt_opt_list_info *)match->data;
    if(info->invert)
        printf(" !");
    printf(" opt %u info %s", info->match_option, info->match_info);
    if(info->is_src)
        printf(" is_src 1");
}

/* Saves the union ipt_info in parsable form to stdout. */
static void opt_list_save(const void *ip, const struct xt_entry_match *match)
{
    struct xt_opt_list_info *info = (struct xt_opt_list_info *)match->data;
    if(info->invert)
        printf(" !");
    printf(" --opt %u --info %s", info->match_option, info->match_info);
    if(info->is_src)
        printf(" --is_src 1");
}
static struct xtables_match opt_list_match = {
	.name		= "opt_list",
	.family		= NFPROTO_UNSPEC,
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_opt_list_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_opt_list_info)),
	.help		= opt_list_help,
	.print		= opt_list_print,
	.save		= opt_list_save,
	.x6_parse		= opt_list_parse,
	.x6_options	= opt_list_opts,
};

void _init(void)
{
	xtables_register_match(&opt_list_match);
}

