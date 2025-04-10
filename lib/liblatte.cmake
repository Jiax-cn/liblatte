if (NOT DEFINED $(LATTE_ROOT_DIR))
    set (LATTE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif ()

#if (NOT DEFINED $(SGXSDK))
#    set (SGXSDK /opt/merge/sgxsdk)
#endif ()

set (CRYPTO_DIR ${CMAKE_CURRENT_LIST_DIR}/crypto)

include_directories (${CRYPTO_DIR})

set (LATTE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR})

include_directories (${LATTE_LIB_DIR})
#include_directories (${SGXSDK}/include)

set (LATTE_LIB_SRC 
    ${LATTE_LIB_DIR}/latte.c
    ${LATTE_LIB_DIR}/latte_wasm.c
    ${LATTE_LIB_DIR}/crypto/sha256.c
    ${LATTE_LIB_DIR}/crypto/sm3.c)

set (UTILS_LIB_SRC ${LATTE_LIB_DIR}/utils.c)