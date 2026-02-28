/**
 * Lab-Web-Demo Main Application Module
 * Centralized controller for LED control and telemetry display
 */

class LabWebApp {
    constructor() {
        this.currentSection = 'not-connected';
        this.isInitialized = false;
        this.connectionState = 'disconnected';
        this.offlineMode = false;

        // LED state tracking
        this.ledState = {
            yellow: false,
            red: false
        };

        // Telemetry tracking
        this.lastTelemetryTime = null;
        this.telemetryCount = 0;
    }

    /**
     * Initialize the application
     */
    async init() {
        if (this.isInitialized) return;

        console.log('Initializing Lab-Web-Demo Application');

        try {
            this.setupNavigation();
            this.setupLedControls();
            this.setupLogControls();
            this.setupOfflineModeButton();
            this.initializeWebSocket();

            this.showSection('not-connected');
            this.setNavbarState(false);

            // Attempt connection
            this.connectWebSocket();

            this.isInitialized = true;
            console.log('Lab-Web-Demo initialized successfully');

        } catch (error) {
            console.error('Failed to initialize Lab-Web-Demo:', error);
        }
    }

    // =========================================================================
    // WebSocket Setup
    // =========================================================================

    initializeWebSocket() {
        if (!window.LabWebWebSocket) {
            console.error('LabWebWebSocket not available');
            return;
        }

        const ws = window.LabWebWebSocket;
        ws.eventHandlers.clear();

        // Connection events
        ws.on('connected', () => {
            this.connectionState = 'connected';
            this.offlineMode = false;
            this.updateConnectionStatus('connected');
            this.setNavbarState(true);
            this.showSection('dashboard');
            this.setStatusMessage('Connected to STM32 controller', 'success');
            this.enableLedButtons(true);
            this.addLogEntry('Connected to WebSocket server', 'success');
        });

        ws.on('disconnected', (data) => {
            this.connectionState = 'disconnected';
            this.updateConnectionStatus('disconnected');
            this.setStatusMessage('Disconnected from controller', 'warning');
            this.enableLedButtons(false);
            this.addLogEntry(`Disconnected (code: ${data.code})`, 'warning');

            if (!this.offlineMode) {
                this.setNavbarState(false);
                this.showSection('not-connected');
            }
        });

        ws.on('connecting', () => {
            this.connectionState = 'connecting';
            this.updateConnectionStatus('connecting');
        });

        // Data events
        ws.on('led-status', (data) => {
            this.handleLedStatus(data);
        });

        ws.on('telemetry', (data) => {
            this.handleTelemetry(data);
        });

        console.log('WebSocket event handlers registered');
    }

    connectWebSocket() {
        if (!window.LabWebWebSocket) return;
        this.updateConnectionStatus('connecting');
        window.LabWebWebSocket.connect();
    }

    // =========================================================================
    // LED Control
    // =========================================================================

    setupLedControls() {
        const yellowBtn = document.getElementById('yellow-led-btn');
        const redBtn = document.getElementById('red-led-btn');

        if (yellowBtn) {
            yellowBtn.addEventListener('click', () => {
                const newState = !this.ledState.yellow;
                window.LabWebWebSocket.sendLedCommand('yellow', newState);
                this.addLogEntry(`TX: led-command yellow=${newState}`, 'tx');
            });
        }

        if (redBtn) {
            redBtn.addEventListener('click', () => {
                const newState = !this.ledState.red;
                window.LabWebWebSocket.sendLedCommand('red', newState);
                this.addLogEntry(`TX: led-command red=${newState}`, 'tx');
            });
        }
    }

    enableLedButtons(enabled) {
        const yellowBtn = document.getElementById('yellow-led-btn');
        const redBtn = document.getElementById('red-led-btn');
        if (yellowBtn) yellowBtn.disabled = !enabled;
        if (redBtn) redBtn.disabled = !enabled;
    }

    /**
     * Handle led-status message from server
     */
    handleLedStatus(data) {
        this.ledState.yellow = data.yellowLed;
        this.ledState.red = data.redLed;

        this.updateLedUI('yellow', data.yellowLed);
        this.updateLedUI('red', data.redLed);

        this.addLogEntry(`RX: led-status yellow=${data.yellowLed} red=${data.redLed}`, 'rx');
    }

