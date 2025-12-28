cmake_minimum_required(VERSION 3.13)

# Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(librecrypt_wallet C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializar SDK
pico_sdk_init()

# Executável principal
add_executable(librecrypt_wallet
    src/main.c
    src/crypto/librecipher.c
    src/crypto/sha256.c
    src/crypto/aes_gcm.c
    src/wallet/wallet.c
    src/wallet/keystore.c
    src/protocol/usb_protocol.c
)

# Includes
target_include_directories(librecrypt_wallet PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Bibliotecas do Pico SDK
target_link_libraries(librecrypt_wallet
    pico_stdlib
    pico_unique_id
    hardware_flash
    hardware_sync
    tinyusb_device
    tinyusb_board
)

# Configurações de segurança
target_compile_definitions(librecrypt_wallet PRIVATE
    LIBRECIPHER_CONSTANT_TIME=1
    LIBRECIPHER_ZERO_ALLOC=1
)

# Otimizações
target_compile_options(librecrypt_wallet PRIVATE
    -Wall
    -Wextra
    -Werror
    -O2
)

# USB stdio
pico_enable_stdio_usb(librecrypt_wallet 1)
pico_enable_stdio_uart(librecrypt_wallet 0)

# UF2 output
pico_add_extra_outputs(librecrypt_wallet)
