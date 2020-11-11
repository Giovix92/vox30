#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_CLASSIFY.h>
#include <linux/pkt_sched.h>

enum {
	O_SET_CLASS = 0,
};


static void
classify_help(void)
{
	printf(
"classify match options:\n"
"[!] --priority value    Match skb->priority value\n");
}
static void
classify_print(const void *ip,
      const struct xt_entry_target *target,
      int numeric)
{
	const struct xt_classify_target_info *clinfo =
		(const struct xt_classify_target_info *)target->data;
	printf(" classify match 0x%02x", clinfo->priority);
}
static void
classify_save(const void *ip,
      const struct xt_entry_target *target,
      int numeric)
{
	const struct xt_classify_target_info *clinfo =
		(const struct xt_classify_target_info *)target->data;
	printf("--priority 0x%02x", clinfo->priority);
}
static const struct xt_option_entry classify_opts[] = {
	{.name = "priority", .id = O_SET_CLASS, .type = XTTYPE_UINT32,
	 .flags = XTOPT_MAND | XTOPT_PUT, XTOPT_POINTER(struct xt_classify_target_info, priority)},
	XTOPT_TABLEEND,
};

static void classify_parse(struct xt_option_call *cb)
{
	struct xt_classify_target_info *clinfo = cb->data;

	xtables_option_parse(cb);
}

static struct xtables_match classify_match = {
	.family		= NFPROTO_UNSPEC,
	.name 		= "classify",
    .revision      = 0,
	.version 	= XTABLES_VERSION,
	.size 		= XT_ALIGN(sizeof(struct xt_classify_target_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_classify_target_info)),
	.help		= classify_help,
	.print		= classify_print,
	.save		= classify_save,
	.x6_parse	= classify_parse,
	.x6_options	= classify_opts,
};
void _init(void)
{
	xtables_register_match(&classify_match);
}
