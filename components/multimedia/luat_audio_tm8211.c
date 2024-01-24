#include "luat_base.h"
#include "luat_gpio.h"
#include "luat_audio.h"
#include "luat_rtos.h"

#define LUAT_LOG_TAG "tm8211"
#include "luat_log.h"


static int tm8211_codec_init(luat_audio_codec_conf_t* conf,uint8_t mode){
    //如果有dac pin 初始化dac pin
    //如果有pa pin 初始化pa pin
    return 0;
}

static int tm8211_codec_deinit(luat_audio_codec_conf_t* conf){
    //如果有dac pin 控制断电后去初始化
    //如果有pa pin 控制断电后去初始化
    return 0;
}

static void tm8211_codec_pa(luat_audio_codec_conf_t* conf,uint8_t on){
    //如果pa_pin
//    if (conf->pa_pin == LUAT_CODEC_PA_NONE) return;
//	if (on){
//        if (conf->dummy_time_len)
//            luat_rtos_task_sleep(conf->dummy_time_len);
//        luat_gpio_set(conf->pa_pin, conf->pa_on_level);
//        if (conf->pa_delay_time)
//            luat_rtos_task_sleep(conf->pa_delay_time);
//	}else{
//        luat_gpio_set(conf->pa_pin, !conf->pa_on_level);
//	}
}

static int tm8211_codec_control(luat_audio_codec_conf_t* conf,luat_audio_codec_ctl_t cmd,uint32_t data){
    switch (cmd){
        case LUAT_CODEC_SET_PA:
            tm8211_codec_pa(conf,(uint8_t)data);
            break;
        default:
            break;
    }
    return 0;
}

static int tm8211_codec_start(luat_audio_codec_conf_t* conf){
    //dac pin 控制上电, pa_pin 控制使能功放
    return 0;
}

static int tm8211_codec_stop(luat_audio_codec_conf_t* conf){
    //dac pin 控制断电, pa_pin 控制失能功放
    return 0;
}

luat_audio_codec_opts_t codec_opts_tm8211 = {
    .name = "tm8211",
    .init = tm8211_codec_init,
    .deinit = tm8211_codec_deinit,
    .control = tm8211_codec_control,
    .start = tm8211_codec_start,
    .stop = tm8211_codec_stop,
	.no_control = 1,
};
































