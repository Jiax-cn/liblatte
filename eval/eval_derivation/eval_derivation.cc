#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "test_timming.h"
#include "latte.h"
#include "latte_wasm.h"
#include "crypto/sha256.h"

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  SHA256_CTX sha_ctx;
  portid_t portid;
  portid_hash_state_t portid_state;
  latte_sgx_measurement_t sgx_mr;
  sgx_hash_state_t sgx_state;
  latte_penglai_measurement_t penglai_mr;
  penglai_hash_state_t penglai_state;

  printf("size of portid: %lu\n", sizeof(portid_t));
  printf("size of portid state: %lu\n", sizeof(portid_hash_state_t));
  printf("size of sgx state: %lu\n", sizeof(sgx_hash_state_t));
  printf("size of penglai state: %lu\n", sizeof(penglai_hash_state_t));

  SHA256_Init(&sha_ctx);
  memcpy(portid_state, sha_ctx.h, SHA256_DIGEST_LENGTH);

  memcpy(sgx_state.digest, sha_ctx.h, SHA256_DIGEST_LENGTH);
  sgx_state.offset = 0;
  sgx_state.size = (sha_ctx.Nh << 29) + (sha_ctx.Nl >> 3);

  memset(&penglai_state, 0, sizeof(latte_penglai_measurement_t));

  uint32_t stamp = 50 * 4096;

  uint32_t samples = 6;
  uint32_t max_size = samples * stamp;
  uint32_t i = 0, j = 0, iteration = 100000;

  uint8_t *buf = reinterpret_cast<uint8_t *>(malloc(max_size));
  if (!buf) {
    return -1;
  }

  // for (i = stamp; i <= max_size; i += stamp) {
  //   BENCHMARK_START(t_portid);
  //   for (j = 0; j < iteration; j ++) {
  //     derive_portid(portid_state, buf, i, portid);
  //   }
  //   BENCHMARK_STOP(t_portid);
  //   printf("size %u, t_portid time :  %10ld ns\n", i,
  //          t_portid.tv_sec * 1000000000 + t_portid.tv_nsec);
  // }

  {
    BENCHMARK_START(t_portid);
    for (j = 0; j < iteration; j ++) {
      derive_portid(portid_state, buf, 4096, portid);
    }
    BENCHMARK_STOP(t_portid);
    printf("4096 t_portid time :  %10ld ns\n",
           t_portid.tv_sec * 1000000000 + t_portid.tv_nsec);
  }

  // for (i = stamp; i <= max_size; i += stamp) {
  //   BENCHMARK_START(t_sgx);
  //   for (j = 0; j < iteration; j ++) {
  //     sgx_derive_rest_data(sgx_state, buf, i, sgx_mr);
  //   }
  //   BENCHMARK_STOP(t_sgx);
  //   printf("size %u, t_sgx time :  %10ld ns\n", i,
  //          t_sgx.tv_sec * 1000000000 + t_sgx.tv_nsec);
  // }

  {
    BENCHMARK_START(t_sgx);
    for (j = 0; j < iteration; j ++) {
      sgx_derive_rest_data(sgx_state, buf, 8192, sgx_mr);
    }
    BENCHMARK_STOP(t_sgx);
    printf("8192 t_sgx time :  %10ld ns\n",
           t_sgx.tv_sec * 1000000000 + t_sgx.tv_nsec);
  }

  // for (i = stamp; i <= max_size; i += stamp) {
  //   BENCHMARK_START(t_penglai);
  //   for (j = 0; j < iteration; j ++) {
  //     penglai_derive_rest_data(penglai_state, buf, i, 0, penglai_mr);
  //   }
  //   BENCHMARK_STOP(t_penglai);
  //   printf("size %u, t_penglai time :  %10ld ns\n", i,
  //          t_penglai.tv_sec * 1000000000 + t_penglai.tv_nsec);
  // }

  {
    BENCHMARK_START(t_penglai);
    for (j = 0; j < iteration; j ++) {
      penglai_derive_rest_data(penglai_state, buf, 8192, 0, penglai_mr);
    }
    BENCHMARK_STOP(t_penglai);
    printf("8192 t_penglai time :  %10ld ns\n",
           t_penglai.tv_sec * 1000000000 + t_penglai.tv_nsec);
  }

  return 0;
}
