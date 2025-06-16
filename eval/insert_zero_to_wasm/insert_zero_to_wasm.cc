#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "latte.h"
#include "latte_wasm.h"
#include "utils.h"

#define OUT_PREFIX "_out.wasm"

uint8_t *encode_custom_section(const char *name, uint8_t *data,
    uint32_t data_length, uint32_t *ret_section_length) {
  uint8_t *section = nullptr;
  uint32_t name_length, content_length, section_length;
  uint32_t offset = 0;
  const wasm_section_idx sec_idx = {WASM_CUSTOM_SEC_IDX};

  // wasm custom section:
  //  id | content length(leb) | content
  //  id | content length(leb) | name length(leb) | section name | custom data
  name_length = strlen(name);
  content_length = leb128_encode_uint32_len(name_length)
                   + name_length + data_length;
  section_length = sizeof(wasm_section_idx)
                   + leb128_encode_uint32_len(content_length) + content_length;

  if (!(section = reinterpret_cast<uint8_t *>(malloc(section_length)))) {
    return nullptr;
  }

  memset(section, 0, section_length);

  // custom section id;
  memcpy(section+offset, &sec_idx, sizeof(wasm_section_idx));
  offset += sizeof(wasm_section_idx);

  // custom content length;
  offset += leb128_encode_uint32(content_length, section+offset);

  // custom name length;
  offset += leb128_encode_uint32(name_length, section+offset);

  // custom name;
  memcpy(section+offset, name, name_length);
  offset += name_length;

  // data;
  memcpy(section+offset, data, data_length);
  offset += data_length;

  *ret_section_length = offset;
  return section;
}


int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: insert_zero_to_wasm test.wasm [size]\n");
    return 0;
  }

  const char *f_in = argv[1];
  int d_size = atoi(argv[2]);

  uint32_t f_size;
  uint8_t *f_buf = read_file(f_in, &f_size);

  int f;
  char *f_out = reinterpret_cast<char *>(
                  malloc(strlen(f_in) + 1 + strlen(OUT_PREFIX)));
  memcpy(f_out, f_in, strlen(f_in));
  memcpy(f_out+strlen(f_in), OUT_PREFIX, strlen(OUT_PREFIX)+1);
  if ((f = open(f_out, O_RDWR | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU)) == -1) {
    printf("file open failed: open file %s failed.\n", f_out);
    return 0;
  }

  uint32_t w_size = write(f, f_buf, f_size);

  if (w_size != f_size) {
    printf("Writing WASM source error: size: %d invalid!\n", w_size);
    return 0;
  }

  uint8_t *data = reinterpret_cast<uint8_t *>(malloc(d_size));
  memset(data, 0, d_size);
  uint32_t c_size = 0;
  uint8_t *c_sec = encode_custom_section("empty", data, d_size, &c_size);

  w_size = write(f, c_sec, c_size);

  if (w_size != c_size) {
    printf("Writing WASM source error: size: %d invalid!\n", w_size);
    return 0;
  }

  free(data);
  free(c_sec);

  close(f);
  return 0;
}
