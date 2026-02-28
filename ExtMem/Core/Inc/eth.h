/**
  ****************************************************************************************
  * @file           : eth.h
  * @brief          : This file contains all the function prototypes for
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
#ifndef __ETH_H__
#define __ETH_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "main.h"
#include "string.h"
#include "app_netxduo.h"


extern ETH_HandleTypeDef heth;
	
extern UINT PingCnt;

void MX_ETH_Init(void);



#ifdef __cplusplus
}
#endif

#endif // __ETH_H__

