/**
  ****************************************************************************************
 * @file           : ws_server.c (FIXED VERSION)
 * @brief          : Simplified WebSocket TCP server that actually works
  ****************************************************************************************
 *
  * Copyright (c) 2020-2026 Intector Inc.
 * All rights reserved.
 *
  ****************************************************************************************
 */

#include "ws_server.h"
#include "ws_telemetry.h"
#include "ws_commands.h"
#include <stdio.h>
#include <string.h>

// Forward declarations
static UINT ws_perform_handshake(NX_TCP_SOCKET *socket_ptr, NX_PACKET_POOL *pool_ptr);
static UINT WS_ReceiveFrame(NX_TCP_SOCKET *socket_ptr, NX_PACKET_POOL *pool_ptr);
static void ws_compute_accept_key(char *accept_key, const char *client_key);
static char *ws_strnstr(const char *haystack, const char *needle, size_t len);
static void ws_base64_encode(char *dst, unsigned char *src, int len);
static void ws_sha1(uint32_t *hash, uint32_t *block);
static uint32_t ws_rol(uint32_t value, uint32_t bits);
static uint32_t ws_blk(uint32_t *block, uint32_t i);

// start function code -------------------------------------------------------------------
UINT WS_Server_Create(WS_Server_t *server, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, VOID *stack_ptr)
{
    UINT status;

    memset(server, 0, sizeof(WS_Server_t));
    server->ip_instance = ip_ptr;
    server->packet_pool = pool_ptr;

    // Create mutex
    status = tx_mutex_create(&server->clients_mutex, "WS Clients Mutex", TX_NO_INHERIT);
    if (status != TX_SUCCESS) {
        return status;
    }

    // Create listen socket
    status = nx_tcp_socket_create(ip_ptr,
                                  &server->listen_socket,
                                  "WebSocket Listen Socket",
                                  NX_IP_NORMAL,
                                  NX_FRAGMENT_OKAY,
                                  NX_IP_TIME_TO_LIVE,
                                  8192,
                                  NX_NULL,
                                  NX_NULL);
    if (status != NX_SUCCESS) {
        tx_mutex_delete(&server->clients_mutex);
        return status;
    }

    // Create client sockets
    for (int i = 0; i < WS_MAX_CLIENTS; i++) {
        status = nx_tcp_socket_create(ip_ptr,
                                      &server->clients[i].socket,
                                      "WebSocket Client Socket",
                                      NX_IP_NORMAL,
                                      NX_FRAGMENT_OKAY,
                                      NX_IP_TIME_TO_LIVE,
                                      8192,
                                      NX_NULL,
                                      NX_NULL);
        if (status != NX_SUCCESS) {
            // Cleanup
            for (int j = 0; j < i; j++) {
                nx_tcp_socket_delete(&server->clients[j].socket);
            }
            nx_tcp_socket_delete(&server->listen_socket);
            tx_mutex_delete(&server->clients_mutex);
            return status;
        }
        server->clients[i].active = 0;
    }

    // Create accept thread
    status = tx_thread_create(&server->accept_thread,
                              "WebSocket Accept Thread",
                              WS_Server_AcceptThread_Entry,
                              (ULONG)server,
                              stack_ptr,
                              WS_SERVER_THREAD_STACK_SIZE,
                              WS_SERVER_THREAD_PRIORITY,
                              WS_SERVER_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE,
                              TX_DONT_START);
    if (status != TX_SUCCESS) {
        for (int i = 0; i < WS_MAX_CLIENTS; i++) {
            nx_tcp_socket_delete(&server->clients[i].socket);
        }
        nx_tcp_socket_delete(&server->listen_socket);
        tx_mutex_delete(&server->clients_mutex);
        return status;
    }

    server->initialized = 1;
    return NX_SUCCESS;
}

UINT WS_Server_Start(WS_Server_t *server)
{
    UINT status;

    if (!server->initialized) {
        return NX_NOT_CREATED;
    }

    // Bind and listen
    status = nx_tcp_server_socket_listen(server->ip_instance,
                                         WS_SERVER_PORT,
                                         &server->listen_socket,
                                         WS_SERVER_LISTEN_QUEUE_SIZE,
                                         NX_NULL);
    if (status != NX_SUCCESS) {
        return status;
    }

    // Start accept thread
    status = tx_thread_resume(&server->accept_thread);
    if (status != TX_SUCCESS) {
        nx_tcp_server_socket_unlisten(server->ip_instance, WS_SERVER_PORT);
        return status;
    }
    return NX_SUCCESS;
}

