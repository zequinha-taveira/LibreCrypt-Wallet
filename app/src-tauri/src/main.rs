//! LibreCrypt Wallet App - Backend Rust
//!
//! Comunicação USB com hardware wallet

#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod usb;
mod protocol;

use serde::{Deserialize, Serialize};
use tauri::Manager;
use crate::protocol::{VersionInfo, WalletStatus};

/// Status da wallet estendido para UI (inclui estados de conexão)
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub enum AppWalletStatus {
    Disconnected,
    Uninitialized,
    Locked,
    Unlocked,
}

/// Comando: Verificar conexão com hardware
#[tauri::command]
async fn check_connection() -> Result<bool, String> {
    usb::is_device_connected().await
}

/// Comando: Obter versão do firmware
#[tauri::command]
async fn get_firmware_version() -> Result<VersionInfo, String> {
    usb::get_version().await
}

/// Comando: Obter status da wallet
#[tauri::command]
async fn get_wallet_status() -> Result<AppWalletStatus, String> {
    usb::get_status().await
}

/// Comando: Criar nova wallet
#[tauri::command]
async fn create_wallet(pin: String) -> Result<bool, String> {
    usb::create_wallet(&pin).await
}

/// Comando: Desbloquear wallet
#[tauri::command]
async fn unlock_wallet(pin: String) -> Result<bool, String> {
    usb::unlock(&pin).await
}

/// Comando: Bloquear wallet
#[tauri::command]
async fn lock_wallet() -> Result<(), String> {
    usb::lock().await
}

/// Comando: Obter endereço
#[tauri::command]
async fn get_address(account_index: u32) -> Result<String, String> {
    usb::get_address(account_index).await
}

fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![
            check_connection,
            get_firmware_version,
            get_wallet_status,
            create_wallet,
            unlock_wallet,
            lock_wallet,
            get_address,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
