/*
 * Shared library add-on for iptables. Adds PORT TRIGGERING support.
 *
 * Copyright (C) 2010 Broadlight Ltd.. All rights reserved.
 *
 * Contact: KT <kirill@broadlight.com>
 *
 */

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_PORTTRIG.h>

#if 1
#define DEBUGP(args...) printf("XT_PORTTRIG | " args);
#define DEBUGP2(format, args...)
#else
#define DEBUGP2(format, args...)
#endif

enum {
	PORTTRIG_TG_OPT_TRIGPORT 	= 1 << 0,
	PORTTRIG_TG_OPT_TRIGPROTO 	= 1 << 1,
	PORTTRIG_TG_OPT_RELPORT 	= 1 << 2,
	PORTTRIG_TG_OPT_STRICTSRC 	= 1 << 3,
#ifdef CONFIG_SCM_SUPPORT
	PORTTRIG_TG_OPT_RELPROTO 	= 1 << 4,
#endif

};

/* Function which prints out usage message. */
static void PORTTRIG_help(void)
{
	printf(
		"PORTTRIG target v%s options:\n"
		"--trigger-port-range	port:port			The trigger port (DNAT only)\n"
		"--trigger-proto		type (tcp,udp,all)	The trigger protocol\n"
#ifdef CONFIG_SCM_SUPPORT
		"--rel-proto			type (tcp,udp,all)	The Related protocol\n"
#endif
		"--rel-port-range		port:port			Related port range\n"
		"--strict-source	                    	Create source based firewall rule\n"
		"\n",
		XTABLES_VERSION);
}

static const struct option PORTTRIG_tg_opts[] = {
	{ .name = "trigger-port-range",	.has_arg = 1, .flag = 0, .val = '1' },
	{ .name = "trigger-proto",   	.has_arg = 1, .flag = 0, .val = '2' },
	{ .name = "rel-port-range",  	.has_arg = 1, .flag = 0, .val = '3' },
	{ .name = "strict-source",  	.has_arg = 0, .flag = 0, .val = '4' },
#ifdef CONFIG_SCM_SUPPORT
	{ .name = "rel-proto",   		.has_arg = 1, .flag = 0, .val = '5' },
#endif
	{ .name = NULL }
};

#define IPPROTO_ALL 0xFE

static u_int8_t name_to_proto(char* name)
{
	if (!strncmp(name, "tcp", 3))
		return IPPROTO_TCP;
	else
		if (!strncmp(name, "udp", 3))
			return IPPROTO_UDP;
		else
			if (!strncmp(name, "all", 3))
				return IPPROTO_ALL;

	return 0;
}

static char* proto_to_name(u_int8_t proto)
{
	switch (proto) {
		case IPPROTO_UDP:
			return "udp";
		case IPPROTO_TCP:
			return "tcp";
		case IPPROTO_ALL:
			return "all";
		default:
			return "unknown";
	}

	return "error-proto";
}

/* Function which parses command options; returns true=1 if it
   ate an option */
static int
PORTTRIG_parse(int c, char **argv, int invert, unsigned int *flags,
      const void *entry,
      struct xt_entry_target **target)
{
	struct xt_porttrig_target_info *info = (struct xt_porttrig_target_info *)(*target)->data;
	int port_val1 = 0;
	int port_val2 = 0;

	DEBUGP2(">>>PORTTRIG_parse() IN:[%c]\n", c);

	switch (c) {
	case '1':

		if (*flags & PORTTRIG_TG_OPT_TRIGPORT) {
			xtables_error(PARAMETER_PROBLEM,
				      "Can't specify --trigger-port-range twice");
		} else {
			if (sscanf(optarg, "%d:%d", &port_val1, &port_val2) != 2)
				return 1;
		}

		info->trigger_first_port = port_val1;
		info->trigger_last_port  = port_val2;

		DEBUGP("--trigger-port-range:[%d...%d]\n", info->trigger_first_port, info->trigger_last_port);

		*flags |= PORTTRIG_TG_OPT_TRIGPORT;
		break;

	case '2':

		if (*flags & PORTTRIG_TG_OPT_TRIGPROTO) {
			xtables_error(PARAMETER_PROBLEM,
				      "Can't specify --trigger-proto twice");
		} else {
			info->trigger_proto = name_to_proto((char*) optarg);

			DEBUGP2(">>>Protocol name convert:[%s-->%d]\n", (char*) optarg, info->trigger_proto);

			if (info->trigger_proto)
				return 1;
		}

		DEBUGP("--trigger-proto:[%d]\n", info->trigger_proto);

		*flags |= PORTTRIG_TG_OPT_TRIGPROTO;
		break;

	case '3':

		if (*flags & PORTTRIG_TG_OPT_RELPORT) {
			xtables_error(PARAMETER_PROBLEM,
				      "Can't specify --rel-port-range twice");
		} else {
			if (sscanf(optarg, "%d:%d", &port_val1, &port_val2) != 2)
				return 1;
		}

		info->related_first_port = port_val1;
		info->related_last_port  = port_val2;

		DEBUGP("--rel-port-range:[%d:%d]\n", info->related_first_port, info->related_last_port);

		*flags |= PORTTRIG_TG_OPT_RELPORT;
		break;

	case '4':

		if (*flags & PORTTRIG_TG_OPT_STRICTSRC) {
			xtables_error(PARAMETER_PROBLEM,
				      "Can't specify --strict-source twice");
		}

		info->strict_src = 1;

		DEBUGP("--strict-source:[%s]\n", (info->strict_src?"YES":"NO"));

		*flags |= PORTTRIG_TG_OPT_STRICTSRC;
		break;
#ifdef CONFIG_SCM_SUPPORT
	case '5':

		if (*flags & PORTTRIG_TG_OPT_RELPROTO) {
			xtables_error(PARAMETER_PROBLEM,
				      "Can't specify --rel-proto twice");
		} else {
			info->related_proto = name_to_proto((char*) optarg);

			DEBUGP2(">>>Protocol name convert:[%s-->%d]\n", (char*) optarg, info->related_proto);

			if (info->related_proto)
				return 1;
		}

		DEBUGP("--rel-proto:[%d]\n", info->related_proto);

		*flags |= PORTTRIG_TG_OPT_RELPROTO;
		break;
#endif

	default:
		return 0;
	}

