/**
 * @file button.c
 * @brief BOOTSEL Button Driver Implementation
 */

#include "drivers/button.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

// Linker marker for code that must run from RAM (flash is disabled when
// checking BOOTSEL)
// __no_inline_not_in_flash_func is defined in pico/platform.h

void button_init(void) {
#ifdef PIN_BUTTON_CONFIRM
  gpio_init(PIN_BUTTON_CONFIRM);
  gpio_set_dir(PIN_BUTTON_CONFIRM, GPIO_IN);
  gpio_pull_up(PIN_BUTTON_CONFIRM);
#endif
}

// Function to read BOOTSEL button state
// Must run from RAM because it toggles Flash CS pin
bool __no_inline_not_in_flash_func(button_is_pressed)(void) {
  const unsigned int CS_PIN_INDEX = 1;

  // 1. Disable interrupts and other cores to prevent flash access
  // For simplicity in this stubs/single-core proof of concept:
  uint32_t flags = save_and_disable_interrupts();

  // 2. Set CS pin to high impedance input to read button
  hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                  GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                  IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

  // 3. Delay slightly to allow signal to settle
  for (volatile int i = 0; i < 1000; ++i)
    ;

  // 4. Read status (low = pressed, usually?)
  // BOOTSEL pulls CS low when pressed?
  // Actually, on RP2040/RP2350 BOOTSEL is usually connected to CS.
  // Standard logic: The SIO GPIO_HI_IN register reflects the logic level.
  // bit 1 is usually CS.

  // Note: This specific register access depends on RP2350 architecture details.
  // For RP2350 (RISC-V/ARM), the SIO might differ slightly from RP2040.
  // Using a safe approximation for now assuming RP2040-like behavior for the CS
  // pin logic. If running on RP2350, we should check `hardware/structs/qmi.h`
  // for QSPI.

  // FIXME: RP2350 specific implementation required.
  // For now, returning false to avoid crashing if register map differs.
  // In production, use the `pico_bootsel_via_double_reset` library function if
  // available. OR: Assume a distinct GPIO if the board maps it.

  // Stub implementation:
  // Only return true if a specific memory location is set (mocking)
  // or if we decide to implement standard GPIO reading for an external button
  // as backup.

  bool pressed = false;

  // Restore interrupts
  restore_interrupts(flags);

  return pressed;
}
