# liblatte

**liblatte** is a lightweight，standalone library for implementing **layered attestation mechanisms** in portable enclaved applications. It is particularly well-suited for WebAssembly (WASM) and other sandboxed or embedded environments where trust must be established across multiple TEEs.

## 📁 Project Structure

### 📌 src/
Provides the main API interfaces for deriving measurements and portable identities across different Trusted Execution Environment (TEE) platforms.
Currently supported TEEs include Intel SGX and Penglai.

### 📌 eval/
Contains benchmarking tools used to test the performance of the derivation processes implemented in src/.

### 📌 tools/
Includes tools designed to support the deployment of applications that rely on liblatte for layered attestation.

Example:
```bash
cd tools/insert_wasm_latte
mkdir build && cd build
cmake ..
make -j
```

## 📄 License

The source code of liblatte are released under the Apache license 2.0. Check the file LICENSE for more information.
