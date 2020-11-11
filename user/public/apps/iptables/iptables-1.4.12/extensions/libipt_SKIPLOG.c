/*
 * Shared library add-on to iptables to stop logging.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>
#include <linux/netfilter_ipv4/ip_tables.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"SKIPLOG target v%s takes no options\n",
XTABLES_VERSION);
}

static struct option opts[] = {
	{ 0 }
};

/* Initialize the target. */
static void
init(struct xt_entry_target *t, unsigned int *nfcache)
{
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct xt_entry_target **target)
{
	return 0;
}

static void
final_check(unsigned int flags)
{
}

static struct xtables_target skiplog = {
	.next		= NULL,
	.name		= "SKIPLOG",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(0),
	.userspacesize	= XT_ALIGN(0),
	.help		= &help,
	.init		= &init,
	.parse		= &parse,
	.final_check = &final_check,
	.print		= NULL,
	.save		= NULL,
	.extra_opts	= opts
};

void _init(void)
{
	xtables_register_target(&skiplog);
}
