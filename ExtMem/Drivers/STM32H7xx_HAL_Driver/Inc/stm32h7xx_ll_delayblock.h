/**
  ******************************************************************************
  * @file    stm32h7xx_ll_delayblock.h
  * @author  MCD Application Team
  * @brief   Header file of Delay Block module.
  *
  * @note    Modified to add U5-style GetClockPeriod / SetDelay / GetDelay API
  *          for correct DTR quarter-period calibration on OCTOSPI.
  *          Original legacy DelayBlock_Enable() retained for backward compat.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H7xx_LL_DLYB_H
#define STM32H7xx_LL_DLYB_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal_def.h"

/** @addtogroup STM32H7xx_HAL_Driver
  * @{
  */

/** @addtogroup DELAYBLOCK_LL
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  DLYB Configuration Structure definition
  * @note   Ported from STM32U5 LL_DLYB_CfgTypeDef for use with
  *         DelayBlock_GetClockPeriod / DelayBlock_SetDelay / DelayBlock_GetDelay.
  */
typedef struct
{
  uint32_t Units;       /*!< Delay of a unit delay cell.
                             Value between 0 and DLYB_MAX_UNIT-1 (0..127)     */

  uint32_t PhaseSel;    /*!< Number of delay taps spanning one clock period.
                             Value between 0 and DLYB_MAX_SELECT (0..12)
                             For DTR/DDR mode, apply /4 before programming SEL */
} LL_DLYB_CfgTypeDef;

/* Exported constants --------------------------------------------------------*/

#define DLYB_MAX_UNIT   ((uint32_t)0x00000080U) /*!< Max UNIT value (128)  */
#define DLYB_MAX_SELECT ((uint32_t)0x0000000CU) /*!< Max SELECT value (12) */

/* Exported functions --------------------------------------------------------*/

/** @addtogroup DelayBlock_LL_Exported_Functions
  * @{
  */

/* --- New API (ported from U5, follows RM procedure) ---------------------- */

/**
  * @brief  Measure the clock period and return UNIT + PhaseSel (tap count).
  * @note   Follows the RM0468 §27.3.4 delay line length configuration procedure.
  *         DEN must be set and a free-running clock (FRCK) must be active before
  *         calling this function.
  * @param  DLYBx   Pointer to DLYB instance (e.g. DLYB_OCTOSPI1)
  * @param  pdlyb_cfg  [out] Receives measured Units and PhaseSel (full period)
  * @retval HAL_OK on success, HAL_TIMEOUT or HAL_ERROR on failure
  */
HAL_StatusTypeDef DelayBlock_GetClockPeriod(DLYB_TypeDef *DLYBx,
                                            LL_DLYB_CfgTypeDef *pdlyb_cfg);

/**
  * @brief  Program the delay block with a specific UNIT and SEL value.
  * @note   Handles the SEN enable/disable sequence per RM0468 §27.3.5.
  *         DEN must already be set.
  * @param  DLYBx      Pointer to DLYB instance
  * @param  pdlyb_cfg  Configuration to apply (Units + PhaseSel)
  */
void DelayBlock_SetDelay(DLYB_TypeDef *DLYBx,
                         const LL_DLYB_CfgTypeDef *pdlyb_cfg);

/**
  * @brief  Read back the current UNIT and SEL values from the delay block.
  * @param  DLYBx      Pointer to DLYB instance
  * @param  pdlyb_cfg  [out] Receives current Units and PhaseSel
  */
void DelayBlock_GetDelay(const DLYB_TypeDef *DLYBx,
                         LL_DLYB_CfgTypeDef *pdlyb_cfg);

/* --- Legacy API (retained for backward compatibility) -------------------- */

/**
  * @brief  Enable the Delay Block instance (legacy H7 algorithm).
  * @note   DEPRECATED for OCTOSPI DTR use. This function uses a non-standard
  *         sweep algorithm that does NOT follow the RM procedure and returns
  *         a SEL value with different semantics than the RM-documented tap count.
  *         For DTR/DDR mode, use DelayBlock_GetClockPeriod() + /4 + SetDelay().
  * @param  DLYBx   Pointer to DLYB instance
  * @retval HAL_OK or HAL_TIMEOUT
  */
HAL_StatusTypeDef DelayBlock_Enable(DLYB_TypeDef *DLYBx);

/**
  * @brief  Disable the Delay Block instance.
  * @param  DLYBx   Pointer to DLYB instance
  * @retval HAL_OK
  */
HAL_StatusTypeDef DelayBlock_Disable(DLYB_TypeDef *DLYBx);

/**
  * @brief  Configure the Delay Block with specific PhaseSel and Units.
  * @note   Low-level: directly writes CR and CFGR without measuring.
  *         Prefer DelayBlock_SetDelay() for new code.
  * @param  DLYBx     Pointer to DLYB instance
  * @param  PhaseSel  Phase selection [0..11]
  * @param  Units     Delay units [0..127]
  * @retval HAL_OK
  */
HAL_StatusTypeDef DelayBlock_Configure(DLYB_TypeDef *DLYBx,
                                       uint32_t PhaseSel, uint32_t Units);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32H7xx_LL_DLYB_H */