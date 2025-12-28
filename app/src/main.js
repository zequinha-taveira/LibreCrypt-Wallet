/**
 * LibreCrypt Wallet - Main Application
 */

// Import Tauri API if available
let invoke = null;
let listen = null;

async function setupTauri() {
    try {
        const tauri = await import('@tauri-apps/api/core');
        invoke = tauri.invoke;
        const event = await import('@tauri-apps/api/event');
        listen = event.listen;
        return true;
    } catch (e) {
        console.log('Running in browser mode (Tauri not available)');
        return false;
    }
}

// State
const state = {
    connected: false,
    walletStatus: 'disconnected', // disconnected, locked, unlocked
    accounts: [],
    currentPage: 'dashboard',
    deviceInfo: null
};

// DOM Elements
const elements = {
    statusDot: document.getElementById('status-dot'),
    statusText: document.getElementById('status-text'),
    pageTitle: document.getElementById('page-title'),
    btnConnect: document.getElementById('btn-connect'),
    btnRefresh: document.getElementById('btn-refresh'),
    pinModal: document.getElementById('pin-modal'),
    toastContainer: document.getElementById('toast-container'),
    walletStatusBadge: document.getElementById('wallet-status-badge'),
    totalBalance: document.getElementById('total-balance'),
    accountCount: document.getElementById('account-count'),
    firmwareVersion: document.getElementById('firmware-version')
};

// Navigation
const navItems = document.querySelectorAll('.nav-item[data-page]');
const pages = document.querySelectorAll('.page');

function showPage(pageName) {
    state.currentPage = pageName;

    // Update nav
    navItems.forEach(item => {
        item.classList.toggle('active', item.dataset.page === pageName);
    });

    // Update pages
    pages.forEach(page => {
        const isActive = page.id === `page-${pageName}`;
        page.classList.toggle('hidden', !isActive);
        if (isActive) {
            page.classList.add('animate-fadeIn');
        }
    });

    // Update title
    const titles = {
        dashboard: 'Dashboard',
        accounts: 'Contas',
        send: 'Enviar',
        receive: 'Receber',
        settings: 'Configurações',
        security: 'Segurança'
    };
    elements.pageTitle.textContent = titles[pageName] || 'Dashboard';
}

// Make showPage global for onclick handlers
window.showPage = showPage;

navItems.forEach(item => {
    item.addEventListener('click', () => showPage(item.dataset.page));
});

// Device Connection
async function checkConnection() {
    if (!invoke) {
        // Simulate connection in browser mode
        updateDeviceStatus('disconnected');
        return;
    }

    try {
        const connected = await invoke('check_connection');
        if (connected) {
            await updateWalletStatus();
        } else {
            updateDeviceStatus('disconnected');
        }
    } catch (error) {
        console.error('Connection check failed:', error);
        updateDeviceStatus('disconnected');
    }
}

async function connect() {
    showToast('Conectando ao dispositivo...', 'info');

    if (!invoke) {
        // Simulate in browser mode
        setTimeout(() => {
            state.connected = true;
            updateDeviceStatus('locked');
            showToast('Conectado (modo demo)', 'success');
        }, 1000);
        return;
    }

    try {
        const connected = await invoke('check_connection');
        if (connected) {
            state.connected = true;
            await updateWalletStatus();
            showToast('Dispositivo conectado!', 'success');
        } else {
            showToast('Nenhum dispositivo encontrado', 'error');
        }
    } catch (error) {
        showToast(`Erro: ${error}`, 'error');
    }
}

async function updateWalletStatus() {
    if (!invoke) return;

    try {
        const status = await invoke('get_wallet_status');
        state.walletStatus = status.toLowerCase();
        updateDeviceStatus(state.walletStatus);

        // Update UI based on status
        updateDashboard();
    } catch (error) {
        console.error('Failed to get wallet status:', error);
    }
}

function updateDeviceStatus(status) {
    state.walletStatus = status;

    const statusDot = elements.statusDot;
    const statusText = elements.statusText;
    const badge = elements.walletStatusBadge;

    statusDot.classList.remove('connected', 'locked', 'disconnected');
    badge.classList.remove('badge-success', 'badge-warning', 'badge-danger');

    switch (status) {
        case 'unlocked':
            statusDot.classList.add('connected');
            statusText.textContent = 'Desbloqueado';
            badge.textContent = 'Desbloqueado';
            badge.classList.add('badge-success');
            elements.btnConnect.innerHTML = '🔒 Bloquear';
            break;
        case 'locked':
            statusDot.classList.add('locked');
            statusText.textContent = 'Bloqueado';
            badge.textContent = 'Bloqueado';
            badge.classList.add('badge-warning');
            elements.btnConnect.innerHTML = '🔓 Desbloquear';
            break;
        default:
            statusDot.classList.add('disconnected');
            statusText.textContent = 'Desconectado';
            badge.textContent = 'Desconectado';
            badge.classList.add('badge-danger');
            elements.btnConnect.innerHTML = '🔌 Conectar';
    }
}

