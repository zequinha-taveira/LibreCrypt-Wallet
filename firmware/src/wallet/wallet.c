/**
 * Wallet Engine - Implementação
 */

#include "wallet.h"
#include "librecipher.h"
#include <string.h>

// Estado da wallet
static wallet_status_t g_status = WALLET_STATUS_UNINITIALIZED;
static uint8_t g_master_key[32];
static uint8_t g_pin_hash[32];

/**
 * Inicializa wallet
 */
void wallet_init(void) {
  librecipher_secure_zero(g_master_key, sizeof(g_master_key));
  librecipher_secure_zero(g_pin_hash, sizeof(g_pin_hash));
  g_status = WALLET_STATUS_UNINITIALIZED;

  // TODO: Verificar se existe wallet salva na flash
}

/**
 * Retorna status
 */
wallet_status_t wallet_get_status(void) { return g_status; }

/**
 * Cria nova wallet
 */
bool wallet_create(const uint8_t *pin, size_t pin_len) {
  if (g_status != WALLET_STATUS_UNINITIALIZED) {
    return false;
  }

  // Gerar seed aleatória
  uint8_t seed[32];
  librecipher_random(seed, sizeof(seed));

  // Hash do PIN
  librecipher_sha256(pin, pin_len, g_pin_hash);

  // Derivar master key
  const uint8_t info[] = "wallet-master";
  librecipher_kdf(seed, sizeof(seed), g_pin_hash, sizeof(g_pin_hash), info,
                  sizeof(info) - 1, g_master_key, sizeof(g_master_key));

  // Limpar seed temporária
  librecipher_secure_zero(seed, sizeof(seed));

  // TODO: Salvar wallet criptografada na flash

  g_status = WALLET_STATUS_UNLOCKED;
  return true;
}

/**
 * Restaura wallet (placeholder)
 */
bool wallet_restore(const char *mnemonic, const uint8_t *pin, size_t pin_len) {
  (void)mnemonic;
  (void)pin;
  (void)pin_len;
  // TODO: Implementar conversão de mnemonic BIP-39
  return false;
}

/**
 * Desbloqueia wallet
 */
bool wallet_unlock(const uint8_t *pin, size_t pin_len) {
  if (g_status != WALLET_STATUS_LOCKED) {
    return false;
  }

  uint8_t pin_hash_attempt[32];
  librecipher_sha256(pin, pin_len, pin_hash_attempt);

  if (!librecipher_secure_compare(pin_hash_attempt, g_pin_hash, 32)) {
    librecipher_secure_zero(pin_hash_attempt, sizeof(pin_hash_attempt));
    return false;
  }

  librecipher_secure_zero(pin_hash_attempt, sizeof(pin_hash_attempt));
  g_status = WALLET_STATUS_UNLOCKED;
  return true;
}

/**
 * Bloqueia wallet
 */
void wallet_lock(void) {
  librecipher_secure_zero(g_master_key, sizeof(g_master_key));
  g_status = WALLET_STATUS_LOCKED;
}

/**
 * Assina transação (placeholder)
 */
bool wallet_sign_transaction(const uint8_t *tx_hash, uint32_t account_index,
                             uint8_t *signature) {
  (void)tx_hash;
  (void)account_index;
  (void)signature;

  if (g_status != WALLET_STATUS_UNLOCKED) {
    return false;
  }

  // TODO: Implementar assinatura Ed25519
  memset(signature, 0, 64);
  return true;
}

/**
 * Obtém endereço (placeholder)
 */
size_t wallet_get_address(uint32_t account_index, char *address,
                          size_t address_len) {
  (void)account_index;

  const char *placeholder = "addr1_placeholder";
  size_t len = strlen(placeholder);

  if (len >= address_len) {
    return 0;
  }

  memcpy(address, placeholder, len + 1);
  return len;
}
