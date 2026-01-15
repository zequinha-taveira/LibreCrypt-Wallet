/**
 * @file kyber.h
 * @brief ML-KEM (Kyber) Post-Quantum Key Encapsulation
 *
 * ⚠️ EXPERIMENTAL - POST-QUANTUM CRYPTOGRAPHY
 *
 * This is a research implementation of CRYSTALS-Kyber (now ML-KEM),
 * a lattice-based key encapsulation mechanism standardized by NIST.
 *
 * DO NOT use for production without thorough auditing.
 */

#ifndef LIBRECRYPT_KYBER_H
#define LIBRECRYPT_KYBER_H

#include <stdbool.h>
#include <stdint.h>


// =============================================================================
// Kyber Parameters (Kyber-768 - recommended security level)
// =============================================================================

#define KYBER_K 3    // Module dimension
#define KYBER_N 256  // Polynomial degree
#define KYBER_Q 3329 // Modulus

#define KYBER_PUBKEY_BYTES 1184
#define KYBER_SECKEY_BYTES 2400
#define KYBER_CIPHERTEXT_BYTES 1088
#define KYBER_SHARED_SECRET_BYTES 32

// =============================================================================
// API Functions
// =============================================================================

/**
 * @brief Generate Kyber keypair
 * @param pk Output public key
 * @param sk Output secret key
 * @return true on success
 */
bool kyber_keypair(uint8_t pk[KYBER_PUBKEY_BYTES],
                   uint8_t sk[KYBER_SECKEY_BYTES]);

/**
 * @brief Encapsulate shared secret
 * @param ct Output ciphertext
 * @param ss Output shared secret
 * @param pk Recipient's public key
 * @return true on success
 */
bool kyber_encapsulate(uint8_t ct[KYBER_CIPHERTEXT_BYTES],
                       uint8_t ss[KYBER_SHARED_SECRET_BYTES],
                       const uint8_t pk[KYBER_PUBKEY_BYTES]);

/**
 * @brief Decapsulate shared secret
 * @param ss Output shared secret
 * @param ct Ciphertext
 * @param sk Secret key
 * @return true on success
 */
bool kyber_decapsulate(uint8_t ss[KYBER_SHARED_SECRET_BYTES],
                       const uint8_t ct[KYBER_CIPHERTEXT_BYTES],
                       const uint8_t sk[KYBER_SECKEY_BYTES]);

#endif // LIBRECRYPT_KYBER_H
