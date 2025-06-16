#include "latte_wasm.h"

#include <string.h>
#include "latte.h"

#define WASM_HEADER_LENGTH 8
#define LATTE_SECTION_NAME "portid"
#define CONST_STR_SIZE(s) (sizeof(s) - 1)

uint32_t leb128_encode_uint32_len(uint32_t msg) {
    if (msg >> 28)
        return 5;
    else if (msg >> 21)
        return 4;
    else if (msg >> 14)
        return 3;
    else if (msg >> 7)
        return 2;
    else
        return 1;
}

uint32_t leb128_encode_uint32(uint32_t msg, uint8_t *out) {
    uint32_t i, offset = 0;

    if (msg == 0) {
        out[0] = 0;
        return 1;
    }

    for (i = 0; i < 5; i++) {
        uint8_t cur_byte = (msg >> i * 7) % (1 << 7);
        if (msg >> i * 7)
            out[offset++] = cur_byte | 1 << 7;
        else
            break;
    }

    out[offset-1] &= 0x7f;

    return offset;
}

uint32_t leb128_decode_uint32(uint8_t *in, uint32_t *ret_msg) {
    uint32_t msg = 0;
    uint32_t offset = 0, i = 0;
    *ret_msg = 0;

    for (i = 0; i != 4; i++) {
        if (in[offset] & 0x80) {
            msg += (in[offset++] & 0x7f) << 7 * i;
        }
    }

    msg += (in[offset] & 0x7f) << 7 * offset;
    offset++;

    *ret_msg = msg;
    return offset;
}

uint8_t *serialize_portid_section(uint8_t *data, uint32_t data_length,
                                  uint32_t *ret_section_length) {
    uint8_t *section = NULL;
    uint32_t name_size = 0, content_size = 0, section_size = 0, offset = 0;
    const wasm_section_idx sec_idx = {WASM_CUSTOM_SEC_IDX};

    /* wasm custom section: 
      id | content size(leb) | name size(leb) | section name | portid data */
    name_size = CONST_STR_SIZE(LATTE_SECTION_NAME);
    content_size = leb128_encode_uint32_len(name_size)
                   + name_size + data_length;
    section_size = sizeof(wasm_section_idx)
                   + leb128_encode_uint32_len(content_size) + content_size;

    if ((section = (uint8_t *)malloc(section_size)) == NULL)
        return NULL;
    memset(section, 0, section_size);

    // custom section id;
    memcpy(section+offset, sec_idx, sizeof(wasm_section_idx));
    offset += sizeof(wasm_section_idx);

    // custom content length;
    offset += leb128_encode_uint32(content_size, section+offset);

    // custom name length;
    offset += leb128_encode_uint32(name_size, section+offset);

    // custom name;
    memcpy(section+offset, LATTE_SECTION_NAME, name_size);
    offset += name_size;

    // data;
    memcpy(section+offset, data, data_length);
    offset += data_length;

    *ret_section_length = offset;
    return section;
}

void deserialize_portid_section(uint8_t *wasm_buf, uint32_t wasm_size,
                                uint8_t **ret_portid_section,
                                uint32_t *ret_portid_section_size,
                                uint32_t *ret_section_content_size) {
    uint32_t offset = 0;

    offset += WASM_HEADER_LENGTH;
    while (offset < wasm_size) {
        uint8_t *sec = wasm_buf+offset;
        uint32_t section_size = 0, sec_offset = 0;
        uint8_t section_id = 0;

        // section id;
        section_id = sec[sec_offset++];

        // section length;
        sec_offset += leb128_decode_uint32(sec+sec_offset, &section_size);

        if (section_id == WASM_CUSTOM_SEC_IDX) {
            uint32_t name_size = 0, content_offset = 0;

            // name length;
            content_offset = leb128_decode_uint32(sec+sec_offset, &name_size);

            // name;
            if (name_size == CONST_STR_SIZE(LATTE_SECTION_NAME)
                && !memcmp(sec+sec_offset+content_offset, LATTE_SECTION_NAME,
                           name_size)) {
                *ret_portid_section = sec;
                *ret_portid_section_size = sec_offset + section_size;
                *ret_section_content_size = section_size
                                            - content_offset - name_size;
                return;
            }
        }

        offset += sec_offset + section_size;
    }
    return;
}
