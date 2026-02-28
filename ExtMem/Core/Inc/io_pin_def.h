/**
  ****************************************************************************************
 * @file           : io_pin_def.h
 * @brief          : IO-Pin definitions for the Prototype Board
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

#ifndef __IO_PIN_DEF_H
#define __IO_PIN_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
// debugging interface
// ---------------------------------------------------------------------------------------
#define SWO_Pin         GPIO_PIN_3
#define SWO_GPIO_Port   GPIOB
#define MCO_Pin         GPIO_PIN_0
#define MCO_GPIO_Port   GPIOH
#define SWDIO_Pin       GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin       GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA

// ---------------------------------------------------------------------------------------
// ETHERNET signals
// ---------------------------------------------------------------------------------------
#define RMII_TX_EN_Pin         GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port   GPIOG
#define RMII_TXD0_Pin          GPIO_PIN_13
#define RMII_TXD0_GPIO_Port    GPIOG
#define RMII_TXD1_Pin          GPIO_PIN_13
#define RMII_TXD1_GPIO_Port    GPIOB

#define RMII_RXD0_Pin          GPIO_PIN_4
#define RMII_RXD0_GPIO_Port    GPIOC
#define RMII_RXD1_Pin          GPIO_PIN_5
#define RMII_RXD1_GPIO_Port    GPIOC

#define RMII_CRS_DV_Pin        GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port  GPIOA
#define RMII_MDC_Pin           GPIO_PIN_1
#define RMII_MDC_GPIO_Port     GPIOC
#define RMII_MDIO_Pin          GPIO_PIN_2
#define RMII_MDIO_GPIO_Port    GPIOA
#define RMII_REF_CLK_Pin       GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA

// ---------------------------------------------------------------------------------------
// user button signals
// ---------------------------------------------------------------------------------------
#define USER_Btn_Pin       GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC

// ---------------------------------------------------------------------------------------
// main board LED signals
// ---------------------------------------------------------------------------------------
#define LED_Green_Pin        GPIO_PIN_0
#define LED_Green_GPIO_Port  GPIOB
#define LED_Yellow_Pin       GPIO_PIN_1
#define LED_Yellow_GPIO_Port GPIOE
#define LED_Red_Pin          GPIO_PIN_14
#define LED_Red_GPIO_Port    GPIOB

// ---------------------------------------------------------------------------------------
// SPI signals
// ---------------------------------------------------------------------------------------
#define SPI1_SCK_Pin        GPIO_PIN_5
#define SPI1_SCK_GPIO_Port  GPIOA
#define SPI1_MISO_Pin       GPIO_PIN_6
#define SPI1_MISO_GPIO_Port GPIOA
#define SPI1_MOSI_Pin       GPIO_PIN_5
#define SPI1_MOSI_GPIO_Port GPIOB

// ---------------------------------------------------------------------------------------
// signals for OLED display
// ---------------------------------------------------------------------------------------
#define OLED_SCL_Pin       GPIO_PIN_1
#define OLED_SCL_GPIO_Port GPIOF
#define OLED_SDA_Pin       GPIO_PIN_0
#define OLED_SDA_GPIO_Port GPIOF

// ---------------------------------------------------------------------------------------
// eMMC IO signals
// ---------------------------------------------------------------------------------------
#define EMMC_D0_Pin        GPIO_PIN_8
#define EMMC_D0_GPIO_Port  GPIOC
#define EMMC_D1_Pin        GPIO_PIN_9
#define EMMC_D1_GPIO_Port  GPIOC
#define EMMC_D2_Pin        GPIO_PIN_10
#define EMMC_D2_GPIO_Port  GPIOC
#define EMMC_D3_Pin        GPIO_PIN_11
#define EMMC_D3_GPIO_Port  GPIOC
#define EMMC_D4_Pin        GPIO_PIN_8
#define EMMC_D4_GPIO_Port  GPIOB
#define EMMC_D5_Pin        GPIO_PIN_9
#define EMMC_D5_GPIO_Port  GPIOB
#define EMMC_D6_Pin        GPIO_PIN_6
#define EMMC_D6_GPIO_Port  GPIOC
#define EMMC_D7_Pin        GPIO_PIN_7
#define EMMC_D7_GPIO_Port  GPIOC
#define EMMC_CLK_Pin       GPIO_PIN_12
#define EMMC_CLK_GPIO_Port GPIOC
#define EMMC_CMD_Pin       GPIO_PIN_2
#define EMMC_CMD_GPIO_Port GPIOD

// ---------------------------------------------------------------------------------------
// PSRAM IO signals (directly on NUCLEO-MEM via OCTOSPI1 Port 1)
// ---------------------------------------------------------------------------------------
#define PSRAM_D0_Pin        GPIO_PIN_8
#define PSRAM_D0_GPIO_Port  GPIOF
#define PSRAM_D1_Pin        GPIO_PIN_9
#define PSRAM_D1_GPIO_Port  GPIOF
#define PSRAM_D2_Pin        GPIO_PIN_2
#define PSRAM_D2_GPIO_Port  GPIOE
#define PSRAM_D3_Pin        GPIO_PIN_6
#define PSRAM_D3_GPIO_Port  GPIOF
#define PSRAM_D4_Pin        GPIO_PIN_7
#define PSRAM_D4_GPIO_Port  GPIOE
#define PSRAM_D5_Pin        GPIO_PIN_2
#define PSRAM_D5_GPIO_Port  GPIOC
#define PSRAM_D6_Pin        GPIO_PIN_3
#define PSRAM_D6_GPIO_Port  GPIOC
#define PSRAM_D7_Pin        GPIO_PIN_10
#define PSRAM_D7_GPIO_Port  GPIOE
#define PSRAM_CLK_Pin       GPIO_PIN_3
#define PSRAM_CLK_GPIO_Port GPIOA
#define PSRAM_DQS_Pin       GPIO_PIN_2
#define PSRAM_DQS_GPIO_Port GPIOB
#define PSRAM_NCS_Pin       GPIO_PIN_11
#define PSRAM_NCS_GPIO_Port GPIOE

// ---------------------------------------------------------------------------------------
// Oscillator IOs
// ---------------------------------------------------------------------------------------
#define HSE1_Pin       GPIO_PIN_0
#define HSE1_GPIO_Port GPIOH
#define HSE2_Pin       GPIO_PIN_1
#define HSE2_GPIO_Port GPIOH
#define LSE1_Pin       GPIO_PIN_14
#define LSE1_GPIO_Port GPIOC
#define LSE2_Pin       GPIO_PIN_15
#define LSE2_GPIO_Port GPIOC

// ---------------------------------------------------------------------------------------
// 24-40V Booster PWM Output IOs
// ---------------------------------------------------------------------------------------
#define TIM15PWM1N_Pin         GPIO_PIN_4
#define TIM15PWM1N_GPIO_Port   GPIOE
#define TIM15PWM1_Pin          GPIO_PIN_5
#define TIM15PWM1_GPIO_Port    GPIOE

// ---------------------------------------------------------------------------------------
// H-Bridge PWM Output IOs
// ---------------------------------------------------------------------------------------
#define TIM1_PWM1N_Pin         GPIO_PIN_8
#define TIM1_PWM1N_GPIO_Port   GPIOE
#define TIM1_PWM1_Pin          GPIO_PIN_9
#define TIM1_PWM1_GPIO_Port    GPIOE

// ---------------------------------------------------------------------------------------
// 24-40V Booster Enable Output IOs
// ---------------------------------------------------------------------------------------
#define DIS1_Pin               GPIO_PIN_13
#define DIS1_GPIO_Port         GPIOD

// ---------------------------------------------------------------------------------------
// H-Bridge Ready Input IOs
// ---------------------------------------------------------------------------------------
// #define U1_RDY_Pin             GPIO_PIN_2    // CONFLICT: PE2 = PSRAM_D2
// #define U1_RDY_GPIO_Port       GPIOE         // CONFLICT: PE2 = PSRAM_D2
#define U2_RDY_Pin             GPIO_PIN_3
#define U2_RDY_GPIO_Port       GPIOD
#define U3_RDY_Pin             GPIO_PIN_4
#define U3_RDY_GPIO_Port       GPIOD
#define U4_RDY_Pin             GPIO_PIN_5
#define U4_RDY_GPIO_Port       GPIOD

// ---------------------------------------------------------------------------------------
// H-Bridge Fault Input IOs
// ---------------------------------------------------------------------------------------
#define U1_FLTN_Pin            GPIO_PIN_6
#define U1_FLTN_GPIO_Port      GPIOD
#define U2_FLTN_Pin            GPIO_PIN_7
#define U2_FLTN_GPIO_Port      GPIOD
#define U3_FLTN_Pin            GPIO_PIN_3
#define U3_FLTN_GPIO_Port      GPIOG
#define U4_FLTN_Pin            GPIO_PIN_2
#define U4_FLTN_GPIO_Port      GPIOG

// ---------------------------------------------------------------------------------------
// H-Bridge Temperature Analog Input IOs
// ---------------------------------------------------------------------------------------
// #define NTC_A_Q1_Pin           GPIO_PIN_2    // CONFLICT: PC2 = PSRAM_D5
// #define NTC_A_Q1_GPIO_Port     GPIOC         // CONFLICT: PC2 = PSRAM_D5
// #define NTC_A_Q2_Pin           GPIO_PIN_3    // CONFLICT: PC3 = PSRAM_D6
// #define NTC_A_Q2_GPIO_Port     GPIOC         // CONFLICT: PC3 = PSRAM_D6
#define NTC_A_Q3_Pin           GPIO_PIN_3
#define NTC_A_Q3_GPIO_Port     GPIOF
#define NTC_A_Q4_Pin           GPIO_PIN_4
#define NTC_A_Q4_GPIO_Port     GPIOF

// ---------------------------------------------------------------------------------------
// 24-40V Booster Temperature Analog Input IOs
// ---------------------------------------------------------------------------------------
#define NTC_L1_Pin             GPIO_PIN_5
#define NTC_L1_GPIO_Port       GPIOF
#define NTC_B_Q1_Pin           GPIO_PIN_10
#define NTC_B_Q1_GPIO_Port     GPIOF
// #define NTC_B_Q2_Pin           GPIO_PIN_6    // CONFLICT: PF6 = PSRAM_D3
// #define NTC_B_Q2_GPIO_Port     GPIOF         // CONFLICT: PF6 = PSRAM_D3

// ---------------------------------------------------------------------------------------
// H-Bridge Current Measurement Analog Input IOs
// ---------------------------------------------------------------------------------------
#define ANA_IN_CS_Pin          GPIO_PIN_0
#define ANA_IN_CS_GPIO_Port    GPIOB

// ---------------------------------------------------------------------------------------
// H-Bridge Voltage Measurement Analog Input IOs
// ---------------------------------------------------------------------------------------
#define INP18_VS_Pin           GPIO_PIN_4
#define INP18_VS_GPIO_Port     GPIOA
#define INN18_VS_Pin           GPIO_PIN_5
#define INN18_VS_GPIO_Port     GPIOA

// ---------------------------------------------------------------------------------------
// 24-40V Booster Current Measurement Analog Input IOs
// ---------------------------------------------------------------------------------------
// #define ANA_IN_CS1_Pin         GPIO_PIN_3    // CONFLICT: PA3 = PSRAM_CLK
// #define ANA_IN_CS1_GPIO_Port   GPIOA         // CONFLICT: PA3 = PSRAM_CLK

#define ANA_IN_CS2_Pin         GPIO_PIN_0
#define ANA_IN_CS2_GPIO_Port   GPIOC

// ---------------------------------------------------------------------------------------
// 24-40V Booster Voltage Measurement Analog Input IOs
// ---------------------------------------------------------------------------------------
#define SWN_SENSE_Pin          GPIO_PIN_6
#define SWN_SENSE_GPIO_Port    GPIOA

#define VOUT_SENSE_Pin         GPIO_PIN_1
#define VOUT_SENSE_GPIO_Port   GPIOB

#ifdef __cplusplus
}
#endif

#endif // __IO_PIN_DEF_H
