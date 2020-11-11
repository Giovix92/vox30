#ifndef _DRND_CLSOCK_H_
#define _DRND_CLSOCK_H_

/* Define domain type value */
#define DOMAIN_TYPE_UNKNOWN  (0x0000)
#define DOMAIN_TYPE_DATA     (0x0001)
#define DOMAIN_TYPE_TR069    (0x0002)
#define DOMAIN_TYPE_NTP      (0x0003)
#define DOMAIN_TYPE_VOIP     (0x0004)
#define DOMAIN_TYPE_IPTV     (0x0005)
#define DOMAIN_TYPE_CLI      (0x0006)
#define DOMAIN_TYPE_IPPHONE  (0x0007)

int cls_handle(void);
void sc_dns_answer_parse(int dtype, char *msg, int *len);
int sc_dtype_parse(char *dns_pkt, int *len);

#endif
