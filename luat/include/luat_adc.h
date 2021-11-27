/******************************************************************************
 *  ADC设备操作抽象层
 *  @author wendal
 *  @since 0.1.5
 *****************************************************************************/
#ifndef Luat_ADC_H
#define Luat_ADC_H

#include "luat_base.h"


/******************************************************************************
 * luat_adc_open
 * Description: 打开一个adc通道
 * @param pin[in] adc通道的序号
 * @param args[in] 保留用,传NULL
 * @return 0 成功, 其他值为失败
******************************************************************************/
int luat_adc_open(int pin, void* args);

/******************************************************************************
 * luat_adc_read
 * Description: 读取adc通道的值
 * @param pin[in] adc通道的序号
 * @param val[out] adc通道的原始值
 * @param val2[out] adc通道的计算值,与具体通道有关
 * @return 0 成功, 其他值为失败
******************************************************************************/
int luat_adc_read(int pin, int* val, int* val2);

/******************************************************************************
 * luat_adc_close
 * Description: 关闭adc通道
 * @param pin[in] adc通道的序号
 * @return 0 成功, 其他值为失败
******************************************************************************/
int luat_adc_close(int pin);

#endif
