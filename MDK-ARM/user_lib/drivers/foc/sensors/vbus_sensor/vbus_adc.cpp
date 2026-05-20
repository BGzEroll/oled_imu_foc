#include "vbus_adc.h"

/**
 * @brief vbus_adc 构造函数
 * 
 * @param adc_handle ADC句柄指针
 */
vbus_adc::vbus_adc(ADC_HandleTypeDef *adc_handle)
    : adc_handle(adc_handle){}

/**
 * @brief vbus_adc 初始化
 * 
 * @param adc_handle ADC句柄指针
 */
void vbus_adc::init()
{
    HAL_ADC_Start_DMA(adc_handle, (uint32_t *)&vbus, 1);
}

/**
 * @brief 获取母线电压
 * 
 * @return 电压值，单位：伏特
 */
float vbus_adc::get_raw_vbus()
{
    return (float)vbus;
}
