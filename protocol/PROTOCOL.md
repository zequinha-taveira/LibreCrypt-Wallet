# LibreCrypt USB Protocol v1.0

## Overview

The LibreCrypt Wallet communicates with host applications via USB HID (Human Interface Device).
This protocol defines the message format and available commands.

## Why USB HID?

- **No drivers required**: Works on all major operating systems
- **Secure**: No COM port exposure, limited attack surface
- **Low latency**: Optimized for interactive operations
- **Portable**: Same protocol works on desktop, mobile (OTG), and web (WebHID)

## Packet Format

All packets are 64 bytes (standard HID report size):

```
┌─────────┬─────────┬─────────────────────┬─────────┐
│ CMD (1) │ LEN (2) │ PAYLOAD (0-59)      │ CRC (2) │
└─────────┴─────────┴─────────────────────┴─────────┘
```

| Field   | Size  | Description |
|---------|-------|-------------|
| CMD     | 1     | Command code |
| LEN     | 2     | Payload length (little-endian) |
| PAYLOAD | 0-59  | Command-specific data |
| CRC     | 2     | CRC16-CCITT checksum |

## Commands

### Standard Commands (0x00-0x3F)

| Code | Name | Description |
|------|------|-------------|
| 0x01 | PING | Health check, returns "PONG" |
| 0x10 | GET_PUBKEY | Get public key for derivation path |
| 0x11 | SIGN_TX | Sign transaction hash (requires button) |
| 0x12 | VERIFY_ADDRESS | Verify address on device display |
| 0x20 | GET_DEVICE_INFO | Get firmware version and capabilities |
| 0x30 | INIT_DEVICE | First-time device setup |

### Experimental Commands (0xE0-0xEF)

⚠️ **Warning**: These use experimental post-quantum cryptography.

| Code | Name | Description |
|------|------|-------------|
| 0xE0 | PQ_KEYGEN | Generate Kyber/Dilithium keypair |
| 0xE1 | PQ_SIGN | Sign with Dilithium |
| 0xE2 | PQ_VERIFY | Verify Dilithium signature |

## Response Status Codes

| Code | Name | Description |
|------|------|-------------|
| 0x00 | OK | Success |
| 0x01 | ERROR | Generic error |
| 0x02 | BUSY | Device busy |
| 0x03 | NEED_CONFIRM | Waiting for button press |
| 0x04 | TIMEOUT | Operation timed out |
| 0x05 | INVALID_CMD | Unknown command |
| 0x06 | NOT_INIT | Device not initialized |
| 0x07 | LOCKED | Device locked |

## Command Details

### GET_PUBKEY (0x10)

Request the public key for a BIP-32 derivation path.

**Request:**
```
CMD: 0x10
LEN: variable
PAYLOAD: UTF-8 path string (e.g., "m/44'/0'/0'/0/0")
```

**Response:**
```
STATUS: 0x00 (OK)
PAYLOAD: 33 bytes compressed public key
```

### SIGN_TX (0x11)

Sign a transaction hash. **Requires physical button confirmation.**

**Request:**
```
CMD: 0x11
LEN: 32 + path_len
PAYLOAD: 
  - hash[32]: Transaction hash (double SHA-256)
  - path[]: UTF-8 derivation path
```

**Response (after button press):**
```
STATUS: 0x00 (OK)
PAYLOAD: 64 bytes signature (r || s)
```

### GET_DEVICE_INFO (0x20)

Get device information and capabilities.

**Request:**
```
CMD: 0x20
LEN: 0
```

**Response:**
```
STATUS: 0x00 (OK)
PAYLOAD:
  - version_major[1]
  - version_minor[1]
  - version_patch[1]
  - device_state[1]
  - capabilities[1] (bitmask)
  - device_id[8] (unique chip ID)
```

**Capabilities Bitmask:**
- Bit 0: BIP-32 HD Wallet
- Bit 1: BIP-39 Mnemonic
- Bit 2: secp256k1 ECDSA
- Bit 3: SHA-256 HW Acceleration
- Bit 4: Post-Quantum (Experimental)
- Bit 5: TrustZone Enabled

## Security Considerations

1. **Button Confirmation**: All signing operations require physical button press
2. **No Key Export**: Private keys never leave the device
3. **Rate Limiting**: Failed attempts trigger exponential backoff
4. **Anti-Replay**: Transaction hashes include nonce to prevent replay
