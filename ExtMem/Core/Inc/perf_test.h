/**
  ****************************************************************************************
  * @file           : perf_test.h
  * @brief          : perf_test.h - PSRAM and eMMC throughput benchmarks for ThreadX
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

// perf_test.h - PSRAM and eMMC throughput benchmarks for ThreadX

#ifndef __PERF_TEST_H__
#define __PERF_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

// PSRAM test buffer addresses
#define PSRAM_CACHED_BASE    0x901C2000UL // Mem_Buff1 (unused, cached)
#define PSRAM_NONCACHED_BASE 0x90431000UL // Mem_Buff2 (unused, non-cached)
#define PSRAM_TEST_SIZE       (64 * 1024)

// eMMC test parameters
#define EMMC_TEST_SECTORS     256
#define EMMC_SECTOR_SIZE      512
#define EMMC_TEST_SIZE        (EMMC_TEST_SECTORS * EMMC_SECTOR_SIZE)

// Run 64 KB PSRAM throughput test (cached vs non-cached, word vs memcpy)
// Prints results via printf
void PERF_RunPSRAMTest(void);

// Run 128 KB eMMC sequential read/write throughput test
// Uses sectors 2048+ to avoid FAT structures
// Prints results via printf
void PERF_RunEMMCTest(void);

#ifdef __cplusplus
}
#endif

#endif // __PERF_TEST_H__
