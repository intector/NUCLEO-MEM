/**
  ****************************************************************************************
 * @file           : dts.h
 * @brief          : Digital Temperature Sensor (DTS) driver interface
  ****************************************************************************************
 *
  * Copyright (c) 2020-2026 Intector Inc.
 * All rights reserved.
 *
  ****************************************************************************************
 */

// Define to prevent recursive inclusion -------------------------------------------------
#ifndef __DTS_H__
#define __DTS_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "main.h"

// Exported functions --------------------------------------------------------------------

STM_ERR_T MX_DTS_Init(void);
float     DTS_ReadTemperature(void);

#ifdef __cplusplus
}
#endif

#endif // __DTS_H__
