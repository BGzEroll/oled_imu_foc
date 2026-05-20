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
