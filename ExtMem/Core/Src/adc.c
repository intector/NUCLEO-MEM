/**
  ****************************************************************************************
 * @file           : adc.c
 * @brief          : analog-to-digital converter (ADC) configuration
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

#include "adc.h"
#include "err_codes.h"
#include "dma.h"

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

void MX_ADC1_Init(void)
{
    ADC_MultiModeTypeDef multimode = {0};
    ADC_ChannelConfTypeDef sConfig = {0};

    // Common config
    hadc1.Instance                      = ADC1;
    hadc1.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV4;
    hadc1.Init.Resolution               = ADC_RESOLUTION_16B;
    hadc1.Init.ScanConvMode             = ADC_SCAN_ENABLE;
    hadc1.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
    hadc1.Init.LowPowerAutoWait         = DISABLE;
    hadc1.Init.ContinuousConvMode       = ENABLE;
    hadc1.Init.NbrOfConversion          = 2;
    hadc1.Init.DiscontinuousConvMode    = DISABLE;
    hadc1.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc1.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
    hadc1.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
    hadc1.Init.OversamplingMode         = DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_ADC_Init, HAL_ADC_Init(&hadc1));

    // Configure the ADC multi-mode
    multimode.Mode = ADC_MODE_INDEPENDENT;
    STM32_ERROR_CHECK(ERR_HAL_ADCEx_MMCC, HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode));

    // Configure Regular Channel: PA4/PA5 - INP18/INN18-VS (H-Bridge Differential Voltage)
    sConfig.Channel                = ADC_CHANNEL_18;
    sConfig.Rank                   = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime           = ADC_SAMPLETIME_64CYCLES_5;
    sConfig.SingleDiff             = ADC_DIFFERENTIAL_ENDED;
    sConfig.OffsetNumber           = ADC_OFFSET_NONE;
    sConfig.Offset                 = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc1, &sConfig));

    // Configure Regular Channel: PB0 - ANA-IN-CS (H-Bridge Current)
    sConfig.Channel      = ADC_CHANNEL_9;
    sConfig.Rank         = ADC_REGULAR_RANK_2;
    sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc1, &sConfig));

    // Perform ADC calibration (both differential and single-ended)
    STM32_ERROR_CHECK(ERR_HAL_ADCEx_Calibration, HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_DIFFERENTIAL_ENDED));
    STM32_ERROR_CHECK(ERR_HAL_ADCEx_Calibration, HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED));
}

void MX_ADC2_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    // Common config
    hadc2.Instance                      = ADC2;
    hadc2.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV4;
    hadc2.Init.Resolution               = ADC_RESOLUTION_16B;
    hadc2.Init.ScanConvMode             = ADC_SCAN_ENABLE;
    hadc2.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
    hadc2.Init.LowPowerAutoWait         = DISABLE;
    hadc2.Init.ContinuousConvMode       = ENABLE;
    hadc2.Init.NbrOfConversion          = 4;
    hadc2.Init.DiscontinuousConvMode    = DISABLE;
    hadc2.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    hadc2.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc2.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
    hadc2.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
    hadc2.Init.OversamplingMode         = DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_ADC_Init, HAL_ADC_Init(&hadc2));

    // Configure Regular Channel: PA3 - ANA-IN-CS1 (Booster Input Current)
    sConfig.Channel                = ADC_CHANNEL_15;
    sConfig.Rank                   = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime           = ADC_SAMPLETIME_64CYCLES_5;
    sConfig.SingleDiff             = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber           = ADC_OFFSET_NONE;
    sConfig.Offset                 = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc2, &sConfig));

    // Configure Regular Channel: PC0 - ANA-IN-CS2 (Booster Switch Current)
    sConfig.Channel = ADC_CHANNEL_10;
    sConfig.Rank    = ADC_REGULAR_RANK_2;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc2, &sConfig));

    // Configure Regular Channel: PA6 - SWN-SENSE (Booster Switch Voltage)
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank    = ADC_REGULAR_RANK_3;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc2, &sConfig));

    // Configure Regular Channel: PB1 - VOUT-SENSE (Booster Output Voltage)
    sConfig.Channel = ADC_CHANNEL_5;
    sConfig.Rank    = ADC_REGULAR_RANK_4;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc2, &sConfig));

    // Perform ADC calibration
    STM32_ERROR_CHECK(ERR_HAL_ADCEx_Calibration, HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED));
}

void MX_ADC3_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    // Common config
    hadc3.Instance                      = ADC3;
    hadc3.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV4;
    hadc3.Init.Resolution               = ADC_RESOLUTION_16B;
    hadc3.Init.ScanConvMode             = ADC_SCAN_ENABLE;
    hadc3.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
    hadc3.Init.LowPowerAutoWait         = DISABLE;
    hadc3.Init.ContinuousConvMode       = ENABLE;
    hadc3.Init.NbrOfConversion          = 7;
    hadc3.Init.DiscontinuousConvMode    = DISABLE;
    hadc3.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    hadc3.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc3.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
    hadc3.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
    hadc3.Init.OversamplingMode         = DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_ADC_Init, HAL_ADC_Init(&hadc3));

    // Configure Regular Channel: PC2 - NTC-A-Q1 (H-Bridge Q1 Temperature)
    sConfig.Channel                = ADC_CHANNEL_0;
    sConfig.Rank                   = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime           = ADC_SAMPLETIME_810CYCLES_5;
    sConfig.SingleDiff             = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber           = ADC_OFFSET_NONE;
    sConfig.Offset                 = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc3, &sConfig));

    // Configure Regular Channel: PC3 - NTC-A-Q2 (H-Bridge Q2 Temperature)
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank    = ADC_REGULAR_RANK_2;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc3, &sConfig));

    // Configure Regular Channel: PF3 - NTC-A-Q3 (H-Bridge Q3 Temperature)
    sConfig.Channel = ADC_CHANNEL_5;
    sConfig.Rank    = ADC_REGULAR_RANK_3;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc3, &sConfig));

    // Configure Regular Channel: PF4 - NTC-A-Q4 (H-Bridge Q4 Temperature)
    sConfig.Channel = ADC_CHANNEL_9;
    sConfig.Rank    = ADC_REGULAR_RANK_4;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc3, &sConfig));

    // Configure Regular Channel: PF5 - NTC-L1 (Booster Inductor Temperature)
    sConfig.Channel = ADC_CHANNEL_4;
    sConfig.Rank    = ADC_REGULAR_RANK_5;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc3, &sConfig));

    // Configure Regular Channel: PF6 - NTC-B-Q2 (Booster Q2 Temperature)
    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank    = ADC_REGULAR_RANK_6;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc3, &sConfig));

    // Configure Regular Channel: PF10 - NTC-B-Q1 (Booster Q1 Temperature)
    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank    = ADC_REGULAR_RANK_7;
    STM32_ERROR_CHECK(ERR_HAL_ADC_ConfigChannel, HAL_ADC_ConfigChannel(&hadc3, &sConfig));

    // Perform ADC calibration
    STM32_ERROR_CHECK(ERR_HAL_ADCEx_Calibration, HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED));
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (adcHandle->Instance == ADC1) {
        // Peripheral clock enable (PLL3 already configured in PeriphCommonClock_Config)
        __HAL_RCC_ADC12_CLK_ENABLE();

        // ADC1 GPIO Configuration
        // PA4 -> ADC1_INP18 (INP18-VS)
        // PA5 -> ADC1_INN18 (INN18-VS)
        // PB0 -> ADC1_INP9 (ANA-IN-CS)

        GPIO_InitStruct.Pin  = INP18_VS_Pin | INN18_VS_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(INP18_VS_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin  = ANA_IN_CS_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ANA_IN_CS_GPIO_Port, &GPIO_InitStruct);

        // DMA configuration for ADC1
        hdma_adc1.Instance                 = DMA1_Stream2;
        hdma_adc1.Init.Request             = DMA_REQUEST_ADC1;
        hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        hdma_adc1.Init.Mode                = DMA_CIRCULAR;
        hdma_adc1.Init.Priority            = DMA_PRIORITY_HIGH;
        hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

        STM32_ERROR_CHECK(ERR_HAL_DMA_Init, HAL_DMA_Init(&hdma_adc1));

        __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);
    }
    else if (adcHandle->Instance == ADC2) {
        // Peripheral clock enable
        __HAL_RCC_ADC12_CLK_ENABLE();

        // ADC2 GPIO Configuration
        // PA3 -> ADC2_INP15 (ANA-IN-CS1)
        // PC0 -> ADC2_INP10 (ANA-IN-CS2)
        // PA6 -> ADC2_INP3 (SWN-SENSE)
        // PB1 -> ADC2_INP5 (VOUT-SENSE)

        // ##### conflict with PSRAM IO
        // GPIO_InitStruct.Pin  = ANA_IN_CS1_Pin | SWN_SENSE_Pin;
        // GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        // GPIO_InitStruct.Pull = GPIO_NOPULL;
        // HAL_GPIO_Init(ANA_IN_CS1_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin  = VOUT_SENSE_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(VOUT_SENSE_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin  = ANA_IN_CS2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ANA_IN_CS2_GPIO_Port, &GPIO_InitStruct);

        // DMA configuration for ADC2
        hdma_adc2.Instance                 = DMA1_Stream3;
        hdma_adc2.Init.Request             = DMA_REQUEST_ADC2;
        hdma_adc2.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        hdma_adc2.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_adc2.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc2.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        hdma_adc2.Init.Mode                = DMA_CIRCULAR;
        hdma_adc2.Init.Priority            = DMA_PRIORITY_HIGH;
        hdma_adc2.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

        STM32_ERROR_CHECK(ERR_HAL_DMA_Init, HAL_DMA_Init(&hdma_adc2));

        __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc2);
    }
    else if (adcHandle->Instance == ADC3) {
        // Peripheral clock enable
        __HAL_RCC_ADC3_CLK_ENABLE();

        // ADC3 GPIO Configuration
        // PC2 -> ADC3_INP0 (NTC-A-Q1)
        // PC3 -> ADC3_INP1 (NTC-A-Q2)
        // PF3 -> ADC3_INP5 (NTC-A-Q3)
        // PF4 -> ADC3_INP9 (NTC-A-Q4)
        // PF5 -> ADC3_INP4 (NTC-L1)
        // PF6 -> ADC3_INP8 (NTC-B-Q2)
        // PF10 -> ADC3_INP6 (NTC-B-Q1)

        // ##### conflict with PSRAM IO
        // GPIO_InitStruct.Pin  = NTC_A_Q1_Pin | NTC_A_Q2_Pin;
        // GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        // GPIO_InitStruct.Pull = GPIO_NOPULL;
        // HAL_GPIO_Init(NTC_A_Q1_GPIO_Port, &GPIO_InitStruct);
        //
        // GPIO_InitStruct.Pin  = NTC_A_Q3_Pin | NTC_A_Q4_Pin | NTC_L1_Pin | NTC_B_Q2_Pin | NTC_B_Q1_Pin;
        // GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        // GPIO_InitStruct.Pull = GPIO_NOPULL;
        // HAL_GPIO_Init(NTC_A_Q3_GPIO_Port, &GPIO_InitStruct);

        // DMA configuration for ADC3
        hdma_adc3.Instance                 = DMA2_Stream0;
        hdma_adc3.Init.Request             = DMA_REQUEST_ADC3;
        hdma_adc3.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        hdma_adc3.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_adc3.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc3.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        hdma_adc3.Init.Mode                = DMA_CIRCULAR;
        hdma_adc3.Init.Priority            = DMA_PRIORITY_HIGH;
        hdma_adc3.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

        STM32_ERROR_CHECK(ERR_HAL_DMA_Init, HAL_DMA_Init(&hdma_adc3));

        __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc3);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle)
{

    if (adcHandle->Instance == ADC1) {
        __HAL_RCC_ADC12_CLK_DISABLE();

        HAL_GPIO_DeInit(INP18_VS_GPIO_Port, INP18_VS_Pin | INN18_VS_Pin);
        HAL_GPIO_DeInit(ANA_IN_CS_GPIO_Port, ANA_IN_CS_Pin);

        // DMA de-init
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
    }
    else if (adcHandle->Instance == ADC2) {
        __HAL_RCC_ADC12_CLK_DISABLE();

        // ##### conflict with PSRAM IO
        // HAL_GPIO_DeInit(ANA_IN_CS1_GPIO_Port, ANA_IN_CS1_Pin | SWN_SENSE_Pin);
        // HAL_GPIO_DeInit(VOUT_SENSE_GPIO_Port, VOUT_SENSE_Pin);
        // HAL_GPIO_DeInit(ANA_IN_CS2_GPIO_Port, ANA_IN_CS2_Pin);
        
        // DMA de-init
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
    }
    else if (adcHandle->Instance == ADC3) {
        __HAL_RCC_ADC3_CLK_DISABLE();

        // ##### conflict with PSRAM IO
        // HAL_GPIO_DeInit(NTC_A_Q1_GPIO_Port, NTC_A_Q1_Pin | NTC_A_Q2_Pin);
        // HAL_GPIO_DeInit(NTC_A_Q3_GPIO_Port, NTC_A_Q3_Pin | NTC_A_Q4_Pin | NTC_L1_Pin | NTC_B_Q2_Pin | NTC_B_Q1_Pin);
        
        // DMA de-init
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
    }
}
