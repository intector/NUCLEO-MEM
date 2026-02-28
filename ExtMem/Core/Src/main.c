/**
  ****************************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
// exsample section comment --------------------------------------------------------------
// Includes ------------------------------------------------------------------------------
#include "app_threadx.h"
#include "main.h"
#include "uart_debug.h"

#include "dma.h"
#include "dts.h"
#include "eth.h"
#include "psram.h"
#include "gpio.h"
#include "tim.h"
#include "adc.h"
#include "adc_buffers.h"

// #include "sdmmc.h"

#include "rng.h"
#include "i2c.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "fw_settings.h"
#include "psram_1k_diag.h"

// global variables ----------------------------------------------------------------------
FWS_NETWORK NetworkSettings;

// OLED display sections
const V_RECT FullScreen = { { 0, 0 }, { 127, 31 } };
//const V_RECT LT_S = { { 0, 0 }, { 41, 9 } };
const V_RECT LT_S = { { 0, 0 }, { 62, 9 } };
const V_RECT LM_S = { { 0, 10 }, { 62, 19 } };
const V_RECT MT_S = { { 42, 0 }, { 85, 9 } };
//const V_RECT RT_S = { { 86, 0 }, { 127, 9 } };
const V_RECT RT_S = { { 65, 0 }, { 127, 9 } };
const V_RECT RM_S = { { 65, 10 }, { 127, 19 } };
const V_RECT BL_S = { { 0, 21 }, { 127, 31 } };

char hexStr[MAX_HEX_STR][MAX_HEX_STR_LENGTH];
uint8_t hexStrIdx = 0;

FlagStatus BoardInit              = RESET;
FlagStatus ButtonPressed          = RESET;
FlagStatus PWR_BOOST_DIS          = SET;

uint16_t B1_USER_BTN_DEBOUNCE_CNT = 0;

// public functions ----------------------------------------------------------------------

int main(void)
{
    // MPU Configuration ------------------------------------------------------
    MPU_Config();
    // Enable I-Cache ---------------------------------------------------------
    SCB_EnableICache();
    // Enable D-Cache ---------------------------------------------------------
    SCB_EnableDCache();

    // Reset of all peripherals, Initializes the Flash interface and the Systick.
    HAL_Init();

    // Configure the system clock ----------------------------------------------
    SystemClock_Config();
    // Configure the peripherals common clocks ---------------------------------
    PeriphCommonClock_Config();

    // debug UART init ---------------------------------------------------------
    MX_USART3_UART_Init();
    printf("=== System Starting ===\r\n");
    
    // initialize the firmware settings ---------------------------------------
    STM32_ERROR_CHECK(ERR_FW_InitSettings, FW_InitSettings());

    // Initialize all configured peripherals
    MX_GPIO_Init();
    MX_DTS_Init();
    MX_RNG_Init();

    // I2C and OLED init --------------------------------------------------------------
    MX_I2C2_Init();
    ssd1306_Init();

    Show_Intector_Logo();
    Show_TagID_Logo();

    Show_OLED_Message("*** Info ***", "start initializing", 100);

    // temporary blocked until PSRAM issue is cleared -------------------------

    // MX_TIM1_Init();
    // MX_TIM15_Init();

    // MX_DMA_Init();
    // MX_ADC1_Init();
    // MX_ADC2_Init();
    // MX_ADC3_Init();
    // ADC_Buffers_Init();
    // ADC1_DMA_Start();
    // ADC2_DMA_Start();
    // ADC3_DMA_Start();

    Show_OLED_Message("*** Info ***", "Ethernet init", 50);
    MX_ETH_Init();

    // PSRAM init & calibration -----------------------------------------------
    Show_OLED_Message("*** Info ***", "OCTOSPI1 init", 50);
    MX_OCTOSPI1_Init();
    smart_delay(2);                 // tPU power-up
    Show_OLED_Message("*** Info ***", "PSRAM init", 50);
    PSRAM_GlobalReset();            // reset to defaults
    PSRAM_DLYB_TuneDelayLine();     // measure period + apply /4 (does DeInit/ReInit inside)
    PSRAM_ConfigureMemory();        // write MR0/MR8 + read-back verify (indirect mode)

    Show_OLED_Message("*** Info ***", "PSRAM mapping", 50);
    PSRAM_EnableMemMapped();        // now enter memory-mapped with 16-bit DTR commands

    PSRAM_DLYB_CalibratePhase();    // sweep SEL taps, find optimal window
    Show_OLED_Message("*** Info ***", "PSRAM Bus Verify", 50);
    PSRAM_BusVerify();              // test on non-cached region
    Show_OLED_Message("*** Info ***", "PSRAM test", 50);
    PSRAM_1K_Diag();

    // board init finished --------------------------------------------------------------

    Show_OLED_Message("*** Initialization done ***", "starting AZ-RTOS", 50);
    BoardInit = SET;

    // AZ-RTOS start -----------------------------------------------------------
    MX_ThreadX_Init();

    // We should never get here as control is now taken by the scheduler -------
    // Infinite loop -----------------------------------------------------------
    while (1)
    {
        // nothing to see here...
        HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_SET);
        smart_delay(100);
        HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_RESET);
        smart_delay(250);
    }
}

void Show_Intector_Logo(void)
{
    V_RECT _Label = { { 0, 0 }, { 127, 20 } };
    ssd1306_Fill(Black);
    
    // ssd1306_SetCursor(25, 0);
    ssd1306_WriteString_Aligned(_Label, CENTER, "Intector", font_ubuntu_18pt, White);
    _Label.Start.Y = 21;
    _Label.End.Y = 31;
    // ssd1306_SetCursor(22, 21);
    ssd1306_WriteString_Aligned(_Label, CENTER, "the technical edge", font_ubuntu_8pt, White);
    ssd1306_UpdateScreen();
    smart_delay(1000); // wait for a second
}

void Show_TagID_Logo(void)
{
    V_RECT _Label = { { 0, 5 }, { 127, 31 } };
    ssd1306_Fill(Black);

    //ssd1306_SetCursor(25, 5);
    ssd1306_WriteString_Aligned(_Label, CENTER, "TAG-ID", font_ubuntu_18pt, White);

    ssd1306_FillRectangle(29, 0, 97, 1, White);
    ssd1306_FillRectangle(29, 30, 97, 31, White);

    ssd1306_UpdateScreen();
    smart_delay(1000); // wait for a second
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    // Supply configuration update enable --------------------------------------

    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    // Configure the main internal regulator output voltage --------------------

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    // Configure LSE Drive Capability

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    // Initializes the RCC Oscillators -----------------------------------------
    // according to the specified parameters 
    // in the RCC_OscInitTypeDef structure.

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState       = RCC_LSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 5;
    RCC_OscInitStruct.PLL.PLLN       = 110;
    RCC_OscInitStruct.PLL.PLLP       = 1;
    RCC_OscInitStruct.PLL.PLLQ       = 2;
    RCC_OscInitStruct.PLL.PLLR       = 5;
    RCC_OscInitStruct.PLL.PLLRGE     = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL  = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN   = 0;
    STM32_ERROR_CHECK(ERR_RCC_OscConfig, HAL_RCC_OscConfig(&RCC_OscInitStruct));

    // Initializes the CPU, AHB and APB buses clocks --------------------------

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                       RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    STM32_ERROR_CHECK(ERR_RCC_ClockConfig, HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3));
}

void PeriphCommonClock_Config(void)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

    // Initializes the peripherals clock for PLL2

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_OSPI | RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_SPI2 | RCC_PERIPHCLK_SPI1;
    PeriphClkInitStruct.PLL2.PLL2M           = 5;
    PeriphClkInitStruct.PLL2.PLL2N           = 120;
    PeriphClkInitStruct.PLL2.PLL2P           = 6;
    PeriphClkInitStruct.PLL2.PLL2Q           = 2;
    PeriphClkInitStruct.PLL2.PLL2R           = 3;
    PeriphClkInitStruct.PLL2.PLL2RGE         = RCC_PLL2VCIRANGE_2;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL      = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN       = 0;
    PeriphClkInitStruct.OspiClockSelection   = RCC_OSPICLKSOURCE_PLL2;
    PeriphClkInitStruct.SdmmcClockSelection  = RCC_SDMMCCLKSOURCE_PLL2;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;

    STM32_ERROR_CHECK(ERR_RCCEx_PeriphCLKConfig, HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct));

    // Initializes the peripherals clock for PLL3
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    PeriphClkInitStruct.PLL3.PLL3M           = 10;
    PeriphClkInitStruct.PLL3.PLL3N           = 120;
    PeriphClkInitStruct.PLL3.PLL3P           = 1;
    PeriphClkInitStruct.PLL3.PLL3Q           = 4;
    PeriphClkInitStruct.PLL3.PLL3R           = 6;
    PeriphClkInitStruct.PLL3.PLL3RGE         = RCC_PLL3VCIRANGE_1;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL      = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN       = 0.0;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C1235CLKSOURCE_PLL3;

    STM32_ERROR_CHECK(ERR_RCCEx_PeriphCLKConfig, HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct));
}

/*** info ***
//
// MPU Configuration for STM32H723ZGTx + OCTOSPI PSRAM + Ethernet
//
// Region Map:
// ┌────────┬────────────┬────────┬──────────────────────────┬─────────────────────────────┐
// │ Region │ Address    │ Size   │ Policy                   │ Purpose                     │
// ├────────┼────────────┼────────┼──────────────────────────┼─────────────────────────────┤
// │ 0      │ 0x00000000 │ 4 GB   │ No Access                │ Background deny             │
// │ 1      │ 0x08000000 │ 1 MB   │ RO, Cacheable (WT)       │ Flash                       │
// │ 2      │ 0x24000000 │ 512 KB │ Non-cacheable Normal     │ AXI SRAM (general + ETH)    │
// │ 3      │ 0x24000000 │ 1 KB   │ Strongly-Ordered         │ ETH DMA descriptors overlay │
// │ 4      │ 0x30000000 │ 32 KB  │ Non-cacheable, Shareable │ SRAM1+SRAM2 (DMA buffers)   │
// │ 5      │ 0x90000000 │ 4 MB   │ WB, R+W Allocate         │ PSRAM ThreadX heap          │
// │ 6      │ 0x90400000 │ 4 MB   │ Non-cacheable Normal     │ PSRAM DMA buffers           │
// │ 7      │ (free)     │        │                          │ Reserved for future use     │
// └────────┴────────────┴────────┴──────────────────────────┴─────────────────────────────┘
//
// Notes:
// - DTCM (0x20000000, 128 KB) gets default cacheable attributes from background
// - Region 3 overlays Region 2 at 0x24000000 — higher region number wins,
// so the first 1 KB is Strongly-Ordered for ETH DMA descriptors
// - AXI SRAM is 320 KB on H723; the 512 KB MPU region covers some unmapped
// space beyond it, which is harmless
// - SRAM4 (0x38000000, 16 KB) not configured — add Region 7 if needed
*** info ***/

