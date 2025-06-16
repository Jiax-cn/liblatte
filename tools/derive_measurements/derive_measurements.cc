#include "derive_measurements.h"
#include <iostream>
#include "utils.h"

bool Deriver::ParseHashStates(const char *file) {
  if (!(rt_common_ = read_file(file, &rt_common_size_))) {
    return false;
  }

  auto offset = sizeof(uint32_t);
  sgx_state_ = reinterpret_cast<sgx_hash_state_t *>(rt_common_ + offset);
  offset += sizeof(sgx_hash_state_t);

  penglai_state_ =
    reinterpret_cast<penglai_hash_state_t *>(rt_common_ + offset);
  offset += sizeof(penglai_hash_state_t);
  return true;
}

void Deriver::OutputMeasurements(const char *name, portid_t portid) {
  if (sgx_state_) {
    latte_sgx_measurement_t sgx_mr;
    sgx_derive_hardcode_portid(*sgx_state_, portid, rt_common_, sgx_mr);

    std::cout << "sgx measurement of : " << name << std::endl;
    hexdump(sgx_mr, sizeof(latte_sgx_measurement_t));
  }

  if (penglai_state_) {
    latte_penglai_measurement_t penglai_mr;
    penglai_derive_hardcode_portid(*penglai_state_, portid,
                                   rt_common_, 0, penglai_mr);

    std::cout << "penglai measurement of : " << name << std::endl;
    hexdump(penglai_mr, sizeof(latte_penglai_measurement_t));
  }
}

int main(int argc, char *argv[]) {
  Deriver mr_deriver;
  if (!mr_deriver.ParseHashStates(argv[1])) {
    return 1;
  }

  portid_t portid;
  for (int i = 2; i < argc; i++) {
    if (!read_file_to_buf(argv[i], portid, sizeof(portid_t))) {
      continue;
    }
    mr_deriver.OutputMeasurements(argv[i], portid);
  }

  return 0;
}
