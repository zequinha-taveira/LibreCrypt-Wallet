/**
 * @file board_config.h
 * @brief Tenstar RP2350-USB Board Configuration
 *
 * Pin assignments based on the board's hardware layout.
 */

#ifndef LIBRECRYPT_BOARD_CONFIG_H
#define LIBRECRYPT_BOARD_CONFIG_H

// =============================================================================
// Board: Tenstar RP2350-USB 16MB
// =============================================================================

// Flash chip: W25Q128JVPQ (16MB)
#define BOARD_FLASH_SIZE (16 * 1024 * 1024)

// Dimensions: 17.8mm x 25.4mm (12.7mm USB plug)

// =============================================================================
// GPIO Assignments
// =============================================================================

// WS2812 RGB LED (built-in on board)
#define PIN_LED_RGB 22
#define LED_IS_WS2812 1

// Confirm button (external, active low with pull-up)
#define PIN_BUTTON_CONFIRM 0
#define BUTTON_ACTIVE_LOW 1

// Debug UART (optional)
#define PIN_UART_TX 4
#define PIN_UART_RX 5

// I2C for optional OLED display
#define PIN_I2C_SDA 6
#define PIN_I2C_SCL 7
#define I2C_INSTANCE i2c1

// =============================================================================
// WS2812 LED Colors
// =============================================================================

// GRB format (WS2812 uses GRB, not RGB)
#define LED_COLOR_OFF 0x000000
#define LED_COLOR_BLUE 0x0000FF   // Idle/Locked
#define LED_COLOR_GREEN 0x00FF00  // Ready/Unlocked
#define LED_COLOR_YELLOW 0xFFFF00 // Waiting confirmation
#define LED_COLOR_RED 0xFF0000    // Error
#define LED_COLOR_PURPLE 0xFF00FF // Experimental mode
#define LED_COLOR_WHITE 0xFFFFFF  // Processing

// =============================================================================
// Timing Constants
// =============================================================================

#define BUTTON_DEBOUNCE_MS 50
#define BUTTON_LONG_PRESS_MS 3000
#define LED_BLINK_SLOW_MS 1000
#define LED_BLINK_FAST_MS 200
#define AUTO_LOCK_TIMEOUT_MS (5 * 60 * 1000) // 5 minutes

// =============================================================================
// Security Settings
// =============================================================================

#define MAX_PIN_ATTEMPTS 3
#define PIN_LOCKOUT_MS (30 * 1000) // 30 seconds after failed attempts

#endif // LIBRECRYPT_BOARD_CONFIG_H
