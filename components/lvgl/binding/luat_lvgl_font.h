#include "luat_base.h"
#include "luat_msgbus.h"
#include "lvgl.h"

int luat_lv_font_get(lua_State *L);
int luat_lv_font_load(lua_State *L);
int luat_lv_font_free(lua_State *L);

#define LUAT_LV_FONT_EX_RLT {"font_get", luat_lv_font_get, 0},\
{"font_load", luat_lv_font_load, 0},\
{"font_free", luat_lv_font_free, 0},\

