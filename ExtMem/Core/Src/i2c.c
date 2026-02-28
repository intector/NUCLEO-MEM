/**
  ****************************************************************************************
  * @file           : i2c.c
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

// Includes ------------------------------------------------------------------------------
#include "i2c.h"

I2C_HandleTypeDef Handle_I2C2;

// I2C4 init function
void MX_I2C2_Init(void)
{
    Handle_I2C2.Instance              = I2C2;
    Handle_I2C2.Init.Timing           = 0x00C0EAFF;
    Handle_I2C2.Init.OwnAddress1      = 0;
    Handle_I2C2.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    Handle_I2C2.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    Handle_I2C2.Init.OwnAddress2      = 0;
    Handle_I2C2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    Handle_I2C2.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    Handle_I2C2.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    STM32_ERROR_CHECK(ERR_HAL_I2C_Init, HAL_I2C_Init(&Handle_I2C2));

	// * Configure Analogue filter
	STM32_ERROR_CHECK(ERR_HAL_I2CEx_ConfigAnalogFilter, HAL_I2CEx_ConfigAnalogFilter(&Handle_I2C2, I2C_ANALOGFILTER_ENABLE));

	// * Configure Digital filter
	STM32_ERROR_CHECK(ERR_HAL_I2CEx_ConfigDigitalFilter, HAL_I2CEx_ConfigDigitalFilter(&Handle_I2C2, 0));

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
    if (i2cHandle->Instance == I2C2)
    {

        // Initializes the peripherals clock

        // PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
        // PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
        // STM32_ERROR_CHECK(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct));

        __HAL_RCC_GPIOB_CLK_ENABLE();
        // I2C2 GPIO Configuration
        // PF1     ------> I2C2_SCL
        // PF0     ------> I2C2_SDA

        GPIO_InitStruct.Pin       = OLED_SCL_Pin | OLED_SDA_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
        // GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        // I2C2 clock enable
        __HAL_RCC_I2C2_CLK_ENABLE();
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

	if (i2cHandle->Instance == I2C2)
	{
		  // Peripheral clock disable
		__HAL_RCC_I2C2_CLK_DISABLE();

		// *I2C2 GPIO Configuration
		// PF1     ------> I2C2_SCL
		// PF0     ------> I2C2_SDA
		HAL_GPIO_DeInit(OLED_SCL_GPIO_Port, OLED_SCL_Pin);
		HAL_GPIO_DeInit(OLED_SDA_GPIO_Port, OLED_SDA_Pin);

	}
}



