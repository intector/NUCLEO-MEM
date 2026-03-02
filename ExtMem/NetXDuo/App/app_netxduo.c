/**
  ****************************************************************************************
  * @file           : app_netxduo.c
  * @brief          : NetXDuo applicative file
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
#include "app_netxduo.h"

#include "nx_stm32_eth_config.h"
#include "http_server_cmd.h"
#include "ws_server.h"

#if NX_MDNS_ENABLE
#include "nxd_mdns.h"
#endif

#if NX_LLMNR_ENABLE
#include "llmnr_responder.h"
#endif

// Define Threadx global data structures.

TX_THREAD NxAppThread;
TX_THREAD LedThread;
TX_THREAD EthLinkCheckThread;

TX_SEMAPHORE Semaphore;
TX_QUEUE  MsgQueueOne;

// WebSocket dedicated server
WS_Server_t WebSocket_Server;

// Define NetX global data structures.

NX_PACKET_POOL EthPacketPool;
NX_PACKET_POOL HTTP_ServerPacketPool;
NX_PACKET_POOL FTP_ServerPacketPool;
NX_PACKET_POOL WS_ServerPacketPool;
NX_IP IpInstance;

ULONG IpAddress;
ULONG NetMask;

// App memory pointer.
CHAR *pointer;

NX_WEB_HTTP_SERVER HTTP_Server;
NX_FTP_SERVER FTP_Server;

static uint8_t __attribute__((section(".dma_buffer"))) nx_http_server_pool[HTTP_SRV_PACKET_POOL_SIZE];

static uint8_t __attribute__((section(".Nx_FTP_ServerPoolSection"))) nx_ftp_server_pool[FTP_SRV_PACKET_POOL_SIZE];

static uint8_t __attribute__((section(".dma_buffer"))) nx_ws_server_pool[WS_SRV_PACKET_POOL_SIZE];

static uint8_t __attribute__((section(".dma_buffer"))) eth_packet_pool_buffer[NX_ETH_PACKET_POOL_SIZE];

#if NX_MDNS_ENABLE
NX_MDNS      mDNS_Instance;
static UCHAR mdns_local_cache[NX_MDNS_LOCAL_CACHE_SIZE];
// Peer cache required because NX_MDNS_DISABLE_CLIENT in nx_user.h is not
// visible to nxd_mdns.c (NX_INCLUDE_USER_DEFINE_FILE not in compiler flags).
// Provide a minimal buffer so the pointer check passes; never used.
static UCHAR mdns_peer_cache[512];
#endif

// Define FileX global data structures.
// the server reads the content from the uSD, a FX_MEDIA instance is required
// FX_MEDIA     SDMedia;
// ULONG        SD_Card_Free_Bytes = 0;
// Buffer for FileX FX_MEDIA sector cache. this should be 32-Bytes aligned to avoid cache maintenance issues
ALIGN_32BYTES(uint32_t DataBuffer[512]);

// Private function prototypes -----------------------------------------------------------

/// main NetXDuo thread entry
static VOID nx_app_thread_entry(ULONG thread_input);

/// ethernet link check thread entry
static VOID Eth_Link_Check_Thread_Entry(ULONG thread_input);

/// additional mime types for http server
static NX_WEB_HTTP_SERVER_MIME_MAP add_http_mime_maps[] =
{
        {"css",   "text/css"},
        {"svg",   "image/svg+xml"},
        {"png",   "image/png"},
        {"jpg",   "image/jpg"},
        {"js",    "application/javascript"},
        {"woff",  "font/woff"},
        {"woff2", "font/woff2"},
        {"ttf",   "font/ttf"}
};

/// HTTP-Server callback when a new request from a client is triggered
static UINT webserver_request_notify_callback(NX_WEB_HTTP_SERVER *server_ptr,
                                              UINT request_type,
                                              CHAR *resource,
                                              NX_PACKET *packet_ptr);

/// locale helper functions
UINT Get_jQuery_Parameter(NX_PACKET *packet_ptr);
/// FTP-Server login and logout callback
UINT FTP_ServerLogin(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr,
                     ULONG client_ip_address,
                     UINT client_port,
                     CHAR *name,
                     CHAR *password,
                     CHAR *extra_info);
UINT FTP_ServerLogout(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr,
                      ULONG client_ip_address,
                      UINT client_port,
                      CHAR *name,
                      CHAR *password,
                      CHAR *extra_info);

UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;

    // Initialize the NetXDuo system.
    nx_system_initialize();

    // Packet pool uses static buffer in AXI SRAM for ETH DMA access
    STM32_ERROR_CHECK(ERR_NX_PacketPoolCreate, nx_packet_pool_create(&EthPacketPool, "NetX Main Packet Pool", DEFAULT_PAYLOAD_SIZE, eth_packet_pool_buffer, NX_ETH_PACKET_POOL_SIZE));

    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, Nx_IP_INSTANCE_THREAD_SIZE, TX_NO_WAIT));

    STM32_ERROR_CHECK(ERR_NX_IpCreate, nx_ip_create(&IpInstance,
                                   "NetX Ip instance",
                                   NetworkSettings.ip_addr,
                                   NetworkSettings.nw_mask,
                                   &EthPacketPool,
                                   nx_stm32_eth_driver,
                                   pointer,
                                   Nx_IP_INSTANCE_THREAD_SIZE,
                                   NX_APP_INSTANCE_PRIORITY));
	STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **) &pointer, DEFAULT_ARP_CACHE_SIZE, TX_NO_WAIT));
	STM32_ERROR_CHECK(ERR_NX_ArpEnable, nx_arp_enable(&IpInstance, (VOID *)pointer, DEFAULT_ARP_CACHE_SIZE));

	STM32_ERROR_CHECK(ERR_NX_IcmpEnable, nx_icmp_enable(&IpInstance));

	STM32_ERROR_CHECK(ERR_NX_TcpEnable, nx_tcp_enable(&IpInstance));

	STM32_ERROR_CHECK(ERR_NX_UdpEnable, nx_udp_enable(&IpInstance));

#if NX_MDNS_ENABLE || NX_LLMNR_ENABLE
    // IGMP required for mDNS/LLMNR multicast
    nx_igmp_enable(&IpInstance);
#endif

    // ---------- set TAGID_SE_IP_ADDR_OK event flag ----------
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_IP_ADDR_OK, TX_OR));

#if NX_HTTP_SERVER_ENABLE
    // Create the server packet pool.
    STM32_ERROR_CHECK(ERR_NX_PacketPoolCreate, nx_packet_pool_create(&HTTP_ServerPacketPool, "HTTP Server Packet Pool", HTTP_SERVER_PACKET_SIZE, nx_http_server_pool, HTTP_SRV_PACKET_POOL_SIZE));
    // allocate the server stack.
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, HTTP_SRV_STACK_SIZE, TX_NO_WAIT));
    // Create the HTTP Server.
    STM32_ERROR_CHECK(ERR_NX_HTTP_ServerCreate, nx_web_http_server_create(&HTTP_Server,
                                                "WEB HTTP Server",
                                                &IpInstance,
                                                NetworkSettings.http_port,
                                                &mmc_disk,
                                                pointer,
                                                HTTP_SRV_STACK_SIZE,
                                                &HTTP_ServerPacketPool,
                                                NX_NULL,
                                                webserver_request_notify_callback));

#endif // NETX_HTTP_SERVER_ENABLE

#if NX_FTP_SERVER_ENABLE
    // Create the server packet pool.
    STM32_ERROR_CHECK(ERR_NX_PacketPoolCreate, nx_packet_pool_create(&FTP_ServerPacketPool, "FTP Server Packet Pool", FTP_SERVER_PACKET_SIZE, nx_ftp_server_pool, FTP_SRV_PACKET_POOL_SIZE));
    // Check server stack memory allocation.
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, FTP_SRV_STACK_SIZE, TX_NO_WAIT));
    // Create the FTP server.
    STM32_ERROR_CHECK(ERR_NX_FTP_ServerCreate, nx_ftp_server_create(&FTP_Server,
                                           "FTP Server Instance",
                                           &IpInstance,
                                           &mmc_disk,
                                           pointer,
                                           FTP_SRV_STACK_SIZE,
                                           &FTP_ServerPacketPool,
                                           FTP_ServerLogin,
                                           FTP_ServerLogout));

#endif // NETX_FTP_SERVER_ENABLE

#if NX_WS_SERVER_ENABLE

    // Create the WS-Server packet pool.
    STM32_ERROR_CHECK(ERR_NX_PacketPoolCreate, nx_packet_pool_create(&WS_ServerPacketPool, "WS Server Packet Pool", WS_SERVER_PACKET_SIZE, nx_ws_server_pool, WS_SRV_PACKET_POOL_SIZE));
    // allocate the WS-Server stack.
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, WS_SRV_STACK_SIZE, TX_NO_WAIT));
    // Create dedicated WebSocket server
    STM32_ERROR_CHECK(ERR_NX_WS_ServerCreate, WS_Server_Create(&WebSocket_Server,
                                       &IpInstance,
                                       &WS_ServerPacketPool,
                                       pointer));
#endif

#if NX_MDNS_ENABLE
    // Allocate mDNS thread stack from byte pool
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, NX_MDNS_STACK_SIZE, TX_NO_WAIT));
    // Create mDNS instance
    STM32_ERROR_CHECK(ERR_NX_MDNS_Create, nx_mdns_create(&mDNS_Instance,
                                       &IpInstance,
                                       &EthPacketPool,
                                       NX_APP_THREAD_PRIORITY + 1,
                                       pointer,
                                       NX_MDNS_STACK_SIZE,
                                       (UCHAR *)NX_HOSTNAME,
                                       mdns_local_cache,
                                       NX_MDNS_LOCAL_CACHE_SIZE,
                                       mdns_peer_cache,
                                       sizeof(mdns_peer_cache),
                                       NX_NULL));
#endif

#if NX_LLMNR_ENABLE
    // Allocate LLMNR thread stack from byte pool
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, NX_LLMNR_STACK_SIZE, TX_NO_WAIT));
    // Initialize LLMNR responder (creates socket and thread, does not start)
    STM32_ERROR_CHECK(ERR_NX_LLMNR_SocketCreate, LLMNR_Responder_Init(&IpInstance,
                                       &EthPacketPool,
                                       pointer,
                                       NX_LLMNR_STACK_SIZE,
                                       NX_HOSTNAME));
#endif

#if NX_THREAD_ENABLE

    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, NX_APP_THREAD_STACK_SIZE, TX_NO_WAIT));

    STM32_ERROR_CHECK(ERR_Tx_ThreadCreate, tx_thread_create(&NxAppThread,
                                       "NetXDuo App thread",
                                       nx_app_thread_entry,
                                       0,
                                       pointer,
                                       NX_APP_THREAD_STACK_SIZE,
                                       NX_APP_THREAD_PRIORITY,
                                       NX_APP_THREAD_PRIORITY,
                                       TX_NO_TIME_SLICE,
                                       TX_DONT_START));

#endif // NETX_THREAD_ENABLE

#if NX_LINK_CHECK_ENABLE
    // Allocate the memory for ethernet link check thread
    STM32_ERROR_CHECK(ERR_Tx_ByteAllocate, tx_byte_allocate(byte_pool, (VOID **)&pointer, 2 * DEFAULT_MEMORY_SIZE, TX_NO_WAIT));
    // create the Link thread
    STM32_ERROR_CHECK(ERR_Tx_ThreadCreate, tx_thread_create(&EthLinkCheckThread,
                                       "Ethernet Link Check Thread",
                                       Eth_Link_Check_Thread_Entry,
                                       0,
                                       pointer,
                                       2 * DEFAULT_MEMORY_SIZE,
                                       LINK_PRIORITY,
                                       LINK_PRIORITY,
                                       TX_NO_TIME_SLICE,
                                       TX_DONT_START));
#endif // NETX_LINK_CHECK_ENABLE

#if NX_THREAD_ENABLE

    tx_thread_resume(&NxAppThread);

    // ---------- set TAGID_SE_NX_INIT_OK event flag ----------
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_NX_INIT_OK, TX_OR));

#endif // NETX_THREAD_ENABLE

    return NX_SUCCESS;
}

#if NX_THREAD_ENABLE
static VOID nx_app_thread_entry(ULONG thread_input)
{
    UNUSED(thread_input); // avoid gcc/g++ warnings

    UINT status;
    ULONG _EventFlags = 0x00000000UL;
    // ULONG _EventFlags_ALL = 0xFFFFFFFFUL;
    ULONG tmp_actual_events = 0x00000000UL;

    // ---------- getting all actual event flags ----------
    //STM32_ERROR_CHECK(tx_event_flags_get(&TAGID_status_event_group, _EventFlags_ALL, TX_OR, &tmp_actual_events, TX_NO_WAIT));

    _EventFlags = TAGID_SE_tx_app_OLED_thread;
    _EventFlags |= TAGID_SE_App_ThreadX_Init_OK | TAGID_SE_tx_app_main_thread;
    _EventFlags |= TAGID_SE_MX_FileX_Init | TAGID_SE_EMMC_INIT_OK;
    // _EventFlags &= tmp_actual_events;
    
    // wait for netXduo relevant event flags
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsGet, tx_event_flags_get(&TAGID_status_event_group, _EventFlags, TX_AND, &tmp_actual_events, TX_WAIT_FOREVER));

#if NX_HTTP_SERVER_ENABLE
    // add mime types to HTTP Server.
    STM32_ERROR_CHECK(ERR_NX_HTTP_MimeMapsSet, nx_web_http_server_mime_maps_additional_set(&HTTP_Server, &add_http_mime_maps[0], 8));
    // Start the HTTP Server.
    STM32_ERROR_CHECK(ERR_NX_HTTP_ServerStart, nx_web_http_server_start(&HTTP_Server));

    // ---------- set TAGID_SE_HTTP_SERVER_OK event flag ----------
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_HTTP_SERVER_OK, TX_OR));

#endif // NETX_HTTP_SERVER_ENABLE

#if NX_FTP_SERVER_ENABLE
    // Start the FTP Server.
    STM32_ERROR_CHECK(ERR_NX_FTP_ServerStart, nx_ftp_server_start(&FTP_Server));

    // ---------- set TAGID_SE_FTP_SERVER_OK event flag ----------
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_FTP_SERVER_OK, TX_OR));

#endif // NETX_FTP_SERVER_ENABLE

#if NX_WS_SERVER_ENABLE
    // Start WebSocket server (begins listening on port 8080)
    STM32_ERROR_CHECK(ERR_NX_WS_ServerStart, WS_Server_Start(&WebSocket_Server));

    // Set WebSocket server ready flag
    STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_status_event_group, TAGID_SE_WS_SERVER_OK, TX_OR));

#endif         // WS_SERVER_ENABLE

#if NX_MDNS_ENABLE
    // Enable mDNS on interface 0
    STM32_ERROR_CHECK(ERR_NX_MDNS_Enable, nx_mdns_enable(&mDNS_Instance, 0));
    // Advertise HTTP service
    nx_mdns_service_add(&mDNS_Instance,
                        NX_NULL,
                        (UCHAR *)"_http._tcp",
                        NX_NULL, NX_NULL,
                        120, 0, 0, 80,
                        NX_TRUE, 0);
    printf("mDNS: hostname '%s.local' registered\r\n", NX_HOSTNAME);
#endif

#if NX_LLMNR_ENABLE
    // Start LLMNR responder (bind socket, join multicast, start thread)
    LLMNR_Responder_Start();
    printf("LLMNR: hostname '%s' registered\r\n", NX_HOSTNAME);
#endif

    // OLED queue message buffer
    char _OLED_message[TX_OLED_SINGLE_MSG_SIZE_BYTES] = {0};
    char _OLED_text[TX_OLED_SINGLE_MSG_SIZE_BYTES - 2] = {0};

    // OLED screen text Right-Top-Section
    memset(_OLED_message, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);
    _OLED_message[0] = TX_OLED_CMD_UPDATE_RTS;
    strcpy(_OLED_text, "ETHERNET");
    memcpy(&_OLED_message[1], _OLED_text, TX_OLED_SINGLE_MSG_SIZE_BYTES - 2);
    STM32_ERROR_CHECK(ERR_Tx_QueueSend, tx_queue_send(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));

    // OLED screen text Right-Middle-Section
    memset(_OLED_message, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);
    _OLED_message[0] = TX_OLED_CMD_UPDATE_RMS;
    strcpy(_OLED_text, "disconnected");
    memcpy(&_OLED_message[1], _OLED_text, TX_OLED_SINGLE_MSG_SIZE_BYTES - 2);
    STM32_ERROR_CHECK(ERR_Tx_QueueSend, tx_queue_send(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));

    // OLED screen text Bottom-Section
    memset(_OLED_message, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);
    _OLED_message[0] = TX_OLED_CMD_UPDATE_BLS;
    strcpy(_OLED_text, "IP info");
    memcpy(&_OLED_message[1], _OLED_text, TX_OLED_SINGLE_MSG_SIZE_BYTES - 2);
    STM32_ERROR_CHECK(ERR_Tx_QueueSend, tx_queue_send(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));

    tx_thread_resume(&EthLinkCheckThread);

}
#endif // NETX_THREAD_ENABLE

#if NX_LINK_CHECK_ENABLE
static VOID Eth_Link_Check_Thread_Entry(ULONG thread_input)
{
    UNUSED(thread_input); 
    // OLED queue message buffer
    char _OLED_message[TX_OLED_SINGLE_MSG_SIZE_BYTES] = { 0 };
    char _OLED_text[TX_OLED_SINGLE_MSG_SIZE_BYTES - 2] = { 0 };
    char _OLED_LinkOK_sent = 0x00;
    char _OLED_LinkNOK_sent = 0x00;
    ULONG actual_status;
    UINT status;

    while (1)
    {
        // Get Physical Link stackavailtus.
        status = nx_ip_interface_status_check(&IpInstance, 0, NX_IP_LINK_ENABLED, &actual_status, 10);

        if (status == NX_SUCCESS)
        {
            if (!_OLED_LinkOK_sent) {
                _OLED_LinkNOK_sent = 0x00;
                // show "connected" on OLED display
                memset(_OLED_message, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);
                _OLED_message[0] = TX_OLED_CMD_UPDATE_RMS;
                strcpy(_OLED_text, "> connected <");
                memcpy(&_OLED_message[1], _OLED_text, TX_OLED_SINGLE_MSG_SIZE_BYTES - 2);
                STM32_ERROR_CHECK(ERR_Tx_QueueSend, tx_queue_send(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));
                _OLED_LinkOK_sent = 0x01;
            }
        }
        else
        {
            if (!_OLED_LinkNOK_sent) {
                _OLED_LinkOK_sent = 0x00;

                // show "disconnected" on OLED display
                memset(_OLED_message, 0x00, TX_OLED_SINGLE_MSG_SIZE_BYTES);
                _OLED_message[0] = TX_OLED_CMD_UPDATE_RMS;
                strcpy(_OLED_text, "disconnected");
                memcpy(&_OLED_message[1], _OLED_text, TX_OLED_SINGLE_MSG_SIZE_BYTES - 2);
                STM32_ERROR_CHECK(ERR_Tx_QueueSend, tx_queue_send(&tx_OLED_msg_queue, _OLED_message, TX_WAIT_FOREVER));
                _OLED_LinkNOK_sent = 0x01;
            }
            // Send command to Enable Nx driver.
            nx_ip_driver_direct_command(&IpInstance, NX_LINK_ENABLE, &actual_status);
        }

        tx_thread_sleep(NX_ETH_CABLE_CONNECTION_CHECK_PERIOD);
    }
}
#endif // NETX_LINK_CHECK_ENABLE

#ifdef NX_HTTP_SERVER_ENABLE
UINT webserver_request_notify_callback(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{

    CHAR temp_string[30] = {'\0'};
    CHAR data[1024] = {'\0'};
    UINT string_length, Idx;
    NX_PACKET *resp_packet_ptr = NULL;
    UINT status;
    ULONG resumptions;
    ULONG suspensions;
    ULONG idle_returns;
    ULONG non_idle_returns;
    ULONG total_bytes_sent;
    ULONG total_bytes_received;
    ULONG connections;
    ULONG disconnections;
    ULONG server_thread_count;
    ULONG led_thread_count;
    CHAR *server_thread_name;
    CHAR *led_thread_name;

    ULONG _EventFlags = 0x00000000UL;
    ULONG tmp_actual_events = 0x00000000UL;

#if 0 
    // ====================================================================
    // CRITICAL WEBSOCKET HANDLING: Must come FIRST before any other checks
    // ====================================================================

    // Check if this is a WebSocket frame (post-upgrade data)
    unsigned char *msg = (unsigned char *)packet_ptr->nx_packet_prepend_ptr;
    int len            = packet_ptr->nx_packet_length;

    // WebSocket frame detection:
    // - First byte contains FIN bit (0x80) and opcode (0x0F mask)
    // - HTTP requests start with printable ASCII: "GET", "POST", etc (0x47, 0x50...)
    // - WebSocket frames: 0x81 (text), 0x82 (binary), 0x88 (close), 0x89 (ping), 0x8A (pong)

    if (len >= 2) {
        unsigned char byte0 = msg[0];
        unsigned char byte1 = msg[1];

        // Check if this looks like a WebSocket frame:
        // - FIN bit set (0x80) AND
        // - Valid opcode (0x01-0x02 for data, 0x08-0x0A for control) AND
        // - Second byte has mask bit (0x80) - all browser frames are masked

        if ((byte0 & 0x80) != 0 && (byte1 & 0x80) != 0) {
            unsigned char opcode = byte0 & 0x0F;
            if ((opcode >= 0x01 && opcode <= 0x02) || (opcode >= 0x08 && opcode <= 0x0A)) {
                // This is a WebSocket frame - route to WebSocket handler
                status = WS_HandleIncomingFrame(server_ptr, server_ptr->nx_web_http_server_current_session_ptr);
                return NX_WEB_HTTP_CALLBACK_COMPLETED;
            }
        }
    }

    // Check for WebSocket upgrade FIRST
    status = WS_CheckUpgradeRequest(server_ptr, packet_ptr, resp_packet_ptr);
    if (status == NX_WEB_HTTP_CALLBACK_COMPLETED) {
        return status;
    }
#endif
    
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // +++++ response to jQuery: "/ScanForDisplay" +++++
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (strcmp(resource, SCAN_FOR_DISPLAY_URL) == 0) {

        // ---------- set TAGID_SE_tx_app_NFCV_Scanner_thread event flag ----------
        STM32_ERROR_CHECK(ERR_Tx_EventFlagsSet, tx_event_flags_set(&TAGID_ctrl_event_group, TAGID_CE_NFCV_SCANNING_START, TX_OR));

        // ---------- wait for TAGID_CE_NFCV_SCANNING_START event flag ----------
        _EventFlags = TAGID_CE_NFCV_SCANNING_START;
        STM32_ERROR_CHECK(ERR_Tx_EventFlagsGet, tx_event_flags_get(&TAGID_status_event_group, _EventFlags, TX_OR, &tmp_actual_events, TX_WAIT_FOREVER));

        strcpy(data, "NFCV scanning active");
    }

    else {
        return NX_SUCCESS;
    }

    // Derive the client request type from the client request.
    nx_web_http_server_type_get(server_ptr, server_ptr->nx_web_http_server_request_resource, temp_string, &string_length);

    // sprintf(data, "%s", "got your request, but nothing is programmed so far...");

    // Null terminate the string.
    temp_string[string_length] = '\0';

    // Now build a response header with server status is OK and no additional header info.
    status = nx_web_http_server_callback_generate_response_header(server_ptr,
                                                                  &resp_packet_ptr,
                                                                  NX_WEB_HTTP_STATUS_OK,
                                                                  strlen(data),
                                                                  temp_string,
                                                                  NX_NULL);

    status = _nxe_packet_data_append(resp_packet_ptr, data, strlen(data), server_ptr->nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
    // Now send the packet!

    status = nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);
    if (status != NX_SUCCESS) {
        nx_packet_release(resp_packet_ptr);
        return status;
    }
    return (NX_WEB_HTTP_CALLBACK_COMPLETED);
}
#endif // NETX_HTTP_SERVER_ENABLE

UINT Get_jQuery_Parameter(NX_PACKET *packet_ptr)
{
    UINT _query_number = 0;
    UINT _max_query_size = 20;
    CHAR *_query_ptr = (CHAR *)calloc(_max_query_size, sizeof(CHAR));
    UINT _query_size = 0x00;
    UINT _ParameterValue = 0x0000;
    CHAR *tmpStrPointer;

    char *token;

    if (_nxe_web_http_server_query_get(packet_ptr, _query_number, _query_ptr, &_query_size, _max_query_size) != NX_SUCCESS) {
        _ParameterValue = 0xFFFF;
    }

    tmpStrPointer = strchr(_query_ptr, '=');

    if (tmpStrPointer) {
        _ParameterValue = strtol(tmpStrPointer + 1, NULL, 10);
    }
    return _ParameterValue;
}

#ifdef NX_FTP_SERVER_ENABLE
UINT FTP_ServerLogin(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr,
                     ULONG client_ip_address,
                     UINT client_port,
                     CHAR *name,
                     CHAR *password,
                     CHAR *extra_info)
{
    // Always return success
    return (NX_SUCCESS);
}

UINT FTP_ServerLogout(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr,
                      ULONG client_ip_address,
                      UINT client_port,
                      CHAR *name,
                      CHAR *password,
                      CHAR *extra_info)
{
    // Always return success
    return (NX_SUCCESS);
}
#endif // NETX_FTP_SERVER_ENABLE
