/**
  ****************************************************************************************
 * @file    app_azure_rtos_config.h
 * @brief   azure_rtos config header file
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
#ifndef APP_AZURE_RTOS_CONFIG_H
#define APP_AZURE_RTOS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


// Using static memory allocation via threadX Byte memory pools
#define USE_STATIC_ALLOCATION                    1
#define TX_APP_MEM_POOL_SIZE                     1024 * 500
#define FX_APP_MEM_POOL_SIZE                     1024 * 500
#define NX_APP_MEM_POOL_SIZE                     1024 * 500




#ifdef __cplusplus
}
#endif

#endif // APP_AZURE_RTOS_CONFIG_H
