# insert_zero_to_wasm

Insert empty custom section of specific size in target wasm.

```
cd insert_zero_to_wasm
mkdir build && cd build
cmake ..
make -j
./insert_zero_to_wasm test.wasm 10000
```

# eval_derivation

Test the performance of the derivation of measurements.
