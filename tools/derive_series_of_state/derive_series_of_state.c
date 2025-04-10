#include "utils.h"
#include "latte.h"
#include "latte_platform.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <wasm>\n", argv[0]);
        return 1;
    }

    uint32_t wasm_size = 0;
    uint8_t *wasm = NULL, vm_mr_sec[WASM_COMMON_SEC_SIZE] = {};
    latte_sgx_measurement_t sgx_mr;

    read_file_to_buf("../sgx_vm_mr.bin", vm_mr_sec, WASM_COMMON_SEC_SIZE);

    if (!(wasm = read_file(argv[1], &wasm_size)))
        goto fail;

    if (!sgx_init_state_list(*(sgx_hash_state_t *)vm_mr_sec)) {
        printf("sgx_init_state_list failed\n");
        goto fail;
    }

    if (!sgx_derive_hardcode_wasm(wasm, wasm_size, vm_mr_sec, sgx_mr)) {
        printf("sgx_derive_hardcode_wasm failed\n");
        goto fail;
    }

    printf("======= sgx native measurement =======\n");
    hexdump(sgx_mr, sizeof(latte_sgx_measurement_t));

    free(wasm);
    return 0;

fail:
    if (wasm)
        free(wasm);

    return 1;
}
