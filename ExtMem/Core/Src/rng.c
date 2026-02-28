/**
  ****************************************************************************************
  * @file           : rng.c
  * @brief          : This file provides code for the configuration
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

// USER CODE END Header
// Includes ------------------------------------------------------------------------------
#include "rng.h"

// USER CODE BEGIN 0

// USER CODE END 0

RNG_HandleTypeDef hrng;

// RNG init function
void MX_RNG_Init(void)
{

  // USER CODE BEGIN RNG_Init 0

  // USER CODE END RNG_Init 0

  // USER CODE BEGIN RNG_Init 1

  // Enable HSI48 oscillator — required as RNG clock source but not
  // enabled by SystemClock_Config / PeriphCommonClock_Config.
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State     = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler(ERR_RCC_OscConfig, STM32_FAIL);
  }

  // USER CODE END RNG_Init 1
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler(ERR_HAL_RNG_Init, STM32_FAIL);
  }
  // USER CODE BEGIN RNG_Init 2

  // USER CODE END RNG_Init 2

}

void HAL_RNG_MspInit(RNG_HandleTypeDef* rngHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(rngHandle->Instance==RNG)
  {
  // USER CODE BEGIN RNG_MspInit 0

  // USER CODE END RNG_MspInit 0

  // * Initializes the peripherals clock
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
    PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler(ERR_RCCEx_PeriphCLKConfig, STM32_FAIL);
    }

    // RNG clock enable
    __HAL_RCC_RNG_CLK_ENABLE();
  // USER CODE BEGIN RNG_MspInit 1

  // USER CODE END RNG_MspInit 1
  }
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef* rngHandle)
{

  if(rngHandle->Instance==RNG)
  {
  // USER CODE BEGIN RNG_MspDeInit 0

  // USER CODE END RNG_MspDeInit 0
    // Peripheral clock disable
    __HAL_RCC_RNG_CLK_DISABLE();
  // USER CODE BEGIN RNG_MspDeInit 1

  // USER CODE END RNG_MspDeInit 1
  }
}

// USER CODE BEGIN 1

// USER CODE END 1