UINT WS_Server_Stop(WS_Server_t *server)
{
    if (!server->initialized) {
        return NX_NOT_CREATED;
    }

    nx_tcp_server_socket_unlisten(server->ip_instance, WS_SERVER_PORT);
    tx_thread_terminate(&server->accept_thread);

    tx_mutex_get(&server->clients_mutex, TX_WAIT_FOREVER);
    for (int i = 0; i < WS_MAX_CLIENTS; i++) {
        if (server->clients[i].active) {
            nx_tcp_socket_disconnect(&server->clients[i].socket, NX_NO_WAIT);
            server->clients[i].active = 0;
        }
    }
    tx_mutex_put(&server->clients_mutex);

    return NX_SUCCESS;
}

void WS_Server_AcceptThread_Entry(ULONG thread_input)
{
    WS_Server_t *server = (WS_Server_t *)thread_input;
    UINT status;

    while (1) {
        // Accept connection (blocks until client connects)
        status = nx_tcp_server_socket_accept(&server->listen_socket, TX_WAIT_FOREVER);

        if (status == NX_SUCCESS) {
            // Perform WebSocket handshake on the listen socket
            status = ws_perform_handshake(&server->listen_socket, server->packet_pool);

            if (status == NX_SUCCESS) {
                // Mark first client slot as active
                tx_mutex_get(&server->clients_mutex, TX_WAIT_FOREVER);
                server->clients[0].active             = 1;
                server->clients[0].handshake_complete = 1;
                server->clients[0].last_activity      = tx_time_get();
                tx_mutex_put(&server->clients_mutex);

                // Send initial status update to new client
                WS_SendLedStatus();

                // Receive loop for incoming frames
                while (server->listen_socket.nx_tcp_socket_state == NX_TCP_ESTABLISHED) {
                    status = WS_ReceiveFrame(&server->listen_socket, server->packet_pool);

                    if (status == NX_NOT_CONNECTED) {
                        break;
                    }

                    tx_thread_sleep(1); // 10ms
                }

                // Client disconnected, mark as inactive
                tx_mutex_get(&server->clients_mutex, TX_WAIT_FOREVER);
                server->clients[0].active             = 0;
                server->clients[0].handshake_complete = 0;
                tx_mutex_put(&server->clients_mutex);

                // Unaccept and relisten for next connection
                nx_tcp_server_socket_unaccept(&server->listen_socket);
                status = nx_tcp_server_socket_relisten(server->ip_instance,
                                                       WS_SERVER_PORT,
                                                       &server->listen_socket);
                if (status != NX_SUCCESS && status != NX_CONNECTION_PENDING) {
                    printf("WS: Relisten failed: 0x%X\r\n", status);
                }
            }
            else {
                printf("WS: Handshake failed: 0x%X\r\n", status);
                nx_tcp_socket_disconnect(&server->listen_socket, NX_NO_WAIT);
                nx_tcp_server_socket_unaccept(&server->listen_socket);
                nx_tcp_server_socket_relisten(server->ip_instance,
                                              WS_SERVER_PORT,
                                              &server->listen_socket);
            }
        }
        else if (status != NX_NO_PACKET) {
            printf("WS: Accept error: 0x%X\r\n", status);
            tx_thread_sleep(100);
        }
    }
}

UINT WS_Server_SendJSON(WS_Server_t *server, const char *json_string)
{
    NX_PACKET *packet_ptr;
    UINT status;
    uint32_t json_len = strlen(json_string);
    unsigned char frame_header[10];
    uint32_t header_len;

    if (!server->initialized) {
        return NX_NOT_CREATED;
    }

    // Build WebSocket TEXT frame header (not binary!)
    frame_header[0] = WS_FIN_FLAG | WS_OPCODE_TEXT; // TEXT frame

    if (json_len < 126) {
        frame_header[1] = (unsigned char)json_len;
        header_len      = 2;
    }
    else if (json_len < 65536) {
        frame_header[1] = 126;
        frame_header[2] = (json_len >> 8) & 0xFF;
        frame_header[3] = json_len & 0xFF;
        header_len      = 4;
    }
    else {
        return NX_NOT_SUCCESSFUL; // Frame too large
    }

    tx_mutex_get(&server->clients_mutex, TX_WAIT_FOREVER);

    if (server->clients[0].active && server->clients[0].handshake_complete) {
        NX_TCP_SOCKET *socket_ptr = &server->listen_socket;

        if (socket_ptr->nx_tcp_socket_state != NX_TCP_ESTABLISHED) {
            server->clients[0].active = 0;
            tx_mutex_put(&server->clients_mutex);
            return NX_NOT_CONNECTED;
        }

        // Allocate packet
        status = nx_packet_allocate(server->packet_pool, &packet_ptr,
                                    NX_TCP_PACKET, NX_NO_WAIT);
        if (status == NX_SUCCESS) {
            // Append frame header
            status = nx_packet_data_append(packet_ptr, frame_header, header_len,
                                           server->packet_pool, NX_NO_WAIT);
            if (status == NX_SUCCESS) {
                // Append JSON payload
                status = nx_packet_data_append(packet_ptr, (void *)json_string, json_len,
                                               server->packet_pool, NX_NO_WAIT);
                if (status == NX_SUCCESS) {
                    // Send
                    status = nx_tcp_socket_send(socket_ptr, packet_ptr, NX_NO_WAIT);
                    if (status != NX_SUCCESS) {
                        nx_packet_release(packet_ptr);
                    }
                }
                else {
                    nx_packet_release(packet_ptr);
                }
            }
            else {
                nx_packet_release(packet_ptr);
            }
        }
    }

    tx_mutex_put(&server->clients_mutex);
    return NX_SUCCESS;
}

