/**
  ****************************************************************************************
 * @file           : ws_server.h
 * @brief          : Dedicated WebSocket TCP server (bypasses HTTP server)
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

#ifndef WS_SERVER_H
#define WS_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "nx_api.h"

// WebSocket GUID for handshake (RFC 6455)
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// WebSocket opcodes
#define WS_OPCODE_TEXT   0x01
#define WS_OPCODE_BINARY 0x02
#define WS_OPCODE_CLOSE  0x08
#define WS_OPCODE_PING   0x09
#define WS_OPCODE_PONG   0x0A

// WebSocket frame flags
#define WS_FIN_FLAG  0x80
#define WS_MASK_FLAG 0x80

// Maximum connected WebSocket clients
#define WS_MAX_CLIENTS 4

// WebSocket server configuration

// Packet pool configuration
#define WS_SERVER_MIN_PACKET_SIZE 1024
#define WS_SERVER_PACKET_SIZE     WS_SERVER_MIN_PACKET_SIZE          // 1024 bytes for JSON messages
#define WS_SRV_PACKET_POOL_SIZE   (20 * WS_SERVER_MIN_PACKET_SIZE)   // 20KB for JSON telemetry

// Thread stack configuration
#define WS_SERVER_THREAD_STACK_SIZE 4096                             // 4KB for accept thread stack
#define WS_SRV_STACK_SIZE           WS_SERVER_THREAD_STACK_SIZE      // Alias for clarity

// Server configuration
#define WS_SERVER_PORT              8080
#define WS_SERVER_LISTEN_QUEUE_SIZE 4
#define WS_SERVER_THREAD_PRIORITY   12
#define WS_SERVER_ACCEPT_TIMEOUT    (1 * NX_IP_PERIODIC_RATE)

// WebSocket client connection state
typedef struct
{
    NX_TCP_SOCKET socket;
    ULONG last_activity;
    UINT active;
    UINT handshake_complete;
} WS_ClientConnection_t;

// WebSocket server instance
typedef struct
{
    NX_TCP_SOCKET listen_socket;
    WS_ClientConnection_t clients[WS_MAX_CLIENTS];
    TX_THREAD accept_thread;
    TX_MUTEX clients_mutex;
    NX_IP *ip_instance;
    NX_PACKET_POOL *packet_pool;
    UINT initialized;
} WS_Server_t;

// Function prototypes
UINT WS_Server_Create(WS_Server_t *server, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, VOID *stack_ptr);
UINT WS_Server_Start(WS_Server_t *server);
UINT WS_Server_Stop(WS_Server_t *server);
UINT WS_Server_SendJSON(WS_Server_t *server, const char *json_string);
void WS_Server_AcceptThread_Entry(ULONG thread_input);

#ifdef __cplusplus
}
#endif

#endif // WS_SERVER_H
