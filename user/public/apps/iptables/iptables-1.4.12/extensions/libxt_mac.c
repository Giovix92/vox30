#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <xtables.h>
#include <linux/netfilter/xt_mac.h>

enum {
	O_MAC = 0,
};

static void mac_help(void)
{
	printf(
"mac match options:\n"
"[!] --mac-source XX:XX:XX:XX:XX:XX\n"
"				Match source MAC address\n");
}

#ifdef CONFIG_SCM_SUPPORT
#define  WILDCHAR '*'
static const struct option mac_opts[] = {
	{.name = "mac-source",      .has_arg = true,  .val = '1'},
	XT_GETOPT_TABLEEND,
};
#warning sercomm mac wildcard
static void
parse_mac(const char *mac, struct xt_mac_info *info)
{
	unsigned int i = 0;
	char mac_mac[18] = {0};
	char mac_msk[18] = {0};

	if (strlen(mac) != ETH_ALEN*3-1)
		xtables_error(PARAMETER_PROBLEM, "Bad mac address `%s'", mac);

	// remove wildcard char *
	sprintf(mac_mac,mac);
	for(i=0;i<strlen(mac_mac);i++)
		if(mac_mac[i] == '*') mac_mac[i] = '0';

	for (i = 0; i < ETH_ALEN; i++) {
			long number;
			char *end;

			number = strtol(mac_mac + i*3, &end, 16);

			if (end == mac_mac + i*3 + 2
				&& number >= 0
				&& number <= 255)
					info->srcaddr[i] = number;
			else
				xtables_error(PARAMETER_PROBLEM,
							   "Bad mac address `%s'", mac);
	}

	// leave wildcard char *
	sprintf(mac_msk,mac);
	for(i=0;i<strlen(mac_msk);i++)
		if(mac_msk[i] != WILDCHAR && mac_msk[i] != ':') mac_msk[i] = 'F';
		else if(mac_msk[i] == WILDCHAR) mac_msk[i] = '0';

		for (i = 0; i < ETH_ALEN; i++) {
			long number;
			char *end;

			number = strtol(mac_msk + i*3, &end, 16);

			if (end == mac_msk + i*3 + 2
				&& number >= 0
				&& number <= 255)
					info->srcaddrmsk[i] = number;
			else
				xtables_error(PARAMETER_PROBLEM,
				   "Bad mac address `%s'", mac);
	}
}

static int
mac_parse(int c, char **argv, int invert, unsigned int *flags,
          const void *entry, struct xt_entry_match **match)
{
	struct xt_mac_info *macinfo = (struct xt_mac_info *)(*match)->data;

	switch (c) {
	case '1':
		parse_mac(optarg, macinfo);
		if (invert)
			macinfo->invert = 1;
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}
static void mac_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "You must specify `--mac-source'");
}
#else
#define s struct xt_mac_info
static const struct xt_option_entry mac_opts[] = {
	{.name = "mac-source", .id = O_MAC, .type = XTTYPE_ETHERMAC,
	 .flags = XTOPT_MAND | XTOPT_INVERT | XTOPT_PUT,
	 XTOPT_POINTER(s, srcaddr)},
	XTOPT_TABLEEND,
};
#undef s

static void mac_parse(struct xt_option_call *cb)
{
	struct xt_mac_info *macinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		macinfo->invert = 1;
}
#endif


static void print_mac(const unsigned char *macaddress)
{
	unsigned int i;

	printf(" %02X", macaddress[0]);
	for (i = 1; i < ETH_ALEN; ++i)
		printf(":%02X", macaddress[i]);
}

static void
mac_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mac_info *info = (void *)match->data;
	printf(" MAC");

	if (info->invert)
		printf(" !");
	
	print_mac(info->srcaddr);
}

static void mac_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mac_info *info = (void *)match->data;

	if (info->invert)
		printf(" !");

	printf(" --mac-source");
	print_mac(info->srcaddr);
}

static struct xtables_match mac_match = {
	.family		= NFPROTO_UNSPEC,
 	.name		= "mac",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_mac_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_mac_info)),
	.help		= mac_help,
	.print		= mac_print,
	.save		= mac_save,
#ifdef CONFIG_SCM_SUPPORT
	.parse		= mac_parse,
	.extra_opts	= mac_opts,
	.final_check	= mac_check,
#else
	.x6_parse	= mac_parse,
	.x6_options	= mac_opts,
#endif

};

void _init(void)
{
	xtables_register_match(&mac_match);
}
