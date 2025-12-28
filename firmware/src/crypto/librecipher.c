/**
 * LibreCipher - Implementação
 */

#include "librecipher.h"
#include "hardware/structs/rosc.h"
#include "pico/stdlib.h"
#include <string.h>


/**
 * Inicializa LibreCipher
 */
void librecipher_init(void) {
  // Seed do RNG já inicializado pelo ROSC
}

/**
 * Zera memória de forma segura
 */
void librecipher_secure_zero(void *ptr, size_t len) {
  volatile uint8_t *p = (volatile uint8_t *)ptr;
  while (len--) {
    *p++ = 0;
  }
}

/**
 * Comparação constant-time
 */
int librecipher_secure_compare(const uint8_t *a, const uint8_t *b, size_t len) {
  uint8_t diff = 0;
  for (size_t i = 0; i < len; i++) {
    diff |= a[i] ^ b[i];
  }
  return diff == 0;
}

/**
 * RNG baseado no ROSC do RP2350
 */
void librecipher_random(uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t random_byte = 0;
    for (int bit = 0; bit < 8; bit++) {
      // Lê bit do ROSC
      while (!rosc_hw->randombit) {
        tight_loop_contents();
      }
      random_byte = (random_byte << 1) | (rosc_hw->randombit & 1);
    }
    buf[i] = random_byte;
  }
}

/**
 * SHA-256 - Implementação placeholder
 * TODO: Implementar SHA-256 completo
 */
void librecipher_sha256(const uint8_t *data, size_t len, uint8_t *hash) {
  // Placeholder - usar implementação real
  (void)data;
  (void)len;
  memset(hash, 0, LIBRECIPHER_HASH_SIZE);
}

/**
 * HMAC-SHA256 - Placeholder
 */
void librecipher_hmac_sha256(const uint8_t *key, size_t key_len,
                             const uint8_t *data, size_t data_len,
                             uint8_t *mac) {
  (void)key;
  (void)key_len;
  (void)data;
  (void)data_len;
  memset(mac, 0, LIBRECIPHER_HASH_SIZE);
}

/**
 * KDF - Placeholder
 */
void librecipher_kdf(const uint8_t *password, size_t password_len,
                     const uint8_t *salt, size_t salt_len, const uint8_t *info,
                     size_t info_len, uint8_t *output, size_t output_len) {
  (void)password;
  (void)password_len;
  (void)salt;
  (void)salt_len;
  (void)info;
  (void)info_len;
  memset(output, 0, output_len);
}

/**
 * AES-GCM Encrypt - Placeholder
 */
bool librecipher_encrypt(const uint8_t *key, const uint8_t *nonce,
                         const uint8_t *plaintext, size_t plaintext_len,
                         const uint8_t *aad, size_t aad_len,
                         uint8_t *ciphertext, uint8_t *tag) {
  (void)key;
  (void)nonce;
  (void)aad;
  (void)aad_len;
  memcpy(ciphertext, plaintext, plaintext_len);
  memset(tag, 0, LIBRECIPHER_TAG_SIZE);
  return true;
}

/**
 * AES-GCM Decrypt - Placeholder
 */
bool librecipher_decrypt(const uint8_t *key, const uint8_t *nonce,
                         const uint8_t *ciphertext, size_t ciphertext_len,
                         const uint8_t *aad, size_t aad_len, const uint8_t *tag,
                         uint8_t *plaintext) {
  (void)key;
  (void)nonce;
  (void)aad;
  (void)aad_len;
  (void)tag;
  memcpy(plaintext, ciphertext, ciphertext_len);
  return true;
}
