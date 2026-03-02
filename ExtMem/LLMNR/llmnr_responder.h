/**
  ****************************************************************************************
  * @file           : llmnr_responder.h
  * @brief          : LLMNR (Link-Local Multicast Name Resolution) responder
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
#ifndef __LLMNR_RESPONDER_H
#define __LLMNR_RESPONDER_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "nx_api.h"
#include "tx_api.h"

// Defines -------------------------------------------------------------------------------
#define LLMNR_PORT              5355
#define LLMNR_MCAST_ADDR        IP_ADDRESS(224, 0, 0, 252)
#define LLMNR_TTL               30

// Functions prototypes ------------------------------------------------------------------

/// Initialize LLMNR responder (allocate resources). Call from MX_NetXDuo_Init.
UINT LLMNR_Responder_Init(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                           VOID *stack_ptr, ULONG stack_size, const char *hostname);

/// Start the LLMNR responder thread. Call from nx_app_thread_entry after network is up.
UINT LLMNR_Responder_Start(void);

#ifdef __cplusplus
}
#endif

#endif // __LLMNR_RESPONDER_H
