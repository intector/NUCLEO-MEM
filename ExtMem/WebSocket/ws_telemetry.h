/**
  ****************************************************************************************
 * @file           : ws_telemetry.h
 * @brief          : Demo telemetry thread for WebSocket data streaming
  ****************************************************************************************
 *
  * Copyright (c) 2020-2026 Intector Inc.
 * All rights reserved.
 *
  ****************************************************************************************
 */

#ifndef WS_TELEMETRY_H
#define WS_TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tx_api.h"

// Thread configuration
#define TX_TELEMETRY_STACK_SIZE  4096
#define TX_TELEMETRY_THREAD_PRIO 15 // Low priority - data reporting only

// Telemetry update rate
#define TELEMETRY_UPDATE_RATE_MS 1000
#define TELEMETRY_UPDATE_TICKS   (TELEMETRY_UPDATE_RATE_MS * TX_TIMER_TICKS_PER_SECOND / 1000)

// Thread handle (external)
extern TX_THREAD tx_tp_TELEMETRY;

// Function prototypes
void tx_telemetry_thread_entry(ULONG thread_input);

#ifdef __cplusplus
}
#endif

#endif // WS_TELEMETRY_H
