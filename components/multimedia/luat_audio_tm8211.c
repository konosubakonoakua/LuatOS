#include "luat_base.h"
#include "luat_gpio.h"
#include "luat_audio.h"
#include "luat_rtos.h"

#define LUAT_LOG_TAG "tm8211"
#include "luat_log.h"

static int tm8211_codec_init(luat_audio_codec_conf_t* conf,uint8_t mode){
    return 0;
}

static int tm8211_codec_deinit(luat_audio_codec_conf_t* conf){
    // if (conf->pa_pin != LUAT_CODEC_PA_NONE){
    //     luat_gpio_set(conf->pa_pin, !conf->pa_on_level);
    //     luat_gpio_close(conf->pa_pin);
    // }
    return 0;
}

static int tm8211_mode_pwrdown(luat_audio_codec_conf_t* conf){
    // if (conf->power_pin != LUAT_CODEC_PA_NONE){
    //     luat_gpio_set(conf->power_pin, !conf->power_on_level);
    // }
}

static int tm8211_codec_control(luat_audio_codec_conf_t* conf,luat_audio_codec_ctl_t cmd,uint32_t data){
    switch (cmd){
        case LUAT_CODEC_MODE_PWRDOWN:
            tm8211_mode_pwrdown(conf);
            break;
        default:
            break;
    }
    return 0;
}

static int tm8211_codec_start(luat_audio_codec_conf_t* conf){
	luat_audio_pa(conf->multimedia_id,1, 0);
    // if (conf->power_pin != LUAT_CODEC_PA_NONE){
    //     luat_gpio_set(conf->power_pin, conf->power_on_level);
    // }
    return 0;
}

static int tm8211_codec_stop(luat_audio_codec_conf_t* conf){
	luat_audio_pa(conf->multimedia_id,0, 0);
    // if (conf->power_pin != LUAT_CODEC_PA_NONE){
    //     luat_gpio_set(conf->power_pin, !conf->power_on_level);
    // }
    return 0;
}

const luat_audio_codec_opts_t codec_opts_tm8211 = {
    .name = "tm8211",
    .init = tm8211_codec_init,
    .deinit = tm8211_codec_deinit,
    .control = tm8211_codec_control,
    .start = tm8211_codec_start,
    .stop = tm8211_codec_stop,
	.no_control = 1,
};































