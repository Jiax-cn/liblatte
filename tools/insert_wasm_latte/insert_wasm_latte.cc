#include "insert_wasm_latte.h"

#include <fstream>
#include <iostream>
#include <cstring>

#include "latte_wasm.h"
#include "utils.h"

bool LatteGroup::WasmMember::GenPortIdState() {
  if (!content_ && !(content_ = read_file(path_.c_str(), &size_))) {
    return false;
  }
  gen_portid_state(content_, size_, portid_state_);
  return true;
}

bool LatteGroup::AddWasm(const std::string& path) {
  auto wasm = std::make_shared<WasmMember>(path);
  if (wasm->GenPortIdState()) {
    members_.push_back(wasm);
    return true;
  }
  return false;
}

uint8_t *LatteGroup::GenPortIdSec(uint32_t *sec_size) {
  /*
  * PortId Section content:
  * | portable identity state 1 |
  * | portable identity state 2 |
  * ...
  */
  auto size = members_.size() * sizeof(portid_hash_state_t);
  std::vector<uint8_t> data(size, 0);
  size_t offset = 0;
  for (auto wasm : members_) {
    std::memcpy(data.data() + offset, wasm->portid_state_,
                sizeof(portid_hash_state_t));
    offset += sizeof(portid_hash_state_t);
  }

  // Serialize to wasm custom section
  return serialize_portid_section(data.data(), size, sec_size);
}

void LatteGroup::GenModifiedWasms(uint8_t *sec, uint32_t size) {
  if (!sec) {
    return;
  }

  for (auto wasm : members_) {
    const auto& out_path = wasm->GenOutputPath();
    std::ofstream out(out_path, std::ios::binary);
    if (!out) {
      continue;
    }
    out.write(reinterpret_cast<char*>(wasm->content_), wasm->size_);
    out.write(reinterpret_cast<const char*>(sec), size);
    std::cout << "writing " << wasm->size_ + size
              << " to " << out_path << std::endl;
    out.close();
  }
}

int main(int argc, char *argv[]) {
  auto group = LatteGroup();
  for (int i = 1; i < argc; i++) {
    if (!group.AddWasm(argv[i]))  {
      std::cout << "Failed to add member: " << argv[i] << std::endl;
    }
  }

  uint32_t size;
  auto *sec = group.GenPortIdSec(&size);
  if (sec) {
    std::cout << "wasm common portid section: " << std::endl;
    hexdump(sec, size);
  }

  group.GenModifiedWasms(sec, size);

  return 0;
}
