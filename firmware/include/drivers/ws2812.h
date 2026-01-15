/**
 * @file ws2812.h
 * @brief WS2812 RGB LED Driver
 *
 * Drives the onboard WS2812 addressable LED using PIO.
 */

#ifndef LIBRECRYPT_WS2812_H
#define LIBRECRYPT_WS2812_H

#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Initialize WS2812 LED
 * @param pin GPIO pin number
 * @return true on success
 */
bool ws2812_init(uint8_t pin);

/**
 * @brief Set LED color
 * @param grb Color in GRB format (0xGGRRBB)
 */
void ws2812_set_color(uint32_t grb);

/**
 * @brief Set LED color from RGB values
 */
void ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Turn off LED
 */
void ws2812_off(void);

/**
 * @brief Blink LED with specified color and timing
 * @param grb Color
 * @param on_ms On duration
 * @param off_ms Off duration
 * @param count Number of blinks (0 = infinite)
 */
void ws2812_blink(uint32_t grb, uint16_t on_ms, uint16_t off_ms, uint8_t count);

/**
 * @brief Pulse LED (fade in/out)
 * @param grb Color
 * @param period_ms Full pulse period
 */
void ws2812_pulse(uint32_t grb, uint16_t period_ms);

#endif // LIBRECRYPT_WS2812_H