	return 1;
}

/* Final check; must have specified --src-range or --dst-range. */
static void PORTTRIG_final_check(unsigned int flags)
{
	if (!(flags & PORTTRIG_TG_OPT_TRIGPORT))
		xtables_error(PARAMETER_PROBLEM, "PORTTRIG target: "
			      "--trigger-port-range parameter is required");

	if (!(flags & PORTTRIG_TG_OPT_RELPORT))
		xtables_error(PARAMETER_PROBLEM, "PORTTRIG target: "
			      "--rel-port-range parameter is required");
}

/* Prints out the info. */
static void PORTTRIG_print(const void *ip,const struct xt_entry_target *target,
                          int numeric)
{
	struct xt_porttrig_target_info *info = (struct xt_porttrig_target_info *)target->data;

//	printf(">>>PORTTRIG_print() IN\n");

#ifdef CONFIG_SCM_SUPPORT
	printf("%s:trigger:[%d...%d] --> %s:related:[%d...%d]",
		proto_to_name(info->trigger_proto),
		info->trigger_first_port, info->trigger_last_port,
		proto_to_name(info->related_proto),
		info->related_first_port, info->related_last_port);
#else
	printf("%s:trigger:[%d...%d] --> related:[%d...%d]",
		proto_to_name(info->trigger_proto),
		info->trigger_first_port, info->trigger_last_port,
		info->related_first_port, info->related_last_port);
#endif
}

/* Saves the union ipt_info in parsable form to stdout. */
static void PORTTRIG_save(const void *ip, const struct xt_entry_target *target)
{
	struct xt_porttrig_target_info *info = (struct xt_porttrig_target_info *)target->data;

//	printf(">>>PORTTRIG_save() IN\n");
#ifdef CONFIG_SCM_SUPPORT
	printf("%s:trigger:[%d...%d] --> %s:related:[%d...%d]",
		proto_to_name(info->trigger_proto),
		info->trigger_first_port, info->trigger_last_port,
		proto_to_name(info->related_proto),
		info->related_first_port, info->related_last_port);
#else

	printf("%s:trigger:[%d...%d] --> related:[%d...%d]",
		proto_to_name(info->trigger_proto),
		info->trigger_first_port, info->trigger_last_port,
		info->related_first_port, info->related_last_port);
#endif
}

static struct xtables_target porttrig_target = {
	.family		= AF_INET,
	.name		= "PORTTRIG",
	.version	= XTABLES_VERSION,
	.revision	= 0,
	.size			= XT_ALIGN(sizeof(struct xt_porttrig_target_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_porttrig_target_info)),
	.parse		= PORTTRIG_parse,
	.help		= PORTTRIG_help,
	.final_check	= PORTTRIG_final_check,
	.print		= PORTTRIG_print,
	.save		= PORTTRIG_save,
	.extra_opts	= PORTTRIG_tg_opts,
};

static struct xtables_target porttrig_target6 = {
	.family		= AF_INET6,
	.name		= "PORTTRIG",
	.version	= XTABLES_VERSION,
	.revision	= 0,
	.size			= XT_ALIGN(sizeof(struct xt_porttrig_target_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_porttrig_target_info)),
	.parse		= PORTTRIG_parse,
	.help		= PORTTRIG_help,
	.final_check	= PORTTRIG_final_check,
	.print		= PORTTRIG_print,
	.save		= PORTTRIG_save,
	.extra_opts	= PORTTRIG_tg_opts,
};

void _init(void)
{
	xtables_register_target(&porttrig_target);
	xtables_register_target(&porttrig_target6);
}
