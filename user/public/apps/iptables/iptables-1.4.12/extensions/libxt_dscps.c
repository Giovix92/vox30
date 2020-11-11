/* Shared library add-on to iptables for DSCP
 *
 * (C) 2002 by Harald Welte <laforge@gnumonks.org>
 *
 * This program is distributed under the terms of GNU GPL v2, 1991
 *
 * libipt_dscp.c borrowed heavily from libipt_tos.c
 *
 * --class support added by Iain Barnes
 * 
 * For a list of DSCP codepoints see 
 * http://www.iana.org/assignments/dscp-registry
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_dscp_range.h>

static void dscps_help(void) 
{
	printf(
"DSCPS range match options\n"
"[!] --dscps value:value Match DSCP range codepoint with numerical value\n"
"  		                This value can be in decimal (ex: 32)\n"
"               		or in hex (ex: 0x20)\n"
"\n"
"				These two options are mutually exclusive !\n"
);
}

static void
parse_dscps(const char *s, struct xt_dscps_info *dinfo)
{
	unsigned int dscp, dscp_e;
	char *buffer;
	char *cp;

	buffer = strdup(s);
	if ((cp = strchr(buffer, ':')) == NULL)
        {	
		xtables_error(PARAMETER_PROBLEM,
			   "DSCP range wrong\n");
	}
	else 
        {
		*cp = '\0';
		cp++;

    	if(!xtables_strtoui(buffer,NULL, &dscp, 0, 255))
    		xtables_error(PARAMETER_PROBLEM,
    			   "Invalid dscp `%s'\n", buffer);
    
    	if(dscp > XT_DSCP_MAX)
    		xtables_error(PARAMETER_PROBLEM,
    			   "DSCP `%d` out of range\n", dscp);

    	if(!xtables_strtoui(cp, NULL, &dscp_e, 0, 255))
    		xtables_error(PARAMETER_PROBLEM,
    			   "Invalid dscp `%s'\n", cp);
    
    	if(dscp_e > XT_DSCP_MAX)
    		xtables_error(PARAMETER_PROBLEM,
    			   "DSCP `%d` out of range\n", dscp_e);
    
    	        dinfo->dscp = (u_int8_t )dscp;
		dinfo->dscp_e = (u_int8_t )dscp_e;
	}
	
	free(buffer);
	
	return;
}
//
enum {
	O_DSCP_RANGE = 0,
};

static const struct xt_option_entry dscps_opts[] = {
	{.name = "dscps", .id = O_DSCP_RANGE, .type = XTTYPE_STRING,
	 .flags = XTOPT_INVERT},
	XTOPT_TABLEEND,
};

static void dscps_parse(struct xt_option_call *cb)
{
	struct xt_dscps_info *dinfo = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_DSCP_RANGE:
		if (cb->invert)
			dinfo->invert = 1;
		parse_dscps(cb->arg, dinfo);
		break;
	}
}

static void dscps_check(struct xt_fcheck_call *cb)
{
	if (cb->xflags == 0)
		xtables_error(PARAMETER_PROBLEM,
		           "DSCPS match: Parameter --dscps is required");
}

static void
dscps_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_dscps_info *dinfo =
		(const struct xt_dscps_info *)match->data;
	printf(" DSCPS match %s0x%02x:%02x", dinfo->invert ? "!" : "", dinfo->dscp, dinfo->dscp_e);
}

static void dscps_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_dscps_info *dinfo =
		(const struct xt_dscps_info *)match->data;

	printf("%s --dscps %s0x%02x:%02x", dinfo->invert ? " !" : "", dinfo->dscp, dinfo->dscp_e);
}

static struct xtables_match dscps_match = {
	.family		= NFPROTO_UNSPEC,
	.name 		= "dscps",
	.version 	= XTABLES_VERSION,
	.size 		= XT_ALIGN(sizeof(struct xt_dscps_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_dscps_info)),
	.help		= dscps_help,
	.print		= dscps_print,
	.save		= dscps_save,
	.x6_parse	= dscps_parse,
	.x6_fcheck	= dscps_check,
	.x6_options	= dscps_opts,
};

void _init(void)
{
	xtables_register_match(&dscps_match);
}
