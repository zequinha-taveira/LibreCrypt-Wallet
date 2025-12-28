//! USB Communication Module
//!
//! Comunicação serial com LibreCrypt Wallet hardware

use crate::protocol::{self, Command, Status, VersionInfo, WalletStatus};
use crate::AppWalletStatus;
use serialport::SerialPort;
use std::io::{Read, Write};
use std::sync::Mutex;
use std::time::Duration;

/// Porta serial global
static PORT: Mutex<Option<Box<dyn SerialPort>>> = Mutex::new(None);

/// Procura dispositivo LibreCrypt conectado
fn find_device() -> Option<String> {
    let ports = serialport::available_ports().ok()?;
    for port in ports {
        // TODO: Filtrar por VID/PID específico (Raspberry Pi Debug Probe / RP2350 USB)
        // Por enquanto, aceita qualquer porta COM/ttyACM para facilitar debug
        if port.port_name.contains("COM") || port.port_name.contains("ttyACM") {
            return Some(port.port_name);
        }
    }
    None
}

/// Conecta ao dispositivo
fn connect() -> Result<(), String> {
    let port_name = find_device().ok_or("Device not found")?;
    
    // Tenta abrir a porta apenas se não estiver já aberta (ou tenta reabrir)
    let mut guard = PORT.lock().map_err(|_| "Lock error")?;
    
    if guard.is_some() {
        // Já conectado, mas vamos checar se a porta ainda é válida com um ping?
        // Por simplificação, assumimos que se find_device encontrou, reconectamos.
        // Release lock to drop old port
        *guard = None;
    }

    let port = serialport::new(&port_name, 115200)
        .timeout(Duration::from_millis(1000))
        .open()
        .map_err(|e| format!("Failed to open port: {}", e))?;
    
    *guard = Some(port);
    
    Ok(())
}

/// Envia comando e recebe resposta usando protocolo
fn send_command(cmd: Command, data: &[u8]) -> Result<Vec<u8>, String> {
    // Escopo para lock
    let mut port_guard = PORT.lock().map_err(|_| "Lock error")?;
    let port = port_guard.as_mut().ok_or("Not connected")?;
    
    // Build frame
    let frame = protocol::build_frame(cmd, data);
    
    // Send
    // Clear input buffer first?
    let _ = port.clear(serialport::ClearBuffer::Input);
    
    port.write_all(&frame).map_err(|e| format!("Write error: {}", e))?;
    
    // Receive response
    // Wait for SOF
    let mut buf = [0u8; 1];
    let mut response = Vec::new();
    
    // Read header (at least 5 bytes: SOF LEN STATUS CRC CRC)
    // Simple implementation: read buffer and parse
    let mut raw_buffer = vec![0u8; 512];
    // Espera un pouco para dados chegarem
    std::thread::sleep(Duration::from_millis(10)); 
    let n = port.read(&mut raw_buffer).map_err(|e| format!("Read error: {}", e))?;
    
    if n == 0 {
        return Err("No response".to_string());
    }
    
    response.extend_from_slice(&raw_buffer[..n]);
    
    // Parse
    let (status, payload) = protocol::parse_response(&response).map_err(|e| e.to_string())?;
    
    if status != Status::Ok {
        return Err(format!("Device returned status: {:?}", status));
    }
    
    Ok(payload)
}

/// Verifica se dispositivo está conectado
pub async fn is_device_connected() -> Result<bool, String> {
    if find_device().is_some() {
        if let Err(_) = connect() {
            return Ok(false);
        }
        // Envia ping para garantir que é o dispositivo certo e está respondendo
        let result = send_command(Command::Ping, &[]);
        Ok(result.is_ok())
    } else {
        Ok(false)
    }
}

/// Obtém versão do firmware
pub async fn get_version() -> Result<VersionInfo, String> {
    let data = send_command(Command::GetVersion, &[])?;
    // Protocolo define campos, mas vamos desserializar ou parse manual seguro
    // App espera struct VersionInfo
    // O firmware manda: major(1), minor(1), patch(1), string...
    if data.len() >= 3 {
        // Tenta ler version string se houver
        let version_str = if data.len() > 3 {
             String::from_utf8_lossy(&data[3..]).to_string()
        } else {
             String::new()
        };

        Ok(VersionInfo {
            major: data[0],
            minor: data[1],
            patch: data[2],
            crypto_version: version_str,
        })
    } else {
        Err("Invalid version data".to_string())
    }
}

/// Obtém status da wallet e mapeia para status de UI
pub async fn get_status() -> Result<AppWalletStatus, String> {
    // Tenta obter status do hardware
    match send_command(Command::GetStatus, &[]) {
        Ok(data) => {
            if data.is_empty() {
                return Err("No status data".to_string());
            }
            // Byte 0: WalletStatus (0=Uninitialized, 1=Locked, 2=Unlocked)
            match data[0] {
                0 => Ok(AppWalletStatus::Uninitialized),
                1 => Ok(AppWalletStatus::Locked),
                2 => Ok(AppWalletStatus::Unlocked),
                _ => Ok(AppWalletStatus::Disconnected), // Estado inválido
            }
        },
        Err(_) => {
            // Se falhou comando, assume desconectado
            Ok(AppWalletStatus::Disconnected)
        }
    }
}

/// Cria nova wallet
pub async fn create_wallet(pin: &str) -> Result<bool, String> {
    let pin_bytes = pin.as_bytes();
    // Protocolo usa create_wallet com PIN
    let _ = send_command(Command::CreateWallet, pin_bytes)?;
    Ok(true)
}

/// Desbloqueia wallet
pub async fn unlock(pin: &str) -> Result<bool, String> {
    let pin_bytes = pin.as_bytes();
    let _ = send_command(Command::Unlock, pin_bytes)?;
    Ok(true)
}

/// Bloqueia wallet
pub async fn lock() -> Result<(), String> {
    send_command(Command::Lock, &[])?;
    Ok(())
}

/// Obtém endereço
pub async fn get_address(account_index: u32) -> Result<String, String> {
    let data = account_index.to_le_bytes();
    let response = send_command(Command::GetAddress, &data)?;
    String::from_utf8(response).map_err(|_| "Invalid address encoding".to_string())
}
