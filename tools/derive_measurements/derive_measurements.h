#ifndef LIBLATTE_TOOLS_DERIVE_MEASUREMENTS_DERIVE_MEASUREMENTS_H_
#define LIBLATTE_TOOLS_DERIVE_MEASUREMENTS_DERIVE_MEASUREMENTS_H_

#include <cstdlib>
#include "latte.h"

class Deriver {
 public:
  Deriver() {}
  ~Deriver() {
    if (rt_common_) {
      free(rt_common_);
    }
  }

  // Parse TEE's intermediate hash states from file.
  bool ParseHashStates(const char *file);

  // Derive and print the measurements in stdout.
  void OutputMeasurements(const char *name, portid_t portid);

 private:
  uint32_t rt_common_size_;
  uint8_t *rt_common_ = nullptr;
  sgx_hash_state_t *sgx_state_ = nullptr;
  penglai_hash_state_t *penglai_state_ = nullptr;
};

#endif  // LIBLATTE_TOOLS_DERIVE_MEASUREMENTS_DERIVE_MEASUREMENTS_H_
