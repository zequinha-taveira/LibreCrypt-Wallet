// Crypto helper module
// Will interface with app/core dependencies

pub fn validate_address(address: &str) -> bool {
    // TODO: Implement address validation (P2PKH, P2WPKH)
    if address.is_empty() {
        return false;
    }
    true
}
