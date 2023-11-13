#include "luat_base.h"
#include "luat_lcd.h"

#define LUAT_LOG_TAG "gc9a01"
#include "luat_log.h"

static const uint16_t gc9a01_init_cmds[] = {
    0x02EF,
    0x02EB,0x0314,
    0x02FE,
    0x02EF,
    0x02EB,0x0314,
    0x0284,0x0340,
    0x0285,0x03FF,
    0x0286,0x03FF,
    0x0287,0x03FF,
    0x0288,0x030A,
    0x0289,0x0321,
    0x028A,0x0300,
    0x028B,0x0380,
    0x028C,0x0301,
    0x028D,0x0301,
    0x028E,0x03FF,
    0x028F,0x03FF,
    0x02B6,0x0300,0x0320,
    0x023A,0x0305,
    0x0290,0x0308,0x0308,0x0308,0x0308,
    0x02BD,0x0306,
    0x02BC,0x0300,
    0x02FF,0x0360,0x0301,0x0304,
    0x02C3,0x0313,
    0x02C4,0x0313,
    0x02C9,0x0322,
    0x02BE,0x0311,
    0x02E1,0x0310,0x030E,
    0x02DF,0x0321,0x030c,0x0302,
    0x02F0,0x0345,0x0309,0x0308,0x0308,0x0326,0x032A,
    0x02F1,0x0343,0x0370,0x0372,0x0336,0x0337,0x036F,
    0x02F2,0x0345,0x0309,0x0308,0x0308,0x0326,0x032A,
    0x02F3,0x0343,0x0370,0x0372,0x0336,0x0337,0x036F,
    0x02ED,0x031B,0x030B,
    0x02AE,0x0377,
    0x02CD,0x0363,
    0x0270,0x0307,0x0307,0x0304,0x030E,0x030F,0x0309,0x0307,0x0308,0x0303,
    0x02E8,0x0334,
    0x0262,0x0318,0x030D,0x0371,0x03ED,0x0370,0x0370,0x0318,0x030F,0x0371,0x03EF,0x0370,0x0370,
    0x0263,0x0318,0x0311,0x0371,0x03F1,0x0370,0x0370,0x0318,0x0313,0x0371,0x03F3,0x0370,0x0370,
    0x0264,0x0328,0x0329,0x03F1,0x0301,0x03F1,0x0300,0x0307,
    0x0266,0x033C,0x0300,0x03CD,0x0367,0x0345,0x0345,0x0310,0x0300,0x0300,0x0300,
    0x0267,0x0300,0x033C,0x0300,0x0300,0x0300,0x0301,0x0354,0x0310,0x0332,0x0398,
    0x0274,0x0310,0x0385,0x0380,0x0300,0x0300,0x034E,0x0300,
    0x0298,0x033e,0x0307,
    0x0235,
    0x0221,
};


const luat_lcd_opts_t lcd_opts_gc9a01 = {
    .name = "gc9a01",
    .init_cmds_len = sizeof(gc9a01_init_cmds)/sizeof(gc9a01_init_cmds[0]),
    .init_cmds = gc9a01_init_cmds,
    .direction0 = 0x08,
    .direction90 = 0xC8,
    .direction180 = 0x68,
    .direction270 = 0xA8
};

