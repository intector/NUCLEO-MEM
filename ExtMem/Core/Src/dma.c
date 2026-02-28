/**
  ****************************************************************************************
 * @file           : dma.c
 * @brief          : This file provides code for the configuration of all the
 *                   requested memory to memory DMA transfers and ADC DMAs
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
#include "dma.h"

// Private variables ---------------------------------------------------------------------

// DMA handles for ADC peripherals
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_adc3;

// Function definitions ------------------------------------------------------------------

// Enable DMA controller clock and configure DMA for ADC peripherals
void MX_DMA_Init(void)
{
    // DMA controller clock enable
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    // DMA interrupt init
    // DMA1_Stream0_IRQn interrupt configuration (existing - for SPI or other)
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    // DMA1_Stream1_IRQn interrupt configuration (existing - for SPI or other)
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

    // ADC1 DMA interrupt configuration (DMA1 Stream 2)
    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);

    // ADC2 DMA interrupt configuration (DMA1 Stream 3)
    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

    // ADC3 DMA interrupt configuration (DMA2 Stream 0)
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}