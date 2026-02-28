/**
  ****************************************************************************************
 * @file    app_azure_rtos.c
 * @brief   app_azure_rtos application implementation file
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
#include "app_azure_rtos.h"
#include "stm32h7xx.h"

// Variables -----------------------------------------------------------------------------
TX_EVENT_FLAGS_GROUP TAGID_status_event_group;

TX_EVENT_FLAGS_GROUP TAGID_ctrl_event_group;

static UCHAR __attribute__((section(".TxPoolSection"))) tx_byte_pool_buffer[TX_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL tx_app_byte_pool;

static UCHAR __attribute__((section(".FxPoolSection"))) fx_byte_pool_buffer[FX_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL fx_app_byte_pool;

static UCHAR __attribute__((section(".NxPoolSection"))) nx_byte_pool_buffer[NX_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL nx_app_byte_pool;

// function code -------------------------------------------------------------------------
VOID tx_application_define(VOID *first_unused_memory)
{
	VOID *memory_ptr;

	// create event flags for status group
	STM32_ERROR_CHECK(ERR_Tx_StatusEventFlagsCreate, tx_event_flags_create(&TAGID_status_event_group, "TAGID_status_event_group_name"));
	// create event flags for ctrl group
	STM32_ERROR_CHECK(ERR_Tx_CtrlEventFlagsCreate, tx_event_flags_create(&TAGID_ctrl_event_group, "TAGID_ctrl_event_group_name"));

	STM32_ERROR_CHECK(ERR_Tx_BytePoolCreate, tx_byte_pool_create(&tx_app_byte_pool, "Tx App memory pool", tx_byte_pool_buffer, TX_APP_MEM_POOL_SIZE));

	memory_ptr = (VOID *)&tx_app_byte_pool;
	STM32_ERROR_CHECK(ERR_ThreadX_Init, App_ThreadX_Init(memory_ptr));


	STM32_ERROR_CHECK(ERR_Tx_BytePoolCreate, tx_byte_pool_create(&fx_app_byte_pool, "Fx App memory pool", fx_byte_pool_buffer, FX_APP_MEM_POOL_SIZE));

	memory_ptr = (VOID *)&fx_app_byte_pool;
	STM32_ERROR_CHECK(ERR_FX_Init, MX_FileX_Init(memory_ptr));

	STM32_ERROR_CHECK(ERR_Tx_BytePoolCreate, tx_byte_pool_create(&nx_app_byte_pool, "Nx App memory pool", nx_byte_pool_buffer, NX_APP_MEM_POOL_SIZE));

	memory_ptr = (VOID *)&nx_app_byte_pool;
	STM32_ERROR_CHECK(ERR_NX_Init, MX_NetXDuo_Init(memory_ptr));

}
