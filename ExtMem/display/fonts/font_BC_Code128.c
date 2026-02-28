// ---------------------------------------------------------------------------
// Font: Barcode Code-128
// Width:   11 px
// ---------------------------------------------------------------------------

#include "fonts.h"

#ifdef INCLUDE_FONT_BARCODE_CODE_128
static const BC_CTRL_CODE_BITMAP bmp_BC_CTRL_CODE_Bitmap[] = {
0xD0, 0x80, // @0 Start Code 128-A (11 pixels wide)
0xD2, 0x00, // @2 Start Code 128-B (11 pixels wide)
0xD3, 0x80, // @4 Start Code 128-C (11 pixels wide)
0xC7, 0x40, // @6 Stop (11 pixels wide)
0xD7, 0x00, // @8 Reverse Stop (11 pixels wide)
0xC7, 0x58, // @10 Stop pattern (7 bars/spaces) (13 pixels wide)
};
static const BC_CHAR_INFO Barcode_ControlCode_Descr[] = {
{ 11, 103, 0 }, // Start Code 128-A
{ 11, 104, 2 }, // Start Code 128-B
{ 11, 105, 4 }, // Start Code 128-C
{ 11, 106, 6 }, // Stop
{ 11, 107, 8 }, // Reverse Stop
{ 13, 108, 10 }, // Stop pattern
};
static const BC_CHAR_BITMAP bmp_Barcode_Code_128_Bitmap[] = {
0xD9, 0x80, // @0 '␣' (11 pixels wide)
0xCD, 0x80, // @2 '!' (11 pixels wide)
0xCC, 0xC0, // @4 '"' (11 pixels wide)
0x93, 0x00, // @6 '#' (11 pixels wide)
0x91, 0x80, // @8 '$' (11 pixels wide)
0x89, 0x80, // @10 '%' (11 pixels wide)
0x99, 0x00, // @12 '&' (11 pixels wide)
0x98, 0x80, // @14 ''' (11 pixels wide)
0x8C, 0x80, // @16 '(' (11 pixels wide)
0xC9, 0x00, // @18 ')' (11 pixels wide)
0xC8, 0x80, // @20 '*' (11 pixels wide)
0xC4, 0x80, // @22 '+' (11 pixels wide)
0xB3, 0x80, // @24 ',' (11 pixels wide)
0x9B, 0x80, // @26 '-' (11 pixels wide)
0x99, 0xC0, // @28 '.' (11 pixels wide)
0xB9, 0x80, // @30 '/' (11 pixels wide)
0x9D, 0x80, // @32 '0' (11 pixels wide)
0x9C, 0xC0, // @34 '1' (11 pixels wide)
0xCE, 0x40, // @36 '2' (11 pixels wide)
0xCB, 0x80, // @38 '3' (11 pixels wide)
0xC9, 0xC0, // @40 '4' (11 pixels wide)
0xDC, 0x80, // @42 '5' (11 pixels wide)
0xCE, 0x80, // @44 '6' (11 pixels wide)
0xED, 0xC0, // @46 '7' (11 pixels wide)
0xE9, 0x80, // @48 '8' (11 pixels wide)
0xE5, 0x80, // @50 '9' (11 pixels wide)
0xE4, 0xC0, // @52 ':' (11 pixels wide)
0xEC, 0x80, // @54 ';' (11 pixels wide)
0xE6, 0x80, // @56 '<' (11 pixels wide)
0xE6, 0x40, // @58 '=' (11 pixels wide)
0xDB, 0x00, // @60 '>' (11 pixels wide)
0xD8, 0xC0, // @62 '?' (11 pixels wide)
0xC6, 0xC0, // @64 '@' (11 pixels wide)
0xA3, 0x00, // @66 'A' (11 pixels wide)
0x8B, 0x00, // @68 'B' (11 pixels wide)
0x88, 0xC0, // @70 'C' (11 pixels wide)
0xB1, 0x00, // @72 'D' (11 pixels wide)
0x8D, 0x00, // @74 'E' (11 pixels wide)
0x8C, 0x40, // @76 'F' (11 pixels wide)
0xD1, 0x00, // @78 'G' (11 pixels wide)
0xC5, 0x00, // @80 'H' (11 pixels wide)
0xC4, 0x40, // @82 'I' (11 pixels wide)
0xB7, 0x00, // @84 'J' (11 pixels wide)
0xB1, 0xC0, // @86 'K' (11 pixels wide)
0x8D, 0xC0, // @88 'L' (11 pixels wide)
0xBB, 0x00, // @90 'M' (11 pixels wide)
0xB8, 0xC0, // @92 'N' (11 pixels wide)
0x8E, 0xC0, // @94 'O' (11 pixels wide)
0xEE, 0xC0, // @96 'P' (11 pixels wide)
0xD1, 0xC0, // @98 'Q' (11 pixels wide)
0xC5, 0xC0, // @100 'R' (11 pixels wide)
0xDD, 0x00, // @102 'S' (11 pixels wide)
0xDC, 0x40, // @104 'T' (11 pixels wide)
0xDD, 0xC0, // @106 'U' (11 pixels wide)
0xEB, 0x00, // @108 'V' (11 pixels wide)
0xE8, 0xC0, // @110 'W' (11 pixels wide)
0xE2, 0xC0, // @112 'X' (11 pixels wide)
0xED, 0x00, // @114 'Y' (11 pixels wide)
0xEC, 0x40, // @116 'Z' (11 pixels wide)
0xE3, 0x40, // @118 '[' (11 pixels wide)
0xEF, 0x40, // @120 '\' (11 pixels wide)
0xC8, 0x40, // @122 ']' (11 pixels wide)
0xF1, 0x40, // @124 '^' (11 pixels wide)
0xA6, 0x00, // @126 '_' (11 pixels wide)
0xA1, 0x80, // @128 '`' (11 pixels wide)
0x96, 0x00, // @130 'a' (11 pixels wide)
0x90, 0xC0, // @132 'b' (11 pixels wide)
0x85, 0x80, // @134 'c' (11 pixels wide)
0x84, 0xC0, // @136 'd' (11 pixels wide)
0xB2, 0x00, // @138 'e' (11 pixels wide)
0xB0, 0x80, // @140 'f' (11 pixels wide)
0x9A, 0x00, // @142 'g' (11 pixels wide)
0x98, 0x40, // @144 'h' (11 pixels wide)
0x86, 0x80, // @146 'i' (11 pixels wide)
0x86, 0x40, // @148 'j' (11 pixels wide)
0xC2, 0x40, // @150 'k' (11 pixels wide)
0xCA, 0x00, // @152 'l' (11 pixels wide)
0xF7, 0x40, // @154 'm' (11 pixels wide)
0xC2, 0x80, // @156 'n' (11 pixels wide)
0x8F, 0x40, // @158 'o' (11 pixels wide)
0xA7, 0x80, // @160 'p' (11 pixels wide)
0x97, 0x80, // @162 'q' (11 pixels wide)
0x93, 0xC0, // @164 'r' (11 pixels wide)
0xBC, 0x80, // @166 's' (11 pixels wide)
0x9E, 0x80, // @168 't' (11 pixels wide)
0x9E, 0x40, // @170 'u' (11 pixels wide)
0xF4, 0x80, // @172 'v' (11 pixels wide)
0xF2, 0x80, // @174 'w' (11 pixels wide)
0xF2, 0x40, // @176 'x' (11 pixels wide)
0xDB, 0xC0, // @178 'y' (11 pixels wide)
0xDE, 0xC0, // @180 'z' (11 pixels wide)
0xF6, 0xC0, // @182 '{' (11 pixels wide)
0xAF, 0x00, // @184 '|' (11 pixels wide)
0xA3, 0xC0, // @186 '}' (11 pixels wide)
0x8B, 0xC0, // @188 '~' (11 pixels wide)
};
static const BC_CHAR_INFO Barcode_Code_128_Descr[] = {
{11, 0, 0}, // '␣'
{11, 1, 2}, // '!'
{11, 2, 4}, // '"'
{11, 3, 6}, // '#'
{11, 4, 8}, // '$'
{11, 5, 10}, // '%'
{11, 6, 12}, // '&'
{11, 7, 14}, // '''
{11, 8, 16}, // '('
{11, 9, 18}, // ')'
{11, 10, 20}, // '*'
{11, 11, 22}, // '+'
{11, 12, 24}, // ','
{11, 13, 26}, // '-'
{11, 14, 28}, // '.'
{11, 15, 30}, // '/'
{11, 16, 32}, // '0'
{11, 17, 34}, // '1'
{11, 18, 36}, // '2'
{11, 19, 38}, // '3'
{11, 20, 40}, // '4'
{11, 21, 42}, // '5'
{11, 22, 44}, // '6'
{11, 23, 46}, // '7'
{11, 24, 48}, // '8'
{11, 25, 50}, // '9'
{11, 26, 52}, // ':'
{11, 27, 54}, // ';'
{11, 28, 56}, // '<'
{11, 29, 58}, // '='
{11, 30, 60}, // '>'
{11, 31, 62}, // '?'
{11, 32, 64}, // '@'
{11, 33, 66}, // 'A'
{11, 34, 68}, // 'B'
{11, 35, 70}, // 'C'
{11, 36, 72}, // 'D'
{11, 37, 74}, // 'E'
{11, 38, 76}, // 'F'
{11, 39, 78}, // 'G'
{11, 40, 80}, // 'H'
{11, 41, 82}, // 'I'
{11, 42, 84}, // 'J'
{11, 43, 86}, // 'K'
{11, 44, 88}, // 'L'
{11, 45, 90}, // 'M'
{11, 46, 92}, // 'N'
{11, 47, 94}, // 'O'
{11, 48, 96}, // 'P'
{11, 49, 98}, // 'Q'
{11, 50, 100}, // 'R'
{11, 51, 102}, // 'S'
{11, 52, 104}, // 'T'
{11, 53, 106}, // 'U'
{11, 54, 108}, // 'V'
{11, 55, 110}, // 'W'
{11, 56, 112}, // 'X'
{11, 57, 114}, // 'Y'
{11, 58, 116}, // 'Z'
{11, 59, 118}, // '['
{11, 60, 120}, // '\'
{11, 61, 122}, // ']'
{11, 62, 124}, // '^'
{11, 63, 126}, // '_'
{11, 64, 128}, // '`'
{11, 65, 130}, // 'a'
{11, 66, 132}, // 'b'
{11, 67, 134}, // 'c'
{11, 68, 136}, // 'd'
{11, 69, 138}, // 'e'
{11, 70, 140}, // 'f'
{11, 71, 142}, // 'g'
{11, 72, 144}, // 'h'
{11, 73, 146}, // 'i'
{11, 74, 148}, // 'j'
{11, 75, 150}, // 'k'
{11, 76, 152}, // 'l'
{11, 77, 154}, // 'm'
{11, 78, 156}, // 'n'
{11, 79, 158}, // 'o'
{11, 80, 160}, // 'p'
{11, 81, 162}, // 'q'
{11, 82, 164}, // 'r'
{11, 83, 166}, // 's'
{11, 84, 168}, // 't'
{11, 85, 170}, // 'u'
{11, 86, 172}, // 'v'
{11, 87, 174}, // 'w'
{11, 88, 176}, // 'x'
{11, 89, 178}, // 'y'
{11, 90, 180}, // 'z'
{11, 91, 182}, // '{'
{11, 92, 184}, // '|'
{11, 93, 186}, // '}'
{11, 94, 188}, // '~'
};
const BC_INFO font_Barcode_Code_128 = { 
' ', // start character
'~', // end character
bmp_BC_CTRL_CODE_Bitmap, // pointer to ctrl code bitmap
Barcode_ControlCode_Descr, // pointer to ctrl code info array
bmp_Barcode_Code_128_Bitmap, // pointer to bc character bitmap
Barcode_Code_128_Descr // pointer to bc character info array
};

#endif
