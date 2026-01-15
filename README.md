# LibreCrypt Wallet

> 🔐 Open-source hardware cryptocurrency wallet based on RP2350

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Status](https://img.shields.io/badge/status-development-orange.svg)
![Hardware](https://img.shields.io/badge/hardware-RP2350-green.svg)

## Overview

LibreCrypt Wallet is a professional-grade hardware cryptocurrency wallet that prioritizes:

- **Open Cryptography**: All code is open source and auditable
- **Post-Quantum Ready**: Experimental support for Kyber/Dilithium algorithms
- **Multi-Platform**: Apps for Android, iOS, Desktop, and Web
- **Affordable Hardware**: Built on the $7 RP2350 microcontroller

## Features

| Feature | Status |
|---------|--------|
| BIP-32/39/44 HD Wallet | 🚧 In Progress |
| Bitcoin Support | 🚧 In Progress |
| USB HID Communication | 🚧 In Progress |
| Hardware SHA-256 | 🚧 In Progress |
| TrustZone Isolation | 🚧 In Progress |
| SecureBoot | 📋 Planned |
| Kyber/Dilithium (PQ) | 📋 Experimental |
| Desktop App | 📋 Planned |
| Mobile Apps | 📋 Planned |

## Project Structure

```
LibreCrypt-Wallet/
├── firmware/           # RP2350 firmware (C)
│   ├── src/
│   │   ├── crypto/     # BIP-32/39, secp256k1, SHA-256
│   │   ├── storage/    # OTP and flash management
│   │   ├── usb/        # HID protocol
│   │   └── trustzone/  # Secure World code
│   └── include/
├── app/                # Host applications
│   ├── core/           # Shared Rust library
│   ├── desktop/        # Tauri app
│   ├── android/        # Kotlin
│   ├── ios/            # Swift
│   └── web/            # React + WebHID
├── protocol/           # Communication spec
├── docs/               # Documentation
└── tools/              # Development utilities
```

## Hardware

**Recommended Board**: Tenstar RP2350-USB 16MB

- Dual-core ARM Cortex-M33 @ 150MHz
- 520KB SRAM, 16MB Flash
- Hardware SHA-256 accelerator
- Hardware TRNG
- ARM TrustZone support
- 8KB OTP for key storage
- USB-A form factor (plug directly into port)

**Cost**: ~$7 USD

## Building

### Prerequisites

- [Pico SDK 2.0+](https://github.com/raspberrypi/pico-sdk)
- ARM GCC toolchain
- CMake 3.13+

### Firmware

```bash
cd firmware
mkdir build && cd build
cmake ..
make
```

Flash `librecrypt_wallet.uf2` to the device via USB bootloader.

## Security Model

1. **Master Seed**: Generated via hardware TRNG, stored in OTP (write-once)
2. **Key Derivation**: BIP-32/44 paths derived in TrustZone Secure World
3. **Signing**: Transaction hashes signed internally, private keys never exported
4. **Confirmation**: All sensitive operations require physical button press

## ⚠️ Experimental Features

This project includes experimental post-quantum cryptography:

- **Kyber (ML-KEM)**: Key encapsulation
- **Dilithium (ML-DSA)**: Digital signatures

These are marked clearly in the UI and require explicit user confirmation.

**DO NOT** use experimental features for real funds until audited.

## Contributing

Contributions welcome! Please read [CONTRIBUTING.md](docs/CONTRIBUTING.md) first.

## License

MIT License - See [LICENSE](LICENSE) for details.

## Disclaimer

This is experimental software. Use at your own risk. The authors are not responsible for any loss of funds.
