# NUCLEO-MEM — Memory Expansion Shield for STM32 NUCLEO-144

> *"Hardware is easy. It's the software that's hard."* — Elon Musk (We beg to differ.)

A custom expansion shield that adds **OPI PSRAM**, **eMMC storage**, and an **OLED display** to ST NUCLEO-144 development boards — paired with a complete Azure RTOS firmware stack featuring network servers, real-time WebSocket telemetry, and a live web dashboard.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/MCU-STM32H723ZG-03234B.svg)
![RTOS](https://img.shields.io/badge/RTOS-Azure%20RTOS%20(ThreadX)-blue.svg)
![IDE](https://img.shields.io/badge/IDE-Visual%20Studio%20%2B%20VisualGDB-5C2D91.svg)

<p align="center">
  <img src="pictures/pic_03_cover.jpg" alt="NUCLEO-MEM shield mounted on NUCLEO-H723ZG" width="800">
</p>

## Features

### Memory Subsystem
- **OPI PSRAM** — APS6408L-3OBM 8 MB Octal SPI with DLYB calibration, dual MPU regions for cached (Write-Back) and non-cached (DMA-safe) operation
- **eMMC Storage** — 8-bit SDMMC with FileX FAT filesystem for persistent file storage and web content serving

### Network & Communication
- **HTTP Server** — Static web dashboard served from eMMC on port 80
- **FTP Server** — Remote file access on port 21 for uploading web content and configuration
- **WebSocket Server** — Real-time bidirectional JSON communication on port 8080

### User Interface
- **Web Dashboard** — Bootstrap-based single-page application with live telemetry display, LED control, and WebSocket debug log
- **OLED Display** — 128×32 SSD1306 driven by a dedicated ThreadX thread with message queue for status, IP address, and network state

### Firmware
- **Azure RTOS (ThreadX)** — Preemptive RTOS with multiple application threads, byte/block pools in external PSRAM
- **Persistent Settings** — Network configuration stored in internal flash (FW_DATA sector) surviving power cycles
- **Telemetry Streaming** — MCU junction temperature (ADC3), hardware RNG values, and RTOS uptime broadcast at ~1 Hz
- **Debug UART** — printf output on USART3 via ST-Link VCP

## Hardware

| Component | Part | Interface | Notes |
|-----------|------|-----------|-------|
| MCU Board | [NUCLEO-H723ZG](https://www.st.com/en/evaluation-tools/nucleo-h723zg.html) | — | Cortex-M7, 550 MHz, 1 MB Flash |
| Shield | NUCLEO-MEM (custom) | NUCLEO-144 pinout | Designed by [Intector](https://intector.com) |
| PSRAM | AP Memory APS6408L-3OBM | OCTOSPI1 | 8 MB OPI, 3.3V variant |
| eMMC | EMMCxxG-MV28 | SDMMC1 (8-bit) | Mass storage for web content |
| OLED Display | SSD1306 128×32 | I2C2 | Status display |
| Ethernet PHY | LAN8742A | RMII | 100 Mbit (on NUCLEO board) |

<p align="center">
  <img src="pictures/pic_01.jpg" alt="NUCLEO-MEM shield — front and back" width="700">
</p>

Shield schematics are available in the [`hardware/`](hardware/) folder.

## Software Architecture

### RTOS Threads

| Thread | Stack | Priority | Purpose |
|--------|-------|----------|---------|
| Main | 2 KB | Normal | Core app logic, green LED heartbeat |
| OLED | 1 KB | Normal | SSD1306 display updates via message queue |
| Telemetry | 4 KB | Normal | WebSocket telemetry streaming (~1 Hz) |
| NetXDuo App | 2–4 KB | Normal | HTTP / FTP / WebSocket servers |
| ETH Link Check | — | — | Ethernet link status monitor |

### Middleware Stack

```
┌─────────────────────────────────────────────────────────┐
│                   Application Threads                   │
├──────────┬──────────┬────────────┬──────────┬───────────┤
│   HTTP   │   FTP    │  WebSocket │ Telemetry│   OLED    │
│  :80     │  :21     │   :8080    │  ~1 Hz   │  I2C2     │
├──────────┴──────────┴────────────┴──────────┴───────────┤
│              NetXDuo  (TCP/IP + BSD)                    │
├────────────────────────┬────────────────────────────────┤
│     FileX (FAT/eMMC)  │           cJSON                 │
├────────────────────────┴────────────────────────────────┤
│                 ThreadX  (Azure RTOS)                   │
├─────────────────────────────────────────────────────────┤
│                  STM32 HAL / BSP                        │
├──────────┬──────────┬────────────┬──────────┬───────────┤
│ OCTOSPI  │  SDMMC   │    ETH     │   I2C    │  USART    │
│ (PSRAM)  │  (eMMC)  │  (LAN8742) │ (OLED)   │  (Debug)  │
└──────────┴──────────┴────────────┴──────────┴───────────┘
```

### Memory Map

| Region | Address | Size | Usage |
|--------|---------|------|-------|
| FLASH | `0x08000000` | 896 KB | Application code |
| FW_DATA | `0x080E0000` | 128 KB | Persistent firmware settings |
| DTCMRAM | `0x20000000` | 128 KB | Stack |
| AXI SRAM | `0x24000000` | 320 KB | Ethernet DMA descriptors/buffers, general RAM |
| Ext. PSRAM (cached) | `0x90000000` | 4 MB | ThreadX byte/block pools, FileX, NetXDuo buffers |
| Ext. PSRAM (uncached) | `0x90400000` | 4 MB | DMA-safe region (no cache coherency issues) |

## Web Dashboard

The `Lab-Page-Demo/` directory contains a Bootstrap single-page application served by the on-board HTTP server. It connects to the WebSocket server on port 8080 for real-time interaction.

<p align="center">
  <img src="pictures/Web-Interface-1.png" alt="Web dashboard — LED control and live telemetry" width="900">
</p>

### WebSocket API (port 8080)

**Server → Client:**

| Message | Description |
|---------|-------------|
| `led-status` | Current yellow/red LED state (sent on connect and after each command) |
| `telemetry` | Junction temp, random float/int, uptime — sent every ~1 s |

**Client → Server:**

| Message | Description |
|---------|-------------|
| `led-command` | Toggle yellow or red LED: `{"type":"led-command","led":"yellow","state":true}` |

Full protocol details: [`ExtMem/WebSocket/WEBSOCKET_API.md`](ExtMem/WebSocket/WEBSOCKET_API.md)

## Getting Started

### Prerequisites

- **Visual Studio** (2019 or later) with C++ workload
- **[VisualGDB](https://visualgdb.com/)** plugin — provides the ARM BSP, CMake integration, and ST-Link debugging
- **ARM GCC toolchain** — installed automatically by VisualGDB (`arm-none-eabi-gcc`)
- ST-Link debugger (built into the NUCLEO board)

### Build & Flash

```bash
# Clone the repository
git clone https://github.com/intector/NUCLEO-MEM.git
```

1. Open `NUCLEO-H723ZG.sln` in Visual Studio.
2. VisualGDB will resolve the BSP and toolchain automatically.
3. Build the project (**Build → Build Solution** or `Ctrl+Shift+B`).
4. Flash via the built-in ST-Link (**Debug → Start Debugging** or `F5`).

### Default Network Configuration

| Setting | Value |
|---------|-------|
| IP Address | `192.168.0.50` |
| Subnet Mask | `255.255.255.0` |
| Gateway | `192.168.0.1` |

Connect the NUCLEO board to your network and navigate to `http://192.168.0.50` for the web dashboard.

> Network settings are stored in internal flash and persist across power cycles. Modify via `fw_settings.c`.

## OLED Status Display

The shield includes an SSD1306 128×32 OLED that shows real-time system status including network state, IP address, and Ethernet link information — driven by a dedicated RTOS thread.

<p align="center">
  <img src="pictures/pic_06.jpg" alt="NUCLEO-MEM running with OLED status display" width="500">
</p>

## Technical Details

### PSRAM Configuration
- **Chip:** APS6408L-3OBM (3.3V variant — critical for STM32H723 GPIO compatibility)
- **Interface:** OCTOSPI1, memory-mapped mode
- **Clock:** 25 MHz (signal integrity limited by pin header interconnect)
- **Calibration:** DLYB with 3-tap passing window (SEL 2–4, centered at tap 3)
- **MPU Regions:** Cached (Write-Back, Write-Allocate) + Non-cached (DMA-safe)

### Timing & Performance
- Telemetry broadcast rate: ~1 Hz over WebSocket
- OLED update: message-queue driven, on-demand
- Ethernet: 100 Mbit via LAN8742A PHY (RMII)
- eMMC: 8-bit SDMMC1 bus width

### Build System
- **CMake** with explicit file listing (no globbing)
- **Linker script:** `STM32H723ZGTX_FLASH.ld` — custom memory regions for PSRAM cached/uncached split
- **ThreadX port:** Cortex-M7 GNU assembly (`tx_initialize_low_level.S`)

## Project Structure

```
NUCLEO-MEM/
├── ExtMem/                        # Firmware project
│   ├── AZURE_RTOS/App/            # ThreadX application entry and config
│   ├── Core/
│   │   ├── Inc/                   # Headers (main.h, psram.h, octospi.h, eth.h, ...)
│   │   ├── Src/                   # Sources (main.c, psram.c, octospi.c, eth.c, ...)
│   │   └── Startup/               # MCU startup code
│   ├── cjson/                     # cJSON library
│   ├── display/                   # SSD1306/SSD1316 OLED drivers, fonts, display API
│   ├── Drivers/                   # CMSIS, STM32H7 HAL, BSP (LAN8742)
│   ├── FileX/                     # FileX application layer and eMMC driver glue
│   ├── int_flash/                 # Internal flash read/write for persistent settings
│   ├── Middlewares/ST/            # ThreadX, NetXDuo, FileX source (Cortex-M7 port)
│   ├── NetXDuo/                   # NetXDuo TCP/IP application layer
│   ├── ssd1306/                   # SSD1306 standalone driver
│   ├── tools/                     # Firmware settings (fw_settings.c/h)
│   ├── WebSocket/                 # WebSocket server, commands, telemetry, API docs
│   ├── CMakeLists.txt             # Build configuration
│   └── STM32H723ZGTX_FLASH.ld    # Linker script (PSRAM regions)
├── Lab-Page-Demo/                 # Web dashboard (served from eMMC)
│   ├── index.html                 # Single-page application
│   ├── assets/                    # Custom CSS, fonts, images, JS
│   ├── css/                       # Bootstrap CSS (Slate theme)
│   └── js/                        # jQuery, Bootstrap JS
├── hardware/                      # Shield schematics (PDF)
├── pictures/                      # Project photos
├── LICENSE
├── NUCLEO-H723ZG.sln              # Visual Studio solution
└── README.md
```

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the [MIT License](LICENSE).

Copyright (c) 2025 [Intector Inc.](https://intector.com)

---

*Built with caffeine and a logic analyzer*
