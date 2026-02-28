/**
  ****************************************************************************************
 * @file           : ws_telemetry.c
 * @brief          : Demo telemetry thread — junction temperature + random values
  ****************************************************************************************
 *
  * Copyright (c) 2020-2026 Intector Inc.
 * All rights reserved.
 *
  ****************************************************************************************
 */

#include "ws_telemetry.h"
#include "app_azure_rtos.h"
#include "app_netxduo.h"
#include "ws_server.h"
#include "dts.h"
#include "rng.h"
#include "cJSON.h"
#include <stdio.h>

extern WS_Server_t WebSocket_Server;

// Thread handle
TX_THREAD tx_tp_TELEMETRY;

void tx_telemetry_thread_entry(ULONG thread_input)
{
    UNUSED(thread_input);
    ULONG tmp_actual_events = 0x00000000UL;
    uint32_t random_value   = 0;
    float random_float      = 0.0f;
    uint32_t random_int     = 0;

    // Wait for HTTP server to be ready
    tx_event_flags_get(&TAGID_status_event_group,
                       TAGID_SE_HTTP_SERVER_OK,
                       TX_AND,
                       &tmp_actual_events,
                       TX_WAIT_FOREVER);

    printf("Telemetry thread started (demo mode)\r\n");

    // Main telemetry loop
    while (1) {
        // Read junction temperature from DTS
        float junction_temp = DTS_ReadTemperature();

        // Update random values with random chance each cycle
        HAL_RNG_GenerateRandomNumber(&hrng, &random_value);
        if ((random_value & 0x03) == 0) { // ~25% chance to change
            HAL_RNG_GenerateRandomNumber(&hrng, &random_value);
            random_float = (float)(random_value % 10000) / 100.0f; // 0.00 - 99.99
        }

        HAL_RNG_GenerateRandomNumber(&hrng, &random_value);
        if ((random_value & 0x03) == 0) { // ~25% chance to change
            HAL_RNG_GenerateRandomNumber(&hrng, &random_value);
            random_int = random_value % 1001; // 0 - 1000
        }

        // Uptime in seconds
        ULONG uptime = tx_time_get() / TX_TIMER_TICKS_PER_SECOND;

        // Build JSON telemetry message
        cJSON *msg  = cJSON_CreateObject();
        cJSON *data = cJSON_CreateObject();

        cJSON_AddNumberToObject(data, "junctionTemp", junction_temp);
        cJSON_AddNumberToObject(data, "randomFloat", random_float);
        cJSON_AddNumberToObject(data, "randomInt", (double)random_int);
        cJSON_AddNumberToObject(data, "uptime", (double)uptime);

        cJSON_AddStringToObject(msg, "type", "telemetry");
        cJSON_AddItemToObject(msg, "data", data);

        char *json_string = cJSON_PrintUnformatted(msg);
        WS_Server_SendJSON(&WebSocket_Server, json_string);

        cJSON_free(json_string);
        cJSON_Delete(msg);

        // Sleep ~1 second
        tx_thread_sleep(TELEMETRY_UPDATE_TICKS);
    }
}
