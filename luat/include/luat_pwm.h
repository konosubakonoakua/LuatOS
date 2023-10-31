
#ifndef Luat_PWM_H
#define Luat_PWM_H

#include "luat_base.h"

typedef struct luat_pwm_conf {
    int channel;
    int stop_level;
    size_t period;
    size_t pulse;
    size_t pnum;
    size_t precision;
    uint8_t reverse;
} luat_pwm_conf_t;

int luat_pwm_open(int channel, size_t period, size_t pulse, int pnum);
int luat_pwm_setup(luat_pwm_conf_t* conf);
int luat_pwm_capture(int channel,int freq);
int luat_pwm_close(int channel);
int luat_pwm_update_dutycycle(int channel, size_t pulse);
#endif
