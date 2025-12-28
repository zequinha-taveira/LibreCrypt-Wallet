# LibreCrypt Wallet - Firmware Build Guide

## Pré-requisitos

### 1. Instalar Pico SDK 2.0

```powershell
# Clonar Pico SDK
cd C:\
git clone https://github.com/raspberrypi/pico-sdk.git --branch 2.0.0
cd pico-sdk
git submodule update --init

# Configurar variável de ambiente
[Environment]::SetEnvironmentVariable("PICO_SDK_PATH", "C:\pico-sdk", "User")
```

### 2. Instalar Toolchain ARM

Baixar e instalar: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

Versão recomendada: `arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-arm-none-eabi`

### 3. Instalar CMake e Ninja

```powershell
winget install Kitware.CMake
winget install Ninja-build.Ninja
```

## Build do Firmware

```powershell
cd c:\LibreCrypt-Wallet\firmware

# Criar diretório de build
mkdir build
cd build

# Configurar CMake
cmake -G Ninja ..

# Compilar
ninja
```

### Saída

- `librecrypt_wallet.uf2` - Arquivo para flash
- `librecrypt_wallet.elf` - Debug

## Flash no RP2350-USB

1. Segure o botão **BOOT** na placa
2. Conecte o cabo USB
3. Solte o botão BOOT
4. A placa aparecerá como drive **RPI-RP2**
5. Copie `librecrypt_wallet.uf2` para o drive

## Verificação

Após o flash, o LED WS2812 deve:
- Mostrar animação rainbow no boot
- Piscar verde 3x
- Ficar laranja (wallet não inicializada)

## Troubleshooting

### "PICO_SDK_PATH not set"

```powershell
$env:PICO_SDK_PATH = "C:\pico-sdk"
```

### "Could not find toolchain"

Adicione o toolchain ARM ao PATH:
```powershell
$env:PATH += ";C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\13.2 Rel1\bin"
```
