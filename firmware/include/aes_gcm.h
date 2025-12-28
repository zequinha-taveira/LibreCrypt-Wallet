/**
 * LibreCipher AES-256-GCM Implementation
 *
 * Authenticated encryption with associated data
 * Constant-time implementation for embedded systems
 */

#ifndef AES_GCM_H
#define AES_GCM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE 32 // AES-256
#define AES_GCM_IV_SIZE 12
#define AES_GCM_TAG_SIZE 16

typedef struct {
  uint32_t rk[60]; // Round keys (14 rounds for AES-256)
} aes_ctx_t;

typedef struct {
  aes_ctx_t aes;
  uint8_t H[AES_BLOCK_SIZE];  // GHASH key
  uint8_t J0[AES_BLOCK_SIZE]; // Initial counter
  uint8_t counter[AES_BLOCK_SIZE];
  uint8_t ghash[AES_BLOCK_SIZE];
  uint64_t aad_len;
  uint64_t ct_len;
} aes_gcm_ctx_t;

/**
 * Initialize AES-256 context with key
 */
void aes256_init(aes_ctx_t *ctx, const uint8_t key[32]);

/**
 * Encrypt single block (ECB mode, internal use)
 */
void aes256_encrypt_block(const aes_ctx_t *ctx, const uint8_t in[16],
                          uint8_t out[16]);

/**
 * Initialize AES-GCM context
 */
void aes_gcm_init(aes_gcm_ctx_t *ctx, const uint8_t key[32],
                  const uint8_t iv[12]);

/**
 * Add additional authenticated data
 */
void aes_gcm_aad(aes_gcm_ctx_t *ctx, const uint8_t *aad, size_t aad_len);

/**
 * Encrypt plaintext
 */
void aes_gcm_encrypt(aes_gcm_ctx_t *ctx, const uint8_t *plaintext, size_t len,
                     uint8_t *ciphertext);

/**
 * Finalize and get authentication tag
 */
void aes_gcm_finish(aes_gcm_ctx_t *ctx, uint8_t tag[16]);

/**
 * Decrypt and verify (returns false if authentication fails)
 */
bool aes_gcm_decrypt_verify(const uint8_t key[32], const uint8_t iv[12],
                            const uint8_t *ciphertext, size_t ct_len,
                            const uint8_t *aad, size_t aad_len,
                            const uint8_t tag[16], uint8_t *plaintext);

/**
 * One-shot encrypt with GCM
 */
void aes_gcm_encrypt_full(const uint8_t key[32], const uint8_t iv[12],
                          const uint8_t *plaintext, size_t pt_len,
                          const uint8_t *aad, size_t aad_len,
                          uint8_t *ciphertext, uint8_t tag[16]);

#endif // AES_GCM_H
