/**
  ****************************************************************************************
  * @file           : llmnr_responder.c
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

// Includes ------------------------------------------------------------------------------
#include "llmnr_responder.h"
#include "main.h"
#include "app_netxduo.h"
#include <string.h>
#include <ctype.h>

// Private variables ---------------------------------------------------------------------
static NX_UDP_SOCKET  llmnr_socket;
static TX_THREAD      llmnr_thread;
static NX_IP          *llmnr_ip_ptr;
static NX_PACKET_POOL *llmnr_pool_ptr;
static VOID           *llmnr_stack_ptr;
static ULONG          llmnr_stack_size;
static char           llmnr_hostname[64];
static UCHAR          llmnr_hostname_len;

// DNS record type constants
#define DNS_TYPE_A      1
#define DNS_TYPE_ANY    255
#define DNS_CLASS_IN    1

// Private function prototypes -----------------------------------------------------------
static VOID llmnr_thread_entry(ULONG thread_input);
static UINT llmnr_parse_and_respond(NX_PACKET *packet_ptr);
static UINT llmnr_name_compare(const UCHAR *qname, UINT qname_len);

// Functions -----------------------------------------------------------------------------

UINT LLMNR_Responder_Init(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                           VOID *stack_ptr, ULONG stack_size, const char *hostname)
{
    UINT status;

    llmnr_ip_ptr     = ip_ptr;
    llmnr_pool_ptr   = pool_ptr;
    llmnr_stack_ptr  = stack_ptr;
    llmnr_stack_size = stack_size;

    // Store hostname (lowercase for case-insensitive comparison)
    llmnr_hostname_len = 0;
    while (hostname[llmnr_hostname_len] && llmnr_hostname_len < sizeof(llmnr_hostname) - 1)
    {
        llmnr_hostname[llmnr_hostname_len] = (char)tolower((unsigned char)hostname[llmnr_hostname_len]);
        llmnr_hostname_len++;
    }
    llmnr_hostname[llmnr_hostname_len] = '\0';

    // Create UDP socket
    status = nx_udp_socket_create(ip_ptr, &llmnr_socket, "LLMNR Socket",
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY, 255, 8);
    if (status != NX_SUCCESS)
        return status;

    // Create the LLMNR thread (don't start yet)
    status = tx_thread_create(&llmnr_thread, "LLMNR Thread",
                               llmnr_thread_entry, 0,
                               stack_ptr, stack_size,
                               NX_LLMNR_THREAD_PRIORITY, NX_LLMNR_THREAD_PRIORITY,
                               TX_NO_TIME_SLICE, TX_DONT_START);

    return status;
}

UINT LLMNR_Responder_Start(void)
{
    UINT status;

    // Bind to LLMNR port 5355
    status = nx_udp_socket_bind(&llmnr_socket, LLMNR_PORT, TX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
        return status;

    // Join LLMNR multicast group 224.0.0.252
    status = nx_igmp_multicast_join(llmnr_ip_ptr, LLMNR_MCAST_ADDR);
    if (status != NX_SUCCESS)
        return status;

    // Start the listener thread
    status = tx_thread_resume(&llmnr_thread);

    return status;
}

// Private functions ---------------------------------------------------------------------

static VOID llmnr_thread_entry(ULONG thread_input)
{
    UNUSED(thread_input);
    NX_PACKET *packet_ptr;
    UINT status;

    printf("LLMNR: responder started for '%s'\r\n", llmnr_hostname);

    while (1)
    {
        // Block waiting for incoming LLMNR queries
        status = nx_udp_socket_receive(&llmnr_socket, &packet_ptr, TX_WAIT_FOREVER);
        if (status == NX_SUCCESS)
        {
            llmnr_parse_and_respond(packet_ptr);
            nx_packet_release(packet_ptr);
        }
    }
}

/**
 * @brief  Case-insensitive comparison of DNS-encoded QNAME against our hostname.
 * @param  qname: pointer to the QNAME field in the DNS packet (label-encoded)
 * @param  qname_len: available bytes from qname to end of packet data
 * @retval NX_SUCCESS if matches, NX_NOT_SUCCESSFUL otherwise
 *
 * DNS label format: [len][chars...][len][chars...][0]
 * For a single-label name like "lab-web": [7]lab-web[0]
 */
static UINT llmnr_name_compare(const UCHAR *qname, UINT qname_len)
{
    if (qname_len < 2)
        return NX_NOT_SUCCESSFUL;

    // First byte is the label length
    UCHAR label_len = qname[0];

    // Must match our hostname length exactly
    if (label_len != llmnr_hostname_len)
        return NX_NOT_SUCCESSFUL;

    // Need at least label_len + 2 bytes (length byte + chars + terminator)
    if (qname_len < (UINT)(label_len + 2))
        return NX_NOT_SUCCESSFUL;

    // Must be terminated by a zero-length label (single-label LLMNR name)
    if (qname[label_len + 1] != 0)
        return NX_NOT_SUCCESSFUL;

    // Case-insensitive character comparison
    for (UCHAR i = 0; i < label_len; i++)
    {
        if (tolower(qname[1 + i]) != llmnr_hostname[i])
            return NX_NOT_SUCCESSFUL;
    }

    return NX_SUCCESS;
}

/**
 * @brief  Parse an incoming LLMNR query and send a response if the name matches.
 *
 * LLMNR packet format (DNS wire format):
 *   [0..1]   Transaction ID
 *   [2..3]   Flags          (query = 0x0000)
 *   [4..5]   QDCOUNT        (should be 1)
 *   [6..7]   ANCOUNT        (0 for query)
 *   [8..9]   NSCOUNT        (0)
 *   [10..11] ARCOUNT        (0)
 *   [12..]   Question section: QNAME (variable), QTYPE (2), QCLASS (2)
 */
