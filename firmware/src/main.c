/**
 * @file main.c
 * @brief LibreCrypt Wallet - Main Entry Point
 * 
 * Hardware cryptocurrency wallet firmware for RP2350.
 * 
 * Security Model:
 * - All private keys stored in OTP memory (write-once)
 * - Cryptographic operations in TrustZone Secure World
 * - USB HID for host communication (no serial exposure)
 * 
 * @copyright 2026 LibreCrypt Project - Open Source
 * @license MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "hardware/flash.h"
#include "tusb.h"

// LibreCrypt modules
#include "crypto/bip32.h"
#include "crypto/bip39.h"
#include "usb/hid_protocol.h"
#include "storage/otp_manager.h"

// =============================================================================
// Constants
// =============================================================================

#define LIBRECRYPT_VERSION_MAJOR 0
#define LIBRECRYPT_VERSION_MINOR 1
#define LIBRECRYPT_VERSION_PATCH 0

#define LED_PIN 25  // Onboard LED (if available)

// =============================================================================
// Device State
// =============================================================================

typedef enum {
    DEVICE_STATE_UNINITIALIZED = 0,
    DEVICE_STATE_LOCKED,
    DEVICE_STATE_UNLOCKED,
    DEVICE_STATE_ERROR
} device_state_t;

static volatile device_state_t g_device_state = DEVICE_STATE_UNINITIALIZED;

// =============================================================================
// Initialization
// =============================================================================

static void init_gpio(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

static void blink_status(uint8_t count, uint32_t delay_ms) {
    for (uint8_t i = 0; i < count; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(delay_ms);
        gpio_put(LED_PIN, 0);
        sleep_ms(delay_ms);
    }
}

static bool check_device_initialized(void) {
    // Check if OTP has been programmed with a master seed
    return otp_manager_is_initialized();
}

// =============================================================================
// Main Loop
// =============================================================================

int main(void) {
    // Initialize hardware
    stdio_init_all();
    init_gpio();
    
    // Initialize TinyUSB
    tusb_init();
    
    // Check device state
    if (check_device_initialized()) {
        g_device_state = DEVICE_STATE_LOCKED;
        blink_status(2, 200);  // 2 blinks = initialized but locked
    } else {
        g_device_state = DEVICE_STATE_UNINITIALIZED;
        blink_status(5, 100);  // 5 fast blinks = needs setup
    }
    
    // Main loop
    while (1) {
        // Process USB tasks
        tud_task();
        
        // Process HID commands from host
        if (tud_hid_ready()) {
            hid_protocol_process();
        }
        
        // Heartbeat LED (slow blink when idle)
        static uint32_t last_blink = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_blink > 2000) {
            gpio_put(LED_PIN, 1);
            sleep_ms(50);
            gpio_put(LED_PIN, 0);
            last_blink = now;
        }
    }
    
    return 0;
}

// =============================================================================
// USB HID Callbacks (TinyUSB)
// =============================================================================

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, 
                                hid_report_type_t report_type, 
                                uint8_t* buffer, uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

// Invoked when received SET_REPORT control request or data on OUT endpoint
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    
    // Route to HID protocol handler
    hid_protocol_handle_report(buffer, bufsize);
}
