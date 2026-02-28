/**
  ****************************************************************************************
 * @file           : ws_commands.h
 * @brief          : WebSocket JSON command handlers for demo LED control
  ****************************************************************************************
 *
  * Copyright (c) 2020-2026 Intector Inc.
 * All rights reserved.
 *
  ****************************************************************************************
 */

#ifndef WS_COMMANDS_H
#define WS_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Command handler functions
void WS_HandleJSONCommand(const char *json_string);
void WS_SendLedStatus(void);

#ifdef __cplusplus
}
#endif

#endif // WS_COMMANDS_H