static UINT ws_perform_handshake(NX_TCP_SOCKET *socket_ptr, NX_PACKET_POOL *pool_ptr)
{
    NX_PACKET *request_packet;
    NX_PACKET *response_packet;
    UINT status;
    char *msg;
    int len;
    char *key_ptr, *crlf_ptr, *upgrade_ptr;
    char client_key[32];
    char accept_key[32];
    char response_header[512];

    // Receive HTTP upgrade request
    status = nx_tcp_socket_receive(socket_ptr, &request_packet, 5 * NX_IP_PERIODIC_RATE);
    if (status != NX_SUCCESS) {
        return status;
    }

    msg = (char *)request_packet->nx_packet_prepend_ptr;
    len = request_packet->nx_packet_length;

    // Check for GET request
    if (ws_strnstr(msg, "GET ", len) == NULL) {
        nx_packet_release(request_packet);
        return NX_NOT_SUCCESSFUL;
    }

    // Check for Upgrade: websocket
    upgrade_ptr = ws_strnstr(msg, "Upgrade:", len);
    if (upgrade_ptr == NULL || ws_strnstr(upgrade_ptr, "websocket", 50) == NULL) {
        nx_packet_release(request_packet);
        return NX_NOT_SUCCESSFUL;
    }

    // Extract Sec-WebSocket-Key
    key_ptr = ws_strnstr(msg, "Sec-WebSocket-Key:", len);
    if (key_ptr == NULL) {
        nx_packet_release(request_packet);
        return NX_NOT_SUCCESSFUL;
    }

    key_ptr += 18;
    while (*key_ptr == ' ' || *key_ptr == '\t')
        key_ptr++;

    crlf_ptr = strchr(key_ptr, '\r');
    if (crlf_ptr == NULL || (crlf_ptr - key_ptr) > 30) {
        nx_packet_release(request_packet);
        return NX_NOT_SUCCESSFUL;
    }

    int key_len = (int)(crlf_ptr - key_ptr);
    strncpy(client_key, key_ptr, key_len);
    client_key[key_len] = '\0';

    nx_packet_release(request_packet);

    // Compute accept key
    ws_compute_accept_key(accept_key, client_key);

    // Build response
    snprintf(response_header, sizeof(response_header),
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n"
             "\r\n",
             accept_key);

    // Allocate response packet
    status = nx_packet_allocate(pool_ptr, &response_packet, NX_TCP_PACKET, TX_WAIT_FOREVER);
    if (status != NX_SUCCESS) {
        return status;
    }

    // Append response
    status = nx_packet_data_append(response_packet, response_header, strlen(response_header),
                                   pool_ptr, TX_WAIT_FOREVER);
    if (status != NX_SUCCESS) {
        nx_packet_release(response_packet);
        return status;
    }

    // Send response
    status = nx_tcp_socket_send(socket_ptr, response_packet, TX_WAIT_FOREVER);
    if (status != NX_SUCCESS) {
        nx_packet_release(response_packet);
        return status;
    }

    // Disable TCP timeout
    socket_ptr->nx_tcp_socket_timeout      = 0;
    socket_ptr->nx_tcp_socket_timeout_rate = 0;

    return NX_SUCCESS;
}

