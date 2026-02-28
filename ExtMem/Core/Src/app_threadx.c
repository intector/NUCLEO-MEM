/**
  ****************************************************************************************
  * @file           : app_threadx.c
  * @brief          : ThreadX applicative file
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

// Includes ------------------------------------------------------------------------------
#include "app_threadx.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ws_telemetry.h"

// Private variables ---------------------------------------------------------------------
TX_THREAD tx_tp_MAIN;
TX_THREAD tx_tp_OLED;
TX_SEMAPHORE tx_app_semaphore;
TX_MUTEX tx_app_mutex;
TX_QUEUE tx_OLED_msg_queue;
TX_TIMER test;

// Private function prototypes -----------------------------------------------------------
void tx_app_main_thread(ULONG thread_input);
void tx_app_OLED_thread(ULONG thread_input);
void Update_OLED(UCHAR cmd, char *text);

UINT App_ThreadX_Init(VOID *memory_ptr)
{
    UINT ret = TX_SUCCESS;
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
    CHAR *pointer;

#if TX_THREAD_ENABLE
    // ---------- setup first thread ----------
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID**) &pointer, TX_APP_STACK_SIZE, TX_NO_WAIT));

    STM32_ERROR_CHECK(ERR_Tx_ThreadCreate, tx_thread_create(&tx_tp_MAIN,
                                       "tx app main thread",
                                       tx_app_main_thread,
                                       0,
                                       pointer,
                                       TX_APP_STACK_SIZE,
                                       TX_APP_THREAD_PRIO,
                                       TX_APP_THREAD_PREEMPTION_THRESHOLD,
                                       TX_APP_THREAD_TIME_SLICE,
                                       TX_DONT_START));

    // ---------- setup OLED thread ----------
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID**) &pointer, TX_OLED_STACK_SIZE, TX_NO_WAIT));

    STM32_ERROR_CHECK(ERR_Tx_ThreadCreate, tx_thread_create(&tx_tp_OLED,
                                       "tx app OLED thread",
                                       tx_app_OLED_thread,
                                       0,
                                       pointer,
                                       TX_OLED_STACK_SIZE,
                                       TX_OLED_THREAD_PRIO,
                                       TX_APP_THREAD_PREEMPTION_THRESHOLD,
                                       TX_APP_THREAD_TIME_SLICE,
                                       TX_AUTO_START));

    // =========================================================================
    // NEW: Setup telemetry thread for WebSocket data streaming
    // =========================================================================
#if NX_WS_SERVER_ENABLE
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, TX_TELEMETRY_STACK_SIZE, TX_NO_WAIT));

    STM32_ERROR_CHECK(ERR_Tx_ThreadCreate, tx_thread_create(&tx_tp_TELEMETRY,
                                       "Telemetry Thread",
                                       tx_telemetry_thread_entry,
                                       0,
                                       pointer,
                                       TX_TELEMETRY_STACK_SIZE,
                                       TX_TELEMETRY_THREAD_PRIO,
                                       TX_TELEMETRY_THREAD_PRIO, // No preemption threshold
                                       TX_NO_TIME_SLICE,
                                       TX_AUTO_START)); // Auto-start telemetry
#endif    

#endif // THREAD_THREAD_ENABLE

#if TX_QUEUE_ENABLE

    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **) &pointer, TX_OLED_MSG_QUEUE_FULL_SIZE * sizeof(ULONG), TX_NO_WAIT));

    STM32_ERROR_CHECK(ERR_Tx_QueueCreate, tx_queue_create(&tx_OLED_msg_queue,
        "OLED queue",
        TX_OLED_SINGLE_MSG_SIZE,
        pointer,
        TX_OLED_MSG_QUEUE_FULL_SIZE * sizeof(ULONG)));

#endif // THREADX_QUEUE_ENABLE

#if TX_SEMAPHORE_ENABLE
    STM32_ERROR_CHECK(ERR_Tx_SemaphoreCreate, tx_semaphore_create(&tx_app_semaphore, "tx app semaphore", 0));
#endif // THREADX_SEMAPHORE_ENABLE

#if TX_MUTEX_ENABLE
    STM32_ERROR_CHECK(ERR_Tx_MutexCreate, tx_mutex_create(&tx_app_mutex, "tx app mutex", TX_NO_INHERIT));
#endif // THREADX_MUTEX_ENABLE
    
    // ---------- set Main-Thread init OK event flag ----------
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_App_ThreadX_Init_OK, TX_OR));

    return ret;
}

void tx_app_main_thread(ULONG thread_input)
{
    UNUSED(thread_input); // avoid gcc/g++ warnings
    ULONG _EventFlags = 0x00000000UL;
    ULONG tmp_actual_events = 0x00000000UL;

    _EventFlags = TAGID_SE_tx_app_OLED_thread;

    // ---------- wait for TAGID_SE_tx_app_OLED_thread event flag ----------
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsGet, tx_event_flags_get(&TAGID_status_event_group, _EventFlags, TX_OR, &tmp_actual_events, TX_WAIT_FOREVER));

    // OLED queue message buffer
    char _OLED_message[TX_OLED_SINGLE_MSG_SIZE_BYTES] = {0};
    char _OLED_text[TX_OLED_SINGLE_MSG_SIZE_BYTES - 2] = {0};

    // clear OLED screen
    memset(_OLED_message, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);
    _OLED_message[0] = TX_OLED_CMD_CLEAR;
    strcpy(_OLED_text, "clear screen");
    memcpy(&_OLED_message[1], _OLED_text, TX_OLED_SINGLE_MSG_SIZE_BYTES - 2);
    STM32_ERROR_CHECK(ERR_Tx_QueueSend, tx_queue_send(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));

    // OLED screen text Left-Top-Section
    memset(_OLED_message, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);
    _OLED_message[0] = TX_OLED_CMD_UPDATE_LTS;
    strcpy(_OLED_text, "WPT-State");
    memcpy(&_OLED_message[1], _OLED_text, TX_OLED_SINGLE_MSG_SIZE_BYTES - 2);
    STM32_ERROR_CHECK(ERR_Tx_QueueSend, tx_queue_send(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));

    // OLED screen text Left-Middle-Section
    memset(_OLED_message, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);
    _OLED_message[0] = TX_OLED_CMD_UPDATE_LMS;
    strcpy(_OLED_text, "disabled");
    memcpy(&_OLED_message[1], _OLED_text, TX_OLED_SINGLE_MSG_SIZE_BYTES - 2);
    STM32_ERROR_CHECK(ERR_Tx_QueueSend, tx_queue_send(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));
    
    while (1)
    {
        // ---------- set TAGID_SE_tx_app_main_thread event flag ----------
        STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_tx_app_main_thread, TX_OR));

        HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_SET);
        tx_thread_sleep(10);
        HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_RESET);
        tx_thread_sleep(500);
    }
}

void tx_app_OLED_thread(ULONG thread_input)
{
    UNUSED(thread_input); // avoid gcc/g++ warnings
    UINT    status = 0x0000;
    UCHAR   _OLED_cmd_index = 0x00;
    char    _OLED_text[TX_OLED_SINGLE_MSG_SIZE_BYTES] = { 0x00 };
    // OLED queue buffer
    char _OLED_message[TX_OLED_SINGLE_MSG_SIZE_BYTES] = { 0 };

    tx_thread_resume(&tx_tp_MAIN);

    while (1)
    {
        // ---------- set TAGID_SE_tx_app_OLED_thread event flag ----------
        STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_tx_app_OLED_thread, TX_OR));

        // ---------- wait for new OLED message in queue ----------
        STM32_ERROR_CHECK(ERR_Tx_QueueReceive, tx_queue_receive(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));

        // extracting command byte
        _OLED_cmd_index = _OLED_message[0];
        memcpy(_OLED_text, &_OLED_message[1], strlen(_OLED_message) - 1);
        Update_OLED(_OLED_cmd_index, _OLED_text);
        memset(_OLED_text, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);

        tx_thread_sleep(5);
    }
}

void Update_OLED(UCHAR cmd, char *text)
{
    char IP_Str[40];
    uint8_t IP_A, IP_B, IP_C, IP_D;
    uint8_t Mask_A, Mask_B, Mask_C, Mask_D;

    switch (cmd) {
        case TX_OLED_CMD_CLEAR:
            ssd1306_Fill(Black);
            break;
        case TX_OLED_CMD_UPDATE_BLS:
            IP_A = (NetworkSettings.ip_addr & 0xFF000000) >> 24;
            IP_B = (NetworkSettings.ip_addr & 0x00FF0000) >> 16;
            IP_C = (NetworkSettings.ip_addr & 0x0000FF00) >> 8;
            IP_D = (NetworkSettings.ip_addr & 0x000000FF);
            Mask_A = (NetworkSettings.nw_mask & 0xFF000000) >> 24;
            Mask_B = (NetworkSettings.nw_mask & 0x00FF0000) >> 16;
            Mask_C = (NetworkSettings.nw_mask & 0x0000FF00) >> 8;
            Mask_D = (NetworkSettings.nw_mask & 0x000000FF);
            sprintf(IP_Str, "IP: %u.%u.%u.%u / %u.%u.%u.%u", IP_A, IP_B, IP_C, IP_D, Mask_A, Mask_B, Mask_C, Mask_D);

            // IP-Address info
            ssd1306_FillRect_B(BL_S, Black);
            ssd1306_WriteString_Aligned(BL_S, CENTER, IP_Str, font_ubuntuCondensed_8pt, White);
            ssd1306_UpdateScreen();
            break;
        case TX_OLED_CMD_UPDATE_LTS:
            // left top
            ssd1306_FillRect_B(LT_S, Black);
            ssd1306_WriteString_Aligned(LT_S, LEFT, text, font_ubuntu_8pt, White);
            ssd1306_UpdateScreen();
            break;
        case TX_OLED_CMD_UPDATE_LMS:
            // left middle
            ssd1306_FillRect_B(LM_S, Black);
            ssd1306_WriteString_Aligned(LM_S, LEFT, text, font_ubuntuCondensed_8pt, White);
            ssd1306_UpdateScreen();
            break;
        case TX_OLED_CMD_UPDATE_RTS:
            // right top
            ssd1306_FillRect_B(RT_S, Black);
            ssd1306_WriteString_Aligned(RT_S, RIGHT, text, font_ubuntu_8pt, White);
            ssd1306_UpdateScreen();
            break;
        case TX_OLED_CMD_UPDATE_RMS:
            // right middle
            ssd1306_FillRect_B(RM_S, Black);
            ssd1306_WriteString_Aligned(RM_S, RIGHT, text, font_ubuntuCondensed_8pt, White);
            ssd1306_UpdateScreen();
            break;
        default:
            // do something here later...
            ;
    };

}

void MX_ThreadX_Init(void)
{
    tx_kernel_enter();
}
