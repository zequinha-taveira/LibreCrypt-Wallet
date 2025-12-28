# Arquitetura do Sistema – LibreCrypt Wallet

## Visão Geral

O LibreCrypt Wallet é composto por três camadas principais:

```
┌─────────────────────────────────────────────────────┐
│              App Gerenciador (Tauri v2)             │
│         • UI/UX • Gerenciamento de contas           │
│         • Criação de transações                     │
└─────────────────────▲───────────────────────────────┘
                      │ USB (Protocolo Seguro)
┌─────────────────────┴───────────────────────────────┐
│              Hardware Wallet (RP2350)               │
│  ┌──────────────┬──────────────┬────────────────┐  │
│  │  Bootloader  │ Wallet Engine│  USB Protocol  │  │
│  │   Seguro     │              │     Layer      │  │
│  └──────────────┴──────┬───────┴────────────────┘  │
│                        │                            │
│  ┌─────────────────────┴────────────────────────┐  │
│  │         LibreCipher Crypto Core              │  │
│  │  • KDF • Hash • Assinatura Digital • RNG     │  │
│  └──────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

## Componentes do Hardware

### RP2350 (Raspberry Pi Pico 2)
- **CPU**: Dual-core ARM Cortex-M33 @ 150MHz
- **Flash**: 4MB (mínimo)
- **RAM**: 520KB
- **Recursos de Segurança**: Secure Boot, ARM TrustZone

### Interfaces
| Interface | Uso |
|-----------|-----|
| USB-C | Comunicação com App |
| SPI | Display OLED |
| GPIO | Botões de confirmação |

## Módulos do Firmware

### 1. Bootloader Seguro
- Verificação de assinatura do firmware
- Proteção contra downgrade
- Recovery mode

### 2. LibreCipher Crypto Core
- Primitivos criptográficos (ver `crypto-design.md`)
- Zero alocação dinâmica
- Implementações constant-time

### 3. Wallet Engine
- Derivação de chaves (BIP-32/BIP-44)
- Assinatura de transações
- Gerenciamento de contas

### 4. USB Protocol Layer
- Protocolo binário com framing
- Autenticação de sessão
- Anti-replay

## Fluxo de Dados

```
[App] → USB Request → [RP2350]
                         ↓
                   Validação
                         ↓
                   Processamento
                         ↓
                   Confirmação Física (botão)
                         ↓
                   Assinatura (LibreCipher)
                         ↓
[App] ← USB Response ← [RP2350]
```

## Isolamento de Chaves

> ⚠️ **Princípio Fundamental**: Chaves privadas NUNCA saem do hardware.

- Seed gerada internamente (TRNG)
- Armazenada criptografada na Flash
- Operações de assinatura in-device
