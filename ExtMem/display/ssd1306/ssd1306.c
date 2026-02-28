#include "ssd1306.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>  // For memcpy

#if defined(SSD1306_USE_I2C)

void ssd1306_Reset(void) {
    /* for I2C - do nothing */
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8_t byte) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}

// Send data
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}

#elif defined(SSD1306_USE_SPI)

void ssd1306_Reset(void) {
    // CS = High (not selected)
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET);

    // Reset the OLED
    HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8_t byte) {
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_RESET); // select OLED
    HAL_GPIO_WritePin(SSD1306_DC_Port, SSD1306_DC_Pin, GPIO_PIN_RESET); // command
    HAL_SPI_Transmit(&SSD1306_SPI_PORT, (uint8_t *) &byte, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
}

// Send data
void ssd1306_WriteData(uint8_t* buffer, size_t buff_size) {
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_RESET); // select OLED
    HAL_GPIO_WritePin(SSD1306_DC_Port, SSD1306_DC_Pin, GPIO_PIN_SET); // data
    HAL_SPI_Transmit(&SSD1306_SPI_PORT, buffer, buff_size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SSD1306_CS_Port, SSD1306_CS_Pin, GPIO_PIN_SET); // un-select OLED
}

#else
#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro"
#endif


// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

// Screen object
static SSD1306_t SSD1306;

/* Fills the Screenbuffer with values from a given buffer of a fixed length */
SSD1306_Error_t ssd1306_FillBuffer(uint8_t* buf, uint32_t len) {
    SSD1306_Error_t ret = SSD1306_ERR;
    if (len <= SSD1306_BUFFER_SIZE) {
        memcpy(SSD1306_Buffer,buf,len);
        ret = SSD1306_OK;
    }
    return ret;
}

/* Initialize the oled screen */
void ssd1306_Init(void) {
    // Reset OLED
    ssd1306_Reset();

    // Wait for the screen to boot
    HAL_Delay(100);

    // Init OLED
    ssd1306_SetDisplayOn(0); //display off

    ssd1306_WriteCommand(0x20); //Set Memory Addressing Mode
    ssd1306_WriteCommand(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
                                // 10b,Page Addressing Mode (RESET); 11b,Invalid

    ssd1306_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

#ifdef SSD1306_MIRROR_VERT
    ssd1306_WriteCommand(0xC0); // Mirror vertically
#else
    ssd1306_WriteCommand(0xC8); //Set COM Output Scan Direction
#endif

    ssd1306_WriteCommand(0x00); //---set low column address
    ssd1306_WriteCommand(0x10); //---set high column address

    ssd1306_WriteCommand(0x40); //--set start line address - CHECK

    ssd1306_SetContrast(0x3F);

#ifdef SSD1306_MIRROR_HORIZ
    ssd1306_WriteCommand(0xA0); // Mirror horizontally
#else
    ssd1306_WriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
    ssd1306_WriteCommand(0xA7); //--set inverse color
#else
    ssd1306_WriteCommand(0xA6); //--set normal color
#endif

// Set multiplex ratio.
#if (SSD1306_HEIGHT == 128)
    // Found in the Luma Python lib for SH1106.
    ssd1306_WriteCommand(0xFF);
#else
    ssd1306_WriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK
#endif

#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x1F); //
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x3F); //
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x3F); // Seems to work for 128px high displays too.
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1306_WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

    ssd1306_WriteCommand(0xD3); //-set display offset - CHECK
    ssd1306_WriteCommand(0x00); //-not offset

    ssd1306_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
    ssd1306_WriteCommand(0xF0); //--set divide ratio

    ssd1306_WriteCommand(0xD9); //--set pre-charge period
    ssd1306_WriteCommand(0x22); //

    ssd1306_WriteCommand(0xDA); //--set com pins hardware configuration - CHECK
#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x02);
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x12);
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    ssd1306_WriteCommand(0xDB); //--set vcomh
    ssd1306_WriteCommand(0x20); //0x20,0.77xVcc

    ssd1306_WriteCommand(0x8D); //--set DC-DC enable
    ssd1306_WriteCommand(0x14); //
    ssd1306_SetDisplayOn(1); //--turn on SSD1306 panel

    // Clear screen
    ssd1306_Fill(Black);
    //ssd1306_Fill(White);
    
    // Flush buffer to screen
    ssd1306_UpdateScreen();
    
    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;
    
    SSD1306.Initialized = 1;
}

