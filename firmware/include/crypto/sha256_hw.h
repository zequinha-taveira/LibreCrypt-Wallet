/**
 * @file sha256_hw.h
 * @brief Hardware-Accelerated SHA-256
 *
 * Uses RP2350's built-in SHA-256 accelerator for high performance.
 */

#ifndef LIBRECRYPT_SHA256_HW_H
#define LIBRECRYPT_SHA256_HW_H

#include <stdbool.h>
#include <stdint.h>


#define SHA256_BLOCK_SIZE 64
#define SHA256_DIGEST_SIZE 32

typedef struct {
  uint32_t state[8];
  uint64_t count;
  uint8_t buffer[SHA256_BLOCK_SIZE];
} sha256_ctx_t;

/**
 * @brief Initialize SHA-256 context
 */
void sha256_init(sha256_ctx_t *ctx);

/**
 * @brief Update hash with data
 */
void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, uint32_t len);

/**
 * @brief Finalize and get digest
 */
void sha256_final(sha256_ctx_t *ctx, uint8_t digest[SHA256_DIGEST_SIZE]);

/**
 * @brief One-shot SHA-256 hash
 */
void sha256(const uint8_t *data, uint32_t len,
            uint8_t digest[SHA256_DIGEST_SIZE]);

/**
 * @brief Double SHA-256 (SHA256(SHA256(data))) - used in Bitcoin
 */
void sha256d(const uint8_t *data, uint32_t len,
             uint8_t digest[SHA256_DIGEST_SIZE]);

#endif // LIBRECRYPT_SHA256_HW_H
