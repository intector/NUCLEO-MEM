/**
  ****************************************************************************************
  * @file           : app_filex.c
  * @brief          : FileX applicative file
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
#include "app_filex.h"
#include "perf_test.h"

// Private defines -----------------------------------------------------------------------

// Private variables ---------------------------------------------------------------------

// Main thread global data structures.
TX_THREAD       fx_app_thread;

// Buffer for FileX FX_MEDIA sector cache.

// ALIGN_32BYTES(uint32_t fx_mmc_media_memory[FX_STM32_MMC_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);
__attribute__((section(".dma_buffer"))) ALIGN_32BYTES(uint32_t fx_mmc_media_memory[FX_STM32_MMC_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);

// Define FileX global data structures ---------------------------------------------------
FX_MEDIA mmc_disk;
extern MMC_HandleTypeDef hmmc1;
HAL_MMC_CardInfoTypeDef *pCardInfo;
EMMC_Status eMMC_Stat;

// Private function prototypes -----------------------------------------------------------

// Main thread entry function.
void fx_app_thread_entry(ULONG thread_input);

UINT MX_FileX_Init(VOID *memory_ptr)
{
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

    // Initialize FileX.
    fx_system_initialize();

#if FX_THREAD_ENABLE
 
    VOID *pointer;

    // allocating memory for the FileX main thread's stack
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, &pointer, FX_APP_THREAD_STACK_SIZE, TX_NO_WAIT));

    // create the FileX main thread
    STM32_ERROR_CHECK(ERR_Tx_ThreadCreate, tx_thread_create(&fx_app_thread,
        FX_APP_THREAD_NAME,
        fx_app_thread_entry,
        0,
        pointer,
        FX_APP_THREAD_STACK_SIZE,
        FX_APP_THREAD_PRIO,
        FX_APP_PREEMPTION_THRESHOLD,
        FX_APP_THREAD_TIME_SLICE,
        FX_APP_THREAD_AUTO_START));

    // set TAGID_SE_MX_FileX_Init event flag
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_MX_FileX_Init, TX_OR));

#endif // FILEX_THREAD_ENABLE

    return FX_SUCCESS;
}

void fx_app_thread_entry(ULONG thread_input)
{
    UNUSED(thread_input); // avoid gcc/g++ warnings
    UINT mmc_status = FX_SUCCESS;
    UINT ret = FX_SUCCESS;
    UINT is_formatted = MMC_FORMAT_NOK;

    ULONG _EventFlags = 0x00000000UL;
    // ULONG _EventFlags_ALL = 0xFFFFFFFFUL;
    ULONG tmp_actual_events = 0x00000000UL;

    // ---------- getting all actual event flags ----------
    // STM32_ERROR_CHECK(tx_event_flags_get(&TAGID_status_event_group, _EventFlags_ALL, TX_OR, &tmp_actual_events, TX_NO_WAIT));

    _EventFlags = TAGID_SE_tx_app_OLED_thread;
    _EventFlags |= TAGID_SE_App_ThreadX_Init_OK | TAGID_SE_tx_app_main_thread;
//    _EventFlags &= tmp_actual_events;

    // wait for fileX relevant event flags
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsGet, tx_event_flags_get(&TAGID_status_event_group, _EventFlags, TX_AND, &tmp_actual_events, TX_WAIT_FOREVER));

    // open the MMC disk driver.
    ret = fx_media_open(&mmc_disk, FX_MMC_VOLUME_NAME, fx_stm32_mmc_driver, SD_DRIVER_INFO_POINTER, fx_mmc_media_memory, sizeof(fx_mmc_media_memory));

    if (ret == FX_SUCCESS)
    {
        is_formatted = MMC_FORMAT_OK;
    }
    
    // checking the EMMC-Chip info
    STM32_ERROR_CHECK(ERR_HAL_MMC_GetCardInfo, HAL_MMC_GetCardInfo(&hmmc1, pCardInfo));

    if (is_formatted == MMC_FORMAT_NOK)
    {
        // make sure the MMC is closed
        ret = fx_media_close(&mmc_disk);

        // formatting the MMC memory as FAT
        ret = fx_media_format(&mmc_disk,                // MMC_Disk pointer
            fx_stm32_mmc_driver,                        // Driver entry
            (VOID *)FX_NULL,                            // Device info pointer
            (UCHAR *) fx_mmc_media_memory,              // Media buffer pointer
            sizeof(fx_mmc_media_memory),                // Media buffer size
            FX_MMC_VOLUME_NAME,                         // Volume Name
            FX_MMC_NUMBER_OF_FATS,                      // Number of FATs
            32,                                         // Directory Entries
            FX_MMC_HIDDEN_SECTORS,                      // Hidden sectors
            pCardInfo->BlockNbr,                        // Total sectors
            pCardInfo->BlockSize,                       // Sector size
            8,                                          // Sectors per cluster
            1,                                          // Heads
            1);                                         // Sectors per track

        if (ret != FX_SUCCESS)
        {
            while (1)
            {
                // formatting went wrong....
                Error_Handler(ERR_FX_MediaOpen, STM32_FAIL);
            }
        }

        // open the MMC disk driver.
        STM32_ERROR_CHECK(ERR_FX_MediaOpen, fx_media_open(&mmc_disk, FX_MMC_VOLUME_NAME, fx_stm32_mmc_driver, SD_DRIVER_INFO_POINTER, fx_mmc_media_memory, sizeof(fx_mmc_media_memory)));
        
    }

    // getting the available memory on the MMC
    GET_FREE_EMMC_MEM(mmc_disk, eMMC_Stat);

    // STM32_ERROR_CHECK(_fx_media_extended_space_available(&mmc_disk, &eMMC_Stat.MEM_Free_Bytes));
    // eMMC_Stat.MEM_Free_kB = eMMC_Stat.MEM_Free_Bytes / 1024;
    // eMMC_Stat.MEM_Free_MB = eMMC_Stat.MEM_Free_kB / 1024;
    // eMMC_Stat.MEM_Free_GB = eMMC_Stat.MEM_Free_GB / 1024;

    // set EMMC init OK event flag
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_EMMC_INIT_OK, TX_OR));

    // memory performance test ------------------------------------------------
    PERF_RunPSRAMTest();
    PERF_RunEMMCTest();

    while (1)
    {
        // doing something here later...
        tx_thread_sleep(1000);
    }

}