static UINT WS_ReceiveFrame(NX_TCP_SOCKET *socket_ptr, NX_PACKET_POOL *pool_ptr)
{
    NX_PACKET *packet_ptr;
    UINT status;
    unsigned char *data;
    uint32_t data_len;

    // Receive packet (non-blocking)
    status = nx_tcp_socket_receive(socket_ptr, &packet_ptr, NX_NO_WAIT);
    if (status != NX_SUCCESS) {
        return status; // No data available
    }

    data     = packet_ptr->nx_packet_prepend_ptr;
    data_len = packet_ptr->nx_packet_length;

    // Minimum WebSocket frame is 2 bytes
    if (data_len < 2) {
        nx_packet_release(packet_ptr);
        return NX_NOT_SUCCESSFUL;
    }

    // Parse WebSocket frame header
    unsigned char byte0  = data[0];
    unsigned char byte1  = data[1];

    unsigned char fin    = (byte0 & 0x80) != 0;
    unsigned char opcode = byte0 & 0x0F;
    unsigned char masked = (byte1 & 0x80) != 0;
    uint64_t payload_len = byte1 & 0x7F;

    uint32_t header_len  = 2;

    // Extended payload length
    if (payload_len == 126) {
        if (data_len < 4) {
            nx_packet_release(packet_ptr);
            return NX_NOT_SUCCESSFUL;
        }
        payload_len = (data[2] << 8) | data[3];
        header_len  = 4;
    }
    else if (payload_len == 127) {
        // We don't support >64KB frames
        nx_packet_release(packet_ptr);
        return NX_NOT_SUCCESSFUL;
    }

    // Masking key (always present from browser)
    unsigned char mask[4] = {0};
    if (masked) {
        if (data_len < header_len + 4) {
            nx_packet_release(packet_ptr);
            return NX_NOT_SUCCESSFUL;
        }
        memcpy(mask, &data[header_len], 4);
        header_len += 4;
    }

    // Check if we have complete payload
    if (data_len < header_len + payload_len) {
        nx_packet_release(packet_ptr);
        return NX_NOT_SUCCESSFUL;
    }

    // Handle different opcodes
    if (opcode == WS_OPCODE_TEXT || opcode == WS_OPCODE_BINARY) {
        // Unmask payload
        unsigned char *payload = &data[header_len];
        for (uint32_t i = 0; i < payload_len; i++) {
            payload[i] ^= mask[i % 4];
        }

        // Null-terminate for text frames
        if (opcode == WS_OPCODE_TEXT && payload_len < 1024) {
            char json_buffer[1024];
            memcpy(json_buffer, payload, payload_len);
            json_buffer[payload_len] = '\0';

            // Parse and handle JSON command
            WS_HandleJSONCommand(json_buffer);
        }
    }
    else if (opcode == WS_OPCODE_CLOSE) {
        nx_packet_release(packet_ptr);
        return NX_NOT_CONNECTED;
    }
    else if (opcode == WS_OPCODE_PING) {
        // TODO: Send pong response
    }

    nx_packet_release(packet_ptr);
    return NX_SUCCESS;
}

// ---------------------------------------------------------------------------------------
// SHA-1 and Base64 helper functions (same as before)
// ---------------------------------------------------------------------------------------

static uint32_t ws_rol(uint32_t value, uint32_t bits)
{
    return (value << bits) | (value >> (32 - bits));
}

static uint32_t ws_blk(uint32_t *block, uint32_t i)
{
    return ws_rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^
                      block[(i + 2) & 15] ^ block[i],
                  1);
}

