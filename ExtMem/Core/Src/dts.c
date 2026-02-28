/**
  ****************************************************************************************
 * @file           : dts.c
 * @brief          : Digital Temperature Sensor (DTS) driver — die temperature via
 *                   the dedicated DTS peripheral with factory calibration
  ****************************************************************************************
 *
  * Copyright (c) 2020-2026 Intector Inc.
 * All rights reserved.
 *
  ****************************************************************************************
 */

// Includes ------------------------------------------------------------------------------
#include "dts.h"
#include <stdio.h>

// Private variables ---------------------------------------------------------------------
static DTS_HandleTypeDef hdts;

// Exported functions --------------------------------------------------------------------

/**
 * @brief  Initialize the DTS peripheral in continuous measurement mode.
 *         Uses LSE (32.768 kHz) as reference clock to avoid PCLK prescaler overflow.
 * @retval STM32_OK on success, STM32_FAIL on error
 */
STM_ERR_T MX_DTS_Init(void)
{
    __HAL_RCC_DTS_CLK_ENABLE();

    hdts.Instance                  = DTS;
    hdts.Init.QuickMeasure         = DTS_QUICKMEAS_DISABLE;
    hdts.Init.RefClock             = DTS_REFCLKSEL_LSE;
    hdts.Init.TriggerInput         = DTS_TRIGGER_HW_NONE;
    hdts.Init.SamplingTime         = DTS_SMP_TIME_15_CYCLE;
    hdts.Init.Divider              = 0;
    hdts.Init.HighThreshold        = 0;
    hdts.Init.LowThreshold         = 0;

    // Set state to non-RESET so HAL_DTS_Init skips HAL_DTS_MspInit.
    // We already enabled the clock above — no other Msp setup is needed.
    hdts.State = HAL_DTS_STATE_READY;

    if (HAL_DTS_Init(&hdts) != HAL_OK) {
        printf("DTS init failed\r\n");
        return STM32_FAIL;
    }

    if (HAL_DTS_Start(&hdts) != HAL_OK) {
        printf("DTS start failed\r\n");
        return STM32_FAIL;
    }

    printf("DTS temperature sensor initialized\r\n");
    return STM32_OK;
}

/**
 * @brief  Read the current die temperature from the DTS peripheral.
 *         HAL_DTS_GetTemperature handles the factory-calibrated conversion internally.
 * @retval Temperature in degrees Celsius (float), or 0.0f on error
 */
float DTS_ReadTemperature(void)
{
    int32_t temp_int = 0;

    if (HAL_DTS_GetTemperature(&hdts, &temp_int) != HAL_OK) {
        return 0.0f;
    }

    return (float)temp_int;
}
