# build

```bash
sh build.sh
```

## insert_wasm_latte

Insert portid section in WASMs.

1. **input:** a group of WASM files (trainer.wasm, runner.wasm)
2. `./insert_wasm_latte trainer.wasm runner.wasm`
3. **output:** `trainer_latte.wasm` `runner_latte.wasm`

## verify_portable_identity

Validate and output portable identities of latte modified WASMs.

1. **input:** a group of modified WASM files (trainer_latte.wasm, runner_latte.wasm)
2. `./verify_portable_identity trainer_latte.wasm runner_latte.wasm`
3. **output:** `runner_latte.wasm.id` `trainer_latte.wasm.id`

## generate_runtime_common

Generate the common part of heterogeneous enclaves.

1. **input:** The intermediate hash state of TEEs.
2. `./generate_runtime_common sgx_rt_mr.bin penglai_rt_mr.bin`
3. **output:** `runtime_common.bin`

## derive_measurements

Derive the expected measurement of the enclaves.

1. **input:** `runtime_common.bin`, the portable identities of WASMs.
2. `./derive_measurements runtime_common.bin runner_latte.wasm.id trainer_latte.wasm.id`
