#ifndef LIB_CRYPTO_SHA256_H_
#define LIB_CRYPTO_SHA256_H_

#include <stdint.h>
#include <stddef.h>

#define DECLARE_IS_ENDIAN   \
        const union {       \
            long one;       \
            char little;    \
        } is_endian = { 1 }

#define IS_LITTLE_ENDIAN (is_endian.little != 0)
#define IS_BIG_ENDIAN    (is_endian.little == 0)

#define SHA_LONG unsigned int
#define SHA256_DIGEST_LENGTH    32

#define SHA_LBLOCK  16
#define SHA_CBLOCK   (SHA_LBLOCK*4)

typedef struct SHA256state_st {
    SHA_LONG h[8];
    SHA_LONG Nl, Nh;
    SHA_LONG data[SHA_LBLOCK];
    unsigned int num, md_len;
} SHA256_CTX;

#ifdef  __cplusplus
extern "C" {
#endif

void SHA256_Init(SHA256_CTX *c);
void SHA256_Update(SHA256_CTX *c, const unsigned char *data, size_t len);
void SHA256_Final(unsigned char *md, SHA256_CTX *c);
void SHA256(const unsigned char *d, size_t n, unsigned char *md);

#ifdef  __cplusplus
}
#endif

#endif  // LIB_CRYPTO_SHA256_H_
