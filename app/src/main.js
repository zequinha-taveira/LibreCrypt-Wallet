/**
 * LibreCrypt Wallet - Main JavaScript
 */

// Tauri APIs (se disponível)
let invoke = null;

// Check if running in Tauri
if (window.__TAURI__) {
    invoke = window.__TAURI__.invoke;
}

// State
const state = {
    connected: false,
    walletStatus: 'disconnected',
    firmwareVersion: null,
    currentSection: 'dashboard',
};

// DOM Elements
const elements = {
    connectionStatus: document.getElementById('connectionStatus'),
    walletStatus: document.getElementById('walletStatus'),
    firmwareVersion: document.getElementById('firmwareVersion'),
    unlockBtn: document.getElementById('unlockBtn'),
    unlockModal: document.getElementById('unlockModal'),
    pinInput: document.getElementById('pinInput'),
    cancelUnlock: document.getElementById('cancelUnlock'),
    confirmUnlock: document.getElementById('confirmUnlock'),
    navItems: document.querySelectorAll('.nav-item'),
    sections: document.querySelectorAll('.section'),
    receiveAddress: document.getElementById('receiveAddress'),
    copyAddressBtn: document.getElementById('copyAddressBtn'),
};

// Navigation
function navigateTo(sectionId) {
    state.currentSection = sectionId;

    elements.sections.forEach(section => {
        section.classList.remove('active');
    });

    elements.navItems.forEach(item => {
        item.classList.remove('active');
        if (item.dataset.section === sectionId) {
            item.classList.add('active');
        }
    });

    const targetSection = document.getElementById(sectionId);
    if (targetSection) {
        targetSection.classList.add('active');
    }
}

// Initialize navigation
elements.navItems.forEach(item => {
    item.addEventListener('click', () => {
        navigateTo(item.dataset.section);
    });
});

// Connection Status
function updateConnectionStatus(connected) {
    state.connected = connected;
    const statusDot = elements.connectionStatus.querySelector('.status-dot');
    const statusText = elements.connectionStatus.querySelector('.status-text');

    if (connected) {
        statusDot.classList.remove('disconnected');
        statusDot.classList.add('connected');
        statusText.textContent = 'Conectado';
    } else {
        statusDot.classList.remove('connected');
        statusDot.classList.add('disconnected');
        statusText.textContent = 'Desconectado';
    }
}

// Wallet Status
function updateWalletStatus(status) {
    state.walletStatus = status;
    const statusIcon = elements.walletStatus.querySelector('.status-icon');
    const statusLabel = elements.walletStatus.querySelector('.status-label');

    switch (status) {
        case 'Unlocked':
            statusIcon.textContent = '🔓';
            statusLabel.textContent = 'Desbloqueada';
            elements.unlockBtn.textContent = 'Bloquear';
            break;
        case 'Locked':
            statusIcon.textContent = '🔒';
            statusLabel.textContent = 'Bloqueada';
            elements.unlockBtn.textContent = 'Desbloquear';
            break;
        default:
            statusIcon.textContent = '⚪';
            statusLabel.textContent = 'Desconectada';
            elements.unlockBtn.textContent = 'Conectar';
    }
}

// Firmware Version
function updateFirmwareVersion(version) {
    state.firmwareVersion = version;
    if (version) {
        elements.firmwareVersion.textContent = `v${version.major}.${version.minor}.${version.patch}`;
    } else {
        elements.firmwareVersion.textContent = 'v0.0.0';
    }
}

// Modal
function showUnlockModal() {
    elements.unlockModal.classList.add('active');
    elements.pinInput.focus();
}

function hideUnlockModal() {
    elements.unlockModal.classList.remove('active');
    elements.pinInput.value = '';
}

elements.unlockBtn.addEventListener('click', () => {
    if (state.walletStatus === 'Unlocked') {
        lockWallet();
    } else {
        showUnlockModal();
    }
});

elements.cancelUnlock.addEventListener('click', hideUnlockModal);

elements.confirmUnlock.addEventListener('click', async () => {
    const pin = elements.pinInput.value;
    if (pin) {
        await unlockWallet(pin);
        hideUnlockModal();
    }
});

// Tauri Commands
async function checkConnection() {
    if (!invoke) return false;
    try {
        return await invoke('check_connection');
    } catch (e) {
        console.error('Connection check failed:', e);
        return false;
    }
}

async function getFirmwareVersion() {
    if (!invoke) return null;
    try {
        return await invoke('get_firmware_version');
    } catch (e) {
        console.error('Failed to get version:', e);
        return null;
    }
}

async function getWalletStatus() {
    if (!invoke) return 'Disconnected';
    try {
        return await invoke('get_wallet_status');
    } catch (e) {
        console.error('Failed to get status:', e);
        return 'Disconnected';
    }
}

async function unlockWallet(pin) {
    if (!invoke) return false;
    try {
        const result = await invoke('unlock_wallet', { pin });
        if (result) {
            updateWalletStatus('Unlocked');
        }
        return result;
    } catch (e) {
        console.error('Unlock failed:', e);
        return false;
    }
}

async function lockWallet() {
    if (!invoke) return;
    try {
        await invoke('lock_wallet');
        updateWalletStatus('Locked');
    } catch (e) {
        console.error('Lock failed:', e);
    }
}

async function getAddress(accountIndex) {
    if (!invoke) return null;
    try {
        return await invoke('get_address', { accountIndex });
    } catch (e) {
        console.error('Failed to get address:', e);
        return null;
    }
}

// Copy Address
elements.copyAddressBtn.addEventListener('click', () => {
    const address = elements.receiveAddress.querySelector('code').textContent;
    navigator.clipboard.writeText(address).then(() => {
        elements.copyAddressBtn.textContent = '✅ Copiado!';
        setTimeout(() => {
            elements.copyAddressBtn.textContent = '📋 Copiar';
        }, 2000);
    });
});

// Polling for connection
async function pollConnection() {
    const connected = await checkConnection();
    updateConnectionStatus(connected);

    if (connected) {
        const version = await getFirmwareVersion();
        updateFirmwareVersion(version);

        const status = await getWalletStatus();
        updateWalletStatus(status);

        // Get address if unlocked
        if (status === 'Unlocked') {
            const address = await getAddress(0);
            if (address) {
                elements.receiveAddress.querySelector('code').textContent = address;
            }
        }
    }
}

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    // Initial state
    updateConnectionStatus(false);
    updateWalletStatus('Disconnected');

    // Start polling (if Tauri is available)
    if (invoke) {
        pollConnection();
        setInterval(pollConnection, 5000);
    } else {
        // Demo mode without Tauri
        console.log('Running in demo mode (no Tauri)');

        // Simulate connection after 2 seconds
        setTimeout(() => {
            updateConnectionStatus(true);
            updateFirmwareVersion({ major: 0, minor: 1, patch: 0 });
            updateWalletStatus('Locked');
        }, 2000);
    }
});

console.log('LibreCrypt Wallet App initialized');
