/**
  ****************************************************************************************
 * @file           : octospi.h
 * @brief          : This file contains all the function prototypes for
 *                   the octospi.c file
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
#ifndef __OCTOSPI_H__
#define __OCTOSPI_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------------------
#include "main.h"

// OCTOSPI handle
extern OSPI_HandleTypeDef hospi1;

#define PSRAM_SIZE_16B   (0x00000000U) // !<  16 bits  (  2  Byte = 2^( 0+1))
#define PSRAM_SIZE_32B   (0x00000001U) // !<  32 bits  (  4  Byte = 2^( 1+1))
#define PSRAM_SIZE_64B   (0x00000002U) // !<  64 bits  (  8  Byte = 2^( 2+1))
#define PSRAM_SIZE_128B  (0x00000003U) // !< 128 bits  ( 16  Byte = 2^( 3+1))
#define PSRAM_SIZE_256B  (0x00000004U) // !< 256 bits  ( 32  Byte = 2^( 4+1))
#define PSRAM_SIZE_512B  (0x00000005U) // !< 512 bits  ( 64  Byte = 2^( 5+1))
#define PSRAM_SIZE_1KB   (0x00000006U) // !<   1 Kbits (128  Byte = 2^( 6+1))
#define PSRAM_SIZE_2KB   (0x00000007U) // !<   2 Kbits (256  Byte = 2^( 7+1))
#define PSRAM_SIZE_4KB   (0x00000008U) // !<   4 Kbits (512  Byte = 2^( 8+1))
#define PSRAM_SIZE_8KB   (0x00000009U) // !<   8 Kbits (  1 KByte = 2^( 9+1))
#define PSRAM_SIZE_16KB  (0x0000000AU) // !<  16 Kbits (  2 KByte = 2^(10+1))
#define PSRAM_SIZE_32KB  (0x0000000BU) // !<  32 Kbits (  4 KByte = 2^(11+1))
#define PSRAM_SIZE_64KB  (0x0000000CU) // !<  64 Kbits (  8 KByte = 2^(12+1))
#define PSRAM_SIZE_128KB (0x0000000DU) // !< 128 Kbits ( 16 KByte = 2^(13+1))
#define PSRAM_SIZE_256KB (0x0000000EU) // !< 256 Kbits ( 32 KByte = 2^(14+1))
#define PSRAM_SIZE_512KB (0x0000000FU) // !< 512 Kbits ( 64 KByte = 2^(15+1))
#define PSRAM_SIZE_1MB   (0x00000010U) // !<   1 Mbits (128 KByte = 2^(16+1))
#define PSRAM_SIZE_2MB   (0x00000011U) // !<   2 Mbits (256 KByte = 2^(17+1))
#define PSRAM_SIZE_4MB   (0x00000012U) // !<   4 Mbits (512 KByte = 2^(18+1))
#define PSRAM_SIZE_8MB   (0x00000013U) // !<   8 Mbits (  1 MByte = 2^(19+1))
#define PSRAM_SIZE_16MB  (0x00000014U) // !<  16 Mbits (  2 MByte = 2^(20+1))
#define PSRAM_SIZE_32MB  (0x00000015U) // !<  32 Mbits (  4 MByte = 2^(21+1))
#define PSRAM_SIZE_64MB  (0x00000016U) // !<  64 Mbits (  8 MByte = 2^(22+1))
#define PSRAM_SIZE_128MB (0x00000017U) // !< 128 Mbits ( 16 MByte = 2^(23+1))
#define PSRAM_SIZE_256MB (0x00000018U) // !< 256 Mbits ( 32 MByte = 2^(24+1))
#define PSRAM_SIZE_512MB (0x00000019U) // !< 512 Mbits ( 64 MByte = 2^(25+1))
#define PSRAM_SIZE_1GB   (0x0000001AU) // !<   1 Gbits (128 MByte = 2^(26+1))
#define PSRAM_SIZE_2GB   (0x0000001BU) // !<   2 Gbits (256 MByte = 2^(27+1))
#define PSRAM_SIZE_4GB   (0x0000001CU) // !<   4 Gbits (256 MByte = 2^(28+1))
#define PSRAM_SIZE_8GB   (0x0000001DU) // !<   8 Gbits (256 MByte = 2^(29+1))
#define PSRAM_SIZE_16GB  (0x0000001EU) // !<  16 Gbits (256 MByte = 2^(30+1))
#define PSRAM_SIZE_32GB  (0x0000001FU) // !<  32 Gbits (256 MByte = 2^(31+1))

// Session 7 test mode -------------------------------------------------------------------
// Set to 1 to bypass DLYB and run bus verification only.
// Set to 0 for normal operation with DLYB calibration.
#define PSRAM_BYPASS_DLYB_TEST  0

// Public functions ----------------------------------------------------------------------
void MX_OCTOSPI1_Init(void);


#ifdef __cplusplus
}
#endif

#endif // __OCTOSPI_H__
