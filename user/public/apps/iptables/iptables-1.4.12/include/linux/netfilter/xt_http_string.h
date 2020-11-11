#ifndef _IPT_HTTP_STRING_H
#define _IPT_HTTP_STRING_H

#define MAX_URL_NUM 16
#define MAX_URL_LENGTH 64
struct ipt_http_string_info
{
	char url_string[MAX_URL_NUM][MAX_URL_LENGTH];
    int url_length[MAX_URL_NUM];
    int invert;
};
#endif /* _IPT_STRING_H */
