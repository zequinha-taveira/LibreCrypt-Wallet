/**
 * @file ws2812.c
 * @brief WS2812 RGB LED Driver using PIO
 */

#include "drivers/ws2812.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

// PIO program for WS2812 (simplified - actual timing from pico-examples)
// WS2812 requires 800kHz data rate with specific T0H, T1H, T0L, T1L timings

static uint8_t ws2812_pin = 22; // Default
static bool initialized = false;

bool ws2812_init(uint8_t pin) {
  ws2812_pin = pin;
  // Initialize GPIO
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
  gpio_put(pin, 0);

  // TODO: Load actual PIO program from pico-examples/pio/ws2812
  // For now, use simple bit-banging fallback

  initialized = true;
  return true;
}

// Bit-bang fallback (less timing accurate but works for single LED)
static void ws2812_send_bit(uint8_t pin, bool bit) {
  if (bit) {
    // T1H: ~800ns high, T1L: ~450ns low
    gpio_put(pin, 1);
    __asm volatile("nop; nop; nop; nop; nop; nop; nop; nop;");
    __asm volatile("nop; nop; nop; nop; nop; nop; nop; nop;");
    gpio_put(pin, 0);
    __asm volatile("nop; nop; nop; nop;");
  } else {
    // T0H: ~400ns high, T0L: ~850ns low
    gpio_put(pin, 1);
    __asm volatile("nop; nop; nop; nop;");
    gpio_put(pin, 0);
    __asm volatile("nop; nop; nop; nop; nop; nop; nop; nop;");
  }
}

static void ws2812_send_byte(uint8_t pin, uint8_t byte) {
  for (int i = 7; i >= 0; i--) {
    ws2812_send_bit(pin, (byte >> i) & 1);
  }
}

void ws2812_set_color(uint32_t grb) {
  if (!initialized)
    return;

  uint8_t pin = ws2812_pin;

  // Disable interrupts for timing-critical section
  uint32_t irq = save_and_disable_interrupts();

  // Send GRB data (WS2812 order)
  ws2812_send_byte(pin, (grb >> 16) & 0xFF); // G
  ws2812_send_byte(pin, (grb >> 8) & 0xFF);  // R
  ws2812_send_byte(pin, grb & 0xFF);         // B

  restore_interrupts(irq);

  // Reset: >50us low
  sleep_us(60);
}

void ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
  ws2812_set_color(grb);
}

void ws2812_off(void) { ws2812_set_color(0x000000); }

void ws2812_blink(uint32_t grb, uint16_t on_ms, uint16_t off_ms,
                  uint8_t count) {
  uint8_t iterations = (count == 0) ? 255 : count;

  for (uint8_t i = 0; i < iterations; i++) {
    ws2812_set_color(grb);
    sleep_ms(on_ms);
    ws2812_off();
    sleep_ms(off_ms);

    if (count == 0)
      i = 0; // Infinite loop
  }
}

void ws2812_pulse(uint32_t grb, uint16_t period_ms) {
  // Extract base colors
  uint8_t g = (grb >> 16) & 0xFF;
  uint8_t r = (grb >> 8) & 0xFF;
  uint8_t b = grb & 0xFF;

  uint16_t step_ms = period_ms / 512;
  if (step_ms < 1)
    step_ms = 1;

  // Fade in
  for (int brightness = 0; brightness < 256; brightness++) {
    uint8_t g_scaled = (g * brightness) / 255;
    uint8_t r_scaled = (r * brightness) / 255;
    uint8_t b_scaled = (b * brightness) / 255;
    ws2812_set_rgb(r_scaled, g_scaled, b_scaled);
    sleep_ms(step_ms);
  }

  // Fade out
  for (int brightness = 255; brightness >= 0; brightness--) {
    uint8_t g_scaled = (g * brightness) / 255;
    uint8_t r_scaled = (r * brightness) / 255;
    uint8_t b_scaled = (b * brightness) / 255;
    ws2812_set_rgb(r_scaled, g_scaled, b_scaled);
    sleep_ms(step_ms);
  }
}
