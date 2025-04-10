cmake_minimum_required (VERSION 2.9)

project (insert_zero_to_wasm)

# Reset default linker flags
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

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

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

set (ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
include (${ROOT_DIR}/lib/liblatte.cmake)

add_library(latte ${LATTE_LIB_SRC})

add_library(utils ${UTILS_LIB_SRC})

set_target_properties (latte PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_target_properties(utils PROPERTIES POSITION_INDEPENDENT_CODE ON)

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections \
                                     -Wall -Wextra -fvisibility=hidden -fpie" )

add_executable (insert_zero_to_wasm insert_zero_to_wasm.cpp ${LATTE_LIB_SRC})

set_target_properties (insert_zero_to_wasm PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries (insert_zero_to_wasm latte utils)
