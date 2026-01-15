cmake_minimum_required(VERSION 3.13)

# =============================================================================
# LibreCrypt Wallet Firmware
# Hardware Cryptocurrency Wallet for RP2350
# =============================================================================

# Initialize the Pico SDK
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
set(PICO_BOARD "pico2")  # RP2350-based board

# Pull in SDK (must be before project())
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(librecrypt_wallet C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# =============================================================================
# Compiler Flags for Security
# =============================================================================
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -fstack-protector-strong
    -ffunction-sections
    -fdata-sections
)

# =============================================================================
# Source Files
# =============================================================================
set(CRYPTO_SOURCES
    src/crypto/bip32.c
    src/crypto/bip39.c
    src/crypto/bip39_wordlist.c
    src/crypto/secp256k1.c
    src/crypto/sha256_hw.c
)

set(CRYPTO_EXPERIMENTAL_SOURCES
    src/crypto/experimental/kyber.c
    src/crypto/experimental/dilithium.c
)

set(STORAGE_SOURCES
    src/storage/otp_manager.c
    src/storage/flash_encrypted.c
)

set(USB_SOURCES
    src/usb/hid_protocol.c
)

set(TRUSTZONE_SOURCES
    src/trustzone/secure_world.c
)

# =============================================================================
# Main Executable
# =============================================================================
add_executable(librecrypt_wallet
    src/main.c
    ${CRYPTO_SOURCES}
    ${CRYPTO_EXPERIMENTAL_SOURCES}
    ${STORAGE_SOURCES}
    ${USB_SOURCES}
    ${TRUSTZONE_SOURCES}
)

# Include directories
target_include_directories(librecrypt_wallet PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Link libraries
target_link_libraries(librecrypt_wallet
    pico_stdlib
    pico_unique_id
    hardware_flash
    hardware_sync
    hardware_sha256
    tinyusb_device
    tinyusb_board
)

# Enable USB output, disable UART
pico_enable_stdio_usb(librecrypt_wallet 0)
pico_enable_stdio_uart(librecrypt_wallet 0)

# Create UF2 file for flashing
pico_add_extra_outputs(librecrypt_wallet)

# =============================================================================
# Custom USB Descriptors
# =============================================================================
target_compile_definitions(librecrypt_wallet PRIVATE
    # LibreCrypt Wallet USB identifiers
    USBD_VID=0x1209           # pid.codes VID (open source)
    USBD_PID=0x4C43           # "LC" = LibreCrypt
    USBD_MANUFACTURER="LibreCrypt"
    USBD_PRODUCT="LibreCrypt Wallet"
    
    # Security flags
    LIBRECRYPT_EXPERIMENTAL_CRYPTO=1
    LIBRECRYPT_TRUSTZONE_ENABLED=1
)

# =============================================================================
# Unit Tests (optional, via Unity framework)
# =============================================================================
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
