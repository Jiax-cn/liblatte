#include "latte.h"
#include "latte_platform.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

/* Portable Identity */
void gen_portid_state(uint8_t *wasm, uint32_t wasm_size,
                      portid_hash_state_t ret_hash_state) {
    SHA256((unsigned char *)wasm, (size_t)wasm_size,
           (unsigned char *)ret_hash_state);
    return;
}

void derive_portid(portid_hash_state_t hash_state, uint8_t *common_part,
                   uint32_t common_part_size, portid_t ret_portid) {
    SHA256_CTX sha_ctx;

    SHA256_Init(&sha_ctx);
    SHA256_Update(&sha_ctx, (unsigned char *)hash_state,
                  sizeof(portid_hash_state_t));
    SHA256_Update(&sha_ctx, common_part, common_part_size);
    SHA256_Final((unsigned char *)ret_portid, &sha_ctx);
    return;
}


/* TEE Common */
uint8_t *build_aligned_wasm_sec(uint8_t *wasm, uint32_t wasm_size,
                                uint8_t *common_part, uint32_t alignment,
                                uint32_t *ret_aligned_size) {
    uint8_t *wasm_sec = NULL;
    uint32_t aligned_total = 0, offset = 0;

    aligned_total = (wasm_size + sizeof(wasm_size) + WASM_COMMON_SEC_SIZE
                    + alignment - 1) & ~(alignment - 1);

    if (!(wasm_sec = (uint8_t *)malloc(aligned_total)))
        return NULL;

    /* 0x00 align */
    offset = aligned_total - wasm_size - sizeof(wasm_size)
             - WASM_COMMON_SEC_SIZE;
    memset(wasm_sec, 0, offset);

    /* wasm */
    memcpy(wasm_sec + offset, wasm, wasm_size);
    offset += wasm_size;

    /* wasm size */
    memcpy(wasm_sec + offset, &wasm_size, sizeof(wasm_size));
    offset += sizeof(wasm_size);

    /* runtime common part */
    memcpy(wasm_sec + offset, common_part, WASM_COMMON_SEC_SIZE);
    offset += WASM_COMMON_SEC_SIZE;

    *ret_aligned_size = offset;
    return wasm_sec;
}


/* SGX */

#define SIZE_NAMED_VALUE 8
#define DATA_BLOCK_SIZE 64

sgx_hash_state_t *sgx_initialized_state_list = NULL;

    /* Update the sha_ctx using SHA256_Update(); */
static
void sgx_update_hash_state_internal(uint8_t *aligned_d, uint32_t aligned_s,
                                    uint64_t page_offset, SHA256_CTX *sha_ctx) {
    uint8_t *cur_addr, *end_addr;

    cur_addr = aligned_d;
    end_addr = aligned_d + aligned_s;
    while (cur_addr < end_addr) {
        uint8_t eadd_val[SIZE_NAMED_VALUE] = "EADD\0\0\0";
        uint8_t sinfo[64] = {0x01, 0x02};
        int i = 0;

        size_t db_offset = 0;
        uint8_t data_block[DATA_BLOCK_SIZE];

        uint8_t eextend_val[SIZE_NAMED_VALUE] = "EEXTEND";

        memset(data_block, 0, DATA_BLOCK_SIZE);
        memcpy(data_block, eadd_val, SIZE_NAMED_VALUE);

        db_offset += SIZE_NAMED_VALUE;
        memcpy(data_block + db_offset, &page_offset, sizeof(page_offset));
        db_offset += sizeof(page_offset);
        memcpy(data_block + db_offset, &sinfo, sizeof(data_block) - db_offset);

        SHA256_Update(sha_ctx, data_block, DATA_BLOCK_SIZE);

        #define EEXTEND_TIME  4
        for (i = 0; i < SE_PAGE_SIZE; i += (DATA_BLOCK_SIZE * EEXTEND_TIME)) {
            int j = 0;
            db_offset = 0;
            memset(data_block, 0, DATA_BLOCK_SIZE);
            memcpy(data_block, eextend_val, SIZE_NAMED_VALUE);
            db_offset += SIZE_NAMED_VALUE;
            memcpy(data_block + db_offset, &page_offset, sizeof(page_offset));

            SHA256_Update(sha_ctx, data_block, DATA_BLOCK_SIZE);

            for (j = 0; j < EEXTEND_TIME; j++) {
                memcpy(data_block, cur_addr, DATA_BLOCK_SIZE);

                SHA256_Update(sha_ctx, data_block, DATA_BLOCK_SIZE);

                cur_addr += DATA_BLOCK_SIZE;
                page_offset += DATA_BLOCK_SIZE;
            }
        }
    }
}