/* Fill the whole screen with the given color */
void ssd1306_Fill(SSD1306_COLOR color) {
    memset(SSD1306_Buffer, (color == Black) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}

/* Write the screenbuffer with changed to the screen */
void ssd1306_UpdateScreen(void) {
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for(uint8_t i = 0; i < SSD1306_HEIGHT/8; i++) {
        ssd1306_WriteCommand(0xB0 + i); // Set the current RAM page address.
        ssd1306_WriteCommand(0x00 + SSD1306_X_OFFSET_LOWER);
        ssd1306_WriteCommand(0x10 + SSD1306_X_OFFSET_UPPER);
        ssd1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH*i],SSD1306_WIDTH);
    }
}

void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
    if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        // Don't write outside the buffer
        return;
    }
   
    // Draw in the right color
    if(color == White) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else { 
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

#pragma region // ***** BARCODE generation *****

#pragma region // ***** 'ssd1306_Get_BC_CheckSum' *****
char ssd1306_Get_BC_CheckSum(char* BC_String, BC_INFO BC_Font, uint8_t BC_C128_variant) {

    uint16_t tmpBC_CheckSum = 0;
    uint16_t Idx1 = 0x01;
    
    switch (BC_C128_variant) {
        case BC_CODE_128_A: tmpBC_CheckSum = 103; // value for start code A
                            break;
        case BC_CODE_128_B: tmpBC_CheckSum = 104; // value for start code B
                            break;
        case BC_CODE_128_C: tmpBC_CheckSum = 105; // value for start code C
                            break;
        default: tmpBC_CheckSum = 103; // value for start code A
    }
    
    
    while (*BC_String) {
        tmpBC_CheckSum += (*BC_String - 32) * Idx1;
        BC_String++;
        Idx1++;
    }

    tmpBC_CheckSum = (tmpBC_CheckSum % 103) + 32;

    return tmpBC_CheckSum;
}
#pragma endregion

#pragma region // ***** 'ssd1306_Write_BC_Char' *****
char ssd1306_Write_BC_Char(char BC_Char, bool CTRL, uint8_t height, uint8_t ModuleWidth, BC_INFO BC_Font, SSD1306_COLOR color) {
    
    uint8_t IdxY, IdxX, IdxBC, IdxMW, b;
    uint16_t char_Info_Idx = 0;
    uint16_t char_Bitmap_Idx = 0;
    uint16_t tmpCurr_X_Pos = 0;
    div_t char_width_key;
    uint8_t bytes_per_char = 0;
    uint8_t char_act_width = 0;
    
    // if control code
    if (CTRL) {
        // Check remaining space on current line
        if (SSD1306_WIDTH < (SSD1306.CurrentX + (BC_Font.ctrl_code_info[char_Info_Idx].width)) ||
            SSD1306_HEIGHT < (SSD1306.CurrentY + height)) {
            return 0xFF; // Not enough space on current line
        }
            // Check if character is valid
        if (BC_Char < BC_CTRL_CODE_START_128_A || BC_Char > BC_CTRL_CODE_STOP_PATTERN) {
            return 0xFF; // character not in bitmap
        }
        char_Info_Idx = BC_Char - BC_CTRL_CODE_START_128_A;
        char_Bitmap_Idx = BC_Font.ctrl_code_info[char_Info_Idx].offset;
        if (BC_Font.ctrl_code_info[char_Info_Idx].width < 8) {
            char_width_key = div(BC_Font.ctrl_code_info[char_Info_Idx].width, 8);
        }
        else {
            char_width_key = div(BC_Font.ctrl_code_info[char_Info_Idx].width - 1, 8);
        }
        bytes_per_char = (uint8_t)char_width_key.quot + 1;
    }
    else {
        // Check remaining space on current line
        if (SSD1306_WIDTH < (SSD1306.CurrentX + (BC_Font.char_info[char_Info_Idx].width)) ||
            SSD1306_HEIGHT < (SSD1306.CurrentY + height)) {
            return 0xFF; // Not enough space on current line
        }
        // Check if character is valid
        if (BC_Char < BC_Font.start_char || BC_Char > BC_Font.end_char) {
            return 0xFF; // character not in bitmap
        }
        char_Info_Idx = BC_Char - BC_Font.start_char;
        char_Bitmap_Idx = BC_Font.char_info[char_Info_Idx].offset;
        if (BC_Font.char_info[char_Info_Idx].width < 8) {
            char_width_key = div(BC_Font.char_info[char_Info_Idx].width, 8);
        }
        else {
            char_width_key = div(BC_Font.char_info[char_Info_Idx].width - 1, 8);
        }
        bytes_per_char = (uint8_t)char_width_key.quot + 1;
    }

    // Use the font to write
    for (IdxY = 0; IdxY < height; IdxY++) {
        for (IdxBC = 0; IdxBC < bytes_per_char; IdxBC++) {
            char_act_width = 8;
            if (char_width_key.rem > 0) {
                if (bytes_per_char == 1 || (IdxBC + 1) == bytes_per_char) {
                    char_act_width = (uint8_t)char_width_key.rem;
                }
            }
            if (CTRL) {
                b = BC_Font.ctrl_code_bitmap[char_Bitmap_Idx + IdxBC];
            }
            else {
                b = BC_Font.char_bitmap[char_Bitmap_Idx + IdxBC];
            }
            b = BC_Font.char_bitmap[char_Bitmap_Idx + IdxBC];
//            tmpCurr_X_Pos = SSD1306.CurrentX;
            for (IdxX = 0; IdxX < char_act_width; IdxX++) {
                if ((b << IdxX) & 0x80) {
//                    for (IdxMW = 0; IdxMW < ModuleWidth; IdxMW++) {
                    ssd1306_DrawPixel(SSD1306.CurrentX + IdxX + (IdxBC * 8), (SSD1306.CurrentY + IdxY), (SSD1306_COLOR) color);
//                    }
//                    tmpCurr_X_Pos += ModuleWidth;
                }
                else {
//                    for (IdxMW = 0; IdxMW < ModuleWidth; IdxMW++) {
                    ssd1306_DrawPixel(SSD1306.CurrentX + IdxX + (IdxBC * 8), (SSD1306.CurrentY + IdxY), (SSD1306_COLOR)!color);
//                    }
//                    tmpCurr_X_Pos += ModuleWidth;
                }
            }
        }
    }

    // The current space is now taken
    if (CTRL) {
        SSD1306.CurrentX += (BC_Font.ctrl_code_info[char_Info_Idx].width) + 1 ;
    }
    else {
        SSD1306.CurrentX += (BC_Font.char_info[char_Info_Idx].width) + 1;
    }

    // Return written char for validation
    return BC_Char;
}
#pragma endregion

#pragma region // ***** 'ssd1306_Write_BC_String' *****
char ssd1306_Write_BC_String(char* BC_String, uint8_t BC_C128_variant, uint8_t height, uint8_t ModuleWidth, BC_INFO BC_Font, SSD1306_COLOR color) {
    char tmpBC_String = *BC_String;
    char* tmpBC_String_Safe = BC_String;
    char tmpBC_CheckSum = 0x00;

    switch (BC_C128_variant) {
        case BC_CODE_128_A: tmpBC_String = (char)BC_CTRL_CODE_START_128_A;
                            break;
        case BC_CODE_128_B: tmpBC_String = (char)BC_CTRL_CODE_START_128_B;
                            break;
        case BC_CODE_128_C: tmpBC_String = (char)BC_CTRL_CODE_START_128_C;
                            break;
        default: tmpBC_String = (char)BC_CTRL_CODE_START_128_A;
    }

    if (ssd1306_Write_BC_Char(tmpBC_String, true, height, ModuleWidth, BC_Font, color) != tmpBC_String) {
        return tmpBC_String; // Char could not be written
    }

    while (*tmpBC_String_Safe) {
        if (ssd1306_Write_BC_Char(*tmpBC_String_Safe, false, height, ModuleWidth, BC_Font, color) != *tmpBC_String_Safe) {
            return *tmpBC_String_Safe; // Char could not be written
        }
        tmpBC_String_Safe++;
    }
    
    // calculate checksum and add to BC
    tmpBC_CheckSum = ssd1306_Get_BC_CheckSum(BC_String, BC_Font, BC_C128_variant);
    if (ssd1306_Write_BC_Char(tmpBC_CheckSum, false, height, ModuleWidth, BC_Font, color) != tmpBC_CheckSum) {
        return tmpBC_CheckSum; // Char could not be written
    }

    // add stop code to BC
    tmpBC_String = (char)BC_CTRL_CODE_STOP_PATTERN;
    if (ssd1306_Write_BC_Char(tmpBC_String, true, height, ModuleWidth, BC_Font, color) != tmpBC_String) {
        return tmpBC_String; // Char could not be written
    }
    
    return 0x00;
}
#pragma endregion

#pragma endregion

#pragma region // ***** TEXT generation *****

#pragma region // ***** new 'ssd1306_WriteChar' *****
char ssd1306_WriteChar(char ch, FONT_INFO Font, SSD1306_COLOR color) {
    uint8_t IdxY, IdxX, IdxBC, b;
    uint16_t char_Info_Idx = 0;
    uint16_t char_Bitmap_Idx = 0;
    div_t char_width_key;
    uint8_t bytes_per_char = 0;
    uint8_t char_act_width = 0;
    
    // Check if character is valid
    if (ch < Font.start_char || ch > Font.end_char) 
    {
        // character not in bitmap
        return 0;
    }

    char_Info_Idx = ch - Font.start_char;
    char_Bitmap_Idx = Font.char_info[char_Info_Idx].offset;

/*
    if (Font.char_info[char_Info_Idx].width < 8)
    {
        char_width_key = div(Font.char_info[char_Info_Idx].width, 8);
    }
    else
    {
        char_width_key = div(Font.char_info[char_Info_Idx].width - 1, 8);
    }
*/
    
    char_width_key = div(Font.char_info[char_Info_Idx].width, 8);
    bytes_per_char = (uint8_t)char_width_key.quot;
    if (char_width_key.rem > 0) {
        bytes_per_char = (uint8_t)char_width_key.quot + 1;
    }

    // Check remaining space on current line
    if (SSD1306_WIDTH < (SSD1306.CurrentX + Font.char_info[char_Info_Idx].width) ||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.char_info[char_Info_Idx].height))
    {
        // Not enough space on current line
        return 0;
    }

    // Use the font to write
    for (IdxY = 0; IdxY < Font.height; IdxY++) {
        for (IdxBC = 0; IdxBC < bytes_per_char; IdxBC++) {
            char_act_width = 8;
            if (char_width_key.rem > 0)
            {
                if (bytes_per_char == 1 || (IdxBC + 1) == bytes_per_char)
                {
                    char_act_width = (uint8_t)char_width_key.rem;
                }
            }
            b = Font.char_bitmap[char_Bitmap_Idx + ((IdxY * bytes_per_char) + IdxBC)];
            for (IdxX = 0; IdxX < char_act_width; IdxX++) {
                if ((b << IdxX) & 0x80) {
                    ssd1306_DrawPixel(SSD1306.CurrentX + IdxX + (IdxBC * 8), (SSD1306.CurrentY + IdxY), (SSD1306_COLOR) color) ;
                }
                else {
                    ssd1306_DrawPixel(SSD1306.CurrentX + IdxX + (IdxBC * 8), (SSD1306.CurrentY + IdxY), (SSD1306_COLOR)!color);
                }
            }
        }
        // drawing one pixel gap to next character
        if (SSD1306_WIDTH < (SSD1306.CurrentX + Font.char_info[char_Info_Idx].width))
        {
//            ssd1306_DrawPixel(SSD1306.CurrentX + Font.char_info[char_Info_Idx].width + 1, SSD1306.CurrentY + IdxY, (SSD1306_COLOR)!color);
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.char_info[char_Info_Idx].width + 1;

    // Return written char for validation
    return ch;
}
#pragma endregion
#pragma region // ***** new 'ssd1306_WriteString' *****
char ssd1306_WriteString(char* str, FONT_INFO Font, SSD1306_COLOR color) {
    while (*str) {
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }
        str++;
    }
    
    // Everything ok
    return *str;
}

char ssd1306_WriteString_Aligned(V_RECT rect, enum H_ALIGNMENT align, char* str, FONT_INFO Font, SSD1306_COLOR color) {

    char* _str = str;
    int _Pixel_CNT = 0;
    
    while (*_str) {
        _Pixel_CNT += Font.char_info[*_str - Font.start_char].width + 1;
        _str++;
    }
    _Pixel_CNT -= 2;
    
    if (_Pixel_CNT > rect.End.X - rect.Start.X) {
        _Pixel_CNT = rect.End.X;
    }

    if (align == LEFT) {
        ssd1306_SetCursor(rect.Start.X, rect.Start.Y);
    }

    if (align == CENTER) {
        ssd1306_SetCursor(rect.Start.X + (((rect.End.X - rect.Start.X) / 2) - (_Pixel_CNT / 2)), rect.Start.Y);
    }

    if (align == RIGHT) {
        ssd1306_SetCursor(rect.End.X - _Pixel_CNT, rect.Start.Y);
    }

    
    while (*str) {
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }
        str++;
    }
    
    // Everything ok
    return *str;
}