void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};

    HAL_MPU_Disable();

    // -- Region 0: 4 GB background - deny all --------------------------------
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress      = 0x00000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // -- Region 1: 1 MB Flash - read-only, cacheable (Write-Through) ---------
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
    MPU_InitStruct.BaseAddress      = 0x08000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_1MB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // -- Region 7: 128 KB Flash Sector 7 - R/W for settings storage -----------
    // Overlays Region 1 at Sector 7. Higher region number wins,
    // so this 128 KB becomes writable while Sectors 0-6 stay read-only.
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER7;
    MPU_InitStruct.BaseAddress      = 0x080E0000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_128KB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // -- Region 2: 512 KB AXI SRAM - Non-cacheable Normal --------------------
    // Covers 0x24000000–0x2407FFFF (H723 AXI SRAM is 320 KB)
    // Non-cacheable so ETH DMA and other bus masters see coherent data
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
    MPU_InitStruct.BaseAddress      = 0x24000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // -- Region 3: 1 KB ETH DMA descriptors - Strongly-Ordered ---------------
    // Overlays first 1 KB of Region 2 at 0x24000000
    // Higher region number wins → this 1 KB becomes Device/Strongly-
    // Ordered, ensuring ETH DMA descriptor writes are not reordered
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
    MPU_InitStruct.BaseAddress      = 0x24000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_1KB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // -- Region 4: 32 KB SRAM1+SRAM2 - Non-cacheable, Shareable --------------
    // 0x30000000–0x30007FFF: SRAM1 (16 KB) + SRAM2 (16 KB)
    // Shareable for DMA coherency with bus masters
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER4;
    MPU_InitStruct.BaseAddress      = 0x30000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_32KB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // -- Region 5: 4 MB PSRAM - ThreadX heap (Write-Back, R+W Allocate) ------
    // TEX=1, C=1, B=1 → WB/WA caching for heavy pointer-chasing
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER5;
    MPU_InitStruct.BaseAddress      = 0x90000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_4MB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    // -- Region 6: 4 MB PSRAM - DMA buffers (Non-cacheable Normal) -----------
    // TEX=1, C=0, B=0 → no cache maintenance needed for DMA
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER6;
    MPU_InitStruct.BaseAddress      = 0x90400000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_4MB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        HAL_IncTick();

        if (BoardInit) {
            GPIO_PinState pinState = HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin);

            if (HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin)) {
                if (B1_USER_BTN_DEBOUNCE_CNT < BTN_DEBOUNCE_TIME) {
                    B1_USER_BTN_DEBOUNCE_CNT++;
                }
                else if (B1_USER_BTN_DEBOUNCE_CNT == BTN_DEBOUNCE_TIME) {
                    if (!ButtonPressed) {
                        // User button pressed — available for demo use
                    }
                    ButtonPressed = SET;
                }
            }
            else {
                B1_USER_BTN_DEBOUNCE_CNT = 0;
                ButtonPressed            = RESET;
            }
        }
    }
}

