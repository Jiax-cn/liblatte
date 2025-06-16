cmake_minimum_required (VERSION 3.15)

project (generate_runtime_common)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

set (ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
include (${ROOT_DIR}/src/liblatte.cmake)

add_library(latte ${LATTE_LIB_SRC})

add_library(utils ${UTILS_LIB_SRC})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -Wall -Wextra" )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections -Wall -Wextra" )
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

add_executable (generate_runtime_common generate_runtime_common.cc)

target_link_libraries (generate_runtime_common latte utils)
