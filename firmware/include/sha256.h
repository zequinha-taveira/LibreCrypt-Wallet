/**
 * LibreCipher SHA-256 Implementation
 *
 * Constant-time SHA-256 following FIPS 180-4
 * Optimized for RP2350 (ARM Cortex-M33)
 */

#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>


#define SHA256_BLOCK_SIZE 64
#define SHA256_DIGEST_SIZE 32

typedef struct {
  uint32_t state[8];
  uint64_t count;
  uint8_t buffer[SHA256_BLOCK_SIZE];
} sha256_ctx_t;

/**
 * Initialize SHA-256 context
 */
void sha256_init(sha256_ctx_t *ctx);

/**
 * Update hash with data
 */
void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len);

/**
 * Finalize hash and output digest
 */
void sha256_final(sha256_ctx_t *ctx, uint8_t *digest);

/**
 * One-shot SHA-256
 */
void sha256_hash(const uint8_t *data, size_t len, uint8_t *digest);

#endif // SHA256_H