void sgx_update_common_part(sgx_hash_state_t in_hash_state,
                            uint8_t *aligned_d, uint32_t aligned_s,
                            sgx_hash_state_t *ret_hash_state) {
    SHA256_CTX sha_ctx;
    SHA256_Init(&sha_ctx);
    memcpy((uint8_t*)sha_ctx.h, in_hash_state.digest, SHA256_DIGEST_LENGTH);
    sha_ctx.Nh = in_hash_state.size >> 29;
    sha_ctx.Nl = in_hash_state.size << 3;

    sgx_update_hash_state_internal(aligned_d, aligned_s,
                                   in_hash_state.offset, &sha_ctx);

    memcpy(ret_hash_state->digest, (uint8_t*)sha_ctx.h, SHA256_DIGEST_LENGTH);
    ret_hash_state->size = (sha_ctx.Nh << 29) + (sha_ctx.Nl >> 3);
    ret_hash_state->offset = in_hash_state.offset + aligned_s;
    return;
}

bool sgx_init_state_list(sgx_hash_state_t meta_hash_state) {
    uint8_t empty_page[SE_PAGE_SIZE] = {0};
    SHA256_CTX sha_ctx;
    uint32_t i = 0;

    if (!(sgx_initialized_state_list = (sgx_hash_state_t *)malloc(
            WASM_PLD_SEC_SIZE / SE_PAGE_SIZE * sizeof(sgx_hash_state_t))))
        return false;

    memcpy(&sgx_initialized_state_list[0], (uint8_t *)&meta_hash_state,
           sizeof(sgx_hash_state_t));

    SHA256_Init(&sha_ctx);
    memcpy((uint8_t*)sha_ctx.h, sgx_initialized_state_list[0].digest,
           SHA256_DIGEST_LENGTH);
    sha_ctx.Nh = sgx_initialized_state_list[0].size >> 29;
    sha_ctx.Nl = sgx_initialized_state_list[0].size << 3;
    for (i = 1; i < WASM_PLD_SEC_SIZE / SE_PAGE_SIZE; i++) {
        sgx_update_hash_state_internal(
                    empty_page, SE_PAGE_SIZE,
                    sgx_initialized_state_list[i-1].offset, &sha_ctx);
        memcpy(sgx_initialized_state_list[i].digest,
               (uint8_t*)sha_ctx.h, SHA256_DIGEST_LENGTH);
        sgx_initialized_state_list[i].size = (sha_ctx.Nh << 29) +
                                             (sha_ctx.Nl >> 3);
        sgx_initialized_state_list[i].offset =
              sgx_initialized_state_list[i-1].offset + SE_PAGE_SIZE;
    }

    return true;
}

void sgx_derive_rest_data(sgx_hash_state_t hash_state, uint8_t *aligned_d,
                          uint32_t aligned_s, latte_sgx_measurement_t ret_mr) {
    SHA256_CTX sha_ctx;
    SHA256_Init(&sha_ctx);

    memcpy((uint8_t*)sha_ctx.h, hash_state.digest, SHA256_DIGEST_LENGTH);
    sha_ctx.Nh = hash_state.size >> 29;
    sha_ctx.Nl = hash_state.size << 3;

    sgx_update_hash_state_internal(aligned_d, aligned_s, hash_state.offset,
                                   &sha_ctx);

    SHA256_Final((unsigned char *)ret_mr, &sha_ctx);
    return;
}

