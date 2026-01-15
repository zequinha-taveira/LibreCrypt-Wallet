/**
 * @file bip32.h
 * @brief BIP-32 Hierarchical Deterministic Wallet
 *
 * Implementation of HD key derivation as per BIP-32 spec.
 * Uses SHA-256 hardware acceleration on RP2350.
 */

#ifndef LIBRECRYPT_BIP32_H
#define LIBRECRYPT_BIP32_H

#include <stdbool.h>
#include <stdint.h>


// =============================================================================
// Constants
// =============================================================================

#define BIP32_KEY_SIZE 32
#define BIP32_CHAINCODE_SIZE 32
#define BIP32_EXTKEY_SIZE (BIP32_KEY_SIZE + BIP32_CHAINCODE_SIZE)
#define BIP32_PUBKEY_SIZE 33 // Compressed public key

// Hardened key derivation threshold
#define BIP32_HARDENED 0x80000000

// =============================================================================
// Types
// =============================================================================

typedef struct {
  uint8_t private_key[BIP32_KEY_SIZE];
  uint8_t chain_code[BIP32_CHAINCODE_SIZE];
  uint8_t depth;
  uint32_t child_index;
  uint8_t parent_fingerprint[4];
} bip32_key_t;

// =============================================================================
// API Functions
// =============================================================================

/**
 * @brief Initialize master key from BIP-39 seed
 * @param seed 64-byte seed from BIP-39 mnemonic
 * @param key Output master key
 * @return true on success
 */
bool bip32_from_seed(const uint8_t seed[64], bip32_key_t *key);

/**
 * @brief Derive child key from parent
 * @param parent Parent extended key
 * @param index Child index (use BIP32_HARDENED for hardened derivation)
 * @param child Output child key
 * @return true on success
 */
bool bip32_derive_child(const bip32_key_t *parent, uint32_t index,
                        bip32_key_t *child);

/**
 * @brief Derive key from path string
 * @param master Master key
 * @param path Derivation path (e.g., "m/44'/0'/0'/0/0")
 * @param result Output derived key
 * @return true on success
 */
bool bip32_derive_path(const bip32_key_t *master, const char *path,
                       bip32_key_t *result);

/**
 * @brief Get compressed public key from private key
 * @param key Extended private key
 * @param pubkey Output 33-byte compressed public key
 * @return true on success
 */
bool bip32_get_pubkey(const bip32_key_t *key,
                      uint8_t pubkey[BIP32_PUBKEY_SIZE]);

/**
 * @brief Get key fingerprint (first 4 bytes of HASH160(pubkey))
 */
bool bip32_get_fingerprint(const bip32_key_t *key, uint8_t fingerprint[4]);

/**
 * @brief Serialize extended key to Base58Check format
 * @param key Extended key
 * @param is_private true for xprv, false for xpub
 * @param output Output buffer (at least 112 bytes)
 * @return Length of serialized string, 0 on error
 */
uint8_t bip32_serialize(const bip32_key_t *key, bool is_private, char *output);

/**
 * @brief Securely wipe key from memory
 */
void bip32_wipe(bip32_key_t *key);

#endif // LIBRECRYPT_BIP32_H
