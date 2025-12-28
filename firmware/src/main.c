/**
 * LibreCrypt Wallet - Main Entry Point
 * 
 * Firmware para hardware wallet baseada em RP2350
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "hardware/flash.h"

#include "librecipher.h"
#include "wallet.h"
#include "usb_protocol.h"

// Versão do firmware
#define FIRMWARE_VERSION_MAJOR 0
#define FIRMWARE_VERSION_MINOR 1
#define FIRMWARE_VERSION_PATCH 0

// LED Status
#define LED_PIN 25

/**
 * Inicialização do hardware
 */
static void hardware_init(void) {
    stdio_init_all();
    
    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

/**
 * Blink LED para indicar status
 */
static void led_blink(int times, int delay_ms) {
    for (int i = 0; i < times; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(delay_ms);
        gpio_put(LED_PIN, 0);
        sleep_ms(delay_ms);
    }
}

/**
 * Loop principal
 */
int main(void) {
    // Inicializar hardware
    hardware_init();
    
    // Inicializar módulos
    librecipher_init();
    wallet_init();
    usb_protocol_init();
    
    // Indicar boot OK
    led_blink(3, 100);
    
    printf("LibreCrypt Wallet v%d.%d.%d\n", 
           FIRMWARE_VERSION_MAJOR, 
           FIRMWARE_VERSION_MINOR, 
           FIRMWARE_VERSION_PATCH);
    
    // Loop principal
    while (true) {
        // Processar comandos USB
        usb_protocol_task();
        
        // Heartbeat LED
        static uint32_t last_blink = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_blink > 1000) {
            gpio_put(LED_PIN, !gpio_get(LED_PIN));
            last_blink = now;
        }
    }
    
    return 0;
}
