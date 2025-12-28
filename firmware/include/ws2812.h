/**
 * WS2812 RGB LED Driver for RP2350
 *
 * Bit-banging implementation for single LED on GP22
 */

#ifndef WS2812_H
#define WS2812_H

#include <stdint.h>

// GPIO pin for WS2812 (RP2350-USB board)
#define WS2812_PIN 22

// Color structure
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} ws2812_color_t;

// Predefined colors
#define WS2812_RED (ws2812_color_t){255, 0, 0}
#define WS2812_GREEN (ws2812_color_t){0, 255, 0}
#define WS2812_BLUE (ws2812_color_t){0, 0, 255}
#define WS2812_YELLOW (ws2812_color_t){255, 255, 0}
#define WS2812_CYAN (ws2812_color_t){0, 255, 255}
#define WS2812_MAGENTA (ws2812_color_t){255, 0, 255}
#define WS2812_WHITE (ws2812_color_t){255, 255, 255}
#define WS2812_OFF (ws2812_color_t){0, 0, 0}
#define WS2812_ORANGE (ws2812_color_t){255, 128, 0}
#define WS2812_PURPLE (ws2812_color_t){128, 0, 255}

/**
 * Initialize WS2812 driver
 */
void ws2812_init(void);

/**
 * Set LED color
 */
void ws2812_set_color(ws2812_color_t color);

/**
 * Set LED color by RGB values
 */
void ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * Turn off LED
 */
void ws2812_off(void);

/**
 * Blink LED with color
 */
void ws2812_blink(ws2812_color_t color, int times, int delay_ms);

#endif // WS2812_H