#pragma endregion

#pragma endregion

#pragma region // ***** original TEXT generation *****
#pragma region // ***** original 'ssd1306_WriteChar' *****
/*
char ssd1306_WriteChar(char ch, SSD1306_Font_t Font, SSD1306_COLOR color) {
    uint32_t i, b, j;
    
    // Check if character is valid
    if (ch < 32 || ch > 126)
        return 0;
    
    // Check remaining space on current line
    if (SSD1306_WIDTH < (SSD1306.CurrentX + Font.width) ||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.height))
    {
        // Not enough space on current line
        return 0;
    }
    
    // Use the font to write
    for(i = 0; i < Font.height; i++) {
        b = Font.data[(ch - 32) * Font.height + i];
        for(j = 0; j < Font.width; j++) {
            if((b << j) & 0x8000)  {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            } else {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }
    
    // The current space is now taken
    SSD1306.CurrentX += Font.char_width ? Font.char_width[ch - 32] : Font.width;
    
    // Return written char for validation
    return ch;
}
*/
#pragma endregion
#pragma region // ***** original 'ssd1306_WriteString' *****
/* Write full string to screenbuffer */
/*
char ssd1306_WriteString(char* str, SSD1306_Font_t Font, SSD1306_COLOR color) {
    while (*str) {
        if (ssd1306_WriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }
        str++;
    }
    
    // Everything ok
    return *str;
}
*/
#pragma endregion
#pragma endregion

