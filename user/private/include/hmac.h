#ifndef _HMAC_H
#define _HMAC_H

void hmac_sha256(unsigned char* text, int text_len, unsigned char* key, int key_len,char* digest);
void hmac_sha512(unsigned char* text, int text_len, unsigned char* key, int key_len,char* digest);

#endif /* _HMAC_MD5_H */
