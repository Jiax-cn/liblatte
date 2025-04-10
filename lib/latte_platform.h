#ifndef LIB_LATTE_PLATFORM_H_
#define LIB_LATTE_PLATFORM_H_

#include <stdint.h>

#define SE_PAGE_SIZE 0x1000

#define WASM_PLD_SEC_SIZE 0x10000
#define WASM_COMMON_SEC_SIZE 0x1000
#define WASM_SEC_SIZE (WASM_PLD_SEC_SIZE + WASM_COMMON_SEC_SIZE)

typedef struct _sgx_hash_state_t {
    uint64_t size;                          // number of blocks updated
    uint64_t offset;                        // offset of payload section
    uint8_t digest[32];                     // sha-256 internal state
} sgx_hash_state_t;                         // size: 48

#define PENGLAI_PLD_SEC_ADDR       0xfffffff000000000UL

typedef struct _penglai_hash_state_t {
    unsigned long offset;       /*!< offset of payload section */
    unsigned long total[2];     /*!< number of bytes processed      */
    unsigned long state[8];     /*!< intermediate digest state      */
    unsigned char buffer[64];   /*!< data block being processed     */
} penglai_hash_state_t;         /* size: 152 */

#endif  // LIB_LATTE_PLATFORM_H_
