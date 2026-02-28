/**
  ****************************************************************************************
 * @file           : psram.h
 * @brief          : APS6408L-OBM OPI PSRAM driver for STM32H723ZG + NUCLEO-MEM
 *
 * Call sequence from main():
 *   1. MX_OCTOSPI1_Init()             // in octospi.c
 *   2. HAL_Delay(2)                   // tPU power-up
 *   3. PSRAM_GlobalReset()            // reset PSRAM mode registers to defaults
 *   4. PSRAM_DLYB_TuneDelayLine()    // Phase 1: measure period, apply DTR correction
 *   5. PSRAM_ConfigureMemory()        // read-back MR0-MR8 to verify bus health
 *   6. PSRAM_EnableMemMapped()
 *   7. PSRAM_BusVerify()              // byte + word + burst test on non-cached region
 *   8. PSRAM_DLYB_CalibratePhase()   // Phase 2: sweep SEL taps for optimal window
 *   9. PSRAM_WriteReadTest(...)       // full verification
 *
 * Hardware:  NUCLEO-H723ZG + NUCLEO-MEM (APS6408L on OCTOSPI1 Port 1)
 * Clock:     PLL2R = 200 MHz, prescaler = 4 -> 50 MHz OCTOSPI bus clock
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
#ifndef __PSRAM_H
#define __PSRAM_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "main.h"
#include "octospi.h"

// Memory map ----------------------------------------------------------------------------
#define PSRAM_BASE_ADDR  0x90000000U
#define PSRAM_TOTAL_SIZE 0x00800000U // 8 MB total

#define PSRAM_HEAP_BASE  0x90000000U // 4 MB, cached  (ThreadX byte pool)
#define PSRAM_HEAP_SIZE  0x00400000U

#define PSRAM_DMA_BASE   0x90400000U // 4 MB, non-cached (DMA buffers)
#define PSRAM_DMA_SIZE   0x00400000U

// APS6408L commands & timing ------------------------------------------------------------
// Legacy 8-bit SDR commands (used in memory-mapped mode at PSRAM defaults)
#define LINEAR_BURST_READ             0x20U
#define LINEAR_BURST_WRITE            0xA0U
#define DUMMY_CLOCK_CYCLES_SRAM_READ  5U // default LC=5, max 133 MHz
#define DUMMY_CLOCK_CYCLES_SRAM_WRITE 4U // default WLC=5, max 133 MHz (per AN5050)

// APS6408L commands (16-bit DTR format, byte doubled)
#define PSRAM_READ_CMD           0x0000
#define PSRAM_WRITE_CMD          0x8080
#define PSRAM_READ_LINEAR_CMD    0x2020
#define PSRAM_WRITE_LINEAR_CMD   0xA0A0
#define PSRAM_READ_REG_CMD       0x40 // 8-bit SDR (register access)
#define PSRAM_WRITE_REG_CMD      0xC0 // 8-bit SDR (register access)
#define PSRAM_RESET_CMD          0xFF // 8-bit SDR (global reset)

#define PSRAM_DUMMY_CYCLES_READ  8
#define PSRAM_DUMMY_CYCLES_WRITE 4

// APS6408L mode register addresses
#define PSRAM_MR0 0x00000000
#define PSRAM_MR1 0x00000001
#define PSRAM_MR2 0x00000002
#define PSRAM_MR3 0x00000003
#define PSRAM_MR4 0x00000004
#define PSRAM_MR6 0x00000006
#define PSRAM_MR8 0x00000008

// DLYB constants (DLYB_MAX_UNIT, DLYB_MAX_SELECT defined in stm32h7xx_ll_delayblock.h)

// Bus verification result (Session 7) ---------------------------------------------------
typedef struct
{
    uint8_t byte_write_read;  // 1 = pass, 0 = fail
    uint8_t word_write_read;  // 1 = pass, 0 = fail
    uint8_t burst_write_read; // 1 = pass, 0 = fail
    uint8_t byte_expected;
    uint8_t byte_actual;
    uint32_t word_expected;
    uint32_t word_actual;
    uint32_t burst_fail_index; // first failing word index (0xFFFFFFFF = none)
    uint32_t burst_expected;
    uint32_t burst_actual;
} PSRAM_BusVerify_t;

// Full test result ----------------------------------------------------------------------
typedef struct
{
    uint32_t error_count;
    uint32_t first_fail_addr;
    uint32_t first_fail_expected;
    uint32_t first_fail_actual;
    uint32_t pattern_index; // 0 = address-as-data, 1+ = constant pattern
} PSRAM_TestResult_t;

// Public functions ----------------------------------------------------------------------

// Bus verification (byte + word + burst on non-cached region)
PSRAM_BusVerify_t PSRAM_BusVerify(void);

// Register access (indirect mode, before memory-mapped)
uint32_t PSRAM_WriteReg(uint32_t Address, uint8_t *Value);
uint32_t PSRAM_ReadReg(uint32_t Address, uint8_t *Value, uint32_t DummyCycles);
void PSRAM_ConfigureMemory(void); // reads MR0-MR8, verifies bus health

// Memory-mapped mode
void PSRAM_EnableMemMapped(void);
void PSRAM_GlobalReset(void);

// DLYB calibration flow
void PSRAM_DLYB_TuneDelayLine(void);  // Phase 1: before PSRAM_EnableMemMapped()
void PSRAM_DLYB_CalibratePhase(void); // Phase 2: after PSRAM_EnableMemMapped()

// Full write/read verification
PSRAM_TestResult_t PSRAM_WriteReadTest(uint32_t base_addr, uint32_t size_bytes);

// DLYB calibration results (inspect in debugger)
extern volatile uint32_t dlyb_cal_min;
extern volatile uint32_t dlyb_cal_max;
extern volatile uint32_t dlyb_cal_center;

// DLYB tuning results (inspect in debugger)
extern volatile uint32_t dlyb_tuned_unit;
extern volatile uint32_t dlyb_tuned_sel_full;
extern volatile uint32_t dlyb_tuned_sel_dtr;

// Bus verification result (inspect in debugger)
extern volatile PSRAM_BusVerify_t psram_bus_result;

#ifdef __cplusplus
}
#endif

#endif // __PSRAM_H
