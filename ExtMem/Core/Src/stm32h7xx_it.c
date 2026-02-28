/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

// Includes ------------------------------------------------------------------
#include "main.h"
#include "stm32h7xx_it.h"
#include "dma.h"
#include "adc.h"

// External variables --------------------------------------------------------
extern ETH_HandleTypeDef heth;
//extern SD_HandleTypeDef hsd1;
extern MMC_HandleTypeDef hmmc1;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim6;

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/

// This function handles Non maskable interrupt.
void NMI_Handler(void)
{
   while (1)
  {
  }
}

// This function handles Hard fault interrupt.
void HardFault_Handler(void)
{
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
    Show_OLED_Message("!!! SYSTEM ERROR !!!", "HardFault Handler", 100);
    while (1) {
    }
}

// This function handles Memory management fault.
void MemManage_Handler(void)
{
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
    Show_OLED_Message("!!! SYSTEM ERROR !!!", "MemManage Handler", 100);
    while (1) {
    }
}

// This function handles Pre-fetch fault, memory access fault.
void BusFault_Handler(void)
{
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
    Show_OLED_Message("!!! SYSTEM ERROR !!!", "BusFault Handler", 100);
    while (1) {
    }
}

// This function handles Undefined instruction or illegal state.
void UsageFault_Handler(void)
{
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
    Show_OLED_Message("!!! SYSTEM ERROR !!!", "UsageFault Handler", 100);
    while (1) {
    }
}

// This function handles Debug monitor.
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

// This function handles EXTI line[15:10] interrupts.
void EXTI15_10_IRQHandler(void)
{
  // HAL_GPIO_EXTI_IRQHandler(USER_Btn_Pin);
  // HAL_GPIO_EXTI_IRQHandler(NFC_IRQ_OUT_Pin);
}

// This function handles SDMMC1 global interrupt.
void SDMMC1_IRQHandler(void)
{
    HAL_MMC_IRQHandler(&hmmc1);
}

// This function handles TIM6 global interrupt, DAC1_CH1 and DAC1_CH2 underrun error interrupts.
void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim6);
}

// This function handles Ethernet global interrupt.
void ETH_IRQHandler(void)
{
  HAL_ETH_IRQHandler(&heth);
}

// DMA1 Stream2 interrupt handler - ADC1
void DMA1_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc1);
}

// DMA1 Stream3 interrupt handler - ADC2
void DMA1_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc2);
}

// DMA2 Stream0 interrupt handler - ADC3
void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc3);
}

