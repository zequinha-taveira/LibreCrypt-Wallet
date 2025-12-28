# LibreCrypt Environment Setup Script
# Checks and sets up environment variables for development

Write-Host "🔐 Configuring LibreCrypt Wallet Environment..." -ForegroundColor Cyan

# 1. PICO SDK
$PicoSdkPath = "C:\pico-sdk"
if (Test-Path $PicoSdkPath) {
    Write-Host "✅ Pico SDK found at $PicoSdkPath" -ForegroundColor Green
    $env:PICO_SDK_PATH = $PicoSdkPath
    [System.Environment]::SetEnvironmentVariable("PICO_SDK_PATH", $PicoSdkPath, [System.EnvironmentVariableTarget]::User)
    Write-Host "   -> PICO_SDK_PATH set to $PicoSdkPath" -ForegroundColor Gray
} else {
    Write-Host "❌ Pico SDK NOT found at $PicoSdkPath" -ForegroundColor Red
    Write-Host "   Please clone it: git clone https://github.com/raspberrypi/pico-sdk.git $PicoSdkPath" -ForegroundColor Yellow
}

# 2. Picotool
if (Get-Command picotool -ErrorAction SilentlyContinue) {
    Write-Host "✅ Picotool found in PATH" -ForegroundColor Green
} else {
    $PicotoolPath = "C:\picotool"
    if (Test-Path "$PicotoolPath\picotool.exe") {
        Write-Host "✅ Picotool found at $PicotoolPath" -ForegroundColor Green
        $env:PATH += ";$PicotoolPath"
        [System.Environment]::SetEnvironmentVariable("PATH", $env:PATH + ";$PicotoolPath", [System.EnvironmentVariableTarget]::User)
        Write-Host "   -> Added to PATH" -ForegroundColor Gray
    } else {
        Write-Host "⚠️ Picotool not found in PATH or at $PicotoolPath" -ForegroundColor Yellow
        Write-Host "   We will use the manual copy method or you can install it." -ForegroundColor Gray
    }
}

# 3. mbedTLS (Optional/Legacy)
$MbedTlsPath = "C:\mbedtls"
if (Test-Path $MbedTlsPath) {
    Write-Host "ℹ️  mbedTLS found at $MbedTlsPath (Not currently used by LibreCipher)" -ForegroundColor Gray
} else {
    Write-Host "ℹ️  mbedTLS not found (OK - LibreCipher uses internal crypto)" -ForegroundColor Gray
}

# 4. Ninja & CMake
if (Get-Command ninja -ErrorAction SilentlyContinue) {
    Write-Host "✅ Ninja build system found" -ForegroundColor Green
} else {
    Write-Host "❌ Ninja not found! Install it and add to PATH." -ForegroundColor Red
}

if (Get-Command cmake -ErrorAction SilentlyContinue) {
    Write-Host "✅ CMake found" -ForegroundColor Green
} else {
    Write-Host "❌ CMake not found! Install it and add to PATH." -ForegroundColor Red
}

Write-Host "`n🚀 Environment check complete!" -ForegroundColor Cyan
Write-Host "To load these variables in your current session, run this script as: . .\setup_environment.ps1" -ForegroundColor Gray
