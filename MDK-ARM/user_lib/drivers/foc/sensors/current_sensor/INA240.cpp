#include "INA240.h"

#include "system/delay.h"

/**
 * @brief 构造函数
 * 
 * @param adc_handle ADC句柄指针
 * @param injected_rank_1 注入通道1
 * @param shunt_resistance  采样电阻值，单位：欧姆
 * @param gain 放益值
 */
INA240::INA240(ADC_HandleTypeDef *adc_handle,
    uint32_t injected_rank_1,
    float shunt_resistance, float gain)
    : adc_handle(adc_handle), injected_rank{injected_rank_1}, shunt_resistance(shunt_resistance), gain(gain)
    {
        rank_num = 1;
    }

/**
 * @brief 构造函数
 * 
 * @param adc_handle ADC句柄指针
 * @param injected_rank_1 注入通道1
 * @param injected_rank_2 注入通道2
 * @param shunt_resistance  采样电阻值，单位：欧姆
 * @param gain 放益值
 */
INA240::INA240(ADC_HandleTypeDef *adc_handle,
    uint32_t injected_rank_1, uint32_t injected_rank_2,
    float shunt_resistance, float gain)
    : adc_handle(adc_handle), injected_rank{injected_rank_1, injected_rank_2}, shunt_resistance(shunt_resistance), gain(gain)
    {
        rank_num = 2;
    }

/**
 * @brief 构造函数
 * 
 * @param adc_handle ADC句柄指针
 * @param injected_rank_1 注入通道1
 * @param injected_rank_2 注入通道2
 * @param injected_rank_3 注入通道3
 * @param shunt_resistance  采样电阻值，单位：欧姆
 * @param gain 放益值
 */
INA240::INA240(ADC_HandleTypeDef *adc_handle,
    uint32_t injected_rank_1, uint32_t injected_rank_2, uint32_t injected_rank_3,
    float shunt_resistance, float gain)
    : adc_handle(adc_handle), injected_rank{injected_rank_1, injected_rank_2, injected_rank_3}, shunt_resistance(shunt_resistance), gain(gain)
    {
        rank_num = 3;
    }

/**
 * @brief 初始化 INA240 传感器
 */
void INA240::init()
{
    HAL_ADCEx_InjectedStart(adc_handle);
}

/**
 * @brief 获取 INA240 传感器的偏移电压
 */
void INA240::get_offset()
{
    uint32_t tmp[3] = {0};
    for(uint8_t i = 0; i < 200; i++)
    {
        delay::ms(1);
        for(uint8_t j = 0; j < rank_num; j++)
        {
            tmp[j] += HAL_ADCEx_InjectedGetValue(adc_handle, injected_rank[j]);
        }
    }

    for(uint8_t j = 0; j < rank_num; j++)
    {
        offset_voltage[j] = (float)tmp[j] / 200.0f / 4095.0f * 3.3f;
    }
}

/**
 * @brief 更新 INA240 传感器的电流和电压
 */
void INA240::update()
{
    for(uint8_t i = 0; i < rank_num; i++)
    {
        raw_data[i] = HAL_ADCEx_InjectedGetValue(adc_handle, injected_rank[i]);
        voltage[i] = (float)raw_data[i] / 4095.0f * 3.3f;
        current[i] = (voltage[i] - offset_voltage[i]) / (shunt_resistance * gain);
    }
}

/**
 * @brief 获取 INA240 传感器的原始电流
 * 
 * @return float* 指向原始电流数组的指针
 */
float *INA240::get_raw_current()
{
    return current;
}

/**
 * @brief 获取 INA240 传感器的原始电压
 * 
 * @return float* 指向原始电压数组的指针
 */
float *INA240::get_raw_voltage()
{
    return voltage;
}
