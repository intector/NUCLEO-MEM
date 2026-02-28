/**
  ****************************************************************************************
  ****************************************************************************************
  ****************************************************************************************
  */
#include "stdint.h"
#include "stm32h7xx_it.h"
#include "fw_settings.h"

// ---------------------------------------------------------------------------------------
//
// Out-Of-Box settings and data
//
// location:   internal flash Sector 7
// address:    0x080E0000 - 0x080FFFFF
// total size: 128 KByte
//
// ---------------------------------------------------------------------------------------

// network settings stucture
ULONG64 __attribute__((section(".FWS_DownloadID"))) FWS_DownloadID[FLASH_WORD_SIZE_08_BYTE] = {
    OOB_FW_SPARE_3,
    OOB_FW_SPARE_3,
    OOB_FW_SPARE_3,
    OOB_FW_SPARE_3 };

// network settings stucture
ULONG64 __attribute__((section(".FWS_Network"))) FWS_Network[FLASH_WORD_SIZE_08_BYTE] = {
    ((((ULONG64)OOB_IP_ADDR) << 32) | OOB_NW_MASK),
    ((((ULONG64)OOB_GW_ADDR) << 32) | OOB_HTTP_PORT),
    ((((ULONG64)OOB_FTP_PORT) << 32) | OOB_FW_SPARE_1),
    OOB_FW_SPARE_2 };

//
// ---------------------------------------------------------------------------------------

UINT Check_32Bytes_FlashData(ULONG StartAddress, ULONG Data)
{
	ULONG tmpStartAddr = StartAddress;
	ULONG tmpData = Data;
	ULONG EndAddr = tmpStartAddr + 0x20;
	ULONG Index = 0;
	__IO uint64_t data64 = 0;
	
	while (tmpStartAddr < EndAddr)
	{
		data64 = *(uint64_t*)tmpStartAddr;
		__DSB();
		if (data64 != *(uint64_t*)tmpData)
		{
			return FWS_DATA_NOT_FOUND;
		}
		tmpStartAddr += 8;
		tmpData += 8;
	}
	return FWS_DATA_FOUND;
}

UINT FW_InitSettings()
{
	UINT ret;

	// if firmware was downloaded, write the Download ID to flash
	if (Check_32Bytes_FlashData((ULONG)FWS_DownloadID, (ULONG)OOB_DL_ID) != FWS_DATA_FOUND)
	{
		// writing Download ID
		ret = flash_store((ULONG)FWS_DownloadID, (UINT *)OOB_DL_ID, FLASH_WORD_SIZE_01_BYTE, FLASH_ERASE_YES);
		if (ret != HAL_OK)
		{
			return 0xFF;
		}
		if (Check_32Bytes_FlashData((ULONG)FWS_DownloadID, (ULONG)OOB_DL_ID) != FWS_DATA_FOUND)
		{
			return 0xFF;
		}

		// write network settings to flash
		ret = flash_store((ULONG)FWS_Network, (UINT *)OOB_NW_Settings, FLASH_WORD_SIZE_01_BYTE, FLASH_ERASE_NO);
		if (ret != HAL_OK)
		{
			return 0xFF;
		}
		if (Check_32Bytes_FlashData((ULONG)FWS_Network, (ULONG)OOB_NW_Settings) != FWS_DATA_FOUND)
		{
			return 0xFF;
		}
	}
	
	// flash data are OK, go ahaed and read the settings
	ULONG tmpNWS = (ULONG)OOB_NW_Settings;
	NetworkSettings.ip_addr = *(ULONG*)(tmpNWS + OOB_IP_ADDR_DATA_OFFSET);
	NetworkSettings.nw_mask = *(ULONG*)(tmpNWS + OOB_NW_MASK_DATA_OFFSET);
	NetworkSettings.gw_addr = *(ULONG*)(tmpNWS + OOB_GW_ADDR_DATA_OFFSET);
	NetworkSettings.http_port = *(ULONG*)(tmpNWS + OOB_HTTP_PORT_DATA_OFFSET);
	NetworkSettings.ftp_port = *(ULONG*)(tmpNWS + OOB_FTP_PORT_DATA_OFFSET);
		
	return FWS_INIT_OK;
}

