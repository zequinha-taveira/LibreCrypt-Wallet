/**
 * @file bip39.c
 * @brief BIP-39 Mnemonic Implementation
 *
 * TODO: Full implementation pending
 * - English wordlist (2048 words)
 * - PBKDF2-HMAC-SHA512 for seed derivation
 */

#include "crypto/bip39.h"
#include "crypto/sha256_hw.h"
#include "pico/rand.h"
#include <string.h>

// =============================================================================
// BIP-39 English Wordlist (first 10 words for demo)
// TODO: Include full 2048-word list
// =============================================================================

static const char *const BIP39_WORDLIST[] = {
    "abandon", "ability", "able", "about", "above", "absent", "absorb",
    "abstract", "absurd", "abuse",
    // ... 2038 more words
    NULL};

static const uint16_t BIP39_WORDLIST_SIZE = 10; // TODO: 2048

// =============================================================================
// Hardware TRNG Access
// =============================================================================

static bool get_random_bytes(uint8_t *buffer, uint16_t len) {
  // Use RP2350 hardware TRNG
  for (uint16_t i = 0; i < len; i += 4) {
    uint32_t rand_val = get_rand_32();
    uint16_t copy_len = (len - i) < 4 ? (len - i) : 4;
    memcpy(buffer + i, &rand_val, copy_len);
  }
  return true;
}

// =============================================================================
// Public API
// =============================================================================

bool bip39_generate(uint8_t word_count, uint16_t *words) {
  if (!words ||
      (word_count != BIP39_WORDS_12 && word_count != BIP39_WORDS_24)) {
    return false;
  }

  // Calculate entropy size
  uint8_t entropy_bytes =
      (word_count == BIP39_WORDS_12) ? BIP39_ENTROPY_128 : BIP39_ENTROPY_256;

  uint8_t entropy[BIP39_ENTROPY_256];

  // Generate entropy from hardware TRNG
  if (!get_random_bytes(entropy, entropy_bytes)) {
    return false;
  }

  // TODO: Calculate checksum (SHA-256 of entropy)
  // Checksum bits = entropy_bits / 32

  // TODO: Convert entropy + checksum to word indices
  // Each word = 11 bits

  // Placeholder: just use random indices
  for (uint8_t i = 0; i < word_count; i++) {
    words[i] = entropy[i % entropy_bytes] % BIP39_WORDLIST_SIZE;
  }

  // Clear entropy from memory
  memset(entropy, 0, sizeof(entropy));

  return true;
}

bool bip39_validate(const uint16_t *words, uint8_t word_count) {
  if (!words ||
      (word_count != BIP39_WORDS_12 && word_count != BIP39_WORDS_24)) {
    return false;
  }

  // TODO: Reconstruct entropy and verify checksum

  // Check all words are valid indices
  for (uint8_t i = 0; i < word_count; i++) {
    if (words[i] >= BIP39_WORDLIST_SIZE) {
      return false;
    }
  }

  return true;
}

bool bip39_to_seed(const uint16_t *words, uint8_t word_count,
                   const char *passphrase, uint8_t seed[BIP39_SEED_SIZE]) {
  if (!words || !seed) {
    return false;
  }

  // TODO: Implement PBKDF2-HMAC-SHA512
  // Password = mnemonic sentence (space-separated words)
  // Salt = "mnemonic" + passphrase
  // Iterations = 2048
  // Output = 64 bytes

  (void)word_count;
  (void)passphrase;

  return false;
}

const char *bip39_get_word(uint16_t index) {
  if (index >= BIP39_WORDLIST_SIZE) {
    return NULL;
  }
  return BIP39_WORDLIST[index];
}

uint16_t bip39_find_word(const char *word) {
  if (!word) {
    return 0xFFFF;
  }

  for (uint16_t i = 0; i < BIP39_WORDLIST_SIZE; i++) {
    if (strcmp(word, BIP39_WORDLIST[i]) == 0) {
      return i;
    }
  }

  return 0xFFFF;
}

uint16_t bip39_to_string(const uint16_t *words, uint8_t word_count,
                         char *output) {
  if (!words || !output) {
    return 0;
  }

  uint16_t pos = 0;
  for (uint8_t i = 0; i < word_count; i++) {
    const char *word = bip39_get_word(words[i]);
    if (!word) {
      return 0;
    }

    if (i > 0) {
      output[pos++] = ' ';
    }

    uint8_t len = strlen(word);
    memcpy(output + pos, word, len);
    pos += len;
  }

  output[pos] = '\0';
  return pos;
}
