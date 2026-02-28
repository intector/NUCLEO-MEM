/**
  ****************************************************************************************
  ****************************************************************************************
  ****************************************************************************************
  */

#ifndef INC_FW_SETTINGS_H_
#define INC_FW_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "main.h"
#include "internal_flash.h"
UINT Check_32Bytes_FlashData(ULONG StartAddress, ULONG Data);
extern UINT FW_InitSettings();

#define FWS_INIT_OK        ((UINT) 0x00)
#define FWS_DATA_FOUND     ((UINT) 0x01)
#define FWS_DLID_OK        ((UINT) 0x02)
	
#define FWS_WR_DLID_ERR    ((UINT) 0x81)
#define FWS_WR_NWD_ERR     ((UINT) 0x82)
#define FWS_DATA_NOT_FOUND ((UINT) 0x83)
#define FWS_BAD_ADdR_RANGE ((UINT) 0x84)
	

#define OOB_FW_MN01    (ULONG64)0xDEADBEEFDEADCAFE // 8 Byte magig number DEAD BEEF DEAD CAFE
#define OOB_FW_MN02    (ULONG64)0x1234567089ABCDEF // 8 Byte magig number 1234567 89ABCDEF
#define OOB_FW_FILL    (ULONG64)0xA5A5A5A5A5A5A5A5 // 8 Byte fill pattern repeating 0xA5
#define OOB_FW_DL_ID01 (ULONG64)0x2A0006E406E4002A // 8 Byte firmware download ID 42 1764 1764 42
#define OOB_FW_SPARE_1 (ULONG)0xA5A5A5A5           // 4 Byte fill pattern repeating 0xA5
#define OOB_FW_SPARE_2 (ULONG64)0xA5A5A5A5A5A5A5A5 // 8 Byte fill pattern repeating 0xA5
#define OOB_FW_SPARE_3 (ULONG64)0x0000000000000000 // 8 Byte fill pattern repeating 0x00

#define	OOB_IP_ADDR    IP_ADDR(192, 168, 0, 50)
#define	OOB_NW_MASK    IP_ADDR(255, 255, 255, 0)
#define	OOB_GW_ADDR    IP_ADDR(192, 168, 0, 1)
#define	OOB_HTTP_PORT  (ULONG)80
#define	OOB_FTP_PORT   (ULONG)21

// fix download ID if no firmware download was done
#define OOB_DL_ID (const ULONG64 []) { OOB_FW_MN01,    \
                                       OOB_FW_DL_ID01, \
                                       OOB_FW_FILL,    \
                                       OOB_FW_MN02     }

// OOB Network settings
#define OOB_NW_Settings (const ULONG64 []) { ((((ULONG64)OOB_IP_ADDR) << 32) | OOB_NW_MASK),     \
                                             ((((ULONG64)OOB_GW_ADDR) << 32) | OOB_HTTP_PORT),   \
                                             ((((ULONG64)OOB_FTP_PORT) << 32) | OOB_FW_SPARE_1), \
                                             OOB_FW_SPARE_2                                      }

#define OOB_IP_ADDR_DATA_OFFSET           (ULONG)0x04
#define OOB_NW_MASK_DATA_OFFSET           (ULONG)0x00
#define OOB_GW_ADDR_DATA_OFFSET           (ULONG)0x0C
#define OOB_HTTP_PORT_DATA_OFFSET         (ULONG)0x08
#define OOB_FTP_PORT_DATA_OFFSET          (ULONG)0x14

	
	
	
#ifdef __cplusplus
}
#endif

#endif // INC_FW_SETTINGS_H_
