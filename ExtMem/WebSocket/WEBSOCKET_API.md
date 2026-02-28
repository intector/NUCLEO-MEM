# WebSocket API — Demo LED Control & Telemetry

## Connection

- **Port:** 8080 (dedicated TCP server, separate from the HTTP server on port 80)
- **Protocol:** Standard WebSocket (RFC 6455)
- **URL:** `ws://<board-ip>:8080`
- **Default board IP:** `192.168.0.50`
- **Max clients:** 1 active connection (single-client design)

## Message Format

All messages are **JSON text frames**. Both directions use this envelope:

```json
{"type": "<message-type>", ...}
```

---

## Server → Client Messages

### `led-status`

Sent automatically on client connect and after every LED command.

```json
{
  "type": "led-status",
  "data": {
    "yellowLed": false,
    "redLed": false
  }
}
```

| Field              | Type | Description                        |
|--------------------|------|------------------------------------|
| `data.yellowLed`   | bool | Current state of the yellow LED    |
| `data.redLed`      | bool | Current state of the red LED       |

### `telemetry`

Sent every ~1 second while a client is connected.

```json
{
  "type": "telemetry",
  "data": {
    "junctionTemp": 42.0,
    "randomFloat": 73.21,
    "randomInt": 456,
    "uptime": 12345
  }
}
```

| Field               | Type   | Range      | Description                                  |
|---------------------|--------|------------|----------------------------------------------|
| `data.junctionTemp` | float  | varies     | MCU internal junction temperature (°C)       |
| `data.randomFloat`  | float  | 0 – 99.99  | Random float (HW RNG), changes ~25% of cycles |
| `data.randomInt`    | int    | 0 – 1000   | Random integer (HW RNG), changes ~25% of cycles |
| `data.uptime`       | int    | 0+         | Seconds since RTOS started                   |

---

## Client → Server Messages

### `led-command`

Control the yellow or red LED on the NUCLEO board. The green LED is reserved for the heartbeat.

```json
{
  "type": "led-command",
  "led": "yellow",
  "state": true
}
```

| Field   | Type   | Values               | Description               |
|---------|--------|----------------------|---------------------------|
| `led`   | string | `"yellow"`, `"red"`  | Which LED to control      |
| `state` | bool   | `true` / `false`     | `true` = ON, `false` = OFF |

Server responds with a `led-status` message after each command.

---

## Quick Test

Open a browser console while connected to the board's network:

```javascript
const ws = new WebSocket('ws://192.168.0.50:8080');

ws.onmessage = (e) => console.log(JSON.parse(e.data));

// Turn on yellow LED
ws.send(JSON.stringify({type: 'led-command', led: 'yellow', state: true}));

// Turn off yellow LED
ws.send(JSON.stringify({type: 'led-command', led: 'yellow', state: false}));
```
