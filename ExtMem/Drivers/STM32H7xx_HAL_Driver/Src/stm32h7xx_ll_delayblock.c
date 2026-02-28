/**
 ******************************************************************************
 * @file    stm32h7xx_ll_delayblock.c
 * @author  MCD Application Team
 * @brief   DelayBlock Low Layer HAL module driver.
 *
 *          This file provides firmware functions to manage the following
 *          functionalities of the Delay Block peripheral:
 *           + input clock frequency range 25MHz to 208MHz
 *           + up to 12 oversampling phases
 *
 * @note    Modified to add U5-style GetClockPeriod / SetDelay / GetDelay API.
 *          The GetClockPeriod function follows the RM0468 §27.3.4 documented
 *          procedure (identical to RM0456 §32.4.4 for STM32U5).  This returns
 *          PhaseSel = number of taps spanning one clock period, suitable for
 *          the DDR/DTR quarter-period correction (PhaseSel /= 4).
 *
 *          The original DelayBlock_Enable() is retained for backward compat
 *          but should NOT be used for OCTOSPI DTR calibration -- its non-standard
 *          algorithm produces a SEL with different semantics.
 *
 *          Compilation guard fixed to include HAL_OSPI_MODULE_ENABLED.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_ll_delayblock.h"
#include "stm32h7xx_hal.h"

/** @addtogroup STM32H7xx_HAL_Driver
 * @{
 */

/** @defgroup DELAYBLOCK_LL DELAYBLOCK_LL
 * @brief Low layer module for Delay Block
 * @{
 */

/* FIX: Original guard only had SD and QSPI, missing OSPI entirely.
 * This caused the functions to compile to nothing when only OSPI was enabled. */
