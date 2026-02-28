/**
  ****************************************************************************************
  * @file           : fonts.h
  * @brief          : #include "ssd1306.h"
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

#ifndef __FONTS_H__
#define __FONTS_H__

#include <stddef.h>
#include <stdint.h>
#include <_ansi.h>

_BEGIN_STD_C

//#include "ssd1306.h"

// Barcode -------------------------------------------------------------------------------
#define BC_CODE_128_A 0
#define BC_CODE_128_B 1
#define BC_CODE_128_C 2
#define BC_CTRL_CODE_START_128_A 0x20
#define BC_CTRL_CODE_START_128_B 0x21
#define BC_CTRL_CODE_START_128_C 0x22
#define BC_CTRL_CODE_STOP 0x23
#define BC_CTRL_CODE_REVERSE_STOP 0x24
#define BC_CTRL_CODE_STOP_PATTERN 0x25
    
typedef uint8_t BC_CTRL_CODE_BITMAP;
typedef uint8_t BC_CHAR_BITMAP;
typedef struct {
    const uint8_t width; // BC char width in pixels
    const uint8_t value; // BC char value
    const uint16_t offset; // start offset in font bitmap
} BC_CHAR_INFO;
typedef struct
{
    const char start_char; // start character
    const char end_char; // end character
    const BC_CHAR_BITMAP *const ctrl_code_bitmap; // pointer to ctrl code bitmap
    const BC_CHAR_INFO *const ctrl_code_info; // pointer to ctrl code info array
    const BC_CHAR_BITMAP *const char_bitmap; // pointer to bc character bitmap
    const BC_CHAR_INFO *const char_info; // pointer to bc character info array
} BC_INFO;

// Font ----------------------------------------------------------------------------------
typedef uint8_t FONT_CHAR_BITMAP;
typedef struct {
    const uint8_t width; // font width in pixels
    const uint8_t height; // font height in pixels
    const uint16_t offset; // start offset in font bitmap
} FONT_CHAR_INFO;
typedef struct
{
    const uint8_t height; // font height in pixels
    const char start_char; // start character
    const char end_char; // end character
    const FONT_CHAR_BITMAP *const char_bitmap; // pointer to font data bitmap
    const FONT_CHAR_INFO *const char_info; // pointer to font info array
} FONT_INFO;

// Include only used fonts ---------------------------------------------------------------

// barcode font: Code 128
#define BC_CODE_128_QUIET_ZONE 10
#define INCLUDE_FONT_BARCODE_CODE_128

// ubuntu fonts
#define INCLUDE_FONT_UBUNTU_MONO_8PT
#define INCLUDE_FONT_UBUNTU_MONO_10PT
#define INCLUDE_FONT_UBUNTU_MONO_12PT
#define INCLUDE_FONT_UBUNTU_MONO_14PT
#define INCLUDE_FONT_UBUNTU_MONO_16PT
#define INCLUDE_FONT_UBUNTU_MONO_18PT
#define INCLUDE_FONT_UBUNTU_MONO_20PT

#define INCLUDE_FONT_UBUNTU_COND_8PT
#define INCLUDE_FONT_UBUNTU_COND_10PT
#define INCLUDE_FONT_UBUNTU_COND_12PT
#define INCLUDE_FONT_UBUNTU_COND_14PT
#define INCLUDE_FONT_UBUNTU_COND_16PT
#define INCLUDE_FONT_UBUNTU_COND_18PT
#define INCLUDE_FONT_UBUNTU_COND_20PT

#define INCLUDE_FONT_UBUNTU_REG_8PT
#define INCLUDE_FONT_UBUNTU_REG_10PT
#define INCLUDE_FONT_UBUNTU_REG_12PT
#define INCLUDE_FONT_UBUNTU_REG_14PT
#define INCLUDE_FONT_UBUNTU_REG_16PT
#define INCLUDE_FONT_UBUNTU_REG_18PT
#define INCLUDE_FONT_UBUNTU_REG_20PT
#ifdef INCLUDE_FONT_BARCODE_CODE_128
extern const BC_INFO font_Barcode_Code_128;
#endif

#ifdef INCLUDE_FONT_UBUNTU_MONO_8PT
extern const FONT_INFO font_ubuntuMono_8pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_MONO_10PT
extern const FONT_INFO font_ubuntuMono_10pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_MONO_12PT
extern const FONT_INFO font_ubuntuMono_12pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_MONO_14PT
extern const FONT_INFO font_ubuntuMono_14pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_MONO_16PT
extern const FONT_INFO font_ubuntuMono_16pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_MONO_18PT
extern const FONT_INFO font_ubuntuMono_18pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_MONO_20PT
extern const FONT_INFO font_ubuntuMono_20pt;
#endif

#ifdef INCLUDE_FONT_UBUNTU_COND_8PT
extern const FONT_INFO font_ubuntuCondensed_8pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_COND_10PT
extern const FONT_INFO font_ubuntuCondensed_10pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_COND_12PT
extern const FONT_INFO font_ubuntuCondensed_12pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_COND_14PT
extern const FONT_INFO font_ubuntuCondensed_14pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_COND_16PT
extern const FONT_INFO font_ubuntuCondensed_16pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_COND_18PT
extern const FONT_INFO font_ubuntuCondensed_18pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_COND_20PT
extern const FONT_INFO font_ubuntuCondensed_20pt;
#endif

#ifdef INCLUDE_FONT_UBUNTU_REG_8PT
extern const FONT_INFO font_ubuntu_8pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_REG_10PT
extern const FONT_INFO font_ubuntu_10pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_REG_12PT
extern const FONT_INFO font_ubuntu_12pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_REG_14PT
extern const FONT_INFO font_ubuntu_14pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_REG_16PT
extern const FONT_INFO font_ubuntu_16pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_REG_18PT
extern const FONT_INFO font_ubuntu_18pt;
#endif
#ifdef INCLUDE_FONT_UBUNTU_REG_20PT
extern const FONT_INFO font_ubuntu_20pt;
#endif

_END_STD_C

#endif // __FONTS_H__
