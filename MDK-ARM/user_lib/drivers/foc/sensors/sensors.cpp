#include "sensors.h"

/**
 * @brief 获取角度
 * 
 * @return 角度值，单位：弧度，范围：[0, 2π)
 */
float encoder_sensors::get_angle()
{
    return get_raw_angle();
}

/**
 * @brief 获取累加角度
 * 
 * @return 累加角度值，单位：弧度
 */
float encoder_sensors::get_full_angle()
{
    return get_raw_full_angle();
}

/**
 * @brief 获取速度
 * 
 * @return 速度值，单位：弧度/秒
 */
float encoder_sensors::get_velocity()
{
    uint32_t id = get_sample_id();
    if(id != sample_id)
    {
        velocity_filter.set_Tf(velocity_filter_Tf);
        velocity = velocity_filter(get_raw_velocity());
        sample_id = id;
    }

    return velocity;
}

/**
 * @brief 获取电流
 * 
 * @param i_a 电流A指针
 * @param i_b 电流B指针
 * @param i_c 电流C指针
 */
void current_sensors::get_current(float *i_a, float *i_b, float *i_c)
{
    float *raw_current = get_raw_current();
    *i_a = raw_current[0];
    *i_b = raw_current[1];
    *i_c = raw_current[2];
}

/**
 * @brief 获取电压
 * 
 * @param v_a 电压A指针
 * @param v_b 电压B指针
 * @param v_c 电压C指针
 */
void current_sensors::get_voltage(float *v_a, float *v_b, float *v_c)
{
    float *raw_voltage = get_raw_voltage();
    *v_a = raw_voltage[0];
    *v_b = raw_voltage[1];
    *v_c = raw_voltage[2];
}
