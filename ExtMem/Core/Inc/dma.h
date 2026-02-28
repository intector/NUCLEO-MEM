/**
  ****************************************************************************************
 * @file           : dma.h
 * @brief          : This file contains all the function prototypes for the dma.c file
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
#ifndef __DMA_H__
#define __DMA_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "main.h"

// Exported variables --------------------------------------------------------------------

// DMA handles for ADC peripherals
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_adc3;

// Exported functions --------------------------------------------------------------------

void MX_DMA_Init(void);

#ifdef __cplusplus
}
#endif

#endif // __DMA_H__