char *hex2Str(unsigned char *data, size_t dataLen)
{
#if (USE_LOGGER == LOGGER_ON)
    const char *hex = "0123456789ABCDEF";

    unsigned char *pin = data;
    char *pout = hexStr[hexStrIdx];

    uint8_t idx = hexStrIdx;

    if (dataLen > (MAX_HEX_STR_LENGTH / 2)) {
        dataLen = (MAX_HEX_STR_LENGTH / 2) - 1;
    }

    for (uint32_t i = 0; i < dataLen; i++) {
        *pout++ = hex[(*pin >> 4) & 0x0F];
        *pout++ = hex[(*pin++) & 0x0F];
    }
    *pout = 0;

    hexStrIdx++;
    hexStrIdx %= MAX_HEX_STR;

    return hexStr[idx];
#else
    return NULL;
#endif // #if USE_LOGGER == LOGGER_ON
}

void smart_delay(uint32_t ms)
{
    extern ULONG _tx_thread_system_state;
    extern TX_THREAD *_tx_thread_current_ptr;

    if (_tx_thread_system_state == 0 && _tx_thread_current_ptr != TX_NULL) {
        tx_thread_sleep((ULONG)(ms * TX_TIMER_TICKS_PER_SECOND / 1000));
    }
    else {
        HAL_Delay(ms);
    }
}

