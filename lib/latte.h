#ifndef LIB_LATTE_H_
#define LIB_LATTE_H_

#include <stdint.h>
#include <stdbool.h>

#include "crypto/sha256.h"
#include "crypto/sm3.h"
#include "latte_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Portable Identity */

    /** Definition of portid_t
     * 
     *   1. hash_update(intermediate_hash_state)
     *   2. hash_update(common_part)
     * Hash algorithm: SHA256 */
typedef uint8_t portid_t[SHA256_DIGEST_LENGTH];

    /* portid_hash_state_t = hash(raw_wasm) */
typedef uint8_t portid_hash_state_t[SHA256_DIGEST_LENGTH];

    /* Generate the raw wasm's intermediate hash state. */
void gen_portid_state(uint8_t *wasm, uint32_t wasm_size,
                      portid_hash_state_t ret_hash_state);

    /** Generate the portid */
void derive_portid(portid_hash_state_t hash_state, uint8_t *common_part,
                   uint32_t common_part_size, portid_t ret_portid);


/* TEE Common */

    /** Build the aligned wasm section from wasm stream.
     *  Layout: | 0x00 .. | wasm | wasm size(uint32_t) | runtime common part |
     *  Return: aligned wasm section */
uint8_t *build_aligned_wasm_sec(uint8_t *wasm, uint32_t wasm_size,
                                uint8_t *common_part, uint32_t alignment,
                                uint32_t *ret_aligned_size);


/* SGX */

typedef uint8_t latte_sgx_measurement_t[SHA256_DIGEST_LENGTH];

    /* Update the hash state in common part using the aligned buffer. */
void sgx_update_common_part(sgx_hash_state_t in_hash_state,
                            uint8_t *aligned_buf, uint32_t aligned_buf_size,
                            sgx_hash_state_t *ret_hash_state);

    /* Caculates a series of intermediate hash for measurement derivation. */
bool sgx_init_state_list(sgx_hash_state_t meta_hash_state);

    /* Derives final measurement using common part.
    *  Return: int - 0 or failure */
void sgx_derive_common_part(sgx_hash_state_t hash_state, uint8_t *common_part,
                            latte_sgx_measurement_t ret_mr);

    /* Finish the derivation and return the final measurement. */
void sgx_derive_rest_data(sgx_hash_state_t hash_state, uint8_t *aligned_d,
                          uint32_t aligned_s, latte_sgx_measurement_t ret_mr);

    /* Derives final measurement using wasm binary and common part, 
     * from the caculated series of intermediate hash.
    *  Return: int - 0 or failure */
bool sgx_derive_hardcode_wasm(uint8_t *wasm, uint32_t wasm_size,
                             uint8_t *common_part,
                             latte_sgx_measurement_t ret_mr);

    /* Derives final measurement using portable identity and common part.
    *  Return: int - 0 or failure */
bool sgx_derive_hardcode_portid(sgx_hash_state_t hash_state, portid_t portid,
                               uint8_t *common_part,
                               latte_sgx_measurement_t ret_mr);


/* Penglai */

#define SM3_DIGEST_LENGTH 32
typedef uint8_t latte_penglai_measurement_t[SM3_DIGEST_LENGTH];

void penglai_update_common_part(penglai_hash_state_t in_hash_state,
                                uint8_t *aligned_buf,
                                uint32_t aligned_buf_size,
                                penglai_hash_state_t *ret_hash_state);

bool penglai_init_state_list(penglai_hash_state_t meta_hash_state);

void penglai_derive_common_part(penglai_hash_state_t hash_state,
                                uint8_t *common_part,
                                unsigned long nonce,
                                latte_penglai_measurement_t ret_mr);

void penglai_derive_rest_data(penglai_hash_state_t hash_state,
                              uint8_t *aligned_d, uint32_t aligned_s,
                              unsigned long nonce,
                              latte_penglai_measurement_t ret_mr);

bool penglai_derive_hardcode_wasm(uint8_t *wasm, uint32_t wasm_size,
                                  uint8_t *common_part, unsigned long nonce,
                                  latte_penglai_measurement_t ret_mr);

bool penglai_derive_hardcode_portid(penglai_hash_state_t hash_state,
                                    portid_t portid, uint8_t *common_part,
                                    unsigned long nonce,
                                    latte_penglai_measurement_t ret_mr);

#ifdef __cplusplus
}
#endif

#endif /* LIB_LATTE_H_ */