/* Position the cursor */
void ssd1306_SetCursor(uint8_t x, uint8_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

/* Draw line by Bresenhem's algorithm */
void ssd1306_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {
    int32_t deltaX = abs(x2 - x1);
    int32_t deltaY = abs(y2 - y1);
    int32_t signX = ((x1 < x2) ? 1 : -1);
    int32_t signY = ((y1 < y2) ? 1 : -1);
    int32_t error = deltaX - deltaY;
    int32_t error2;
    
    ssd1306_DrawPixel(x2, y2, color);

    while((x1 != x2) || (y1 != y2)) {
        ssd1306_DrawPixel(x1, y1, color);
        error2 = error * 2;
        if(error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        
        if(error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
    return;
}

/* Draw polyline */
void ssd1306_Polyline(const SSD1306_VERTEX *par_vertex, uint16_t par_size, SSD1306_COLOR color) {
    uint16_t i;
    if(par_vertex == NULL) {
        return;
    }

    for(i = 1; i < par_size; i++) {
        ssd1306_Line(par_vertex[i - 1].X, par_vertex[i - 1].Y, par_vertex[i].X, par_vertex[i].Y, color);
    }

    return;
}

/* Convert Degrees to Radians */
static float ssd1306_DegToRad(float par_deg) {
    return par_deg * (3.14f / 180.0f);
}

/* Normalize degree to [0;360] */
static uint16_t ssd1306_NormalizeTo0_360(uint16_t par_deg) {
    uint16_t loc_angle;
    if(par_deg <= 360) {
        loc_angle = par_deg;
    } else {
        loc_angle = par_deg % 360;
        loc_angle = (loc_angle ? loc_angle : 360);
    }
    return loc_angle;
}

/*
 * DrawArc. Draw angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle in degree
 * sweep in degree
 */
void ssd1306_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1306_COLOR color) {
    static const uint8_t CIRCLE_APPROXIMATION_SEGMENTS = 36;
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1,xp2;
    uint8_t yp1,yp2;
    uint32_t count;
    uint32_t loc_sweep;
    float rad;
    
    loc_sweep = ssd1306_NormalizeTo0_360(sweep);
    
    count = (ssd1306_NormalizeTo0_360(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;
    while(count < approx_segments)
    {
        rad = ssd1306_DegToRad(count*approx_degree);
        xp1 = x + (int8_t)(sinf(rad)*radius);
        yp1 = y + (int8_t)(cosf(rad)*radius);    
        count++;
        if(count != approx_segments) {
            rad = ssd1306_DegToRad(count*approx_degree);
        } else {
            rad = ssd1306_DegToRad(loc_sweep);
        }
        xp2 = x + (int8_t)(sinf(rad)*radius);
        yp2 = y + (int8_t)(cosf(rad)*radius);    
        ssd1306_Line(xp1,yp1,xp2,yp2,color);
    }
    
    return;
}

/*
 * Draw arc with radius line
 * Angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle: start angle in degree
 * sweep: finish angle in degree
 */
void ssd1306_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1306_COLOR color) {
    const uint32_t CIRCLE_APPROXIMATION_SEGMENTS = 36;
    float approx_degree;
    uint32_t approx_segments;
    uint8_t xp1;
    uint8_t xp2 = 0;
    uint8_t yp1;
    uint8_t yp2 = 0;
    uint32_t count;
    uint32_t loc_sweep;
    float rad;
    
    loc_sweep = ssd1306_NormalizeTo0_360(sweep);
    
    count = (ssd1306_NormalizeTo0_360(start_angle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_segments = (loc_sweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    approx_degree = loc_sweep / (float)approx_segments;

    rad = ssd1306_DegToRad(count*approx_degree);
    uint8_t first_point_x = x + (int8_t)(sinf(rad)*radius);
    uint8_t first_point_y = y + (int8_t)(cosf(rad)*radius);   
    while (count < approx_segments) {
        rad = ssd1306_DegToRad(count*approx_degree);
        xp1 = x + (int8_t)(sinf(rad)*radius);
        yp1 = y + (int8_t)(cosf(rad)*radius);    
        count++;
        if (count != approx_segments) {
            rad = ssd1306_DegToRad(count*approx_degree);
        } else {
            rad = ssd1306_DegToRad(loc_sweep);
        }
        xp2 = x + (int8_t)(sinf(rad)*radius);
        yp2 = y + (int8_t)(cosf(rad)*radius);    
        ssd1306_Line(xp1,yp1,xp2,yp2,color);
    }
    
    // Radius line
    ssd1306_Line(x,y,first_point_x,first_point_y,color);
    ssd1306_Line(x,y,xp2,yp2,color);
    return;
}

/* Draw circle by Bresenhem's algorithm */
void ssd1306_DrawCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r,SSD1306_COLOR par_color) {
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
        return;
    }

    do {
        ssd1306_DrawPixel(par_x - x, par_y + y, par_color);
        ssd1306_DrawPixel(par_x + x, par_y + y, par_color);
        ssd1306_DrawPixel(par_x + x, par_y - y, par_color);
        ssd1306_DrawPixel(par_x - x, par_y - y, par_color);
        e2 = err;

        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if(-x == y && e2 <= x) {
                e2 = 0;
            }
        }

        if (e2 > x) {
            x++;
            err = err + (x * 2 + 1);
        }
    } while (x <= 0);

    return;
}

/* Draw filled circle. Pixel positions calculated using Bresenham's algorithm */
void ssd1306_FillCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r,SSD1306_COLOR par_color) {
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
        return;
    }

    do {
        for (uint8_t _y = (par_y + y); _y >= (par_y - y); _y--) {
            for (uint8_t _x = (par_x - x); _x >= (par_x + x); _x--) {
                ssd1306_DrawPixel(_x, _y, par_color);
            }
        }

        e2 = err;
        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if (-x == y && e2 <= x) {
                e2 = 0;
            }
        }

        if (e2 > x) {
            x++;
            err = err + (x * 2 + 1);
        }
    } while (x <= 0);

    return;
}

/* Draw a rectangle */
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {
    ssd1306_Line(x1,y1,x2,y1,color);
    ssd1306_Line(x2,y1,x2,y2,color);
    ssd1306_Line(x2,y2,x1,y2,color);
    ssd1306_Line(x1,y2,x1,y1,color);

    return;
}

/* Draw a filled rectangle */
void ssd1306_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color) {
    uint8_t x_start = ((x1<=x2) ? x1 : x2);
    uint8_t x_end   = ((x1<=x2) ? x2 : x1);
    uint8_t y_start = ((y1<=y2) ? y1 : y2);
    uint8_t y_end   = ((y1<=y2) ? y2 : y1);

    for (uint8_t y= y_start; (y<= y_end)&&(y<SSD1306_HEIGHT); y++) {
        for (uint8_t x= x_start; (x<= x_end)&&(x<SSD1306_WIDTH); x++) {
            ssd1306_DrawPixel(x, y, color);
        }
    }
    return;
}

