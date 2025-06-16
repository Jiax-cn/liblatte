#ifndef LIBLATTE_TOOLS_VERIFY_PORTABLE_IDENTITY_VERIFY_PORTABLE_IDENTITY_H_
#define LIBLATTE_TOOLS_VERIFY_PORTABLE_IDENTITY_VERIFY_PORTABLE_IDENTITY_H_

#include <string>

#include "latte.h"

const char kOutPortIdSuffix[] = ".id";

class LatteWasm {
 public:
  explicit LatteWasm(const std::string& path) : path_(path) {}
  ~LatteWasm() {
    if (content_) {
      free(content_);
    }
  }

  // Caculate self portable identity without embedded intermediate hash.
  void GenSelfPortId(portid_t portid);

  // Get the number of wasm members.
  uint32_t GetGroupSize();

  // Derive the portable identity of the ${idx}th wasm.
  void DerivePortId(uint32_t idx);

 private:
  // Deserialize wasm and get pointer to portid section.
  bool DeserializePortIdSection();

 public:
  std::string path_;
  uint8_t *content_ = nullptr;
  uint32_t size_;

  uint8_t *portid_sec_ = nullptr;
  uint32_t portid_sec_size_;
  uint32_t portid_sec_content_size_;
};

#endif  // LIBLATTE_TOOLS_VERIFY_PORTABLE_IDENTITY_VERIFY_PORTABLE_IDENTITY_H_
