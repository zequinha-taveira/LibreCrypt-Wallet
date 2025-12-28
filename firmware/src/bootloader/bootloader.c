/**
 * LibreCrypt Wallet Bootloader Implementation
 *
 * Secure boot with firmware verification
 */

#include "bootloader.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "sha256.h"
#include "ws2812.h"
#include <string.h>


// Flash addresses
#define FLASH_BASE 0x10000000
#define ROLLBACK_OFFSET 0x0F000 // 60KB - rollback counter storage
#define ROLLBACK_ADDR (FLASH_BASE + ROLLBACK_OFFSET)

// Recovery button (GPIO for user button if available, otherwise use timeout)
#define RECOVERY_GPIO 14 // Placeholder - adjust for your board

/**
 * Initialize bootloader
 */
void bootloader_init(void) {
  stdio_init_all();
  ws2812_init();

  // Initialize recovery GPIO if available
  gpio_init(RECOVERY_GPIO);
  gpio_set_dir(RECOVERY_GPIO, GPIO_IN);
  gpio_pull_up(RECOVERY_GPIO);
}

/**
 * Read firmware header from flash
 */
static bool read_firmware_header(firmware_header_t *header) {
  const uint8_t *flash_ptr =
      (const uint8_t *)(FLASH_BASE + FIRMWARE_HEADER_OFFSET);
  memcpy(header, flash_ptr, sizeof(firmware_header_t));
  return header->magic == FIRMWARE_MAGIC;
}

/**
 * Compute hash of firmware
 */
static void compute_firmware_hash(const firmware_header_t *header,
                                  uint8_t *hash) {
  const uint8_t *firmware_ptr =
      (const uint8_t *)(FLASH_BASE + FIRMWARE_START_OFFSET);
  sha256_hash(firmware_ptr, header->size, hash);
}

/**
 * Constant-time compare
 */
static bool secure_compare(const uint8_t *a, const uint8_t *b, size_t len) {
  uint8_t diff = 0;
  for (size_t i = 0; i < len; i++) {
    diff |= a[i] ^ b[i];
  }
  return diff == 0;
}

/**
 * LED indicator for boot status
 */
static void indicate_status(boot_status_t status) {
  switch (status) {
  case BOOT_STATUS_OK:
    ws2812_set_rgb(0, 255, 0); // Green
    break;
  case BOOT_STATUS_RECOVERY_MODE:
    ws2812_set_rgb(0, 0, 255); // Blue
    break;
  case BOOT_STATUS_NO_FIRMWARE:
    ws2812_set_rgb(255, 255, 0); // Yellow
    break;
  default:
    // Error - blink red
    for (int i = 0; i < 5; i++) {
      ws2812_set_rgb(255, 0, 0);
      sleep_ms(100);
      ws2812_off();
      sleep_ms(100);
    }
    break;
  }
}

/**
 * Verify firmware integrity
 */
boot_status_t bootloader_verify_firmware(void) {
  firmware_header_t header;

  // Read header
  if (!read_firmware_header(&header)) {
    return BOOT_STATUS_NO_FIRMWARE;
  }

  // Verify magic
  if (header.magic != FIRMWARE_MAGIC) {
    return BOOT_STATUS_INVALID_MAGIC;
  }

  // Verify size is reasonable
  if (header.size == 0 || header.size > (2 * 1024 * 1024)) { // Max 2MB
    return BOOT_STATUS_INVALID_HASH;
  }

  // Compute and verify hash
  uint8_t computed_hash[32];
  compute_firmware_hash(&header, computed_hash);

  if (!secure_compare(computed_hash, header.hash, 32)) {
    return BOOT_STATUS_INVALID_HASH;
  }

  // Check rollback counter
  uint32_t stored_counter = bootloader_get_rollback_counter();
  if (header.rollback_counter < stored_counter) {
    return BOOT_STATUS_ROLLBACK_DETECTED;
  }

  // TODO: Verify Ed25519 signature (when implemented)
  // if (!verify_signature(header.signature, header.hash, public_key)) {
  //     return BOOT_STATUS_INVALID_SIGNATURE;
  // }

  // Update rollback counter if needed
  if (header.rollback_counter > stored_counter) {
    bootloader_update_rollback_counter(header.rollback_counter);
  }

  return BOOT_STATUS_OK;
}

