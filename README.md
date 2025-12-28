# LibreCrypt Wallet

🔐 **Carteira digital de criptomoedas em hardware (RP2350) com App Gerenciador Multiplataforma e criptografia aberta**

---

## 📌 Visão Geral

O **LibreCrypt Wallet** é um projeto **open-source e auditável** de carteira digital de criptomoedas, desenvolvido para **projetos sérios**, com foco em **segurança**, **liberdade tecnológica** e **criptografia aberta**.

O sistema é composto por:

* 🧩 **Hardware Wallet** baseada no microcontrolador **RP2350**
* 🔐 **Firmware seguro** com isolamento total de chaves privadas
* 🖥️ **App Gerenciador Multiplataforma**
* 🧠 **Novo algoritmo criptográfico aberto**, otimizado para sistemas embarcados

---

## 🎯 Objetivos do Projeto

* Garantir que **chaves privadas nunca saiam do hardware**
* Permitir **assinatura de transações offline**
* Desenvolver **criptografia própria, aberta e documentada**
* Facilitar **auditoria de segurança e revisão comunitária**
* Oferecer um **ecossistema multiplataforma moderno**

---

## 🧱 Arquitetura do Sistema

```
┌─────────────────────────┐
│      App Gerenciador    │
│  (Desktop / Mobile)     │
└───────────▲─────────────┘
            │ USB (Secure Protocol)
┌───────────┴─────────────┐
│   LibreCrypt Wallet     │
│   Hardware (RP2350)     │
│                         │
│ • Geração de chaves     │
│ • Assinatura digital    │
│ • Crypto Core           │
└─────────────────────────┘
```

---

## 🔐 Criptografia - LibreCipher

O LibreCrypt Wallet utiliza **LibreCipher**, nossa suite criptográfica própria:

* 🔓 **Código aberto e documentado**
* 🧠 **LibreCipher-KDF** - Derivação de chaves
* 🛡️ **LibreCipher-Hash** - SHA-256 constant-time
* ⚙️ **AES-256-GCM** - Criptografia simétrica
* ⏱️ Implementações **constant-time**
* 🛡️ Proteção contra ataques de canal lateral

> ⚠️ Algoritmos próprios coexistem com **padrões consolidados** (Ed25519) para validação.

---

## 🖥️ App Gerenciador (Tauri v2 + Rust)

Plataformas suportadas:

* Windows
* Linux
* macOS
* Android (futuro)
* iOS (futuro)

Funções principais:

* Gerenciamento de contas e endereços
* Criação e envio de transações
* Comunicação segura com o hardware
* Atualização segura de firmware

---

## 🚀 Quick Start

### Pré-requisitos

* [Rust](https://rustup.rs/)
* [Node.js](https://nodejs.org/) (v18+)
* [Pico SDK](https://github.com/raspberrypi/pico-sdk) (para firmware)

### App (Tauri v2)

```bash
cd app
npm install
npm run tauri dev
```

Ou para criar um novo projeto Tauri:
```bash
npm create tauri-app@latest
```

### Firmware

```bash
cd firmware
mkdir build && cd build
cmake -G Ninja ..
ninja
```

O arquivo `librecrypt_wallet.uf2` será gerado em `build/`.

---

## 📂 Estrutura do Repositório

```
librecrypt-wallet/
├── firmware/          # Firmware RP2350
│   ├── bootloader/
│   ├── crypto/
│   └── wallet/
├── app/               # App Multiplataforma
│   ├── desktop/
│   ├── mobile/
│   └── shared/
├── hardware/          # Schematics e PCB
├── docs/              # Documentação técnica
│   ├── architecture.md
│   ├── threat-model.md
│   └── crypto-design.md
├── LICENSE
└── README.md
```

---

## 🛡️ Modelo de Ameaças (Threat Model)

O projeto considera ameaças como:

* Malware no computador host
* Ataques físicos ao dispositivo
* Firmware malicioso
* Ataques de canal lateral

As principais mitigações incluem:

* Confirmação física no hardware
* Secure Boot
* Firmware assinado
* Chaves isoladas em hardware

---

## 📜 Licenciamento

Licenciamento planejado:

* **Firmware / Criptografia**: Apache 2.0 ou GPLv3
* **App**: MIT or Apache 2.0
* **Hardware**: CERN Open Hardware License (CERN-OHL)

---

## 🤝 Contribuições

Contribuições são bem-vindas.

Antes de contribuir:

* Leia a documentação em `/docs`
* Siga os padrões de código
* Priorize segurança e legibilidade

Um arquivo `CONTRIBUTING.md` será adicionado.

---

## ⚠️ Aviso Importante

Este projeto **não deve ser utilizado em produção** enquanto estiver em fase experimental.

A criptografia própria está em **desenvolvimento e avaliação acadêmica**.

---

## 🌐 Filosofia do Projeto

> **Sem código fechado. Sem chaves escondidas. Sem confiança cega.**

LibreCrypt Wallet defende **segurança verificável**, **liberdade tecnológica** e **transparência total**.

---

## 📬 Contato

Projeto mantido pela comunidade.

> Mais informações e documentação serão adicionadas em breve.

---

**LibreCrypt Wallet** – Segurança, Liberdade e Criptografia Aberta.
