//! USB Communication Module
//!
//! Comunicação serial com LibreCrypt Wallet hardware

use crate::{VersionInfo, WalletStatus};
use serialport::SerialPort;
use std::io::{Read, Write};
use std::sync::Mutex;
use std::time::Duration;

/// Frame SOF byte
const SOF: u8 = 0xAA;

/// Comandos
const CMD_PING: u8 = 0x01;
const CMD_GET_VERSION: u8 = 0x02;
const CMD_GET_STATUS: u8 = 0x03;
const CMD_CREATE_WALLET: u8 = 0x10;
const CMD_UNLOCK: u8 = 0x11;
const CMD_LOCK: u8 = 0x12;
const CMD_GET_ADDRESS: u8 = 0x20;

/// Status de resposta
const STATUS_OK: u8 = 0x00;

/// Porta serial global
static PORT: Mutex<Option<Box<dyn SerialPort>>> = Mutex::new(None);

/// CRC-16
fn crc16(data: &[u8]) -> u16 {
    let mut crc: u16 = 0xFFFF;
    for byte in data {
        crc ^= *byte as u16;
        for _ in 0..8 {
            if crc & 1 != 0 {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    crc
}

/// Procura dispositivo LibreCrypt conectado
fn find_device() -> Option<String> {
    let ports = serialport::available_ports().ok()?;
    for port in ports {
        // TODO: Filtrar por VID/PID específico
        if port.port_name.contains("COM") || port.port_name.contains("ttyACM") {
            return Some(port.port_name);
        }
    }
    None
}

/// Conecta ao dispositivo
fn connect() -> Result<(), String> {
    let port_name = find_device().ok_or("Device not found")?;
    
    let port = serialport::new(&port_name, 115200)
        .timeout(Duration::from_millis(1000))
        .open()
        .map_err(|e| format!("Failed to open port: {}", e))?;
    
    let mut guard = PORT.lock().map_err(|_| "Lock error")?;
    *guard = Some(port);
    
    Ok(())
}

/// Envia comando e recebe resposta
fn send_command(cmd: u8, data: &[u8]) -> Result<Vec<u8>, String> {
    let mut guard = PORT.lock().map_err(|_| "Lock error")?;
    let port = guard.as_mut().ok_or("Not connected")?;
    
    // Build frame
    let mut frame = vec![SOF, (data.len() + 1) as u8, cmd];
    frame.extend_from_slice(data);
    let crc = crc16(&frame[1..]);
    frame.push((crc & 0xFF) as u8);
    frame.push((crc >> 8) as u8);
    
    // Send
    port.write_all(&frame).map_err(|e| format!("Write error: {}", e))?;
    
    // Receive response
    let mut response = vec![0u8; 256];
    let n = port.read(&mut response).map_err(|e| format!("Read error: {}", e))?;
    
    if n < 5 || response[0] != SOF {
        return Err("Invalid response".to_string());
    }
    
    let len = response[1] as usize;
    let status = response[2];
    
    if status != STATUS_OK {
        return Err(format!("Device error: {}", status));
    }
    
    // Return data (excluding status)
    Ok(response[3..3 + len - 1].to_vec())
}

/// Verifica se dispositivo está conectado
pub async fn is_device_connected() -> Result<bool, String> {
    if find_device().is_some() {
        connect()?;
        let result = send_command(CMD_PING, &[]);
        Ok(result.is_ok())
    } else {
        Ok(false)
    }
}

/// Obtém versão do firmware
pub async fn get_version() -> Result<VersionInfo, String> {
    let data = send_command(CMD_GET_VERSION, &[])?;
    if data.len() >= 3 {
        Ok(VersionInfo {
            major: data[0],
            minor: data[1],
            patch: data[2],
        })
    } else {
        Err("Invalid version data".to_string())
    }
}

/// Obtém status da wallet
pub async fn get_status() -> Result<WalletStatus, String> {
    let data = send_command(CMD_GET_STATUS, &[])?;
    if data.is_empty() {
        return Err("No status data".to_string());
    }
    
    match data[0] {
        0 => Ok(WalletStatus::Disconnected),
        1 => Ok(WalletStatus::Locked),
        2 => Ok(WalletStatus::Unlocked),
        _ => Ok(WalletStatus::Connected),
    }
}

/// Cria nova wallet
pub async fn create_wallet(pin: &str) -> Result<bool, String> {
    let pin_bytes = pin.as_bytes();
    let result = send_command(CMD_CREATE_WALLET, pin_bytes)?;
    Ok(result.is_empty() || result[0] == STATUS_OK)
}

/// Desbloqueia wallet
pub async fn unlock(pin: &str) -> Result<bool, String> {
    let pin_bytes = pin.as_bytes();
    let result = send_command(CMD_UNLOCK, pin_bytes)?;
    Ok(result.is_empty() || result[0] == STATUS_OK)
}

/// Bloqueia wallet
pub async fn lock() -> Result<(), String> {
    send_command(CMD_LOCK, &[])?;
    Ok(())
}

/// Obtém endereço
pub async fn get_address(account_index: u32) -> Result<String, String> {
    let data = account_index.to_le_bytes();
    let response = send_command(CMD_GET_ADDRESS, &data)?;
    String::from_utf8(response).map_err(|_| "Invalid address encoding".to_string())
}
