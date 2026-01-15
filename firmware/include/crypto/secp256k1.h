/**
 * @file secp256k1.h
 * @brief secp256k1 Elliptic Curve Cryptography
 */

#ifndef LIBRECRYPT_SECP256K1_H
#define LIBRECRYPT_SECP256K1_H

#include <stdbool.h>
#include <stdint.h>


#define SECP256K1_PRIVKEY_SIZE 32
#define SECP256K1_PUBKEY_SIZE 33 // Compressed
#define SECP256K1_SIG_SIZE 64    // r + s

/**
 * @brief Generate public key from private key
 */
bool secp256k1_keypair_generate(const uint8_t private_key[32],
                                uint8_t public_key[33]);

/**
 * @brief Sign message hash with private key (random k)
 */
bool secp256k1_sign(const uint8_t private_key[32],
                    const uint8_t message_hash[32], uint8_t signature[64]);

/**
 * @brief Sign with deterministic k (RFC 6979)
 */
bool secp256k1_sign_deterministic(const uint8_t private_key[32],
                                  const uint8_t message_hash[32],
                                  uint8_t signature[64]);

/**
 * @brief Verify signature against public key
 */
bool secp256k1_verify(const uint8_t public_key[33],
                      const uint8_t message_hash[32],
                      const uint8_t signature[64]);

/**
 * @brief Recover public key from signature
 */
bool secp256k1_recover_pubkey(const uint8_t message_hash[32],
                              const uint8_t signature[64], uint8_t recovery_id,
                              uint8_t public_key[33]);

#endif // LIBRECRYPT_SECP256K1_H
