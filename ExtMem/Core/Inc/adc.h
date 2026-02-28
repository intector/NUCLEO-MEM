/**
  ****************************************************************************************
 * @file           : adc.h
 * @brief          : analog-to-digital converter (ADC) header file
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

#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Exported variables
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

// ADC1 Channel Assignments (3 channels - H-Bridge Measurements)
// Rank 1: ADC_CHANNEL_18 (PA4/PA5) - INP18/INN18-VS (H-Bridge Differential Voltage)
// Rank 2: ADC_CHANNEL_9  (PB0)     - ANA-IN-CS (H-Bridge Current)

// ADC2 Channel Assignments (4 channels - Boost Converter Measurements)
// Rank 1: ADC_CHANNEL_15 (PA3) - ANA-IN-CS1 (Booster Input Current)
// Rank 2: ADC_CHANNEL_10 (PC0) - ANA-IN-CS2 (Booster Switch Current)
// Rank 3: ADC_CHANNEL_3  (PA6) - SWN-SENSE (Booster Switch Voltage)
// Rank 4: ADC_CHANNEL_5  (PB1) - VOUT-SENSE (Booster Output Voltage)

// ADC3 Channel Assignments (7 channels - all temperature)
// Rank 1: ADC_CHANNEL_0  (PC2)  - NTC-A-Q1 (H-Bridge Q1 Temperature)
// Rank 2: ADC_CHANNEL_1  (PC3)  - NTC-A-Q2 (H-Bridge Q2 Temperature)
// Rank 3: ADC_CHANNEL_5  (PF3)  - NTC-A-Q3 (H-Bridge Q3 Temperature)
// Rank 4: ADC_CHANNEL_9  (PF4)  - NTC-A-Q4 (H-Bridge Q4 Temperature)
// Rank 5: ADC_CHANNEL_4  (PF5)  - NTC-L1 (Booster Inductor Temperature)
// Rank 6: ADC_CHANNEL_8  (PF6)  - NTC-B-Q2 (Booster Q2 Temperature)
// Rank 7: ADC_CHANNEL_6  (PF10) - NTC-B-Q1 (Booster Q1 Temperature)

// Exported functions
void MX_ADC1_Init(void);
void MX_ADC2_Init(void);
void MX_ADC3_Init(void);

#ifdef __cplusplus
}
#endif

#endif // __ADC_H__
