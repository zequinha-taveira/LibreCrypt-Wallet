# LibreCrypt Master Build Script
# Builds both Firmware and Native App

$ErrorActionPreference = "Stop"

Write-Host "🚀 Starting LibreCrypt Master Build..." -ForegroundColor Cyan

# 1. Environment Setup
Write-Host "`n[1/3] Setting up Environment..." -ForegroundColor Yellow
if (Test-Path ".\setup_environment.ps1") {
    . .\setup_environment.ps1
}
else {
    Write-Error "setup_environment.ps1 not found!"
}

# 2. Build Firmware
Write-Host "`n[2/3] Building Firmware (C/Ninja)..." -ForegroundColor Yellow
Push-Location firmware

if (Test-Path "build") {
    Write-Host "   -> Cleaning previous build..." -ForegroundColor Gray
    Remove-Item "build" -Recurse -Force | Out-Null
}
New-Item -ItemType Directory -Force -Path "build" | Out-Null
Set-Location build

# Configure CMake
Write-Host "   -> Configuring CMake (Target: RP2350)..." -ForegroundColor Gray
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DPICO_BOARD=pico2 -DPICO_PLATFORM=rp2350 .. 
if ($LASTEXITCODE -ne 0) { Write-Error "CMake configuration failed" }

# Build
Write-Host "   -> Compiling..." -ForegroundColor Gray
ninja
if ($LASTEXITCODE -ne 0) { Write-Error "Firmware compilation failed" }

# Check/Convert UF2
if (-not (Test-Path "librecrypt_wallet.uf2")) {
    Write-Host "   -> UF2 not found, attempting conversion..." -ForegroundColor Yellow
    if (Get-Command picotool -ErrorAction SilentlyContinue) {
        picotool uf2 convert librecrypt_wallet.elf librecrypt_wallet.uf2
    }
    else {
        Write-Warning "Picotool not in PATH. Cannot generate UF2 automatically."
    }
}

if (Test-Path "librecrypt_wallet.uf2") {
    Write-Host "✅ Firmware Built: $(Get-Item librecrypt_wallet.uf2).FullName" -ForegroundColor Green
}

Pop-Location

# 3. Build Tauri App
Write-Host "`n[3/3] Building Tauri App (Rust/JS)..." -ForegroundColor Yellow
Push-Location app

if (-not (Test-Path "node_modules")) {
    Write-Host "   -> Intalling NPM dependencies..." -ForegroundColor Gray
    cmd /c "npm install"
}

Write-Host "   -> Building Native App (Requires Internet for first run)..." -ForegroundColor Gray
cmd /c "npm run tauri build"
if ($LASTEXITCODE -ne 0) { Write-Error "App build failed" }

Pop-Location

Write-Host "`n🎉 BUILD COMPLETE!" -ForegroundColor Green
Write-Host "1. Firmware: firmware/build/librecrypt_wallet.uf2"
Write-Host "2. App Installer: app/src-tauri/target/release/bundle/msi/..."
