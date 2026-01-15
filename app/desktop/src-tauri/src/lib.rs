use librecrypt_core::protocol::DeviceInfo;

// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

#[tauri::command]
fn get_device_info() -> DeviceInfo {
    // Mock response for now
    DeviceInfo {
        version: "0.1.0".to_string(),
        locked: true,
        initialized: true,
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    librecrypt_core::init();
    
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![greet, get_device_info])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