void sgx_derive_common_part(sgx_hash_state_t hash_state, uint8_t *common_part,
                            latte_sgx_measurement_t ret_mr) {
    sgx_derive_rest_data(hash_state, common_part,
                         WASM_COMMON_SEC_SIZE, ret_mr);
    return;
}

bool sgx_derive_hardcode_wasm(uint8_t *wasm, uint32_t wasm_size,
                              uint8_t *common_part,
                              latte_sgx_measurement_t ret_mr) {
    uint8_t *aligned_d = NULL;
    uint32_t aligned_s = 0, seq = 0;

    if (!sgx_initialized_state_list || wasm_size > WASM_PLD_SEC_SIZE)
        return false;

    if (!(aligned_d = build_aligned_wasm_sec(wasm, wasm_size, common_part,
                                             SE_PAGE_SIZE, &aligned_s)))
        return false;

    seq = (WASM_PLD_SEC_SIZE - aligned_s) / SE_PAGE_SIZE;

    sgx_derive_rest_data(sgx_initialized_state_list[seq], aligned_d, aligned_s,
                         ret_mr);
    free(aligned_d);
    return true;
}

bool sgx_derive_hardcode_portid(sgx_hash_state_t hash_state, portid_t portid,
                                uint8_t *common_part,
                                latte_sgx_measurement_t ret_mr) {
    uint8_t *aligned_d = NULL;

    if (!(aligned_d = (uint8_t *)malloc(WASM_SEC_SIZE)))
        return false;

    memset(aligned_d, 0, WASM_SEC_SIZE);
    memcpy(aligned_d, portid, sizeof(portid_t));
    memcpy(aligned_d+WASM_PLD_SEC_SIZE, common_part, WASM_COMMON_SEC_SIZE);

    sgx_derive_rest_data(hash_state, aligned_d, WASM_SEC_SIZE, ret_mr);

    free(aligned_d);
    return true;
}


/* Penglai */

#define PENGLAI_SM3_SIZE (sizeof(penglai_hash_state_t) - sizeof(unsigned long))

penglai_hash_state_t *penglai_initialized_state_list = NULL;

void penglai_update_common_part(penglai_hash_state_t in_hash_state,
                                uint8_t *aligned_buf,
                                uint32_t aligned_buf_size,
                                penglai_hash_state_t *ret_hash_state) {
    sm3_context sm3_ctx;
    memcpy(&sm3_ctx, (void*)(in_hash_state.total), PENGLAI_SM3_SIZE);

    unsigned long start_va = PENGLAI_PLD_SEC_ADDR;
    unsigned char pte = 0;
    sm3_update(&sm3_ctx, (unsigned char*)&start_va, sizeof(unsigned long));
    sm3_update(&sm3_ctx, &pte, 1);

    sm3_update(&sm3_ctx, aligned_buf, aligned_buf_size);
    memcpy(ret_hash_state->total, &sm3_ctx, PENGLAI_SM3_SIZE);
}

