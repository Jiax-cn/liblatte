#include "verify_portable_identity.h"

#include <fstream>
#include <iostream>

#include "latte_wasm.h"
#include "utils.h"

static void WriteToDisk(const std::string& file, const char* buf, size_t size) {
  std::ofstream out(file, std::ios::binary);
  if (!out) {
    return;
  }
  out.write(buf, size);
  std::cout << "writing " << size << " to " << file << std::endl;
  out.close();
}

bool LatteWasm::DeserializePortIdSection() {
  if (!content_ && !(content_ = read_file(path_.c_str(), &size_))) {
    return false;
  }
  deserialize_portid_section(content_, size_, &portid_sec_,
                             &portid_sec_size_, &portid_sec_content_size_);
  return portid_sec_ != nullptr;
}

void LatteWasm::GenSelfPortId(portid_t portid) {
  if (!portid_sec_ && !DeserializePortIdSection()) {
    return;
  }

  // Generate intermediate hash state of portable identity.
  portid_hash_state_t hash_state;
  gen_portid_state(content_, size_ - portid_sec_size_, hash_state);

  derive_portid(hash_state, portid_sec_, portid_sec_size_, portid);

  auto pos = path_.rfind('/');
  std::string file;
  if (pos < path_.size()) {
    file = path_.substr(pos+1);
  }
  auto out_path = file + kOutPortIdSuffix;
  WriteToDisk(out_path, reinterpret_cast<char*>(portid), sizeof(portid_t));
}

uint32_t LatteWasm::GetGroupSize() {
  if (!portid_sec_ && !DeserializePortIdSection()) {
    return 0;
  }
  return portid_sec_content_size_ / sizeof(portid_hash_state_t);
}

void LatteWasm::DerivePortId(uint32_t idx) {
  uint32_t offset = portid_sec_size_ - portid_sec_content_size_
                    + idx * sizeof(portid_hash_state_t);
  portid_hash_state_t *hash_state =
    reinterpret_cast<portid_hash_state_t*>(portid_sec_ + offset);

  portid_t portid = {0};
  derive_portid(*hash_state, portid_sec_, portid_sec_size_, portid);

  std::cout << "portid of : " << idx << std::endl;
  hexdump(portid, sizeof(portid_t));
}

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    auto latte_wasm = LatteWasm(argv[i]);
    portid_t portid = {0};
    latte_wasm.GenSelfPortId(portid);

    std::cout << "portid of : " << argv[i] << std::endl;
    hexdump(portid, sizeof(portid_t));

    for (uint32_t j = 0; j < latte_wasm.GetGroupSize(); j++) {
      latte_wasm.DerivePortId(j);
    }
  }

  return 0;
}
