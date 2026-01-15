/**
 * @file flash_encrypted.h
 * @brief Encrypted Flash Storage Header
 */

#ifndef LIBRECRYPT_FLASH_ENCRYPTED_H
#define LIBRECRYPT_FLASH_ENCRYPTED_H

#include <stdbool.h>
#include <stdint.h>


bool flash_encrypted_init(void);
bool flash_encrypted_read(uint32_t offset, uint8_t *data, uint32_t len);
bool flash_encrypted_write(uint32_t offset, const uint8_t *data, uint32_t len);

#endif // LIBRECRYPT_FLASH_ENCRYPTED_H
