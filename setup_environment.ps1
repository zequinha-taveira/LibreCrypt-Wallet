# Simple Environment Setup
$pico = "C:\pico-sdk"
if (Test-Path $pico) {
    Write-Host "Pico SDK found."
    $env:PICO_SDK_PATH = $pico
    [System.Environment]::SetEnvironmentVariable("PICO_SDK_PATH", $pico, "User")
}
else {
    Write-Host "Pico SDK not found at $pico"
}

$picotool = "C:\picotool"
if (Test-Path "$picotool\picotool.exe") {
    Write-Host "Picotool found."
    $env:PATH += ";$picotool"
    $newPath = $env:PATH + ";$picotool"
    [System.Environment]::SetEnvironmentVariable("PATH", $newPath, "User")
}

Write-Host "Environment setup done."
