# LibreCrypt Wallet - Security Model

## Overview

This document describes the security architecture of the LibreCrypt hardware wallet.

## Threat Model

### Assets to Protect

1. **Master Seed (64 bytes)**: The root of all derived keys
2. **Derived Private Keys**: Used for transaction signing
3. **Transaction Integrity**: Ensure signed transactions match user intent

### Threats

| Threat | Mitigation |
|--------|------------|
| Malware on host PC | All signing happens on hardware; host never sees private keys |
| Physical theft of device | PIN protection, OTP cannot be read after locking |
| Firmware tampering | Secure Boot with signature verification |
| Supply chain attack | Open source + reproducible builds |
| Side-channel attacks | TrustZone isolation, constant-time crypto |
| Glitching/fault injection | RP2350 glitch detection hardware |
| Quantum computer (future) | Experimental Kyber/Dilithium support |

## Security Layers

### Layer 1: Hardware (RP2350)

- **OTP Memory**: Master seed stored in write-once memory
- **Secure Boot**: Firmware verified before execution
- **TRNG**: True random number generator for key generation
- **Glitch Detection**: Hardware protection against fault injection

### Layer 2: TrustZone

- **Secure World**: Handles all cryptographic operations
- **Non-Secure World**: Handles USB communication, UI
- **NSC Gateway**: Controlled interface between worlds

### Layer 3: Protocol

- **USB HID**: No serial port exposure
- **CRC Checksums**: Packet integrity verification
- **Button Confirmation**: Physical interaction required for signing

### Layer 4: Application

- **PIN Protection**: Device locks after failed attempts
- **Timeout**: Auto-lock after inactivity
- **Experimental Warnings**: Clear UI for non-standard features

## Key Storage

```
┌─────────────────────────────────────────┐
│              OTP Memory (8KB)           │
├─────────────────────────────────────────┤
│ Page 0: Config flags                    │ ← Indicates initialized state
│ Page 1-2: Master seed (64B) + HMAC      │ ← LOCKED after init
│ Page 3: Boot key fingerprint            │ ← Secure boot verification
│ Page 4-63: Reserved                     │
└─────────────────────────────────────────┘
```

## Audit Checklist

- [ ] All crypto implementations reviewed
- [ ] No secret-dependent branches (constant-time)
- [ ] Memory cleared after use (memset_s or volatile)
- [ ] No debug interfaces in production
- [ ] Secure Boot enabled
- [ ] OTP properly locked

## References

- [RP2350 Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)
- [BIP-32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki)
- [BIP-39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki)
- [NIST Post-Quantum Standards](https://csrc.nist.gov/projects/post-quantum-cryptography)
