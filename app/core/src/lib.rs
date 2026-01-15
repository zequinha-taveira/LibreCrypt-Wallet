pub mod protocol;
pub mod crypto;
pub mod transactions;

use log::{info, error};

pub fn init() {
    env_logger::init();
    info!("LibreCrypt Core initialized");
}
