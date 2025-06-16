#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include "latte_platform.h"
#include "utils.h"

static void WriteToDisk(const std::string& file,
                        const char* buf, size_t size) {
  std::ofstream out(file, std::ios::binary);
  if (!out) {
    return;
  }
  out.write(buf, size);
  std::cout << "writing " << size << " to " << file << std::endl;
  out.close();
}

static bool CollectHashStateTo(uint8_t *dest,
                               uint32_t *offset, const char* path) {
  uint32_t size = 0;
  uint8_t *buf = nullptr;
  if (!(buf = read_file(path, &size))) {
    return false;
  }
  if (*offset + size > WASM_COMMON_SEC_SIZE) {
    free(buf);
    return false;
  }
  memcpy(dest+*offset, buf, size);
  *offset += size;
  return true;
}

int main(int argc, char *argv[]) {
  /*
  * common part(Latte):
  * | num_platform(u32) | sgx_im_hash | penglai_im_hash |
  */
  uint32_t count = 0, offset = sizeof(uint32_t);
  uint8_t rt_common_part_[WASM_COMMON_SEC_SIZE] = {0};
  for (int i = 1; i < argc; i++) {
    if (CollectHashStateTo(rt_common_part_, &offset, argv[i])) {
      count++;
    }
  }
  *reinterpret_cast<uint32_t *>(rt_common_part_) = count;
  std::cout << "successfully collect " << count
            << " intermediate hash states." << std::endl;
  WriteToDisk("runtime_common.bin",
              reinterpret_cast<char*>(rt_common_part_), offset);
  return 0;
}