/**
 * Jump to main firmware
 */
void bootloader_jump_to_firmware(void) {
  firmware_header_t header;
  read_firmware_header(&header);

  // Calculate entry point
  uint32_t entry = FLASH_BASE + FIRMWARE_START_OFFSET + header.entry_point;

  // Disable interrupts
  __disable_irq();

  // Set vector table
  // Note: On RP2350, this requires proper vector table relocation

  // Jump to firmware
  typedef void (*firmware_entry_t)(void);
  firmware_entry_t firmware_main = (firmware_entry_t)entry;

  // Reset stack and jump
  __asm__ volatile("msr msp, %0\n"
                   "bx %1\n" ::"r"(*(uint32_t *)entry),
                   "r"(entry + 4));

  // Should never reach here
  while (1) {
  }
}

/**
 * Recovery mode
 */
void bootloader_recovery_mode(void) {
  // Blue LED indicates recovery mode
  indicate_status(BOOT_STATUS_RECOVERY_MODE);

  printf("\n");
  printf("=================================\n");
  printf(" LibreCrypt Wallet - RECOVERY\n");
  printf("=================================\n");
  printf("Waiting for firmware upload...\n");

  // In recovery mode, wait for new firmware over USB
  // This would typically handle XMODEM or custom protocol
  while (true) {
    // Blink blue LED
    ws2812_set_rgb(0, 0, 128);
    sleep_ms(500);
    ws2812_set_rgb(0, 0, 255);
    sleep_ms(500);

    // Check for incoming data
    // TODO: Implement firmware upload protocol
  }
}

/**
 * Get rollback counter from flash
 */
uint32_t bootloader_get_rollback_counter(void) {
  const uint32_t *counter_ptr = (const uint32_t *)ROLLBACK_ADDR;

  // Find the highest valid counter value
  // Using wear-leveling scheme: scan sector for highest counter
  uint32_t max_counter = 0;
  for (int i = 0; i < 1024; i++) { // 4KB sector / 4 bytes = 1024 entries
    uint32_t val = counter_ptr[i];
    if (val != 0xFFFFFFFF && val > max_counter) {
      max_counter = val;
    }
  }

  return max_counter;
}

/**
 * Update rollback counter
 */
bool bootloader_update_rollback_counter(uint32_t new_counter) {
  // Find next free slot in sector
  const uint32_t *counter_ptr = (const uint32_t *)ROLLBACK_ADDR;
  int free_slot = -1;

  for (int i = 0; i < 1024; i++) {
    if (counter_ptr[i] == 0xFFFFFFFF) {
      free_slot = i;
      break;
    }
  }

  // If sector is full, erase and start fresh
  if (free_slot < 0) {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(ROLLBACK_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
    free_slot = 0;
  }

  // Write new counter value
  uint32_t ints = save_and_disable_interrupts();
  flash_range_program(ROLLBACK_OFFSET + (free_slot * 4),
                      (const uint8_t *)&new_counter, 4);
  restore_interrupts(ints);

  return true;
}

/**
 * Check if recovery button is pressed
 */
bool bootloader_is_recovery_pressed(void) {
  // Active low button
  return !gpio_get(RECOVERY_GPIO);
}

/**
 * Bootloader main entry (if used as separate binary)
 */
void bootloader_main(void) {
  bootloader_init();

  printf("\n");
  printf("LibreCrypt Bootloader v%d.%d.%d\n", BOOTLOADER_VERSION_MAJOR,
         BOOTLOADER_VERSION_MINOR, BOOTLOADER_VERSION_PATCH);

  // Check for recovery mode
  if (bootloader_is_recovery_pressed()) {
    printf("Recovery button pressed\n");
    bootloader_recovery_mode();
    return;
  }

  // Verify and boot firmware
  boot_status_t status = bootloader_verify_firmware();
  indicate_status(status);

  if (status == BOOT_STATUS_OK) {
    printf("Firmware verified, jumping...\n");
    sleep_ms(100);
    bootloader_jump_to_firmware();
  } else {
    printf("Firmware verification failed: %d\n", status);
    bootloader_recovery_mode();
  }
}
