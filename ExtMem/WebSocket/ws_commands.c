/**
  ****************************************************************************************
 * @file           : ws_commands.c
 * @brief          : WebSocket JSON command handlers for demo LED control
  ****************************************************************************************
 *
  * Copyright (c) 2020-2026 Intector Inc.
 * All rights reserved.
 *
  ****************************************************************************************
 */

#include "ws_commands.h"
#include "ws_server.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

extern WS_Server_t WebSocket_Server;

// LED state tracking
static bool led_yellow_on = false;
static bool led_red_on    = false;

static void handle_led_command(cJSON *root);

void WS_HandleJSONCommand(const char *json_string)
{
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL) {
        printf("WS: Failed to parse JSON command\r\n");
        return;
    }

    cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");
    if (cJSON_IsString(type)) {
        if (strcmp(type->valuestring, "led-command") == 0) {
            handle_led_command(root);
        }
        else {
            printf("WS: Unknown command type: %s\r\n", type->valuestring);
        }
    }

    cJSON_Delete(root);
}

static void handle_led_command(cJSON *root)
{
    cJSON *led   = cJSON_GetObjectItemCaseSensitive(root, "led");
    cJSON *state = cJSON_GetObjectItemCaseSensitive(root, "state");

    if (!cJSON_IsString(led) || !cJSON_IsBool(state)) {
        printf("WS: Invalid led-command format\r\n");
        return;
    }

    GPIO_PinState pin_state = cJSON_IsTrue(state) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    if (strcmp(led->valuestring, "yellow") == 0) {
        HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, pin_state);
        led_yellow_on = cJSON_IsTrue(state);
        printf("WS: Yellow LED %s\r\n", led_yellow_on ? "ON" : "OFF");
    }
    else if (strcmp(led->valuestring, "red") == 0) {
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, pin_state);
        led_red_on = cJSON_IsTrue(state);
        printf("WS: Red LED %s\r\n", led_red_on ? "ON" : "OFF");
    }
    else {
        printf("WS: Unknown LED: %s\r\n", led->valuestring);
        return;
    }

    WS_SendLedStatus();
}

void WS_SendLedStatus(void)
{
    cJSON *msg  = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();

    cJSON_AddBoolToObject(data, "yellowLed", led_yellow_on);
    cJSON_AddBoolToObject(data, "redLed", led_red_on);

    cJSON_AddStringToObject(msg, "type", "led-status");
    cJSON_AddItemToObject(msg, "data", data);

    char *json_string = cJSON_PrintUnformatted(msg);
    WS_Server_SendJSON(&WebSocket_Server, json_string);

    cJSON_free(json_string);
    cJSON_Delete(msg);
}
