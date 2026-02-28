/**
 * Lab-Web-Demo WebSocket Communication Module
 * Handles real-time communication with the STM32H723 controller
 *
 * Protocol: JSON text frames over ws://<host>:8080
 *
 * Server -> Client:
 *   { "type": "led-status",  "data": { "yellowLed": bool, "redLed": bool } }
 *   { "type": "telemetry",   "data": { "junctionTemp": float, "randomFloat": float, "randomInt": int, "uptime": int } }
 *
 * Client -> Server:
 *   { "type": "led-command", "led": "yellow"|"red", "state": bool }
 */

class LabWebWebSocket {
    constructor() {
        this.ws = null;
        this.reconnectTimer = null;
        this.reconnectAttempts = 0;
        this.reconnectDelay = 3000;
        this.isConnected = false;
        this.messageQueue = [];
        this.eventHandlers = new Map();

        this.autoReconnectEnabled = true;

        // Bind methods
        this.connect = this.connect.bind(this);
        this.onOpen = this.onOpen.bind(this);
        this.onMessage = this.onMessage.bind(this);
        this.onClose = this.onClose.bind(this);
        this.onError = this.onError.bind(this);
    }

    /**
     * Initialize WebSocket connection
     */
    connect() {
        try {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            const wsUrl = `${protocol}//${window.location.hostname}:8080`;

            console.log('Connecting to WebSocket:', wsUrl);
            this.emit('connecting', {});

            this.ws = new WebSocket(wsUrl);
            this.ws.onopen = this.onOpen;
            this.ws.onmessage = this.onMessage;
            this.ws.onclose = this.onClose;
            this.ws.onerror = this.onError;

        } catch (error) {
            console.error('WebSocket connection error:', error);
            this.scheduleReconnect();
        }
    }

    /**
     * WebSocket opened
     */
    onOpen(event) {
        console.log('WebSocket connected');
        this.isConnected = true;
        this.reconnectAttempts = 0;

        this.updateConnectionStatus('connected');
        this.flushMessageQueue();
        this.emit('connected', {});
    }

    /**
     * WebSocket message received — route by type
     */
    onMessage(event) {
        try {
            const message = JSON.parse(event.data);

            switch (message.type) {
                case 'led-status':
                    this.emit('led-status', message.data);
                    break;

                case 'telemetry':
                    this.emit('telemetry', message.data);
                    break;

                default:
                    console.log('Unknown message type:', message.type, message);
                    this.emit('message', message);
            }

        } catch (error) {
            console.error('WebSocket JSON parse error:', error);
            console.error('Raw data:', event.data);
        }
    }

    /**
     * WebSocket closed
     */
    onClose(event) {
        console.log('WebSocket closed:', event.code, event.reason);
        this.isConnected = false;
        this.updateConnectionStatus('disconnected');
        this.emit('disconnected', { code: event.code, reason: event.reason });

        if (event.code !== 1000 && this.autoReconnectEnabled) {
            this.scheduleReconnect();
        }
    }

    /**
     * WebSocket error
     */
    onError(error) {
        console.error('WebSocket error:', error);
        this.updateConnectionStatus('error');
    }

    // -------------------------------------------------------------------------
    // LED Commands
    // -------------------------------------------------------------------------

    /**
     * Send LED command
     * @param {string} led  - "yellow" or "red"
     * @param {boolean} state - true = ON, false = OFF
     */
    sendLedCommand(led, state) {
        return this.send({
            type: 'led-command',
            led: led,
            state: state
        });
    }

    // -------------------------------------------------------------------------
    // Generic send / queue
    // -------------------------------------------------------------------------

    /**
     * Send a JSON message (or queue if disconnected)
     */
    send(message) {
        if (this.isConnected && this.ws && this.ws.readyState === WebSocket.OPEN) {
            try {
                this.ws.send(JSON.stringify(message));
                console.log('WS TX:', message);
                return true;
            } catch (error) {
                console.error('Error sending WebSocket message:', error);
                return false;
            }
        } else {
            this.messageQueue.push(message);
            console.log('WebSocket not connected, message queued');
            return false;
        }
    }

    /**
     * Flush queued messages after reconnect
     */
    flushMessageQueue() {
        while (this.messageQueue.length > 0 && this.isConnected) {
            const msg = this.messageQueue.shift();
            this.send(msg);
        }
    }

    // -------------------------------------------------------------------------
    // Reconnection
    // -------------------------------------------------------------------------

    scheduleReconnect() {
        if (this.reconnectTimer) return;

        this.reconnectAttempts++;
        const delay = Math.min(this.reconnectDelay * this.reconnectAttempts, 15000);

        console.log(`Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts})`);

        this.reconnectTimer = setTimeout(() => {
            this.reconnectTimer = null;
            this.connect();
        }, delay);
    }

    // -------------------------------------------------------------------------
    // Connection status UI helper
    // -------------------------------------------------------------------------

    updateConnectionStatus(status) {
        const icons = document.querySelectorAll('.connection-status-icon');
        icons.forEach(icon => {
            icon.classList.remove('connecting', 'connected', 'disconnected', 'error');
            icon.classList.add(status);
        });
    }

    // -------------------------------------------------------------------------
    // Event emitter
    // -------------------------------------------------------------------------

    on(event, handler) {
        if (!this.eventHandlers.has(event)) {
            this.eventHandlers.set(event, []);
        }
        this.eventHandlers.get(event).push(handler);
    }

    off(event, handler) {
        if (this.eventHandlers.has(event)) {
            const handlers = this.eventHandlers.get(event);
            const index = handlers.indexOf(handler);
            if (index > -1) handlers.splice(index, 1);
        }
    }

    emit(event, data) {
        if (this.eventHandlers.has(event)) {
            this.eventHandlers.get(event).forEach(handler => {
                try {
                    handler(data);
                } catch (error) {
                    console.error(`Error in event handler for ${event}:`, error);
                }
            });
        }
    }

    // -------------------------------------------------------------------------
    // Disconnect / status
    // -------------------------------------------------------------------------

    disconnect() {
        if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
        if (this.ws) {
            this.ws.close(1000, 'Client disconnect');
            this.ws = null;
        }
        this.isConnected = false;
        this.updateConnectionStatus('disconnected');
    }

    getStatus() {
        return {
            connected: this.isConnected,
            readyState: this.ws ? this.ws.readyState : WebSocket.CLOSED,
            reconnectAttempts: this.reconnectAttempts
        };
    }
}

// Export global instance
window.LabWebWebSocket = new LabWebWebSocket();
