#ifndef LIBLATTE_SRC_LATTE_WASM_H_
#define LIBLATTE_SRC_LATTE_WASM_H_

#include <stdint.h>
#include <stdlib.h>

#define WASM_CUSTOM_SEC_IDX 0
typedef uint8_t wasm_section_idx[1];

#ifdef __cplusplus
extern "C" {
#endif

uint32_t leb128_encode_uint32_len(uint32_t msg);

uint32_t leb128_encode_uint32(uint32_t msg, uint8_t *out);

uint32_t leb128_decode_uint32(uint8_t *in, uint32_t *ret_msg);

uint8_t *serialize_portid_section(uint8_t *data, uint32_t data_length,
                                  uint32_t *ret_section_length);

void deserialize_portid_section(uint8_t *wasm_buf, uint32_t wasm_size,
                                uint8_t **ret_portid_section,
                                uint32_t *ret_portid_section_size,
                                uint32_t *ret_section_content_size);

#ifdef __cplusplus
}
#endif

#endif  // LIBLATTE_SRC_LATTE_WASM_H_
