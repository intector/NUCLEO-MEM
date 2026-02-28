/**
  ****************************************************************************************
 * @file    tim.h
 * @brief   This file contains all the function prototypes for
 *          the tim.c file
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "main.h"
#include "io_pin_def.h"
#include "string.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim15;

void MX_TIM1_Init(void);
void MX_TIM15_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif // __TIM_H__

