/**
 * @file flash_encrypted.c
 * @brief Encrypted Flash Storage
 *
 * Stores non-critical data in external flash with encryption.
 * Uses OTP-stored key for AES-256-GCM encryption.
 */

#include "storage/flash_encrypted.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

// Flash layout (16MB total)
// 0x000000 - 0x0FFFFF: Firmware (1MB)
// 0x100000 - 0x1FFFFF: Settings and metadata (1MB)
// 0x200000 - 0xFFFFFF: Transaction history, cached data (14MB)

#define FLASH_SETTINGS_OFFSET 0x100000
#define FLASH_DATA_OFFSET 0x200000

bool flash_encrypted_init(void) {
  // TODO: Load encryption key from OTP
  return false;
}

bool flash_encrypted_read(uint32_t offset, uint8_t *data, uint32_t len) {
  if (!data)
    return false;

  // TODO: Read and decrypt
  (void)offset;
  (void)len;
  return false;
}

bool flash_encrypted_write(uint32_t offset, const uint8_t *data, uint32_t len) {
  if (!data)
    return false;

  // TODO: Encrypt and write
  // Must erase sector first (4KB granularity)
  (void)offset;
  (void)len;
  return false;
}