bool penglai_init_state_list(penglai_hash_state_t meta_hash_state) {
    sm3_context sm3_ctx;
    unsigned long i = 0;

    if (!(penglai_initialized_state_list = (penglai_hash_state_t *)malloc(
            WASM_PLD_SEC_SIZE / SE_PAGE_SIZE * sizeof(penglai_hash_state_t))))
        return false;

    memcpy(&sm3_ctx, (void*)(meta_hash_state.total), PENGLAI_SM3_SIZE);

    unsigned long start_va = PENGLAI_PLD_SEC_ADDR;
    unsigned char pte = 0;
    sm3_update(&sm3_ctx, (unsigned char*)&start_va, sizeof(unsigned long));
    sm3_update(&sm3_ctx, &pte, 1);
    memcpy(penglai_initialized_state_list[0].total, &sm3_ctx, PENGLAI_SM3_SIZE);

    uint8_t empty_page[SE_PAGE_SIZE] = {0};
    unsigned long max_pages = WASM_PLD_SEC_SIZE / SE_PAGE_SIZE;
    for (i = 1; i < max_pages; i++) {
      sm3_update(&sm3_ctx, empty_page, SE_PAGE_SIZE);
      memcpy(penglai_initialized_state_list[i].total,
             &sm3_ctx, PENGLAI_SM3_SIZE);
    }
    return true;
}

void penglai_derive_rest_data(penglai_hash_state_t hash_state,
                              uint8_t *aligned_d, uint32_t aligned_s,
                              unsigned long nonce,
                              latte_penglai_measurement_t ret_mr) {
    sm3_context sm3_ctx;
    unsigned long offset = 0;

    memcpy(&sm3_ctx, hash_state.total, PENGLAI_SM3_SIZE);
    for (offset = 0; offset < aligned_s; offset += SE_PAGE_SIZE)
        sm3_update(&sm3_ctx, (unsigned char*)(aligned_d+offset), SE_PAGE_SIZE);

    sm3_update(&sm3_ctx, (unsigned char*)(&nonce), sizeof(unsigned long));
    sm3_final(&sm3_ctx, (unsigned char*)(ret_mr));

    sm3_init(&sm3_ctx);
    sm3_update(&sm3_ctx, (unsigned char*)(ret_mr), SM3_DIGEST_LENGTH);
    sm3_update(&sm3_ctx, (unsigned char*)(&nonce), sizeof(unsigned long));
    sm3_final(&sm3_ctx, ret_mr);
}

void penglai_derive_common_part(penglai_hash_state_t hash_state,
                                uint8_t *common_part, unsigned long nonce,
                                latte_penglai_measurement_t ret_mr)
{
    penglai_derive_rest_data(hash_state, common_part, WASM_COMMON_SEC_SIZE,
                             nonce, ret_mr);
}

bool penglai_derive_hardcode_wasm(uint8_t *wasm, uint32_t wasm_size,
                                  uint8_t *common_part, unsigned long nonce,
                                  latte_penglai_measurement_t ret_mr)
{
    uint8_t *aligned_d = NULL;
    uint32_t aligned_s = 0, seq = 0;

    if (!penglai_initialized_state_list)
        return false;

    if (!(aligned_d = build_aligned_wasm_sec(wasm, wasm_size, common_part,
                                             SE_PAGE_SIZE, &aligned_s)))
        return false;

    seq = (WASM_PLD_SEC_SIZE - aligned_s) / SE_PAGE_SIZE;
    penglai_derive_rest_data(penglai_initialized_state_list[seq], aligned_d,
                             aligned_s, nonce, ret_mr);
    free(aligned_d);
    return true;
}

bool penglai_derive_hardcode_portid(penglai_hash_state_t hash_state,
                                    portid_t portid, uint8_t *common_part,
                                    unsigned long nonce,
                                    latte_penglai_measurement_t ret_mr)
{
    uint8_t *aligned_d = NULL;

    if (!(aligned_d = (uint8_t *)malloc(WASM_SEC_SIZE)))
        return false;

    memset(aligned_d, 0, WASM_SEC_SIZE);
    memcpy(aligned_d, portid, sizeof(portid_t));
    memcpy(aligned_d+WASM_PLD_SEC_SIZE, common_part, WASM_COMMON_SEC_SIZE);

    penglai_derive_rest_data(hash_state, aligned_d, WASM_SEC_SIZE,
                             nonce, ret_mr);
    free(aligned_d);
    return true;
}

/* Penglai End */
