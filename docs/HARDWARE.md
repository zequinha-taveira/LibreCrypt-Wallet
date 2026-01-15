# Tenstar RP2350-USB Hardware Reference

## Board Specifications

| Parameter | Value |
|-----------|-------|
| MCU | RP2350A (Dual ARM Cortex-M33 + RISC-V Hazard3) |
| Clock | Up to 150MHz |
| SRAM | 520KB |
| Flash | 16MB (W25Q128JVPQ) |
| USB | USB-A plug (built-in) |
| Dimensions | 17.8mm x 25.4mm (12.7mm USB plug) |

## Pin Diagram

![Pin Diagram](file:///C:/Users/zequi/.gemini/antigravity/brain/32b60894-2f5e-435c-9ad9-825b111464e9/uploaded_image_1_1768502079112.png)

## LibreCrypt Pin Assignments

| Function | GPIO | Notes |
|----------|------|-------|
| **Status LED (RGB)** | GP22 | WS2812 addressable LED |
| **Confirm Button** | GP0 | External button (pull-up) |
| **UART TX (Debug)** | GP4 | Optional debug output |
| **UART RX (Debug)** | GP5 | Optional debug input |
| **I2C SDA (Display)** | GP6 | For OLED display (optional) |
| **I2C SCL (Display)** | GP7 | For OLED display (optional) |

## WS2812 RGB LED

The board has a built-in WS2812 addressable RGB LED on **GP22**.

| Color | Meaning |
|-------|---------|
| 🔵 Blue slow blink | Idle, locked |
| 🟢 Green | Ready, unlocked |
| 🟡 Yellow pulse | Waiting for confirmation |
| 🔴 Red | Error / Not initialized |
| 🟣 Purple | Experimental mode active |

## Power

- USB powered (5V from host)
- 3.3V regulator onboard
- Low power consumption (~30mA typical)

## Boot Modes

1. **Normal Boot**: Run main firmware
2. **BOOTSEL Mode**: Hold BOOT button while connecting USB
3. **Debug Mode**: UART on GP4/GP5 (if enabled in firmware)

## Secure Features

| Feature | RP2350 Support |
|---------|---------------|
| Secure Boot | ✅ Yes |
| OTP Memory | ✅ 8KB |
| ARM TrustZone | ✅ Yes |
| SHA-256 HW | ✅ Yes |
| TRNG | ✅ Yes |
| Glitch Detection | ✅ Yes |

## References

- [RP2350 Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)
- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
