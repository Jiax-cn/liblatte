cmake_minimum_required (VERSION 3.15)

project (verify_portable_identity)

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

add_executable (verify_portable_identity verify_portable_identity.cc)

target_link_libraries (verify_portable_identity latte utils)
