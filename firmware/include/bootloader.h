/**
 * LibreCrypt Wallet Bootloader
 *
 * Secure Boot implementation for RP2350
 * - Firmware signature verification
 * - Anti-rollback protection
 * - Recovery mode support
 */

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdbool.h>
#include <stdint.h>


// Bootloader version
#define BOOTLOADER_VERSION_MAJOR 1
#define BOOTLOADER_VERSION_MINOR 0
#define BOOTLOADER_VERSION_PATCH 0

// Firmware header location (after bootloader)
#define FIRMWARE_HEADER_OFFSET 0x10000 // 64KB reserved for bootloader
#define FIRMWARE_START_OFFSET 0x10100  // After header

// Header magic
#define FIRMWARE_MAGIC 0x4C435746 // "LWCF" - LibreCrypt Wallet Firmware

// Firmware header structure
typedef struct __attribute__((packed)) {
  uint32_t magic;            // Magic number
  uint32_t version;          // Version (major.minor.patch packed)
  uint32_t size;             // Firmware size in bytes
  uint32_t entry_point;      // Entry point offset
  uint8_t hash[32];          // SHA-256 hash of firmware
  uint8_t signature[64];     // Ed25519 signature (future)
  uint32_t rollback_counter; // Anti-rollback counter
  uint32_t flags;            // Flags (debug, etc.)
  uint8_t reserved[120];     // Reserved for future use
} firmware_header_t;

// Bootloader status
typedef enum {
  BOOT_STATUS_OK = 0,
  BOOT_STATUS_NO_FIRMWARE,
  BOOT_STATUS_INVALID_MAGIC,
  BOOT_STATUS_INVALID_HASH,
  BOOT_STATUS_INVALID_SIGNATURE,
  BOOT_STATUS_ROLLBACK_DETECTED,
  BOOT_STATUS_RECOVERY_MODE
} boot_status_t;

/**
 * Initialize bootloader
 */
void bootloader_init(void);

/**
 * Verify firmware integrity
 * @return boot status
 */
boot_status_t bootloader_verify_firmware(void);

/**
 * Jump to main firmware
 */
void bootloader_jump_to_firmware(void);

/**
 * Enter recovery mode
 */
void bootloader_recovery_mode(void);

/**
 * Get stored rollback counter
 */
uint32_t bootloader_get_rollback_counter(void);

/**
 * Update rollback counter
 */
bool bootloader_update_rollback_counter(uint32_t new_counter);

/**
 * Check if recovery button is pressed
 */
bool bootloader_is_recovery_pressed(void);

#endif // BOOTLOADER_H
