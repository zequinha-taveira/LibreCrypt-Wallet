/**
 * @file secure_world.c
 * @brief TrustZone Secure World Implementation
 *
 * Critical cryptographic operations run in the Secure World,
 * isolated from the main application via ARM TrustZone.
 */

#include "trustzone/secure_world.h"
#include <string.h>

// =============================================================================
// Secure World Entry Points (NSC - Non-Secure Callable)
// =============================================================================

// TODO: Implement TrustZone secure entry points
// These functions are callable from Non-Secure world but execute in Secure
// world

__attribute__((cmse_nonsecure_entry)) bool
secure_derive_key(const uint8_t *path, uint8_t path_len, uint8_t *pubkey_out) {
  if (!path || !pubkey_out) {
    return false;
  }

  // TODO: Load master seed from OTP (only accessible from Secure World)
  // Derive child key according to path
  // Return only public key, never private

  (void)path_len;
  return false;
}

__attribute__((cmse_nonsecure_entry)) bool
secure_sign_hash(const uint8_t *path, uint8_t path_len, const uint8_t hash[32],
                 uint8_t signature[64]) {
  if (!path || !hash || !signature) {
    return false;
  }

  // TODO:
  // 1. Derive private key from path
  // 2. Sign hash with private key
  // 3. Clear private key from memory
  // 4. Return only signature

  (void)path_len;
  return false;
}

__attribute__((cmse_nonsecure_entry)) bool
secure_get_device_pubkey(uint8_t pubkey[33]) {
  if (!pubkey) {
    return false;
  }

  // Return master public key (safe to share)
  return false;
}
