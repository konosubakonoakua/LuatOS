
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
#include "luat_fs.h"

#define LUAT_LOG_TAG "lcd"
#include "luat_log.h"

#include "u8g2.h"
#include "u8g2_luat_fonts.h"

// #include "../qrcode/qrcode.h"

int8_t u8g2_font_decode_get_signed_bits(u8g2_font_decode_t *f, uint8_t cnt);
uint8_t u8g2_font_decode_get_unsigned_bits(u8g2_font_decode_t *f, uint8_t cnt);

extern luat_color_t BACK_COLOR , FORE_COLOR ;

extern const luat_lcd_opts_t lcd_opts_st7735;
extern const luat_lcd_opts_t lcd_opts_st7735v;
extern const luat_lcd_opts_t lcd_opts_st7735s;
extern const luat_lcd_opts_t lcd_opts_st7789;
extern const luat_lcd_opts_t lcd_opts_gc9a01;
extern const luat_lcd_opts_t lcd_opts_gc9106l;
extern const luat_lcd_opts_t lcd_opts_gc9306x;
extern const luat_lcd_opts_t lcd_opts_ili9341;
extern const luat_lcd_opts_t lcd_opts_ili9488;
extern const luat_lcd_opts_t lcd_opts_custom;

typedef struct lcd_reg {
  const char *name;
  const luat_lcd_opts_t *lcd_opts;
}lcd_reg_t;

static const lcd_reg_t lcd_regs[] = {
  {"custom",  &lcd_opts_custom},   //0 固定为第零个
  {"st7735",  &lcd_opts_st7735},   //1
  {"st7735v", &lcd_opts_st7735v},  //2
  {"st7735s", &lcd_opts_st7735s},  //3
  {"st7789",  &lcd_opts_st7789},   //4
  {"gc9a01",  &lcd_opts_gc9a01},   //5
  {"gc9106l", &lcd_opts_gc9106l},  //6
  {"gc9306x", &lcd_opts_gc9306x},  //7
  {"gc9306",  &lcd_opts_gc9306x},  //gc9306是gc9306x的别名
  {"ili9341", &lcd_opts_ili9341},  //8
  {"ili9488", &lcd_opts_ili9488},  //9
  {"", NULL} // 最后一个必须是空字符串
};


static luat_lcd_conf_t *default_conf = NULL;
static int dft_conf_lua_ref = 0;

// 所有绘图相关的函数都应该调用本函数
static void lcd_auto_flush(luat_lcd_conf_t *conf) {
#ifndef LUAT_USE_LCD_SDL2
  if (conf == NULL || conf->buff == NULL || conf->auto_flush == 0)
    return;
#endif
  luat_lcd_flush(conf);
}

