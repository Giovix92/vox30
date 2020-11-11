#ifndef __COMMON_CRYPT_H__
#define __COMMON_CRYPT_H__
typedef struct _crypt_h{
    unsigned char rand[32];
    unsigned char vec[32];
    unsigned char size[32];
}crypt_hd;
#define HEADER_POSITION 100
#define CRYPT_POSITION 200
#define CRYPT_DEFAULT_XML "/etc/default.xml"
int crypt_get_public_key(unsigned char* key, unsigned char *vec);
int crypt_get_default_key(int fd, unsigned char* key, unsigned char *vec);
int crypt_get_private_key(unsigned char *key, unsigned char *vec);
void crypt_init_key(unsigned char* key, unsigned char *vec);
char *crypt_xml_encryption(void *str, int len, unsigned char *key);
char *crypt_xml_decryption(void *str, int len, unsigned char *key);
char *crypt_xml_key_encryption(void *str, int len, int fd, unsigned char *key);
int crypt_xml_key_decryption(int fd, void *str, unsigned char *key, int offset);
int crypt_encrypt_buffer2file_p(void *str, int len, int fd);
int crypt_decrypt_buffer2file_p(void *str, int len, int fd);
int crypt_encrypt_buffer2file_c(void *str, int len, int fd);
int crypt_decrypt_buffer2file_c(void *str, int len, int fd);
int crypt_encrypt_file2buffer_p(int fd, void *out);
int crypt_decrypt_file2buffer_p(int fd, void *out);
int crypt_encrypt_file2buffer_c(int fd, void *out);
int crypt_decrypt_file2buffer_c(int fd, void *out);
char *crypt_xml_encryption_p(void *str, int len);
char *crypt_xml_encryption_c(void *str, int len);
char *crypt_xml_decryption_p(void *str);
char *crypt_xml_decryption_c(void *str);
#ifdef CONFIG_SUPPORT_SJCL_ENCRYPT
#define SJCL_DEFAULT_ITER       1000
#define SJCL_DEFAULT_DK_LEN     16
#define SJCL_DEFAULT_TAG_LEN    8
int sjcl_pbkdf2_aes_ccm_encrypt( const char* password, int password_len,
                    const unsigned char* salt, int salt_len,
                    const unsigned char* ccm_nonce, int nonce_len, 
                    const unsigned char* ccm_pt, int pt_len, int tag_len,
                    unsigned char* out);
int sjcl_pbkdf2_aes_ccm_decrypt( const char* password, int password_len,
                    const unsigned char* salt, int salt_len,
                    const unsigned char* ccm_nonce, int nonce_len, 
                    const unsigned char* ccm_ct, int ct_len, int tag_len,
                    unsigned char* out);
int sjcl_encrypt_data_format(char* data, char* dk, int dk_len, unsigned char* out);
int sjcl_paser_decrypt_from_query(char* querry, char* dk, int dk_len, unsigned char* out);
unsigned char* sjcl_get_pwd_dk(char* password, int password_len, unsigned char* salt, int salt_len);
#endif
enum{
CRYPT_PUBLIC_KEY,
CRYPT_PRIVATE_KEY,
CRYPT_DEFAULT_KEY,
};

enum{
CRYPT_AES_DECRYPT,
CRYPT_AES_ENCRYPT,
};


#define AES_KEY_FILE "/mnt/2/.p"

unsigned char * crypt_digital_signature(void * data, int size);

#endif
