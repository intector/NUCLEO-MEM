/**
  ****************************************************************************************
 * @file           : psram.c
 * @brief          : APS6408L-OBM OPI PSRAM driver for STM32H723ZG + NUCLEO-MEM
 *
 * DLYB calibration, memory-mapped mode, bus verification, and write/read test.
 * Peripheral initialization (GPIO, clocks, OSPIM) is in octospi.c.
 *
 * Hardware:  NUCLEO-H723ZG + NUCLEO-MEM (APS6408L on OCTOSPI1 Port 1)
 * Clock:     PLL2R = 200 MHz, prescaler = 4 -> 50 MHz OCTOSPI bus clock
 *
 * Session 15 changes:
 *   - Ported U5 HAL DLYB wrapper pattern (FRCK + Abort + CLEAR_BIT)
 *   - Eliminated DeInit/ReInit workaround that corrupted DLYB UNIT
 *
 * Session 16 changes (U5 reference alignment):
 *   - Added DLYB zero-init before GetClockPeriod (matches U5 MX_OCTOSPI1_Init)
 *   - Added DLYB readback verification after SetConfig (matches U5 main.c)
 *   - Removed extra Address/NbData from EnableMemMapped (matches U5 main.c)
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
#include "psram.h"
#include "psram_diag.h"
#include "stm32h7xx_ll_delayblock.h"
#include <string.h>

// Private variables ---------------------------------------------------------------------
volatile uint32_t dlyb_cal_min    = 0xFF;
volatile uint32_t dlyb_cal_max    = 0xFF;
volatile uint32_t dlyb_cal_center = 0xFF;

volatile PSRAM_BusVerify_t psram_bus_result;
volatile PSRAM_Diag_t psram_diag;

// Diagnostic globals for debugger
volatile uint32_t dlyb_tuned_unit     = 0;
volatile uint32_t dlyb_tuned_sel_full = 0; // taps spanning one period
volatile uint32_t dlyb_tuned_sel_dtr  = 0; // quarter-period for DTR

// ---------------------------------------------------------------------------------------
// DLYB HAL Wrappers -- ported from stm32u5xx_hal_ospi.c
//
// The U5 HAL has wrapper functions that manage the FRCK (free-running clock)
// lifecycle around DLYB operations.  The H7 HAL does NOT have these.
//
// Key insight: HAL_OSPI_Abort() clears the BUSY state so that DCR1 (which
// contains FRCK) becomes writable.  This avoids the DeInit/ReInit workaround
// that was corrupting DLYB UNIT (34 -> 54).
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
// DLYB HAL wrapper: measure clock period
// Mirrors U5 HAL_OSPI_DLYB_GetClockPeriod()
// Sequence: FRCK on -> enable DLYB -> measure -> disable DLYB -> Abort -> FRCK off
// ---------------------------------------------------------------------------------------
static HAL_StatusTypeDef PSRAM_DLYB_GetClockPeriod(LL_DLYB_CfgTypeDef *pdlyb_cfg)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    // Enable free-running clock (mandatory for DLYB measurement)
    SET_BIT(OCTOSPI1->DCR1, OCTOSPI_DCR1_FRCK);

    // Mark OCTOSPI as busy (matches U5 HAL pattern)
    hospi1.State = HAL_OSPI_STATE_BUSY_CMD;

    // Enable DLYB
    SET_BIT(DLYB_OCTOSPI1->CR, DLYB_CR_DEN);

    // Run RM-compliant period measurement (U5 LL algorithm)
    if (DelayBlock_GetClockPeriod(DLYB_OCTOSPI1, pdlyb_cfg) == HAL_OK)
        status = HAL_OK;

    // Disable DLYB (measurement done, will re-enable in SetConfig)
    DLYB_OCTOSPI1->CR = 0U;

    // Abort clears BUSY state -- this is the key U5 trick that makes
    // DCR1 writable again so we can clear FRCK without DeInit/ReInit
    (void)HAL_OSPI_Abort(&hospi1);

    // Now FRCK can be cleared (DCR1 writable when BUSY=0)
    CLEAR_BIT(OCTOSPI1->DCR1, OCTOSPI_DCR1_FRCK);

    return status;
}

// ---------------------------------------------------------------------------------------
// DLYB HAL wrapper: apply delay configuration
// Mirrors U5 HAL_OSPI_DLYB_SetConfig()
// Sequence: FRCK on -> enable DLYB -> write delay -> Abort -> FRCK off
// Note: leaves DLYB enabled (DEN=1, SEN=0) for data sampling
// ---------------------------------------------------------------------------------------
static HAL_StatusTypeDef PSRAM_DLYB_SetConfig(LL_DLYB_CfgTypeDef *pdlyb_cfg)
{
    // Enable free-running clock (mandatory for DLYB register writes)
    SET_BIT(OCTOSPI1->DCR1, OCTOSPI_DCR1_FRCK);

    // Mark OCTOSPI as busy
    hospi1.State = HAL_OSPI_STATE_BUSY_CMD;

    // Enable DLYB
    SET_BIT(DLYB_OCTOSPI1->CR, DLYB_CR_DEN);

    // Apply the delay setting (SEN=1 -> write CFGR -> SEN=0)
    DelayBlock_SetDelay(DLYB_OCTOSPI1, pdlyb_cfg);

    // Abort clears BUSY state
    (void)HAL_OSPI_Abort(&hospi1);

    // Clear FRCK -- DLYB stays enabled with output clock active
    CLEAR_BIT(OCTOSPI1->DCR1, OCTOSPI_DCR1_FRCK);

    return HAL_OK;
}

// ---------------------------------------------------------------------------------------
// DLYB HAL wrapper: read back current delay configuration
// Mirrors U5 HAL_OSPI_DLYB_GetConfig()
// Reads UNIT and SEL from DLYB CFGR register (no FRCK needed for read)
// ---------------------------------------------------------------------------------------
static void PSRAM_DLYB_GetConfig(LL_DLYB_CfgTypeDef *pdlyb_cfg)
{
    uint32_t cfgr       = DLYB_OCTOSPI1->CFGR;
    pdlyb_cfg->Units    = (cfgr >> DLYB_CFGR_UNIT_Pos) & 0x7FU;
    pdlyb_cfg->PhaseSel = (cfgr >> DLYB_CFGR_SEL_Pos) & 0x0FU;
}

// ---------------------------------------------------------------------------------------
// DLYB Phase 1: measure clock period and apply quarter-period correction
// Must be called BEFORE PSRAM_EnableMemMapped()
// ---------------------------------------------------------------------------------------
void PSRAM_DLYB_TuneDelayLine(void)
{
    LL_DLYB_CfgTypeDef dlyb_cfg = {0};

    // Zero-init DLYB to known state before measurement (matches U5 MX_OCTOSPI1_Init)
    // Prevents stale values from warm reset / debug restart
    PSRAM_DLYB_SetConfig(&dlyb_cfg);

    // Phase 1: Measure clock period
    // Wrapper handles: FRCK on -> measure -> FRCK off (no DeInit needed)
    if (PSRAM_DLYB_GetClockPeriod(&dlyb_cfg) != HAL_OK)
        Error_Handler(ERR_PSRAM_DLYB_Tune, STM32_FAIL);

    // Save full-period measurement for debugger inspection
    dlyb_tuned_unit     = dlyb_cfg.Units;
    dlyb_tuned_sel_full = dlyb_cfg.PhaseSel;

    // Phase 2: Apply DTR quarter-period correction
    // For DDR sampling, we need ~1/4 period shift to center in data eye
    dlyb_cfg.PhaseSel /= 4;
    if (dlyb_cfg.PhaseSel == 0)
        dlyb_cfg.PhaseSel = 1;
    dlyb_tuned_sel_dtr = dlyb_cfg.PhaseSel;

    // Save expected values before applying
    LL_DLYB_CfgTypeDef dlyb_cfg_expected = dlyb_cfg;

    // Apply corrected delay
    // Wrapper handles: FRCK on -> apply -> FRCK off (leaves DLYB enabled)
    PSRAM_DLYB_SetConfig(&dlyb_cfg);

    // Readback verification (matches U5 main.c pattern)
    // Catches UNIT corruption that was seen in Session 14 (34 -> 54)
    LL_DLYB_CfgTypeDef dlyb_cfg_readback = {0};
    PSRAM_DLYB_GetConfig(&dlyb_cfg_readback);
    if ((dlyb_cfg_readback.PhaseSel != dlyb_cfg_expected.PhaseSel) ||
        (dlyb_cfg_readback.Units != dlyb_cfg_expected.Units))
        Error_Handler(ERR_PSRAM_DLYB_Verify, STM32_FAIL);
}

// ---------------------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------------------

// APS6408L register write
uint32_t PSRAM_WriteReg(uint32_t Address, uint8_t *Value)
{
    OSPI_RegularCmdTypeDef sCommand = {0};
    sCommand.OperationType          = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId                = HAL_OSPI_FLASH_ID_1;
    sCommand.InstructionMode        = HAL_OSPI_INSTRUCTION_8_LINES;
    sCommand.InstructionSize        = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode     = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.Instruction            = PSRAM_WRITE_REG_CMD; // 0xC0
    sCommand.AddressMode            = HAL_OSPI_ADDRESS_8_LINES;
    sCommand.AddressSize            = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AddressDtrMode         = HAL_OSPI_ADDRESS_DTR_ENABLE;
    sCommand.Address                = Address;
    sCommand.AlternateBytesMode     = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode               = HAL_OSPI_DATA_8_LINES;
    sCommand.DataDtrMode            = HAL_OSPI_DATA_DTR_ENABLE;
    sCommand.NbData                 = 2;
    sCommand.DummyCycles            = 0;
    sCommand.DQSMode                = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode               = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        Error_Handler(ERR_PSRAM_WriteReg_Cmd, STM32_FAIL);
    if (HAL_OSPI_Transmit(&hospi1, Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        Error_Handler(ERR_PSRAM_WriteReg_Tx, STM32_FAIL);
    return HAL_OK;
}

// APS6408L register read
uint32_t PSRAM_ReadReg(uint32_t Address, uint8_t *Value, uint32_t DummyCycles)
{
    OSPI_RegularCmdTypeDef sCommand = {0};
    sCommand.OperationType          = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId                = HAL_OSPI_FLASH_ID_1;
    sCommand.InstructionMode        = HAL_OSPI_INSTRUCTION_8_LINES;
    sCommand.InstructionSize        = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode     = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.Instruction            = PSRAM_READ_REG_CMD; // 0x40
    sCommand.AddressMode            = HAL_OSPI_ADDRESS_8_LINES;
    sCommand.AddressSize            = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AddressDtrMode         = HAL_OSPI_ADDRESS_DTR_ENABLE;
    sCommand.Address                = Address;
    sCommand.AlternateBytesMode     = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode               = HAL_OSPI_DATA_8_LINES;
    sCommand.DataDtrMode            = HAL_OSPI_DATA_DTR_ENABLE;
    sCommand.NbData                 = 2; // MUST be 2 for APMEMORY DDR mode
    sCommand.DummyCycles            = DummyCycles;
    sCommand.DQSMode                = HAL_OSPI_DQS_ENABLE;
    sCommand.SIOOMode               = HAL_OSPI_SIOO_INST_EVERY_CMD;

    if (HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        Error_Handler(ERR_PSRAM_ReadReg_Cmd, STM32_FAIL);
    if (HAL_OSPI_Receive(&hospi1, Value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        Error_Handler(ERR_PSRAM_ReadReg_Rx, STM32_FAIL);

    return HAL_OK;
}

// Configure MR0 (latency/drive strength) and MR8 (burst type), verify read-back
void PSRAM_ConfigureMemory(void)
{
    UINT ret             = HAL_OK;
    uint32_t DummyCycles = 5;

    uint8_t regW_MR0[2]  = {0x24, 0x24};
    uint8_t regR_MR0[2]  = {0};
    uint8_t regR_MR2[2]  = {0};
    uint8_t regW_MR4[2]  = {0x40, 0x40};
    uint8_t regR_MR4[2]  = {0};
    uint8_t regW_MR8[2]  = {0x0B, 0x0B};
    uint8_t regR_MR8[2]  = {0};

    // for testing only
    // STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR0, PSRAM_ReadReg(PSRAM_MR0, regR_MR0, DummyCycles));
    // STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR2, PSRAM_ReadReg(PSRAM_MR2, regR_MR2, DummyCycles));
    // STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR4, PSRAM_ReadReg(PSRAM_MR4, regR_MR4, DummyCycles));
    // STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR8, PSRAM_ReadReg(PSRAM_MR8, regR_MR8, DummyCycles));
    //
    // __BKPT(1);

    // Write + read-back MR0
    STM32_ERROR_CHECK(ERR_PSRAM_WriteReg_MR0, PSRAM_WriteReg(PSRAM_MR0, regW_MR0));
    STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR0, PSRAM_ReadReg(PSRAM_MR0, regR_MR0, DummyCycles));

    if (regR_MR0[0] != regW_MR0[0])
        Error_Handler(ERR_PSRAM_MR0_Verify, STM32_FAIL);

    // Read MR2
    STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR2, PSRAM_ReadReg(PSRAM_MR2, regR_MR2, DummyCycles));

    // Write + read-back MR4
    // STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR4, PSRAM_WriteReg(PSRAM_MR4, regW_MR4));
    STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR4, PSRAM_ReadReg(PSRAM_MR4, regR_MR4, DummyCycles));

    // if (regR_MR4[0] != regW_MR4[0])
        // Error_Handler((UINT)ERR_PSRAM_MR4_Verify);

    // Write + read-back MR8
    STM32_ERROR_CHECK(ERR_PSRAM_WriteReg_MR8, PSRAM_WriteReg(PSRAM_MR8, regW_MR8));
    STM32_ERROR_CHECK(ERR_PSRAM_ReadReg_MR8, PSRAM_ReadReg(PSRAM_MR8, regR_MR8, DummyCycles));
    if (regR_MR8[0] != regW_MR8[0])
        Error_Handler(ERR_PSRAM_MR8_Verify, STM32_FAIL);
}

// Enable memory-mapped mode (regular-command protocol, per AN5050 Sec II) ---
void PSRAM_EnableMemMapped(void)
{
    OSPI_RegularCmdTypeDef sCommand        = {0};
    OSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

    // putting the command together
    // Address and NbData left at 0 from zero-init (matches U5 reference)
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_16_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
    sCommand.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_8_LINES;
    sCommand.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
    sCommand.DQSMode            = HAL_OSPI_DQS_ENABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    // write configuration
    sCommand.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
    sCommand.Instruction   = PSRAM_WRITE_CMD;
    sCommand.DummyCycles   = PSRAM_DUMMY_CYCLES_WRITE;

    STM32_ERROR_CHECK(ERR_PSRAM_MemMapped_WriteCfg, HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE));

    // read configuration
    sCommand.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
    sCommand.Instruction   = PSRAM_READ_CMD;
    sCommand.DummyCycles   = PSRAM_DUMMY_CYCLES_READ;

    STM32_ERROR_CHECK(ERR_PSRAM_MemMapped_ReadCfg, HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE));

    sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_ENABLE;
    sMemMappedCfg.TimeOutPeriod     = 0x34;

    STM32_ERROR_CHECK(ERR_PSRAM_MemMapped_Enable, HAL_OSPI_MemoryMapped(&hospi1, &sMemMappedCfg));
}

void PSRAM_GlobalReset(void)
{
    OSPI_RegularCmdTypeDef sCommand = {0};

    // APS6408L Global Reset (datasheet Section 7.3)
    // Resets all mode registers to defaults (LC=5, WLC=5, etc.)
    sCommand.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
    sCommand.FlashId            = HAL_OSPI_FLASH_ID_1;
    sCommand.Instruction        = 0xFFU;
    sCommand.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
    sCommand.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    sCommand.AddressMode        = HAL_OSPI_ADDRESS_NONE;
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = HAL_OSPI_DATA_NONE;
    sCommand.DummyCycles        = 0;
    sCommand.DQSMode            = HAL_OSPI_DQS_DISABLE;
    sCommand.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

    STM32_ERROR_CHECK(ERR_PSRAM_GlobalReset_Cmd, HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE));

    // tRST = 2 us recovery (smart_delay(1) = 1ms, plenty of margin)
    smart_delay(1);
}

PSRAM_BusVerify_t PSRAM_BusVerify(void)
{
    PSRAM_BusVerify_t r = {0};
    r.burst_fail_index  = 0xFFFFFFFFU;

    // all tests use the non-cached DMA region so reads always hit the bus
    volatile uint8_t *pb  = (volatile uint8_t *)(PSRAM_DMA_BASE + 0x100);
    volatile uint32_t *pw = (volatile uint32_t *)(PSRAM_DMA_BASE + 0x100);

    // before the write, enable BusFault handler
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;

    volatile uint32_t cfsr_before = SCB->CFSR;
    // __BKPT(0); // check cfsr_before, should be 0

    // test 1: single byte write + read
    pb[0] = 0xA5U;
    __DSB();

    volatile uint32_t cfsr_after = SCB->CFSR;
    // __BKPT(0); // did we get here?

    uint8_t rb        = pb[0];
    r.byte_expected   = 0xA5U;
    r.byte_actual     = rb;
    r.byte_write_read = (rb == 0xA5U) ? 1U : 0U;

    // test 1b: second byte at different address to rule out bus latch
    pb[2] = 0x5AU; // even address (PSRAM requires A[0]=0)
    __DSB();
    uint8_t rb2 = pb[2];
    if (rb2 != 0x5AU)
        r.byte_write_read = 0U;

    // test 2: 32-bit word write + read
    pw[0] = 0xDEADBEEFU;
    __DSB();
    uint32_t rw       = pw[0];
    r.word_expected   = 0xDEADBEEFU;
    r.word_actual     = rw;
    r.word_write_read = (rw == 0xDEADBEEFU) ? 1U : 0U;

    // test 2b: second word with different pattern
    pw[1] = 0x12345678U;
    __DSB();
    uint32_t rw2 = pw[1];
    if (rw2 != 0x12345678U) {
        r.word_write_read = 0U;
        r.word_expected   = 0x12345678U;
        r.word_actual     = rw2;
    }

    // test 3: small burst -- 16 words (64 bytes)
    const uint32_t burst_len      = 16;
    volatile uint32_t *burst_base = (volatile uint32_t *)(PSRAM_DMA_BASE + 0x200);
    r.burst_write_read            = 1U;

    for (uint32_t i = 0; i < burst_len; i++)
        burst_base[i] = 0xCAFE0000U | i;
    __DSB();

    for (uint32_t i = 0; i < burst_len; i++) {
        uint32_t expected = 0xCAFE0000U | i;
        uint32_t actual   = burst_base[i];
        if (actual != expected) {
            r.burst_write_read = 0U;
            if (r.burst_fail_index == 0xFFFFFFFFU) {
                r.burst_fail_index = i;
                r.burst_expected   = expected;
                r.burst_actual     = actual;
            }
        }
    }

    // copy to global for debugger visibility
    psram_bus_result = r;

    // LED feedback: green = all pass, red = byte failed, yellow = partial
    if (r.byte_write_read && r.word_write_read && r.burst_write_read) {
        HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_SET);
    }
    else if (!r.byte_write_read) {
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_SET);
    }

    PSRAM_DiagDump();

    // __BKPT(0); // inspect psram_bus_result in debugger

    return r;
}

// ---------------------------------------------------------------------------------------
// DLYB Phase Calibration — sweep all SEL taps, find passing window center
//
// Must be called AFTER PSRAM_EnableMemMapped().
// For each tap: exit mem-mapped → change DLYB → re-enter mem-mapped → test.
//
// Session 16: rewritten to use proper DLYB wrapper sequence.
// The previous version called DelayBlock_Configure() directly, which cannot
// modify DLYB registers while memory-mapped mode keeps OCTOSPI busy.
// ---------------------------------------------------------------------------------------
void PSRAM_DLYB_CalibratePhase(void)
{
    volatile uint32_t *pw       = (volatile uint32_t *)PSRAM_DMA_BASE;
    const uint32_t test_words   = 256; // 1 KB — matches the block size where corruption appears
    const uint32_t patterns[]   = {0x55AA55AAU, 0xAA5500FFU, 0x12345678U, 0xDEADBEEFU};
    const uint32_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);

    uint32_t current_unit       = dlyb_tuned_unit;
    uint8_t min_found           = 0;
    uint8_t min_sel = 0, max_sel = 0;
    uint8_t tap_results[DLYB_MAX_SELECT] = {0}; // 0=fail, 1=pass — for debugger

    for (uint8_t sel = 0; sel < DLYB_MAX_SELECT; sel++) {
        // --- Step 1: Exit memory-mapped mode ---
        // HAL_OSPI_Abort brings OCTOSPI back to indirect/idle state
        if (HAL_OSPI_Abort(&hospi1) != HAL_OK)
            Error_Handler(ERR_PSRAM_DLYB_NoValidPhase, STM32_FAIL);

        // --- Step 2: Change DLYB using proper wrapper ---
        LL_DLYB_CfgTypeDef dlyb_cfg;
        dlyb_cfg.Units    = current_unit;
        dlyb_cfg.PhaseSel = sel;
        PSRAM_DLYB_SetConfig(&dlyb_cfg);

        // --- Step 3: Re-enter memory-mapped mode ---
        PSRAM_EnableMemMapped();

        // --- Step 4: Write/read test at this tap ---
        uint8_t pass = 1;

        for (uint32_t p = 0; p < num_patterns && pass; p++) {
            // Write pattern
            for (uint32_t i = 0; i < test_words; i++)
                pw[i] = patterns[p];
            __DSB();

            // Read back and verify
            for (uint32_t i = 0; i < test_words; i++) {
                if (pw[i] != patterns[p]) {
                    pass = 0;
                    break;
                }
            }
        }

        // Also test counting pattern (position-dependent corruption)
        if (pass) {
            for (uint32_t i = 0; i < test_words; i++)
                pw[i] = i;
            __DSB();

            for (uint32_t i = 0; i < test_words; i++) {
                if (pw[i] != i) {
                    pass = 0;
                    break;
                }
            }
        }

        tap_results[sel] = pass;

        if (pass) {
            if (!min_found) {
                min_sel   = sel;
                min_found = 1;
            }
            max_sel = sel;
        }
        else if (min_found) {
            break; // end of passing window
        }
    }

    if (!min_found) {
        // Store tap_results for debugger inspection before halting
        dlyb_cal_min    = 0xDEAD;
        dlyb_cal_max    = 0xDEAD;
        dlyb_cal_center = 0xDEAD;
        // __BKPT(0); // inspect tap_results[] in debugger
        Error_Handler(ERR_PSRAM_DLYB_NoValidPhase, STM32_FAIL);
    }

    // --- Apply center of passing window ---
    uint8_t center = (min_sel + max_sel) / 2;

    if (HAL_OSPI_Abort(&hospi1) != HAL_OK)
        Error_Handler(ERR_PSRAM_DLYB_NoValidPhase, STM32_FAIL);

    LL_DLYB_CfgTypeDef dlyb_final;
    dlyb_final.Units    = current_unit;
    dlyb_final.PhaseSel = center;
    PSRAM_DLYB_SetConfig(&dlyb_final);

    PSRAM_EnableMemMapped();

    dlyb_cal_min    = min_sel;
    dlyb_cal_max    = max_sel;
    dlyb_cal_center = center;
    // __BKPT(0); // inspect tap_results[] in debugger
}

PSRAM_TestResult_t PSRAM_WriteReadTest(uint32_t base_addr, uint32_t size_bytes)
{
    volatile uint32_t *ptr;
    uint32_t words = size_bytes / 4U;
    uint32_t i;
    PSRAM_TestResult_t result   = {0};

    const uint32_t patterns[]   = {0x55AA55AAU, 0xAA5500FFU};
    const uint32_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);

    // pass 0: address-as-data
    ptr = (volatile uint32_t *)base_addr;
    for (i = 0; i < words; i++)
        ptr[i] = base_addr + i * 4U;

    SCB_CleanDCache_by_Addr((uint32_t *)base_addr, (int32_t)size_bytes);
    SCB_InvalidateDCache_by_Addr((uint32_t *)base_addr, (int32_t)size_bytes);

    ptr = (volatile uint32_t *)base_addr;
    for (i = 0; i < words; i++) {
        uint32_t expected = base_addr + i * 4U;
        if (ptr[i] != expected) {
            if (result.error_count == 0) {
                result.first_fail_addr     = base_addr + i * 4U;
                result.first_fail_expected = expected;
                result.first_fail_actual   = ptr[i];
                result.pattern_index       = 0;
            }
            result.error_count++;
        }
    }

    // pass 1+: constant patterns
    for (uint32_t p = 0; p < num_patterns; p++) {
        ptr = (volatile uint32_t *)base_addr;
        for (i = 0; i < words; i++)
            ptr[i] = patterns[p];

        SCB_CleanDCache_by_Addr((uint32_t *)base_addr, (int32_t)size_bytes);
        SCB_InvalidateDCache_by_Addr((uint32_t *)base_addr, (int32_t)size_bytes);

        ptr = (volatile uint32_t *)base_addr;
        for (i = 0; i < words; i++) {
            if (ptr[i] != patterns[p]) {
                if (result.error_count == 0) {
                    result.first_fail_addr     = base_addr + i * 4U;
                    result.first_fail_expected = patterns[p];
                    result.first_fail_actual   = ptr[i];
                    result.pattern_index       = p + 1;
                }
                result.error_count++;
            }
        }
    }

    return result;
}
