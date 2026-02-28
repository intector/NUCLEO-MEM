/**
  ****************************************************************************************
  * @file           : perf_test.c
  * @brief          : perf_test.c - PSRAM and eMMC throughput benchmarks for ThreadX
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

// perf_test.c - PSRAM and eMMC throughput benchmarks for ThreadX
//
// Usage: call from any ThreadX thread after PSRAM and eMMC are initialized
//   PERF_RunPSRAMTest();
//   PERF_RunEMMCTest();

#include "perf_test.h"
#include "main.h"
#include "app_filex.h"
#include <string.h>
#include <stdio.h>

// DWT cycle counter helpers
static inline void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t dwt_get(void)
{
    return DWT->CYCCNT;
}

static float cycles_to_ms(uint32_t cycles)
{
    return (float)cycles / (SystemCoreClock / 1000.0f);
}

static float calc_mbps(uint32_t bytes, uint32_t cycles)
{
    float seconds = (float)cycles / (float)SystemCoreClock;
    return (float)bytes / (1024.0f * 1024.0f * seconds);
}

// eMMC test buffer in AXI SRAM for DMA access
__attribute__((section(".dma_buffer"))) static uint8_t emmc_test_buf[EMMC_SECTOR_SIZE * 8];

void PERF_RunPSRAMTest(void)
{
    uint32_t start, elapsed;
    volatile uint32_t *dst;
    volatile uint32_t *src;
    uint32_t i;
    float ms, mbps;
    uint32_t test_words = PSRAM_TEST_SIZE / sizeof(uint32_t);

    dwt_init();

    printf("\n===== PSRAM Speed Test =====\r\n");
    printf("CPU Clock: %lu MHz\r\n", SystemCoreClock / 1000000UL);
    printf("Test size: %u KB\r\n\n", PSRAM_TEST_SIZE / 1024);

    // --- Cached region write (word-by-word) ---
    dst = (volatile uint32_t *)PSRAM_CACHED_BASE;
    start = dwt_get();
    for (i = 0; i < test_words; i++)
        dst[i] = i;
    elapsed = dwt_get() - start;
    ms = cycles_to_ms(elapsed);
    mbps = calc_mbps(PSRAM_TEST_SIZE, elapsed);
    printf("Cached Write:      %8.2f ms  %6.1f MB/s\r\n", ms, mbps);

    // --- Cached region read (word-by-word) ---
    volatile uint32_t sink = 0;
    start = dwt_get();
    for (i = 0; i < test_words; i++)
        sink += dst[i];
    elapsed = dwt_get() - start;
    ms = cycles_to_ms(elapsed);
    mbps = calc_mbps(PSRAM_TEST_SIZE, elapsed);
    printf("Cached Read:       %8.2f ms  %6.1f MB/s\r\n", ms, mbps);
    (void)sink;

    // --- Non-cached region write (word-by-word) ---
    dst = (volatile uint32_t *)PSRAM_NONCACHED_BASE;
    start = dwt_get();
    for (i = 0; i < test_words; i++)
        dst[i] = i;
    elapsed = dwt_get() - start;
    ms = cycles_to_ms(elapsed);
    mbps = calc_mbps(PSRAM_TEST_SIZE, elapsed);
    printf("Non-cached Write:  %8.2f ms  %6.1f MB/s\r\n", ms, mbps);

    // --- Non-cached region read (word-by-word) ---
    sink = 0;
    start = dwt_get();
    for (i = 0; i < test_words; i++)
        sink += dst[i];
    elapsed = dwt_get() - start;
    ms = cycles_to_ms(elapsed);
    mbps = calc_mbps(PSRAM_TEST_SIZE, elapsed);
    printf("Non-cached Read:   %8.2f ms  %6.1f MB/s\r\n", ms, mbps);
    (void)sink;

    // --- Cached memcpy write ---
    dst = (volatile uint32_t *)PSRAM_CACHED_BASE;
    uint32_t local_buf[256]; // 1 KB on stack
    memset(local_buf, 0xAA, sizeof(local_buf));
    start = dwt_get();
    for (i = 0; i < PSRAM_TEST_SIZE / sizeof(local_buf); i++)
        memcpy((void *)(dst + i * 256), local_buf, sizeof(local_buf));
    elapsed = dwt_get() - start;
    ms = cycles_to_ms(elapsed);
    mbps = calc_mbps(PSRAM_TEST_SIZE, elapsed);
    printf("Cached memcpy:     %8.2f ms  %6.1f MB/s\r\n", ms, mbps);

    // --- Non-cached memcpy write ---
    dst = (volatile uint32_t *)PSRAM_NONCACHED_BASE;
    start = dwt_get();
    for (i = 0; i < PSRAM_TEST_SIZE / sizeof(local_buf); i++)
        memcpy((void *)(dst + i * 256), local_buf, sizeof(local_buf));
    elapsed = dwt_get() - start;
    ms = cycles_to_ms(elapsed);
    mbps = calc_mbps(PSRAM_TEST_SIZE, elapsed);
    printf("Non-cached memcpy: %8.2f ms  %6.1f MB/s\r\n", ms, mbps);

    printf("===========================\r\n\n");
}

void PERF_RunEMMCTest(void)
{
    uint32_t start, elapsed;
    float ms, mbps;
    FX_FILE test_file;
    UINT status;
    ULONG bytes_read;

    uint32_t total_size = EMMC_TEST_SECTORS * EMMC_SECTOR_SIZE; // 128 KB
    uint32_t chunk_size = sizeof(emmc_test_buf);
    uint32_t iterations = total_size / chunk_size;

    dwt_init();

    printf("\n===== eMMC Speed Test (FileX) =====\r\n");
    printf("CPU Clock: %lu MHz\r\n", SystemCoreClock / 1000000UL);
    printf("Test size: %lu KB, chunk: %lu bytes\r\n", total_size / 1024, chunk_size);
    printf("Clock divider: 2 (target ~100 MHz)\r\n\n");

    // fill test buffer with pattern
    for (uint32_t i = 0; i < chunk_size; i++)
        emmc_test_buf[i] = (uint8_t)(i & 0xFF);

    // --- Sequential Write ---
    status = fx_file_create(&mmc_disk, "_perftest.bin");
    if (status != FX_SUCCESS && status != FX_ALREADY_CREATED)
    {
        printf("eMMC file create failed: 0x%02X\r\n", status);
        return;
    }

    status = fx_file_open(&mmc_disk, &test_file, "_perftest.bin", FX_OPEN_FOR_WRITE);
    if (status != FX_SUCCESS)
    {
        printf("eMMC file open for write failed: 0x%02X\r\n", status);
        return;
    }

    fx_file_seek(&test_file, 0);

    start = dwt_get();
    for (uint32_t i = 0; i < iterations; i++)
    {
        status = fx_file_write(&test_file, emmc_test_buf, chunk_size);
        if (status != FX_SUCCESS)
        {
            printf("eMMC Write FAILED at iteration %lu: 0x%02X\r\n", i, status);
            fx_file_close(&test_file);
            return;
        }
    }
    fx_media_flush(&mmc_disk);
    elapsed = dwt_get() - start;
    ms = cycles_to_ms(elapsed);
    mbps = calc_mbps(total_size, elapsed);
    printf("Sequential Write:  %8.2f ms  %6.2f MB/s\r\n", ms, mbps);

    fx_file_close(&test_file);

    // --- Sequential Read ---
    status = fx_file_open(&mmc_disk, &test_file, "_perftest.bin", FX_OPEN_FOR_READ);
    if (status != FX_SUCCESS)
    {
        printf("eMMC file open for read failed: 0x%02X\r\n", status);
        return;
    }

    fx_file_seek(&test_file, 0);
    memset(emmc_test_buf, 0, chunk_size);

    start = dwt_get();
    for (uint32_t i = 0; i < iterations; i++)
    {
        status = fx_file_read(&test_file, emmc_test_buf, chunk_size, &bytes_read);
        if (status != FX_SUCCESS || bytes_read != chunk_size)
        {
            printf("eMMC Read FAILED at iteration %lu: 0x%02X\r\n", i, status);
            fx_file_close(&test_file);
            return;
        }
    }
    elapsed = dwt_get() - start;
    ms = cycles_to_ms(elapsed);
    mbps = calc_mbps(total_size, elapsed);
    printf("Sequential Read:   %8.2f ms  %6.2f MB/s\r\n", ms, mbps);

    // verify last chunk
    uint32_t errors = 0;
    for (uint32_t i = 0; i < chunk_size; i++)
    {
        if (emmc_test_buf[i] != (uint8_t)(i & 0xFF))
            errors++;
    }
    printf("Verify: %s (%lu errors in last chunk)\r\n", errors ? "FAIL" : "PASS", errors);

    fx_file_close(&test_file);

    // cleanup
    fx_file_delete(&mmc_disk, "_perftest.bin");

    printf("===================================\r\n\n");
}
