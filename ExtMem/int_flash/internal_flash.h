/**
  ****************************************************************************************
  ****************************************************************************************
  ****************************************************************************************
  */

#ifndef INC_INTERNAL_FLASH_H_
#define INC_INTERNAL_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "main.h"

#define FLASH_WORD_SIZE_01_BYTE 32
#define FLASH_WORD_SIZE_08_BYTE 4

#define FLASH_BASE_ADDR         (uint32_t)(FLASH_BANK1_BASE)
#define FLASH_END_ADDR          (uint32_t)(FLASH_END)

#define FLASH_ERASE_NO          0x00
#define FLASH_ERASE_YES         0x01

// Base address of the Flash sectors Bank 1
#define ADDR_FLASH_SECTOR_0_BANK1 ((uint32_t)0x08000000) // Base @ of Sector 0, 128 Kbytes
#define ADDR_FLASH_SECTOR_1_BANK1 ((uint32_t)0x08020000) // Base @ of Sector 1, 128 Kbytes
#define ADDR_FLASH_SECTOR_2_BANK1 ((uint32_t)0x08040000) // Base @ of Sector 2, 128 Kbytes
#define ADDR_FLASH_SECTOR_3_BANK1 ((uint32_t)0x08060000) // Base @ of Sector 3, 128 Kbytes
#define ADDR_FLASH_SECTOR_4_BANK1 ((uint32_t)0x08080000) // Base @ of Sector 4, 128 Kbytes
#define ADDR_FLASH_SECTOR_5_BANK1 ((uint32_t)0x080A0000) // Base @ of Sector 5, 128 Kbytes
#define ADDR_FLASH_SECTOR_6_BANK1 ((uint32_t)0x080C0000) // Base @ of Sector 6, 128 Kbytes
#define ADDR_FLASH_SECTOR_7_BANK1 ((uint32_t)0x080E0000) // Base @ of Sector 7, 128 Kbytes

#define ERR_ADDR_OUT_OF_RANGE     0x01

typedef ULONG EraseSectorError;

EraseSectorError flash_erase(ULONG start_sector, ULONG number_of_sectors);
UINT flash_store(ULONG memory_address, UINT *data, UINT data_length, UINT FlashErase);
VOID flash_read(ULONG memory_address, UCHAR *data, UINT data_length);
ULONG GetSector(ULONG Address);

#ifdef __cplusplus
}
#endif

#endif // INC_INTERNAL_FLASH_H_
