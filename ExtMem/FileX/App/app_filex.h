/**
  ****************************************************************************************
  * @file           : app_filex.h
  * @brief          : FileX applicative header file
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
#ifndef __APP_FILEX_H__
#define __APP_FILEX_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "fx_api.h"
#include "fx_media.h"
#include "fx_stm32_mmc_driver.h"
#include "main.h"
#include "app_azure_rtos.h"


// Exported functions prototypes ---------------------------------------------------------
UINT MX_FileX_Init(VOID *memory_ptr);

// type defines --------------------------------------------------------------------------
typedef struct
{
    ULONG64 MEM_Free_Bytes;
    float MEM_Free_kB;
    float MEM_Free_MB;
    float MEM_Free_GB;
} EMMC_Status;

// Buffer for FileX FX_MEDIA sector cache.
extern ALIGN_32BYTES(uint32_t fx_mmc_media_memory[FX_STM32_MMC_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
extern FX_MEDIA mmc_disk;
extern MMC_HandleTypeDef hmmc1;
extern HAL_MMC_CardInfoTypeDef *pCardInfo;
extern EMMC_Status eMMC_Stat;

#define GET_FREE_EMMC_MEM(emmc, emmc_stat)                                                       \
    {                                                                                            \
        STM32_ERROR_CHECK(ERR_FX_MediaSpaceAvailable, _fx_media_extended_space_available(&emmc, &emmc_stat.MEM_Free_Bytes)); \
        emmc_stat.MEM_Free_kB = emmc_stat.MEM_Free_Bytes / 1024;                                 \
        emmc_stat.MEM_Free_MB = emmc_stat.MEM_Free_kB / 1024;                                    \
        emmc_stat.MEM_Free_GB = emmc_stat.MEM_Free_GB / 1024;                                    \
    }

#define MMC_FORMAT_NOK      0
#define MMC_FORMAT_OK       1

// Main FileX thread Name
#ifndef FX_APP_THREAD_NAME
    #define FX_APP_THREAD_NAME "FileX app thread"
#endif

// Main FileX thread time slice
#ifndef FX_APP_THREAD_TIME_SLICE
    #define FX_APP_THREAD_TIME_SLICE TX_NO_TIME_SLICE
#endif

// Main FileX thread auto start
#ifndef FX_APP_THREAD_AUTO_START
    #define FX_APP_THREAD_AUTO_START TX_AUTO_START
#endif

// Main FileX thread preemption threshold
#ifndef FX_APP_PREEMPTION_THRESHOLD
    #define FX_APP_PREEMPTION_THRESHOLD FX_APP_THREAD_PRIO
#endif

// Main FileX thread stack size
#ifndef FX_APP_THREAD_STACK_SIZE
    #define FX_APP_THREAD_STACK_SIZE    1024 * 2
#endif

// Main FileX thread priority
#ifndef FX_APP_THREAD_PRIO
    #define FX_APP_THREAD_PRIO          10
#endif

// fx mmc volume name
#ifndef FX_MMC_VOLUME_NAME
    #define FX_MMC_VOLUME_NAME "STM32_MMC_DISK"
#endif

// fx mmc number of FATs
#ifndef FX_MMC_NUMBER_OF_FATS
    #define FX_MMC_NUMBER_OF_FATS 1
#endif

// fx mmc Hidden sectors
#ifndef FX_MMC_HIDDEN_SECTORS
    #define FX_MMC_HIDDEN_SECTORS 0
#endif


#ifdef __cplusplus
}
#endif
#endif // __APP_FILEX_H__
