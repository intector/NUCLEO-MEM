/**
  ****************************************************************************************
  * @file           : uart_debug.h
  * @brief          : uart_debug.h - USART3 debug printf via ST-Link VCP
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

// uart_debug.h - USART3 debug printf via ST-Link VCP
// NUCLEO-H723ZG: PD8 = USART3_TX, PD9 = USART3_RX (routed to ST-Link)

#ifndef __UART_DEBUG_H__
#define __UART_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

extern UART_HandleTypeDef huart3;

void MX_USART3_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif // __UART_DEBUG_H__
