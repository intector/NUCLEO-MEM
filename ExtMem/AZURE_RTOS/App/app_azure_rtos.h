/**
  ****************************************************************************************
 * @file    app_azure_rtos.h
 * @brief   app_azure_rtos application header file
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
#ifndef APP_AZURE_RTOS_H
#define APP_AZURE_RTOS_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "app_azure_rtos_config.h"
#include "app_threadx.h"
#include "app_filex.h"
#include "app_netxduo.h"
#include "main.h"
extern TX_EVENT_FLAGS_GROUP TAGID_status_event_group;
extern TX_EVENT_FLAGS_GROUP TAGID_ctrl_event_group;

// status events definitions
#define TAGID_SE_App_ThreadX_Init_OK        0x00000001UL    // 0x0000_0001UL
#define TAGID_SE_tx_app_main_thread         0x00000002UL    // 0x0000_0002UL
#define TAGID_SE_tx_app_NFCV_Scanner_thread 0x00000004UL    // 0x0000_0004UL
#define TAGID_SE_tx_app_OLED_thread         0x00000008UL    // 0x0000_0008UL

#define TAGID_SE_0x0000_0000_0000_0010      0x00000010UL    // 0x0000_0010UL
#define TAGID_SE_0x0000_0000_0000_0020      0x00000020UL    // 0x0000_0020UL
#define TAGID_SE_0x0000_0000_0000_0040      0x00000040UL    // 0x0000_0040UL
#define TAGID_SE_0x0000_0000_0000_0080      0x00000080UL    // 0x0000_0080UL

#define TAGID_SE_MX_FileX_Init              0x00000100UL    // 0x0000_0100UL
#define TAGID_SE_EMMC_INIT_OK               0x00000200UL    // 0x0000_0200UL
#define TAGID_SE_0x0000_0000_0000_0400      0x00000400UL    // 0x0000_0400UL
#define TAGID_SE_0x0000_0000_0000_0800      0x00000800UL    // 0x0000_0800UL

#define TAGID_SE_0x0000_0000_0000_1000      0x00001000UL    // 0x0000_1000UL
#define TAGID_SE_0x0000_0000_0000_2000      0x00002000UL    // 0x0000_2000UL
#define TAGID_SE_0x0000_0000_0000_4000      0x00004000UL    // 0x0000_4000UL
#define TAGID_SE_0x0000_0000_0000_8000      0x00008000UL    // 0x0000_8000UL

#define TAGID_SE_NX_INIT_OK                 0x00010000UL    // 0x0001_0000UL
#define TAGID_SE_IP_ADDR_OK                 0x00020000UL    // 0x0002_0000UL
#define TAGID_SE_HTTP_SERVER_OK             0x00040000UL    // 0x0004_0000UL
#define TAGID_SE_FTP_SERVER_OK              0x00080000UL    // 0x0008_0000UL

#define TAGID_SE_WS_SERVER_OK               0x00100000UL    // 0x0010_0000UL
#define TAGID_SE_0x0000_0000_0020_0000      0x00200000UL    // 0x0020_0000UL
#define TAGID_SE_0x0000_0000_0040_0000      0x00400000UL    // 0x0040_0000UL
#define TAGID_SE_0x0000_0000_0080_0000      0x00800000UL    // 0x0080_0000UL

#define TAGID_SE_LABEL_DEF_OK               0x01000000UL    // 0x0100_0000UL
#define TAGID_SE_0x0000_0002_0000_0000      0x02000000UL    // 0x0200_0000UL
#define TAGID_SE_0x0000_0004_0000_0000      0x04000000UL    // 0x0400_0000UL
#define TAGID_SE_0x0000_0008_0000_0000      0x08000000UL    // 0x0800_0000UL

// control events definitions
#define TAGID_CE_NFCV_SCANNING_START        0x00000001UL    // 0x0000_0001UL
#define TAGID_CE_NFCV_FTM_START             0x00000002UL    // 0x0000_0002UL
#define TAGID_CE_0x0000_0000_0000_0004      0x00000004UL    // 0x0000_0004UL
#define TAGID_CE_0x0000_0000_0000_0008      0x00000008UL    // 0x0000_0008UL

#define TAGID_CE_0x0000_0000_0000_0010      0x00000010UL    // 0x0000_0010UL
#define TAGID_CE_0x0000_0000_0000_0020      0x00000020UL    // 0x0000_0020UL
#define TAGID_CE_0x0000_0000_0000_0040      0x00000040UL    // 0x0000_0040UL
#define TAGID_CE_0x0000_0000_0000_0080      0x00000080UL    // 0x0000_0080UL

#ifdef __cplusplus
}
#endif

#endif // APP_AZURE_RTOS_H
