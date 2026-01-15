/**
 * @file dilithium.h
 * @brief ML-DSA (Dilithium) Post-Quantum Digital Signatures
 *
 * ⚠️ EXPERIMENTAL - POST-QUANTUM CRYPTOGRAPHY
 *
 * This is a research implementation of CRYSTALS-Dilithium (now ML-DSA),
 * a lattice-based digital signature scheme standardized by NIST.
 *
 * DO NOT use for production without thorough auditing.
 */

#ifndef LIBRECRYPT_DILITHIUM_H
#define LIBRECRYPT_DILITHIUM_H

#include <stdbool.h>
#include <stdint.h>


// =============================================================================
// Dilithium Parameters (Dilithium3 - recommended security level)
// =============================================================================

#define DILITHIUM_K 6       // Module dimension for A
#define DILITHIUM_L 5       // Module dimension for s1, s2
#define DILITHIUM_N 256     // Polynomial degree
#define DILITHIUM_Q 8380417 // Modulus

#define DILITHIUM_PUBKEY_BYTES 1952
#define DILITHIUM_SECKEY_BYTES 4000
#define DILITHIUM_SIG_BYTES 3293

// =============================================================================
// API Functions
// =============================================================================

/**
 * @brief Generate Dilithium keypair
 * @param pk Output public key
 * @param sk Output secret key
 * @return true on success
 */
bool dilithium_keypair(uint8_t pk[DILITHIUM_PUBKEY_BYTES],
                       uint8_t sk[DILITHIUM_SECKEY_BYTES]);

/**
 * @brief Sign message
 * @param sig Output signature
 * @param sig_len Output signature length
 * @param msg Message to sign
 * @param msg_len Message length
 * @param sk Secret key
 * @return true on success
 */
bool dilithium_sign(uint8_t sig[DILITHIUM_SIG_BYTES], uint32_t *sig_len,
                    const uint8_t *msg, uint32_t msg_len,
                    const uint8_t sk[DILITHIUM_SECKEY_BYTES]);

/**
 * @brief Verify signature
 * @param msg Message
 * @param msg_len Message length
 * @param sig Signature
 * @param sig_len Signature length
 * @param pk Public key
 * @return true if valid
 */
bool dilithium_verify(const uint8_t *msg, uint32_t msg_len,
                      const uint8_t sig[DILITHIUM_SIG_BYTES], uint32_t sig_len,
                      const uint8_t pk[DILITHIUM_PUBKEY_BYTES]);

#endif // LIBRECRYPT_DILITHIUM_H
