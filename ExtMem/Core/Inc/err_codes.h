/**
 ****************************************************************************************
 * @file           : err_codes.h
 * @brief          : error code definitions
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
 *
 * Encoding (64-bit):
 *
 *   0x 80 GG MM EE 00000000
 *      |  |  |  |
 *      |  |  |  +--- Error number within module  (byte 4, 00-FF)
 *      |  |  +------ Module within group          (byte 5, 00-FF)
 *      |  +--------- Group / subsystem            (byte 6, 00-FF)
 *      +------------ Error flag                   (byte 7, always 0x80)
 *
 ****************************************************************************************
 */

// Define to prevent recursive inclusion -------------------------------------------------

#ifndef __ERR_CODES_H
#define __ERR_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

// Group 0x00 -- System ------------------------------------------------------------------

#define STM32_OK                       0x0000000000000000ULL // Success (no error)
#define STM32_FAIL                     0x8000000100000000ULL // Generic failure

#define ERR_RCC_OscConfig              0x8000010100000000ULL // RCC Oscillator config
#define ERR_RCC_ClockConfig            0x8000010200000000ULL // RCC Clock config
#define ERR_RCCEx_PeriphCLKConfig      0x8000010300000000ULL // RCC Extended Periph CLK config
#define ERR_RTC_INIT                   0x8000010400000000ULL // RTC init
#define ERR_RTCEx_PeriphCLKConfig      0x8000010500000000ULL // RTC Extended Periph CLK config

#define ERR_FW_InitSettings            0x8000020100000000ULL // Firmware settings init

#define ERR_HAL_RNG_Init               0x8000030100000000ULL // RNG init

// Group 0x01 -- Analog (ADC, OPAMP) -----------------------------------------------------

#define ERR_HAL_ADC_Init               0x8001000100000000ULL // ADC init
#define ERR_HAL_ADCEx_MMCC             0x8001000200000000ULL // ADC Ex MultiMode Channel config
#define ERR_HAL_ADC_ConfigChannel      0x8001000300000000ULL // ADC Config Channel
#define ERR_HAL_ADCEx_Calibration      0x8001000400000000ULL // ADC Ex Calibration Start

#define ERR_HAL_OPAMP1_Init            0x8001010100000000ULL // OPAMP1 init
#define ERR_HAL_OPAMP2_Init            0x8001010200000000ULL // OPAMP2 init

#define ERR_ADC_SERVICE_Init           0x8001020100000000ULL // ADC service init
#define ERR_ADC_SERVICE_Start          0x8001020200000000ULL // ADC service start

// Group 0x02 -- Timers (TIM, PWM) -------------------------------------------------------

#define ERR_HAL_TIM_PWM_Init           0x8002000100000000ULL // TIM PWM init
#define ERR_HAL_TIMEx_MCS              0x8002000200000000ULL // TIMEx MasterConfig Sync
#define ERR_HAL_TIM_PWM_CC             0x8002000300000000ULL // TIM PWM Config Channel
#define ERR_HAL_TIMEx_ConfigBDT        0x8002000400000000ULL // TIMEx Config BreakDeadTime

#define ERR_HAL_TIM3_CH4_PWM_Init      0x8002010100000000ULL // TIM3_CH4 PWM init

// Group 0x03 -- Communication (UART, I2C, SPI) ------------------------------------------

#define ERR_UART_MspInit               0x8003000100000000ULL // UART MspInit
#define ERR_HAL_UART_Init              0x8003000200000000ULL // UART init
#define ERR_HAL_UARTEx_SetTxFifoTH     0x8003000300000000ULL // UART Tx FIFO threshold
#define ERR_HAL_UARTEx_SetRxFifoTH     0x8003000400000000ULL // UART Rx FIFO threshold
#define ERR_HAL_UARTEx_DisableFifo     0x8003000500000000ULL // UART disable FIFO

#define ERR_HAL_I2C_Init               0x8003010100000000ULL // I2C init
#define ERR_HAL_I2CEx_ConfigAnalogFilter  0x8003010200000000ULL // I2C analog filter config
#define ERR_HAL_I2CEx_ConfigDigitalFilter 0x8003010300000000ULL // I2C digital filter config

#define ERR_HAL_SPI_Init               0x8003020100000000ULL // SPI init

#define ERR_HAL_ETH_Init               0x8003030100000000ULL // Ethernet init

// Group 0x04 -- DMA ---------------------------------------------------------------------

#define ERR_HAL_DMA_Init               0x8004000100000000ULL // DMA init

// Group 0x05 -- Storage (SDMMC/eMMC) ----------------------------------------------------

#define ERR_HAL_MMC_Init               0x8005000100000000ULL // eMMC/SDMMC init
#define ERR_HAL_MMC_GetCardInfo        0x8005000200000000ULL // MMC GetCardInfo

