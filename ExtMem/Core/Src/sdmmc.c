/**
  ****************************************************************************************
  * @file    sdmmc.c
  * @brief   This file provides code for the configuration
  *          of the SDMMC instances.
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
#include "sdmmc.h"

MMC_HandleTypeDef hmmc1;

// SDMMC1 init function ------------------------------------------------------------------
void MX_SDMMC1_MMC_Init(void)
{
    hmmc1.Instance = SDMMC1;
    hmmc1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hmmc1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hmmc1.Init.BusWide = SDMMC_BUS_WIDE_8B;
    hmmc1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hmmc1.Init.ClockDiv = 4;

    STM32_ERROR_CHECK(ERR_HAL_MMC_Init, HAL_MMC_Init(&hmmc1));
}

void HAL_MMC_MspInit(MMC_HandleTypeDef* mmcHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
    if (mmcHandle->Instance == SDMMC1)
    {
        // SDMMC1 clock enable -------------------------------------------------
        __HAL_RCC_SDMMC1_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        // SDMMC1 GPIO Configuration -------------------------------------------
        // PC6     ------> SDMMC1_D6
        // PC7     ------> SDMMC1_D7
        // PC8     ------> SDMMC1_D0
        // PC9     ------> SDMMC1_D1
        // PC10     ------> SDMMC1_D2
        // PC11     ------> SDMMC1_D3
        // PC12     ------> SDMMC1_CK
        // PD2     ------> SDMMC1_CMD
        // PB8     ------> SDMMC1_D4
        // PB9     ------> SDMMC1_D5

        GPIO_InitStruct.Pin = EMMC_D6_Pin | EMMC_D7_Pin | EMMC_D0_Pin | EMMC_D1_Pin
                              | EMMC_D2_Pin | EMMC_D3_Pin | EMMC_CLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = EMMC_CMD_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(EMMC_CMD_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = EMMC_D4_Pin | EMMC_D5_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // SDMMC1 interrupt Init ---------------------------------------------
        HAL_NVIC_SetPriority(SDMMC1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
    }
}

void HAL_MMC_MspDeInit(MMC_HandleTypeDef* mmcHandle)
{
    if (mmcHandle->Instance == SDMMC1)
    {
        // Peripheral clock disable --------------------------------------------
        __HAL_RCC_SDMMC1_CLK_DISABLE();

        // SDMMC1 GPIO Configuration -------------------------------------------
        // PC6     ------> SDMMC1_D6
        // PC7     ------> SDMMC1_D7
        // PC8     ------> SDMMC1_D0
        // PC9     ------> SDMMC1_D1
        // PC10     ------> SDMMC1_D2
        // PC11     ------> SDMMC1_D3
        // PC12     ------> SDMMC1_CK
        // PD2     ------> SDMMC1_CMD
        // PB8     ------> SDMMC1_D4
        // PB9     ------> SDMMC1_D5

        HAL_GPIO_DeInit(GPIOC, EMMC_D6_Pin|EMMC_D7_Pin|EMMC_D0_Pin|EMMC_D1_Pin
                              |EMMC_D2_Pin|EMMC_D3_Pin|EMMC_CLK_Pin);

        HAL_GPIO_DeInit(EMMC_CMD_GPIO_Port, EMMC_CMD_Pin);

        HAL_GPIO_DeInit(GPIOB, EMMC_D4_Pin | EMMC_D5_Pin);

        // SDMMC1 interrupt Deinit ---------------------------------------------
        HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
    }
}

