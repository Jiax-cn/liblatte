#!/bin/bash

root_path="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

tools="derive_measurements  generate_runtime_common  insert_wasm_latte  verify_portable_identity"

for tool in ${tools}; do
    cd "${root_path}/${tool}" && \
    mkdir -p build && cd build && \
    cmake .. && make -j
done
