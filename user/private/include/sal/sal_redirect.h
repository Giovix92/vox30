#ifndef _SAL_REDIRECT_H_
#define _SAL_REDIRECT_H_

#define SHARE_UPNP_DESC "/tmp/share_upnp_desc"
#define MAX_UPNP_DESC   100
#define SHARE_UPNP_SIZE (sizeof(UPNP_DESC)*MAX_UPNP_DESC)

typedef struct UPNP_DESC_S
{
    struct UPNP_DESC_S *next;
    struct UPNP_DESC_S *first;
    unsigned short eport;
    unsigned short iport;
    unsigned short rulenable;
    short proto;
    char addr[32];
    char str[256];
    time_t serviceStartTime;
    time_t duration;
}UPNP_DESC;

void sal_init_redirection(void);
void sal_upnp_get_natt_table(char *outbuf);
char *sal_http_random_key_generate(char* remote_ip);
#ifdef CONFIG_SUPPORT_SJCL_ENCRYPT
char *sal_http_random_salt_generate(char* remote_ip);
#endif
char *sal_redirect_get_redirect_t(void);
int sal_redirect_set_redirect_t(char *value);

#endif
