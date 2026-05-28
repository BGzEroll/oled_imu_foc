#include "sensors.h"

#include "../foc_utils.h"

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
 * @brief 获取 DC 电流
 * 
 * @param e_angle 电角度，单位：弧度
 * 
 * @return DC 电流值，单位：安培
 */
float current_sensors::get_dc_current(float e_angle)
{
    float *raw_current = get_raw_current();
    float sign = 1.0f;

    float i_alpha, i_beta;
    if(!raw_current[2])
    {
        i_alpha = raw_current[0];
        i_beta = _1_SQRT3 * raw_current[0] + _2_SQRT3 * raw_current[1];
    }
    else if(!raw_current[0])
    {
        float i_a = -raw_current[2] - raw_current[1];
        i_alpha = i_a;
        i_beta = _1_SQRT3 * i_a + _2_SQRT3 * raw_current[1];
    }
    else if(!raw_current[1])
    {
        float i_b = -raw_current[0] - raw_current[2];
        i_alpha = raw_current[0];
        i_beta = _1_SQRT3 * raw_current[0] + _2_SQRT3 * i_b;
    }
    else
    {
        float mid = (1.0f / 3.0f) * (raw_current[0] + raw_current[1] + raw_current[2]);
        float i_a = raw_current[0] - mid;
        float i_b = raw_current[1] - mid;
        i_alpha = i_a;
        i_beta = _1_SQRT3 * i_a + _2_SQRT3 * i_b;
    }

    if(e_angle)
    {
        sign = (i_beta * foc_utils::cos(e_angle) - i_alpha * foc_utils::sin(e_angle)) > 0.0f ? 1.0f : -1.0f;
    }

    return sign * _sqrt(i_alpha * i_alpha + i_beta * i_beta);
}

/**
 * @brief 获取 FOC 电流
 * 
 * @param e_angle 电角度，单位：弧度
 * @param i_q q 电流指针
 * @param i_d d 电流指针
 */
void current_sensors::get_foc_current(float e_angle, float *i_q, float *i_d)
{
    float *raw_current = get_raw_current();

    float i_alpha, i_beta;
    if(!raw_current[2])
    {
        i_alpha = raw_current[0];
        i_beta = _1_SQRT3 * raw_current[0] + _2_SQRT3 * raw_current[1];
    }
    else if(!raw_current[0])
    {
        float i_a = -raw_current[2] - raw_current[1];
        i_alpha = i_a;
        i_beta = _1_SQRT3 * i_a + _2_SQRT3 * raw_current[1];
    }
    else if(!raw_current[1])
    {
        float i_b = -raw_current[0] - raw_current[2];
        i_alpha = raw_current[0];
        i_beta = _1_SQRT3 * raw_current[0] + _2_SQRT3 * i_b;
    }
    else
    {
        float mid = (1.0f / 3.0f) * (raw_current[0] + raw_current[1] + raw_current[2]);
        float i_a = raw_current[0] - mid;
        float i_b = raw_current[1] - mid;
        i_alpha = i_a;
        i_beta = _1_SQRT3 * i_a + _2_SQRT3 * i_b;
    }

    float ct = foc_utils::cos(e_angle);
    float st = foc_utils::sin(e_angle);
    *i_q = i_beta * ct - i_alpha * st;
    *i_d = i_alpha * ct + i_beta * st;
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

/**
 * @brief 获取母线电压
 * 
 * @return 电压值，单位：伏特
 */
float vbus_sensors::get_vbus()
{
    return get_raw_vbus();
}
