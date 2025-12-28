
/// Assina transação
pub async fn sign_transaction(account_index: u32, to_address: &str, amount: u64, fee: u64) -> Result<String, String> {
    // Serializar dados da transação para envio ao hardware
    // Formato simples: [AccountIndex(4)][Amount(8)][Fee(8)][AddressLen(1)][AddressBytes...]
    let mut data = Vec::new();
    data.extend_from_slice(&account_index.to_le_bytes());
    data.extend_from_slice(&amount.to_le_bytes());
    data.extend_from_slice(&fee.to_le_bytes());
    
    let addr_bytes = to_address.as_bytes();
    if addr_bytes.len() > 200 {
        return Err("Address too long".to_string());
    }
    data.push(addr_bytes.len() as u8);
    data.extend_from_slice(addr_bytes);

    let signature = send_command(Command::SignTransaction, &data)?;
    
    // Retorna assinatura em hex
    Ok(hex::encode(signature))
}