    /**
     * Update LED button and indicator UI
     */
    updateLedUI(led, state) {
        const indicator = document.getElementById(`${led}-led-indicator`);
        const statusText = document.getElementById(`${led}-led-status-text`);
        const btn = document.getElementById(`${led}-led-btn`);
        const statusBarIcon = document.getElementById(`status-bar-${led}-led`);
        const statusBarText = document.getElementById(`status-bar-${led}-text`);

        const onColor = led === 'yellow' ? '#FFD600' : '#FF1744';
        const offColor = '#555';

        if (indicator) indicator.style.color = state ? onColor : offColor;
        if (statusText) {
            statusText.textContent = state ? 'ON' : 'OFF';
            statusText.className = state ? 'text-success' : 'text-muted';
        }
        if (btn) {
            btn.className = `btn btn-lg px-4 ${state ? 'btn-success' : 'btn-secondary'}`;
            btn.innerHTML = `<i class="bi bi-power"></i> Turn ${state ? 'OFF' : 'ON'}`;
        }
        if (statusBarIcon) statusBarIcon.style.color = state ? onColor : offColor;
        if (statusBarText) {
            statusBarText.textContent = state ? 'ON' : 'OFF';
            statusBarText.className = state ? 'text-success' : '';
        }
    }

    // =========================================================================
    // Telemetry Display
    // =========================================================================

    /**
     * Handle telemetry message from server
     */
    handleTelemetry(data) {
        this.telemetryCount++;
        this.lastTelemetryTime = Date.now();

        // Junction Temperature
        updateNumericDisplay('telemetry-junction-temp', data.junctionTemp, 1);

        // Random Float
        updateNumericDisplay('telemetry-random-float', data.randomFloat, 2);

        // Random Integer
        const intEl = document.getElementById('telemetry-random-int');
        if (intEl) intEl.textContent = (data.randomInt !== null && data.randomInt !== undefined)
            ? data.randomInt.toString()
            : '--';

        // Uptime
        const uptimeEl = document.getElementById('telemetry-uptime');
        if (uptimeEl) uptimeEl.textContent = this.formatUptime(data.uptime);

        // Update status
        const statusEl = document.getElementById('telemetry-status');
        if (statusEl) {
            statusEl.className = 'alert alert-success mb-0 mt-3 py-2';
            statusEl.innerHTML = `<i class="bi bi-broadcast"></i> Receiving telemetry ` +
                `(${this.telemetryCount} packets) &mdash; Uptime: ${this.formatUptime(data.uptime)}`;
        }
    }

    /**
     * Format seconds to a human-readable uptime string
     */
    formatUptime(seconds) {
        if (seconds === null || seconds === undefined) return '--';
        const h = Math.floor(seconds / 3600);
        const m = Math.floor((seconds % 3600) / 60);
        const s = seconds % 60;
        if (h > 0) return `${h}h ${m}m ${s}s`;
        if (m > 0) return `${m}m ${s}s`;
        return `${s}s`;
    }

    // =========================================================================
    // WebSocket Log
    // =========================================================================

    setupLogControls() {
        const clearBtn = document.getElementById('clear-log-btn');
        if (clearBtn) {
            clearBtn.addEventListener('click', () => {
                const log = document.getElementById('ws-log');
                if (log) log.innerHTML = '<div class="text-muted">Log cleared.</div>';
            });
        }
    }

    /**
     * Add an entry to the WebSocket log
     */
    addLogEntry(message, type = 'info') {
        const log = document.getElementById('ws-log');
        if (!log) return;

        const colorMap = {
            tx: '#42A5F5',       // blue
            rx: '#66BB6A',       // green
            success: '#00C853',  // bright green
            warning: '#FFD600',  // yellow
            error: '#FF1744',    // red
            info: '#90A4AE'      // gray
        };

        const color = colorMap[type] || colorMap.info;
        const time = new Date().toLocaleTimeString('en-US', { hour12: false });

        const entry = document.createElement('div');
        entry.style.color = color;
        entry.textContent = `[${time}] ${message}`;

        log.appendChild(entry);
        log.scrollTop = log.scrollHeight;

        // Limit log size
        while (log.children.length > 200) {
            log.removeChild(log.firstChild);
        }
    }

