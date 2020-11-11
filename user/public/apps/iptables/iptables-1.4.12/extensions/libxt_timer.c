/**
 * @file libxt_timer.h
 * @author Phil Zhang
 * @date   2009-11-22
 * @brief  parse the end time of a rule. 
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
#include <linux/netfilter/xt_timer.h>

#define XT_FINISH_TIME 0x01

/* Function which prints out usage message. */
static void timer_help(void)
{
	printf(
    "timer match v%s options:\n"
    "--finish-time seconds            Seconds from bootup\n"
    "\n",
    XTABLES_VERSION);
}

static const struct option timer_opts[] = {
	{.name = "finish-time",      .has_arg = true,  .val = '1'},
	XT_GETOPT_TABLEEND,
};

static void parse_timer(char *arg, struct xt_timer_info *info)
{
	int length=0;
	
	length=strlen(arg);
	if(length <= 0)
	    xtables_error(PARAMETER_PROBLEM,
			   "timer match: mapping string of finish time was wrong!\n");
	
	info->finish_time = strtoul(arg, NULL, 10);
}

/* Initialize the match. */
static void timer_init(struct xt_entry_match *match)
{
	struct xt_timer_info *timerinfo = (struct xt_timer_info *)match->data;

	timerinfo->finish_time = 0;
}


/* Function which parses command options; returns true if it ate an option */
static int parse(int c, char **argv, int invert, unsigned int *flags,
        const void *entry, struct xt_entry_match **match)
{
	struct xt_timer_info *info = (struct xt_timer_info *)(*match)->data;

	switch (c) {
	case '1':
		if (*flags & XT_FINISH_TIME)
			xtables_error(PARAMETER_PROBLEM,
				   "timer match: Only use --finish_time ONCE!");
		*flags |= XT_FINISH_TIME;
		parse_timer(optarg, info);		
		break;

	default:
		return 0;
	}
	return 1;
}

/* Final check; we don't care. */
static void final_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "timer match: You must specify `--finish-time'");
}

static void print_timer(const struct xt_timer_info *info)
{
	printf("--finish-time %u \n", info->finish_time);
}

/* Prints out the info. */
static void timer_print(const void *ip, const struct xt_entry_match *match,
        int numeric)
{
	struct xt_timer_info *info = (struct xt_timer_info *)match->data;

    print_timer(info);
}

/* Saves the union xt_info in parsable form to stdout. */
static void save(const void *ip, const struct xt_entry_match *match)
{
	struct xt_timer_info *info = (struct xt_timer_info *)match->data;

    printf("--finish-time %u ", info->finish_time);
}

static struct xtables_match timer = { 
	.name		= "timer",
	.family		= AF_INET,
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_timer_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_timer_info)),
	.help		= &timer_help,
	.init		= &timer_init,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &timer_print,
	.save		= &save,
	.extra_opts	= timer_opts
};

void _init(void)
{
	xtables_register_match(&timer);
}