// Group 0x06 -- Memory (OCTOSPI/PSRAM) --------------------------------------------------

#define ERR_PSRAM_OSPI_Init            0x8006000100000000ULL // OCTOSPI1 peripheral init
#define ERR_PSRAM_OSPIM_Config         0x8006000200000000ULL // OCTOSPI I/O manager config

#define ERR_PSRAM_DLYB_Tune            0x8006010100000000ULL // DLYB delay line tuning
#define ERR_PSRAM_DLYB_SetConfig       0x8006010200000000ULL // DLYB config application
#define ERR_PSRAM_DLYB_Verify          0x8006010300000000ULL // DLYB readback mismatch
#define ERR_PSRAM_DLYB_NoValidPhase    0x8006010400000000ULL // DLYB no valid sampling window

#define ERR_PSRAM_WriteReg_MR0         0x8006020100000000ULL // Write register MR0
#define ERR_PSRAM_ReadReg_MR0          0x8006020200000000ULL // Read register MR0
#define ERR_PSRAM_ReadReg_MR2          0x8006020300000000ULL // Read register MR2
#define ERR_PSRAM_ReadReg_MR4          0x8006020400000000ULL // Read register MR4
#define ERR_PSRAM_WriteReg_MR8         0x8006020500000000ULL // Write register MR8
#define ERR_PSRAM_ReadReg_MR8          0x8006020600000000ULL // Read register MR8
#define ERR_PSRAM_WriteReg_Cmd         0x8006020700000000ULL // MR write: HAL_OSPI_Command
#define ERR_PSRAM_WriteReg_Tx          0x8006020800000000ULL // MR write: HAL_OSPI_Transmit
#define ERR_PSRAM_ReadReg_Cmd          0x8006020900000000ULL // MR read: HAL_OSPI_Command
#define ERR_PSRAM_ReadReg_Rx           0x8006020A00000000ULL // MR read: HAL_OSPI_Receive

#define ERR_PSRAM_MR0_Verify           0x8006030100000000ULL // MR0 read-back mismatch
#define ERR_PSRAM_MR4_Verify           0x8006030200000000ULL // MR4 read-back mismatch
#define ERR_PSRAM_MR8_Verify           0x8006030300000000ULL // MR8 read-back mismatch

#define ERR_PSRAM_MemMapped_WriteCfg   0x8006040100000000ULL // Memory-mapped write cmd config
#define ERR_PSRAM_MemMapped_ReadCfg    0x8006040200000000ULL // Memory-mapped read cmd config
#define ERR_PSRAM_MemMapped_Enable     0x8006040300000000ULL // Memory-mapped mode activation

#define ERR_PSRAM_GlobalReset_Cmd      0x8006050100000000ULL // Global reset command

#define ERR_PSRAM_WriteReadTest        0x8006060100000000ULL // Write/read verification test

// Group 0x07 -- RTOS (ThreadX) ----------------------------------------------------------

#define ERR_ThreadX_Init               0x8007000100000000ULL // ThreadX init
#define ERR_Tx_BytePoolCreate          0x8007000200000000ULL // Byte pool create

#define ERR_Tx_StatusEventFlagsCreate  0x8007010100000000ULL // Status event flags create
#define ERR_Tx_CtrlEventFlagsCreate    0x8007010200000000ULL // Control event flags create
#define ERR_Tx_EventFlagsSet           0x8007010300000000ULL // Event flags set
#define ERR_Tx_EventFlagsGet           0x8007010400000000ULL // Event flags get

#define ERR_Tx_ByteAllocate            0x8007020100000000ULL // Byte allocate
#define ERR_Tx_ThreadCreate            0x8007020200000000ULL // Thread create

#define ERR_Tx_QueueCreate             0x8007030100000000ULL // Queue create
#define ERR_Tx_QueueSend               0x8007030200000000ULL // Queue send
#define ERR_Tx_QueueReceive            0x8007030300000000ULL // Queue receive

#define ERR_Tx_SemaphoreCreate         0x8007040100000000ULL // Semaphore create
#define ERR_Tx_MutexCreate             0x8007040200000000ULL // Mutex create

// Group 0x08 -- FileX -------------------------------------------------------------------

#define ERR_FX_Init                    0x8008000100000000ULL // FileX init
#define ERR_FX_MediaOpen               0x8008000200000000ULL // Media open
#define ERR_FX_MediaSpaceAvailable     0x8008000300000000ULL // Media extended space available

// Group 0x09 -- NetXDuo / Network -------------------------------------------------------

