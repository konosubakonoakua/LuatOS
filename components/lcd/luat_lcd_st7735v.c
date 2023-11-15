#include "luat_base.h"
#include "luat_lcd.h"

#define LUAT_LOG_TAG "st7735v"
#include "luat_log.h"

static const uint16_t st7735v_init_cmds[] = {
    0x02B1,0x0305,0x033A,0x033A,
    0x02B2,0x0305,0x033A,0x033A,
    0x02B3,0x0305,0x033A,0x033A,0x0305,0x033A,0x033A,
    0x02B4, 0x0303,
    0x02C0,0x0362,0x0302,0x0304,
    0x02C1,0x03C0,
    0x02C2,0x030D,0x0300,
    0x02C3,0x038D,0x036A,
    0x02C4,0x038D,0x03EE,
    0x02C5,0x030E,
    0x02E0,0x0310,0x030E,0x0302,0x0303,0x030E,0x0307,0x0302,0x0307,0x030A,0x0312,0x0327,0x0337,0x0300,0x030D,0x030E,0x0310,
    0x02E1,0x0310,0x030E,0x0303,0x0303,0x030F,0x0306,0x0302,0x0308,0x030A,0x0313,0x0326,0x0336,0x0300,0x030D,0x030E,0x0310,
    0x023A,0x0305,
};

const luat_lcd_opts_t lcd_opts_st7735v = {
    .name = "st7735v",
    .init_cmds_len = sizeof(st7735v_init_cmds)/sizeof(st7735v_init_cmds[0]),
    .init_cmds = st7735v_init_cmds,
    .interface_mode = LUAT_LCD_IM_4_WIRE_8_BIT_INTERFACE_I,
    .direction0 = 0xC8,
    .direction90 = 0x78,
    .direction180 = 0x08,
    .direction270 = 0xA8
};