function updateDashboard() {
    // Update account count
    elements.accountCount.textContent = state.accounts.length;

    // Update balance
    const totalBalance = state.accounts.reduce((sum, acc) => sum + acc.balance, 0);
    elements.totalBalance.textContent = `${totalBalance.toFixed(8)} BTC`;
}

// PIN Modal
let pinCallback = null;

function showPinModal(callback) {
    pinCallback = callback;
    elements.pinModal.classList.remove('hidden');

    // Focus first input
    const firstInput = document.querySelector('.pin-digit[data-index="0"]');
    if (firstInput) firstInput.focus();
}

function hidePinModal() {
    elements.pinModal.classList.add('hidden');
    pinCallback = null;

    // Clear inputs
    document.querySelectorAll('.pin-digit').forEach(input => {
        input.value = '';
    });
}

function getPin() {
    const digits = document.querySelectorAll('.pin-digit');
    return Array.from(digits).map(d => d.value).join('');
}

// PIN input handling
document.querySelectorAll('.pin-digit').forEach(input => {
    input.addEventListener('input', (e) => {
        const index = parseInt(e.target.dataset.index);
        if (e.target.value && index < 5) {
            const next = document.querySelector(`.pin-digit[data-index="${index + 1}"]`);
            if (next) next.focus();
        }
    });

    input.addEventListener('keydown', (e) => {
        if (e.key === 'Backspace' && !e.target.value) {
            const index = parseInt(e.target.dataset.index);
            if (index > 0) {
                const prev = document.querySelector(`.pin-digit[data-index="${index - 1}"]`);
                if (prev) prev.focus();
            }
        }
    });
});

document.getElementById('modal-close')?.addEventListener('click', hidePinModal);
document.getElementById('btn-cancel-pin')?.addEventListener('click', hidePinModal);
document.getElementById('btn-confirm-pin')?.addEventListener('click', () => {
    const pin = getPin();
    if (pin.length === 6 && pinCallback) {
        pinCallback(pin);
        hidePinModal();
    }
});

// Toast Notifications
function showToast(message, type = 'info') {
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;

    const icons = {
        success: '✅',
        error: '❌',
        warning: '⚠️',
        info: 'ℹ️'
    };

    toast.innerHTML = `
    <span>${icons[type] || icons.info}</span>
    <span>${message}</span>
  `;

    elements.toastContainer.appendChild(toast);

    setTimeout(() => {
        toast.style.animation = 'fadeIn 0.3s ease reverse';
        setTimeout(() => toast.remove(), 300);
    }, 3000);
}

// Button handlers
elements.btnConnect?.addEventListener('click', async () => {
    if (state.walletStatus === 'disconnected') {
        await connect();
    } else if (state.walletStatus === 'locked') {
        showPinModal(async (pin) => {
            try {
                if (invoke) {
                    const success = await invoke('unlock_wallet', { pin });
                    if (success) {
                        showToast('Wallet desbloqueada!', 'success');
                        updateDeviceStatus('unlocked');
                    } else {
                        showToast('PIN incorreto', 'error');
                    }
                } else {
                    // Demo mode
                    showToast('Wallet desbloqueada (demo)', 'success');
                    updateDeviceStatus('unlocked');
                }
            } catch (error) {
                showToast(`Erro: ${error}`, 'error');
            }
        });
    } else if (state.walletStatus === 'unlocked') {
        try {
            if (invoke) {
                await invoke('lock_wallet');
            }
            showToast('Wallet bloqueada', 'info');
            updateDeviceStatus('locked');
        } catch (error) {
            showToast(`Erro: ${error}`, 'error');
        }
    }
});

elements.btnRefresh?.addEventListener('click', async () => {
    elements.btnRefresh.classList.add('animate-spin');
    await checkConnection();
    setTimeout(() => {
        elements.btnRefresh.classList.remove('animate-spin');
    }, 1000);
});

// Create wallet button
document.getElementById('btn-create-wallet')?.addEventListener('click', () => {
    if (state.walletStatus === 'disconnected') {
        showToast('Conecte o dispositivo primeiro', 'warning');
        return;
    }

    showPinModal(async (pin) => {
        try {
            if (invoke) {
                const success = await invoke('create_wallet', { pin });
                if (success) {
                    showToast('Wallet criada com sucesso!', 'success');
                    updateDeviceStatus('unlocked');
                }
            } else {
                showToast('Wallet criada (demo)', 'success');
                updateDeviceStatus('unlocked');
            }
        } catch (error) {
            showToast(`Erro: ${error}`, 'error');
        }
    });
});

// Copy address button
document.getElementById('btn-copy-address')?.addEventListener('click', async () => {
    const addressEl = document.getElementById('receive-address');
    const address = addressEl?.textContent || '';

    try {
        await navigator.clipboard.writeText(address);
        showToast('Endereço copiado!', 'success');
    } catch (error) {
        showToast('Erro ao copiar', 'error');
    }
});

// Toggle switches
document.querySelectorAll('.toggle').forEach(toggle => {
    toggle.addEventListener('click', () => {
        toggle.classList.toggle('active');
    });
});

// Initialize
async function init() {
    console.log('LibreCrypt Wallet App initialized');
    await setupTauri();
    await checkConnection();

    // Start polling for connection status
    setInterval(checkConnection, 5000);
}

init();
