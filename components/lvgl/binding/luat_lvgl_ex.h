
#ifndef LUAT_LVGL_EX
#define LUAT_LVGL_EX

#include "luat_base.h"
#include "lvgl.h"

void luat_lvgl_ex_libs(lua_State *L);

#define LUAT_LV_EX_RLT {"DPI", NULL, LV_DPI},\
{"IMG_ZOOM_NONE", NULL, LV_IMG_ZOOM_NONE},\
{"COLOR_WHITE", NULL, 0xFFFFFFFF},\
{"COLOR_SILVER", NULL, 0xFFC0C0C0},\
{"COLOR_GRAY", NULL, 0xFF808080},\
{"COLOR_BLACK", NULL, 0xFF000000},\
{"COLOR_RED ", NULL, 0xFFFF0000},\
{"COLOR_MAROON", NULL, 0xFF800000},\
{"COLOR_YELLOW", NULL, 0xFFFFFF00},\
{"COLOR_OLIVE", NULL, 0xFF808000},\
{"COLOR_LIME", NULL, 0xFF00FF00},\
{"COLOR_GREEN", NULL, 0xFF008000},\
{"COLOR_CYAN", NULL, 0xFF00FFFF},\
{"COLOR_AQUA", NULL, 0xFF00FFFF},\
{"COLOR_TEAL", NULL, 0xFF008080},\
{"COLOR_BLUE", NULL, 0xFF0000FF},\
{"COLOR_NAVY", NULL, 0xFF000080},\
{"COLOR_MAGENTA", NULL, 0xFFFF00FF},\
{"COLOR_PURPLE", NULL, 0xFF800080},\
{"COLOR_ORANGE", NULL, 0xFFFFA500},\

#endif
