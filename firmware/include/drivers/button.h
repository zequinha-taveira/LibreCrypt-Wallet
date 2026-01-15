/**
 * @file button.h
 * @brief BOOTSEL Button Driver
 *
 * Helper to read the RP2350 BOOTSEL button status at runtime.
 */

#ifndef LIBRECRYPT_BUTTON_H
#define LIBRECRYPT_BUTTON_H

#include <stdbool.h>

/**
 * @brief Initialize button driver
 */
void button_init(void);

/**
 * @brief Check if BOOTSEL button is currently pressed
 * @return true if pressed
 */
bool button_is_pressed(void);

#endif // LIBRECRYPT_BUTTON_H
