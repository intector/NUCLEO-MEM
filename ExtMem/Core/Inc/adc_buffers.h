/**
  ****************************************************************************************
 * @file           : adc_buffers.h
 * @brief          : DMA buffer structures and management for ADC peripherals
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
#ifndef __ADC_BUFFERS_H__
#define __ADC_BUFFERS_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>

// Exported defines ----------------------------------------------------------------------

// Buffer depth definitions - power of 2 for efficient indexing
#define ADC1_BUFFER_DEPTH    128    // ~1.28ms worth at 100kHz
#define ADC2_BUFFER_DEPTH    128
#define ADC3_BUFFER_DEPTH    256    // Larger for temperature oversampling

#define ADC1_NUM_CHANNELS    2
#define ADC2_NUM_CHANNELS    4
#define ADC3_NUM_CHANNELS    7

// ADC1 Channel assignments
#define ADC1_CH_DIFFERENTIAL_VOLTAGE    0
#define ADC1_CH_TANK_CURRENT           1

// ADC2 Channel assignments (temperatures)
#define ADC2_CH_HBRIDGE_Q1_TEMP        0
#define ADC2_CH_HBRIDGE_Q2_TEMP        1
#define ADC2_CH_HBRIDGE_Q3_TEMP        2
#define ADC2_CH_HBRIDGE_Q4_TEMP        3

// ADC3 Channel assignments
#define ADC3_CH_BOOSTER_INPUT_I        0
#define ADC3_CH_BOOSTER_SWITCH_I       1
#define ADC3_CH_BOOSTER_OUTPUT_V       2
#define ADC3_CH_BOOSTER_Q1_TEMP        3
#define ADC3_CH_BOOSTER_Q2_TEMP        4
#define ADC3_CH_BOOSTER_INDUCTOR_TEMP  5
#define ADC3_CH_SPARE                  6

// Exported types ------------------------------------------------------------------------

// ADC1 DMA buffer structure
// Interleaved format: [CH0, CH1, CH2, CH0, CH1, CH2, ...]
typedef struct {
    uint16_t buffer[ADC1_NUM_CHANNELS * ADC1_BUFFER_DEPTH];
    volatile uint32_t half_complete_flag;
    volatile uint32_t full_complete_flag;
    volatile uint32_t error_count;
} ADC1_Buffer_t;

// ADC2 DMA buffer structure
// Interleaved format: [CH0, CH1, CH2, CH3, CH0, CH1, ...]
typedef struct {
    uint16_t buffer[ADC2_NUM_CHANNELS * ADC2_BUFFER_DEPTH];
    volatile uint32_t half_complete_flag;
    volatile uint32_t full_complete_flag;
    volatile uint32_t error_count;
} ADC2_Buffer_t;

// ADC3 DMA buffer structure
// Interleaved format: [CH0, CH1, ..., CH6, CH0, CH1, ...]
typedef struct {
    uint16_t buffer[ADC3_NUM_CHANNELS * ADC3_BUFFER_DEPTH];
    volatile uint32_t half_complete_flag;
    volatile uint32_t full_complete_flag;
    volatile uint32_t error_count;
} ADC3_Buffer_t;

// Exported variables --------------------------------------------------------------------

// Global buffer declarations - allocated in D1 RAM
extern ADC1_Buffer_t adc1_dma_buffer;
extern ADC2_Buffer_t adc2_dma_buffer;
extern ADC3_Buffer_t adc3_dma_buffer;

// Exported functions --------------------------------------------------------------------

void ADC_Buffers_Init(void);
void ADC1_DMA_Start(void);
void ADC2_DMA_Start(void);
void ADC3_DMA_Start(void);

bool ADC1_IsDataReady(void);
bool ADC2_IsDataReady(void);
bool ADC3_IsDataReady(void);

uint16_t* ADC1_GetReadBuffer(void);
uint16_t* ADC2_GetReadBuffer(void);
uint16_t* ADC3_GetReadBuffer(void);

void ADC1_ClearDataReady(void);
void ADC2_ClearDataReady(void);
void ADC3_ClearDataReady(void);

void ADC1_DMA_HalfCpltCallback(void);
void ADC1_DMA_FullCpltCallback(void);
void ADC2_DMA_HalfCpltCallback(void);
void ADC2_DMA_FullCpltCallback(void);
void ADC3_DMA_HalfCpltCallback(void);
void ADC3_DMA_FullCpltCallback(void);

#ifdef __cplusplus
}
#endif

#endif // __ADC_BUFFERS_H__
