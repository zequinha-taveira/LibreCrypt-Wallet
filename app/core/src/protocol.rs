use serde::{Serialize, Deserialize};

// Constants from PROTOCOL.md
pub const VID: u16 = 0x1209;
pub const PID: u16 = 0x4C43;
pub const USAGE_PAGE: u16 = 0xFF00;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum CommandId {
    Ping = 0x01,
    GetPubkey = 0x10,
    SignTx = 0x11,
    GetDeviceInfo = 0x20,
    InitDevice = 0x30,
    Error = 0xFF,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct DeviceInfo {
    pub version: String,
    pub locked: bool,
    pub initialized: bool,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct PubkeyResponse {
    pub pubkey: String, // Hex encoded
    pub chain_code: String, // Hex encoded
}

#[derive(Debug, Serialize, Deserialize)]
pub struct SignTxRequest {
    pub path: Vec<u32>,
    pub hash: String, // Hex encoded
}

#[derive(Debug, Serialize, Deserialize)]
pub struct SignTxResponse {
    pub signature: String, // Hex encoded
}

// Error types
#[derive(thiserror::Error, Debug)]
pub enum WalletError {
    #[error("Device not found")]
    DeviceNotFound,
    #[error("USB error: {0}")]
    UsbError(#[from] hidapi::HidError),
    #[error("Protocol error: {0}")]
    ProtocolError(String),
    #[error("User cancelled")]
    UserCancelled,
}