static luat_color_t lcd_str_fg_color,lcd_str_bg_color;
/*
lcd显示屏初始化
@api lcd.init(tp, args)
@string lcd类型, 当前支持st7789/st7735/st7735v/st7735s/gc9a01/gc9106l/gc9306x/ili9341/custom
@table 附加参数,与具体设备有关,pin_pwr为可选项,可不设置port:spi端口,例如0,1,2...如果为device方式则为"device";pin_dc:lcd数据/命令选择引脚;pin_rst:lcd复位引脚;pin_pwr:lcd背光引脚 可选项,可不设置;direction:lcd屏幕方向 0:0° 1:180° 2:270° 3:90°;w:lcd 水平分辨率;h:lcd 竖直分辨率;xoffset:x偏移(不同屏幕ic 不同屏幕方向会有差异);yoffset:y偏移(不同屏幕ic 不同屏幕方向会有差异)
@userdata spi设备,当port = "device"时有效
@usage
-- 初始化spi0的st7789 注意:lcd初始化之前需要先初始化spi
spi_lcd = spi.deviceSetup(0,20,0,0,8,2000000,spi.MSB,1,1)
log.info("lcd.init",
lcd.init("st7735s",{port = "device",pin_dc = 17, pin_pwr = 7,pin_rst = 19,direction = 2,w = 160,h = 80,xoffset = 1,yoffset = 26},spi_lcd))
*/
static int l_lcd_init(lua_State* L) {
    size_t len = 0;
    luat_lcd_conf_t *conf = luat_heap_malloc(sizeof(luat_lcd_conf_t));
    if (conf == NULL) {
      LLOGE("out of system memory!!!");
      return 0;
    }
    memset(conf, 0, sizeof(luat_lcd_conf_t)); // 填充0,保证无脏数据
    conf->pin_pwr = 255;
    if (lua_type(L, 3) == LUA_TUSERDATA){
        conf->lcd_spi_device = (luat_spi_device_t*)lua_touserdata(L, 3);
        conf->port = LUAT_LCD_SPI_DEVICE;
    }
    const char* tp = luaL_checklstring(L, 1, &len);
    int16_t s_index = -1;//第几个屏幕，-1表示没匹配到
    for(int i = 0; i < 100; i++){
        if (strlen(lcd_regs[i].name) == 0)
          break;
        if(strcmp(lcd_regs[i].name,tp) == 0){
            s_index = i;
            break;
        }
    }
    if (s_index != -1) {
        LLOGD("ic support: %s",tp);
        if (lua_gettop(L) > 1) {
            conf->opts = (struct luat_lcd_opts *)lcd_regs[s_index].lcd_opts;
            lua_settop(L, 2); // 丢弃多余的参数

            lua_pushstring(L, "port");
            int port = lua_gettable(L, 2);
            if (conf->port == LUAT_LCD_SPI_DEVICE && port ==LUA_TNUMBER) {
              LLOGE("port is not device but find luat_spi_device_t");
              goto end;
            }else if (conf->port != LUAT_LCD_SPI_DEVICE && LUA_TSTRING == port){
              LLOGE("port is device but not find luat_spi_device_t");
              goto end;
            }else if (LUA_TNUMBER == port) {
                conf->port = luaL_checkinteger(L, -1);
            }else if (LUA_TSTRING == port){
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
            conf->buffer_size = (conf->w * conf->h) * 2;
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
        if (s_index == 0){
            luat_lcd_custom_t *cst = luat_heap_malloc(sizeof(luat_lcd_custom_t));
            lua_pushstring(L, "initcmd");
            lua_gettable(L, 2);
            if (lua_istable(L, -1)) {
              cst->init_cmd_count = lua_rawlen(L, -1);
              cst->initcmd = luat_heap_malloc(cst->init_cmd_count * sizeof(uint32_t));
              for (size_t i = 1; i <= cst->init_cmd_count; i++){
                  lua_geti(L, -1, i);
                  cst->initcmd[i-1] = luaL_checkinteger(L, -1);
                  lua_pop(L, 1);
              }
            }else if(lua_isstring(L, -1)){
              size_t  len,cmd;
              const char *fail_name = luaL_checklstring(L, -1, &len);
              FILE* fd = (FILE *)luat_fs_fopen(fail_name, "rb");
              cst->init_cmd_count = 0;
              if (fd){
                  #define INITCMD_BUFF_SIZE 128
                  char init_cmd_buff[INITCMD_BUFF_SIZE] ;
                  cst->initcmd = luat_heap_malloc(sizeof(uint32_t));
                  while (1) {
                      memset(init_cmd_buff, 0, INITCMD_BUFF_SIZE);
                      int readline_len = luat_fs_readline(init_cmd_buff, INITCMD_BUFF_SIZE-1, fd);
                      if (readline_len < 1)
                          break;
                      if (memcmp(init_cmd_buff, "#", 1)==0){
                          continue;
                      }
                      char *token = strtok(init_cmd_buff, ",");
                      if (sscanf(token,"%x",&cmd) < 1){
                          continue;
                      }
                      cst->init_cmd_count = cst->init_cmd_count + 1;
                      cst->initcmd = luat_heap_realloc(cst->initcmd,cst->init_cmd_count * sizeof(uint32_t));
                      cst->initcmd[cst->init_cmd_count-1]=cmd;
                      while( token != NULL ) {
                          token = strtok(NULL, ",");
                          if (sscanf(token,"%x",&cmd) < 1){
                              break;
                          }
                          cst->init_cmd_count = cst->init_cmd_count + 1;
                          cst->initcmd = luat_heap_realloc(cst->initcmd,cst->init_cmd_count * sizeof(uint32_t));
                          cst->initcmd[cst->init_cmd_count-1]=cmd;
                      }
                  }
                  cst->initcmd[cst->init_cmd_count]= 0;
                  luat_fs_fclose(fd);
              }else{
                  LLOGE("init_cmd fail open error");
              }
            }
            lua_pop(L, 1);
            conf->userdata = cst;
        }
        // 默认自动flush,即使没有buff
        conf->auto_flush = 1;

#ifdef LUAT_USE_LCD_SDL2
        extern const luat_lcd_opts_t lcd_opts_sdl2;
        conf->opts = &lcd_opts_sdl2;
#endif
        int ret = luat_lcd_init(conf);
        if (ret == 0) {
            if (dft_conf_lua_ref) {
              luaL_unref(L, LUA_REGISTRYINDEX, dft_conf_lua_ref);
            }
            default_conf = conf;
            dft_conf_lua_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }
        u8g2_SetFontMode(&(conf->luat_lcd_u8g2), 0);
        u8g2_SetFontDirection(&(conf->luat_lcd_u8g2), 0);
        lua_pushboolean(L, ret == 0 ? 1 : 0);
        // lua_pushlightuserdata(L, conf);
        return 1;
    }
    LLOGE("ic not support: %s",tp);
end:
    lua_pushboolean(L, 0);
    luat_heap_free(conf);
    return 1;
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
lcd反显
@api lcd.invon()
@usage
-- lcd反显
lcd.invon()
*/
static int l_lcd_inv_on(lua_State* L) {
    int ret = luat_lcd_inv_on(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd反显关闭
@api lcd.invoff()
@usage
-- lcd反显关闭
lcd.invoff()
*/
static int l_lcd_inv_off(lua_State* L) {
    int ret = luat_lcd_inv_off(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd命令
@api lcd.cmd(cmd)
@int cmd
@usage
-- lcd命令
lcd.cmd(0x21)
*/
static int l_lcd_write_cmd(lua_State* L) {
    int ret = lcd_write_cmd(default_conf,(const uint8_t)luaL_checkinteger(L, 1));
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd数据
@api lcd.data(data)
@int data
@usage
-- lcd数据
lcd.data(0x21)
*/
static int l_lcd_write_data(lua_State* L) {
    int ret = lcd_write_data(default_conf,(const uint8_t)luaL_checkinteger(L, 1));
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
    luat_color_t back,fore;
    back = (luat_color_t)luaL_checkinteger(L, 1);
    fore = (luat_color_t)luaL_checkinteger(L, 2);
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
local buff = zbuff.create({201,1,16},0x001F)
lcd.draw(20,30,220,30,buff)
*/
static int l_lcd_draw(lua_State* L) {
    uint16_t x1, y1, x2, y2;
    int ret;
    // luat_color_t *color = NULL;
    luat_zbuff_t *buff;
    x1 = luaL_checkinteger(L, 1);
    y1 = luaL_checkinteger(L, 2);
    x2 = luaL_checkinteger(L, 3);
    y2 = luaL_checkinteger(L, 4);
    if (lua_isinteger(L, 5)) {
        // color = (luat_color_t *)luaL_checkstring(L, 5);
        luat_color_t color = (uint32_t)luaL_checkinteger(L, 1);
        ret = luat_lcd_draw(default_conf, x1, y1, x2, y2, &color);
    }
    else if (lua_isuserdata(L, 5)) {
        buff = luaL_checkudata(L, 5, LUAT_ZBUFF_TYPE);
        luat_color_t *color = (luat_color_t *)buff->addr;
        ret = luat_lcd_draw(default_conf, x1, y1, x2, y2, color);
    }
    else if(lua_isstring(L, 5)) {
        luat_color_t *color = (luat_color_t *)luaL_checkstring(L, 5);
        ret = luat_lcd_draw(default_conf, x1, y1, x2, y2, color);
    }
    else {
        return 0;
    }
    lcd_auto_flush(default_conf);
    // int ret = luat_lcd_draw(default_conf, x1, y1, x2, y2, color);
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
    luat_color_t color = BACK_COLOR;
    if (lua_gettop(L) > 0)
        color = (uint32_t)luaL_checkinteger(L, 1);
    int ret = luat_lcd_clear(default_conf, color);
    lcd_auto_flush(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
lcd颜色填充
@api lcd.fill(x1, y1, x2, y2,color)
@int 左上边缘的X位置.
@int 左上边缘的Y位置.
@int 右上边缘的X位置.
@int 右上边缘的Y位置.
@int 绘画颜色 可选参数,默认背景色
@usage
-- lcd颜色填充
lcd.fill(20,30,220,30,0x0000)
*/
static int l_lcd_draw_fill(lua_State* L) {
    uint16_t x1, y1, x2, y2;
    luat_color_t color = BACK_COLOR;
    x1 = luaL_checkinteger(L, 1);
    y1 = luaL_checkinteger(L, 2);
    x2 = luaL_checkinteger(L, 3);
    y2 = luaL_checkinteger(L, 4);
    if (lua_gettop(L) > 4)
        color = (luat_color_t)luaL_checkinteger(L, 5);
    int ret = luat_lcd_draw_fill(default_conf, x1,  y1,  x2,  y2, color);
    lcd_auto_flush(default_conf);
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
    luat_color_t color = FORE_COLOR;
    x = luaL_checkinteger(L, 1);
    y = luaL_checkinteger(L, 2);
    if (lua_gettop(L) > 2)
        color = (luat_color_t)luaL_checkinteger(L, 3);
    int ret = luat_lcd_draw_point(default_conf, x, y, color);
    lcd_auto_flush(default_conf);
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
    luat_color_t color = FORE_COLOR;
    x1 = luaL_checkinteger(L, 1);
    y1 = luaL_checkinteger(L, 2);
    x2 = luaL_checkinteger(L, 3);
    y2 = luaL_checkinteger(L, 4);
    if (lua_gettop(L) > 4)
        color = (luat_color_t)luaL_checkinteger(L, 5);
    int ret = luat_lcd_draw_line(default_conf, x1,  y1,  x2,  y2, color);
    lcd_auto_flush(default_conf);
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
    luat_color_t color = FORE_COLOR;
    x1 = luaL_checkinteger(L, 1);
    y1 = luaL_checkinteger(L, 2);
    x2 = luaL_checkinteger(L, 3);
    y2 = luaL_checkinteger(L, 4);
    if (lua_gettop(L) > 4)
        color = (luat_color_t)luaL_checkinteger(L, 5);
    int ret = luat_lcd_draw_rectangle(default_conf, x1,  y1,  x2,  y2, color);
    lcd_auto_flush(default_conf);
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
    luat_color_t color = FORE_COLOR;
    x0 = luaL_checkinteger(L, 1);
    y0 = luaL_checkinteger(L, 2);
    r = luaL_checkinteger(L, 3);
    if (lua_gettop(L) > 3)
        color = (luat_color_t)luaL_checkinteger(L, 4);
    int ret = luat_lcd_draw_circle(default_conf, x0,  y0,  r, color);
    lcd_auto_flush(default_conf);
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

#ifdef LUAT_USE_QRCODE
#include "qrcodegen.h"
/**
缓冲区绘制QRCode
@api lcd.drawQrcode(x, y, str, size)
@int x坐标
@int y坐标
@string 二维码的内容
@int 可选,显示大小,不可小于21,默认21
@return nil 无返回值
*/
static int l_lcd_drawQrcode(lua_State *L)
{
    size_t len;
    int x           = luaL_checkinteger(L, 1);
    int y           = luaL_checkinteger(L, 2);
    const char* text = luaL_checklstring(L, 3, &len);
    int size        = luaL_optinteger(L, 4,21);
    uint8_t *qrcode = luat_heap_malloc(qrcodegen_BUFFER_LEN_MAX);
    uint8_t *tempBuffer = luat_heap_malloc(qrcodegen_BUFFER_LEN_MAX);
    if (qrcode == NULL || tempBuffer == NULL) {
        if (qrcode)
            luat_heap_free(qrcode);
        if (tempBuffer)
            luat_heap_free(tempBuffer);
        LLOGE("qrcode out of memory");
        return 0;
    }
    bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, qrcodegen_Ecc_MEDIUM,
        qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    if (ok){
        int qr_size = qrcodegen_getSize(qrcode);
        int scale = size / qr_size ;
        int margin = (size - qr_size * scale) / 2;
        luat_lcd_draw_fill(default_conf,x,y,x+size,y+size,BACK_COLOR);
        x+=margin;
        y+=margin;
        for (int j = 0; j < qr_size; j++) {
            for (int i = 0; i < qr_size; i++) {
                if (qrcodegen_getModule(qrcode, i, j))
                    luat_lcd_draw_fill(default_conf,x+i*scale,y+j*scale,x+(i+1)*scale,y+(j+1)*scale,FORE_COLOR);
            }
        }
    }
    if (qrcode)
        luat_heap_free(qrcode);
    if (tempBuffer)
        luat_heap_free(tempBuffer);
    lcd_auto_flush(default_conf);
    return 0;
}
#endif

static uint8_t utf8_state;
static uint16_t encoding;
static uint16_t utf8_next(uint8_t b)
{
  if ( b == 0 )  /* '\n' terminates the string to support the string list procedures */
    return 0x0ffff; /* end of string detected, pending UTF8 is discarded */
  if ( utf8_state == 0 )
  {
    if ( b >= 0xfc )  /* 6 byte sequence */
    {
      utf8_state = 5;
      b &= 1;
    }
    else if ( b >= 0xf8 )
    {
      utf8_state = 4;
      b &= 3;
    }
    else if ( b >= 0xf0 )
    {
      utf8_state = 3;
      b &= 7;
    }
    else if ( b >= 0xe0 )
    {
      utf8_state = 2;
      b &= 15;
    }
    else if ( b >= 0xc0 )
    {
      utf8_state = 1;
      b &= 0x01f;
    }
    else
    {
      /* do nothing, just use the value as encoding */
      return b;
    }
    encoding = b;
    return 0x0fffe;
  }
  else
  {
    utf8_state--;
    /* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
    encoding<<=6;
    b &= 0x03f;
    encoding |= b;
    if ( utf8_state != 0 )
      return 0x0fffe; /* nothing to do yet */
  }
  return encoding;
}

static void u8g2_draw_hv_line(u8g2_t *u8g2, int16_t x, int16_t y, int16_t len, uint8_t dir, uint16_t color){
  switch(dir)
  {
    case 0:
      luat_lcd_draw_hline(default_conf,x,y,len,color);
      break;
    case 1:
      luat_lcd_draw_vline(default_conf,x,y,len,color);
      break;
    case 2:
        luat_lcd_draw_hline(default_conf,x-len+1,y,len,color);
      break;
    case 3:
      luat_lcd_draw_vline(default_conf,x,y-len+1,len,color);
      break;
  }
}

static void u8g2_font_decode_len(u8g2_t *u8g2, uint8_t len, uint8_t is_foreground){
  uint8_t cnt;  /* total number of remaining pixels, which have to be drawn */
  uint8_t rem;  /* remaining pixel to the right edge of the glyph */
  uint8_t current;  /* number of pixels, which need to be drawn for the draw procedure */
    /* current is either equal to cnt or equal to rem */
  /* local coordinates of the glyph */
  uint8_t lx,ly;
  /* target position on the screen */
  int16_t x, y;
  u8g2_font_decode_t *decode = &(u8g2->font_decode);
  cnt = len;
  /* get the local position */
  lx = decode->x;
  ly = decode->y;
  for(;;){
    /* calculate the number of pixel to the right edge of the glyph */
    rem = decode->glyph_width;
    rem -= lx;
    /* calculate how many pixel to draw. This is either to the right edge */
    /* or lesser, if not enough pixel are left */
    current = rem;
    if ( cnt < rem )
      current = cnt;
    /* now draw the line, but apply the rotation around the glyph target position */
    //u8g2_font_decode_draw_pixel(u8g2, lx,ly,current, is_foreground);
    // printf("lx:%d,ly:%d,current:%d, is_foreground:%d \r\n",lx,ly,current, is_foreground);
    /* get target position */
    x = decode->target_x;
    y = decode->target_y;
    /* apply rotation */
    x = u8g2_add_vector_x(x, lx, ly, decode->dir);
    y = u8g2_add_vector_y(y, lx, ly, decode->dir);
    /* draw foreground and background (if required) */
    if ( current > 0 )		/* avoid drawing zero length lines, issue #4 */
    {
      if ( is_foreground )
      {
	    u8g2_draw_hv_line(u8g2, x, y, current, decode->dir, lcd_str_fg_color);
      }
      // else if ( decode->is_transparent == 0 )
      // {
	    // u8g2_draw_hv_line(u8g2, x, y, current, decode->dir, lcd_str_bg_color);
      // }
    }
    /* check, whether the end of the run length code has been reached */
    if ( cnt < rem )
      break;
    cnt -= rem;
    lx = 0;
    ly++;
  }
  lx += cnt;
  decode->x = lx;
  decode->y = ly;
}
static void u8g2_font_setup_decode(u8g2_t *u8g2, const uint8_t *glyph_data)
{
  u8g2_font_decode_t *decode = &(u8g2->font_decode);
  decode->decode_ptr = glyph_data;
  decode->decode_bit_pos = 0;

  /* 8 Nov 2015, this is already done in the glyph data search procedure */
  /*
  decode->decode_ptr += 1;
  decode->decode_ptr += 1;
  */

  decode->glyph_width = u8g2_font_decode_get_unsigned_bits(decode, u8g2->font_info.bits_per_char_width);
  decode->glyph_height = u8g2_font_decode_get_unsigned_bits(decode,u8g2->font_info.bits_per_char_height);

}
static int8_t u8g2_font_decode_glyph(u8g2_t *u8g2, const uint8_t *glyph_data){
  uint8_t a, b;
  int8_t x, y;
  int8_t d;
  int8_t h;
  u8g2_font_decode_t *decode = &(u8g2->font_decode);
  u8g2_font_setup_decode(u8g2, glyph_data);
  h = u8g2->font_decode.glyph_height;
  x = u8g2_font_decode_get_signed_bits(decode, u8g2->font_info.bits_per_char_x);
  y = u8g2_font_decode_get_signed_bits(decode, u8g2->font_info.bits_per_char_y);
  d = u8g2_font_decode_get_signed_bits(decode, u8g2->font_info.bits_per_delta_x);

  if ( decode->glyph_width > 0 )
  {
    decode->target_x = u8g2_add_vector_x(decode->target_x, x, -(h+y), decode->dir);
    decode->target_y = u8g2_add_vector_y(decode->target_y, x, -(h+y), decode->dir);
    //u8g2_add_vector(&(decode->target_x), &(decode->target_y), x, -(h+y), decode->dir);
    /* reset local x/y position */
    decode->x = 0;
    decode->y = 0;
    /* decode glyph */
    for(;;){
      a = u8g2_font_decode_get_unsigned_bits(decode, u8g2->font_info.bits_per_0);
      b = u8g2_font_decode_get_unsigned_bits(decode, u8g2->font_info.bits_per_1);
      do{
        u8g2_font_decode_len(u8g2, a, 0);
        u8g2_font_decode_len(u8g2, b, 1);
      } while( u8g2_font_decode_get_unsigned_bits(decode, 1) != 0 );
      if ( decode->y >= h )
        break;
    }
  }
  return d;
}
const uint8_t *u8g2_font_get_glyph_data(u8g2_t *u8g2, uint16_t encoding);
static int16_t u8g2_font_draw_glyph(u8g2_t *u8g2, int16_t x, int16_t y, uint16_t encoding){
  int16_t dx = 0;
  u8g2->font_decode.target_x = x;
  u8g2->font_decode.target_y = y;
  const uint8_t *glyph_data = u8g2_font_get_glyph_data(u8g2, encoding);
  if ( glyph_data != NULL ){
    dx = u8g2_font_decode_glyph(u8g2, glyph_data);
  }
  return dx;
}

/*
设置字体
@api lcd.setFont(font)
@int font lcd.font_opposansm8 lcd.font_opposansm10 lcd.font_opposansm16  lcd.font_opposansm18  lcd.font_opposansm20  lcd.font_opposansm22  lcd.font_opposansm24 lcd.font_opposansm32 lcd.font_opposansm12_chinese lcd.font_opposansm16_chinese lcd.font_opposansm24_chinese lcd.font_opposansm32_chinese 
@usage
-- 设置为字体,对之后的drawStr有效,调用lcd.drawStr前一定要先设置
-- 使用中文字体需在luat_conf_bsp.h中开启相对应的宏
lcd.setFont(lcd.font_opposansm12)
lcd.drawStr(40,10,"drawStr")
sys.wait(2000)
lcd.setFont(lcd.font_opposansm12_chinese)
lcd.drawStr(40,40,"drawStr测试")
*/
static int l_lcd_set_font(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
      LLOGE("only font pointer is allow");
      return 0;
    }
    const uint8_t *ptr = (const uint8_t *)lua_touserdata(L, 1);
    if (ptr == NULL) {
      LLOGE("only font pointer is allow");
      return 0;
    }
    u8g2_SetFont(&(default_conf->luat_lcd_u8g2), ptr);
    lua_pushboolean(L, 1);
    return 1;
}

/*
显示字符串
@api lcd.drawStr(x,y,str,fg_color)
@int x 横坐标
@int y 竖坐标  注意:此(x,y)为左下起始坐标
@string str 文件内容
@int fg_color str颜色 注意:此参数可选，如不填写则使用之前设置的颜色，绘制只会绘制字体部分，背景需要自己清除
@usage
-- 显示之前先设置为中文字体,对之后的drawStr有效,使用中文字体需在luat_conf_bsp.h.h开启#define USE_U8G2_OPPOSANSMxx_CHINESE xx代表字号
lcd.setFont(lcd.font_opposansm12)
lcd.drawStr(40,10,"drawStr")
sys.wait(2000)
lcd.setFont(lcd.font_opposansm16_chinese)
lcd.drawStr(40,40,"drawStr测试")
*/
static int l_lcd_draw_str(lua_State* L) {
    int x, y;
    size_t sz;
    const uint8_t* data;
    x = luaL_checkinteger(L, 1);
    y = luaL_checkinteger(L, 2);
    data = (const uint8_t*)luaL_checklstring(L, 3, &sz);
    lcd_str_fg_color = (luat_color_t)luaL_optinteger(L, 4,FORE_COLOR);
    // lcd_str_bg_color = (uint32_t)luaL_optinteger(L, 5,BACK_COLOR);
    if (sz == 0)
        return 0;
    uint16_t e;
    int16_t delta;
    utf8_state = 0;
    
    for(;;){
        e = utf8_next((uint8_t)*data);
        if ( e == 0x0ffff )
        break;
        data++;
        if ( e != 0x0fffe ){
        delta = u8g2_font_draw_glyph(&(default_conf->luat_lcd_u8g2), x, y, e);
        switch(default_conf->luat_lcd_u8g2.font_decode.dir){
            case 0:
            x += delta;
            break;
            case 1:
            y += delta;
            break;
            case 2:
            x -= delta;
            break;
            case 3:
            y -= delta;
            break;
        }
        }
    }
    lcd_auto_flush(default_conf);
    return 0;
}

#ifdef LUAT_USE_GTFONT

#include "GT5SLCD2E_1A.h"
extern void gtfont_draw_w(unsigned char *pBits,unsigned int x,unsigned int y,unsigned int widt,unsigned int high,int(*point)(void*),void* userdata,int mode);
extern void gtfont_draw_gray_hz(unsigned char *data,unsigned short x,unsigned short y,unsigned short w ,unsigned short h,unsigned char grade, unsigned char HB_par,int(*point)(void*,uint16_t, uint16_t, uint32_t),void* userdata,int mode);

/*
使用gtfont显示gb2312字符串
@api lcd.drawGtfontGb2312(str,size,x,y)
@string str 显示字符串
@int size 字体大小 (支持16-192号大小字体)
@int x 横坐标
@int y 竖坐标
@usage
lcd.drawGtfontGb2312("啊啊啊",32,0,0)
*/
static int l_lcd_draw_gtfont_gb2312(lua_State *L) {
    unsigned char buf[128];
	size_t len;
	int i = 0;
	uint8_t strhigh,strlow ;
	uint16_t str;
  const char *fontCode = luaL_checklstring(L, 1,&len);
  unsigned char size = luaL_checkinteger(L, 2);
	int x = luaL_checkinteger(L, 3);
	int y = luaL_checkinteger(L, 4);
	while ( i < len){
		strhigh = *fontCode;
		fontCode++;
		strlow = *fontCode;
		str = (strhigh<<8)|strlow;
		fontCode++;
		int font_size = get_font(buf, 1, str, size, size, size);
    if(font_size != size){
      LLOGW("get gtfont error");
      return 0;
    }
		gtfont_draw_w(buf , x ,y , size , size,luat_lcd_draw_point,default_conf,0);
		x+=size;
		i+=2;
	}
    lcd_auto_flush(default_conf);
    return 0;
}

/*
使用gtfont灰度显示gb2312字符串
@api lcd.drawGtfontGb2312Gray(str,size,gray,x,y)
@string str 显示字符串
@int size 字体大小 (支持16-192号大小字体)
@int gray 灰度[1阶/2阶/3阶/4阶]
@int x 横坐标
@int y 竖坐标
@usage
lcd.drawGtfontGb2312Gray("啊啊啊",32,4,0,40)
*/
static int l_lcd_draw_gtfont_gb2312_gray(lua_State* L) {
	unsigned char buf[2048];
	size_t len;
	int i = 0;
	uint8_t strhigh,strlow ;
	uint16_t str;
  const char *fontCode = luaL_checklstring(L, 1,&len);
  unsigned char size = luaL_checkinteger(L, 2);
	unsigned char font_g = luaL_checkinteger(L, 3);
	int x = luaL_checkinteger(L, 4);
	int y = luaL_checkinteger(L, 5);
	while ( i < len){
		strhigh = *fontCode;
		fontCode++;
		strlow = *fontCode;
		str = (strhigh<<8)|strlow;
		fontCode++;
		int font_size = get_font(buf, 1, str, size*font_g, size*font_g, size*font_g);
    if(font_size != size*font_g){
      LLOGW("get gtfont error");
      return 0;
    }
		Gray_Process(buf,size,size,font_g);
		gtfont_draw_gray_hz(buf, x, y, size , size, font_g, 1,luat_lcd_draw_point,default_conf,0);
		x+=size;
		i+=2;
	}
    lcd_auto_flush(default_conf);
    return 0;
}

#ifdef LUAT_USE_GTFONT_UTF8
extern unsigned short unicodetogb2312 ( unsigned short	chr);

/*
使用gtfont显示UTF8字符串
@api lcd.drawGtfontUtf8(str,size,x,y)
@string str 显示字符串
@int size 字体大小 (支持16-192号大小字体)
@int x 横坐标
@int y 竖坐标
@usage
lcd.drawGtfontUtf8("啊啊啊",32,0,0)
*/
static int l_lcd_draw_gtfont_utf8(lua_State *L) {
    unsigned char buf[128];
    size_t len;
    int i = 0;
    uint8_t strhigh,strlow ;
    uint16_t e,str;
    const char *fontCode = luaL_checklstring(L, 1,&len);
    unsigned char size = luaL_checkinteger(L, 2);
    int x = luaL_checkinteger(L, 3);
    int y = luaL_checkinteger(L, 4);
    for(;;){
      e = utf8_next((uint8_t)*fontCode);
      if ( e == 0x0ffff )
      break;
      fontCode++;
      if ( e != 0x0fffe ){
        uint16_t str = unicodetogb2312(e);
        int font_size = get_font(buf, 1, str, size, size, size);
        if(font_size != size){
          LLOGW("get gtfont error");
          return 0;
        }
        gtfont_draw_w(buf , x ,y , size , size,luat_lcd_draw_point,default_conf,0);
        x+=size;
      }
    }
    lcd_auto_flush(default_conf);
    return 0;
}

/*
使用gtfont灰度显示UTF8字符串
@api lcd.drawGtfontUtf8Gray(str,size,gray,x,y)
@string str 显示字符串
@int size 字体大小 (支持16-192号大小字体)
@int gray 灰度[1阶/2阶/3阶/4阶]
@int x 横坐标
@int y 竖坐标
@usage
lcd.drawGtfontUtf8Gray("啊啊啊",32,4,0,40)
*/
static int l_lcd_draw_gtfont_utf8_gray(lua_State* L) {
	unsigned char buf[2048];
	size_t len;
	int i = 0;
	uint8_t strhigh,strlow ;
	uint16_t e,str;
  const char *fontCode = luaL_checklstring(L, 1,&len);
  unsigned char size = luaL_checkinteger(L, 2);
	unsigned char font_g = luaL_checkinteger(L, 3);
	int x = luaL_checkinteger(L, 4);
	int y = luaL_checkinteger(L, 5);
	for(;;){
        e = utf8_next((uint8_t)*fontCode);
        if ( e == 0x0ffff )
        break;
        fontCode++;
        if ( e != 0x0fffe ){
			uint16_t str = unicodetogb2312(e);
			int font_size = get_font(buf, 1, str, size*font_g, size*font_g, size*font_g);
      if(font_size != size*font_g){
        LLOGW("get gtfont error");
        return 0;
      }
			Gray_Process(buf,size,size,font_g);
      gtfont_draw_gray_hz(buf, x, y, size , size, font_g, 1,luat_lcd_draw_point,default_conf,0);
        	x+=size;
        }
    }
    lcd_auto_flush(default_conf);
    return 0;
}

#endif // LUAT_USE_GTFONT_UTF8

#endif // LUAT_USE_GTFONT

static int l_lcd_set_default(lua_State *L) {
    if (lua_gettop(L) == 1) {
        default_conf = lua_touserdata(L, 1);
        lua_pushboolean(L, 1);
        return 1;
    }
    return 1;
}

static int l_lcd_get_default(lua_State *L) {
    if (default_conf == NULL)
      return 0;
    lua_pushlightuserdata(L, default_conf);
    return 1;
}

/*
获取屏幕尺寸
@api lcd.getSize()
@return int 宽, 如果未初始化会返回0
@return int 高, 如果未初始化会返回0
@usage
log.info("lcd", "size", lcd.getSize())
*/
static int l_lcd_get_size(lua_State *L) {
  if (lua_gettop(L) == 1) {
    luat_lcd_conf_t * conf = lua_touserdata(L, 1);
    if (conf) {
      lua_pushinteger(L, conf->w);
      lua_pushinteger(L, conf->h);
      return 2;
    }
  }
  if (default_conf == NULL) {
    lua_pushinteger(L, 0);
    lua_pushinteger(L, 0);
  }
  else {
    lua_pushinteger(L, default_conf->w);
    lua_pushinteger(L, default_conf->h);
  }
  return 2;
}

static void lcd_DrawHXBM(uint16_t x, uint16_t y, uint16_t len, const uint8_t *b){
  uint8_t mask;
  mask = 1;
  while(len > 0) {
    if ( *b & mask ) luat_lcd_draw_hline(default_conf, x, y, 1,FORE_COLOR);
    else luat_lcd_draw_vline(default_conf, x, y, 1,BACK_COLOR);
    x++;
    mask <<= 1;
    if ( mask == 0 ){
      mask = 1;
      b++;
    }
    len--;
  }
}

/*
绘制位图
@api lcd.drawXbm(x, y, w, h, data)
@int X坐标
@int y坐标
@int 位图宽
@int 位图高
@int 位图数据,每一位代表一个像素
@usage
-- 取模使用PCtoLCD2002软件即可
-- 在(0,0)为左上角,绘制 16x16 "今" 的位图
lcd.drawXbm(0, 0, 16,16, string.char(
    0x80,0x00,0x80,0x00,0x40,0x01,0x20,0x02,0x10,0x04,0x48,0x08,0x84,0x10,0x83,0x60,
    0x00,0x00,0xF8,0x0F,0x00,0x08,0x00,0x04,0x00,0x04,0x00,0x02,0x00,0x01,0x80,0x00
))
*/
static int l_lcd_drawxbm(lua_State *L){
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int w = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);
    size_t len = 0;
    const char* data = luaL_checklstring(L, 5, &len);
    if (h < 1) return 0; // 行数必须大于0
    if (w < h) return 0; // 起码要填满一行
    uint8_t blen;
    blen = w;
    blen += 7;
    blen >>= 3;
    while( h > 0 ){
      lcd_DrawHXBM(x, y, w, (const uint8_t*)data);
      data += blen;
      y++;
      h--;
    }
    lcd_auto_flush(default_conf);
    lua_pushboolean(L, 1);
    return 1;
}

#ifdef LUAT_USE_TJPGD
#include "luat_tjpgd.h"
/*
显示图片,当前只支持jpg,jpeg
@api lcd.showImage(x, y, file)
@int X坐标
@int y坐标
@string 文件路径
@usage
lcd.showImage(0,0,"/luadb/logo.jpg")
*/
static int l_lcd_showimage(lua_State *L){
    size_t size = 0;
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* input_file = luaL_checklstring(L, 3, &size);
    if (memcmp(input_file+size-4, ".jpg", 5) == 0 || memcmp(input_file+size-4, ".JPG", 5) == 0 || memcmp(input_file+size-5, ".jpeg", 6) == 0 || memcmp(input_file+size-5, ".JPEG", 6) == 0){
      luat_tjpgd_data_t* image_data = luat_tjpgd(input_file);
      if(image_data->fbuf != NULL){
        luat_color_t *source = image_data->fbuf;
        luat_color_t tmp;
        for (size_t i = 0; i < image_data->height; i++){
          for (size_t i = 0; i < image_data->width; i++)
          {
            tmp = *source;
            tmp = (tmp >> 8) + ((tmp & 0xFF) << 8);
            *source = tmp;
            source ++;
          }
        }
        luat_lcd_draw(default_conf, x, y, x+image_data->width-1, y+image_data->height-1, (luat_color_t *)image_data->fbuf);
        luat_heap_free(image_data->fbuf);    /* Discard frame buffer */
        luat_heap_free(image_data);          /* Discard frame buffer */
        lcd_auto_flush(default_conf);
        lua_pushboolean(L, 1);
      }
      else {
        lua_pushboolean(L, 0);
      }
    }else{
      LLOGE("input_file not support");
      lua_pushboolean(L, 0);
    }
    return 1;
}
#endif

/*
主动刷新数据到界面, 仅设置buff且禁用自动属性后使用
@api lcd.flush()
@return bool 成功返回true, 否则返回nil/false
@usgae
-- 本API与 lcd.setupBuff lcd.autoFlush 配合使用
lcd.flush()
*/
static int l_lcd_flush(lua_State* L) {
  luat_lcd_conf_t * conf = NULL;
  if (lua_gettop(L) == 1) {
    conf = lua_touserdata(L, 1);
  }
  else {
    conf = default_conf;
  }
  if (conf == NULL) {
    //LLOGW("lcd not init");
    return 0;
  }
  if (conf->buff == NULL) {
    //LLOGW("lcd without buff, not support flush");
    return 0;
  }
  if (conf->auto_flush) {
    //LLOGI("lcd auto flush is enable, no need for flush");
    return 0;
  }
  luat_lcd_flush(conf);
  lua_pushboolean(L, 1);
  return 0;
}

/*
设置显示缓冲区, 所需内存大小为 2*宽*高 字节. 请衡量内存需求与业务所需的刷新频次.
@api lcd.setupBuff(conf, onheap)
@userdata conf指针, 不需要传
@bool true使用heap内存, false使用vm内存, 默认使用vm内存, 不需要主动传
@return bool 是否成功
@usage
-- 初始化lcd的buff缓冲区, 可理解为FrameBuffer区域.
lcd.setupBuff()
*/
static int l_lcd_setup_buff(lua_State* L) {
  luat_lcd_conf_t * conf = NULL;
  if (lua_gettop(L) == 1) {
    conf = lua_touserdata(L, 1);
  }
  else {
    conf = default_conf;
  }
  if (conf == NULL) {
    LLOGW("lcd not init");
    return 0;
  }
  if (conf->buff != NULL) {
    LLOGW("lcd buff is ok");
    return 0;
  }
  if (lua_isboolean(L, 2) && lua_toboolean(L, 2)) {
    conf->buff = luat_heap_malloc(sizeof(luat_color_t) * conf->w * conf->h);
  }
  else {
    conf->buff = lua_newuserdata(L, sizeof(luat_color_t) * conf->w * conf->h);
    if (conf->buff) {
      conf->buff_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
  }
  if (conf->buff == NULL) {
    LLOGE("lcd buff malloc fail, out of memory?");
    return 0;
  }
  // 先设置为不需要的区间
  conf->flush_y_min = conf->h;
  conf->flush_y_max = 0;
  // luat_lcd_clear 会将区域扩展到整个屏幕
  luat_lcd_clear(default_conf, BACK_COLOR);
  lua_pushboolean(L, 1);
  return 1;
}

/*
设置自动刷新, 需配合lcd.setupBuff使用
@api lcd.autoFlush(enable)
@bool 是否自动刷新,默认为true
@usage
-- 设置buff 并禁用自动更新
lcd.setupBuff()
lcd.autoFlush(false)
-- 禁止自动更新后, 需要使用 lcd.flush() 主动刷新数据到屏幕
*/
static int l_lcd_auto_flush(lua_State* L) {
  luat_lcd_conf_t * conf = default_conf;
  if (conf == NULL) {
    LLOGW("lcd not init");
    return 0;
  }
  conf->auto_flush = lua_toboolean(L, 1);
  lua_pushboolean(L, conf->auto_flush);
  return 1;
}

#include "rotable2.h"
static const rotable_Reg_t reg_lcd[] =
{
    { "init",       ROREG_FUNC(l_lcd_init)},
    { "clear",      ROREG_FUNC(l_lcd_clear)},
    { "fill",       ROREG_FUNC(l_lcd_draw_fill)},
    { "drawPoint",  ROREG_FUNC(l_lcd_draw_point)},
    { "drawLine",   ROREG_FUNC(l_lcd_draw_line)},
    { "drawRectangle",   ROREG_FUNC(l_lcd_draw_rectangle)},
    { "drawCircle", ROREG_FUNC(l_lcd_draw_circle)},
#ifdef LUAT_USE_QRCODE
    { "drawQrcode", ROREG_FUNC(l_lcd_drawQrcode)},
#endif
    { "drawStr",    ROREG_FUNC(l_lcd_draw_str)},
    { "flush",      ROREG_FUNC(l_lcd_flush)},
    { "setupBuff",  ROREG_FUNC(l_lcd_setup_buff)},
    { "autoFlush",  ROREG_FUNC(l_lcd_auto_flush)},
    { "setFont",    ROREG_FUNC(l_lcd_set_font)},
    { "setDefault", ROREG_FUNC(l_lcd_set_default)},
    { "getDefault", ROREG_FUNC(l_lcd_get_default)},
    { "getSize",    ROREG_FUNC(l_lcd_get_size)},
    { "drawXbm",    ROREG_FUNC(l_lcd_drawxbm)},
    { "close",      ROREG_FUNC(l_lcd_close)},
    { "on",         ROREG_FUNC(l_lcd_display_on)},
    { "off",        ROREG_FUNC(l_lcd_display_off)},
    { "sleep",      ROREG_FUNC(l_lcd_sleep)},
    { "wakeup",     ROREG_FUNC(l_lcd_wakeup)},
    { "invon",      ROREG_FUNC(l_lcd_inv_on)},
    { "invoff",     ROREG_FUNC(l_lcd_inv_off)},
    { "cmd",        ROREG_FUNC(l_lcd_write_cmd)},
    { "data",       ROREG_FUNC(l_lcd_write_data)},
    { "setColor",   ROREG_FUNC(l_lcd_set_color)},
    { "draw",       ROREG_FUNC(l_lcd_draw)},
#ifdef LUAT_USE_TJPGD
    { "showImage",    ROREG_FUNC(l_lcd_showimage)},
#endif
#ifdef LUAT_USE_GTFONT
    { "drawGtfontGb2312", ROREG_FUNC(l_lcd_draw_gtfont_gb2312)},
    { "drawGtfontGb2312Gray", ROREG_FUNC(l_lcd_draw_gtfont_gb2312_gray)},
#ifdef LUAT_USE_GTFONT_UTF8
    { "drawGtfontUtf8", ROREG_FUNC(l_lcd_draw_gtfont_utf8)},
    { "drawGtfontUtf8Gray", ROREG_FUNC(l_lcd_draw_gtfont_utf8_gray)},
#endif // LUAT_USE_GTFONT_UTF8
#endif // LUAT_USE_GTFONT
    { "font_unifont_t_symbols",   ROREG_PTR((void*)u8g2_font_unifont_t_symbols)},
    { "font_open_iconic_weather_6x_t", ROREG_PTR((void*)u8g2_font_open_iconic_weather_6x_t)},

    { "font_opposansm8",  ROREG_PTR((void*)u8g2_font_opposansm8)},
    { "font_opposansm10", ROREG_PTR((void*)u8g2_font_opposansm10)},
    { "font_opposansm12", ROREG_PTR((void*)u8g2_font_opposansm12)},
    { "font_opposansm16", ROREG_PTR((void*)u8g2_font_opposansm16)},
    { "font_opposansm18", ROREG_PTR((void*)u8g2_font_opposansm18)},
    { "font_opposansm20", ROREG_PTR((void*)u8g2_font_opposansm20)},
    { "font_opposansm22", ROREG_PTR((void*)u8g2_font_opposansm22)},
    { "font_opposansm24", ROREG_PTR((void*)u8g2_font_opposansm24)},
    { "font_opposansm32", ROREG_PTR((void*)u8g2_font_opposansm32)},
#ifdef USE_U8G2_OPPOSANSM12_CHINESE
    { "font_opposansm12_chinese", ROREG_PTR((void*)u8g2_font_opposansm12_chinese)},
#endif
#ifdef USE_U8G2_OPPOSANSM16_CHINESE
    { "font_opposansm16_chinese", ROREG_PTR((void*)u8g2_font_opposansm16_chinese)},
#endif
#ifdef USE_U8G2_OPPOSANSM24_CHINESE
    { "font_opposansm24_chinese", ROREG_PTR((void*)u8g2_font_opposansm24_chinese)},
#endif
#ifdef USE_U8G2_OPPOSANSM32_CHINESE
    { "font_opposansm32_chinese", ROREG_PTR((void*)u8g2_font_opposansm32_chinese)},
#endif
	  {NULL, ROREG_INT(0)}
};

LUAMOD_API int luaopen_lcd( lua_State *L ) {
    luat_newlib2(L, reg_lcd);
    return 1;
}
