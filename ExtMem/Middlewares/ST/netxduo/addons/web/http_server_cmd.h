/**
 ******************************************************************************
 * @file           : http_server_cmd.h
 * @brief          : command definitions for the http server
 ******************************************************************************
 *
 * Copyright (c) 2024 Intector Inc.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

// Define to prevent recursive inclusion -------------------------------------
#ifndef __HTTP_SERVER_CMD_H
#define __HTTP_SERVER_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

// Includes ------------------------------------------------------------------

// defines ------------------------------------------------------------------

// ***** Enter Data Tab *****
#define CLEAR_LABEL_DATA_URL "/ClearLabelData"
#define SCAN_FOR_DISPLAY_URL "/ScanForDisplay"
#define CLEAR_LABEL_DISPLAY_URL "/ClearLabelDisplay"
#define SEND_LABEL_URL "/SendLabel"

// ***** DCU Settings Tab *****
#define WRITE_DCU_SETTINGS_URL "/WriteDCU_Settings"
#define READ_DCU_SETTINGS_URL "/ReadDCU_Settings"

// ***** IO-Link Settings Tab *****
#define IO_LINK_SETTINGS_URL "/GetIO_LinkSettings"

// ***** general commands *****
#define GET_LABEL_INFO_URL "/GetLabelInfo"
#define GET_LABEL_DATA_URL "/GetLabelData"
#define GET_LABEL_PICTURE_URL "/GetLabelPicture"

// ***** testing commands *****
#define TEST_FUNCTION_URL "/TestFunction"


#ifdef __cplusplus
}
#endif

#endif // __HTTP_SERVER_CMD_H
