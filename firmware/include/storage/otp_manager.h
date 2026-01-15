/**
 * @file otp_manager.h
 * @brief OTP Memory Manager for Secure Key Storage
 *
 * The RP2350 has 8KB of OTP (One-Time Programmable) memory.
 * This module manages the secure storage of the master seed
 * and other security-critical data.
 *
 * SECURITY WARNING:
 * - OTP can only be written ONCE
 * - Data written to OTP is PERMANENT
 * - Use locking to prevent unauthorized reads
 */

#ifndef LIBRECRYPT_OTP_MANAGER_H
#define LIBRECRYPT_OTP_MANAGER_H

#include <stdbool.h>
#include <stdint.h>


// =============================================================================
// OTP Layout (8KB total)
// =============================================================================
// Page 0 (128B): Device configuration flags
// Page 1-2 (256B): Master seed (64 bytes) + HMAC
// Page 3 (128B): Boot key fingerprint
// Page 4-63: Reserved for future use

#define OTP_PAGE_SIZE 128
#define OTP_TOTAL_PAGES 64
#define OTP_TOTAL_SIZE (OTP_PAGE_SIZE * OTP_TOTAL_PAGES)

#define OTP_PAGE_CONFIG 0
#define OTP_PAGE_SEED_START 1
#define OTP_PAGE_SEED_END 2
#define OTP_PAGE_BOOT_KEY 3

// Configuration flags (Page 0)
#define OTP_FLAG_INITIALIZED (1 << 0)
#define OTP_FLAG_SEED_LOCKED (1 << 1)
#define OTP_FLAG_DEBUG_DISABLED (1 << 2)

// =============================================================================
// API Functions
// =============================================================================

/**
 * @brief Check if device has been initialized (OTP programmed)
 * @return true if master seed exists in OTP
 */
bool otp_manager_is_initialized(void);

/**
 * @brief Initialize device with master seed
 * @param seed 64-byte master seed from BIP-39
 * @return true on success, false if already initialized or error
 *
 * WARNING: This operation is IRREVERSIBLE
 */
bool otp_manager_init_seed(const uint8_t seed[64]);

/**
 * @brief Read master seed (only allowed in Secure World)
 * @param seed Output buffer for 64-byte seed
 * @return true on success
 */
bool otp_manager_read_seed(uint8_t seed[64]);

/**
 * @brief Lock seed pages to prevent future reads
 * @return true on success
 *
 * After locking, seed can only be used internally by crypto engine
 */
bool otp_manager_lock_seed(void);

/**
 * @brief Get current OTP configuration flags
 */
uint32_t otp_manager_get_flags(void);

/**
 * @brief Verify seed integrity using stored HMAC
 */
bool otp_manager_verify_integrity(void);

#endif // LIBRECRYPT_OTP_MANAGER_H