#define ERR_NX_Init                    0x8009000100000000ULL // NetXDuo init
#define ERR_NX_PacketPoolCreate        0x8009000200000000ULL // Packet pool create
#define ERR_NX_IpCreate                0x8009000300000000ULL // IP instance create
#define ERR_NX_ArpEnable               0x8009000400000000ULL // ARP enable
#define ERR_NX_IcmpEnable              0x8009000500000000ULL // ICMP enable
#define ERR_NX_TcpEnable               0x8009000600000000ULL // TCP enable
#define ERR_NX_UdpEnable               0x8009000700000000ULL // UDP enable

#define ERR_NX_HTTP_ServerCreate       0x8009010100000000ULL // HTTP server create
#define ERR_NX_HTTP_MimeMapsSet        0x8009010200000000ULL // HTTP MIME maps set
#define ERR_NX_HTTP_ServerStart        0x8009010300000000ULL // HTTP server start

#define ERR_NX_FTP_ServerCreate        0x8009020100000000ULL // FTP server create
#define ERR_NX_FTP_ServerStart         0x8009020200000000ULL // FTP server start

#define ERR_NX_WS_ServerCreate         0x8009030100000000ULL // WebSocket server create
#define ERR_NX_WS_ServerStart          0x8009030200000000ULL // WebSocket server start

// Group 0x0A -- Board/GPIO --------------------------------------------------------------

#define ERR_BOARD_DetectionFailed      0x800A000100000000ULL // Board detection failed
#define ERR_BOARD_InvalidType          0x800A000200000000ULL // Invalid board type detected
#define ERR_BOARD_ValidationFailed     0x800A000300000000ULL // Board validation failed
#define ERR_BOARD_GPIOConfigFailed     0x800A000400000000ULL // Board GPIO configuration failed
#define ERR_BOARD_ResourceInitFailed   0x800A000500000000ULL // Board resource initialization failed
#define ERR_BOARD_DetectionPinConfig   0x800A000600000000ULL // Detection pin configuration failed
#define ERR_BOARD_CapabilityMismatch   0x800A000700000000ULL // Board capability mismatch
#define ERR_BOARD_UnsupportedOperation 0x800A000800000000ULL // Unsupported operation for board type

#define ERR_GPIO_ConfigFailed          0x800A010100000000ULL // GPIO configuration failed
#define ERR_GPIO_PinReconfigFailed     0x800A010200000000ULL // Pin reconfiguration failed
#define ERR_GPIO_InvalidPin            0x800A010300000000ULL // Invalid pin specification
#define ERR_GPIO_ClockEnableFailed     0x800A010400000000ULL // GPIO clock enable failed
#define ERR_GPIO_ModeConflict          0x800A010500000000ULL // GPIO mode conflict detected
#define ERR_GPIO_BoardSpecificFailed   0x800A010600000000ULL // Board-specific GPIO config failed

// Group 0x0B -- Display -----------------------------------------------------------------

#define ERR_DISPLAY_InitFailed         0x800B000100000000ULL // Display initialization failed
#define ERR_DISPLAY_SPI_Config         0x800B000200000000ULL // Display SPI configuration failed
#define ERR_DISPLAY_BufferAlloc        0x800B000300000000ULL // Display buffer allocation failed
#define ERR_DISPLAY_ControlPin         0x800B000400000000ULL // Display control pin configuration failed
#define ERR_DISPLAY_ParallelRefresh    0x800B000500000000ULL // Parallel display refresh failed

// Group 0x0C -- Power/LED ---------------------------------------------------------------

#define ERR_LED_BoardNotSupported      0x800C000100000000ULL // LED not supported on this board
#define ERR_LED_SPIConfig              0x800C000200000000ULL // LED SPI configuration failed
#define ERR_LED_BDMAConfig             0x800C000300000000ULL // LED BDMA configuration failed
#define ERR_LED_ClockDomainConfig      0x800C000400000000ULL // LED clock domain configuration failed

#define ERR_POWER_ControlInitFailed    0x800C010100000000ULL // Power control initialization failed
#define ERR_POWER_PWMConfig            0x800C010200000000ULL // Power PWM configuration failed
#define ERR_POWER_H_BRIDGE_Config      0x800C010300000000ULL // H-bridge configuration failed

// Group 0x0D -- IR ----------------------------------------------------------------------

#define ERR_IR_RECEIVE_InitFailed      0x800D000100000000ULL // IR receive initialization failed
#define ERR_IR_RECEIVE_SpeedDetection  0x800D000200000000ULL // IR speed detection failed
#define ERR_IR_RECEIVE_SyncConfig      0x800D000300000000ULL // IR synchronization configuration failed
#define ERR_IR_RECEIVE_DMAConfig       0x800D000400000000ULL // IR receive DMA configuration failed

#define ERR_IR_TRANSMIT_InitFailed     0x800D010100000000ULL // IR transmit initialization failed
#define ERR_IR_TRANSMIT_SPIConfig      0x800D010200000000ULL // IR transmit SPI configuration failed

#ifdef __cplusplus
}
#endif

#endif // __ERR_CODES_H
