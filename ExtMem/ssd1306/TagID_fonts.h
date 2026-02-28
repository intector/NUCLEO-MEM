/**
  ****************************************************************************************
  * @file           : TagID_fonts.h
  * @brief          : #include "ssd1306_conf.h"
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

#ifndef __TAGID_FONTS_H__
#define __TAGID_FONTS_H__

#include <stddef.h>
#include <stdint.h>
#include <_ansi.h>

_BEGIN_STD_C

//#include "ssd1306_conf.h"
#include "ssd1306.h"

typedef uint16_t FONT_BITMAP;
typedef struct {
	uint8_t		CharWidth;         // Character width
	uint8_t		CharHeight;        // Character height
	char		StartChar;         // Start character
	char		EndChar;           // End character
	uint8_t		SpaceCharWidth;    // Width, in pixels, of space character
} FONT_INFO_T;

#ifdef SSD1306_INCLUDE_FONT_UBUNTU_MONO_8PT
extern const FONT_BITMAP ubuntuMono_8pt[];
extern const FONT_INFO_T ubuntuMono_8ptFontInfo;
#endif


_END_STD_C

#endif // __TAGID_FONTS_H__
