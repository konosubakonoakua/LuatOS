/*----------------------------------------------*/
/* TJpgDec System Configurations R0.03          */
/*----------------------------------------------*/
#include "luat_lcd.h"
#ifndef JD_SZBUF
#define JD_SZBUF
#endif
/* Specifies size of stream input buffer */

#if (LUAT_LCD_COLOR_DEPTH == 32)
#define JD_FORMAT       0   
#elif (LUAT_LCD_COLOR_DEPTH == 16)
#define JD_FORMAT       1   
#elif (LUAT_LCD_COLOR_DEPTH == 8)
#define JD_FORMAT       2
#else
#error "no supprt color depth"
#endif
/* Specifies output pixel format.
/  0: RGB888 (24-bit/pix)
/  1: RGB565 (16-bit/pix)
/  2: Grayscale (8-bit/pix)
*/

#ifndef JD_USE_SCALE
#define JD_USE_SCALE    1
#endif
/* Switches output descaling feature.
/  0: Disable
/  1: Enable
*/

#ifndef JD_TBLCLIP
#define JD_TBLCLIP      1
#endif
/* Use table conversion for saturation arithmetic. A bit faster, but increases 1 KB of code size.
/  0: Disable
/  1: Enable
*/

#define JD_FASTDECODE	1
/* Optimization level
/  0: Basic optimization. Suitable for 8/16-bit MCUs.
/  1: + 32-bit barrel shifter. Suitable for 32-bit MCUs.
/  2: + Table conversion for huffman decoding (wants 6 << HUFF_BIT bytes of RAM)
*/