#if defined(HAL_SD_MODULE_ENABLED) || defined(HAL_QSPI_MODULE_ENABLED) || defined(HAL_OSPI_MODULE_ENABLED)

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup DelayBlock_LL_Private_Defines Delay Block Low Layer Private Defines
 * @{
 */
#define DLYB_TIMEOUT 0xFFU

/* LNG field masks -- may not be defined in all H7 CMSIS versions.
 * Define locally using known bit positions from DLYB_CFGR register layout:
 *   LNG[11:0] = bits [27:16], LNGF = bit 31                              */
#ifndef DLYB_LNG_10_0_MASK
#define DLYB_LNG_10_0_MASK 0x07FF0000U /* LNG bits [10:0] = CFGR[26:16] */
#endif

#ifndef DLYB_LNG_11_10_MASK
#define DLYB_LNG_11_10_MASK 0x0C000000U /* LNG bits [11:10] = CFGR[27:26] */
#endif

/* Combined field masks for readback -- safe to redefine with #ifndef */
#ifndef DLYB_CFGR_LNG_Pos
#define DLYB_CFGR_LNG_Pos 16U
#endif

#ifndef DLYB_CFGR_LNG_Msk
#define DLYB_CFGR_LNG_Msk (0xFFFUL << DLYB_CFGR_LNG_Pos) /* bits [27:16] */
#endif

/* Alias used by U5 code: DLYB_CFGR_LNG = full 12-bit LNG mask */
#ifndef DLYB_CFGR_LNG
#define DLYB_CFGR_LNG DLYB_CFGR_LNG_Msk
#endif

/* Individual LNG bit aliases if not defined */
#ifndef DLYB_CFGR_LNG_11
#define DLYB_CFGR_LNG_11 (0x800UL << DLYB_CFGR_LNG_Pos) /* bit 27 */
#endif

#ifndef DLYB_CFGR_LNG_10
#define DLYB_CFGR_LNG_10 (0x400UL << DLYB_CFGR_LNG_Pos) /* bit 26 */
#endif

/* SEL field mask for readback */
#ifndef DLYB_CFGR_SEL_Msk
#define DLYB_CFGR_SEL_Msk 0x0000000FU /* bits [3:0] */
#endif

#ifndef DLYB_CFGR_SEL
#define DLYB_CFGR_SEL DLYB_CFGR_SEL_Msk
#endif

/* UNIT field mask for readback */
#ifndef DLYB_CFGR_UNIT_Msk
#define DLYB_CFGR_UNIT_Msk (0x7FUL << DLYB_CFGR_UNIT_Pos) /* bits [14:8] */
#endif

#ifndef DLYB_CFGR_UNIT
#define DLYB_CFGR_UNIT DLYB_CFGR_UNIT_Msk
#endif

/**
 * @}
 */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup DelayBlock_LL_Exported_Functions Delay Block Low Layer Exported Functions
 * @{
 */

/* ========================================================================== */
/*  NEW API -- Ported from STM32U5 LL_DLYB (RM-compliant algorithm)          */
/* ========================================================================== */

/** @defgroup DLYB_New_API New Delay Block API
 * @brief  Functions ported from STM32U5xx LL DLYB driver.
 *         These follow the documented RM0468 §27.3.4 procedure exactly.
 * @{
 */

/**
 * @brief  Measure the clock period: sweep UNIT to find delay line length,
 *         then count how many taps (PhaseSel) span one input clock period.
 *
 * @note   Follows RM0468 §27.3.4 procedure:
 *         1. DEN=1, SEN=1, SEL=12 (enable all taps)
 *         2. Sweep UNIT 0..127, read LNG after each
 *         3. Break when LNG[10:0]>0 AND (LNG[11] or LNG[10])=0
 *         4. PhaseSel = highest set bit position in LNG[11:0]
 *
 *         The caller must:
 *         - Set DEN=1 before calling (SET_BIT(DLYBx->CR, DLYB_CR_DEN))
 *         - Have a free-running clock active (OCTOSPI DCR1.FRCK=1)
 *
 *         After this returns successfully, caller should:
 *         - Disable the delay block (CLEAR_BIT(DLYBx->CR, DLYB_CR_DEN))
 *         - Apply DTR correction: pdlyb_cfg->PhaseSel /= 4
 *         - Clamp: if (PhaseSel == 0) PhaseSel = 1
 *         - Re-enable and apply via DelayBlock_SetDelay()
 *
 * @param  DLYBx      Pointer to DLYB instance (e.g. DLYB_OCTOSPI1)
 * @param  pdlyb_cfg  [out] Measured clock period parameters:
 *                     - Units: the UNIT value where one period was detected
 *                     - PhaseSel: number of taps spanning one period (0..10)
 *
 * @retval HAL_OK      Valid period detected
 * @retval HAL_TIMEOUT LNGF never asserted for some UNIT value
 * @retval HAL_ERROR   No valid period found (clock too slow or not running)
 */
HAL_StatusTypeDef DelayBlock_GetClockPeriod(DLYB_TypeDef *DLYBx,
                                            LL_DLYB_CfgTypeDef *pdlyb_cfg)
{
    uint32_t i = 0U;
    uint32_t nb;
    uint32_t lng;
    uint32_t tickstart;

    /* Enable the length sampling */
    SET_BIT(DLYBx->CR, DLYB_CR_SEN);

    /* Delay line length detection: sweep UNIT with all 12 taps enabled */
    while (i < DLYB_MAX_UNIT) {
        /* Set UNIT value, keep SEL=12 (all taps active) */
        DLYBx->CFGR = DLYB_MAX_SELECT | (i << DLYB_CFGR_UNIT_Pos);

        /* Wait for length flag */
        tickstart = HAL_GetTick();
        while ((DLYBx->CFGR & DLYB_CFGR_LNGF) == 0U) {
            if ((HAL_GetTick() - tickstart) >= DLYB_TIMEOUT) {
                /* Double-check to avoid false timeout due to preemption */
                if ((DLYBx->CFGR & DLYB_CFGR_LNGF) == 0U) {
                    /* Disable length sampling before returning */
                    CLEAR_BIT(DLYBx->CR, DLYB_CR_SEN);
                    return HAL_TIMEOUT;
                }
            }
        }

        /* Check if delay line spans one input clock period:
         * - LNG[10:0] must have at least one bit set (some taps see transitions)
         * - LNG[11] and LNG[10] must NOT both be set (would mean > 1 period)    */
        if ((DLYBx->CFGR & DLYB_LNG_10_0_MASK) != 0U) {
            if ((DLYBx->CFGR & (DLYB_CFGR_LNG_11 | DLYB_CFGR_LNG_10)) != DLYB_LNG_11_10_MASK) {
                /* Delay line length spans one input clock period -- done */
                break;
            }
        }

        i++;
    }

    if (i != DLYB_MAX_UNIT) {
        /* Found valid period at UNIT = i.
         * Count how many taps span one period: find highest set bit in LNG. */
        lng = (DLYBx->CFGR & DLYB_CFGR_LNG) >> 16U;
        nb  = 10U;
        while ((nb > 0U) && ((lng >> nb) == 0U)) {
            nb--;
        }

        if (nb != 0U) {
            pdlyb_cfg->PhaseSel = nb;
            pdlyb_cfg->Units    = i;

            /* Disable length sampling */
            CLEAR_BIT(DLYBx->CR, DLYB_CR_SEN);

            return HAL_OK;
        }
    }

    /* No valid period detected */
    CLEAR_BIT(DLYBx->CR, DLYB_CR_SEN);
    return HAL_ERROR;
}

/**
 * @brief  Program the delay block with specific UNIT and SEL values.
 *
 * @note   Follows RM0468 §27.3.5 output clock phase configuration:
 *         1. SEN=1 (disable output, allow register writes)
 *         2. Write CFGR with PhaseSel and Units
 *         3. SEN=0 (enable output on selected phase)
 *
 *         DEN must already be set in DLYBx->CR.
 *
 * @param  DLYBx      Pointer to DLYB instance
 * @param  pdlyb_cfg  Configuration: Units (UNIT value) and PhaseSel (SEL value)
 */
void DelayBlock_SetDelay(DLYB_TypeDef *DLYBx,
                         const LL_DLYB_CfgTypeDef *pdlyb_cfg)
{
    /* Enable length sampling to allow register writes */
    SET_BIT(DLYBx->CR, DLYB_CR_SEN);

    /* Write UNIT and SEL */
    DLYBx->CFGR = (pdlyb_cfg->PhaseSel) |
                  ((pdlyb_cfg->Units) << DLYB_CFGR_UNIT_Pos);

    /* Disable length sampling -- enables output clock on selected phase */
    CLEAR_BIT(DLYBx->CR, DLYB_CR_SEN);
}

/**
 * @brief  Read back the current UNIT and SEL values from the delay block.
 *
 * @param  DLYBx      Pointer to DLYB instance
 * @param  pdlyb_cfg  [out] Current Units and PhaseSel
 */
void DelayBlock_GetDelay(const DLYB_TypeDef *DLYBx,
                         LL_DLYB_CfgTypeDef *pdlyb_cfg)
{
    pdlyb_cfg->Units    = ((DLYBx->CFGR & DLYB_CFGR_UNIT) >> DLYB_CFGR_UNIT_Pos);
    pdlyb_cfg->PhaseSel = (DLYBx->CFGR & DLYB_CFGR_SEL);
}

/**
 * @}
 */

/* ========================================================================== */
/*  LEGACY API -- Original H7 functions, retained for backward compatibility  */
/* ========================================================================== */

/** @defgroup HAL_DELAY_LL_Group1 Legacy Initialization Functions
 * @brief  Original H7 delay block functions.
 * @note   DelayBlock_Enable() uses a non-standard algorithm that does NOT
 *         follow the RM0468 §27.3.4 procedure. Its returned SEL value has
 *         different semantics than the RM-documented tap count.
 *         For OCTOSPI DTR use, prefer GetClockPeriod + SetDelay instead.
 * @{
 */

/**
 * @brief  Enable the Delay Block instance (legacy algorithm).
 * @note   DEPRECATED for OCTOSPI DTR calibration. Uses double nested loop
 *         (outer=SEL, inner=UNIT) checking individual LNG bits for transitions.
 *         The resulting SEL value is NOT "taps per period" and CANNOT be
 *         divided by 4 for DTR correction.
 * @param  DLYBx: Pointer to DLYB instance.
 * @retval HAL status
 */
HAL_StatusTypeDef DelayBlock_Enable(DLYB_TypeDef *DLYBx)
{
    uint32_t unit = 0U;
    uint32_t sel  = 0U;
    uint32_t sel_current;
    uint32_t unit_current;
    uint32_t tuning;
    uint32_t lng_mask;
    uint32_t tickstart;

    DLYBx->CR = DLYB_CR_DEN | DLYB_CR_SEN;

    for (sel_current = 0U; sel_current < DLYB_MAX_SELECT; sel_current++) {
        /* lng_mask is the mask bit for the LNG field to check the output of the UNITx */
        lng_mask = DLYB_CFGR_LNG_0 << sel_current;
        tuning   = 0U;
        for (unit_current = 0U; unit_current < DLYB_MAX_UNIT; unit_current++) {
            /* Set the Delay of the UNIT(s) */
            DLYBx->CFGR = DLYB_MAX_SELECT | (unit_current << DLYB_CFGR_UNIT_Pos);

            /* Waiting for a LNG valid value */
            tickstart = HAL_GetTick();
            while ((DLYBx->CFGR & DLYB_CFGR_LNGF) == 0U) {
                if ((HAL_GetTick() - tickstart) >= DLYB_TIMEOUT) {
                    return HAL_TIMEOUT;
                }
            }
            if (tuning == 0U) {
                if ((DLYBx->CFGR & lng_mask) != 0U) {
                    /* 1/2 period HIGH is detected */
                    tuning = 1U;
                }
            }
            else {
                /* 1/2 period LOW detected after HIGH => FULL PERIOD passed */
                if ((DLYBx->CFGR & lng_mask) == 0U) {
                    /* Save the first result */
                    if (unit == 0U) {
                        unit = unit_current;
                        sel  = sel_current + 1U;
                    }
                    break;
                }
            }
        }
    }

    /* Apply the Tuning settings */
    DLYBx->CR   = 0U;
    DLYBx->CR   = DLYB_CR_DEN | DLYB_CR_SEN;
    DLYBx->CFGR = sel | (unit << DLYB_CFGR_UNIT_Pos);
    DLYBx->CR   = DLYB_CR_DEN;

    return HAL_OK;
}

/**
 * @brief  Disable the Delay Block instance.
 * @param  DLYBx: Pointer to DLYB instance.
 * @retval HAL status
 */
HAL_StatusTypeDef DelayBlock_Disable(DLYB_TypeDef *DLYBx)
{
    /* Disable DLYB */
    DLYBx->CR = 0U;
    return HAL_OK;
}

/**
 * @brief  Configure the Delay Block instance.
 * @param  DLYBx: Pointer to DLYB instance.
 * @param  PhaseSel: Phase selection [0..11].
 * @param  Units: Delay units [0..127].
 * @retval HAL status
 */
HAL_StatusTypeDef DelayBlock_Configure(DLYB_TypeDef *DLYBx,
                                       uint32_t PhaseSel,
                                       uint32_t Units)
{
    /* Apply the delay settings */
    // DLYBx->CR   = 0U;
    DLYBx->CR   = DLYB_CR_DEN | DLYB_CR_SEN;
    DLYBx->CFGR = PhaseSel | (Units << DLYB_CFGR_UNIT_Pos);
    DLYBx->CR   = DLYB_CR_DEN;

    return HAL_OK;
}

/**
 * @}
 */

/**
 * @}
 */

#endif /* HAL_SD_MODULE_ENABLED || HAL_QSPI_MODULE_ENABLED || HAL_OSPI_MODULE_ENABLED */

/**
 * @}
 */

/**
 * @}
 */