/**
  ****************************************************************************************
 * @file           : octospi.c
 * @brief          : OCTOSPI1 peripheral initialization for APS6408L PSRAM
 *                   on STM32H723ZG + NUCLEO-MEM expansion board
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
#include "octospi.h"

// Private variables ---------------------------------------------------------------------
OSPI_HandleTypeDef hospi1;

// OCTOSPI1 init function ----------------------------------------------------------------
void MX_OCTOSPI1_Init(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    // SYSCFG->PMCR |= SYSCFG_PMCR_PC2SO | SYSCFG_PMCR_PC3SO;
    HAL_EnableCompensationCell();

    hospi1.Instance                = OCTOSPI1;
    hospi1.Init.FifoThreshold      = 1;
    hospi1.Init.DualQuad           = HAL_OSPI_DUALQUAD_DISABLE;
    hospi1.Init.MemoryType         = HAL_OSPI_MEMTYPE_APMEMORY;
    hospi1.Init.DeviceSize         = PSRAM_SIZE_64MB + 1;
    hospi1.Init.ChipSelectHighTime = 2;
    hospi1.Init.FreeRunningClock   = HAL_OSPI_FREERUNCLK_DISABLE;
    hospi1.Init.ClockMode          = HAL_OSPI_CLOCK_MODE_0;
    hospi1.Init.WrapSize           = HAL_OSPI_WRAP_NOT_SUPPORTED;
    hospi1.Init.ClockPrescaler     = 8;
    // hospi1.Init.ClockPrescaler     = 6;
    hospi1.Init.SampleShifting     = HAL_OSPI_SAMPLE_SHIFTING_NONE;
    hospi1.Init.ChipSelectBoundary = 10; // 1 KB page boundary
    hospi1.Init.MaxTran            = 0;
    hospi1.Init.Refresh            = 99;
    // hospi1.Init.Refresh            = 132;

#if PSRAM_BYPASS_DLYB_TEST
    // DLYB bypassed: use DHQC for quarter-cycle shift at 50 MHz
    hospi1.Init.DelayBlockBypass      = HAL_OSPI_DELAY_BLOCK_BYPASSED;
    hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_ENABLE;
#else
    // DLYB active: it provides the quarter-period shift, DHQC must be off
    hospi1.Init.DelayBlockBypass      = HAL_OSPI_DELAY_BLOCK_USED;
    hospi1.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
#endif

    STM32_ERROR_CHECK(ERR_PSRAM_OSPI_Init, HAL_OSPI_Init(&hospi1));

    OSPIM_CfgTypeDef sOspiManagerCfg = {0};
    sOspiManagerCfg.ClkPort          = 1;
    sOspiManagerCfg.DQSPort          = 1;
    sOspiManagerCfg.NCSPort          = 1;
    sOspiManagerCfg.IOLowPort        = HAL_OSPIM_IOPORT_1_LOW;
    sOspiManagerCfg.IOHighPort       = HAL_OSPIM_IOPORT_1_HIGH;
    sOspiManagerCfg.Req2AckTime      = 1;

    STM32_ERROR_CHECK(ERR_PSRAM_OSPIM_Config, HAL_OSPIM_Config(&hospi1, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE));
}

// HAL MSP callbacks (called from HAL_OSPI_Init) -----------------------------------------

void HAL_OSPI_MspInit(OSPI_HandleTypeDef *ospiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (ospiHandle->Instance == OCTOSPI1) {
        // OCTOSPI1 clock enable
        __HAL_RCC_OCTOSPIM_CLK_ENABLE();
        __HAL_RCC_OSPI1_CLK_ENABLE();

        __HAL_RCC_GPIOE_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        // OCTOSPI1 GPIO Configuration
        // PE2  ------> OCTOSPIM_P1_IO2
        // PE7  ------> OCTOSPIM_P1_IO4
        // PE10 ------> OCTOSPIM_P1_IO7
        // PE11 ------> OCTOSPIM_P1_NCS
        // PF6  ------> OCTOSPIM_P1_IO3
        // PF8  ------> OCTOSPIM_P1_IO0
        // PF9  ------> OCTOSPIM_P1_IO1
        // PC2  ------> OCTOSPIM_P1_IO5
        // PC3  ------> OCTOSPIM_P1_IO6
        // PA3  ------> OCTOSPIM_P1_CLK
        // PB2  ------> OCTOSPIM_P1_DQS

        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

        // PE2 -- IO2 (AF9)
        GPIO_InitStruct.Pin       = PSRAM_D2_Pin;
        GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
        HAL_GPIO_Init(PSRAM_D2_GPIO_Port, &GPIO_InitStruct);

        // PE7 (IO4), PE10 (IO7) -- AF10
        GPIO_InitStruct.Pin       = PSRAM_D4_Pin | PSRAM_D7_Pin;
        GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPIM_P1;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        // PE11 -- NCS (AF11)
        GPIO_InitStruct.Pin       = PSRAM_NCS_Pin;
        GPIO_InitStruct.Alternate = GPIO_AF11_OCTOSPIM_P1;
        HAL_GPIO_Init(PSRAM_NCS_GPIO_Port, &GPIO_InitStruct);

        // PF6 (IO3), PF8 (IO0), PF9 (IO1) -- AF10
        GPIO_InitStruct.Pin       = PSRAM_D3_Pin | PSRAM_D0_Pin | PSRAM_D1_Pin;
        GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPIM_P1;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        // PC2 (IO5), PC3 (IO6) -- AF4
        GPIO_InitStruct.Pin       = PSRAM_D5_Pin | PSRAM_D6_Pin;
        GPIO_InitStruct.Alternate = GPIO_AF4_OCTOSPIM_P1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        // PA3 -- CLK (AF12)
        GPIO_InitStruct.Pin       = PSRAM_CLK_Pin;
        GPIO_InitStruct.Alternate = GPIO_AF12_OCTOSPIM_P1;
        HAL_GPIO_Init(PSRAM_CLK_GPIO_Port, &GPIO_InitStruct);

        // PB2 -- DQS (AF10)
        GPIO_InitStruct.Pin       = PSRAM_DQS_Pin;
        GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPIM_P1;
        HAL_GPIO_Init(PSRAM_DQS_GPIO_Port, &GPIO_InitStruct);

        // OCTOSPI1 interrupt Init
        // HAL_NVIC_SetPriority(OCTOSPI1_IRQn, 0, 0);
        // HAL_NVIC_EnableIRQ(OCTOSPI1_IRQn);
    }
}

void HAL_OSPI_MspDeInit(OSPI_HandleTypeDef *ospiHandle)
{
    if (ospiHandle->Instance == OCTOSPI1) {
        // Peripheral clock disable
        __HAL_RCC_OCTOSPIM_CLK_DISABLE();
        __HAL_RCC_OSPI1_CLK_DISABLE();

        // OCTOSPI1 GPIO DeInit
        HAL_GPIO_DeInit(GPIOE, PSRAM_D2_Pin | PSRAM_D4_Pin | PSRAM_D7_Pin | PSRAM_NCS_Pin);
        HAL_GPIO_DeInit(GPIOF, PSRAM_D3_Pin | PSRAM_D0_Pin | PSRAM_D1_Pin);
        HAL_GPIO_DeInit(GPIOC, PSRAM_D5_Pin | PSRAM_D6_Pin);
        HAL_GPIO_DeInit(PSRAM_CLK_GPIO_Port, PSRAM_CLK_Pin);
        HAL_GPIO_DeInit(PSRAM_DQS_GPIO_Port, PSRAM_DQS_Pin);

        // OCTOSPI1 interrupt DeInit
        HAL_NVIC_DisableIRQ(OCTOSPI1_IRQn);
    }
}
