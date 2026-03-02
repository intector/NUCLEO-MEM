/**
  ****************************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "stm32h7xx_hal.h"
#include "err_codes.h"
#include "io_pin_def.h"
#include "tx_api.h"
#include "tx_port.h"
#include "fw_settings.h"

// The likely and unlikely macro pairs:
// These macros are useful to place when application
// knows the majority ocurrence of a decision paths,
// placing one of these macros can hint the compiler
// to reorder instructions producing more optimized
// code.

#if (CONFIG_COMPILER_OPTIMIZATION_PERF)
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif
#else
#ifndef likely
#define likely(x) (x)
#endif
#ifndef unlikely
#define unlikely(x) (x)
#endif
#endif

extern FlagStatus BoardInit;

// definitions for user button
#define BTN_DEBOUNCE_TIME 50
extern FlagStatus ButtonPressed;
extern FlagStatus PWR_BOOST_DIS;

// type defienitions ---------------------------------------------------------------------
typedef struct FWS_NETWORK_STRUCT
{
    ULONG ip_addr;
    ULONG nw_mask;
    ULONG gw_addr;
    ULONG http_port;
    ULONG ftp_port;
} FWS_NETWORK;

typedef LONG64 STM_ERR_T;

extern FWS_NETWORK NetworkSettings;

// functions prototypes ------------------------------------------------------------------
void smart_delay(uint32_t ms);
void Error_Handler(STM_ERR_T ERR_ID, STM_ERR_T ERR_CODE);
void Show_OLED_Message(char *msg_1, char *msg_2, uint32_t msg_time);
void Show_Intector_Logo(void);
void Show_TagID_Logo(void);
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);

// macro definitions ---------------------------------------------------------------------
#define STM32_ERROR_CHECK(err_id, x)         \
    do {                                     \
        STM_ERR_T err_rc_ = (x);             \
        if (unlikely(err_rc_ != STM32_OK)) { \
            Error_Handler(err_id, err_rc_);  \
        }                                    \
    } while (0)

// Define the macro for building IPv4 addresses.  */
#define IP_ADDR(a, b, c, d) ((((ULONG)a) << 24) | (((ULONG)b) << 16) | (((ULONG)c) << 8) | ((ULONG)d))

// hardware options enable flags ---------------------------------------------------------
#define HW_USE_OLED
#define HW_USE_EMMCxxG_MV28_BOARD
#define HW_USE_TAGID_EXTENSION_BOARD

// program options enable flags ----------------------------------------------------------

#define TX_THREAD_ENABLE 1
#define TX_QUEUE_ENABLE 1
#define TX_SEMAPHORE_ENABLE 0
#define TX_MUTEX_ENABLE 0

#define FX_THREAD_ENABLE 1

#define NX_THREAD_ENABLE      1
#define NX_LINK_CHECK_ENABLE  1
#define NX_HTTP_SERVER_ENABLE 1
#define NX_FTP_SERVER_ENABLE  1
#define NX_WS_SERVER_ENABLE   1
#define NX_MDNS_ENABLE        1
#define NX_LLMNR_ENABLE       1

#define NX_HOSTNAME           "lab-web"

extern TX_SEMAPHORE tx_app_semaphore;
extern TX_MUTEX tx_app_mutex;
extern TX_QUEUE tx_OLED_msg_queue;

enum H_ALIGNMENT
{
    LEFT,
    CENTER,
    RIGHT
};

enum V_ALIGNMENT
{
    TOP,
    MIDDLE,
    BOTTOM
};

typedef struct
{
    uint8_t X; // X position
    uint8_t Y; // Y position
} V_POINT;

typedef struct
{
    V_POINT Start; // start position
    V_POINT End;   // end position
} V_RECT;

extern const V_RECT LT_S;
extern const V_RECT LM_S;
extern const V_RECT MT_S;
extern const V_RECT RT_S;
extern const V_RECT RM_S;
extern const V_RECT BL_S;

#define MAX_HEX_STR 4
#define MAX_HEX_STR_LENGTH 128

char *hex2Str(unsigned char *data, size_t dataLen);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H
