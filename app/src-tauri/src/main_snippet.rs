
/// Comando: Assinar transação
#[tauri::command]
async fn sign_transaction(account_index: u32, to_address: String, amount: u64, fee: u64) -> Result<String, String> {
    usb::sign_transaction(account_index, &to_address, amount, fee).await
}
