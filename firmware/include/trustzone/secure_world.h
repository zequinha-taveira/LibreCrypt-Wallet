/**
 * @file secure_world.h
 * @brief TrustZone Secure World Interface
 */

#ifndef LIBRECRYPT_SECURE_WORLD_H
#define LIBRECRYPT_SECURE_WORLD_H

#include <stdbool.h>
#include <stdint.h>


// NSC entry points callable from Non-Secure world
bool secure_derive_key(const uint8_t *path, uint8_t path_len,
                       uint8_t *pubkey_out);
bool secure_sign_hash(const uint8_t *path, uint8_t path_len,
                      const uint8_t hash[32], uint8_t signature[64]);
bool secure_get_device_pubkey(uint8_t pubkey[33]);

#endif // LIBRECRYPT_SECURE_WORLD_H
