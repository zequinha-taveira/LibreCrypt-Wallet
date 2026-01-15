/**
 * @file bip39.h
 * @brief BIP-39 Mnemonic Seed Generation
 *
 * Generates and validates mnemonic phrases for wallet backup.
 * Uses hardware TRNG for entropy generation.
 */

#ifndef LIBRECRYPT_BIP39_H
#define LIBRECRYPT_BIP39_H

#include <stdbool.h>
#include <stdint.h>


// =============================================================================
// Constants
// =============================================================================

#define BIP39_WORDS_12 12
#define BIP39_WORDS_24 24
#define BIP39_ENTROPY_128 16 // 128 bits = 12 words
#define BIP39_ENTROPY_256 32 // 256 bits = 24 words
#define BIP39_SEED_SIZE 64
#define BIP39_MAX_WORD_LEN 8 // Longest word in wordlist

// =============================================================================
// API Functions
// =============================================================================

/**
 * @brief Generate new mnemonic from hardware TRNG
 * @param word_count Number of words (12 or 24)
 * @param words Output array of word indices
 * @return true on success
 */
bool bip39_generate(uint8_t word_count, uint16_t *words);

/**
 * @brief Validate mnemonic checksum
 * @param words Array of word indices
 * @param word_count Number of words
 * @return true if valid
 */
bool bip39_validate(const uint16_t *words, uint8_t word_count);

/**
 * @brief Convert mnemonic to seed
 * @param words Array of word indices
 * @param word_count Number of words
 * @param passphrase Optional passphrase (can be NULL)
 * @param seed Output 64-byte seed
 * @return true on success
 */
bool bip39_to_seed(const uint16_t *words, uint8_t word_count,
                   const char *passphrase, uint8_t seed[BIP39_SEED_SIZE]);

/**
 * @brief Get word string from index
 * @param index Word index (0-2047)
 * @return Pointer to word string, or NULL if invalid
 */
const char *bip39_get_word(uint16_t index);

/**
 * @brief Find word index from string
 * @param word Word string
 * @return Word index, or 0xFFFF if not found
 */
uint16_t bip39_find_word(const char *word);

/**
 * @brief Get mnemonic as space-separated string
 * @param words Array of word indices
 * @param word_count Number of words
 * @param output Output buffer (at least word_count * 9 bytes)
 * @return Length of output string
 */
uint16_t bip39_to_string(const uint16_t *words, uint8_t word_count,
                         char *output);

#endif // LIBRECRYPT_BIP39_H