    // =========================================================================
    // Navigation
    // =========================================================================

    setupNavigation() {
        document.querySelectorAll('[data-section]').forEach(link => {
            link.addEventListener('click', (e) => {
                e.preventDefault();
                const section = e.target.closest('[data-section]').dataset.section;
                this.showSection(section);
            });
        });
    }

    showSection(sectionName) {
        document.querySelectorAll('.content-section').forEach(section => {
            section.classList.remove('active');
            section.style.display = 'none';
        });

        const target = document.getElementById(`${sectionName}-section`);
        if (target) {
            target.classList.add('active');
            target.style.display = 'block';
        }

        // Update active nav link
        document.querySelectorAll('[data-section]').forEach(link => {
            link.classList.remove('active');
            if (link.dataset.section === sectionName) {
                link.classList.add('active');
            }
        });

        this.currentSection = sectionName;
    }

    setNavbarState(enabled) {
        document.querySelectorAll('.navbar-nav .nav-link').forEach(link => {
            if (enabled) {
                link.classList.remove('disabled');
                link.style.pointerEvents = '';
            } else {
                link.classList.add('disabled');
                link.style.pointerEvents = 'none';
            }
        });
    }

    // =========================================================================
    // Status Bar
    // =========================================================================

    updateConnectionStatus(status) {
        const statusIcon = document.querySelectorAll('.connection-status-icon');
        statusIcon.forEach(icon => {
            icon.classList.remove('connecting', 'connected', 'disconnected', 'error');
            icon.classList.add(status);
        });
        this.connectionState = status;
    }

    setStatusMessage(message, type = 'info') {
        const icon = document.getElementById('status-icon');
        const msg = document.getElementById('status-message');
        if (!icon || !msg) return;

        const iconMap = {
            success: 'bi-check-circle-fill text-success',
            warning: 'bi-exclamation-triangle-fill text-warning',
            error: 'bi-x-circle-fill text-danger',
            info: 'bi-info-circle text-warning'
        };

        const textColorMap = {
            success: 'text-success',
            warning: 'text-warning',
            error: 'text-danger',
            info: 'text-warning'
        };

        icon.className = `bi ${iconMap[type] || iconMap.info} me-2`;
        msg.className = textColorMap[type] || textColorMap.info;
        msg.textContent = message;
    }

    // =========================================================================
    // Offline Mode
    // =========================================================================

    setupOfflineModeButton() {
        const offlineBtn = document.getElementById('btnWorkOffline');
        if (offlineBtn) {
            offlineBtn.addEventListener('click', () => {
                if (this.offlineMode) {
                    // Reconnect
                    this.offlineMode = false;
                    offlineBtn.textContent = 'start without connecting';
                    this.showSection('not-connected');
                    this.setNavbarState(false);
                    this.connectWebSocket();
                } else {
                    // Go offline
                    this.offlineMode = true;
                    offlineBtn.textContent = 'connect';
                    if (window.LabWebWebSocket) {
                        window.LabWebWebSocket.autoReconnectEnabled = false;
                        window.LabWebWebSocket.disconnect();
                    }
                    this.setNavbarState(true);
                    this.showSection('dashboard');
                    this.setStatusMessage('Working offline — no live data', 'warning');
                    this.addLogEntry('Switched to offline mode', 'warning');
                }
            });
        }
    }

    // =========================================================================
    // Notification helper
    // =========================================================================

    showNotification(title, message, type = 'info') {
        console.log(`[${type.toUpperCase()}] ${title}: ${message}`);
        this.addLogEntry(`${title}: ${message}`, type === 'danger' ? 'error' : type);
    }

    // =========================================================================
    // Shutdown
    // =========================================================================

    shutdown() {
        if (window.LabWebWebSocket) {
            window.LabWebWebSocket.disconnect();
        }
    }
}

// Create global app instance
window.LabWebApp = new LabWebApp();

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    console.log('DOM loaded, initializing Lab-Web-Demo...');
    window.LabWebApp.init();
});

// Handle page unload
window.addEventListener('beforeunload', () => {
    window.LabWebApp.shutdown();
});
