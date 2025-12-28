/**
 * LibreCrypt Wallet - Main Entry Point
 *
 * Firmware para hardware wallet RP2350-USB
 * LED WS2812 em GP22
 */

#include "hardware/flash.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include <stdio.h>


#include "librecipher.h"
#include "usb_protocol.h"
#include "wallet.h"
#include "ws2812.h"


// Versão do firmware
#define FIRMWARE_VERSION_MAJOR 0
#define FIRMWARE_VERSION_MINOR 1
#define FIRMWARE_VERSION_PATCH 0

/**
 * Inicialização do hardware
 */
static void hardware_init(void) {
  stdio_init_all();

  // Inicializar LED WS2812
  ws2812_init();
}

/**
 * Mostra status da wallet no LED
 */
static void update_led_status(void) {
  switch (wallet_get_status()) {
  case WALLET_STATUS_UNINITIALIZED:
    ws2812_set_rgb(255, 128, 0); // Laranja
    break;
  case WALLET_STATUS_LOCKED:
    ws2812_set_rgb(255, 0, 0); // Vermelho
    break;
  case WALLET_STATUS_UNLOCKED:
    ws2812_set_rgb(0, 255, 0); // Verde
    break;
  }
}

/**
 * Boot sequence
 */
static void boot_sequence(void) {
  // Rainbow boot animation
  ws2812_set_rgb(255, 0, 0); // Vermelho
  sleep_ms(100);
  ws2812_set_rgb(255, 128, 0); // Laranja
  sleep_ms(100);
  ws2812_set_rgb(255, 255, 0); // Amarelo
  sleep_ms(100);
  ws2812_set_rgb(0, 255, 0); // Verde
  sleep_ms(100);
  ws2812_set_rgb(0, 255, 255); // Ciano
  sleep_ms(100);
  ws2812_set_rgb(0, 0, 255); // Azul
  sleep_ms(100);
  ws2812_set_rgb(128, 0, 255); // Roxo
  sleep_ms(100);
  ws2812_off();
  sleep_ms(100);

  // Flash verde para indicar boot OK
  for (int i = 0; i < 3; i++) {
    ws2812_set_rgb(0, 255, 0);
    sleep_ms(100);
    ws2812_off();
    sleep_ms(100);
  }
}

/**
 * Loop principal
 */
int main(void) {
  // Inicializar hardware
  hardware_init();

  // Boot sequence
  boot_sequence();

  // Inicializar módulos
  librecipher_init();
  wallet_init();
  usb_protocol_init();

  printf("\n");
  printf("=================================\n");
  printf(" LibreCrypt Wallet v%d.%d.%d\n", FIRMWARE_VERSION_MAJOR,
         FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH);
  printf(" Hardware: RP2350-USB\n");
  printf(" Crypto: LibreCipher\n");
  printf("=================================\n");

  // Atualizar LED para status inicial
  update_led_status();

  // Variáveis para heartbeat
  uint32_t last_heartbeat = 0;
  bool led_pulse_up = true;
  uint8_t pulse_brightness = 0;

  // Loop principal
  while (true) {
    // Processar comandos USB
    usb_protocol_task();

    // Heartbeat LED (breathing effect)
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (now - last_heartbeat > 20) { // 50Hz update
      last_heartbeat = now;

      // Breathing effect baseado no status
      if (led_pulse_up) {
        pulse_brightness += 5;
        if (pulse_brightness >= 100)
          led_pulse_up = false;
      } else {
        pulse_brightness -= 5;
        if (pulse_brightness <= 20)
          led_pulse_up = true;
      }

      // Cor baseada no status
      switch (wallet_get_status()) {
      case WALLET_STATUS_UNINITIALIZED:
        ws2812_set_rgb(pulse_brightness * 2, pulse_brightness, 0);
        break;
      case WALLET_STATUS_LOCKED:
        ws2812_set_rgb(pulse_brightness * 2, 0, 0);
        break;
      case WALLET_STATUS_UNLOCKED:
        ws2812_set_rgb(0, pulse_brightness * 2, 0);
        break;
      }
    }
  }

  return 0;
}
