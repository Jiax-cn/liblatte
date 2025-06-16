cmake_minimum_required (VERSION 3.15)

set (CRYPTO_DIR ${CMAKE_CURRENT_LIST_DIR}/crypto)

include_directories (${CRYPTO_DIR})

set (LATTE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR})

include_directories (${LATTE_LIB_DIR})

set (LATTE_LIB_SRC 
    ${LATTE_LIB_DIR}/latte.c
    ${LATTE_LIB_DIR}/latte_wasm.c
    ${LATTE_LIB_DIR}/crypto/sha256.c
    ${LATTE_LIB_DIR}/crypto/sm3.c)

set (UTILS_LIB_SRC ${LATTE_LIB_DIR}/utils.c)

if (BUILD_ON_PENGLAI EQUAL 1)
    set (toolchain_sdk_dir "/home/penglai/penglai-multilib-toolchain-install/bin/")
    set (CMAKE_C_COMPILER ${toolchain_sdk_dir}riscv64-unknown-linux-gnu-gcc)
    set (CMAKE_CXX_COMPILER ${toolchain_sdk_dir}riscv64-unknown-linux-gnu-g++)
    set (CMAKE_ASM_COMPILER ${toolchain_sdk_dir}riscv64-unknown-linux-gnu-g++)

    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mabi=lp64 -march=rv64imac")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mabi=lp64 -march=rv64imac")
    set (CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mabi=lp64 -march=rv64imac")

    message(STATUS "toolchain_sdk_dir=${toolchain_sdk_dir}")
    message(STATUS "cc=${CMAKE_C_COMPILER}")
endif ()

if (BUILD_ON_SGX EQUAL 1) 
    if (NOT DEFINED $(SGXSDK))
        set (SGXSDK /opt/merge/sgxsdk)
    endif ()
    include_directories (${SGXSDK}/include)
endif()
