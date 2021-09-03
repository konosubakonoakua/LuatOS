
#include "luat_base.h"
#include "luat_lvgl.h"
#include "luat_malloc.h"
#include "luat_zbuff.h"
#include "luat_lcd.h"


static lv_disp_t* my_disp = NULL;
static lv_disp_buf_t my_disp_buff = {0};
//static lv_disp_drv_t my_disp_drv;
static luat_lcd_conf_t* lcd_conf;

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
    //-----
    if (lcd_conf != NULL) {
        lcd_conf->opts->draw(lcd_conf, area->x1, area->y1, area->x2, area->y2, color_p);
    }
    // LLOGD("CALL disp_flush (%d, %d, %d, %d)", area->x1, area->y1, area->x2, area->y2);
    lv_disp_flush_ready(disp_drv);
}

#ifdef LUA_USE_WINDOWS
#include <windows.h>
extern uint32_t WINDOW_HOR_RES;
extern uint32_t WINDOW_VER_RES;
#endif

int luat_lv_init(lua_State *L) {
    if (my_disp != NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    #ifdef LUA_USE_WINDOWS
    if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
        WINDOW_HOR_RES= luaL_checkinteger(L, 1);
        WINDOW_VER_RES = luaL_checkinteger(L, 2);
    }
    HWND windrv_init(void);
    windrv_init();
    lua_pushboolean(L, 1);
    return 1;
    #elif defined(LUA_USE_LINUX)
    //lvgl_linux_init();
    lua_pushboolean(L, 1);
    return 1;
    #else
    lcd_conf = luat_lcd_get_default();
    if (lcd_conf == NULL)
        return 0;
    lv_color_t *fbuffer = NULL;
    size_t fbuff_size = 0;

    int w = luaL_optinteger(L, 1, lcd_conf->w);
    int h = luaL_optinteger(L, 2, lcd_conf->h);

    fbuff_size = w * 10;

    LLOGD("w %d h %d buff %d", w, h, fbuff_size);

    if (lua_isuserdata(L, 3)) {
        luat_zbuff_t *zbuff = tozbuff(L);
        fbuffer = (lv_color_t *)zbuff->addr;
        fbuff_size = zbuff->len / sizeof(lv_color_t);
    }
    else {
        fbuffer = luat_heap_malloc(fbuff_size * sizeof(lv_color_t));
    }
    lv_disp_buf_init(&my_disp_buff, fbuffer, NULL, fbuff_size);

    lv_disp_drv_t my_disp_drv;
    lv_disp_drv_init(&my_disp_drv);

    my_disp_drv.flush_cb = disp_flush;

    my_disp_drv.hor_res = w;
    my_disp_drv.ver_res = h;
    my_disp_drv.buffer = &my_disp_buff;
    //LLOGD(">>%s %d", __func__, __LINE__);
    my_disp = lv_disp_drv_register(&my_disp_drv);
    //LLOGD(">>%s %d", __func__, __LINE__);
    lua_pushboolean(L, my_disp != NULL ? 1 : 0);
    //LLOGD(">>%s %d", __func__, __LINE__);
    return 1;
    #endif
}
