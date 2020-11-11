#ifndef __MINI_HTTP_H__
#define __MINI_HTTP_H__
#define HTTPD_SOCK "/tmp/httpd.sock"
#define HTTPD_UPDATE_VPN_PEER_CLIENT 0x1
void http_call(int cmd, unsigned char *data, int len);
#endif
