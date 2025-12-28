/**
 * LibreCipher - Implementação Principal
 *
 * Integra todos os primitivos criptográficos
 */

#include "librecipher.h"
#include "aes_gcm.h"
#include "hardware/structs/rosc.h"
#include "pico/stdlib.h"
#include "sha256.h"
#include <string.h>

/**
 * Inicializa LibreCipher
 */
void librecipher_init(void) {
  // ROSC já inicializado pelo SDK
}

/**
 * Zera memória de forma segura (não otimizado pelo compilador)
 */
void librecipher_secure_zero(void *ptr, size_t len) {
  volatile uint8_t *p = (volatile uint8_t *)ptr;
  while (len--) {
    *p++ = 0;
  }
  // Memory barrier
  __asm__ volatile("" ::: "memory");
}

/**
 * Comparação constant-time
 */
int librecipher_secure_compare(const uint8_t *a, const uint8_t *b, size_t len) {
  uint8_t diff = 0;
  for (size_t i = 0; i < len; i++) {
    diff |= a[i] ^ b[i];
  }
  // Constant-time zero check
  return ((diff - 1) >> 8) & 1;
}

/**
 * RNG baseado no ROSC do RP2350
 * Coleta entropia do oscilador de anel
 */
void librecipher_random(uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t random_byte = 0;
    for (int bit = 0; bit < 8; bit++) {
      // Aguarda bit disponível
      while ((rosc_hw->status & ROSC_STATUS_ENABLED_BITS) == 0) {
        tight_loop_contents();
      }
      // Coleta bit do ROSC
      random_byte = (random_byte << 1) | (rosc_hw->randombit & 1);

      // Pequeno delay para decorrelação
      for (volatile int j = 0; j < 10; j++) {
      }
    }
    buf[i] = random_byte;
  }
}

/**
 * SHA-256 Hash (usa implementação real)
 */
void librecipher_sha256(const uint8_t *data, size_t len, uint8_t *hash) {
  sha256_hash(data, len, hash);
}

/**
 * HMAC-SHA256
 */
void librecipher_hmac_sha256(const uint8_t *key, size_t key_len,
                             const uint8_t *data, size_t data_len,
                             uint8_t *mac) {
  uint8_t k_pad[64];
  uint8_t k_ipad[64];
  uint8_t k_opad[64];
  sha256_ctx_t ctx;

  // Key processing
  if (key_len > 64) {
    sha256_hash(key, key_len, k_pad);
    memset(k_pad + 32, 0, 32);
  } else {
    memcpy(k_pad, key, key_len);
    memset(k_pad + key_len, 0, 64 - key_len);
  }

  // Create ipad and opad
  for (int i = 0; i < 64; i++) {
    k_ipad[i] = k_pad[i] ^ 0x36;
    k_opad[i] = k_pad[i] ^ 0x5c;
  }

  // Inner hash: H(K XOR ipad || data)
  uint8_t inner_hash[32];
  sha256_init(&ctx);
  sha256_update(&ctx, k_ipad, 64);
  sha256_update(&ctx, data, data_len);
  sha256_final(&ctx, inner_hash);

  // Outer hash: H(K XOR opad || inner_hash)
  sha256_init(&ctx);
  sha256_update(&ctx, k_opad, 64);
  sha256_update(&ctx, inner_hash, 32);
  sha256_final(&ctx, mac);

  // Clear sensitive data
  librecipher_secure_zero(k_pad, sizeof(k_pad));
  librecipher_secure_zero(k_ipad, sizeof(k_ipad));
  librecipher_secure_zero(k_opad, sizeof(k_opad));
  librecipher_secure_zero(inner_hash, sizeof(inner_hash));
}

/**
 * Key Derivation (HKDF - RFC 5869)
 */
void librecipher_kdf(const uint8_t *password, size_t password_len,
                     const uint8_t *salt, size_t salt_len, const uint8_t *info,
                     size_t info_len, uint8_t *output, size_t output_len) {
  uint8_t prk[32]; // Pseudorandom key
  uint8_t t[32];
  size_t offset = 0;
  uint8_t counter = 1;

  // Extract: PRK = HMAC(salt, IKM)
  if (salt_len > 0) {
    librecipher_hmac_sha256(salt, salt_len, password, password_len, prk);
  } else {
    // Use zero salt
    uint8_t zero_salt[32] = {0};
    librecipher_hmac_sha256(zero_salt, 32, password, password_len, prk);
  }

  // Expand
  memset(t, 0, 32);

  while (offset < output_len) {
    sha256_ctx_t ctx;
    uint8_t hmac_input[32 + 256 + 1]; // max size
    size_t input_len = 0;

    if (counter > 1) {
      memcpy(hmac_input, t, 32);
      input_len = 32;
    }

    if (info_len > 0) {
      memcpy(hmac_input + input_len, info, info_len);
      input_len += info_len;
    }

    hmac_input[input_len] = counter;
    input_len++;

    librecipher_hmac_sha256(prk, 32, hmac_input, input_len, t);

    size_t copy_len = output_len - offset;
    if (copy_len > 32)
      copy_len = 32;

    memcpy(output + offset, t, copy_len);
    offset += copy_len;
    counter++;
  }

  // Clear sensitive data
  librecipher_secure_zero(prk, sizeof(prk));
  librecipher_secure_zero(t, sizeof(t));
}

/**
 * AES-256-GCM Encrypt
 */
bool librecipher_encrypt(const uint8_t *key, const uint8_t *nonce,
                         const uint8_t *plaintext, size_t plaintext_len,
                         const uint8_t *aad, size_t aad_len,
                         uint8_t *ciphertext, uint8_t *tag) {
  aes_gcm_encrypt_full(key, nonce, plaintext, plaintext_len, aad, aad_len,
                       ciphertext, tag);
  return true;
}

/**
 * AES-256-GCM Decrypt
 */
bool librecipher_decrypt(const uint8_t *key, const uint8_t *nonce,
                         const uint8_t *ciphertext, size_t ciphertext_len,
                         const uint8_t *aad, size_t aad_len, const uint8_t *tag,
                         uint8_t *plaintext) {
  return aes_gcm_decrypt_verify(key, nonce, ciphertext, ciphertext_len, aad,
                                aad_len, tag, plaintext);
}

// ============ Ed25519 Wrappers ============

#include "ed25519.h"

/**
 * Generate Ed25519 key pair
 */
void librecipher_ed25519_keygen(const uint8_t *seed, uint8_t *public_key,
                                uint8_t *secret_key) {
  ed25519_keypair_t keypair;
  ed25519_create_keypair(seed, &keypair);
  memcpy(public_key, keypair.public_key, 32);
  memcpy(secret_key, keypair.secret_key, 64);
  librecipher_secure_zero(&keypair, sizeof(keypair));
}

/**
 * Sign message with Ed25519
 */
void librecipher_ed25519_sign(uint8_t *signature, const uint8_t *message,
                              size_t message_len, const uint8_t *secret_key) {
  ed25519_sign(signature, message, message_len, secret_key);
}

/**
 * Verify Ed25519 signature
 */
bool librecipher_ed25519_verify(const uint8_t *signature,
                                const uint8_t *message, size_t message_len,
                                const uint8_t *public_key) {
  return ed25519_verify(signature, message, message_len, public_key);
}
