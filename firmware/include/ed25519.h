/**
 * Ed25519 Digital Signature Implementation
 *
 * Based on RFC 8032 - Edwards-Curve Digital Signature Algorithm (EdDSA)
 * Optimized for embedded systems (RP2350)
 *
 * Uses Curve25519 in Edwards form (Ed25519)
 */

#ifndef ED25519_H
#define ED25519_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Key and signature sizes
#define ED25519_SEED_SIZE 32
#define ED25519_PUBLIC_KEY_SIZE 32
#define ED25519_SECRET_KEY_SIZE 64 // seed + public key
#define ED25519_SIGNATURE_SIZE 64

/**
 * Key pair structure
 */
typedef struct {
  uint8_t public_key[ED25519_PUBLIC_KEY_SIZE];
  uint8_t secret_key[ED25519_SECRET_KEY_SIZE];
} ed25519_keypair_t;

/**
 * Generate key pair from seed
 * @param seed 32-byte random seed
 * @param keypair output key pair
 */
void ed25519_create_keypair(const uint8_t seed[32], ed25519_keypair_t *keypair);

/**
 * Generate key pair with random seed
 * Uses LibreCipher RNG
 * @param keypair output key pair
 */
void ed25519_generate_keypair(ed25519_keypair_t *keypair);

/**
 * Sign message
 * @param signature output (64 bytes)
 * @param message message to sign
 * @param message_len message length
 * @param secret_key secret key (64 bytes)
 */
void ed25519_sign(uint8_t signature[64], const uint8_t *message,
                  size_t message_len, const uint8_t secret_key[64]);

/**
 * Verify signature
 * @param signature signature to verify (64 bytes)
 * @param message original message
 * @param message_len message length
 * @param public_key public key (32 bytes)
 * @return true if valid
 */
bool ed25519_verify(const uint8_t signature[64], const uint8_t *message,
                    size_t message_len, const uint8_t public_key[32]);

/**
 * Get public key from secret key
 * @param public_key output (32 bytes)
 * @param secret_key secret key (64 bytes)
 */
void ed25519_get_public_key(uint8_t public_key[32],
                            const uint8_t secret_key[64]);

#endif // ED25519_H