void ssd1306_FillRect_B(V_RECT rect, SSD1306_COLOR color) {

    uint8_t x_start = ((rect.Start.X <= rect.End.X) ? rect.Start.X : rect.End.X);
    uint8_t x_end   = ((rect.Start.X <= rect.End.X) ? rect.End.X : rect.Start.X);
    uint8_t y_start = ((rect.Start.Y <= rect.End.Y) ? rect.Start.Y : rect.End.Y);
    uint8_t y_end   = ((rect.Start.Y <= rect.End.Y) ? rect.End.Y : rect.Start.Y);

    for (uint8_t y = y_start; (y <= y_end)&&(y < SSD1306_HEIGHT); y++) {
        for (uint8_t x = x_start; (x <= x_end)&&(x < SSD1306_WIDTH); x++) {
            ssd1306_DrawPixel(x, y, color);
        }
    }
    return;
}

SSD1306_Error_t ssd1306_InvertRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  if ((x2 >= SSD1306_WIDTH) || (y2 >= SSD1306_HEIGHT)) {
    return SSD1306_ERR;
  }
  if ((x1 > x2) || (y1 > y2)) {
    return SSD1306_ERR;
  }
  uint32_t i;
  if ((y1 / 8) != (y2 / 8)) {
    /* if rectangle doesn't lie on one 8px row */
    for (uint32_t x = x1; x <= x2; x++) {
      i = x + (y1 / 8) * SSD1306_WIDTH;
      SSD1306_Buffer[i] ^= 0xFF << (y1 % 8);
      i += SSD1306_WIDTH;
      for (; i < x + (y2 / 8) * SSD1306_WIDTH; i += SSD1306_WIDTH) {
        SSD1306_Buffer[i] ^= 0xFF;
      }
      SSD1306_Buffer[i] ^= 0xFF >> (7 - (y2 % 8));
    }
  } else {
    /* if rectangle lies on one 8px row */
    const uint8_t mask = (0xFF << (y1 % 8)) & (0xFF >> (7 - (y2 % 8)));
    for (i = x1 + (y1 / 8) * SSD1306_WIDTH;
         i <= (uint32_t)x2 + (y2 / 8) * SSD1306_WIDTH; i++) {
      SSD1306_Buffer[i] ^= mask;
    }
  }
  return SSD1306_OK;
}

/* Draw a bitmap */
void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h, SSD1306_COLOR color) {
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    for (uint8_t j = 0; j < h; j++, y++) {
        for (uint8_t i = 0; i < w; i++) {
            if (i & 7) {
                byte <<= 1;
            } else {
                byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }

            if (byte & 0x80) {
                ssd1306_DrawPixel(x + i, y, color);
            }
        }
    }
    return;
}

void ssd1306_SetContrast(const uint8_t value) {
    const uint8_t kSetContrastControlRegister = 0x81;
    ssd1306_WriteCommand(kSetContrastControlRegister);
    ssd1306_WriteCommand(value);
}

void ssd1306_SetDisplayOn(const uint8_t on) {
    uint8_t value;
    if (on) {
        value = 0xAF;   // Display on
        SSD1306.DisplayOn = 1;
    } else {
        value = 0xAE;   // Display off
        SSD1306.DisplayOn = 0;
    }
    ssd1306_WriteCommand(value);
}

uint8_t ssd1306_GetDisplayOn() {
    return SSD1306.DisplayOn;
}
