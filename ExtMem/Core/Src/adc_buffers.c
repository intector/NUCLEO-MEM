/**
  ****************************************************************************************
 * @file           : adc_buffers.c
 * @brief          : DMA buffer management implementation
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
#include "adc_buffers.h"
#include "adc.h"
#include <string.h>

// Private variables ---------------------------------------------------------------------

// Place buffers in D1 domain (AXI SRAM) for DMA access
// Address range: 0x24000000 - 0x2407FFFF (512KB)
#if defined(__GNUC__)
__attribute__((section(".dma_buffer"))) ADC1_Buffer_t adc1_dma_buffer;
__attribute__((section(".dma_buffer"))) ADC2_Buffer_t adc2_dma_buffer;
__attribute__((section(".dma_buffer"))) ADC3_Buffer_t adc3_dma_buffer;
#elif defined(__ICCARM__)
#pragma location = 0x24000000
ADC1_Buffer_t adc1_dma_buffer;
ADC2_Buffer_t adc2_dma_buffer;
ADC3_Buffer_t adc3_dma_buffer;
#endif

// Function definitions ------------------------------------------------------------------

// Initialize all ADC DMA buffers
void ADC_Buffers_Init(void)
{
    // Clear all buffers
    memset(&adc1_dma_buffer, 0, sizeof(ADC1_Buffer_t));
    memset(&adc2_dma_buffer, 0, sizeof(ADC2_Buffer_t));
    memset(&adc3_dma_buffer, 0, sizeof(ADC3_Buffer_t));
}

// Start ADC1 with DMA in circular mode
void ADC1_DMA_Start(void)
{
    HAL_ADC_Start_DMA(&hadc1, 
                     (uint32_t*)adc1_dma_buffer.buffer, 
                     ADC1_NUM_CHANNELS * ADC1_BUFFER_DEPTH);
}

// Start ADC2 with DMA in circular mode
void ADC2_DMA_Start(void)
{
    HAL_ADC_Start_DMA(&hadc2, 
                     (uint32_t*)adc2_dma_buffer.buffer, 
                     ADC2_NUM_CHANNELS * ADC2_BUFFER_DEPTH);
}

// Start ADC3 with DMA in circular mode
void ADC3_DMA_Start(void)
{
    HAL_ADC_Start_DMA(&hadc3, 
                     (uint32_t*)adc3_dma_buffer.buffer, 
                     ADC3_NUM_CHANNELS * ADC3_BUFFER_DEPTH);
}

// Check if ADC1 has new data
bool ADC1_IsDataReady(void)
{
    return (adc1_dma_buffer.half_complete_flag || adc1_dma_buffer.full_complete_flag);
}

// Check if ADC2 has new data
bool ADC2_IsDataReady(void)
{
    return (adc2_dma_buffer.half_complete_flag || adc2_dma_buffer.full_complete_flag);
}

// Check if ADC3 has new data
bool ADC3_IsDataReady(void)
{
    return (adc3_dma_buffer.half_complete_flag || adc3_dma_buffer.full_complete_flag);
}

// Get readable buffer pointer for ADC1
uint16_t* ADC1_GetReadBuffer(void)
{
    if (adc1_dma_buffer.half_complete_flag) {
        return adc1_dma_buffer.buffer;  // First half
    }
    else if (adc1_dma_buffer.full_complete_flag) {
        return &adc1_dma_buffer.buffer[ADC1_NUM_CHANNELS * (ADC1_BUFFER_DEPTH / 2)];  // Second half
    }
    return NULL;
}

// Get readable buffer pointer for ADC2
uint16_t* ADC2_GetReadBuffer(void)
{
    if (adc2_dma_buffer.half_complete_flag) {
        return adc2_dma_buffer.buffer;
    }
    else if (adc2_dma_buffer.full_complete_flag) {
        return &adc2_dma_buffer.buffer[ADC2_NUM_CHANNELS * (ADC2_BUFFER_DEPTH / 2)];
    }
    return NULL;
}

// Get readable buffer pointer for ADC3
uint16_t* ADC3_GetReadBuffer(void)
{
    if (adc3_dma_buffer.half_complete_flag) {
        return adc3_dma_buffer.buffer;
    }
    else if (adc3_dma_buffer.full_complete_flag) {
        return &adc3_dma_buffer.buffer[ADC3_NUM_CHANNELS * (ADC3_BUFFER_DEPTH / 2)];
    }
    return NULL;
}

// Clear ADC1 ready flags
void ADC1_ClearDataReady(void)
{
    adc1_dma_buffer.half_complete_flag = 0;
    adc1_dma_buffer.full_complete_flag = 0;
}

// Clear ADC2 ready flags
void ADC2_ClearDataReady(void)
{
    adc2_dma_buffer.half_complete_flag = 0;
    adc2_dma_buffer.full_complete_flag = 0;
}

// Clear ADC3 ready flags
void ADC3_ClearDataReady(void)
{
    adc3_dma_buffer.half_complete_flag = 0;
    adc3_dma_buffer.full_complete_flag = 0;
}

// ADC1 DMA half transfer complete callback
void ADC1_DMA_HalfCpltCallback(void)
{
    adc1_dma_buffer.half_complete_flag = 1;
    adc1_dma_buffer.full_complete_flag = 0;
}

// ADC1 DMA full transfer complete callback
void ADC1_DMA_FullCpltCallback(void)
{
    adc1_dma_buffer.half_complete_flag = 0;
    adc1_dma_buffer.full_complete_flag = 1;
}

// ADC2 DMA half transfer complete callback
void ADC2_DMA_HalfCpltCallback(void)
{
    adc2_dma_buffer.half_complete_flag = 1;
    adc2_dma_buffer.full_complete_flag = 0;
}

// ADC2 DMA full transfer complete callback
void ADC2_DMA_FullCpltCallback(void)
{
    adc2_dma_buffer.half_complete_flag = 0;
    adc2_dma_buffer.full_complete_flag = 1;
}

// ADC3 DMA half transfer complete callback
void ADC3_DMA_HalfCpltCallback(void)
{
    adc3_dma_buffer.half_complete_flag = 1;
    adc3_dma_buffer.full_complete_flag = 0;
}

// ADC3 DMA full transfer complete callback
void ADC3_DMA_FullCpltCallback(void)
{
    adc3_dma_buffer.half_complete_flag = 0;
    adc3_dma_buffer.full_complete_flag = 1;
}

// HAL callback wrappers - called by HAL driver
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        ADC1_DMA_HalfCpltCallback();
    }
    else if (hadc->Instance == ADC2) {
        ADC2_DMA_HalfCpltCallback();
    }
    else if (hadc->Instance == ADC3) {
        ADC3_DMA_HalfCpltCallback();
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        ADC1_DMA_FullCpltCallback();
    }
    else if (hadc->Instance == ADC2) {
        ADC2_DMA_FullCpltCallback();
    }
    else if (hadc->Instance == ADC3) {
        ADC3_DMA_FullCpltCallback();
    }
}
