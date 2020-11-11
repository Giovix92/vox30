/*
 *  ebt_tcpmss
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <netinet/ip.h>
#include <linux/netfilter_bridge/ebt_tcpmss.h>

enum {
    O_SET_MSS = 0,
    O_CLAMP_MSS,
};


static struct option opts[] =
{
	{ "set-mss" , required_argument, 0, O_SET_MSS },
	{ "clamp-mss-to-pmtu", no_argument, 0, O_CLAMP_MSS },
	{ 0 }
};

static void print_help()
{
    printf(
            "TCPMSS target mutually-exclusive options:\n"
            "  --set-mss value               explicitly set MSS option to specified value\n"
            "  --clamp-mss-to-pmtu           automatically clamp MSS value to (path_MTU - %d)\n",
            sizeof(struct iphdr));

}


static void init(struct ebt_entry_target *target)
{
	struct ebt_tcpmss_info *markinfo =
	   (struct ebt_tcpmss_info *)target->data;

	markinfo->mss = 0;
}

#define OPT_TCPMSS_SET   	0x01
#define OPT_TCPMSS_CLAMP   	0x02

static int parse(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_tcpmss_info *markinfo =
	   (struct ebt_tcpmss_info *)(*target)->data;
	char *end;

	
	switch (c) {
	case O_SET_MSS:
		ebt_check_option2(flags, OPT_TCPMSS_SET);
		markinfo->mss = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			ebt_print_error2("Bad --set-mss value '%s'", optarg);
		break;		
		
	case O_CLAMP_MSS:
		ebt_check_option2(flags, OPT_TCPMSS_CLAMP);
		markinfo->mss = XT_TCPMSS_CLAMP_PMTU;
			

		break;

	 default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
		            
    if ((entry->ethproto != ETH_P_IPV6 && entry->ethproto != ETH_P_IP) || entry->invflags & EBT_IPROTO)
        ebt_print_error2("set-mss value must be used with -p IPv4/IPv6 --ip-proto tcp");
    return;
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	
	struct ebt_tcpmss_info *markinfo =
	   (struct ebt_tcpmss_info *)target->data;
    if(markinfo->mss != XT_TCPMSS_CLAMP_PMTU)
        printf("--set-mss %d",markinfo->mss);
    else
        printf("--clamp-mss-to-pmtu");
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_tcpmss_info *markinfo1 =
	   (struct ebt_tcpmss_info *)t1->data;
	struct ebt_tcpmss_info *markinfo2 =
	   (struct ebt_tcpmss_info *)t2->data;

	return markinfo1->mss == markinfo2->mss;
}

static struct ebt_u_target tcpmss_target =
{
	.name		= EBT_TCPMSS_TARGET,
	.size		= sizeof(struct ebt_tcpmss_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

//static void _init(void) __attribute__ ((constructor));
//static void _init(void)
void _init(void)
{
	ebt_register_target(&tcpmss_target);
}