static void ws_sha1(uint32_t *hash, uint32_t *block)
{
    uint32_t a[5] = {hash[4], hash[3], hash[2], hash[1], hash[0]};

    for (int i = 0; i < 16; ++i) {
        a[i % 5] += ((a[(3 + i) % 5] & (a[(2 + i) % 5] ^ a[(1 + i) % 5])) ^
                     a[(1 + i) % 5]) +
                    block[i] + 0x5a827999 + ws_rol(a[(4 + i) % 5], 5);
        a[(3 + i) % 5] = ws_rol(a[(3 + i) % 5], 30);
    }

    for (int i = 0; i < 4; ++i) {
        block[i] = ws_blk(block, i);
        a[(1 + i) % 5] += ((a[(4 + i) % 5] & (a[(3 + i) % 5] ^ a[(2 + i) % 5])) ^
                           a[(2 + i) % 5]) +
                          block[i] + 0x5a827999 + ws_rol(a[(5 + i) % 5], 5);
        a[(4 + i) % 5] = ws_rol(a[(4 + i) % 5], 30);
    }

    for (int i = 0; i < 20; ++i) {
        block[(i + 4) % 16] = ws_blk(block, (i + 4) % 16);
        a[i % 5] += (a[(3 + i) % 5] ^ a[(2 + i) % 5] ^ a[(1 + i) % 5]) +
                    block[(i + 4) % 16] + 0x6ed9eba1 + ws_rol(a[(4 + i) % 5], 5);
        a[(3 + i) % 5] = ws_rol(a[(3 + i) % 5], 30);
    }

    for (int i = 0; i < 20; ++i) {
        block[(i + 8) % 16] = ws_blk(block, (i + 8) % 16);
        a[i % 5] += (((a[(3 + i) % 5] | a[(2 + i) % 5]) & a[(1 + i) % 5]) |
                     (a[(3 + i) % 5] & a[(2 + i) % 5])) +
                    block[(i + 8) % 16] +
                    0x8f1bbcdc + ws_rol(a[(4 + i) % 5], 5);
        a[(3 + i) % 5] = ws_rol(a[(3 + i) % 5], 30);
    }

    for (int i = 0; i < 20; ++i) {
        block[(i + 12) % 16] = ws_blk(block, (i + 12) % 16);
        a[i % 5] += (a[(3 + i) % 5] ^ a[(2 + i) % 5] ^ a[(1 + i) % 5]) +
                    block[(i + 12) % 16] + 0xca62c1d6 + ws_rol(a[(4 + i) % 5], 5);
        a[(3 + i) % 5] = ws_rol(a[(3 + i) % 5], 30);
    }

    for (int i = 0; i < 5; ++i) {
        hash[i] += a[4 - i];
    }
}

static void ws_base64_encode(char *dst, unsigned char *src, int len)
{
    const char *b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for (int i = 0; i < 18; i += 3) {
        *dst++ = b64[(src[i] >> 2) & 63];
        *dst++ = b64[((src[i] & 3) << 4) | ((src[i + 1] & 240) >> 4)];
        *dst++ = b64[((src[i + 1] & 15) << 2) | ((src[i + 2] & 192) >> 6)];
        *dst++ = b64[src[i + 2] & 63];
    }

    *dst++ = b64[(src[18] >> 2) & 63];
    *dst++ = b64[((src[18] & 3) << 4) | ((src[19] & 240) >> 4)];
    *dst++ = b64[((src[19] & 15) << 2)];
    *dst++ = '=';
    *dst   = '\0';
}

static void ws_compute_accept_key(char *accept_key, const char *client_key)
{
    char key_guid[128];
    int key_len = strlen(client_key);

    char padded_key[25];
    strncpy(padded_key, client_key, 24);
    while (key_len < 24) {
        padded_key[key_len++] = '=';
    }
    padded_key[24] = '\0';

    snprintf(key_guid, sizeof(key_guid), "%s%s", padded_key, WS_GUID);

    uint32_t hash[5]   = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
    uint32_t block[16] = {0};

    for (int i = 0; i < 6; i++) {
        block[i] = (uint32_t)((padded_key[4 * i + 3] & 255) << 0 |
                              (padded_key[4 * i + 2] & 255) << 8 |
                              (padded_key[4 * i + 1] & 255) << 16 |
                              (padded_key[4 * i + 0] & 255) << 24);
    }

    block[6]  = 0x32353845;
    block[7]  = 0x41464135;
    block[8]  = 0x2d453931;
    block[9]  = 0x342d3437;
    block[10] = 0x44412d39;
    block[11] = 0x3543412d;
    block[12] = 0x43354142;
    block[13] = 0x30444338;
    block[14] = 0x35423131;
    block[15] = 0x80000000;

    ws_sha1(hash, block);

    uint32_t last_block[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 480};
    ws_sha1(hash, last_block);

    unsigned char hash_bytes[20];
    for (int i = 0; i < 5; i++) {
        hash_bytes[i * 4 + 0] = (hash[i] >> 24) & 0xFF;
        hash_bytes[i * 4 + 1] = (hash[i] >> 16) & 0xFF;
        hash_bytes[i * 4 + 2] = (hash[i] >> 8) & 0xFF;
        hash_bytes[i * 4 + 3] = (hash[i] >> 0) & 0xFF;
    }

    ws_base64_encode(accept_key, hash_bytes, 20);
}

static char *ws_strnstr(const char *haystack, const char *needle, size_t len)
{
    size_t needle_len = strlen(needle);
    if (needle_len == 0)
        return (char *)haystack;

    for (size_t i = 0; i <= len - needle_len; i++) {
        if (strncmp(&haystack[i], needle, needle_len) == 0) {
            return (char *)&haystack[i];
        }
    }
    return NULL;
}
