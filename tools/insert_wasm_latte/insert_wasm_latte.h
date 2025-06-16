#ifndef LIBLATTE_TOOLS_INSERT_WASM_LATTE_INSERT_WASM_LATTE_H_
#define LIBLATTE_TOOLS_INSERT_WASM_LATTE_INSERT_WASM_LATTE_H_

#include <memory>
#include <string>
#include <vector>

#include "latte.h"

const char kOutWasmSuffix[] = "_latte";

class LatteGroup {
 public:
    class WasmMember {
     public:
      explicit WasmMember(const std::string& path) : path_(path) {}
      ~WasmMember() {
        if (content_) {
          free(content_);
        }
      }

      // Generate the path of output wasm.
      std::string GenOutputPath() {
        auto pos = path_.rfind('/');
        std::string file;
        if (pos < path_.size()) {
          file = path_.substr(pos+1);
        }
        pos = file.rfind('.');
        if (pos == std::string::npos) {
          return file + kOutWasmSuffix;
        }
        return file.substr(0, pos) + kOutWasmSuffix + file.substr(pos);
      }

      // Generate intermediate hash state of portable identity.
      bool GenPortIdState();

     public:
      std::string path_;
      uint8_t *content_ = nullptr;
      uint32_t size_;

      portid_hash_state_t portid_state_;
    };

    LatteGroup() {}
    ~LatteGroup() {}

    // Add wasm member, caculate the portable identity intermediate state.
    bool AddWasm(const std::string& path);

    // Generate group portable section.
    uint8_t *GenPortIdSec(uint32_t *sec_size);

    // Insert portable section to wasm files.
    void GenModifiedWasms(uint8_t *sec, uint32_t size);

 private:
  std::vector<std::shared_ptr<WasmMember>> members_;
};

#endif  // LIBLATTE_TOOLS_INSERT_WASM_LATTE_INSERT_WASM_LATTE_H_
