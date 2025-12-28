/**
 * WS2812 RGB LED Driver for RP2350
 *
 * Bit-banging implementation optimized for 150MHz clock
 * Uses inline assembly for precise timing
 */

#include "ws2812.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

// Timing for WS2812 at 150MHz
// T0H = 0.4us = 60 cycles
// T0L = 0.85us = 127 cycles
// T1H = 0.8us = 120 cycles
// T1L = 0.45us = 67 cycles

#define T0H_CYCLES 60
#define T0L_CYCLES 127
#define T1H_CYCLES 120
#define T1L_CYCLES 67

// Inline delay
static inline void delay_cycles(uint32_t cycles) {
  // Each iteration is approximately 3 cycles
  for (uint32_t i = 0; i < cycles / 3; i++) {
    __asm__ volatile("nop");
  }
}

void ws2812_init(void) {
  gpio_init(WS2812_PIN);
  gpio_set_dir(WS2812_PIN, GPIO_OUT);
  gpio_put(WS2812_PIN, 0);

  // Initial off state
  sleep_ms(1);
}

static void send_bit(bool bit) {
  if (bit) {
    // Send 1: high for T1H, low for T1L
    gpio_put(WS2812_PIN, 1);
    delay_cycles(T1H_CYCLES);
    gpio_put(WS2812_PIN, 0);
    delay_cycles(T1L_CYCLES);
  } else {
    // Send 0: high for T0H, low for T0L
    gpio_put(WS2812_PIN, 1);
    delay_cycles(T0H_CYCLES);
    gpio_put(WS2812_PIN, 0);
    delay_cycles(T0L_CYCLES);
  }
}

static void send_byte(uint8_t byte) {
  for (int i = 7; i >= 0; i--) {
    send_bit((byte >> i) & 1);
  }
}

void ws2812_set_color(ws2812_color_t color) {
  // WS2812 expects GRB order
  uint32_t irq_status = save_and_disable_interrupts();

  send_byte(color.g);
  send_byte(color.r);
  send_byte(color.b);

  restore_interrupts(irq_status);

  // Reset pulse (>50us low)
  sleep_us(60);
}

void ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b) {
  ws2812_color_t color = {r, g, b};
  ws2812_set_color(color);
}

void ws2812_off(void) { ws2812_set_color(WS2812_OFF); }

void ws2812_blink(ws2812_color_t color, int times, int delay_ms) {
  for (int i = 0; i < times; i++) {
    ws2812_set_color(color);
    sleep_ms(delay_ms);
    ws2812_off();
    sleep_ms(delay_ms);
  }
}

// Status indication patterns
void ws2812_status_boot(void) { ws2812_blink(WS2812_GREEN, 3, 100); }

void ws2812_status_locked(void) { ws2812_set_color(WS2812_RED); }

void ws2812_status_unlocked(void) { ws2812_set_color(WS2812_GREEN); }

void ws2812_status_processing(void) { ws2812_set_color(WS2812_BLUE); }

void ws2812_status_error(void) { ws2812_blink(WS2812_RED, 5, 50); }

void ws2812_status_confirm(void) { ws2812_blink(WS2812_YELLOW, 1, 500); }
