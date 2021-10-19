
/*
@module  lcd
@summary lcd驱动模块
@version 1.0
@date    2021.06.16
*/
#include "luat_base.h"
#include "luat_lcd.h"
#include "luat_malloc.h"
#include "luat_zbuff.h"
#include "luat_spi.h"

#define LUAT_LOG_TAG "lcd"
#include "luat_log.h"

extern uint32_t BACK_COLOR , FORE_COLOR ;

extern const luat_lcd_opts_t lcd_opts_st7735;
extern const luat_lcd_opts_t lcd_opts_st7735s;
extern const luat_lcd_opts_t lcd_opts_st7789;
extern const luat_lcd_opts_t lcd_opts_gc9a01;
extern const luat_lcd_opts_t lcd_opts_gc9106l;
extern const luat_lcd_opts_t lcd_opts_gc9306;
extern const luat_lcd_opts_t lcd_opts_ili9341;
extern const luat_lcd_opts_t lcd_opts_custom;

static luat_lcd_conf_t *default_conf = NULL;

/*
lcd显示屏初始化
@api lcd.init(tp, args)
@string lcd类型, 当前支持st7789/st7735/st7735s/gc9a01/gc9106l/gc9306/ili9341/custom
@table 附加参数,与具体设备有关,pin_pwr为可选项,可不设置
@usage
-- 初始化spi0的st7789 注意:lcd初始化之前需要先初始化spi
local spi_lcd = spi.device_setup(0,20,0,0,8,2000000,spi.MSB,1,1)
log.info("lcd.init",
lcd.init("st7735s",{port = "device",pin_dc = 17, pin_pwr = 7,pin_rst = 19,direction = 2,w = 160,h = 80,xoffset = 1,yoffset = 26},spi_lcd))
*/
static int l_lcd_init(lua_State* L) {
    size_t len = 0;
    luat_lcd_conf_t *conf = luat_heap_malloc(sizeof(luat_lcd_conf_t));
    memset(conf, 0, sizeof(luat_lcd_conf_t)); // 填充0,保证无脏数据
    conf->pin_pwr = 255;
    if (lua_type(L, 3) == LUA_TUSERDATA){
        conf->userdata = (luat_spi_device_t*)lua_touserdata(L, 3);
    }
    const char* tp = luaL_checklstring(L, 1, &len);
    if (!strcmp("st7735", tp) || !strcmp("st7789", tp) || !strcmp("st7735s", tp)
            || !strcmp("gc9a01", tp)  || !strcmp("gc9106l", tp)
            || !strcmp("gc9306", tp)  || !strcmp("ili9341", tp)
            || !strcmp("custom", tp)) {
        if (lua_gettop(L) > 1) {
            lua_settop(L, 2); // 丢弃多余的参数

            lua_pushstring(L, "port");
            int port = lua_gettable(L, 2);
            if (LUA_TNUMBER == port) {
                conf->port = luaL_checkinteger(L, -1);
            }
            else if (LUA_TSTRING == port){
                conf->port = LUAT_LCD_SPI_DEVICE;
            }
            lua_pop(L, 1);

            lua_pushstring(L, "pin_dc");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                conf->pin_dc = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_pushstring(L, "pin_pwr");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                conf->pin_pwr = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_pushstring(L, "pin_rst");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                conf->pin_rst = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_pushstring(L, "direction");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                conf->direction = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_pushstring(L, "w");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                conf->w = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_pushstring(L, "h");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                conf->h = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_pushstring(L, "xoffset");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                conf->xoffset = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);

            lua_pushstring(L, "yoffset");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                conf->yoffset = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
        }
        if (!strcmp("st7735", tp))
            conf->opts = (luat_lcd_opts_t*)&lcd_opts_st7735;
        else if (!strcmp("st7735s", tp))
            conf->opts = (luat_lcd_opts_t*)&lcd_opts_st7735s;
        else if (!strcmp("st7789", tp))
            conf->opts = (luat_lcd_opts_t*)&lcd_opts_st7789;
        else if (!strcmp("gc9a01", tp))
            conf->opts = (luat_lcd_opts_t*)&lcd_opts_gc9a01;
        else if (!strcmp("gc9106l", tp))
            conf->opts = (luat_lcd_opts_t*)&lcd_opts_gc9106l;
        else if (!strcmp("gc9306", tp))
            conf->opts = (luat_lcd_opts_t*)&lcd_opts_gc9306;
        else if (!strcmp("ili9341", tp))
            conf->opts = (luat_lcd_opts_t*)&lcd_opts_ili9341;
        else if (!strcmp("custom", tp)) {
            conf->opts = (luat_lcd_opts_t*)&lcd_opts_custom;
            luat_lcd_custom_t *cst = luat_heap_malloc(sizeof(luat_lcd_custom_t));

            // 获取initcmd/sleepcmd/wakecmd
            lua_pushstring(L, "sleepcmd");
            lua_gettable(L, 2);
            cst->sleepcmd = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_pushstring(L, "wakecmd");
            lua_gettable(L, 2);
            cst->wakecmd = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_pushstring(L, "initcmd");
            lua_gettable(L, 2);
            cst->init_cmd_count = lua_rawlen(L, -1);
            luat_heap_realloc(cst, sizeof(luat_lcd_custom_t) + cst->init_cmd_count * 4);
            for (size_t i = 1; i <= cst->init_cmd_count; i++)
            {
                lua_geti(L, -1, i);
                cst->initcmd[i-1] = luaL_checkinteger(L, -1);
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
        int ret = luat_lcd_init(conf);
        lua_pushboolean(L, ret == 0 ? 1 : 0);
        if (ret == 0)
            default_conf = conf;
        lua_pushlightuserdata(L, conf);
        return 2;
    }
    luat_heap_free(conf);
    return 0;
}

/*
关闭lcd显示屏
@api lcd.close()
@usage
-- 关闭lcd
lcd.close()
*/
static int l_lcd_close(lua_State* L) {
    int ret = luat_lcd_close(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
开启lcd显示屏背光
@api lcd.on()
@usage
-- 开启lcd显示屏背光
lcd.on()
*/
static int l_lcd_display_on(lua_State* L) {
    int ret = luat_lcd_display_on(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
关闭lcd显示屏背光
@api lcd.off()
@usage
-- 关闭lcd显示屏背光
lcd.off()
*/
static int l_lcd_display_off(lua_State* L) {
    int ret = luat_lcd_display_off(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd睡眠
@api lcd.sleep()
@usage
-- lcd睡眠
lcd.sleep()
*/
static int l_lcd_sleep(lua_State* L) {
    int ret = luat_lcd_sleep(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd唤醒
@api lcd.wakeup()
@usage
-- lcd唤醒
lcd.wakeup()
*/
static int l_lcd_wakeup(lua_State* L) {
    int ret = luat_lcd_wakeup(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd颜色设置
@api lcd.setColor(back,fore)
@int 背景色
@int 前景色
@usage
-- lcd颜色设置
lcd.setColor(0xFFFF,0x0000)
*/
static int l_lcd_set_color(lua_State* L) {
    uint32_t back,fore;
    back = (uint32_t)luaL_checkinteger(L, 1);
    fore = (uint32_t)luaL_checkinteger(L, 2);
    int ret = luat_lcd_set_color(back, fore);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd颜色填充
@api lcd.draw(x1, y1, x2, y2,color)
@int 左上边缘的X位置.
@int 左上边缘的Y位置.
@int 右上边缘的X位置.
@int 右上边缘的Y位置.
@string 字符串或zbuff对象
@usage
-- lcd颜色填充
buff:writeInt32(0x001F)
lcd.draw(20,30,220,30,buff)
*/
static int l_lcd_draw(lua_State* L) {
    uint16_t x1, y1, x2, y2;
    luat_color_t *color = NULL;
    luat_zbuff_t *buff;
    x1 = luaL_checkinteger(L, 1);
    y1 = luaL_checkinteger(L, 2);
    x2 = luaL_checkinteger(L, 3);
    y2 = luaL_checkinteger(L, 4);
    if (lua_isstring(L, 5)) {
        color = (luat_color_t *)luaL_checkstring(L, 5);
    }
    else if (lua_isuserdata(L, 5)) {
        buff = luaL_checkudata(L, 5, LUAT_ZBUFF_TYPE);
        color = (luat_color_t *)buff->addr;
    }
    else {
        return 0;
    }
    int ret = luat_lcd_draw(default_conf, x1, y1, x2, y2, color);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd清屏
@api lcd.clear(color)
@int 屏幕颜色 可选参数,默认背景色
@usage
-- lcd清屏
lcd.clear()
*/
static int l_lcd_clear(lua_State* L) {
    //size_t len = 0;
    uint32_t color = BACK_COLOR;
    if (lua_gettop(L) > 0)
        color = (uint32_t)luaL_checkinteger(L, 1);
    int ret = luat_lcd_clear(default_conf, color);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
画一个点.
@api lcd.drawPoint(x0,y0,color)
@int 点的X位置.
@int 点的Y位置.
@int 绘画颜色 可选参数,默认前景色
@usage
lcd.drawPoint(20,30,0x001F)
*/
static int l_lcd_draw_point(lua_State* L) {
    uint16_t x, y;
    uint32_t color = FORE_COLOR;
    x = luaL_checkinteger(L, 1);
    y = luaL_checkinteger(L, 2);
    if (lua_gettop(L) > 2)
        color = (uint32_t)luaL_checkinteger(L, 3);
    int ret = luat_lcd_draw_point(default_conf, x, y, color);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
在两点之间画一条线.
@api lcd.drawLine(x0,y0,x1,y1,color)
@int 第一个点的X位置.
@int 第一个点的Y位置.
@int 第二个点的X位置.
@int 第二个点的Y位置.
@int 绘画颜色 可选参数,默认前景色
@usage
lcd.drawLine(20,30,220,30,0x001F)
*/
static int l_lcd_draw_line(lua_State* L) {
    uint16_t x1, y1, x2, y2;
    uint32_t color = FORE_COLOR;
    x1 = luaL_checkinteger(L, 1);
    y1 = luaL_checkinteger(L, 2);
    x2 = luaL_checkinteger(L, 3);
    y2 = luaL_checkinteger(L, 4);
    if (lua_gettop(L) > 4)
        color = (uint32_t)luaL_checkinteger(L, 5);
    int ret = luat_lcd_draw_line(default_conf, x1,  y1,  x2,  y2, color);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
从x / y位置（左上边缘）开始绘制一个框
@api lcd.drawRectangle(x0,y0,x1,y1,color)
@int 左上边缘的X位置.
@int 左上边缘的Y位置.
@int 右下边缘的X位置.
@int 右下边缘的Y位置.
@int 绘画颜色 可选参数,默认前景色
@usage
lcd.drawRectangle(20,40,220,80,0x001F)
*/
static int l_lcd_draw_rectangle(lua_State* L) {
    uint16_t x1, y1, x2, y2;
    uint32_t color = FORE_COLOR;
    x1 = luaL_checkinteger(L, 1);
    y1 = luaL_checkinteger(L, 2);
    x2 = luaL_checkinteger(L, 3);
    y2 = luaL_checkinteger(L, 4);
    if (lua_gettop(L) > 4)
        color = (uint32_t)luaL_checkinteger(L, 5);
    int ret = luat_lcd_draw_rectangle(default_conf, x1,  y1,  x2,  y2, color);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
从x / y位置（圆心）开始绘制一个圆
@api lcd.drawCircle(x0,y0,r,color)
@int 圆心的X位置.
@int 圆心的Y位置.
@int 半径.
@int 绘画颜色 可选参数,默认前景色
@usage
lcd.drawCircle(120,120,20,0x001F)
*/
static int l_lcd_draw_circle(lua_State* L) {
    uint16_t x0, y0, r;
    uint32_t color = FORE_COLOR;
    x0 = luaL_checkinteger(L, 1);
    y0 = luaL_checkinteger(L, 2);
    r = luaL_checkinteger(L, 3);
    if (lua_gettop(L) > 3)
        color = (uint32_t)luaL_checkinteger(L, 4);
    int ret = luat_lcd_draw_circle(default_conf, x0,  y0,  r, color);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

// static int l_lcd_draw_str(lua_State* L) {
//     int x, y;
//     int sz;
//     const uint8_t* data;
//     uint32_t color = FORE_COLOR;
//     x = luaL_checkinteger(L, 1);
//     y = luaL_checkinteger(L, 2);
//     data = (const uint8_t*)luaL_checklstring(L, 3, &sz);
//     if (lua_gettop(L) > 3)
//         color = (uint32_t)luaL_checkinteger(L, 4);
//     if (sz == 0)
//         return 0;
// }

static int l_lcd_set_default(lua_State *L) {
    if (lua_gettop(L) == 1) {
        default_conf = lua_touserdata(L, 1);
        lua_pushboolean(L, 1);
        return 1;
    }
    return 1;
}

#include "u8g2.h"
// void u8g2_read_font_info(u8g2_font_info_t *font_info, const uint8_t *font);
uint8_t luat_u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
const uint8_t *u8g2_font_get_glyph_data(u8g2_t *u8g2, uint16_t encoding);
uint8_t * font_pix_find(uint16_t e, int font_pix_find);
/**
绘制字符串,支持中文
@api eink.printcn(x, y, str, colored, font)
@int x坐标
@int y坐标
@string 字符串
@int 默认是0
@font 字体大小,默认12
@return nil 无返回值
*/
static int l_lcd_draw_str(lua_State *L)
{
    size_t len;
    int x           = luaL_checkinteger(L, 1);
    int y           = luaL_checkinteger(L, 2);
    const char *str = luaL_checklstring(L, 3, &len);
    int colored     = luaL_optinteger(L, 4, 0);
    int font_size        = luaL_optinteger(L, 5, 12);

    //LLOGD("printcn font_size %d len %d", font_size, len);

    // 解码数据 TODO 使用无u8g2的方案
    u8g2_t *u8g2 = luat_heap_malloc(sizeof(u8g2_t));
    u8g2_Setup_ssd1306_i2c_128x64_noname_f( u8g2, U8G2_R0, u8x8_byte_sw_i2c, luat_u8x8_gpio_and_delay);
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);
    u8g2->u8x8.next_cb = u8x8_utf8_next;

    uint16_t e;
    //u8g2_uint_t delta, sum;
    u8x8_utf8_init(u8g2_GetU8x8(u8g2));
    //sum = 0;
    uint8_t* str2 = (uint8_t*)str;
    for(;;)
    {
        e = u8g2->u8x8.next_cb(u8g2_GetU8x8(u8g2), (uint8_t)*str2);
        //LLOGD("chinese >> 0x%04X", e);
        if ( e == 0x0ffff )
            break;
        str2++;
        // if (e != 0x0fffe && e < 0x007e) {
        //     char ch[2] = {e, 0};
        //     if (font_size == 16)
        //         Paint_DrawStringAt(&paint, x, y, ch, &Font16, colored);
        //     else if (font_size == 24)
        //         Paint_DrawStringAt(&paint, x, y, ch, &Font24, colored);
        //     x += font_size;
        // }
        // else
        if ( e != 0x0fffe )
        {
            //delta = u8g2_DrawGlyph(u8g2, x, y, e);
            uint8_t * f = font_pix_find(e, font_size);
            if (f != NULL) {
                // 当前仅支持16p和24p
                int datalen = font_size == 16 ? 32 : 72;
                int xlen = font_size == 16 ? 2 : 3;
                //luat_lcd_draw(default_conf, x, y, x + font_size, y + font_size, );
                //LLOGD("found FONT DATA 0x%04X datalen=%d xlen=%d", e, datalen, xlen);
                // TODO 使用内存块的方式绘制,但背景透明怎么解决呢?直接使用bgcolor?
                for (size_t i = 0; i < datalen; )
                {
                    for (size_t k = 0; k < xlen; k++)
                    {
                        uint8_t pix = f[i];
                        //LLOGD("pix data %02X   x+j=%d y+j/2=%d", pix, x, y+i/2);
                        for (size_t j = 0; j < 8; j++)
                        {
                            if ((pix >> (7-j)) & 0x01) {
                                luat_lcd_draw_point(default_conf, x+j+k*8, y+i/xlen, colored);
                            }
                        }
                        i++;
                    }
                }
            }
            else {
                LLOGD("NOT found FONT DATA 0x%04X", e);
            }

            if (e <= 0x7E)
                x += font_size / 2;
            else
            {
                x += font_size;
            }

        }
    }

    luat_heap_free(u8g2);

    return 0;
}

#include "rotable.h"
static const rotable_Reg reg_lcd[] =
{
    { "init",      l_lcd_init,   0},
    { "close",      l_lcd_close,       0},
    { "on",      l_lcd_display_on,       0},
    { "off",      l_lcd_display_off,       0},
    { "sleep",      l_lcd_sleep,       0},
    { "wakeup",      l_lcd_wakeup,       0},
    { "setColor",      l_lcd_set_color,       0},
    { "draw",      l_lcd_draw,       0},
    { "clear",      l_lcd_clear,       0},
    { "drawPoint",      l_lcd_draw_point,       0},
    { "drawLine",      l_lcd_draw_line,       0},
    { "drawRectangle",      l_lcd_draw_rectangle,       0},
    { "drawCircle",      l_lcd_draw_circle,       0},
    { "drawStr",      l_lcd_draw_str,       0},
    { "setDefault", l_lcd_set_default, 0},
	{ NULL,        NULL,   0}
};

LUAMOD_API int luaopen_lcd( lua_State *L ) {
    luat_newlib(L, reg_lcd);
    return 1;
}
