#ifndef LUAT_CRYPTO_H
#define LUAT_CRYPTO_H
#include "luat_base.h"
#include "mbedtls/md.h"

#define LUAT_CRYPTO_AES_ECB 1
#define LUAT_CRYPTO_AES_CBC 2
#define LUAT_CRYPTO_AES_CTR 3
#define LUAT_CRYPTO_AES_CFB 4
#define LUAT_CRYPTO_AES_OFB 5

#define LUAT_CRYPTO_AES_PAD_ZERO 1
#define LUAT_CRYPTO_AES_PAD_5 2
#define LUAT_CRYPTO_AES_PAD_7 3

typedef struct
{
    size_t key_len;
	mbedtls_md_context_t *ctx;
}luat_crypt_stream_t;

int luat_crypto_trng(char* buff, size_t len);

int luat_crypto_md5_simple(const char* str, size_t str_size, void* out_ptr);
int luat_crypto_hmac_md5_simple(const char* str, size_t str_size, const char* mac, size_t mac_size, void* out_ptr);

int luat_crypto_sha1_simple(const char* str, size_t str_size, void* out_ptr);
int luat_crypto_hmac_sha1_simple(const char* str, size_t str_size, const char* mac, size_t mac_size, void* out_ptr);

int luat_crypto_sha256_simple(const char* str, size_t str_size, void* out_ptr);
int luat_crypto_hmac_sha256_simple(const char* str, size_t str_size, const char* mac, size_t mac_size, void* out_ptr) ;

int luat_crypto_sha512_simple(const char* str, size_t str_size, void* out_ptr) ;
int luat_crypto_hmac_sha512_simple(const char* str, size_t str_size, const char* mac, size_t mac_size, void* out_ptr) ;

int luat_crypto_cipher_list(const char** list, size_t* len);
int luat_crypto_cipher_suites(const char** list, size_t* len);

int luat_crypto_md(const char* md, const char* str, size_t str_size, void* out_ptr, const char* key, size_t key_len);
int luat_crypto_md_file(const char* md, void* out_ptr, const char* key, size_t key_len, const char* path);

int luat_crypto_md_stream_init(const char* md, const char* key, luat_crypt_stream_t *stream);
int luat_crypto_md_stream_update(const char* md, const char* str, size_t str_size, luat_crypt_stream_t *stream);
int luat_crypto_md_stream_finish(const char* md, void* out_ptr, luat_crypt_stream_t *stream);
#endif
