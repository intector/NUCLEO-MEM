/**
  ******************************************************************************
  * @file    app_ftp.h
  * @author  INTECTOR
  * @brief   ftp applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 INTECTOR.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_FTP_H__
#define __APP_FTP_H__

#ifdef __cplusplus
extern "C" {
#endif

	/* Includes ------------------------------------------------------------------*/
#include "nx_api.h"
#include "app_azure_rtos.h"
#include "nxd_ftp_server.h"
#include "fx_stm32_sd_driver.h"

	/* Exported types ------------------------------------------------------------*/

	/* Exported constants --------------------------------------------------------*/

	/* Exported macro ------------------------------------------------------------*/

	/* Exported functions prototypes ---------------------------------------------*/
	UINT MX_FTP_Init(VOID *memory_ptr);

	/* Private defines -----------------------------------------------------------*/

#define FTP_STACK_SIZE				4096
#define FTP_SERVER_ADDRESS			IP_ADDRESS(192,168,0,50)
	
/* FTP return values.  */

#define FTP_SUCCESS					0x00



#ifdef __cplusplus
}
#endif
#endif /* __APP_FTP_H__ */
