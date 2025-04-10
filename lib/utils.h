#ifndef LIB_UTILS_H_
#define LIB_UTILS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hexdump(void *buf, uint32_t size);

uint32_t get_file_size(const char *file_name);

uint32_t read_file_to_buf(const char *file_name, uint8_t *file_buf,
                          uint32_t buf_size);

uint8_t *read_file(const char *file_name, uint32_t *ret_size);

#ifdef __cplusplus
}
#endif

#endif  // LIB_UTILS_H_
