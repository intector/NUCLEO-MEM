/**
  ****************************************************************************************
  ****************************************************************************************
  ****************************************************************************************
  */
#include "internal_flash.h"
#include "stdint.h"
#include "stm32h7xx_it.h"

EraseSectorError flash_erase(ULONG start_sector, ULONG number_of_sectors)
{
    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    HAL_StatusTypeDef ret                  = HAL_OK;
    EraseSectorError SecEraseErr           = 0xFFFFFFFF;

    // Fill EraseInit structure
    EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Banks        = FLASH_BANK_1;
    EraseInitStruct.Sector       = start_sector;
    EraseInitStruct.NbSectors    = number_of_sectors;

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();
    // Disable instruction cache prior to internal cacheable memory update
    // SCB_DisableICache();

    ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SecEraseErr);

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();

    SCB_InvalidateDCache_by_Addr((uint32_t *)(ADDR_FLASH_SECTOR_0_BANK1 + start_sector * 0x20000), number_of_sectors * 0x20000);
    
    // Enable instruction cache prior to internal cacheable memory update
    // SCB_EnableICache();

    if (ret != HAL_OK) {
        // SecEraseErr will contain the faulty sector and then to know the code error on this sector,
        // user can call function 'HAL_FLASH_GetError()'
        return SecEraseErr;
    }

    return HAL_OK;
}

UINT flash_store(ULONG memory_address, UINT *data, UINT data_length, UINT FlashErase)
{
    // Check the parameters
    assert_param(data_length == FLASH_WORD_SIZE_01_BYTE);

    EraseSectorError SecEraseErr;
    ULONG FirstSector = 0, NbOfSectors = 0;

    if ((memory_address < ADDR_FLASH_SECTOR_7_BANK1) || (memory_address >= FLASH_END_ADDR)) {
        return ERR_ADDR_OUT_OF_RANGE;
    }

    // erase flash if required
    if (FlashErase == FLASH_ERASE_YES) {
        // Get the 1st sector to erase
        FirstSector = GetSector(memory_address);
        // Get the number of sector to erase from 1st sector
        NbOfSectors = GetSector(memory_address + data_length) - FirstSector + 1;

        SecEraseErr = flash_erase(FirstSector, NbOfSectors);

        if (SecEraseErr != HAL_OK) {
            return SecEraseErr;
        }
    }

    // write data to flash
    HAL_StatusTypeDef FlashWriteStatus = HAL_OK;

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();
    // Disable instruction cache prior to internal cacheable memory update
    // SCB_DisableICache();

    // using while loop, convey all data to the flash memory
    FlashWriteStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, memory_address, ((uint32_t)data));

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();

    // Enable instruction cache prior to internal cacheable memory update
    // SCB_EnableICache();

    // Invalidate cached flash data so subsequent reads see the new content
    SCB_InvalidateDCache_by_Addr((uint32_t *)memory_address, data_length);
    
    return FlashWriteStatus;
}

VOID flash_read(ULONG memory_address, UCHAR *data, UINT data_length)
{
    for (int i = 0; i < data_length; i++) {
        *(data + i) = (*(UCHAR *)(memory_address + i));
    }
}

ULONG GetSector(ULONG Address)
{
    ULONG sector = 0;

    if ((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1)) {
        sector = FLASH_SECTOR_0;
    }
    else if ((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1)) {
        sector = FLASH_SECTOR_1;
    }
    else if ((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1)) {
        sector = FLASH_SECTOR_2;
    }
    else if ((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1)) {
        sector = FLASH_SECTOR_3;
    }
    else if ((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1)) {
        sector = FLASH_SECTOR_4;
    }
    else if ((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1)) {
        sector = FLASH_SECTOR_5;
    }
    else if ((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1)) {
        sector = FLASH_SECTOR_6;
    }
    else if ((Address >= ADDR_FLASH_SECTOR_7_BANK1) && (Address < FLASH_END_ADDR)) {
        sector = FLASH_SECTOR_7;
    }
    else {
        sector = FLASH_SECTOR_7;
    }

    return sector;
}
