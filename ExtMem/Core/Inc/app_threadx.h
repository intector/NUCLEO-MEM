/**
  ****************************************************************************************
  * @file           : app_threadx.h
  * @brief          : ThreadX applicative header file
  ****************************************************************************************
  *
  * Copyright (c) 2020-2026 Intector Inc.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ****************************************************************************************
  */

// Define to prevent recursive inclusion -------------------------------------------------
#ifndef __APP_THREADX_H__
#define __APP_THREADX_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "tx_api.h"
#include "main.h"
#include "app_azure_rtos.h"

// Private defines -----------------------------------------------------------------------
#define TX_APP_SINGLE_MSG_SIZE                  4
#define TX_APP_MSG_QUEUE_NB_MSG                 10
#define TX_APP_MSG_QUEUE_FULL_SIZE              TX_APP_SINGLE_MSG_SIZE * TX_APP_MSG_QUEUE_NB_MSG

#define TX_APP_STACK_SIZE                       1024 * 2
#define TX_APP_THREAD_PRIO                      10

#define TX_OLED_SINGLE_MSG_SIZE                 5
#define TX_OLED_SINGLE_MSG_SIZE_BYTES           TX_OLED_SINGLE_MSG_SIZE * sizeof(UINT)
#define TX_OLED_MSG_QUEUE_NB_MSG                5
#define TX_OLED_MSG_QUEUE_FULL_SIZE             TX_OLED_SINGLE_MSG_SIZE * TX_OLED_MSG_QUEUE_NB_MSG

#define TX_OLED_STACK_SIZE                      1024
#define TX_OLED_THREAD_PRIO                     10

#define TX_OLED_CMD_UPDATE_ALL                  0xFF
#define TX_OLED_CMD_CLEAR                       0x01
#define TX_OLED_CMD_UPDATE_LTS                  0x10
#define TX_OLED_CMD_UPDATE_LMS                  0x20
#define TX_OLED_CMD_UPDATE_RTS                  0x30
#define TX_OLED_CMD_UPDATE_RMS                  0x40
#define TX_OLED_CMD_UPDATE_BLS                  0x50
// #define TX_OLED_CMD_UPDATE_FRM                  0xF0

// ---------------------------------------------------------------------------------------
// Telemetry thread configuration
// ---------------------------------------------------------------------------------------
// #define TX_TELEMETRY_STACK_SIZE  1024
// #define TX_TELEMETRY_THREAD_PRIO 15 // Lower priority than control

// Main thread defines -------------------------------------------------------------------
#ifndef TX_APP_THREAD_PREEMPTION_THRESHOLD
#define TX_APP_THREAD_PREEMPTION_THRESHOLD      TX_APP_THREAD_PRIO
#endif
#ifndef TX_APP_THREAD_TIME_SLICE
#define TX_APP_THREAD_TIME_SLICE                TX_NO_TIME_SLICE
#endif
#ifndef TX_APP_THREAD_AUTO_START
#define TX_APP_THREAD_AUTO_START                TX_AUTO_START
#endif

// Exported functions prototypes ---------------------------------------------------------
UINT App_ThreadX_Init(VOID *memory_ptr);
void MX_ThreadX_Init(void);
	
#ifdef __cplusplus
}
#endif
#endif // __APP_THREADX_H__
