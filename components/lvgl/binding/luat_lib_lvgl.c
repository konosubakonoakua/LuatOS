
#include "luat_base.h"
#include "luat_msgbus.h"
#include "luat_lvgl.h"

//---------------------------------
// 几个快捷方法
//---------------------------------

static int luat_lv_scr_act(lua_State *L) {
    lua_pushlightuserdata(L, lv_scr_act());
    return 1;
};

static int luat_lv_layer_top(lua_State *L) {
    lua_pushlightuserdata(L, lv_layer_top());
    return 1;
};

static int luat_lv_layer_sys(lua_State *L) {
    lua_pushlightuserdata(L, lv_layer_sys());
    return 1;
};

static int luat_lv_scr_load(lua_State *L) {
    lv_scr_load(lua_touserdata(L, 1));
    return 0;
};

// 函数注册
static const rotable_Reg reg_lvgl[] = {

{"init", luat_lv_init, 0},
{"scr_act", luat_lv_scr_act, 0},
{"layer_top", luat_lv_layer_top, 0},
{"layer_sys", luat_lv_layer_sys, 0},
{"scr_load", luat_lv_scr_load, 0},

// 兼容性命名
{"sw_create", luat_lv_switch_create, 0},

LUAT_LV_DISP_RLT
LUAT_LV_GROUP_RLT
LUAT_LV_OBJ_RLT
LUAT_LV_REFR_RLT

LUAT_LV_STYLE_RLT
LUAT_LV_DRAW_RLT
LUAT_LV_ANIM_RLT
LUAT_LV_AREA_RLT
LUAT_LV_COLOR_RLT
LUAT_LV_THEME_RLT

LUAT_LV_ARC_RLT
LUAT_LV_BAR_RLT
LUAT_LV_BTN_RLT
LUAT_LV_BTNMATRIX_RLT
LUAT_LV_CALENDAR_RLT
LUAT_LV_CANVAS_RLT
LUAT_LV_CHART_RLT
LUAT_LV_CHECKBOX_RLT
LUAT_LV_CONT_RLT
LUAT_LV_CPICKER_RLT
LUAT_LV_DROPDOWN_RLT
LUAT_LV_GAUGE_RLT
LUAT_LV_IMG_RLT
LUAT_LV_IMGBTN_RLT
LUAT_LV_KEYBOARD_RLT
LUAT_LV_LABEL_RLT
LUAT_LV_LED_RLT
LUAT_LV_LINE_RLT

LUAT_LV_LIST_RLT
LUAT_LV_MSGBOX_RLT
LUAT_LV_OBJMASK_RLT
LUAT_LV_PAGE_RLT
LUAT_LV_ROLLER_RLT
LUAT_LV_SLIDER_RLT
LUAT_LV_SPINBOX_RLT
LUAT_LV_SPINNER_RLT
LUAT_LV_SWITCH_RLT
LUAT_LV_TABLE_RLT
LUAT_LV_TABVIEW_RLT
LUAT_LV_TEXTAREA_RLT
LUAT_LV_TILEVIEW_RLT
LUAT_LV_WIN_RLT

// 图像库
LUAT_LV_QRCODE_RLT
LUAT_LV_GIF_RLT

// 回调
LUAT_LV_CB_RLT

// 常量
LUAT_LV_ENMU_RLT
{NULL, NULL, 0},
};




LUAMOD_API int luaopen_lvgl( lua_State *L ) {
    luat_newlib(L, reg_lvgl);
    return 1;
}

static int lv_func_unref(lua_State *L, void* ptr) {
    rtos_msg_t* msg = (rtos_msg_t*)lua_topointer(L, -1);
    luaL_unref(L, LUA_REGISTRYINDEX, msg->arg1);
    return 0;
}

void luat_lv_user_data_free(lv_obj_t * obj) {
    if (obj == NULL || obj->user_data == 0) return;
    rtos_msg_t msg = {0};
    msg.handler = lv_func_unref;
    msg.arg1 = obj->user_data;
    luat_msgbus_put(&msg, 0);
}