static UINT llmnr_parse_and_respond(NX_PACKET *packet_ptr)
{
    UCHAR *data;
    ULONG data_len;
    ULONG src_ip;
    UINT  src_port;

    // Extract sender info
    nx_udp_source_extract(packet_ptr, &src_ip, &src_port);

    data     = packet_ptr->nx_packet_prepend_ptr;
    data_len = packet_ptr->nx_packet_length;

    // Minimum DNS header is 12 bytes + at least 1 byte QNAME + 4 bytes QTYPE/QCLASS
    if (data_len < 17)
        return NX_NOT_SUCCESSFUL;

    // Check flags: must be a standard query (QR=0, Opcode=0)
    USHORT flags = (USHORT)((data[2] << 8) | data[3]);
    if (flags & 0x8000)
        return NX_NOT_SUCCESSFUL; // This is a response, not a query

    // QDCOUNT must be 1
    USHORT qdcount = (USHORT)((data[4] << 8) | data[5]);
    if (qdcount != 1)
        return NX_NOT_SUCCESSFUL;

    // Parse QNAME starting at offset 12
    const UCHAR *qname     = &data[12];
    UINT         qname_max = data_len - 12 - 4; // leave room for QTYPE + QCLASS

    // Check if the name matches our hostname
    if (llmnr_name_compare(qname, qname_max) != NX_SUCCESS)
        return NX_NOT_SUCCESSFUL;

    // Calculate QNAME wire length: label_len + label + terminator
    UINT qname_wire_len = 1 + qname[0] + 1;

    // Extract QTYPE and QCLASS after QNAME
    UINT type_offset = 12 + qname_wire_len;
    if (type_offset + 4 > data_len)
        return NX_NOT_SUCCESSFUL;

    USHORT qtype  = (USHORT)((data[type_offset] << 8) | data[type_offset + 1]);
    USHORT qclass = (USHORT)((data[type_offset + 2] << 8) | data[type_offset + 3]);

    // Only respond to A (1) or ANY (255) queries, class IN (1)
    if ((qtype != DNS_TYPE_A && qtype != DNS_TYPE_ANY) || qclass != DNS_CLASS_IN)
        return NX_NOT_SUCCESSFUL;

    // Get our IP address
    ULONG our_ip;
    ULONG our_mask;
    nx_ip_address_get(llmnr_ip_ptr, &our_ip, &our_mask);

    // Build the response packet
    NX_PACKET *resp_pkt;
    UINT status = nx_packet_allocate(llmnr_pool_ptr, &resp_pkt, NX_UDP_PACKET, NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
        return status;

    // Response layout:
    // [Header: 12 bytes] [Question: qname_wire_len + 4] [Answer: 2 + 2 + 2 + 4 + 2 + 4 = 16]
    UINT question_len = qname_wire_len + 4;
    UINT resp_len = 12 + question_len + 16;

    UCHAR *resp = resp_pkt->nx_packet_prepend_ptr;

    // Transaction ID (echo from query)
    resp[0] = data[0];
    resp[1] = data[1];

    // Flags: QR=1 (response), AA=0, TC=0, RD=0, RA=0, Z=0, RCODE=0
    resp[2] = 0x80;
    resp[3] = 0x00;

    // QDCOUNT = 1
    resp[4] = 0x00;
    resp[5] = 0x01;

    // ANCOUNT = 1
    resp[6] = 0x00;
    resp[7] = 0x01;

    // NSCOUNT = 0
    resp[8] = 0x00;
    resp[9] = 0x00;

    // ARCOUNT = 0
    resp[10] = 0x00;
    resp[11] = 0x00;

    // Copy the question section from the query
    memcpy(&resp[12], &data[12], question_len);

    // Answer section
    UINT ans_offset = 12 + question_len;

    // NAME: pointer to QNAME at offset 12 (0xC00C)
    resp[ans_offset + 0] = 0xC0;
    resp[ans_offset + 1] = 0x0C;

    // TYPE: A (1)
    resp[ans_offset + 2] = 0x00;
    resp[ans_offset + 3] = 0x01;

    // CLASS: IN (1)
    resp[ans_offset + 4] = 0x00;
    resp[ans_offset + 5] = 0x01;

    // TTL: 30 seconds
    resp[ans_offset + 6]  = 0x00;
    resp[ans_offset + 7]  = 0x00;
    resp[ans_offset + 8]  = 0x00;
    resp[ans_offset + 9]  = LLMNR_TTL;

    // RDLENGTH: 4 (IPv4 address)
    resp[ans_offset + 10] = 0x00;
    resp[ans_offset + 11] = 0x04;

    // RDATA: IP address (network byte order — NetXDuo stores MSB first)
    resp[ans_offset + 12] = (UCHAR)((our_ip >> 24) & 0xFF);
    resp[ans_offset + 13] = (UCHAR)((our_ip >> 16) & 0xFF);
    resp[ans_offset + 14] = (UCHAR)((our_ip >> 8) & 0xFF);
    resp[ans_offset + 15] = (UCHAR)(our_ip & 0xFF);

    // Set packet length
    resp_pkt->nx_packet_length      = resp_len;
    resp_pkt->nx_packet_append_ptr  = resp_pkt->nx_packet_prepend_ptr + resp_len;

    // Send response back to the querier (unicast)
    status = nx_udp_socket_send(&llmnr_socket, resp_pkt, src_ip, src_port);
    if (status != NX_SUCCESS)
    {
        nx_packet_release(resp_pkt);
    }

    return status;
}
