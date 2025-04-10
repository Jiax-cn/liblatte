#ifndef LIB_CRYPTO_SM3_H
#define LIB_CRYPTO_SM3_H

typedef struct _sm3_context {
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[8];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}sm3_context;

#ifdef  __cplusplus
extern "C" {
#endif

void sm3_init(sm3_context *ctx);

void sm3_update(sm3_context *ctx, unsigned char *input, int ilen);

void sm3_final(sm3_context *ctx, unsigned char output[32]);

#ifdef  __cplusplus
}
#endif

#endif  // LIB_CRYPTO_SM3_H
