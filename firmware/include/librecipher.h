/**
 * LibreCipher - Biblioteca Criptográfica
 *
 * Suite criptográfica constant-time para hardware wallet
 */

#ifndef LIBRECIPHER_H
#define LIBRECIPHER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Tamanhos de primitivos
#define LIBRECIPHER_HASH_SIZE 32
#define LIBRECIPHER_KEY_SIZE 32
#define LIBRECIPHER_SALT_SIZE 32
#define LIBRECIPHER_NONCE_SIZE 12
#define LIBRECIPHER_TAG_SIZE 16

/**
 * Inicializa a biblioteca LibreCipher
 */
void librecipher_init(void);

/**
 * Zera memória de forma segura (não otimizado pelo compilador)
 */
void librecipher_secure_zero(void *ptr, size_t len);

/**
 * Comparação constant-time
 * @return 1 se iguais, 0 se diferentes
 */
int librecipher_secure_compare(const uint8_t *a, const uint8_t *b, size_t len);

/**
 * Gerador de números aleatórios (TRNG)
 */
void librecipher_random(uint8_t *buf, size_t len);

/**
 * SHA-256 Hash
 */
void librecipher_sha256(const uint8_t *data, size_t len, uint8_t *hash);

/**
 * HMAC-SHA256
 */
void librecipher_hmac_sha256(const uint8_t *key, size_t key_len,
                             const uint8_t *data, size_t data_len,
                             uint8_t *mac);

/**
 * Key Derivation (HKDF)
 */
void librecipher_kdf(const uint8_t *password, size_t password_len,
                     const uint8_t *salt, size_t salt_len, const uint8_t *info,
                     size_t info_len, uint8_t *output, size_t output_len);

/**
 * AES-256-GCM Encrypt
 * @return true se sucesso
 */
bool librecipher_encrypt(const uint8_t *key, const uint8_t *nonce,
                         const uint8_t *plaintext, size_t plaintext_len,
                         const uint8_t *aad, size_t aad_len,
                         uint8_t *ciphertext, uint8_t *tag);

/**
 * AES-256-GCM Decrypt
 * @return true se autenticação OK
 */
bool librecipher_decrypt(const uint8_t *key, const uint8_t *nonce,
                         const uint8_t *ciphertext, size_t ciphertext_len,
                         const uint8_t *aad, size_t aad_len, const uint8_t *tag,
                         uint8_t *plaintext);

#endif // LIBRECIPHER_H
