#include <stdbool.h>
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_mark.h>

struct xt_mark_info {
	unsigned long mark, mask;
	uint8_t invert;
};

enum {
	O_MARK = 0,
};

static void sc_mark_mt_help(void)
{
	printf(
"sc_mark match options:\n"
"[!] --mark value[/mask]    Match nfmark value with optional mask\n");
}

static const struct xt_option_entry sc_mark_mt_opts[] = {
	{.name = "sc_mark", .id = O_MARK, .type = XTTYPE_MARKMASK32,
	 .flags = XTOPT_MAND | XTOPT_INVERT},
	XTOPT_TABLEEND,
};

static void sc_mark_mt_parse(struct xt_option_call *cb)
{
	struct xt_mark_mtinfo1 *info = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		info->invert = true;
	info->mark = cb->val.mark;
	info->mask = cb->val.mask;
}

static void sc_mark_parse(struct xt_option_call *cb)
{
	struct xt_mark_info *markinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		markinfo->invert = 1;
	markinfo->mark = cb->val.mark;
	markinfo->mask = cb->val.mask;
}

static void sc_print_mark(unsigned int mark, unsigned int mask)
{
	if (mask != 0xffffffffU)
		printf(" 0x%x/0x%x", mark, mask);
	else
		printf(" 0x%x", mark);
}

static void
sc_mark_mt_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mark_mtinfo1 *info = (const void *)match->data;

	printf(" sc_mark match");
	if (info->invert)
		printf(" !");
	sc_print_mark(info->mark, info->mask);
}

static void
sc_mark_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mark_info *info = (const void *)match->data;

	printf(" SC_MARK match");

	if (info->invert)
		printf(" !");
	
	sc_print_mark(info->mark, info->mask);
}

static void sc_mark_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mark_mtinfo1 *info = (const void *)match->data;

	if (info->invert)
		printf(" !");

	printf(" --mark");
	sc_print_mark(info->mark, info->mask);
}

static void
sc_mark_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mark_info *info = (const void *)match->data;

	if (info->invert)
		printf(" !");
	
	printf(" --mark");
	sc_print_mark(info->mark, info->mask);
}

static struct xtables_match sc_mark_mt_reg[] = {
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "sc_mark",
		.revision      = 0,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_mark_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_mark_info)),
		.help          = sc_mark_mt_help,
		.print         = sc_mark_print,
		.save          = sc_mark_save,
		.x6_parse      = sc_mark_parse,
		.x6_options    = sc_mark_mt_opts,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "sc_mark",
		.revision      = 1,
		.family        = NFPROTO_UNSPEC,
		.size          = XT_ALIGN(sizeof(struct xt_mark_mtinfo1)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_mark_mtinfo1)),
		.help          = sc_mark_mt_help,
		.print         = sc_mark_mt_print,
		.save          = sc_mark_mt_save,
		.x6_parse      = sc_mark_mt_parse,
		.x6_options    = sc_mark_mt_opts,
	},
};

void _init(void)
{
	xtables_register_matches(sc_mark_mt_reg, ARRAY_SIZE(sc_mark_mt_reg));
}