void Error_Handler(STM_ERR_T ERR_ID, STM_ERR_T ERR_CODE)
{
    if ((ERR_ID == 0x00) & (ERR_CODE == 0x00)) {
        return;
    }

    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);

    char _err_text[50];
    sprintf(_err_text, "ID:%08lX RC:%02lX", (uint32_t)(ERR_ID >> 32), (uint32_t)ERR_CODE);

    Show_OLED_Message("! ! ! ERROR ! ! !", _err_text, 50);

    while (1)
    {
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
        smart_delay(10);
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);
        smart_delay(10);
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
        smart_delay(10);
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);
        smart_delay(10);
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
        smart_delay(10);
        HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);
        smart_delay(100);
    }
}

void Show_OLED_Message(char *msg_1, char *msg_2, uint32_t msg_time) {
    V_RECT RectLineA = {{0, 0}, {127, 15}};
    V_RECT RectLineB = {{0, 16}, {127, 31}};
    ssd1306_Fill(Black);
    // ssd1306_SetCursor(0, 0);

    ssd1306_WriteString_Aligned(RectLineA, CENTER, msg_1, font_ubuntuCondensed_12pt, White);
    ssd1306_WriteString_Aligned(RectLineB, CENTER, msg_2, font_ubuntuCondensed_12pt, White);

    ssd1306_UpdateScreen();
    if (msg_time > 0) {
        smart_delay(msg_time);
    }
    else {
        while (1) {
        }
    }
}

#ifdef  USE_FULL_ASSERT
// ---------------------------------------------------------------------------------------
// @brief  Reports the name of the source file and the source line number
//         where the assert_param error has occurred.
// @param  file: pointer to the source file name
// @param  line: assert_param error line source number
// @retval None
// ---------------------------------------------------------------------------------------
void assert_failed(uint8_t *file, uint32_t line)
{
    // -------------------------------------------------------------------------
    // implement report with the file name and line number later,
    // ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)
    // -------------------------------------------------------------------------

    __disable_irq();
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);
    while (1)
    {
        HAL_GPIO_TogglePin(LED_Yellow_GPIO_Port, LED_Yellow_Pin);
        HAL_GPIO_TogglePin(LED_Red_GPIO_Port, LED_Red_Pin);
        smart_delay(200);
    }
}
#endif // USE_FULL_ASSERT 
