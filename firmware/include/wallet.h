/**
 * Wallet Engine - Gerenciamento de Carteira
 */

#ifndef WALLET_H
#define WALLET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Status da wallet
typedef enum {
  WALLET_STATUS_UNINITIALIZED = 0,
  WALLET_STATUS_LOCKED,
  WALLET_STATUS_UNLOCKED
} wallet_status_t;

/**
 * Inicializa o módulo de wallet
 */
void wallet_init(void);

/**
 * Retorna status atual da wallet
 */
wallet_status_t wallet_get_status(void);

/**
 * Cria nova wallet com seed gerada internamente
 * @param pin PIN do usuário (hash)
 * @param pin_len Tamanho do PIN
 * @return true se sucesso
 */
bool wallet_create(const uint8_t *pin, size_t pin_len);

/**
 * Restaura wallet de mnemonic
 * @param mnemonic BIP-39 mnemonic
 * @param pin PIN do usuário
 * @return true se sucesso
 */
bool wallet_restore(const char *mnemonic, const uint8_t *pin, size_t pin_len);

/**
 * Desbloqueia wallet com PIN
 * @return true se PIN correto
 */
bool wallet_unlock(const uint8_t *pin, size_t pin_len);

/**
 * Bloqueia wallet
 */
void wallet_lock(void);

/**
 * Assina transação
 * @param tx_hash Hash da transação (32 bytes)
 * @param account_index Índice da conta BIP-44
 * @param signature Output (64 bytes)
 * @return true se sucesso
 */
bool wallet_sign_transaction(const uint8_t *tx_hash, uint32_t account_index,
                             uint8_t *signature);

/**
 * Obtém endereço público
 * @param account_index Índice da conta
 * @param address Output
 * @param address_len Tamanho máximo
 * @return tamanho do endereço
 */
size_t wallet_get_address(uint32_t account_index, char *address,
                          size_t address_len);

#endif // WALLET_H
