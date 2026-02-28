/**
  ****************************************************************************************
  * @file           : eth.c
  * @brief          : This file provides code for the configuration
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
#include "eth.h"

ETH_DMADescTypeDef __attribute__((section(".Mem_ETH_Rx_DMA_Descr_Section"))) DMARxDscrTab[ETH_RX_DESC_CNT]; // Ethernet Rx DMA Descriptors
ETH_DMADescTypeDef __attribute__((section(".Mem_ETH_Tx_DMA_Descr_Section"))) DMATxDscrTab[ETH_TX_DESC_CNT]; // Ethernet Tx DMA Descriptors
ETH_BufferTypeDef __attribute__((section(".Mem_ETH_Rx_Buff_Section"))) Txbuffer[ETH_TX_DESC_CNT * 2U];
ETH_TxPacketConfig TxConfig;

ETH_HandleTypeDef heth;

UINT PingCnt = 0;


// ETH init function
void MX_ETH_Init(void)
{
	static uint8_t MACAddr[6];

	heth.Instance = ETH;
	MACAddr[0] = 0xDE;
	MACAddr[1] = 0xAD;
	MACAddr[2] = 0xBE;
	MACAddr[3] = 0xEF;
	MACAddr[4] = 0x17;
	MACAddr[5] = 0x64;
	heth.Init.MACAddr = &MACAddr[0];
	heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
	heth.Init.TxDesc = DMATxDscrTab;
	heth.Init.RxDesc = DMARxDscrTab;
	heth.Init.RxBuffLen = DEFAULT_PAYLOAD_SIZE;

	if (HAL_ETH_Init(&heth) != HAL_OK)
	{
		Error_Handler(ERR_HAL_ETH_Init, STM32_FAIL);
	}

	memset(&TxConfig, 0, sizeof(ETH_TxPacketConfig));
	TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
	TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
	TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

}

void HAL_ETH_MspInit(ETH_HandleTypeDef* ethHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (ethHandle->Instance == ETH)
	{
		  // ETH clock enable
		__HAL_RCC_ETH1MAC_CLK_ENABLE();
		__HAL_RCC_ETH1TX_CLK_ENABLE();
		__HAL_RCC_ETH1RX_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();
		// *ETH GPIO Configuration
		// PC1     ------> ETH_MDC
		// PA1     ------> ETH_REF_CLK
		// PA2     ------> ETH_MDIO
		// PA7     ------> ETH_CRS_DV
		// PC4     ------> ETH_RXD0
		// PC5     ------> ETH_RXD1
		// PB11     ------> ETH_TX_EN
		// PB13     ------> ETH_TXD1
		// PG13     ------> ETH_TXD0
		GPIO_InitStruct.Pin = RMII_MDC_Pin | RMII_RXD0_Pin | RMII_RXD1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RMII_REF_CLK_Pin | RMII_MDIO_Pin | RMII_CRS_DV_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RMII_TXD1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RMII_TX_EN_Pin | RMII_TXD0_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		// ETH interrupt Init
		HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(ETH_IRQn);
	}
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef* ethHandle)
{

	if (ethHandle->Instance == ETH)
	{
		  // Peripheral clock disable
		__HAL_RCC_ETH1MAC_CLK_DISABLE();
		__HAL_RCC_ETH1TX_CLK_DISABLE();
		__HAL_RCC_ETH1RX_CLK_DISABLE();

		// *ETH GPIO Configuration
		// PC1     ------> ETH_MDC
		// PA1     ------> ETH_REF_CLK
		// PA2     ------> ETH_MDIO
		// PA7     ------> ETH_CRS_DV
		// PC4     ------> ETH_RXD0
		// PC5     ------> ETH_RXD1
		// PB11     ------> ETH_TX_EN
		// PB13     ------> ETH_TXD1
		// PG13     ------> ETH_TXD0
		HAL_GPIO_DeInit(GPIOC, RMII_MDC_Pin | RMII_RXD0_Pin | RMII_RXD1_Pin);

		HAL_GPIO_DeInit(GPIOA, RMII_REF_CLK_Pin | RMII_MDIO_Pin | RMII_CRS_DV_Pin);

		HAL_GPIO_DeInit(GPIOB, RMII_TXD1_Pin);

		HAL_GPIO_DeInit(GPIOG, RMII_TX_EN_Pin | RMII_TXD0_Pin);

		// ETH interrupt Deinit
		HAL_NVIC_DisableIRQ(ETH_IRQn);
	}
}
