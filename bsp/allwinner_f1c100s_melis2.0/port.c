#include "port.h"
#include "stdint.h"

#define FB_CNT  2

typedef struct
{
    ES_FILE* hdis;
	__hdle hlayer;
	uint8_t * layer_buf[FB_CNT];    //一个图层使用2个FrameBuffer
    uint32_t width;
    uint32_t height;
    uint32_t layer_buf_len;
	uint8_t fb_index;
    uint8_t color_byte;
    uint8_t test_color;
}display_ctrlstruct;

static display_ctrlstruct g_display;
//初始化显示
static void disp_lcd_init(void)
{	
	__disp_layer_info_t     layer_para;
    __u32 arg[3];
	g_display.hdis   = eLIBs_fopen("b:\\DISP\\DISPLAY", "r+");
	eLIBs_fioctrl(g_display.hdis, DISP_CMD_RESERVE0, 0, NULL);    //加入这句才能开始用驱动，后续还要加入参数来动态选择LCD屏
    eLIBs_fioctrl(g_display.hdis, DISP_CMD_LCD_ON, 0, NULL);    //加入这句才能开始启动LCD
    g_display.width = eLIBs_fioctrl(g_display.hdis, DISP_CMD_SCN_GET_WIDTH, SEL_SCREEN,0); //modified by Derek,2010.12.07.15:05
	g_display.height = eLIBs_fioctrl(g_display.hdis, DISP_CMD_SCN_GET_HEIGHT, SEL_SCREEN,0); //modified by Derek,2010.12.07.15:05
    g_display.fb_index = 0;
    g_display.color_byte = 3;//RGB888
    g_display.layer_buf_len = g_display.width*g_display.height*g_display.color_byte;
    g_display.layer_buf[0] = esMEMS_Palloc(( g_display.layer_buf_len + 1023 ) / 1024, 0);
    g_display.layer_buf[1] = esMEMS_Palloc(( g_display.layer_buf_len + 1023 ) / 1024, 0);
    eLIBs_memset(g_display.layer_buf[0], 0, g_display.layer_buf_len);
    eLIBs_memset(g_display.layer_buf[1], 0, g_display.layer_buf_len);


	eLIBs_memset(&layer_para, 0, sizeof(__disp_layer_info_t));

    layer_para.fb.addr[0] = (__u32)g_display.layer_buf[0];
    layer_para.fb.size.width = g_display.width;
    layer_para.fb.size.height = g_display.height;
    layer_para.fb.mode = DISP_MOD_INTERLEAVED;
    layer_para.fb.format = DISP_FORMAT_RGB888;
    layer_para.fb.br_swap = 0;
    layer_para.fb.seq = DISP_SEQ_ARGB;
    layer_para.ck_enable = 0;
    layer_para.alpha_en = 0;
    layer_para.alpha_val = 0;
    layer_para.pipe = 0;
    layer_para.prio = 0;
    layer_para.src_win.x = 0;
    layer_para.src_win.y = 0;
    layer_para.src_win.width = g_display.width;
    layer_para.src_win.height = g_display.height;
    layer_para.scn_win.x = 0;
    layer_para.scn_win.y = 0;
    layer_para.scn_win.width = g_display.width;
    layer_para.scn_win.height = g_display.height;
    layer_para.mode = DISP_LAYER_WORK_MODE_NORMAL;

    
	arg[0] = DISP_LAYER_WORK_MODE_NORMAL;
	arg[1] = 0;
	arg[2] = 0;
	g_display.hlayer = eLIBs_fioctrl(g_display.hdis, DISP_CMD_LAYER_REQUEST, 0, (void *)arg);
    DBG("layer %x", g_display.hlayer);

	arg[0] = g_display.hlayer;
	arg[1] = (__u32)&layer_para;
	arg[2] = 0;
	eLIBs_fioctrl(g_display.hdis, DISP_CMD_LAYER_SET_PARA, 0, (void *)arg);

	arg[0] = g_display.hlayer;
	arg[1] = 0;
	arg[2] = 0;
	eLIBs_fioctrl(g_display.hdis, DISP_CMD_LAYER_TOP, 0, (void *)arg);

    arg[0] = g_display.hlayer;
    arg[1] = 0;
    arg[2] = 0;
    eLIBs_fioctrl(g_display.hdis, DISP_CMD_LAYER_OPEN, 0, (void*)arg);
}

static void disp_lcd_test(void)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t next_buffer_index = (g_display.fb_index + 1) % FB_CNT;
    __disp_layer_info_t de_lyr;
    __u32 arg[3];

    g_display.test_color = (g_display.test_color + 1) % 3;  //R,G,B轮转测试
    buf = g_display.layer_buf[next_buffer_index];
    i = 0;
    DBG("test %u,%u", g_display.test_color, next_buffer_index);
    while(i < g_display.layer_buf_len)
    {
        eLIBs_memset(&buf[i], 0, 3);
        buf[i + g_display.test_color] = 255;
        i += 3;
    }

    arg[0] = g_display.hlayer;
    arg[1] = (__u32)&de_lyr;
    arg[2] = 0;
    eLIBs_fioctrl(g_display.hdis, DISP_CMD_LAYER_GET_PARA, 0, arg);


    de_lyr.fb.addr[0] = (__u32)buf;
    eLIBs_fioctrl(g_display.hdis, DISP_CMD_LAYER_SET_PARA, 0, arg);
    g_display.fb_index = next_buffer_index;
}

static void port_thread(void *arg)
{
    disp_lcd_init();
    disp_lcd_test();
    while(1)
    {
        esKRNL_TimeDly(1000/SYS_TICK);
        disp_lcd_test();
    }
}

int port_entry(void)
{
    u8 id;
    esKSRV_CloseLogo();
    DBG("entry luatos app!");
    id = esKRNL_TCreate(port_thread, NULL, 0x10000, KRNL_priolevel1);
    DBG("thread id %d!", id);
}