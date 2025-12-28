//! LibreCrypt Wallet Protocol
//!
//! Protocolo de comunicação seguro entre App e Hardware

use serde::{Deserialize, Serialize};

/// Frame format
/// [SOF (0xAA)][LEN][CMD][DATA...][CRC16-LO][CRC16-HI]
pub const SOF: u8 = 0xAA;
pub const MAX_FRAME_SIZE: usize = 256;

/// Commands
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum Command {
    Ping = 0x01,
    GetVersion = 0x02,
    GetStatus = 0x03,
    CreateWallet = 0x10,
    Unlock = 0x11,
    Lock = 0x12,
    GetAddress = 0x20,
    SignTransaction = 0x21,
    VerifySignature = 0x22,
    // Session management
    InitSession = 0x30,
    CloseSession = 0x31,
}

/// Response status
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum Status {
    Ok = 0x00,
    Error = 0x01,
    InvalidCommand = 0x02,
    Locked = 0x03,
    NeedConfirm = 0x04,
    InvalidPin = 0x05,
    InvalidSignature = 0x06,
    SessionExpired = 0x07,
}

/// Version info
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct VersionInfo {
    pub major: u8,
    pub minor: u8,
    pub patch: u8,
    pub crypto_version: String,
}

/// Wallet status
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum WalletStatus {
    Uninitialized = 0,
    Locked = 1,
    Unlocked = 2,
}

/// CRC-16 (Modbus)
pub fn crc16(data: &[u8]) -> u16 {
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

/// Build frame
pub fn build_frame(cmd: Command, data: &[u8]) -> Vec<u8> {
    let mut frame = Vec::with_capacity(data.len() + 5);
    frame.push(SOF);
    frame.push((data.len() + 1) as u8);
    frame.push(cmd as u8);
    frame.extend_from_slice(data);
    
    let crc = crc16(&frame[1..]);
    frame.push((crc & 0xFF) as u8);
    frame.push((crc >> 8) as u8);
    
    frame
}

/// Parse response
pub fn parse_response(frame: &[u8]) -> Result<(Status, Vec<u8>), &'static str> {
    if frame.len() < 5 {
        return Err("Frame too short");
    }
    
    if frame[0] != SOF {
        return Err("Invalid SOF");
    }
    
    let len = frame[1] as usize;
    let expected_len = 2 + len + 2;
    
    if frame.len() < expected_len {
        return Err("Incomplete frame");
    }
    
    // Verify CRC
    let received_crc = (frame[expected_len - 2] as u16) | ((frame[expected_len - 1] as u16) << 8);
    let calc_crc = crc16(&frame[1..expected_len - 2]);
    
    if received_crc != calc_crc {
        return Err("CRC mismatch");
    }
    
    let status = match frame[2] {
        0x00 => Status::Ok,
        0x01 => Status::Error,
        0x02 => Status::InvalidCommand,
        0x03 => Status::Locked,
        0x04 => Status::NeedConfirm,
        0x05 => Status::InvalidPin,
        0x06 => Status::InvalidSignature,
        0x07 => Status::SessionExpired,
        _ => Status::Error,
    };
    
    let data = frame[3..3 + len - 1].to_vec();
    
    Ok((status, data))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_crc16() {
        let data = [0x01, 0x02, 0x03];
        let crc = crc16(&data);
        assert_ne!(crc, 0);
    }

    #[test]
    fn test_build_frame() {
        let frame = build_frame(Command::Ping, &[]);
        assert_eq!(frame[0], SOF);
        assert_eq!(frame[1], 1); // len = cmd only
        assert_eq!(frame[2], Command::Ping as u8);
    }

    #[test]
    fn test_parse_response() {
        let frame = build_frame(Command::Ping, &[]);
        // Simulate response: replace cmd with status
        let mut response = frame.clone();
        response[2] = Status::Ok as u8;
        
        // Recalculate CRC
        let crc = crc16(&response[1..response.len() - 2]);
        let len = response.len();
        response[len - 2] = (crc & 0xFF) as u8;
        response[len - 1] = (crc >> 8) as u8;
        
        let result = parse_response(&response);
        assert!(result.is_ok());
        let (status, _data) = result.unwrap();
        assert_eq!(status, Status::Ok);
    }
}
