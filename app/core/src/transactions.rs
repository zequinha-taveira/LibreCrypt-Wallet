// Transaction builder module

#[derive(Debug)]
pub struct Transaction {
    pub version: i32,
    pub inputs: Vec<TxInput>,
    pub outputs: Vec<TxOutput>,
    pub lock_time: u32,
}

#[derive(Debug)]
pub struct TxInput {
    pub txid: String,
    pub vout: u32,
    // ...
}

#[derive(Debug)]
pub struct TxOutput {
    pub value: u64,
    pub script_pubkey: String,
}

impl Transaction {
    pub fn new() -> Self {
        Transaction {
            version: 2,
            inputs: Vec::new(),
            outputs: Vec::new(),
            lock_time: 0,
        }
    }
}
