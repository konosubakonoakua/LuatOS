#include "luat_base.h"
#include "luat_lcd.h"

#define LUAT_LOG_TAG "nv3037"
#include "luat_log.h"

static const uint16_t nv3037_init_cmds[] = {
    0x02fd,0x0306,0x0308,
    0x0283,0x0313,
    0x0286,0x0307,
    0x02F1,0x0301,
    0x0278,0x0384,
    0x027A,0x032c,
    0x02b0,0x030D,0x0304,0x0311,0x0311,0x0311,0x0313,0x0317,0x030f,
    0x02b1,0x0318, 0x0333,
    0x02b2,0x0300,0x0307,0x0308,0x032c,0x032d,0x0330,
    0x02b3,0x030D,0x0304,0x0311,0x0311,0x0311,0x0314,0x0318,0x0310,
    0x02b4,0x0317,0x0334,
    0x02b5,0x0300,0x0307,0x0308,0x032c,0x032d,0x0330,
    0x02F6,0x0301,0x0310,0x0301,
    0x023A,0x0355,
};

luat_lcd_opts_t lcd_opts_nv3037 = {
    .name = "nv3037",
    .init_cmds_len = sizeof(nv3037_init_cmds)/sizeof(nv3037_init_cmds[0]),
    .init_cmds = nv3037_init_cmds,
    .direction0 = 0x00,
    .direction90 = 0xC0,
    .direction180 = 0x70,
    .direction270 = 0xA0
};

