/* Shared library add-on to iptables to add http string matching support. 
 * 
 * Copyright (C) Oliver.Hao <oliver_hao@sdc.sercomm.com>
 */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif

#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_http_string.h>

//#define HTTP_STRING_DEBUG
/* Function which prints out usage message. */
static void http_string_help(void)
{
    printf("HTTP STRING match options:\n"
            "--string [!] string             Match a string in a packet\n");
}

static struct option http_string_opts[] = {
    { "file", 1, NULL, 'f' },
    { .name = NULL }
};

/* Initialize the match. */
    static void
http_string_init(struct xt_entry_match *m)
{
    //	*nfcache |= NFC_UNKNOWN;
}

/* Final check; must have specified --string. */
static void http_string_check(unsigned int flags)
{
    if (!flags)
        xtables_error(PARAMETER_PROBLEM,
                "STRING match: You must specify `--string'");
}

int get_http_match_condition(char *file, struct ipt_http_string_info *http_match_info)
{
    int fp;
    int i=0;

    memset(http_match_info,0,sizeof(struct ipt_http_string_info));
    if((fp = open(file,O_RDONLY)) < 0)
    {
        printf("open %s fail\n",file);
        return -1;
    }

    i=read(fp,http_match_info,sizeof(struct ipt_http_string_info));
    if(i != sizeof(struct ipt_http_string_info))
    {
        printf("read %s fail\n",file);
        return -1;
    }
    close(fp);
    return 1;
}

#ifdef HTTP_STRING_DEBUG
void dump_http_string_list(struct ipt_http_string_info *http_match_info)
{
    struct ipt_http_string_info *hs_info=NULL;
    int j=0;

    printf("	url string list:\n");
    for(j=0;j<MAX_URL_NUM;j++)
    {
        if(hs_info->url_length[j]>0)
            printf("		len=%d,str=%s\n",
                    hs_info->url_length[j],hs_info->url_string[j]);
    }
}
#endif

/* Function which parses command options; returns true if it
   ate an option */
static int http_string_parse(int c, char **argv, int invert, unsigned int *flags,
        const void *entry,struct xt_entry_match **match)
{
    int i;
    struct ipt_http_string_info *http_match_info = (struct ipt_http_string_info *)(*match)->data;

    switch (c) {
        case 'f':
            xtables_check_inverse(optarg, &invert, &optind, 0);
            if(get_http_match_condition(argv[optind-1], http_match_info) < 0)
                return -1;
            if(invert)
            {
                for(i=0;i<MAX_URL_NUM;i++)
                    http_match_info->invert = 1;
            }
            *flags = 1;
            break;

        default:
            return 0;
    }
#ifdef HTTP_STRING_DEBUG
    dump_http_string_list(http_match_info);
#endif
    return 1;
}


/* Prints out the matchinfo. */
static void http_string_print(const void *ip,const struct xt_entry_match *match,
        int numeric)
{
    printf("HTTP STRING match ");
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void http_string_save(const void *ip, const struct xt_entry_match *match)
{
    printf("Can not Support save ");
}

static struct xtables_match http_string = {
    .name          = "http_string",
    .version       = XTABLES_VERSION,
    .family		= NFPROTO_IPV4,
    .size          = XT_ALIGN(sizeof(struct ipt_http_string_info)),
    .userspacesize = XT_ALIGN(sizeof(struct ipt_http_string_info)),
    .help          = http_string_help,
    .init          = http_string_init,
    .parse         = http_string_parse,
    .final_check   = http_string_check,
    .print         = http_string_print,
    .save          = http_string_save,
    .extra_opts    = http_string_opts,
};

void _init(void)
{
    xtables_register_match(&http_string);
}
