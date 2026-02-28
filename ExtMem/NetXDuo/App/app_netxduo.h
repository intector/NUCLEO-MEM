/**
  ****************************************************************************************
  * @file           : app_netxduo.h
  * @brief          : NetXDuo applicative header file
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
#ifndef __APP_NETXDUO_H__
#define __APP_NETXDUO_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "nx_api.h"
#include "tx_api.h"

// Private includes ----------------------------------------------------------------------
#include "nx_stm32_eth_driver.h"
#include "main.h"
//#include "app_filex.h"
#include "nx_web_http_server.h"
#include "nxd_ftp_server.h"
#include "app_filex.h"

// External defines ----------------------------------------------------------------------
// Buffer for FileX FX_MEDIA sector cache.
extern ALIGN_32BYTES(uint32_t fx_mmc_media_memory[FX_STM32_MMC_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
// extern MMC_HandleTypeDef hmmc1;
// extern FX_MEDIA mmc_disk;


// Exported constants --------------------------------------------------------------------
// The DEFAULT_PAYLOAD_SIZE should match with RxBuffLen configured via MX_ETH_Init
#ifndef DEFAULT_PAYLOAD_SIZE
#define DEFAULT_PAYLOAD_SIZE      1536
#endif

#ifndef DEFAULT_ARP_CACHE_SIZE
#define DEFAULT_ARP_CACHE_SIZE    1024
#endif

// Exported functions prototypes ---------------------------------------------------------
UINT MX_NetXDuo_Init(VOID *memory_ptr);

// Private defines -----------------------------------------------------------------------
#define DEFAULT_MEMORY_SIZE				1024

#define NX_APP_DEFAULT_TIMEOUT			(10 * NX_IP_PERIODIC_RATE)

// ETH DMA-accessible packet pool (must be in AXI SRAM, not PSRAM)
#define NX_ETH_PACKET_COUNT             60
#define NX_ETH_PACKET_POOL_SIZE         ((DEFAULT_PAYLOAD_SIZE + sizeof(NX_PACKET)) * NX_ETH_PACKET_COUNT)

#define NX_APP_THREAD_STACK_SIZE		1024 * 2

// #define NX_PACKET_POOL_SIZE				((1536 + sizeof(NX_PACKET)) * 50)

#define Nx_IP_INSTANCE_THREAD_SIZE		1024 * 4

#define NX_APP_THREAD_PRIORITY			10
#define LINK_PRIORITY					11

#ifndef NX_APP_INSTANCE_PRIORITY
#define NX_APP_INSTANCE_PRIORITY		NX_APP_THREAD_PRIORITY
#endif

// Server pool size 
#define HTTP_SRV_PACKET_POOL_SIZE		1024 * 30
#define FTP_SRV_PACKET_POOL_SIZE		1024 * 100


/// #define NULL_IP_ADDRESS					IP_ADDRESS(0,0,0,0)
/// #define NX_APP_DEFAULT_IP_ADDRESS		IP_ADDRESS(192, 168, 0, 50)
/// #define NX_APP_DEFAULT_NET_MASK			IP_ADDRESS(255, 255, 255, 0)


// HTTP connection port 
/// #define CONNECTION_PORT					80
// HTTP Server stack 
#define HTTP_SRV_STACK_SIZE				4096
// FTP Server stack 
#define FTP_SRV_STACK_SIZE				4096

// Server packet size (match ETH payload for full MSS utilization)
#define HTTP_SERVER_PACKET_SIZE			DEFAULT_PAYLOAD_SIZE
#define FTP_SERVER_PACKET_SIZE			(NX_FTP_SERVER_MIN_PACKET_PAYLOAD * 2)

// SD Driver information pointer 
#define SD_DRIVER_INFO_POINTER			0


#ifdef __cplusplus
}
#endif
#endif // __APP_NETXDUO_H__
