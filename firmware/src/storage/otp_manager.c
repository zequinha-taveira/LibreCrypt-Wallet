/**
 * @file otp_manager.c
 * @brief OTP Memory Manager Implementation
 */

#include "storage/otp_manager.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

// =============================================================================
// RP2350 OTP Access (Placeholder - needs SDK 2.0 specifics)
// =============================================================================

// NOTE: The actual RP2350 OTP API depends on the SDK version.
// These are placeholder implementations that need to be updated
// when using the actual Pico SDK 2.0 OTP functions.

// Simulated OTP storage (for development only - in production this is HW)
#ifdef LIBRECRYPT_SIMULATE_OTP
static uint8_t simulated_otp[OTP_TOTAL_SIZE] = {0};
#endif

static bool otp_read_page(uint8_t page, uint8_t *data) {
  if (page >= OTP_TOTAL_PAGES) {
    return false;
  }

#ifdef LIBRECRYPT_SIMULATE_OTP
  memcpy(data, &simulated_otp[page * OTP_PAGE_SIZE], OTP_PAGE_SIZE);
  return true;
#else
  // TODO: Implement actual RP2350 OTP read using SDK 2.0
  // rom_otp_access_t otp_access;
  // ...
  (void)data;
  return false;
#endif
}

static bool otp_write_page(uint8_t page, const uint8_t *data) {
  if (page >= OTP_TOTAL_PAGES) {
    return false;
  }

#ifdef LIBRECRYPT_SIMULATE_OTP
  // Check if page is already programmed (simulated)
  for (int i = 0; i < OTP_PAGE_SIZE; i++) {
    if (simulated_otp[page * OTP_PAGE_SIZE + i] != 0) {
      // OTP can only be programmed once (0 -> 1 transitions)
      // For simulation, prevent overwrites
      return false;
    }
  }
  memcpy(&simulated_otp[page * OTP_PAGE_SIZE], data, OTP_PAGE_SIZE);
  return true;
#else
  // TODO: Implement actual RP2350 OTP write
  (void)data;
  return false;
#endif
}

// =============================================================================
// Public API Implementation
// =============================================================================

bool otp_manager_is_initialized(void) {
  uint8_t config_page[OTP_PAGE_SIZE];

  if (!otp_read_page(OTP_PAGE_CONFIG, config_page)) {
    return false;
  }

  uint32_t flags = *(uint32_t *)config_page;
  return (flags & OTP_FLAG_INITIALIZED) != 0;
}

bool otp_manager_init_seed(const uint8_t seed[64]) {
  if (otp_manager_is_initialized()) {
    return false; // Already initialized
  }

  uint32_t irq = save_and_disable_interrupts();

  // Write seed to pages 1-2
  uint8_t page1[OTP_PAGE_SIZE] = {0};
  uint8_t page2[OTP_PAGE_SIZE] = {0};

  memcpy(page1, seed, 32);      // First 32 bytes
  memcpy(page2, seed + 32, 32); // Last 32 bytes

  // TODO: Add HMAC for integrity verification

  bool success = otp_write_page(OTP_PAGE_SEED_START, page1) &&
                 otp_write_page(OTP_PAGE_SEED_END, page2);

  if (success) {
    // Write config flags
    uint8_t config[OTP_PAGE_SIZE] = {0};
    uint32_t flags = OTP_FLAG_INITIALIZED;
    memcpy(config, &flags, sizeof(flags));
    success = otp_write_page(OTP_PAGE_CONFIG, config);
  }

  restore_interrupts(irq);

  // Clear sensitive data from stack
  memset(page1, 0, sizeof(page1));
  memset(page2, 0, sizeof(page2));

  return success;
}

bool otp_manager_read_seed(uint8_t seed[64]) {
  // TODO: This should only be callable from Secure World (TrustZone)

  if (!otp_manager_is_initialized()) {
    return false;
  }

  uint8_t page1[OTP_PAGE_SIZE];
  uint8_t page2[OTP_PAGE_SIZE];

  if (!otp_read_page(OTP_PAGE_SEED_START, page1) ||
      !otp_read_page(OTP_PAGE_SEED_END, page2)) {
    return false;
  }

  memcpy(seed, page1, 32);
  memcpy(seed + 32, page2, 32);

  // Clear sensitive data from stack
  memset(page1, 0, sizeof(page1));
  memset(page2, 0, sizeof(page2));

  return true;
}

bool otp_manager_lock_seed(void) {
  // TODO: Implement OTP page locking
  // This prevents further reads of the seed pages
  return false;
}

uint32_t otp_manager_get_flags(void) {
  uint8_t config_page[OTP_PAGE_SIZE];

  if (!otp_read_page(OTP_PAGE_CONFIG, config_page)) {
    return 0;
  }

  return *(uint32_t *)config_page;
}

bool otp_manager_verify_integrity(void) {
  // TODO: Implement HMAC verification of stored seed
  return false;
}
