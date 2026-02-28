/**
  ****************************************************************************************
 * @file    tim.c
 * @brief   This file provides code for the configuration
 *          of the TIM instances.
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
#include "tim.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim15;

// TIM1 init function --------------------------------------------------------------------
void MX_TIM1_Init(void)
{

    TIM_MasterConfigTypeDef sMasterConfig               = {0};
    TIM_OC_InitTypeDef sConfigOC                        = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim1.Instance               = TIM1;
    htim1.Init.Prescaler         = 0;
    htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim1.Init.Period            = 1499;
    htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_TIM_PWM_Init, HAL_TIM_PWM_Init(&htim1));

    sMasterConfig.MasterOutputTrigger  = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_TIMEx_MCS, HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig));

    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = 750;
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    STM32_ERROR_CHECK(ERR_HAL_TIM_PWM_CC, HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1));

    sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime         = 0;
    sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter      = 0;
    sBreakDeadTimeConfig.Break2State      = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.Break2Polarity   = TIM_BREAK2POLARITY_HIGH;
    sBreakDeadTimeConfig.Break2Filter     = 0;
    sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_TIMEx_ConfigBDT, HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig));

    HAL_TIM_MspPostInit(&htim1);
}

// TIM1 init function --------------------------------------------------------------------
void MX_TIM15_Init(void)
{

    TIM_MasterConfigTypeDef sMasterConfig               = {0};
    TIM_OC_InitTypeDef sConfigOC                        = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim15.Instance               = TIM15;
    htim15.Init.Prescaler         = 0;
    htim15.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim15.Init.Period            = 1200;
    htim15.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim15.Init.RepetitionCounter = 0;
    htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_TIM_PWM_Init, HAL_TIM_PWM_Init(&htim15));

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_TIMEx_MCS, HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig));

    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = 240;
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    STM32_ERROR_CHECK(ERR_HAL_TIM_PWM_CC, HAL_TIM_PWM_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_1));

    sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime         = 0;
    sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter      = 0;
    sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_TIMEx_ConfigBDT, HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig));

    HAL_TIM_MspPostInit(&htim15);
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *tim_pwmHandle)
{

    if (tim_pwmHandle->Instance == TIM1) {
        // TIM1 clock enable
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
    else if (tim_pwmHandle->Instance == TIM15) {
        // TIM15 clock enable
        __HAL_RCC_TIM15_CLK_ENABLE();
    }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (timHandle->Instance == TIM1) {

        //__HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();
        // TIM1 GPIO Configuration
        // PB1     ------> TIM1_CH3N
        // PE13     ------> TIM1_CH3
        GPIO_InitStruct.Pin       = TIM1_PWM1_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(TIM1_PWM1_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = TIM1_PWM1N_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(TIM1_PWM1N_GPIO_Port, &GPIO_InitStruct);
    }
    else if (timHandle->Instance == TIM15) {

        __HAL_RCC_GPIOE_CLK_ENABLE();
        // TIM15 GPIO Configuration
        // PE4     ------> TIM15_CH1N
        // PE5     ------> TIM15_CH1
        GPIO_InitStruct.Pin       = TIM15PWM1_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_TIM15;
        HAL_GPIO_Init(TIM15PWM1_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = TIM15PWM1N_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_TIM15;
        HAL_GPIO_Init(TIM15PWM1N_GPIO_Port, &GPIO_InitStruct);
    }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *tim_pwmHandle)
{

    if (tim_pwmHandle->Instance == TIM1) {
        // Peripheral clock disable
        __HAL_RCC_TIM1_CLK_DISABLE();
    }
    else if (tim_pwmHandle->Instance == TIM15) {
        // Peripheral clock disable
        __HAL_RCC_TIM15_CLK_DISABLE();
    }
}

